#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#ifndef _MSC_VER
#include <alloca.h>
#else
#define alloca _alloca
#endif
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "types.h"
#include "opencl_host/host.h"
#include "opencl_host/function.h"

cl_ulong
opencl_run(opencl_function *function, size_t func_n, bool_t multipass, ulong_t offset, opencl_workset *work)
{
    cl_event *ev = alloca(func_n * sizeof *ev);
    //cl_event ev_read;
    cl_event *ev_last;
    cl_ulong sum = 0;
    opencl_context *ctx = function[0].ctx;

    ev_last = 0;
    for (size_t i = 0; i < func_n; ++i)
    {
        assert(function[i].ctx == ctx);
        if (multipass)
            NOFAIL(clSetKernelArg(function[i].kernel, 0, sizeof offset, &offset));

        NOFAIL(clEnqueueNDRangeKernel(ctx->queue, function[i].kernel, 1, NULL,
            &work->pass_size,
            &work->local_size, (ev_last) ? 1 : 0,
            ev_last,
            &ev[i]));

        ev_last = &ev[i];
        // NOFAIL(clFinish(ctx->queue));
    }
    /*
    assert(function[func_n - 1].output != 0);

    NOFAIL(clEnqueueReadBuffer(ctx->queue, function[func_n - 1].output->cl_mem, CL_TRUE, 0,
        function[func_n - 1].output->buf_size,
        function[func_n - 1].output->buf_data, 1, ev_last, &ev_read));

    NOFAIL(clWaitForEvents(1, &ev_read));
    */
    for (size_t i = 0; i < func_n; ++i)
    {
        for (size_t j = 0; j < function[i].args.count; ++j)
        {
            opencl_kernel_arg *arg = &function[i].args.args[j];

            if (arg->type != GA_MEM || !(arg->io_flags & A_OUT))
                continue;

            assert(arg->buf_data != 0);

            NOFAIL(clEnqueueReadBuffer(ctx->queue, arg->cl_mem, CL_TRUE, 0,
                arg->buf_size,
                arg->buf_data, 1, &ev[i], NULL));
        }
    }
    NOFAIL(clFinish(ctx->queue));

    if (ctx->profiling)
    {
        for (size_t i = 0; i < func_n; ++i)
        {
            cl_ulong start = 0, end = 0;
            clGetEventProfilingInfo(ev[i], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
            clGetEventProfilingInfo(ev[i], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
            sum += end - start;
        }
    }

    //clReleaseEvent(ev_read);

    for (size_t i = 0; i < func_n; ++i)
        clReleaseEvent(ev[i]);

    return sum;
}



void
opencl_function_free(opencl_function *function, size_t func_n)
{
    for (size_t i = 0; i < func_n; ++i)
        ka_free(&function[i].args);

    for (size_t i = 0; i < func_n; ++i)
        WARN(clReleaseKernel(function[i].kernel));
    
    free(function);
}


void
PrintBuildLog(opencl_context *ctx)
{
    char *msg;
    size_t len;
    NOFAIL(clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, 0, 0, &len));
    msg = (char*)malloc(len);
    NOFAIL(clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, len, msg, NULL));
    printf("%s\n", msg);
    free(msg);
}


static const char *
load(const char *filename, size_t *size)
{
    char *buf;
    size_t r;
    FILE *f;

    if ((f = fopen(filename, "rb")) == NULL)
        fail(0, __LINE__, __FUNCTION__);

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    rewind(f);

    if ((buf = (char*)malloc(*size + 1)) == NULL)
        fail(0, __LINE__, __FUNCTION__);

    if ((r = fread(buf, 1, *size, f)) != *size)
    {
        printf("load, size: %zu != %zu\n", r, *size);
        fail(0, __LINE__, __FUNCTION__);
    }
    fclose(f);

    buf[*size] = 0;
    return buf;
}


opencl_function * opencl_buildfile(opencl_context *ctx, const char *kernel_function, const char *source_file, const char *build_flags)
{
    const char *fs[1] = { kernel_function };
    const char *ss[1] = { source_file };
    return opencl_buildfilev(ctx, fs, 1, ss, 1, build_flags);
}

