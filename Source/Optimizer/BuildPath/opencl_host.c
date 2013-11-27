#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "x_types.h"
#include "lattice.h"
#include "opencl_host.h"

#include "../../Database/database.h"




void
fail(int err, int line, const char *function)
{
	printf("Error[%d]: %s, line:%d\n", err, function, line); 
	exit(1); 
}



static const char *
load(char *filename, size_t *size)
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

	if ((buf = (char*) malloc(*size + 1)) == NULL)
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


static void CL_CALLBACK context_error(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
	printf("OpenCL.Error: %s\n", errinfo);
}


void
opencl_memcheck(opencl_context *ctx, 	opencl_kernel_params *args,	opencl_workset *work)
{
	cl_ulong const_alloc;
	size_t i;
	cl_ulong cl_kernel_mem_usage = 0;
	/////

	NOFAIL(clGetKernelWorkGroupInfo(ctx->kernel_LE, ctx->device, 
		CL_KERNEL_LOCAL_MEM_SIZE, sizeof cl_kernel_mem_usage, &cl_kernel_mem_usage, 0));

	printf("OpenCL: kernel local mem usage: %llu * %d = %llu\n", 
		cl_kernel_mem_usage, work->local_size, cl_kernel_mem_usage * work->local_size);

	for (const_alloc = 0, i = 0; i < args->count; ++i)
	{
		opencl_kernel_arg *arg = &args->args[i];

		if ((arg->type & GA_CONST) == GA_CONST)
		{
			const_alloc += arg->buf_size;
			printf("OpenCL: const_size(%s:%d) = %d\n",
				arg->symbol, i, arg->buf_size);
		}
	}

	//const_alloc += cl_kernel_mem_usage * work->local_size;

	if (const_alloc > ctx->cfg_max_const_storage)
	{
		printf("OpenCL: max const storage exceeded (%llu, max = %llu)\n", 
			const_alloc,ctx->cfg_max_const_storage);

		exit(-1);
	} else
	{
		printf("OpenCL: allocating %llu/%llu const storage.\n",
			const_alloc, ctx->cfg_max_const_storage);
	}

	return;
}


cl_ulong
opencl_run(opencl_context *ctx, opencl_kernel_params *args, opencl_workset *work)
{
	cl_event event;
	size_t i;

	NOFAIL(clEnqueueNDRangeKernel(ctx->queue, ctx->kernel_LE, 1, NULL, 
		&work->pass_size, 
		&work->local_size, 0, NULL, &event));
	
	NOFAIL(clWaitForEvents(1, &event)); //NOFAIL(clFinish(ctx->queue));

	for (i = 0; i < args->count; ++i)
	{
		opencl_kernel_arg *arg = &args->args[i];

		if (arg->type != GA_MEM || !(arg->io_flags & A_OUT))
			continue;
		
		assert(arg->buf_data != 0);

		NOFAIL(clEnqueueReadBuffer(ctx->queue, arg->cl_mem, CL_TRUE, 0,
			arg->buf_size,
			arg->buf_data, 0, NULL, NULL));
	}

	if (ctx->profiling)
	{
		cl_ulong start = 0, end = 0;
		clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
		clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
		return end - start;
	}
	else
		return 0;
}




void opencl_init(opencl_context *ctx, int profiling, char *kernel_function, char *source_file, char *build_flags)
{
	size_t i;
	size_t srcsize;
	cl_int error;
	cl_uint devices;
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, 0, 0};
	cl_command_queue_properties cqp;

	const char *src;
	const char *srcptr[1];

	// Fetch the Platform and Device IDs; we only want one.
	NOFAIL(clGetPlatformIDs(PLATFORM_MAX, ctx->platform, &ctx->platform_n));
	//NOFAIL(clGetDeviceIDs(ctx->platform, CL_DEVICE_TYPE_ALL, 1, &ctx->device, &devices))

	for (i = 0; i < ctx->platform_n; ++i)
	{
		char buf1[512];
		char buf2[512];
		size_t buf1n, buf2n;
		NOFAIL(clGetPlatformInfo(ctx->platform[i], CL_PLATFORM_NAME, sizeof buf1 - 1, buf1, &buf1n));
		NOFAIL(clGetPlatformInfo(ctx->platform[i], CL_PLATFORM_VENDOR, sizeof buf2 - 1, buf2, &buf2n));
		
		if (clGetDeviceIDs(ctx->platform[i], CL_DEVICE_TYPE_GPU, 1, &ctx->device, &devices) == CL_SUCCESS)
		{
			properties[1] = (cl_context_properties)ctx->platform[i];
			buf1[buf1n]   = 0x0;
			buf2[buf2n]   = 0x0;
			printf("OPENCL: using platform: %s, vendor: %s\n", buf1, buf2);
			break;
		}
	}
	if (properties[1] == 0)
		fail(0, __LINE__, __FUNCTION__);

	// Note that nVidia's OpenCL requires the platform property
	if ((ctx->context = clCreateContext(properties, 1, &ctx->device, &context_error, NULL, &error)) == NULL)
		fail(0, __LINE__, __FUNCTION__);

	cqp = (profiling) ? CL_QUEUE_PROFILING_ENABLE : 0;

	if ((ctx->queue = clCreateCommandQueue(ctx->context, ctx->device, cqp, &error)) == NULL)
		fail(0, __LINE__, __FUNCTION__);

	src = load(source_file, &srcsize);
	srcptr[0] = src;

	if ((ctx->program = clCreateProgramWithSource(ctx->context, 1, srcptr, &srcsize, &error)) == NULL)
	{
		free((void*)src);
		fail(0, __LINE__, __FUNCTION__);
	}

	free((void*)src);

	if (clBuildProgram(ctx->program, 0, NULL, build_flags, NULL, NULL) != CL_SUCCESS)
	{
		char *msg;
		size_t len;
		NOFAIL(clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, 0, 0, &len));
		msg = (char*)malloc(len);
		NOFAIL(clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, len, msg, NULL));
		printf("======== Compile error: ========\n%s\n", msg);
		free(msg);
		fail(0, __LINE__, __FUNCTION__);
	}

	if ((ctx->kernel_LE = clCreateKernel(ctx->program, kernel_function, &error)) == NULL)
		fail(0, __LINE__, __FUNCTION__);


	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof ctx->cfg_compute_units, &ctx->cfg_compute_units, NULL));
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof ctx->cfg_max_const_storage, &ctx->cfg_max_const_storage, NULL));
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_ENDIAN_LITTLE, sizeof ctx->cfg_little_endian, &ctx->cfg_little_endian, NULL));
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof ctx->cfg_max_workgroup_size, &ctx->cfg_max_workgroup_size, NULL));
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof ctx->cfg_max_const_args, &ctx->cfg_max_const_args, NULL));

	printf("CL_INFO: compute_units=%d, const_size=%llu, const_vars=%u, work_group_size=%d, little_endian=%d\n",
		ctx->cfg_compute_units,
		ctx->cfg_max_const_storage,
		ctx->cfg_max_const_args,
		ctx->cfg_max_workgroup_size,
		ctx->cfg_little_endian
		);

	ctx->profiling = profiling;
	return;
}

