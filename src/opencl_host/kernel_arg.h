#ifndef _OPENCL_HOST_KERNEL_ARG_H_
#define _OPENCL_HOST_KERNEL_ARG_H_

#include "types.h"

#define GA_IGNORE 0x0000
#define GA_MEM    0x0001
#define GA_CONST  0x0011
#define GA_TMP    0x0100
#define GA_VAL    0x1000

#define A_IN    0x01
#define A_OUT   0x10
#define A_INOUT 0x11

typedef struct
{
    size_t       index;
    unsigned int type;
    const char  *symbol;
    void        *arg;
    size_t       arg_size;
    cl_kernel    kernel;

    ////////// For ka_mem //////////
    void        *buf_data;
    size_t       buf_size;
    int          io_flags; // rename this to copy_flags or something
    int          dynamic;
    cl_mem       cl_mem;
    //cl_mem_flags cl_flags;
    bool_t       reused;
    ///////// For ka_value 
    unsigned char storage[256]; // spec says CL_DEVICE_MAX_PARAMETER_SIZE minimum is 256 

} opencl_kernel_arg;

#define HCL_ARBITRARY_KPARAM_LIMIT 32
#define OPENCL_KERNEL_PARAMS_INIT {{0},0}

typedef struct
{
    opencl_kernel_arg args[HCL_ARBITRARY_KPARAM_LIMIT];
    size_t count;
} opencl_kernel_params;


typedef struct {
    opencl_context  *ctx;
    cl_kernel kernel;
    opencl_kernel_params args;
} opencl_function;


opencl_kernel_arg *ka_push(opencl_kernel_params *kp);
void ka_free(opencl_kernel_params *kp);

opencl_kernel_arg *ka_ignore(opencl_function *x);
opencl_kernel_arg *ka_mem(opencl_function *x, unsigned int type, const char *sym, int io_flags, cl_mem_flags cl_flags, void *ptr, size_t size);
opencl_kernel_arg *ka_mconst(opencl_function *x, const char *sym, cl_mem_flags cl_flags, const void *ptr, size_t size);
opencl_kernel_arg *ka_mglobal(opencl_function *x, const char *sym, int io_flags, cl_mem_flags cl_flags, void *ptr, size_t size);
opencl_kernel_arg *ka_mlocal(opencl_function *x, const char *sym, size_t size);
opencl_kernel_arg *ka_value(opencl_function *x, const char *sym, void *value, size_t size);
opencl_kernel_arg *ka_reuse(opencl_function *x, opencl_kernel_arg *y);

#define KA_DYN_OUTPUT(X,SYM,SIZE) \
    ka_mglobal(X, SYM, A_OUT, CL_MEM_WRITE_ONLY, 0, SIZE)->buf_data

#endif
