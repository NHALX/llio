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




void
fail(int err, int line, const char *function)
{
	printf("Error[%d]: %s, line:%d\n", err, function, line); 
    	exit(1); 
}




static void CL_CALLBACK context_error(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
	printf("OpenCL.Error: %s\n", errinfo);
}

/*
cl_ulong
opencl_run(opencl_context *ctx, cl_kernel kernel, opencl_kernel_params *args, opencl_workset *work)
{
	cl_event event;
	size_t i;

	NOFAIL(clEnqueueNDRangeKernel(ctx->queue, kernel, 1, NULL, &work->pass_size, &work->local_size, 0, NULL, &event));
	NOFAIL(clWaitForEvents(1, &event)); //NOFAIL(clFinish(ctx->queue));

	for (i = 0; i < args->count; ++i)
	{
		opencl_kernel_arg *arg = &args->args[i];

		if (arg->type != GA_MEM || !(arg->io_flags & A_OUT))
			continue;
		
		assert(arg->buf_data != 0);

		NOFAIL(clEnqueueReadBuffer(ctx->queue, arg->cl_mem, CL_TRUE, 0,
			arg->buf_size,
			arg->buf_data, 0, NULL, NULL)); // TODO: waitlist
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
*/

void opencl_init(opencl_context *ctx, int profiling)
{
	size_t i;
	cl_int error;
	cl_uint devices;
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, 0, 0};
	cl_command_queue_properties cqp;

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

	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof ctx->cfg_compute_units, &ctx->cfg_compute_units, NULL));
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof ctx->cfg_max_const_storage, &ctx->cfg_max_const_storage, NULL));
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_ENDIAN_LITTLE, sizeof ctx->cfg_little_endian, &ctx->cfg_little_endian, NULL));
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof ctx->cfg_max_workgroup_size, &ctx->cfg_max_workgroup_size, NULL));
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof ctx->cfg_max_const_args, &ctx->cfg_max_const_args, NULL));
    NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof ctx->cfg_max_local_storage, &ctx->cfg_max_local_storage, NULL));
    NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof ctx->cfg_max_global_storage, &ctx->cfg_max_global_storage, NULL));

	printf("CL_INFO: compute_units=%d, const_size=%llu, const_vars=%u, work_group_size=%d, little_endian=%d\n",
		ctx->cfg_compute_units,
		ctx->cfg_max_const_storage,
		ctx->cfg_max_const_args,
		ctx->cfg_max_workgroup_size,
		ctx->cfg_little_endian
		);

	ctx->profiling = profiling;
}



void opencl_free(opencl_context *ctx)
{	
	WARN(clReleaseProgram(ctx->program));
	WARN(clReleaseCommandQueue(ctx->queue));
	WARN(clReleaseContext(ctx->context));
	return;
}
