#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#include "../../Common/OpenCLDebugHack.h"
#include "../../../Database/database.h"
#include "../../Common/ll_formulas.h"
#include "../opencl_host.h"
#include "../opencl_bind.h"

#include "../lattice.h"
#include "reference_lattice.h"


void unittest_opencl_mem()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_arg args[6];
	size_t argc = sizeof args / sizeof *args;
	size_t db_len = sizeof db_items / sizeof *db_items;
	cl_ushort *output_stats_s0;
	cl_ushort *output_stats_s3;
	cl_ushort *output_stats_s6;
	stats_t   *output_stats_all;
	c_itemid_t   *output_id;

	CONST_MEM(args[0], "db_items", 0, db_items, sizeof db_items);
	GLOBAL_MEM(args[1], "output_id", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (c_itemid_t)*db_len);
	GLOBAL_MEM(args[2], "output_stats.s0", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (cl_ushort)*db_len);
	GLOBAL_MEM(args[3], "output_stats.s3", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (cl_ushort)*db_len);
	GLOBAL_MEM(args[4], "output_stats.s6", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (cl_ushort)*db_len);
	GLOBAL_MEM(args[5], "output_stats_all", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (stats_t)*db_len);

	opencl_init(&gpu, 1, "kunittest_mem", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = db_len;
	workset.total = db_len;

	opencl_upload(&gpu, args, argc, &workset);
	opencl_run(&gpu, args, argc, &workset);

	output_id        = args[1].buf_data;
	output_stats_s0  = args[2].buf_data;
	output_stats_s3  = args[3].buf_data;
	output_stats_s6  = args[4].buf_data;
	output_stats_all = args[5].buf_data;

	for (size_t i = 0; i < db_len; ++i)
	{
		assert(output_id[i] == db_items[i].id);
		assert(output_stats_s0[i] == db_items[i].stats.s[0]);
		assert(output_stats_s3[i] == db_items[i].stats.s[3]);
		assert(output_stats_s6[i] == db_items[i].stats.s[6]);

		for (size_t j = 0; j < STATS_T_VEC_N; ++j)
		{
			assert(output_stats_all[i].s[j] == db_items[i].stats.s[j]);
		}
	}

	free(args[1].buf_data);
	free(args[2].buf_data);
	free(args[3].buf_data);
	free(args[4].buf_data);
	free(args[5].buf_data);

	opencl_free(&gpu);
}


extern void unittest_lattice_cmp_reference_linext(size_t linext_width, c_ideal_t *le_storage, c_ideal_t *r_le, size_t r_le_n);

void unittest_opencl_le()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_arg args[5];
	size_t argc = sizeof args / sizeof *args;
	struct ideal_lattice lattice;
	buildpath_info info;
	int result;

	result = lattice_create(reference_poset, sizeof reference_poset / sizeof *reference_poset, REFERENCE_POSET_N, &lattice);
	assert(result == G_SUCCESS);
	lattice_valmap(&lattice); // TODO: merge this into lattice create or something

	info.max_neighbors = lattice.max_neighbors;
	info.linext_width  = lattice.linext_width;
	info.linext_count  = lattice.linext_count;
	info.linext_offset = 0;

	GLOBAL_MEM(args[0], "ideals", A_IN, CL_MEM_READ_ONLY, lattice.ideals, lattice.vertex_count*sizeof(*lattice.ideals)*lattice.max_neighbors);
	GLOBAL_MEM(args[1], "counts", A_IN, CL_MEM_READ_ONLY, lattice.counts, lattice.vertex_count*sizeof(*lattice.counts));
	GLOBAL_MEM(args[2], "neighbors", A_IN, CL_MEM_READ_ONLY, lattice.neighbors, lattice.vertex_count*sizeof(*lattice.neighbors)*lattice.max_neighbors);
	VAL(args[3], "max_neighbors", buildpath_info, info);
	GLOBAL_MEM(args[4], "output_le", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (c_ideal_t)*(size_t)info.linext_count*info.linext_width);

	opencl_init(&gpu, 1, "kunittest_le", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size  = (size_t) info.linext_count;
	workset.total      = (size_t) info.linext_count;

	opencl_upload(&gpu, args, argc, &workset);
	opencl_run(&gpu, args, argc, &workset);

	unittest_lattice_cmp_reference_linext(info.linext_width, args[4].buf_data, (c_ideal_t*)reference_le, (size_t)info.linext_count);

	free(args[4].buf_data);
	lattice_free(&lattice);
	opencl_free(&gpu);
}



