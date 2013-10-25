#include <stdlib.h>
#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
/* Simple Hello World for OpenCL, written in C.
 * For real code, check for errors. The error code is stored in all calls here,
 * but no checking is done, which is certainly bad. It'll likely simply crash
 * right after a failing call.
 *
 * On GNU/Linux with nVidia OpenCL, program builds with -lOpenCL.
 * Not sure about other platforms.
 */

#include <stdio.h>
#include <string.h>

//#include <CL/cl.h>
#include "Random123/philox.h"
#include "le.h"

void
fail(int line, const char *function)
{
	printf("Error: %s, line:%d\n", function, line); 
	exit(1); 
}

#define NOFAIL(X) 	\
	if ((X) != CL_SUCCESS) { fail(__LINE__, __FUNCTION__); }
	

const char *
load(char *filename, size_t *size)
{
	char *buf;
	size_t r;
	FILE *f;

	if ((f = fopen(filename, "rb")) == NULL)
		fail(__LINE__, __FUNCTION__);

	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	rewind(f);

	if ((buf = (char*) malloc(*size + 1)) == NULL)
		fail(__LINE__, __FUNCTION__);

	if ((r = fread(buf, 1, *size, f)) != *size)
	{
		printf("load, size: %zu != %zu\n", r, *size);
		fail(__LINE__, __FUNCTION__);
	}
	fclose(f);

	buf[*size] = 0;
	return buf;
}

extern void init_all(struct graph *g, philox2x32_key_t *rand_k);
extern uint64_t samples_needed(struct graph *g, double precision);

struct gpu_context 
{
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;

	cl_kernel LE_random;
    cl_kernel LE_tree;
    
    cl_int   cfg_compute_units;
	cl_ulong cfg_max_const_storage;
	cl_bool  cfg_little_endian;
	size_t   cfg_max_workgroup_size;
};

//#include <Windows.h>

void
gpu_LERandom(struct gpu_context *ctx)
{
	//LARGE_INTEGER t1,t2;
	cl_int error;
	cl_mem mem1;

	////
	size_t vsiz;
	size_t nsamples;
	size_t global_size;
	size_t local_size;
	/////
	struct graph g;
	philox2x32_key_t rand_k;
	cl_ulong spt = 0;


	init_all(&g, &rand_k);
	vsiz = g.vertex_count*sizeof(struct vertex);

	nsamples = samples_needed(&g, 0.99);

	//ceil(containerHeight / 250.0) * 250;
	spt = 10;
	global_size = nsamples / spt;
	local_size  = 1;// global_size / compute_units;


	mem1=clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, vsiz, NULL, &error);
	//mem2=clCreateBuffer(ctx->context, CL_MEM_WRITE_ONLY, worksize, NULL, &error);
	
	NOFAIL(clSetKernelArg(ctx->LE_random, 0, sizeof(rand_k), &rand_k))
	NOFAIL(clSetKernelArg(ctx->LE_random, 1, sizeof(mem1), &mem1))
	NOFAIL(clSetKernelArg(ctx->LE_random, 2, sizeof(g.end), &g.end))
	NOFAIL(clSetKernelArg(ctx->LE_random, 3, sizeof(spt), &spt))

	// Send input data to OpenCL (async, don't alter the buffer!)
	NOFAIL(clEnqueueWriteBuffer(ctx->queue, mem1, CL_FALSE, 0, vsiz, g.vertex, 0, NULL, NULL))
	// Perform the operation
	DEBUG_PRINTF("Starting: %zu,%zu\n", global_size, local_size);

//QueryPerformanceCounter(&t1);

	error = clEnqueueNDRangeKernel(ctx->queue, ctx->LE_random, 1, NULL, &global_size, NULL, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		printf("ENQUE_ERROR: %d\n", error);
		fail(__LINE__, __FUNCTION__);
	}
	// Read the result back into buf2
	//NOFAIL(clEnqueueReadBuffer(ctx->queue, mem2, CL_FALSE, 0, worksize, buf2, 0, NULL, NULL))
	// Await completion of all the above
	NOFAIL(clFinish(ctx->queue))
	
//QueryPerformanceCounter(&t2);
//	DEBUG_PRINTF("RANDOM SAMPLING GPU: %d\n", t2.QuadPart-t1.QuadPart);
}




