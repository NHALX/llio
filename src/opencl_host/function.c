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
