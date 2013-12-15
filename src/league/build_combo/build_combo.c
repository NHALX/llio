#ifndef _MSC_VER
#include <alloca.h>
#else
#define alloca _alloca
#endif
#include <stdio.h>

#include "league/build_combo/build_combo.h"
#include "combinatorics/combination.h"
#include "combinatorics/factorial.h"

#include "league/unit_test/find_max.h" // TODO: remove this
#include "league/build_combo/kernel/k_build_combo.h"

void build_comboGPU(opencl_function *func, llf_criteria *cfg, const item_t *items, size_t item_n, size_t inventory_n, setmax_t *combo)
{
    ulong_t ncombo = NK_MULTISET(item_n, inventory_n);

    opencl_workset workset;
    opencl_kernel_arg *output;
    result_t *results, local_best, best = { 0, 0 };

    workset = opencl_workcfg(func->ctx, ncombo, build_combo__allocnfo__(func, item_n));
    output = build_combo__bind__(func, inventory_n, items, item_n, cfg, workset.pass_size, workset.local_size);
    results = output->buf_data;

    for (ulong_t i = 0, offset = 0; i < workset.iterations; ++i)
    {
        opencl_run(func, 1, TRUE, offset, &workset);
        local_best = FindMax(results, output->buf_size / sizeof *results);
        
        if (best.metric < local_best.metric)
            best = local_best;

        offset += workset.pass_size;
    }
    
    combo_unrank(best.index, item_n, inventory_n, combo);
}


void build_comboCPU(opencl_function *func, llf_criteria *cfg, const item_t *items, size_t item_n, size_t inventory_n, setmax_t *combo)
{
    ulong_t ncombo = NK_MULTISET(item_n, inventory_n);
    ulong_t pass_size;
    ulong_t iterations;
    ulong_t offset = 0;
    result_t result = { 0,0 };

    pass_size = 1000000; // TODO: is this suffeciently large to 

    if (pass_size > ncombo)
        pass_size = (size_t)ncombo;

    iterations = ncombo / pass_size;

	for (ulong_t j = 0; j < iterations; ++j)
	{
		int i;
		result_t local_best;
        setmax_t *scratch;
        result_t r;

		#pragma omp parallel default(shared) private(i,r,local_best,scratch)
		{
			local_best = (result_t){ 0, 0 };
            scratch = alloca(sizeof *scratch * item_n);

			#pragma omp for schedule(static) nowait
			for (i = 0; i < pass_size; ++i)
			{
                r = build_combo_m(items, cfg, offset + i, item_n, inventory_n, scratch);
				if (local_best.metric < r.metric)
					local_best = r;
			}
			
			#pragma omp critical 
			{
				if (result.metric < local_best.metric)
					result = local_best;
			}
		}

		offset += pass_size;
		printf("CPU: progress: %llu/%llu (%f)\n", j+1, iterations, (float)(j+1) / (float)iterations);
	}

    combo_unrank(result.index, item_n, inventory_n, combo);
}
