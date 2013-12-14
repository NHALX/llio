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

//#include "../error.h"
#include "opencl_host/dummy.h"
#include "opencl_host/host.h"
#include "league/database/database.h"
#include "league/ll_formulas.h"
#include "poset/lattice.h"
#include "poset/kernel/lattice_kernel.h"

#include "../kernel/metric_ADPS.h"
#include "../db_input.h"
#include "../opencl_bind.h"
#include "league/unit_test/find_max.h"

//////////////////////////////////////////////////////////////////




static result_t TestBuildPathGPU(ideal_lattice *lattice, item_t *db_filtered, size_t db_len, llf_criteria *cfg)
{
	cl_ulong time;
	cl_ulong i;

	opencl_workset workset;
	opencl_context gpu;
    opencl_function *function;
    opencl_kernel_arg *output;
	result_t max;
	size_t result_n;
    count_t linext_offset = 0;

    opencl_init(&gpu, 1);
	function = clbp_init(&gpu);
	workset = clbp_bind(function, lattice, db_filtered, db_len, cfg, &output);
	
	
    result_n = output->buf_size / sizeof (result_t);
	max = (result_t){ 0, 0 };
	
	for (i = 0, time = 0; i < workset.iterations; ++i)
	{
		result_t local_max;
		
        time += opencl_run(function, CLBP_KERNEL_N, TRUE, linext_offset, &workset);
		linext_offset += workset.pass_size;

        local_max = FindMax(output->buf_data, result_n);

		if (max.metric < local_max.metric)
			max = local_max;

		//printf("GPU: progress: %llu/%llu (%f)\n", i + 1, workset.iterations, (float)(i + 1) / (float)workset.iterations);
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
	//time=63382346  processed=1966080
	//time=72518895  processed=1966080
    //time=26234248  processed=1966080
    //time=57284564  processed=1966080
    //time=42149902  processed=1966080 (const db)

    opencl_function_free(function, CLBP_KERNEL_N);
    opencl_free(&gpu);
	return max;
}


static result_t TestBuildPathCPU(ideal_lattice *lattice, item_t *db_filtered, size_t db_len, llf_criteria *cfg)
{
	size_t pass_size;
    count_t j,iterations;
	result_t result = (result_t){0,0};
	lattice_info info;
    count_t linext_offset = 0;

	info.max_neighbors = lattice->max_neighbors;
	info.linext_width  = lattice->linext_width;
	info.linext_count  = lattice->linext_count;


	pass_size = 1000000; // TODO: is this suffeciently large to 

	if (pass_size > lattice->linext_count)
		pass_size = (size_t) lattice->linext_count;

	iterations = lattice->linext_count / pass_size;

	printf("CPU: n=%llu, pass_size=%u\n", lattice->linext_count, pass_size);

	for (j = 0; j < iterations; ++j)
	{
		int i;
		result_t local_best;
        ideal_t le_buf[LINEXT_WIDTH_MAX];
        result_t r;

		#pragma omp parallel default(shared) private(i,r,local_best,le_buf)
		{
			local_best = (result_t){ 0, 0 };

			#pragma omp for schedule(static) nowait
			for (i = 0; i < pass_size; ++i)
			{
                linext_nth(lattice, le_buf, linext_offset + i, 0);
                r = metric_ADPS(le_buf, db_filtered, cfg, &info, linext_offset + i);

				if (local_best.metric < r.metric)
					local_best = r;
			}
			
			#pragma omp critical 
			{
				if (result.metric < local_best.metric)
					result = local_best;
			}
		}

		linext_offset += pass_size;
		printf("CPU: progress: %llu/%llu (%f)\n", j+1, iterations, (float)(j+1) / (float)iterations);
	}

	return result;
}


static void
PrintExtension(ideal_lattice *il, item_t *db_filtered, itemid_t *idmap, result_t max)
{
	size_t i;
	itemid_t expected[] = { 14, 14, 130, 20, 102, 137 };

	if (max.metric == 0)
		printf("No viable build found.\n");
	
	else
	{
        ideal_t le_buf[LINEXT_WIDTH_MAX];
        linext_nth(il, le_buf, max.index, 0);
        linext_print(le_buf, il->linext_width);

		for (i = 0; i < il->linext_width; ++i)
		{
			itemid_t index = db_filtered[le_buf[i]].id;
			//assert(index == expected[i]);
			assert(dbi_find(index) == idmap[le_buf[i]-1]);
			printf("%s, ", db_names[idmap[le_buf[i] - 1]]);
		}

		printf("\n");
	}
}





typedef result_t(*test_func)(ideal_lattice *, item_t *, size_t, llf_criteria *);


static void TestBuildpathALL()
{
	int result;
	ideal_lattice lattice;
	size_t i; 
	test_func tests[] = {
		&TestBuildPathCPU,
        &TestBuildPathGPU
	};
	#define ITEM_LEN (sizeof(items)/sizeof(*items))
    //char *items[] = { "Youmuu's Ghostblade", "The Bloodthirster"};
    char *items[] = { "The Bloodthirster", "Warmog's Armor" };
	//char *items[] = { "Youmuu's Ghostblade" };
	itemid_t global_idx[IDMAP_MAX_WIDTH*ITEM_LEN];
	
	ideal_t poset[BUILDTREE_MAX*ITEM_LEN][2];
	size_t poset_n;
	size_t vertex_n;
	item_t *db_filtered;
	size_t db_len;

	vertex_n = dbi_poset(items, ITEM_LEN, global_idx, poset, &poset_n);
	result = lattice_create(poset, poset_n, vertex_n, &lattice);
	assert(result == G_SUCCESS);

	db_filtered = dbi_filter(vertex_n, global_idx, &db_len);
	assert(db_filtered);

	for (i = 0; i < sizeof tests / sizeof *tests; ++i)
	{
		result_t max;
		llf_criteria cfg = { 0 };
		cfg.time_frame    = 3;
		cfg.ad_ratio      = 3.4f;
		cfg.ap_ratio      = 0;
		cfg.level         = 18;
		cfg.enemy_armor   = 100;
		cfg.enemy_mr      = 100;
		cfg.build_maxcost = 15000;
		cfg.build_maxinventory = 6;

		max = tests[i](&lattice, db_filtered, db_len, &cfg);
		printf("max=%f,i=%d\n", max.metric, max.index);
		PrintExtension(&lattice, db_filtered, global_idx, max);
	}

	lattice_free(&lattice);
	free(db_filtered);
}


extern void unittest_lattice(int quiet);
extern void unittest_opencl();
extern void unittest_db_input();

void unittest_buildpath()
{
    glbinit_lattice();
    unittest_lattice(1);
    unittest_db_input();
    unittest_opencl();
    TestBuildpathALL();
}



