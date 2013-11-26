#ifndef _OCL_HOST_H_
#define _OCL_HOST_H_

#include "x_types.h"

#define PLATFORM_MAX    32

typedef struct 
{
	cl_platform_id platform[PLATFORM_MAX];
	size_t platform_n;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;

	cl_kernel kernel_LE;

	cl_int   cfg_compute_units;
	cl_ulong cfg_max_const_storage;
	cl_uint  cfg_max_const_args;
	cl_bool  cfg_little_endian;
	size_t   cfg_max_workgroup_size;
	int profiling;
} opencl_context;

typedef struct
{
	cl_uint max_neighbors;
	cl_uint   linext_width;
	c_count_t linext_offset;
	c_count_t linext_count;
} buildpath_info;

typedef struct
{
	size_t   pass_size;
	size_t   local_size;
	cl_ulong iterations;
	cl_ulong total;

}  opencl_workset;


#define GA_IGNORE 0x0000
#define GA_MEM    0x0001
#define GA_CONST  0x0011
#define GA_TMP    0x0100
#define GA_VAL    0x1000

#define A_IN    0x01
#define A_OUT   0x10
#define A_INOUT 0x11

// TODO: make this less stupid
typedef struct
{
	size_t       index;
	unsigned int type;
	const char  *symbol;
	void        *arg;
	size_t       arg_size;

	////////// For ka_mem //////////
	void        *buf_data;
	size_t       buf_size;
	int          io_flags;
	int          dynamic;
	cl_mem       cl_mem;
	//cl_mem_flags cl_flags;
	
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

#define KA_DYN_OUTPUT(GPU,ARGS,SYM,SIZE) \
	ka_mglobal(GPU, ARGS, SYM, A_OUT, CL_MEM_WRITE_ONLY, 0, SIZE)->buf_data

opencl_kernel_arg *ka_push(opencl_kernel_params *kp);
void ka_free(opencl_kernel_params *kp);

opencl_kernel_arg *ka_ignore(opencl_kernel_arg *x);
opencl_kernel_arg *ka_mem(opencl_context *ctx, opencl_kernel_arg *x, unsigned int type, const char *sym, int io_flags, cl_mem_flags cl_flags, void *ptr, size_t size);
opencl_kernel_arg *ka_mconst(opencl_context *, opencl_kernel_arg *x, const char *sym, cl_mem_flags cl_flags, const void *ptr, size_t size);
opencl_kernel_arg *ka_mglobal(opencl_context *, opencl_kernel_arg *x, const char *sym, int io_flags, cl_mem_flags cl_flags, void *ptr, size_t size);
opencl_kernel_arg *ka_mlocal(opencl_kernel_arg *x, const char *sym, size_t size);
opencl_kernel_arg *ka_value(opencl_kernel_arg *x, const char *sym, void *value, size_t size);


void opencl_upload(opencl_context *ctx, opencl_kernel_params *args, opencl_workset *work);
cl_ulong opencl_run(opencl_context *ctx, opencl_kernel_params *args, opencl_workset *work);

void opencl_init(opencl_context *, int profiling, char *kernel_function, char *source_file, char *build_flags);
void opencl_free(opencl_context *ctx);

// TODO: real error handling
#define NOFAIL(X) 	\
do { if ((X) != CL_SUCCESS) { fail(X, __LINE__, __FUNCTION__); } } while (0)


extern void fail(int err, int line, const char *function);

#endif