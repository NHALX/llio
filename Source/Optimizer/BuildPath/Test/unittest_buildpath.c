#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "../error.h"
#include "../../Common/ll_formulas.h"
#include "../../../Database/database.h"
#include "../../Common/OpenCLDebugHack.h"
#include "../lattice.h"
#include "../db_input.h"
#include "../opencl_host.h"



////////////////////// from opencl kernel ///////////////////////

void k_linext_print(c_linext_t *le);

int k_linext_nth(
	__constant c_ideal_t *ideals,
	__constant c_count_t *counts,
	__constant c_index_t *adjacency,
	uint  max_neighbors,
	c_count_t nth_extension,
	c_linext_t *le);
 
c_result_t k_buildpath(
	__constant cl_float items[][ITEM_WIDTH],
	__constant cl_float passives[][PASSIVE_WIDTH],
	__constant cl_short buildtree[][BUILDTREE_WIDTH],
	__constant cl_float *cfg,
	__constant c_itemid_t node2id[],

	__global c_ideal_t *ideals,
	__global c_count_t *counts,
	__global c_index_t *adjacency,
	buildpath_info info,
	size_t global_id
);

/*
void cpu_buildpath(
	__constant cl_float items[][ITEM_WIDTH],
	__constant cl_float passives[][PASSIVE_WIDTH],
	__constant cl_short buildtree[][BUILDTREE_WIDTH],
	__constant cl_float *cfg,
	__constant c_itemid_t node2id[],

	__global c_ideal_t *ideals,
	__global c_count_t *counts,
	__global c_index_t *adjacency,
	buildpath_info info,
	size_t global_id,
	c_result_t *output)
{
	c_result_t r = k_buildpath(items, passives, buildtree, cfg, node2id, ideals, counts, adjacency, info, global_id);

	if ((*output).metric < r.metric)
		(*output) = r;
}
*/

//////////////////////////////////////////////////////////////////
#define ROUND_UP(N,M) ((N + M - 1) / M * M)

static c_result_t
FindMax(c_result_t *rs, size_t result_n)
{
	c_result_t max = { 0, 0 };
	size_t i;

	for (i = 0; i < result_n; ++i)
	{
		if (max.metric < rs[i].metric)
			max = rs[i];
	}

	return max;
}


