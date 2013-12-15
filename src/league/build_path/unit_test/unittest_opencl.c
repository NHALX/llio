#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#include "opencl_host/dummy.h"
#include "opencl_host/host.h"
#include "league/database/database.h"
#include "league/ll_formulas.h"

#include "../opencl_bind.h"
#include "poset/lattice.h"
#include "poset/kernel/lattice_kernel.h"
#include "poset/unit_test/reference_lattice.h"

#define UTCL_DEF "-DUNIT_TEST -ID:/GitRoot/llio/src/"
#define UTCL_SRC_LATTICE  "#include \"poset/kernel/lattice_kernel.c\""
#define UTCL_SRC "#include \"league/build_path/unit_test/unittest_kernel.cl\""


static int VECTOR_CMP(VECTOR(a), VECTOR(b))
{
	for (size_t j = 0; j < VECTOR_VEC_N; ++j)
	{
		stat_t x = PART(a, j) - PART(b,j);
		if (x != 0)
			return (x > 0) ? 1 : -1;
	}

	return 0;
}


void unittest_opencl_mem()
{
	opencl_workset workset;
	opencl_context gpu;
    opencl_function *func;
	itemid_t   *output_id;
	stat_t *output_stats_s0;
	stat_t *output_stats_s3;
	stat_t *output_stats_s6;
	VECTOR(*output_stats_all);
	
    opencl_init(&gpu, 1);

    func = opencl_build(&gpu, "kunittest_mem", UTCL_SRC, UTCL_DEF);
    
    ka_mconst(func, "db_items", 0, db_items, sizeof db_items);
    output_id        = KA_DYN_OUTPUT(func, "output_id", sizeof *output_id * DB_LEN);
    output_stats_s0  = KA_DYN_OUTPUT(func, "output_stats.s0", sizeof *output_stats_s0 * DB_LEN);
    output_stats_s3  = KA_DYN_OUTPUT(func, "output_stats.s3", sizeof *output_stats_s3 * DB_LEN);
    output_stats_s6  = KA_DYN_OUTPUT(func, "output_stats.s6", sizeof *output_stats_s6 * DB_LEN);
    output_stats_all = KA_DYN_OUTPUT(func, "output_stats_all", sizeof *output_stats_all * DB_LEN);

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size  = DB_LEN;
	workset.total      = DB_LEN;

    opencl_run(func, 1, FALSE, 0, &workset);
	
	for (size_t i = 0; i < DB_LEN; ++i)
	{
		assert(output_id[i] == db_items[i].id);
		assert(output_stats_s0[i] == db_items[i].stats[0]);
		assert(output_stats_s3[i] == db_items[i].stats[3]);
		assert(output_stats_s6[i] == db_items[i].stats[6]);
		assert(VECTOR_CMP(output_stats_all[i], db_items[i].stats) == 0);
	}

    opencl_function_free(func, 1);
	opencl_free(&gpu);
}


extern void unittest_lattice_cmp_reference_linext(size_t linext_width, ideal_t *le_storage, ideal_t *r_le, size_t r_le_n);


void unittest_opencl_le()
{
	opencl_workset workset;
	opencl_context gpu;
    opencl_function *func;
	ideal_lattice lattice;
	lattice_info info;
	int result;
	void *output;

    ulong_t linext_offset = 0;

    opencl_init(&gpu, 1);
    func = opencl_build(&gpu, "linext", UTCL_SRC_LATTICE, UTCL_DEF);


	result = lattice_create(reference_poset, sizeof reference_poset / sizeof *reference_poset, REFERENCE_POSET_N, &lattice);
	assert(result == G_SUCCESS);
 
	info.max_neighbors = lattice.max_neighbors;
	info.linext_width  = lattice.linext_width;
	info.linext_count  = lattice.linext_count;
	
    ka_value(func, "offset", &linext_offset, sizeof linext_offset);
	ka_mglobal(func, "ideals", A_IN, CL_MEM_READ_ONLY, lattice.ideals, lattice.vertex_count*sizeof(*lattice.ideals)*lattice.max_neighbors);
	ka_mglobal(func, "counts", A_IN, CL_MEM_READ_ONLY, lattice.counts, lattice.vertex_count*sizeof(*lattice.counts));
	ka_mglobal(func, "neighbors", A_IN, CL_MEM_READ_ONLY, lattice.neighbors, lattice.vertex_count*sizeof(*lattice.neighbors)*lattice.max_neighbors);
	ka_value(func, "max_neighbors", &info, sizeof info);
	output = KA_DYN_OUTPUT(func, "output", sizeof (ideal_t)*(size_t)info.linext_count*info.linext_width);

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size  = (size_t) info.linext_count;
	workset.total      = (size_t) info.linext_count;

    opencl_run(func, 1, FALSE, 0, &workset);

	unittest_lattice_cmp_reference_linext(info.linext_width, output, (ideal_t*)reference_le, (size_t)info.linext_count);

	lattice_free(&lattice);
    opencl_function_free(func, 1);
    opencl_free(&gpu);
}

