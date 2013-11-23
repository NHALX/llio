#include <stdlib.h>
#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif


#include <stdio.h>
#include <string.h>
#include "x_types.h"
#include "lattice.h"
#include "opencl_host.h"

#include "../../Database/database.h"




static void
fail(int err, int line, const char *function)
{
	printf("Error[%d]: %s, line:%d\n", err, function, line); 
	exit(1); 
}

#define NOFAIL(X) 	\
do { if ((X) != CL_SUCCESS) { fail(X, __LINE__, __FUNCTION__); } } while (0)


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
	//printf("OpenCL.Error: %s\n", errinfo);
}



void
opencl_setargs(struct gpu_context *ctx, 
	struct gpu_arg args[KERNEL_ARG_LEN],
	struct ideal_lattice *g, 
	c_itemid_t *idmap, size_t idmap_len, 
	cl_float cfg_input[CFG_SIZE], 
	buildpath_info info,
	size_t global_size,
	size_t local_size)
{
	cl_int error;
	size_t const_alloc;
	size_t i;
	/////
	size_t outlen = global_size / local_size;

	#define CONST_ARGS_START 0
	#define CONST_ARGS_END   4
	#define BPINFO_ARG_INDEX 8
	// NOTE: update KERNEL_ARG_LEN, KERNEL_OUTPUT_0 in opencl_host.h when making changes.

	MEM(args[0], "db_items",     A_IN, CL_MEM_READ_ONLY, db_items, sizeof db_items);
	MEM(args[1], "db_passives",  A_IN, CL_MEM_READ_ONLY, db_passives, sizeof db_passives);
	MEM(args[2], "db_buildtree", A_IN, CL_MEM_READ_ONLY, db_buildtree, sizeof db_buildtree);
	MEM(args[3], "cfg_input",    A_IN, CL_MEM_READ_ONLY, cfg_input, CFG_SIZE*sizeof(*cfg_input));
	MEM(args[4], "id_map",       A_IN, CL_MEM_READ_ONLY, idmap, idmap_len*sizeof(*idmap));
	MEM(args[5], "ideals",       A_IN, CL_MEM_READ_ONLY, g->ideals, g->vertex_count*sizeof(*g->ideals)*g->max_neighbors);
	MEM(args[6], "counts",       A_IN, CL_MEM_READ_ONLY, g->counts, g->vertex_count*sizeof(*g->counts));
	MEM(args[7], "neighbors",    A_IN, CL_MEM_READ_ONLY, g->neighbors, g->vertex_count*sizeof(*g->neighbors)*g->max_neighbors);
	//args[8] set in run
	TMP(args[9], "scratch", local_size * sizeof(c_result_t));
	MEM(args[10], "output", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof(c_result_t) * outlen);

	assert(10 == KERNEL_OUTPUT_0);
	assert(8 == BPINFO_ARG_INDEX);

	for (const_alloc = 0, i = CONST_ARGS_START; i <= CONST_ARGS_END; ++i){
		const_alloc += args[i].buf_size;
		printf("OpenCL: const_size(%s:%d) = %d\n", args[i].symbol, i, args[i].buf_size);
	}

	if (const_alloc > ctx->cfg_max_const_storage)
	{
		printf("OpenCL: max const storage exceeded (%d, max = %d)\n", const_alloc, (size_t)ctx->cfg_max_const_storage);
		exit(-1);
	}
	else
		printf("OpenCL: allocating %d/%d const storage.\n", const_alloc, (size_t)ctx->cfg_max_const_storage);


	for (i = 0; i < KERNEL_ARG_LEN; ++i)
	{
		if (i == BPINFO_ARG_INDEX)
			continue;

		if (args[i].type == GA_MEM)
		{
			args[i].u.cl_mem = clCreateBuffer(ctx->context, args[i].cl_flags, args[i].buf_size, NULL, &error);
			if (args[i].u.cl_mem == 0)
				fail(0, __LINE__, __FUNCTION__);

			if (args[i].io_flags & A_IN)
			{
				assert(args[i].buf_data != 0);
				NOFAIL(clEnqueueWriteBuffer(ctx->queue, args[i].u.cl_mem, CL_FALSE, 0, args[i].buf_size, args[i].buf_data, 0, NULL, NULL));
			}
			else if (args[i].io_flags & A_OUT)
			{
				assert(args[i].buf_data == 0);
				if ((args[i].buf_data = malloc(args[i].buf_size)) == 0)
					fail(0, __LINE__, __FUNCTION__);
			}
		}

		NOFAIL(clSetKernelArg(ctx->kernel_LE, i, args[i].arg_size, args[i].arg));
	}
}


cl_ulong
opencl_run(struct gpu_context *ctx, struct gpu_arg *args, size_t argc, buildpath_info info, size_t global_size, size_t local_size)
{
	cl_event event;
	size_t i;
	VAL(args[BPINFO_ARG_INDEX], "max_neighbors", buildpath_info, info);
	NOFAIL(clSetKernelArg(ctx->kernel_LE, BPINFO_ARG_INDEX, args[BPINFO_ARG_INDEX].arg_size, args[BPINFO_ARG_INDEX].arg));


	NOFAIL(clEnqueueNDRangeKernel(ctx->queue, ctx->kernel_LE, 1, NULL, &global_size, &local_size, 0, NULL, &event));
	//NOFAIL(clFinish(ctx->queue));
	NOFAIL(clWaitForEvents(1, &event));

	for (i = 0; i < argc; ++i)
	{
		if (args[i].type != GA_MEM || !(args[i].io_flags & A_OUT))
			continue;
		assert(args[i].buf_data != 0);
		NOFAIL(clEnqueueReadBuffer(ctx->queue, args[i].u.cl_mem, CL_TRUE, 0, args[i].buf_size, args[i].buf_data, 0, NULL, NULL));
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




void opencl_init(struct gpu_context *ctx, int profiling, char *kernel_function, char *source_file, char *build_flags)
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
		fail(0, __LINE__, __FUNCTION__);

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

	printf("CL_INFO: compute_units=%d, const_size=%lu, work_group_size=%d, little_endian=%d\n",
		(int)ctx->cfg_compute_units,
		(long)ctx->cfg_max_const_storage,
		(int)ctx->cfg_max_workgroup_size,
		(int)ctx->cfg_little_endian
		);

	ctx->profiling = profiling;
	return;
}