opencl_function * opencl_build(opencl_context *ctx, const char *f, const char *source, const char *build_flags)
{
    const char *fs[1] = { f };
    const char *ss[1] = { source };
    return opencl_buildv(ctx, fs, 1, ss, 1, build_flags);
}

opencl_function *
opencl_buildfilev(opencl_context *ctx, const char *kernel_function[], size_t kernel_n, const char *source_file[], size_t source_n, const char *build_flags)
{
    opencl_function *func;
    const char **srcptr = alloca(source_n * sizeof (const char*));
    size_t size;

    for (size_t i = 0; i < source_n; ++i)
        srcptr[i] = load(source_file[i], &size);

    if (!(func = opencl_buildv(ctx, kernel_function, kernel_n, srcptr, source_n, build_flags)))
        fail(0, __LINE__, __FUNCTION__);

    for (size_t i = 0; i < source_n; ++i)
        free((void*)srcptr[i]);

    return func;
}


opencl_function *
opencl_buildv(opencl_context *ctx, const char *kernel_function[], size_t kernel_n, const char *source[], size_t source_n, const char *build_flags)
{
    cl_int error;
    opencl_function *func = calloc(kernel_n, sizeof *func);
   
    if (!func)
    {
        fail(0, __LINE__, __FUNCTION__);
        return 0;
    }
   
    if ((ctx->program = clCreateProgramWithSource(ctx->context, source_n, source, 0, &error)) == NULL)
    {
        fail(0, __LINE__, __FUNCTION__);
        return 0;
    }
    // TODO: see build options for optimization
    if (clBuildProgram(ctx->program, 0, NULL, build_flags, NULL, NULL) != CL_SUCCESS)
    {
        PrintBuildLog(ctx);
        fail(0, __LINE__, __FUNCTION__);
        return 0;
    }

    PrintBuildLog(ctx);

    for (size_t i = 0; i < kernel_n; ++i)
    {
        func[i].ctx = ctx;

        if ((func[i].kernel = clCreateKernel(ctx->program, kernel_function[i], &error)) == NULL)
        {
            fail(0, __LINE__, __FUNCTION__);
            return 0;
        }
    }

    return func;
}



#define ROUND_UP(N,M) ((N + M - 1) / M * M)

static bool_t IsPow2(ulong_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

static size_t FloorPow2(size_t x)
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
opencl_workcfg(opencl_context *ctx, ulong_t total_work, opencl_allocinfo nfo)
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
            ? FloorPow2((max_##F - nfo.fixed.F) / nfo.F2.F) \
            : DEFAULT)

        #define MINIMUM(CRIT,F2,D) \
            min(CRIT(local, F2, D), min(CRIT(constant, F2, D), CRIT(global, F2, D)))

        local_size = MINIMUM(CONSTRAINT, scale_workgroup, ctx->cfg_max_workgroup_size);
        global_limit = MINIMUM(CONSTRAINT, scale_pass, total_work);
    }

    saturation = min(max(1, FloorPow2(global_limit / ctx->cfg_compute_units)), GPU_SATURATION);
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

    if ((ulong_t)pass_size > total_work)
        pass_size = (size_t)ROUND_UP(total_work, 2);

    if (local_size > pass_size)
        local_size = FloorPow2(pass_size);

    total_work = ROUND_UP(total_work, local_size);
    pass_size = ROUND_UP(pass_size, local_size);
    iterations = (cl_ulong)ceil((double)total_work / (double)pass_size);

    printf("OpenCL: n=%llu, pass_size=%u, local_size=%u, iterations=%llu\n", total_work, pass_size, local_size, iterations);

    assert(iterations * (cl_ulong)pass_size >= total_work);
    assert(IsPow2(local_size)); // NOTE: the reduction algorithm in the kernel needs a power of 2 local size

    work.iterations = iterations;
    work.local_size = local_size;
    work.pass_size = pass_size;
    work.total = total_work;

    return work;
}
#undef CONSTRAINT
#undef MINIMUM
#undef ALLOC_SIZE
