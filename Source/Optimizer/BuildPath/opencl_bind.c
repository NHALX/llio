#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../Common/OpenCLDebugHack.h"
#include "../../Database/database.h"
#include "../Common/ll_formulas.h"
#include "../../types.h"
#include "lattice.h"
#include "opencl_host.h"
#include "opencl_bind.h"






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

opencl_workset
ConfigureWorkload(
    opencl_context *ctx, 
    cl_kernel kernel, 
    count_t linext_count, 
    cl_ulong alloc_global_input,
    cl_ulong alloc_global_scratch,
    cl_ulong alloc_local)
{
	//size_t hint;
	size_t local_size, pass_size, saturation;
    cl_ulong global_limit;
	cl_ulong iterations;
    opencl_workset work;

    {
        cl_ulong kernel_local;
        cl_ulong wgs_globalmem, wgs_constmem;

        //NOFAIL(clGetKernelWorkGroupInfo(kernel, ctx->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof hint, &hint, 0));
        NOFAIL(clGetKernelWorkGroupInfo(kernel, ctx->device, CL_KERNEL_LOCAL_MEM_SIZE, sizeof kernel_local, &kernel_local, 0));
        alloc_local += kernel_local;
        
        local_size    = (size_t) floor_pow2(ctx->cfg_max_local_storage / alloc_local);
        if (alloc_global_scratch == 0)
            wgs_globalmem = floor_pow2(ctx->cfg_max_global_storage - alloc_global_input);
        else
            wgs_globalmem = floor_pow2((ctx->cfg_max_global_storage - alloc_global_input) / alloc_global_scratch);

        wgs_constmem = wgs_globalmem; // TODO: check const mem
        global_limit = min(wgs_globalmem, wgs_constmem);
    }
    saturation = min(floor_pow2(global_limit / ctx->cfg_compute_units), GPU_SATURATION);
    pass_size = (ctx->cfg_compute_units * saturation * local_size);

    printf("OpenCL: local_alloc: %llu * %d = %llu\n", 
        alloc_local, 
        local_size, 
        local_size * alloc_local
        );

    printf("OpenCL: global_alloc: %llu + %llu * %d = %llu\n", 
        alloc_global_input, 
        alloc_global_scratch, 
        pass_size,
        alloc_global_input + (pass_size * alloc_global_scratch)
        );

    assert(local_size * alloc_local < ctx->cfg_max_local_storage);
    assert(alloc_global_input + (pass_size * alloc_global_scratch) < ctx->cfg_max_global_storage);


	
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

    /*
    //const_alloc += cl_kernel_mem_usage * work->local_size;

    if (work.const_alloc > ctx->cfg_max_const_storage)
    {
        printf("OpenCL: max const storage exceeded (%llu, max = %llu)\n",
            work.const_alloc, ctx->cfg_max_const_storage);

        exit(-1);
    }
    else
    {
        printf("OpenCL: allocating %llu/%llu const storage.\n",
            work.const_alloc, ctx->cfg_max_const_storage);
    }
    */

    return work;
}

/*
	
// CPU version calls this function directly
__kernel void kernel_reduce(
	__global result_t  *metric,
	__local result_t   *scratch,
	__global result_t  *result)

// CPU version calls this function directly
__kernel void kernel_metric(DB,
	llf_criteria       cfg,
	__local  ideal_t   *pasv_scratch,
	__global ideal_t   *linext,
	ideal_t             linext_width,
	__global result_t  *output)

__kernel void kernel_linext(
	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	struct buildpath_info info,
	__global ideal_t *output)
{
*/