void unittest_opencl_mergestats()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_arg args[5];
	size_t argc = sizeof args / sizeof *args;
	size_t db_len = sizeof db_items / sizeof *db_items;
	stats_t *output_stats;
	stats_t expected[] = {
		{ 0, 15, 0, 0, 0, 0, 560, 0 },
		{ 70, 0, 0, 12, 0, 0, 0, 0 },
		{ 0, 8, 0, 42, 0, 0, 0, 0 },
		{ 40, 0, 0, 0, 0, 0, 280, 0 }
	};

	#define BUILD_PATH_N  4
	#define BUILD_PATH_WIDTH 3
	c_itemid_t build_path[BUILD_PATH_N][BUILD_PATH_WIDTH] = {
		{6,7,10},
		{15,16,18},
		{18,19,20},
		{22,24,28}
	};

	CONST_MEM(args[0], "db_items", 0, db_items, sizeof db_items);
	CONST_MEM(args[1], "db_passives", 0, db_passives, sizeof db_passives);
	CONST_MEM(args[2], "build_path", 0, build_path, sizeof build_path);
	VAL(args[3], "build_path_width", cl_uint, BUILD_PATH_WIDTH);

	GLOBAL_MEM(args[4], "output_stats", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (stats_t)*BUILD_PATH_N);

	opencl_init(&gpu, 1, "kunittest_mergestats", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size  = BUILD_PATH_N;
	workset.total      = BUILD_PATH_N;

	opencl_upload(&gpu, args, argc, &workset);
	opencl_run(&gpu, args, argc, &workset);

	output_stats = args[4].buf_data;

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < STATS_T_VEC_N; ++j)
		{
			assert(output_stats[i].s[j] == expected[i].s[j]);
		}
	}

	free(args[4].buf_data);
	opencl_free(&gpu);
}

/*
void kunittest_passiveuniqe_cpu(
	item_t items[],
	stats_t passives[],
	c_itemid_t *xs_in,
	cl_uint xs_n,
	cl_ushort2 (*output)[LINEXT_WIDTH_MAX],
	c_ideal_t (*output_idx)[LINEXT_WIDTH_MAX],
	size_t id
	)
{
	cl_ushort2 unique[LINEXT_WIDTH_MAX];
	c_ideal_t unique_i[LINEXT_WIDTH_MAX];
	c_itemid_t xs[LINEXT_WIDTH_MAX];

	size_t i;

	xs_in += id * xs_n;

	for (i = 0; i < xs_n; ++i)
		xs[i] = xs_in[i];

	PassiveUnique(items, passives, xs, xs_n, unique, unique_i);

	for (i = 0; i < xs_n; ++i)
	{
		output[id][i] = unique[i];
		output_idx[id][i] = unique_i[i];
	}
}
*/

void unittest_opencl_passiveunique()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_arg args[6];
	size_t argc = sizeof args / sizeof *args;
	size_t db_len = sizeof db_items / sizeof *db_items;