c_result_t unittest_buildpathGPU(struct ideal_lattice *lattice, c_itemid_t *node2id, size_t node2id_n, cl_float *cfg)
{
	cl_ulong time, processed;
	c_count_t global_size;
	size_t local_size, iterations, pass_size, saturation;
	size_t i;
	struct gpu_arg args[KERNEL_ARG_LEN];
	struct gpu_context gpu;
	buildpath_info info;
	c_result_t max;
	size_t result_n;

	info.max_neighbors = lattice->max_neighbors;
	info.linext_width = lattice->linext_width;
	info.linext_offset = 0;
	info.linext_count = lattice->linext_count;

	opencl_init(&gpu, 1, "kernel_buildpath", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	
	saturation = 128;
	local_size = gpu.cfg_max_workgroup_size; 
	pass_size = (gpu.cfg_compute_units * saturation * local_size);
	global_size = lattice->linext_count;

	if (pass_size > global_size)
		pass_size = ROUND_UP(global_size,2);

	if (local_size > pass_size)
		local_size = ROUND_UP(pass_size,2);

	global_size = ROUND_UP(global_size, local_size);
	pass_size = ROUND_UP(pass_size, local_size);
	iterations = ceil((double)global_size / (double)pass_size);

	printf("OpenCL: n=%llu, pass_size=%u, localsize=%u\n", global_size, pass_size, local_size);
	assert(local_size % 2 == 0); // NOTE: the reduction algorithm in the kernel needs a power of 2 local size

	opencl_setargs(&gpu, args, lattice, node2id, node2id_n, cfg, info, pass_size, local_size);
	
	result_n = args[KERNEL_OUTPUT_0].buf_size / sizeof (c_result_t);
	max = (c_result_t){ 0, 0 };

	for (i = 0, time = 0, processed = 0; i < iterations && processed < 393216000; ++i, processed += pass_size)
	{
		c_result_t local_max;

		time               += opencl_run(&gpu, args, KERNEL_ARG_LEN, info, pass_size, local_size);
		info.linext_offset += pass_size;

		local_max = FindMax(args[KERNEL_OUTPUT_0].buf_data, result_n);
		if (max.metric < local_max.metric)
			max = local_max;
	}

	printf("time=%llu, processed=%d\n", time/i, processed); 
	//time=25209728, processed=1330688
	//time=89936320, processed=1330688
	//time=181802912, processed = 1330688
		
	
	return max;
}


c_result_t unittest_buildpathCPU(struct ideal_lattice *lattice, c_itemid_t *node2id, size_t node2id_n, cl_float *cfg)
{
	size_t j;
	c_count_t n;

	size_t local_size, pass_size, iterations;
	c_count_t global_size, processed;
	c_result_t result;
	buildpath_info info;

	info.max_neighbors = lattice->max_neighbors;
	info.linext_width  = lattice->linext_width;
	info.linext_offset = 0;
	info.linext_count  = lattice->linext_count;

	n = lattice->linext_count;

	pass_size   = 1000000;//67108864;
	local_size  = 1;
	global_size = n;

	if (pass_size > global_size)
		pass_size = global_size;

	iterations = global_size / pass_size;

	printf("CPU: n=%llu, pass_size=%u\n", global_size, pass_size);

	// TODO: dont forget to remove this artifical limit
	for (j = 0, processed = 0; j < iterations && processed < 393216000; ++j, processed += pass_size)
	{
		int i;
		c_result_t local_best;

		#pragma omp parallel default(shared) private(i,local_best)
		{
			local_best = (c_result_t){ 0, 0 };

			#pragma omp for schedule(static) nowait
			for (i = 0; i < pass_size; ++i)
			{
				c_result_t r = k_buildpath(db_items, db_passives, db_buildtree, cfg, node2id,
					lattice->ideals,
					lattice->counts,
					lattice->neighbors,
					info, i);

				if (local_best.metric < r.metric)
					local_best = r;
			}
			
			#pragma omp critical 
			{
				if (result.metric < local_best.metric)
					result = local_best;
			}
		}

		info.linext_offset += pass_size;
		printf("CPU: progress: %d/%d (%f)\n", j+1, iterations, (float)(j+1) / (float)iterations);
	}

	return result;
}

static void
PrintExtension(struct ideal_lattice *il, c_itemid_t *idmap, c_result_t max)
{
	size_t i;
	c_linext_t le;
	c_itemid_t expected[] = { 14, 14, 130, 20, 102, 137 };

	le.le_index = il->linext_width;
	le.le_len = il->linext_width;
	k_linext_nth(il->ideals, il->counts, il->neighbors, il->max_neighbors, max.index, &le);
	k_linext_print(&le);

	for (i = 0; i < il->linext_width; ++i)
	{
		c_itemid_t index = idmap[le.le_buf[i] - 1];
		//assert(index == expected[i]);
		printf("%s, ", db_names[index]);
	}
	
	printf("\n");
}





typedef c_result_t (*test_func)(struct ideal_lattice *, c_itemid_t *, size_t, cl_float *);


void unittest_buildpath()
{
	int result;
	struct ideal_lattice lattice;
	size_t i;
	test_func tests[] = {
//		&unittest_buildpathGPU,
		&unittest_buildpathCPU 
	};
	#define ITEM_LEN (sizeof(items)/sizeof(*items))
	char *items[] = { "Last Whisper", "The Bloodthirster", "Blade of the Ruined King" };
	c_itemid_t node2dbi[IDMAP_MAX_WIDTH*ITEM_LEN];
	c_ideal_t poset[BUILDTREE_MAX*ITEM_LEN][2];
	size_t poset_n;
	size_t vertex_n;


	vertex_n = dbi_poset(items, ITEM_LEN, node2dbi, poset, &poset_n);
	result = lattice_create(poset, poset_n, vertex_n, &lattice);
	assert(result == G_SUCCESS);
	lattice_valmap(&lattice);

	for (i = 0; i < sizeof tests / sizeof *tests; ++i)
	{
		c_result_t max;
		cl_float cfg[CFG_SIZE] = { 3.0f, 3.4f, 100.0f, 18.0f, 15000.0f };

		max = tests[i](&lattice, node2dbi, vertex_n, cfg);
		printf("max=%f,i=%d\n", max.metric, max.index);
		PrintExtension(&lattice, node2dbi, max);
	}

	lattice_free(&lattice);
}



extern void unittest_lattice(int quiet);

int main()
{
	glbinit_lattice();
	unittest_lattice(1);
	unittest_buildpath();
	return 0;
}




/*
Starting: 20,1
Args: db_size=10464, passive_size=480, cfg_size=20, id_size=12, mask_size=68, co
unt_size=136, adjacency_size=51, max_neighbor=3, combo_len=6
[0]={1010.394226,0}
[1]={1010.394226,1}
[2]={1019.491089,2}
[3]={1027.983643,3}
[4]={1061.424683,4}
[5]={1019.491089,5}
[6]={1027.983643,6}
[7]={1061.424683,7}
[8]={1028.950684,8}
[9]={1037.443237,9}
[10]={1070.884277,10}
[11]={1046.419189,11}
[12]={1079.860352,12}
[13]={1113.389038,13}
[14]={1028.950684,14}
[15]={1037.443237,15}
[16]={1070.884277,16}
[17]={1046.419189,17}
[18]={1079.860352,18}
[19]={1113.389038,19}
max=1113.389038,i=13
============remake=================================================
CL_INFO: compute_units=30, const_size=65536, work_group_size=512, little_endian=
1
OpenCL: allocating 11214/65536 const storage.
Starting: 20,1
Args: db_size=10464, passive_size=480, cfg_size=20, id_size=12, ideals_size=51,
count_size=136, adjacency_size=51, max_neighbor=3, combo_len=6
[0]={1010.394226,0}
[1]={1010.394226,1}
[2]={1019.491089,2}
[3]={1027.983643,3}
[4]={1061.424683,4}
[5]={1019.491089,5}
[6]={1027.983643,6}
[7]={1061.424683,7}
[8]={1028.950684,8}
[9]={1037.443237,9}
[10]={1070.884277,10}
[11]={1046.419189,11}
[12]={1079.860352,12}
[13]={1113.389038,13}
[14]={1028.950684,14}
[15]={1037.443237,15}
[16]={1070.884277,16}
[17]={1046.419189,17}
[18]={1079.860352,18}
[19]={1113.389038,19}
max=1113.389038,i=13

CL_INFO: compute_units=30, const_size=65536, work_group_size=512, little_endian=
1
OpenCL: allocating 11367/65536 const storage.
Starting: 20,1
Args: db_size=10464, passive_size=480, cfg_size=20, id_size=12, ideals_size=51,
count_size=136, adjacency_size=204, max_neighbor=3, combo_len=6
[0]={1010.394226,0}
[1]={1010.394226,1}
[2]={1019.491089,2}
[3]={1027.983643,3}
[4]={1061.424683,4}
[5]={1019.491089,5}
[6]={1027.983643,6}
[7]={1061.424683,7}
[8]={1028.950684,8}
[9]={1037.443237,9}
[10]={1070.884277,10}
[11]={1046.419189,11}
[12]={1079.860352,12}
[13]={1113.389038,13}
[14]={1028.950684,14}
[15]={1037.443237,15}
[16]={1070.884277,16}
[17]={1046.419189,17}
[18]={1079.860352,18}
[19]={1113.389038,19}
max=1113.389038,i=13


*/