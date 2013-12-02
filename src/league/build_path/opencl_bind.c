#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "opencl_host/dummy.h"
#include "league/database/database.h"
#include "league/ll_formulas.h"
#include "poset/lattice.h"
#include "opencl_host/host.h"
#include "opencl_bind.h"
#include "kernel/metric_ADPS.h"





#define ROUND_UP(N,M) ((N + M - 1) / M * M)

size_t floor_pow2(size_t x) 
{
    size_t y;

    do {
        y = x;
        x = x & (x - 1);
    } while (x != 0);   

    return y;
}

#define GPU_SATURATION 256

opencl_allocinfo
sum_allocinfo(opencl_allocinfo *nfo, size_t nfo_n)
{        
    opencl_allocinfo ai = { 0 };

    for (size_t i = 0; i < nfo_n; ++i)
    {
    #define MERGE(F) \
        ai.F.constant += nfo[i].F.constant; \
        ai.F.global += nfo[i].F.global; \
        ai.F.local += nfo[i].F.local; 

        MERGE(fixed)
        MERGE(scale_pass)
        MERGE(scale_workgroup)
        MERGE(scale_reduce)
    #undef MERGE
    }

    return ai;
}

opencl_workset
ConfigureWorkload(opencl_context *ctx, count_t linext_count, opencl_allocinfo nfo)
{
	size_t local_size, pass_size, saturation;
    cl_ulong global_limit;
	cl_ulong iterations;
    opencl_workset work;
    //NOFAIL(clGetKernelWorkGroupInfo(kernel, ctx->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof hint, &hint, 0));
   
    {
        const cl_ulong max_local = ctx->cfg_max_local_storage;
        const cl_ulong max_global = ctx->cfg_max_global_storage;
        const cl_ulong max_constant = ctx->cfg_max_const_storage;
        
        #define CONSTRAINT(F,F2,DEFAULT) \
            ((nfo.F2.F) \
                ? floor_pow2((max_##F - nfo.fixed.F) / nfo.F2.F) \
                : DEFAULT)

        #define MINIMUM(CRIT,F2,D) \
            min(CRIT(local,F2,D),min(CRIT(constant,F2,D),CRIT(global,F2,D)))

        local_size   = MINIMUM(CONSTRAINT, scale_workgroup, ctx->cfg_max_workgroup_size);
        global_limit = MINIMUM(CONSTRAINT, scale_pass, linext_count);
    }

    saturation = min(floor_pow2(global_limit / ctx->cfg_compute_units), GPU_SATURATION);
    pass_size = (ctx->cfg_compute_units * saturation * local_size);

#define ALLOC_SIZE(F) ( \
        nfo.scale_workgroup.F*local_size + \
        nfo.scale_pass.F*pass_size + \
        nfo.fixed.F \
        )

    printf("OpenCL: %s: %llu\n", "const_alloc", ALLOC_SIZE(constant));
    printf("OpenCL: %s: %llu\n", "global_alloc", ALLOC_SIZE(global));
    printf("OpenCL: %s: %llu\n", "local_alloc", ALLOC_SIZE(local));

    assert(ALLOC_SIZE(constant) < ctx->cfg_max_const_storage);
    assert(ALLOC_SIZE(global) < ctx->cfg_max_global_storage);
    assert(ALLOC_SIZE(local) < ctx->cfg_max_local_storage);
	
	if ((count_t)pass_size > linext_count)
		pass_size = (size_t)ROUND_UP(linext_count, 2);

	if (local_size > pass_size)
		local_size = ROUND_UP(pass_size, 2);

	linext_count = ROUND_UP(linext_count, local_size);
	pass_size = ROUND_UP(pass_size, local_size);
	iterations = (cl_ulong)ceil((double)linext_count / (double)pass_size);

	printf("OpenCL: n=%llu, pass_size=%u, local_size=%u, iterations=%llu\n", linext_count, pass_size, local_size, iterations);

	assert(iterations * (cl_ulong)pass_size >= linext_count);
	assert(local_size % 2 == 0); // NOTE: the reduction algorithm in the kernel needs a power of 2 local size

	work.iterations = iterations;
	work.local_size = local_size;
	work.pass_size = pass_size;
	work.total = linext_count;

    return work;
}
#undef CONSTRAINT
#undef MINIMUM
#undef ALLOC_SIZE


opencl_workset
clbp_bind(clbp_context *bp,	ideal_lattice *l, item_t *items, size_t  items_len,	llf_criteria *cfg_input)
{
	opencl_kernel_arg *linext;
	opencl_context *x = &bp->ctx;
    opencl_allocinfo nfo[2] = { 0 };
    opencl_workset wset[2];
    opencl_workset work;

    nfo[0] = linext__allocnfo__(x, x->kernel[0], l);
    nfo[1] = metric_ADPS__allocnfo__(x, x->kernel[1], items_len);

    wset[0] = ConfigureWorkload(x, l->linext_count, nfo[0]); 
    wset[1] = ConfigureWorkload(x, l->linext_count, nfo[1]);

    // TODO: this doesn't really handle the minimum resource requirement right
    work = (wset[0].local_size < wset[1].local_size)
         ? wset[0]
         : wset[1];

    linext = linext__bind__(x, x->kernel[0], &bp->args[0], l, &bp->lattice_info[0], work.pass_size);
    bp->lattice_info[1] = metric_ADPS__bind__(x, x->kernel[1], &bp->args[1], 
        linext, items, items_len, cfg_input, 
        &bp->output, 
        work.pass_size, 
        work.local_size);

    return work;
}


cl_ulong
clbp_run(clbp_context *bp, lattice_info info, opencl_workset *work)
{
    cl_event ev[CLBP_KERNEL_N];
    cl_event ev_read;
	cl_event *ev_last;
	cl_ulong sum = 0;
	opencl_context *ctx = &bp->ctx;

    NOFAIL(clSetKernelArg(bp->lattice_info[0]->kernel, bp->lattice_info[0]->index, sizeof info, &info));
    NOFAIL(clSetKernelArg(bp->lattice_info[1]->kernel, bp->lattice_info[1]->index, sizeof info, &info));

    ev_last = 0;
    for (size_t i = 0; i < ctx->kernel_n; ++i)
    {
        NOFAIL(clEnqueueNDRangeKernel(ctx->queue, ctx->kernel[i], 1, NULL, 
            &work->pass_size, 
            &work->local_size, (ev_last) ? 1 : 0, 
            ev_last, 
            &ev[i]));

        ev_last = &ev[i];
       // NOFAIL(clFinish(ctx->queue));
    }

    NOFAIL(clEnqueueReadBuffer(ctx->queue, bp->output->cl_mem, CL_TRUE, 0,
        bp->output->buf_size,
        bp->output->buf_data, 1, ev_last, &ev_read));

    NOFAIL(clWaitForEvents(1, &ev_read));
    
	if (ctx->profiling)
	{
		for (size_t i = 0; i < ctx->kernel_n; ++i)
		{
			cl_ulong start = 0, end = 0;
			clGetEventProfilingInfo(ev[i], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
			clGetEventProfilingInfo(ev[i], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
			sum += end - start;
		}
	}

    clReleaseEvent(ev_read);
	
	for (size_t i = 0; i < ctx->kernel_n; ++i)
		clReleaseEvent(ev[i]);

	return sum;
}


void
clbp_init(clbp_context *ctx)
{
    #define C_DEFINES "-ID:/GitRoot/llio/src/"

	char *kernels[CLBP_KERNEL_N] = { "linext", "metric_ADPS" };
    char *files[CLBP_KERNEL_N] = {
        "D:/GitRoot/llio/src/poset/kernel/lattice_kernel.c",
        "D:/GitRoot/llio/src/league/build_path/kernel/metric_ADPS.c",
    };
    opencl_init(&ctx->ctx, 1);
    opencl_buildfilev(&ctx->ctx, kernels, CLBP_KERNEL_N, files, CLBP_KERNEL_N, C_DEFINES);
}

void
clbp_free(clbp_context *ctx)
{
	for (size_t i = 0; i < CLBP_KERNEL_N; ++i)
		ka_free(&ctx->args[i]);
	
	opencl_free(&ctx->ctx);
}