#undef BUILD_PATH_N
#undef BUILD_PATH_WIDTH
#define BUILD_PATH_N  3
#define BUILD_PATH_WIDTH 6
	c_itemid_t build_path[BUILD_PATH_N][BUILD_PATH_WIDTH] = {
		{ 14,  14, 130, 20, 102, 137 },
		{ 130, 14, 130, 20, 102, 137 },
		{ 137, 130, 130, 20, 130, 137 },
	};
	cl_ushort2 expected[BUILD_PATH_N][LINEXT_WIDTH_MAX] =
	{
		{ { 0, 0 }, { 6, 0 }, { 8, 0 } },
		{ { 0, 0 }, { 6, 0 }, { 8, 0 } },
		{ { 0, 0 }, { 8, 0 }, { 6, 0 } },
	};
	c_ideal_t expected_idx[BUILD_PATH_N][LINEXT_WIDTH_MAX] = {
		{ 0, 0, 1, 0, 0, 2 },
		{ 1, 0, 1, 0, 0, 2 },
		{ 1, 2, 2, 0, 2, 1 },
	};

	cl_ushort2 (*output)[LINEXT_WIDTH_MAX];
	c_ideal_t (*output_idx)[LINEXT_WIDTH_MAX];

	//////////////////////////////////////
	/*
	cl_ushort2 xoutput[BUILD_PATH_N][LINEXT_WIDTH_MAX];
	c_ideal_t  xoutput_idx[BUILD_PATH_N][LINEXT_WIDTH_MAX];

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		kunittest_passiveuniqe_cpu(db_items, db_passives, build_path, BUILD_PATH_WIDTH, xoutput, xoutput_idx, i);
	}

	return;
	*/
	//////////////////////////////////////
	
	CONST_MEM(args[0], "db_items", 0, db_items, sizeof db_items);
	CONST_MEM(args[1], "db_passives", 0, db_passives, sizeof db_passives);
	CONST_MEM(args[2], "build_path", 0, build_path, sizeof build_path);
	VAL(args[3], "build_path_width", cl_uint, BUILD_PATH_WIDTH);

	GLOBAL_MEM(args[4], "output_unique", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (cl_ushort2)*LINEXT_WIDTH_MAX*BUILD_PATH_N);
	GLOBAL_MEM(args[5], "output_unique_id", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (c_ideal_t)*LINEXT_WIDTH_MAX*BUILD_PATH_N);

	opencl_init(&gpu, 1, "kunittest_passiveuniqe", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = BUILD_PATH_N;
	workset.total = BUILD_PATH_N;

	opencl_upload(&gpu, args, argc, &workset);
	opencl_run(&gpu, args, argc, &workset);

	output     = args[4].buf_data;
	output_idx = args[5].buf_data;

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < BUILD_PATH_WIDTH; ++j)
		{
			assert(output[i][j].s[0] == expected[i][j].s[0]);
			assert(output[i][j].s[1] == expected[i][j].s[1]);
			assert(output_idx[i][j] == expected_idx[i][j]);

		}
	}

	free(args[4].buf_data);
	free(args[5].buf_data);
	opencl_free(&gpu);
}



void unittest_opencl_clearsubcomponents()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_arg args[7];
	size_t argc = sizeof args / sizeof *args;
	size_t db_len = sizeof db_items / sizeof *db_items; // TODO: replace with DB_LEN
