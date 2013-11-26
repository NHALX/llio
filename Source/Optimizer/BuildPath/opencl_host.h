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
	char *symbol;
	unsigned int type;
	void  *buf_data;
	size_t buf_size;
	void  *arg;
	size_t arg_size;
	cl_mem_flags cl_flags;
	int          io_flags;
	
	union {
		cl_mem  cl_mem;
		cl_uint cl_uint;
		buildpath_info buildpath_info;
	} u;

} opencl_kernel_arg;

#define IGNORE(X) \
	(X).type = GA_IGNORE

#define _MEM(TYPE, X, SYM, IOFLAGS, CLFLAGS, PTR, SIZE) do { \
	X = (opencl_kernel_arg){ SYM, TYPE, (void*)(PTR), SIZE, 0, sizeof(cl_mem), CLFLAGS, IOFLAGS, 0 }; \
	X.arg = &X.u; \
} while (0)

#define CONST_MEM(X, SYM, CFLAGS, PTR, SIZE)           _MEM(GA_CONST, X, SYM, A_IN, CFLAGS|CL_MEM_READ_ONLY, PTR, SIZE)
#define GLOBAL_MEM(X, SYM, IOFLAGS, CLFLAGS, PTR, SIZE) _MEM(GA_MEM, X, SYM, IOFLAGS, CLFLAGS, PTR, SIZE)

#define LOCAL_MEM(X, SYM, SIZE) do { \
	memset(&X, 0, sizeof X); \
	X.symbol = SYM; \
	X.type = GA_TMP; \
	X.arg_size = SIZE; \
} while (0)

#define VAL(X, SYM, TYPE,V) do { \
	memset(&X, 0, sizeof X); \
	X.symbol    = SYM; \
	X.type      = GA_VAL; \
	X.u.##TYPE  = V; \
	X.arg       = &X.u; \
	X.arg_size  = sizeof(TYPE); \
	} while (0)



void opencl_upload(opencl_context *ctx, opencl_kernel_arg *args, size_t argc, opencl_workset *work);
cl_ulong opencl_run(opencl_context *ctx, opencl_kernel_arg *args, size_t argc, opencl_workset *work);

void opencl_init(opencl_context *, int profiling, char *kernel_function, char *source_file, char *build_flags);
void opencl_free(opencl_context *ctx);

// TODO: real error handling
#define NOFAIL(X) 	\
do { if ((X) != CL_SUCCESS) { fail(X, __LINE__, __FUNCTION__); } } while (0)


extern void fail(int err, int line, const char *function);

#endif
