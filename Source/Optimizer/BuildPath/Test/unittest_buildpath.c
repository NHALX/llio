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

extern c_result_t *gpu_LE(struct gpu_context *ctx, struct ideal_lattice *g, c_itemid_t *idmap, size_t idmap_len, cl_float cfg_input[CFG_SIZE], size_t *result_n);
extern void * gpu_init(char *, char *);


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
size_t FindItemIndex(c_itemid_t item)
{
	size_t i;

	for (i = 0; i < sizeof db_items / sizeof *db_items; i++)
	{
		if (db_items[i][F_ID] == item)
			return i;
	}

	assert(0);
	return (size_t) -1;
}



//i = linear_extension(ideals, counts, adjacency, max_neighbors, (count_t)nth_extension, &le);

c_result_t *unittest_buildpathGPU(struct ideal_lattice *lattice, c_itemid_t *node2id, size_t node2id_n, cl_float *cfg, size_t *result_n)
{
	c_count_t n;
	c_result_t *results;

	void *gpu_ctx;

	n = lattice->linext_count;

	gpu_ctx = gpu_init("D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	results = gpu_LE(gpu_ctx, lattice, node2id, node2id_n, cfg, result_n);

	free(gpu_ctx);
	return results;
}


c_result_t *unittest_buildpathCPU(struct ideal_lattice *lattice, c_itemid_t *node2id, size_t node2id_n, cl_float *cfg, size_t *result_n)
{
	size_t i;
	c_count_t n;

	size_t global_size, local_size;
	c_result_t *scratch;
	c_result_t *results;

	n = lattice->linext_count;

	global_size = n;
	local_size = 1;// global_size / 2;
	*result_n = n / local_size;

	results = calloc(*result_n, sizeof(*results));
	scratch = calloc(local_size, sizeof(*results));

	MAINLOOP(i, kernel_LE(db_items, db_passives, cfg, node2id,
		lattice->ideals,
		lattice->counts,
		lattice->neighbors,
		lattice->max_neighbors,
		lattice->linext_width, scratch, results), global_size, local_size);

	free(scratch);
	return results;
}


void
PrintResults(c_result_t *results, size_t result_n)
{
	c_result_t maximum = { -1, -1 };
	size_t i;

	for (i = 0; i < result_n; ++i)
	{
		printf("[%d]={%f,%d}\n", i, results[i].metric, results[i].index);
		if (maximum.metric < results[i].metric)
			maximum = results[i];

	}
	printf("max=%f,i=%d\n", maximum.metric, maximum.index);
	free(results);
}



void unittest_buildpath()
{
	struct ideal_lattice lattice;
	cl_float cfg[CFG_SIZE] = { 3.0f, 3.4f, 100.0f, 18.0f, 15000.0f };
	c_ideal_t poset[][2] = {
		{ 3, 1 },
		{ 5, 4 },
		{ 6, 4 },
		{ 1, 2 },
		{ 4, 2 }
	};
	
	c_itemid_t node2id[6];
	size_t node2id_n = sizeof node2id / sizeof *node2id;
	size_t result_n;
	c_result_t *results;

	node2id[1 - 1] = FindItemIndex(3093); // Avarice Blade
	node2id[2 - 1] = FindItemIndex(3142); // Youmuu's Ghostblade
	node2id[3 - 1] = FindItemIndex(1051); // Brawlers Gloves
	node2id[4 - 1] = FindItemIndex(3134); // The Brutalizer
	node2id[5 - 1] = FindItemIndex(1036); // Long Sword
	node2id[6 - 1] = FindItemIndex(1036); // Long Sword

	assert(lattice_create(poset, sizeof poset / sizeof *poset, 6, &lattice) == G_SUCCESS);
	lattice_valmap(&lattice);

	results = unittest_buildpathCPU(&lattice, node2id, node2id_n, cfg, &result_n);
	PrintResults(results, result_n);
	results = unittest_buildpathGPU(&lattice, node2id, node2id_n, cfg, &result_n);
	PrintResults(results, result_n);

	lattice_free(&lattice);
}



extern void unitest_lattice(int quiet);

int main()
{
	glbinit_lattice();
	unittest_lattice(1);
	unittest_buildpath();
	return 0;
}