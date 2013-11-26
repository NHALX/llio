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
	opencl_kernel_params args = OPENCL_KERNEL_PARAMS_INIT;
	c_itemid_t   *output_id;
	cl_ushort *output_stats_s0;
	cl_ushort *output_stats_s3;
	cl_ushort *output_stats_s6;
	stats_t   *output_stats_all;
	
	opencl_init(&gpu, 1, "kunittest_mem", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	ka_mconst(&gpu,  ka_push(&args), "db_items", 0, db_items, sizeof db_items);

	output_id        = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_id", sizeof (c_itemid_t)*DB_LEN);
	output_stats_s0  = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_stats.s0", sizeof (cl_ushort)*DB_LEN);
	output_stats_s3  = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_stats.s3", sizeof (cl_ushort)*DB_LEN);
	output_stats_s6  = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_stats.s6", sizeof (cl_ushort)*DB_LEN);
	output_stats_all = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_stats_all", sizeof (stats_t)*DB_LEN);

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size  = DB_LEN;
	workset.total      = DB_LEN;

	opencl_upload(&gpu, &args, &workset);
	opencl_run(&gpu, &args, &workset);

	for (size_t i = 0; i < DB_LEN; ++i)
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

	ka_free(&args);
	opencl_free(&gpu);
}


extern void unittest_lattice_cmp_reference_linext(size_t linext_width, c_ideal_t *le_storage, c_ideal_t *r_le, size_t r_le_n);

void unittest_opencl_le()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_params args = OPENCL_KERNEL_PARAMS_INIT;
	struct ideal_lattice lattice;
	buildpath_info info;
	int result;
	void *output;

	result = lattice_create(reference_poset, sizeof reference_poset / sizeof *reference_poset, REFERENCE_POSET_N, &lattice);
	assert(result == G_SUCCESS);
	lattice_valmap(&lattice); // TODO: merge this into lattice create or something

	opencl_init(&gpu, 1, "kunittest_le", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");

	info.max_neighbors = lattice.max_neighbors;
	info.linext_width  = lattice.linext_width;
	info.linext_count  = lattice.linext_count;
	info.linext_offset = 0;

	ka_mglobal(&gpu, ka_push(&args), "ideals", A_IN, CL_MEM_READ_ONLY, lattice.ideals, lattice.vertex_count*sizeof(*lattice.ideals)*lattice.max_neighbors);
	ka_mglobal(&gpu, ka_push(&args), "counts", A_IN, CL_MEM_READ_ONLY, lattice.counts, lattice.vertex_count*sizeof(*lattice.counts));
	ka_mglobal(&gpu, ka_push(&args), "neighbors", A_IN, CL_MEM_READ_ONLY, lattice.neighbors, lattice.vertex_count*sizeof(*lattice.neighbors)*lattice.max_neighbors);
	ka_value(ka_push(&args), "max_neighbors", &info, sizeof info);
	output = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output", sizeof (c_ideal_t)*(size_t)info.linext_count*info.linext_width);

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size  = (size_t) info.linext_count;
	workset.total      = (size_t) info.linext_count;

	opencl_upload(&gpu, &args, &workset);
	opencl_run(&gpu, &args, &workset);

	unittest_lattice_cmp_reference_linext(info.linext_width, output, (c_ideal_t*)reference_le, (size_t)info.linext_count);

	lattice_free(&lattice);
	ka_free(&args);
	opencl_free(&gpu);
}

void
PushUsualArgs(opencl_context *gpu, opencl_kernel_params *args, size_t elem_size, void *xs, size_t xs_n, cl_uint xs_width)
{
	ka_mconst(gpu, ka_push(args), "db_items", 0, db_items, sizeof db_items);
	ka_mconst(gpu, ka_push(args), "xs", 0, xs, elem_size * xs_width * xs_n);
	ka_value(ka_push(args), "xs_n", &xs_width, sizeof xs_width);
}

