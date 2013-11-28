#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "../error.h"
#include "../../Common/OpenCLDebugHack.h"
#include "../../../Database/database.h"
#include "../../Common/ll_formulas.h"

#include "../lattice.h"
#include "../db_input.h"
#include "../opencl_host.h"
#include "../opencl_bind.h"

#include "../le_kernel.h"


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
	__constant item_t items[],
	__constant llf_criteria *cfg,
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

static c_result_t
FindMax(c_result_t *rs, size_t result_n)
{
	c_result_t max = { 0, 0 };
	size_t i;

	for (i = 0; i < result_n; ++i)
	{
		if (rs[i].metric <= 0 && rs[i].index != ERROR_INVENTORY && rs[i].index != ERROR_IGNORED)
			printf("warning: %f:%d\n", rs[i].metric, rs[i].index);

		if (max.metric < rs[i].metric)
			max = rs[i];
	}

	return max;
}


c_result_t unittest_buildpathGPU(struct ideal_lattice *lattice, item_t *db_filtered, c_itemid_t *node2id, size_t node2id_n, llf_criteria *cfg)
{
	cl_ulong time;
	size_t i;

	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_params args = OPENCL_KERNEL_PARAMS_INIT;
	opencl_kernel_arg *output, *bpinfo;

	buildpath_info info;
	c_result_t max;
	size_t result_n;

	info.max_neighbors = lattice->max_neighbors;
	info.linext_width = lattice->linext_width;
	info.linext_offset = 0;
	info.linext_count = lattice->linext_count;

	opencl_init(&gpu, 1, "kernel_buildpath", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	workset = clbp_bindmem(&gpu, &args, lattice, db_filtered, node2id, node2id_n, cfg, &output, &bpinfo);
	opencl_memcheck(&gpu, &args, &workset);
	
	result_n = output->buf_size / sizeof (c_result_t);
	max = (c_result_t){ 0, 0 };
	
	for (i = 0, time = 0; i < workset.iterations; ++i)
	{
		c_result_t local_max;
		
		clbp_bindval(&gpu, bpinfo, info);
		time += opencl_run(&gpu, &args, &workset);
		info.linext_offset += workset.pass_size;

		local_max = FindMax(output->buf_data, result_n);
		if (max.metric < local_max.metric)
			max = local_max;
	}

	printf("time=%llu\n", time/i); 
	//time=25209728, processed=1330688
	//time=89936320, processed=1330688
	//time=181802912, processed = 1330688
	//time=525624219, processed = 82575360
	//time=28561166, processed=1330688
	//time=55859429
	//time=58635767, processed=1966080
	//time=55623763, processed=1966080
	//time=31413480, processed=1966080
	//time=66087542, processed=1966080
	//time=70518173
	return max;
}


c_result_t unittest_buildpathCPU(struct ideal_lattice *lattice, item_t *db_filtered, c_itemid_t *node2id, size_t node2id_n, llf_criteria *cfg)
{
	c_count_t j, pass_size, iterations;
	c_result_t result = (c_result_t){0,0};
	buildpath_info info;

	info.max_neighbors = lattice->max_neighbors;
	info.linext_width  = lattice->linext_width;
	info.linext_offset = 0;
	info.linext_count  = lattice->linext_count;


	pass_size   = 1000000;//67108864;

	if (pass_size > lattice->linext_count)
		pass_size = lattice->linext_count;

	iterations = lattice->linext_count / pass_size;

	printf("CPU: n=%llu, pass_size=%u\n", lattice->linext_count, pass_size);

	for (j = 0; j < iterations; ++j)
	{
		int i;
		c_result_t local_best;

		#pragma omp parallel default(shared) private(i,local_best)
		{
			local_best = (c_result_t){ 0, 0 };

			#pragma omp for schedule(static) nowait
			for (i = 0; i < pass_size; ++i)
			{
				c_result_t r = k_buildpath(db_filtered, cfg, node2id,
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
		printf("CPU: progress: %llu/%llu (%f)\n", j+1, iterations, (float)(j+1) / (float)iterations);
	}

	return result;
}

static void
PrintExtension(struct ideal_lattice *il, item_t *db_filtered, c_itemid_t *idmap, c_result_t max)
{
	size_t i;
	c_linext_t le;
	c_itemid_t expected[] = { 14, 14, 130, 20, 102, 137 };

	if (max.metric == 0)
		printf("No viable build found.\n");
	
	else
	{
		le.le_index = il->linext_width;
		le.le_len = il->linext_width;
		k_linext_nth(il->ideals, il->counts, il->neighbors, il->max_neighbors, max.index, &le);
		k_linext_print(&le);

		for (i = 0; i < il->linext_width; ++i)
		{
			c_itemid_t index = db_filtered[le.le_buf[i]].id;
			//assert(index == expected[i]);
			assert(dbi_find(index) == idmap[le.le_buf[i]-1]);
			printf("%s, ", db_names[idmap[le.le_buf[i] - 1]]);
		}

		printf("\n");
	}
}





typedef c_result_t(*test_func)(struct ideal_lattice *, item_t *, c_itemid_t *, size_t, llf_criteria *);


void unittest_buildpath()
{
	int result;
	struct ideal_lattice lattice;
	size_t i; 
	test_func tests[] = {
		&unittest_buildpathCPU,
//		&unittest_buildpathGPU
	};
	#define ITEM_LEN (sizeof(items)/sizeof(*items))
	char *items[] = { "Last Whisper", "The Bloodthirster", "Blade of the Ruined King" }; 
	//char *items[] = { "Youmuu's Ghostblade" };
	c_itemid_t global_idx[IDMAP_MAX_WIDTH*ITEM_LEN];
	
	c_ideal_t poset[BUILDTREE_MAX*ITEM_LEN][2];
	size_t poset_n;
	size_t vertex_n;
	item_t *db_filtered;

	vertex_n = dbi_poset(items, ITEM_LEN, global_idx, poset, &poset_n);
	result = lattice_create(poset, poset_n, vertex_n, &lattice);
	assert(result == G_SUCCESS);
	lattice_valmap(&lattice); // TODO: merge this into lattice create or something

	db_filtered = dbi_filter(vertex_n, global_idx);

	for (i = 0; i < sizeof tests / sizeof *tests; ++i)
	{
		c_result_t max;
		llf_criteria cfg = { 0 };
		cfg.time_frame    = 3;
		cfg.ad_ratio      = 3.4f;
		cfg.ap_ratio      = 0;
		cfg.level         = 18;
		cfg.enemy_armor   = 100;
		cfg.enemy_mr      = 100;
		cfg.build_maxcost = 15000;
		cfg.build_maxinventory = 6;

		max = tests[i](&lattice, db_filtered, global_idx, vertex_n, &cfg);
		printf("max=%f,i=%d\n", max.metric, max.index);
		PrintExtension(&lattice, db_filtered, global_idx, max);
	}

	lattice_free(&lattice);
	free(db_filtered);
}



extern void unittest_lattice(int quiet);
extern void unittest_opencl();
extern void unittest_db_input();


int main()
{
	glbinit_lattice();
	unittest_lattice(1);
	unittest_db_input();
	unittest_opencl();
	unittest_buildpath();
	return 0;
}