void
PushUsualArgs(opencl_function *func, item_t *items, size_t item_len, size_t elem_size, void *xs, size_t xs_n, cl_uint xs_width)
{
    ka_mconst(func, "db_items", 0, items, sizeof *items * item_len);
    ka_mconst(func, "xs", 0, xs, elem_size * xs_width * xs_n);
    ka_value(func, "xs_n", &xs_width, sizeof xs_width);
}

void unittest_opencl_mergestats()
{
	item_t db[] = {
		{ 0 },

		{ { 1, 0, 3, 0, 5, 0, 7, 0 },
		{ 0, 2, 0, 0, 0, 0, 0, 0 }, 0 },
		{ { 2, 0, 4, 4, 6, 0, 8, 0 },
		{ 0, 0, 0, 0, 0, 6, 0, 0 }, 0 },
		{ { 3, 0, 5, 0, 7, 0, 9, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 8 }, 0 },
	};

	opencl_workset workset;
	opencl_context gpu;
    opencl_function *func;

	VECTOR(*output_stats);
	VECTOR(expected[]) = {
		{6,2,12,4,18,6,24,8}
	};

	#define BUILD_PATH_N  1
	#define BUILD_PATH_WIDTH 3
	itemid_t build_path[BUILD_PATH_N][BUILD_PATH_WIDTH] = {
		{1,2,3}
	};


    opencl_init(&gpu, 1);
    func = opencl_build(&gpu, "kunittest_mergestats", UTCL_SRC, UTCL_DEF);



	PushUsualArgs(func, db, sizeof db / sizeof *db, sizeof *build_path, build_path, BUILD_PATH_N, BUILD_PATH_WIDTH);
	output_stats = KA_DYN_OUTPUT(func, "output_stats", sizeof *output_stats * BUILD_PATH_N);

	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size  = BUILD_PATH_N;
	workset.total      = BUILD_PATH_N;

    opencl_run(func, 1, FALSE, 0, &workset);

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		assert(VECTOR_CMP(output_stats[i], expected[i]) == 0);
	}

    opencl_function_free(func, 1);
    opencl_free(&gpu);
}


#define I_LONGSWORD      1
#define I_BRUTALIZER     2
#define I_BRAWLERSGLOVES 3
#define I_AVARICEBLADE   4
#define I_GHOSTBLADE     5
#define I_RECURVEBOW     6

#define TEST_DB_LEN 7

#ifdef INTEGER_STATS
#define DB_PERCENT(X) X
#else
#define DB_PERCENT(X) ((float)X/100)
#endif

item_t test_db[TEST_DB_LEN] = { 
		{0},
		{{10,0,0,0,0,0,0,0}, 
		 {0,0,0,0,0,0,0,0}, 
		 1036,0,400,400,0,         
		 {0},
		 {0}} /*Long Sword*/,
		{{25,0,0,0,0,0,0,0}, 
		 {0,0,0,0,10,0,0,0}, 
		 3134,6,1337,537,2,         
		 { I_LONGSWORD, I_LONGSWORD },
		 {0}} /*The Brutalizer*/,
		{{0,DB_PERCENT(8),0,0,0,0,0,0}, 
		 {0,0,0,0,0,0,0,0}, 
		 1051,0,400,400,0,         
		 {0},
		 {0}} /*Brawler's Gloves*/,
		{{0,DB_PERCENT(10),0,0,0,0,0,0}, 
		 {0,0,0,0,0,0,0,0}, 
		 3093,0,800,400,1,         
		 {3},
		 {0}} /*Avarice Blade*/,
		{{30,DB_PERCENT(15),0,0,0,0,0,0}, 
		 {0,0,0,0,20,0,0,0}, 
		 3142,8,2700,563,2,         
		 { I_BRUTALIZER, I_AVARICEBLADE },
		 {0}} /*Youmuu's Ghostblade*/,
		{{0,0,0,DB_PERCENT(30),0,0,0,0}, 
		 {0,0,0,0,0,0,0,0}, 
		 1043,0,900,900,0,         
		 {0},
		 {0}} /*Recurve Bow*/,
	};



