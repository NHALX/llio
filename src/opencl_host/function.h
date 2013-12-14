#ifndef _OPENCL_HOST_FUNCTION_H_
#define _OPENCL_HOST_FUNCTION_H_

#include "types.h"
#include "opencl_host/host.h"
#include "opencl_host/kernel_arg.h"

typedef struct
{
    size_t   pass_size;
    size_t   local_size;
    cl_ulong iterations;
    cl_ulong total;

}  opencl_workset;

typedef struct
{
    struct opencl_allocinfo_mem
    {
        cl_ulong local;
        cl_ulong global;
        cl_ulong constant;
    };

    struct opencl_allocinfo_mem fixed;
    struct opencl_allocinfo_mem scale_workgroup;
    struct opencl_allocinfo_mem scale_pass;
    struct opencl_allocinfo_mem scale_reduce;

} opencl_allocinfo;



void opencl_memcheck(opencl_context *ctx, cl_kernel, opencl_kernel_params *args, opencl_workset *work);
cl_ulong opencl_run(opencl_function *function, size_t func_n, bool_t multipass, ulong_t offset, opencl_workset *work);
void opencl_function_free(opencl_function *func, size_t func_n);

opencl_function * opencl_buildfile(opencl_context *ctx, const char *kernel_function, const char *source_file, const char *build_flags);
opencl_function * opencl_buildfilev(opencl_context *ctx, const char *kernel_function[], size_t kernel_n, const char *source_file[], size_t source_n, const char *build_flags);
opencl_function * opencl_build(opencl_context *ctx, const char *kernel_function, const char *source, const char *build_flags);
opencl_function * opencl_buildv(opencl_context *ctx, const char *kernel_function[], size_t kernel_n, const char *source[], size_t source_n, const char *build_flags);


#endif
