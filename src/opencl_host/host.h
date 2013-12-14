#ifndef _OCL_HOST_H_
#define _OCL_HOST_H_

#include "types.h"
#define PLATFORM_MAX    32

typedef struct 
{
	cl_platform_id platform[PLATFORM_MAX];
	size_t platform_n;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;

	cl_int   cfg_compute_units;
    cl_ulong cfg_max_global_storage;
    cl_ulong cfg_max_local_storage;
	cl_ulong cfg_max_const_storage;
	cl_uint  cfg_max_const_args;
	cl_bool  cfg_little_endian;
	size_t   cfg_max_workgroup_size;
	int profiling;
} opencl_context;


void opencl_init(opencl_context *ctx, int profiling);
void opencl_free(opencl_context *ctx);

// TODO: real error handling
#define NOFAIL(X) 	\
    do { if ((X) != CL_SUCCESS) { fail(X, __LINE__, __FUNCTION__); } } while (0)

#define WARN(X) do { \
	int warning = X; \
	if (warning != CL_SUCCESS) \
		printf("OpenCL.Free: failed. Error=[%d].\n", warning); \
	} while (0)



extern void fail(int err, int line, const char *function);


#endif
