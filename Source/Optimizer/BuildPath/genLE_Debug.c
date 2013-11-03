#ifndef __OPENCL_VERSION__ 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

//#include <Windows.h>

#include <time.h>

#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "le_test.h"
#include "../Common/ll_formulas.h"
#include "../../Database/database.h"

#include "../Common/OpenCLDebugHack.h"

void init_all(struct graph *g)
{
	init_graph(g, LE_T_DATA_LEN, ideals, adjacency, LE_T_ADJACENCY_MAX, LE_T_EXTENSION_SIZE, node2id, LE_T_IDMAP_LEN);
	init_count(g);
}


extern c_result_t * gpu_LE(struct gpu_context *ctx, struct graph *g, cl_float *cfg_input, size_t * result_n);
extern void * gpu_init(char *, char *);

//#define DEBUG_ON_CPU
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

int main2()
{
	void *gpu_ctx;
	struct graph g;
	int i;
	c_count_t n;
	c_result_t *results;
	c_result_t maximum = { -1, -1 };
	cl_float cfg[CFG_SIZE] = { 3.0f, 3.4f, 100.0f, 18.0f, 15000.0f };

#ifdef DEBUG_ON_CPU
	int global_size, local_size;
	c_result_t *scratch;
#endif
	int result_n;

	init_all(&g);


	n = g.counts[1]; // TODO: make sure this is a valid assumption

#ifdef DEBUG_ON_CPU
	global_size = n;
	local_size = 1;// global_size / 2;
	result_n    = n / local_size;

	results = calloc(result_n, sizeof(*results));
	scratch = calloc(local_size, sizeof(*results));

	MAINLOOP(i, kernel_LE(db_items, db_passives, cfg, node2id, g.ideals, g.counts, g.adjacency, g.max_neighbors, g.combo_len, scratch, results), global_size, local_size);
#else
	gpu_ctx = gpu_init("D:/GitRoot/llio/Source/Optimizer/BuildPath/genLE_Kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	results = gpu_LE(gpu_ctx, &g, cfg, &result_n);
#endif

	for (i = 0; i < result_n; ++i)
	{
		printf("[%d]={%f,%d}\n", i, results[i].metric, results[i].index);
		if (maximum.metric < results[i].metric)
			maximum = results[i];
			
	}

	printf("max=%f,i=%d\n", maximum.metric, maximum.index);
	

	free(results);

#ifdef DEBUG_ON_CPU
	free(scratch);
#else
	free(gpu_ctx);
#endif
	return 0;
}

#endif