#define WARN(X) do { \
	int warning = X; \
	if (warning != CL_SUCCESS) \
		printf("OpenCL.Free: failed. Error=[%d].\n", warning); \
	} while (0)

void opencl_free(opencl_context *ctx)
{
	WARN(clReleaseKernel(ctx->kernel_LE));
	WARN(clReleaseProgram(ctx->program));
	WARN(clReleaseCommandQueue(ctx->queue));
	WARN(clReleaseContext(ctx->context));
	return;
}

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
		if (kp->args[i].type == GA_MEM)
		{
			if (kp->args[i].io_flags & A_OUT && kp->args[i].dynamic)
				free(kp->args[i].buf_data);

			WARN(clReleaseMemObject(kp->args[i].cl_mem));
		}
	}

	kp->count = 0;
}

opencl_kernel_arg *
ka_ignore(opencl_context *ctx, opencl_kernel_arg *x)
{
	x->arg      = 0;
	x->arg_size = 0;
	x->type     = GA_IGNORE;
	return x;
}

opencl_kernel_arg *
ka_mem(opencl_context *ctx, opencl_kernel_arg *x, unsigned int type, const char *sym, int io_flags, cl_mem_flags cl_flags, void *ptr, size_t size)
{
	cl_int error;

	x->symbol   = sym;
	x->type     = type;
	x->buf_data = ptr;
	x->buf_size = size;
	x->io_flags = io_flags;
	x->cl_mem   = clCreateBuffer(ctx->context, cl_flags, size, NULL, &error);
	x->dynamic  = 0;

	if (x->cl_mem == 0)
		fail(error, __LINE__, __FUNCTION__);

	memcpy(x->storage, &x->cl_mem, sizeof x->cl_mem);
	x->arg      = &x->storage;
	x->arg_size = sizeof x->cl_mem;

	if (io_flags & A_IN)
	{
		assert(x->buf_data != 0);
		NOFAIL(clEnqueueWriteBuffer(ctx->queue,
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

	NOFAIL(clSetKernelArg(ctx->kernel_LE, x->index, x->arg_size, x->arg));
	return x;
}; 

opencl_kernel_arg *
ka_mconst(opencl_context *ctx, opencl_kernel_arg *x, const char *sym, cl_mem_flags cl_flags, const void *ptr, size_t size)
{
	return ka_mem(ctx, x, GA_CONST, sym, A_IN, cl_flags | CL_MEM_READ_ONLY, (void*) ptr, size);
}

opencl_kernel_arg *
ka_mglobal(opencl_context *ctx, opencl_kernel_arg *x, const char *sym, int io_flags, cl_mem_flags cl_flags, void *ptr, size_t size)
{
	return ka_mem(ctx, x, GA_MEM, sym, io_flags, cl_flags, ptr, size);
}

opencl_kernel_arg *
ka_mlocal(opencl_context *ctx, opencl_kernel_arg *x, const char *sym, size_t size)
{
	x->symbol   = sym;
	x->type     = GA_TMP;
	x->arg_size = size;
	x->arg      = 0;

	NOFAIL(clSetKernelArg(ctx->kernel_LE, x->index, x->arg_size, x->arg));
	return x;
}

opencl_kernel_arg *
ka_value(opencl_context *ctx, opencl_kernel_arg *x, const char *sym, void *value, size_t size)
{
	x->symbol   = sym;
	x->type     = GA_VAL;
	x->arg      = &x->storage;
	x->arg_size = size;

	memcpy(x->storage, value, size);
	NOFAIL(clSetKernelArg(ctx->kernel_LE, x->index, x->arg_size, x->arg));
	return x;
}