opencl_workset
clbp_bind(clbp_context *bp,
	ideal_lattice *g,
	item_t *items,
	size_t  items_len,
	llf_criteria *cfg_input,
	opencl_kernel_arg **output,
	opencl_kernel_arg **bpinfo)
{
#ifdef SEPARATE_KERNELS
	opencl_kernel_arg *linext;
	opencl_kernel_arg *metric;
#endif
	size_t outlen;
	opencl_workset work;
	opencl_kernel_params *a;
	cl_kernel k;
	opencl_context *ctx = &bp->ctx;
    const size_t tmpN_pasv   = items_len * sizeof (ideal_t);
    const size_t tmpN_reduce = sizeof (result_t); 
#ifdef SEPARATE_KERNELS
    const size_t glblN_metric = sizeof (result_t); 
    const size_t glblN_linext = g->linext_width * sizeof(*g->ideals);
#else
    const size_t glblN_linext = 0;
    const size_t glblN_metric = 0;
#endif
    cl_ulong alloc_local = tmpN_reduce + tmpN_pasv;
    cl_ulong alloc_global_scratch = 0, alloc_global_input = 0;
    cl_ulong alloc_db = sizeof *items * items_len;
    cl_ulong alloc_lattice;

    alloc_lattice = g->edge_count * (sizeof *g->ideals + sizeof *g->neighbors);
    alloc_lattice += g->vertex_count * sizeof *g->counts;
    alloc_global_input   = alloc_lattice + alloc_db;
    alloc_global_scratch = glblN_metric + glblN_linext; // scales off pass_size

    // TODO: get this working with SEPRATE_KERNELS
    work = ConfigureWorkload(ctx, ctx->kernel[0], g->linext_count, 
        alloc_global_input, 
        alloc_global_scratch, 
        alloc_local);

    

    k = ctx->kernel[0]; a = &bp->args[0];
	ka_mglobal(ctx, k, ka_push(a), "ideals", A_IN, CL_MEM_READ_ONLY, g->ideals, g->edge_count*sizeof(*g->ideals));
	ka_mglobal(ctx, k, ka_push(a), "counts", A_IN, CL_MEM_READ_ONLY, g->counts, g->vertex_count*sizeof(*g->counts));
    ka_mglobal(ctx, k, ka_push(a), "neighbors", A_IN, CL_MEM_READ_ONLY, g->neighbors, g->edge_count*sizeof(*g->neighbors));
	*bpinfo = ka_ignore(ctx, k, ka_push(a));

	
#ifdef SEPARATE_KERNELS
	linext = ka_mem(ctx, k, ka_push(a), GA_MEM, "linexts", 0, CL_MEM_READ_WRITE, 0, glblN_linext*work.pass_size);

	k = ctx->kernel[1]; a = &bp->args[1];
#endif
    ka_mconst(ctx, k, ka_push(a), "db_items", 0, items, alloc_db);
	ka_value(ctx, k, ka_push(a), "cfg_input", cfg_input, sizeof(*cfg_input));
	ka_mlocal(ctx, k, ka_push(a), "pasv_scratch", tmpN_pasv*work.local_size);
#ifdef SEPARATE_KERNELS
	ka_reuse(ctx, k, ka_push(a), linext);
	ka_value(ctx, k, ka_push(a), "linext_width", &g->linext_width, sizeof g->linext_width);
    metric = ka_mem(ctx, k, ka_push(a), GA_MEM, "results_1", 0, CL_MEM_READ_WRITE, 0, glblN_metric*work.pass_size);

	k = ctx->kernel[2]; a = &bp->args[2];
	ka_reuse(ctx, k, ka_push(a), metric);
#endif
	ka_mlocal(ctx, k, ka_push(a), "scratch", tmpN_reduce*work.local_size);
	outlen = work.pass_size / work.local_size;
	*output = ka_mglobal(ctx, k, ka_push(a), "output", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof(result_t)* outlen);

    return work;
}

cl_ulong
clbp_run(clbp_context *bp, opencl_kernel_arg *bpi, buildpath_info info, opencl_kernel_arg *output, opencl_workset *work)
{
	cl_event ev[4];
	cl_event ev_last;
	cl_ulong sum = 0;
	opencl_context *ctx = &bp->ctx;

	NOFAIL(clSetKernelArg(bpi->kernel, bpi->index, sizeof info, &info));
#ifdef SEPARATE_KERNELS
	NOFAIL(clEnqueueNDRangeKernel(ctx->queue, ctx->kernel[0], 1, NULL, &work->pass_size, &work->local_size, 0, NULL, &ev[0]));
	NOFAIL(clEnqueueNDRangeKernel(ctx->queue, ctx->kernel[1], 1, NULL, &work->pass_size, &work->local_size, 1, &ev[0], &ev[1]));
	NOFAIL(clEnqueueNDRangeKernel(ctx->queue, ctx->kernel[2], 1, NULL, &work->pass_size, &work->local_size, 1, &ev[1], &ev[2]));
	ev_last = ev[2];
#else
	NOFAIL(clEnqueueNDRangeKernel(ctx->queue, ctx->kernel[0], 1, NULL, &work->pass_size, &work->local_size, 0, NULL, &ev[0]));
	ev_last = ev[0];
#endif

	NOFAIL(clEnqueueReadBuffer(ctx->queue, output->cl_mem, CL_TRUE, 0,
		output->buf_size,
		output->buf_data, 1, &ev_last, &ev[3]));

	NOFAIL(clWaitForEvents(1, &ev[3]));
	
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

	clReleaseEvent(ev[3]);
	
	for (size_t i = 0; i < ctx->kernel_n; ++i)
		clReleaseEvent(ev[i]);

	return sum;
}

cl_kernel *
clbp_init(clbp_context *ctx)
{
    #define C_DEFINES "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/"

#ifdef SEPARATE_KERNELS

	char *kernels[CLBP_KERNEL_N] = { "kernel_linext", "kernel_metric", "kernel_reduce" };
    return opencl_init(&ctx->ctx, 1, kernels, CLBP_KERNEL_N,
        "D:/GitRoot/llio/Source/Optimizer/BuildPath/kernel_LMR_3.cl",
        C_DEFINES);

#else

	char *kernels[CLBP_KERNEL_N] = { "kernel_LMR" };

    return opencl_init(&ctx->ctx, 1, kernels, CLBP_KERNEL_N,
        "D:/GitRoot/llio/Source/Optimizer/BuildPath/kernel_LMR.cl",
        C_DEFINES);

#endif

}

void
clbp_free(clbp_context *ctx)
{
	for (size_t i = 0; i < CLBP_KERNEL_N; ++i)
		ka_free(&ctx->args[i]);
	
	opencl_free(&ctx->ctx);
}