void
gpu_LETree(struct gpu_context *ctx)
{
	//LARGE_INTEGER t1,t2;
	cl_int error;
	cl_mem mem1;
    
	////
	size_t global_size;
	size_t local_size;
    size_t vsiz;
	/////
	struct graph g;

    cl_uint depth = COMBO_LEN;
	init_all(&g, NULL);
    vsiz        = g.vertex_count*sizeof(struct vertex);
    
	global_size = 1;
	local_size  = 1;// global_size / compute_units;
    
    
	mem1=clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, vsiz, NULL, &error);
	//mem2=clCreateBuffer(ctx->context, CL_MEM_WRITE_ONLY, worksize, NULL, &error);
	
	NOFAIL(clSetKernelArg(ctx->LE_tree, 0, sizeof(mem1), &mem1))
	NOFAIL(clSetKernelArg(ctx->LE_tree, 1, sizeof(depth), &depth))

	// Send input data to OpenCL (async, don't alter the buffer!)
	NOFAIL(clEnqueueWriteBuffer(ctx->queue, mem1, CL_FALSE, 0, vsiz, g.vertex, 0, NULL, NULL))
	// Perform the operation
	DEBUG_PRINTF("Starting: %zu,%zu\n", global_size, local_size);
    
    //QueryPerformanceCounter(&t1);
    
	error = clEnqueueNDRangeKernel(ctx->queue, ctx->LE_tree, 1, NULL, &global_size, NULL, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		printf("ENQUE_ERROR: %d\n", error);
		fail(__LINE__, __FUNCTION__);
	}
	// Read the result back into buf2
	//NOFAIL(clEnqueueReadBuffer(ctx->queue, mem2, CL_FALSE, 0, worksize, buf2, 0, NULL, NULL))
	// Await completion of all the above
	NOFAIL(clFinish(ctx->queue))
	
    //QueryPerformanceCounter(&t2);
    //	DEBUG_PRINTF("RANDOM SAMPLING GPU: %d\n", t2.QuadPart-t1.QuadPart);
}


//__kernel void genWalkLE_NR_kernel(__constant struct vertex *vertex, unsigned int max_depth);


void CL_CALLBACK context_error(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
	//printf("OpenCL.Error: %s\n", errinfo);
}

struct gpu_context * gpu_init(char *source_file, char *build_flags) 
{
	size_t srcsize;
	cl_int error;
	cl_uint platforms, devices;
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, 0, 0};
	struct gpu_context *ctx = (struct gpu_context *) calloc(1, sizeof (struct gpu_context));

	const char *src;
	const char *srcptr[1];

	// Fetch the Platform and Device IDs; we only want one.
	NOFAIL(clGetPlatformIDs(1, &ctx->platform, &platforms))
	NOFAIL(clGetDeviceIDs(ctx->platform, CL_DEVICE_TYPE_ALL, 1, &ctx->device, &devices))
	
	properties[1]=(cl_context_properties)ctx->platform;

	// Note that nVidia's OpenCL requires the platform property
	if ((ctx->context = clCreateContext(properties, 1, &ctx->device, &context_error, NULL, &error)) == NULL)
		fail(__LINE__, __FUNCTION__);

	if ((ctx->queue = clCreateCommandQueue(ctx->context, ctx->device, 0, &error)) == NULL)
		fail(__LINE__, __FUNCTION__);

	src       = load(source_file, &srcsize);
	srcptr[0] = src;
	
	// Submit the source code of the rot13 kernel to OpenCL
	if ((ctx->program = clCreateProgramWithSource(ctx->context, 1, srcptr, &srcsize, &error)) == NULL)
		fail(__LINE__, __FUNCTION__);

	// and compile it (after this we could extract the compiled version)
	if (clBuildProgram(ctx->program, 0, NULL, build_flags, NULL, NULL) != CL_SUCCESS)
	{
		char *msg;
		size_t len;
		NOFAIL(clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, 0, 0, &len))
		msg = (char*) alloca(len);
		NOFAIL(clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, len, msg, NULL))
		printf("======== Compile error: ========\n%s\n", msg);
		fail(__LINE__, __FUNCTION__);
	}

	if ((ctx->LE_random = clCreateKernel(ctx->program, "kernel_LE_random", &error)) == NULL)
		fail(__LINE__, __FUNCTION__);

	if ((ctx->LE_tree = clCreateKernel(ctx->program, "genWalkLE_NR_kernel", &error)) == NULL)
		fail(__LINE__, __FUNCTION__);
	
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof ctx->cfg_compute_units, &ctx->cfg_compute_units, NULL))
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof ctx->cfg_max_const_storage, &ctx->cfg_max_const_storage, NULL))
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_ENDIAN_LITTLE, sizeof ctx->cfg_little_endian, &ctx->cfg_little_endian, NULL))
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof ctx->cfg_max_workgroup_size, &ctx->cfg_max_workgroup_size, NULL))
	
	DEBUG_PRINTF("CL_INFO: compute_units=%d, const_size=%lu, work_group_size=%d, little_endian=%d\n",
                 (int)ctx->cfg_compute_units,
                 (long)ctx->cfg_max_const_storage,
                 (int)ctx->cfg_max_workgroup_size,
                 (int)ctx->cfg_little_endian
                 );
    
	return ctx;
}