void unittest_opencl_clearsubcomponents()
{
	opencl_workset workset;
	opencl_context gpu;
    opencl_function *func;

#undef BUILD_PATH_N
#undef BUILD_PATH_WIDTH
#define BUILD_PATH_N  3
#define BUILD_PATH_WIDTH 6
	itemid_t build_path[BUILD_PATH_N][BUILD_PATH_WIDTH] = {
		{ I_LONGSWORD, I_LONGSWORD, I_BRUTALIZER, I_BRAWLERSGLOVES, I_AVARICEBLADE, I_GHOSTBLADE },
		{ I_LONGSWORD, I_LONGSWORD, I_LONGSWORD, I_LONGSWORD, I_BRUTALIZER, I_BRUTALIZER },
		{ I_RECURVEBOW, I_LONGSWORD, I_LONGSWORD, I_BRAWLERSGLOVES, I_BRUTALIZER, I_RECURVEBOW },
	};
	cl_int expected_inventory[BUILD_PATH_N][LINEXT_WIDTH_MAX] = {
		{ 1, 2, 1, 2, 2, 1 },
		{ 1, 2, 3, 4, 3, 2 },
		{ 1, 2, 3, 4, 3, 4 }
	};
	VECTOR(expected_stats[BUILD_PATH_N]) = { 
		{30,DB_PERCENT(15),0,0,20,0,0,0},
		{50,0,0,0,10,0,0,0},
		{25,DB_PERCENT(8),0,DB_PERCENT(60),10,0,0,0}
	};

	cl_int (*output_inventory)[LINEXT_WIDTH_MAX];
	VECTOR(*output_stats);

    opencl_init(&gpu, 1);
    func = opencl_build(&gpu, "kunittest_clearsubcomponents", UTCL_SRC, UTCL_DEF);


	ka_mlocal(func, "pasv_scratch", TEST_DB_LEN*sizeof(ideal_t)*BUILD_PATH_N);
	PushUsualArgs(func, test_db, TEST_DB_LEN, sizeof *build_path, build_path, BUILD_PATH_N, BUILD_PATH_WIDTH);
	output_inventory = KA_DYN_OUTPUT(func, "output_inventory", sizeof (*output_inventory) * BUILD_PATH_N);
	output_stats = KA_DYN_OUTPUT(func, "output_stats", sizeof (*output_stats) * BUILD_PATH_N);
	
	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = BUILD_PATH_N;
	workset.total = BUILD_PATH_N;

    opencl_run(func, 1, FALSE, 0, &workset);

	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		assert(VECTOR_CMP(output_stats[i], expected_stats[i]) == 0);

		for (size_t j = 0; j < BUILD_PATH_WIDTH; ++j){
			assert(output_inventory[i][j] == expected_inventory[i][j]);
		}
	}

    opencl_function_free(func, 1);
    opencl_free(&gpu);
}


void unittest_opencl_llformulas()
{
	opencl_workset workset;
	opencl_context gpu;
    opencl_function *func;

#undef BUILD_PATH_N
#undef BUILD_PATH_WIDTH
#define BUILD_PATH_N  4
	itemid_t build_path[BUILD_PATH_N] = { I_LONGSWORD, I_BRUTALIZER, I_BRAWLERSGLOVES, I_RECURVEBOW };
	cl_float expected[BUILD_PATH_N][7] = {
		{ 200.89f, 40.30f, 0.5f, 80.0f, 1.0f, 1.0f,  80.0f },
		{ 238.56f, 47.85f, 0.5f, 95.0f, 1.0f, 1.0f,  95.0f },
		{ 184.24f, 38.08f, 0.5f, 70.0f, 1.08f, 1.0f, 70.0f },
		{ 196.29f, 42.10f, 0.5f, 70.0f, 1.0f, 1.2f,  70.0f }
	};
	cl_float (*output)[7];
	llf_criteria cfg = { 0 };

    opencl_init(&gpu, 1);
    func = opencl_build(&gpu, "kunittest_llformulas", UTCL_SRC, UTCL_DEF);


	cfg.time_frame = 3;
	cfg.ad_ratio = 2.0f;
	cfg.ap_ratio = 0;
	cfg.level = 18;
	cfg.enemy_armor = 100;
	cfg.enemy_mr = 100;
	cfg.build_maxcost = 15000;
	cfg.build_maxinventory = 6;
    cfg.metric_type = METRIC_ALL_IN;

	ka_mconst(func, "db_items", 0, test_db, sizeof test_db);
	ka_mconst(func, "build_path", 0, build_path, sizeof build_path);
	ka_value(func, "cfg_input", &cfg, sizeof cfg);

	output = KA_DYN_OUTPUT(func, "output_inventory", sizeof expected);

	
	workset.iterations = 1;
	workset.local_size = 1;
	workset.pass_size = BUILD_PATH_N;
	workset.total = BUILD_PATH_N;

    opencl_run(func, 1, FALSE, 0, &workset);


	for (size_t i = 0; i < BUILD_PATH_N; ++i)
	{
		for (size_t j = 0; j < BUILD_PATH_N; ++j)
		{
			long a = lroundf(output[i][j] * 100);
			long b = lroundf(expected[i][j] * 100);
			assert(a == b);
		}
	}

    opencl_function_free(func, 1);
    opencl_free(&gpu);
}



void unittest_opencl()
{
	unittest_opencl_mem();
	unittest_opencl_le();
	unittest_opencl_mergestats();
	unittest_opencl_clearsubcomponents();
	unittest_opencl_llformulas();
}