#undef BUILD_PATH_N
#undef BUILD_PATH_WIDTH
#define BUILD_PATH_N  3
#define BUILD_PATH_WIDTH 6
	c_itemid_t build_path[BUILD_PATH_N][BUILD_PATH_WIDTH] = {
		{ 14, 14, 130, 20, 102, 137 },
		{ 130, 14, 130, 20, 102, 137 },
		{ 137, 130, 130, 20, 130, 137 },
	};
	cl_int expected_inventory[BUILD_PATH_N][LINEXT_WIDTH_MAX] = {
		{ 1, 2, 1, 2, 2, 1 },
		{ 1, 2, 2, 3, 3, 2 },
		{ 1, 2, 3, 4, 5, 5 }
	};
	stats_t expected_stats[BUILD_PATH_N] = { 
		{30,15,0,0,20,0,0,0},
		{55,15,0,0,30,0,0,0},
		{110,38,0,0,30,0,0,0}
	};

	cl_int (*output_inventory)[LINEXT_WIDTH_MAX];
	stats_t *output_stats;

	//////////////////////////////////////
	//////////////////////////////////////

	CONST_MEM(args[0], "db_items", 0, db_items, sizeof db_items);
	CONST_MEM(args[1], "db_passives", 0, db_passives, sizeof db_passives);
	CONST_MEM(args[2], "db_buildtree", 0, db_buildtree, sizeof db_buildtree);
	CONST_MEM(args[3], "build_path", 0, build_path, sizeof build_path);
	VAL(args[4], "build_path_width", cl_uint, BUILD_PATH_WIDTH);

	GLOBAL_MEM(args[5], "output_inventory", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (cl_int)*LINEXT_WIDTH_MAX*BUILD_PATH_N);
	GLOBAL_MEM(args[6], "output_stats", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof (stats_t)*BUILD_PATH_N);

	opencl_init(&gpu, 1, "kunittest_clearsubcomponents", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = BUILD_PATH_N;
	workset.total = BUILD_PATH_N;

	opencl_upload(&gpu, args, argc, &workset);
	opencl_run(&gpu, args, argc, &workset);

	output_inventory = args[5].buf_data;
	output_stats     = args[6].buf_data;

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < STATS_T_VEC_N; ++j){
			assert(output_stats[i].s[j] == expected_stats[i].s[j]);
		}
		for (size_t j = 0; j < BUILD_PATH_WIDTH; ++j){
			assert(output_inventory[i][j] == expected_inventory[i][j]);
		}
	}

	free(args[5].buf_data);
	free(args[6].buf_data);
	opencl_free(&gpu);
}


void unittest_opencl_llformulas()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_arg args[4];
	size_t argc = sizeof args / sizeof *args;
	size_t db_len = sizeof db_items / sizeof *db_items; // TODO: replace with DB_LEN
#undef BUILD_PATH_N
#undef BUILD_PATH_WIDTH
#define BUILD_PATH_N  4
	c_itemid_t build_path[BUILD_PATH_N] = { 14, 130, 20, 19 };
	cl_float expected[BUILD_PATH_N][7] = {
		{ 200.89f, 40.30f, 0.5f, 80.0f, 0.0f, 1.0f,  80.0f },
		{ 238.56f, 47.85f, 0.5f, 95.0f, 0.0f, 1.0f,  95.0f },
		{ 192.70f, 40.90f, 0.5f, 70.0f, 1.16f, 1.0f, 70.0f },
		{ 196.29f, 42.10f, 0.5f, 70.0f, 0.0f, 1.2f,  70.0f }
	};
	cl_float (*output)[7];
	llf_criteria cfg = { 0 };
	cfg.time_frame = 3;
	cfg.ad_ratio = 200;
	cfg.ap_ratio = 0;
	cfg.level = 18;
	cfg.enemy_armor = 100;
	cfg.enemy_mr = 100;
	cfg.build_maxcost = 15000;
	cfg.build_maxinventory = 6;

	CONST_MEM(args[0], "db_items", 0, db_items, sizeof db_items);
	CONST_MEM(args[1], "build_path", 0, build_path, sizeof build_path);
	CONST_MEM(args[2], "cfg_input", 0, &cfg, sizeof cfg);

	GLOBAL_MEM(args[3], "output_inventory", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof expected);
	
	opencl_init(&gpu, 1, "kunittest_llformulas", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = BUILD_PATH_N;
	workset.total = BUILD_PATH_N;

	opencl_upload(&gpu, args, argc, &workset);
	opencl_run(&gpu, args, argc, &workset);

	output = args[3].buf_data;

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < BUILD_PATH_N; ++j)
		{
			long a = lroundf(output[i][j] * 100);
			long b = lroundf(expected[i][j] * 100);
			assert(a == b);
		}
	}

	free(args[3].buf_data);
	opencl_free(&gpu);
}


void unittest_opencl()
{
	unittest_opencl_mem();
	unittest_opencl_le();
	unittest_opencl_mergestats();
	unittest_opencl_passiveunique();
	unittest_opencl_clearsubcomponents();
	unittest_opencl_llformulas();
}