void unittest_opencl_mergestats()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_params args = OPENCL_KERNEL_PARAMS_INIT;
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

	opencl_init(&gpu, 1, "kunittest_mergestats", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	PushUsualArgs(&gpu, &args, sizeof *build_path, build_path, BUILD_PATH_N, BUILD_PATH_WIDTH);
	output_stats = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_stats", sizeof (stats_t)*BUILD_PATH_N);

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size  = BUILD_PATH_N;
	workset.total      = BUILD_PATH_N;

	opencl_upload(&gpu, &args, &workset);
	opencl_run(&gpu, &args, &workset);

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < STATS_T_VEC_N; ++j)
		{
			assert(output_stats[i].s[j] == expected[i].s[j]);
		}
	}

	ka_free(&args);
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
	opencl_kernel_params args = OPENCL_KERNEL_PARAMS_INIT;

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
	opencl_init(&gpu, 1, "kunittest_passiveuniqe", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	PushUsualArgs(&gpu, &args, sizeof *build_path, build_path, BUILD_PATH_N, BUILD_PATH_WIDTH);
	output = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_unique", sizeof (cl_ushort2)*LINEXT_WIDTH_MAX*BUILD_PATH_N);
	output_idx = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_unique_id", sizeof (c_ideal_t)*LINEXT_WIDTH_MAX*BUILD_PATH_N);

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = BUILD_PATH_N;
	workset.total = BUILD_PATH_N;

	opencl_upload(&gpu, &args, &workset);
	opencl_run(&gpu, &args, &workset);

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < BUILD_PATH_WIDTH; ++j)
		{
			assert(output[i][j].s[0] == expected[i][j].s[0]);
			assert(output[i][j].s[1] == expected[i][j].s[1]);
			assert(output_idx[i][j] == expected_idx[i][j]);

		}
	}

	ka_free(&args);
	opencl_free(&gpu);
}



void unittest_opencl_clearsubcomponents()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_params args = OPENCL_KERNEL_PARAMS_INIT;

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
	opencl_init(&gpu, 1, "kunittest_clearsubcomponents", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	PushUsualArgs(&gpu, &args, sizeof *build_path, build_path, BUILD_PATH_N, BUILD_PATH_WIDTH);
	output_inventory = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_inventory", sizeof (cl_int)*LINEXT_WIDTH_MAX*BUILD_PATH_N);
	output_stats = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_stats", sizeof (stats_t)*BUILD_PATH_N);
	
	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = BUILD_PATH_N;
	workset.total = BUILD_PATH_N;

	opencl_upload(&gpu, &args, &workset);
	opencl_run(&gpu, &args, &workset);

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < STATS_T_VEC_N; ++j){
			assert(output_stats[i].s[j] == expected_stats[i].s[j]);
		}
		for (size_t j = 0; j < BUILD_PATH_WIDTH; ++j){
			assert(output_inventory[i][j] == expected_inventory[i][j]);
		}
	}

	ka_free(&args);
	opencl_free(&gpu);
}


void unittest_opencl_llformulas()
{
	opencl_workset workset;
	opencl_context gpu;
	opencl_kernel_params args = OPENCL_KERNEL_PARAMS_INIT;
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

	opencl_init(&gpu, 1, "kunittest_llformulas", "D:/GitRoot/llio/Source/Optimizer/BuildPath/opencl_kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	
	ka_mconst(&gpu, ka_push(&args), "db_items", 0, db_items, sizeof db_items);
	ka_mconst(&gpu, ka_push(&args), "build_path", 0, build_path, sizeof build_path);
	ka_mconst(&gpu, ka_push(&args), "cfg_input", 0, &cfg, sizeof cfg);
	output = KA_DYN_OUTPUT(&gpu, ka_push(&args), "output_inventory", sizeof expected);

	
	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = BUILD_PATH_N;
	workset.total = BUILD_PATH_N;

	opencl_upload(&gpu, &args, &workset);
	opencl_run(&gpu, &args, &workset);


	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < BUILD_PATH_N; ++j)
		{
			long a = lroundf(output[i][j] * 100);
			long b = lroundf(expected[i][j] * 100);
			assert(a == b);
		}
	}

	ka_free(&args);
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

