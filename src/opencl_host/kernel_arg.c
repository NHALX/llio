#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "types.h"
#include "opencl_host/host.h"
#include "opencl_host/kernel_arg.h"

opencl_kernel_arg *
ka_push(opencl_kernel_params *kp)
{
    size_t index = kp->count++;
    opencl_kernel_arg *x = &kp->args[index];
    assert(kp->count <= HCL_ARBITRARY_KPARAM_LIMIT);
    memset(x, 0, sizeof *x);
    x->index = index;
    return x;
}

void
ka_free(opencl_kernel_params *kp)
{
    for (size_t i = 0; i < kp->count; ++i)
    {
        if (kp->args[i].reused)
            continue;

        if (kp->args[i].type == GA_MEM)
        {
            if (kp->args[i].io_flags & A_OUT && kp->args[i].dynamic && kp->args[i].buf_data)
            {
                free(kp->args[i].buf_data);
                kp->args[i].buf_data = 0;
            }
            WARN(clReleaseMemObject(kp->args[i].cl_mem));
        }
    }

    kp->count = 0;
}

opencl_kernel_arg *
ka_ignore(opencl_function *func)
{
    opencl_kernel_arg *x = ka_push(&func->args);
    x->kernel = func->kernel;
    x->arg = 0;
    x->arg_size = 0;
    x->type = GA_IGNORE;
    return x;
}

opencl_kernel_arg *
ka_mem(opencl_function *func, unsigned int type, const char *sym, int io_flags, cl_mem_flags cl_flags, void *ptr, size_t size)
{
    cl_int error;
    opencl_kernel_arg *x = ka_push(&func->args);

    x->symbol = sym;
    x->type = type;
    x->buf_data = ptr;
    x->buf_size = size;
    x->io_flags = io_flags;
    x->cl_mem = clCreateBuffer(func->ctx->context, cl_flags, size, NULL, &error);
    x->dynamic = 0;
    x->kernel = func->kernel;

    if (x->cl_mem == 0)
        fail(error, __LINE__, __FUNCTION__);

    memcpy(x->storage, &x->cl_mem, sizeof x->cl_mem);
    x->arg = &x->storage;
    x->arg_size = sizeof x->cl_mem;

    if (io_flags & A_IN)
    {
        assert(x->buf_data != 0);
        NOFAIL(clEnqueueWriteBuffer(func->ctx->queue,
            x->cl_mem, CL_FALSE, 0,
            x->buf_size,
            x->buf_data, 0, NULL, NULL));
    }
    else if (io_flags & A_OUT)
    {
        if (x->buf_data == 0)
        {
            if ((x->buf_data = calloc(1, x->buf_size)) == 0)
                fail(0, __LINE__, __FUNCTION__);

            x->dynamic = 1;
        }
    }

    NOFAIL(clSetKernelArg(func->kernel, x->index, x->arg_size, x->arg));
    return x;
};


opencl_kernel_arg *
ka_mconst(opencl_function *func, const char *sym, cl_mem_flags cl_flags, const void *ptr, size_t size)
{
    return ka_mem(func, GA_CONST, sym, A_IN, cl_flags | CL_MEM_READ_ONLY, (void*)ptr, size);
}

opencl_kernel_arg *
ka_mglobal(opencl_function *func, const char *sym, int io_flags, cl_mem_flags cl_flags, void *ptr, size_t size)
{
    return ka_mem(func, GA_MEM, sym, io_flags, cl_flags, ptr, size);
}

opencl_kernel_arg *
ka_mlocal(opencl_function *func, const char *sym, size_t size)
{
    opencl_kernel_arg *x = ka_push(&func->args);

    x->symbol = sym;
    x->type = GA_TMP;
    x->arg_size = size;
    x->arg = 0;
    x->kernel = func->kernel;

    NOFAIL(clSetKernelArg(func->kernel, x->index, x->arg_size, x->arg));
    return x;
}

opencl_kernel_arg *
ka_reuse(opencl_function *func, opencl_kernel_arg *y)
{
    opencl_kernel_arg *x = ka_push(&func->args);

    x->symbol = y->symbol;
    x->type = y->type;
    x->arg_size = y->arg_size;
    x->arg = y->arg;
    x->kernel = func->kernel;
    x->cl_mem = y->cl_mem;
    x->reused = TRUE;

    NOFAIL(clSetKernelArg(func->kernel, x->index, x->arg_size, x->arg));
    return x;
}


opencl_kernel_arg *
ka_value(opencl_function *func, const char *sym, void *value, size_t size)
{
    opencl_kernel_arg *x = ka_push(&func->args);

    x->symbol = sym;
    x->type = GA_VAL;
    x->arg = &x->storage;
    x->arg_size = size;
    x->kernel = func->kernel;

    memcpy(x->storage, value, size);
    NOFAIL(clSetKernelArg(func->kernel, x->index, x->arg_size, x->arg));
    return x;
}
