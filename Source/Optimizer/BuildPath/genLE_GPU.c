#include <stdlib.h>
#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif


#include <stdio.h>
#include <string.h>
#include "le.h"

struct gpu_context
{
	cl_platform_id platform;
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

};


static void
fail(int err, int line, const char *function)
{
	printf("Error[%d]: %s, line:%d\n", err, function, line); 
	exit(1); 
}

#define NOFAIL(X) 	\
	if ((X) != CL_SUCCESS) { fail(X, __LINE__, __FUNCTION__); }
	

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

#include "../../Database/database.h"
#include "../Common/ll_formulas.h"



c_result_t *
gpu_LE(struct gpu_context *ctx, struct graph *g, cl_float cfg_input[CFG_SIZE], size_t *result_n)
{
	cl_int error;
	cl_mem imem,cmem,amem,db,passives,cfg,idmap,outmem;
	c_result_t *results;
	cl_uint combo_len, neighbors;
	////
	size_t global_size;
	size_t local_size;
    size_t isiz,csiz,asiz,idsiz,cfgsiz;
	size_t outsiz;
	size_t const_alloc;
	/////
	combo_len   = g->combo_len;
	neighbors   = g->max_neighbors;
	csiz        = g->vertex_count*sizeof(*g->counts);
	isiz        = g->vertex_count*sizeof(*g->ideals)*g->max_neighbors;
	asiz        = g->vertex_count*sizeof(*g->adjacency)*g->max_neighbors;
	idsiz       = g->idmap_len*sizeof(*g->idmap);
	cfgsiz      = CFG_SIZE*sizeof(*cfg_input);
	
	// TODO: the size_t precision of nth_extension is a problem.
	// the system wont be able to queue the full workload anyway
	// so an offset should be added later to split the work up 
	global_size = g->counts[1];
	local_size  = 1;// global_size / compute_units;

	*result_n = global_size / local_size;
	outsiz    = sizeof(*results) * (*result_n);
	results   = malloc(outsiz);

	const_alloc = isiz + csiz + asiz + idsiz + cfgsiz + sizeof(db_items)+sizeof(db_passives);
	
	if (const_alloc > ctx->cfg_max_const_storage)
	{
		printf("OpenCL: max const storage exceeded (%d, max = %d)\n", const_alloc, (size_t) ctx->cfg_max_const_storage);
		exit(-1);
	}
	else
		printf("OpenCL: allocating %d/%d const storage.\n", const_alloc, (size_t) ctx->cfg_max_const_storage);

	// TODO: most of this shouldnt be done every call
	db       = clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, sizeof(db_items), NULL, &error);
	passives = clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, sizeof(db_passives), NULL, &error);
	cfg      = clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, cfgsiz, NULL, &error);
	idmap    = clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, idsiz, NULL, &error);
	imem     = clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, isiz, NULL, &error);
	cmem     = clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, csiz, NULL, &error);
	amem     = clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, asiz, NULL, &error);
	outmem   = clCreateBuffer(ctx->context, CL_MEM_WRITE_ONLY, outsiz, NULL, &error);


	NOFAIL(clSetKernelArg(ctx->kernel_LE, 0, sizeof(db), &db))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 1, sizeof(passives), &passives))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 2, sizeof(cfg), &cfg))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 3, sizeof(idmap), &idmap))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 4, sizeof(imem), &imem))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 5, sizeof(cmem), &cmem))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 6, sizeof(amem), &amem))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 7, sizeof(neighbors), &neighbors))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 8, sizeof(combo_len), &combo_len))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 9, local_size * sizeof(c_result_t), NULL))
	NOFAIL(clSetKernelArg(ctx->kernel_LE, 10, sizeof(outmem), &outmem))

	NOFAIL(clEnqueueWriteBuffer(ctx->queue, db,       CL_FALSE, 0, sizeof(db_items),          db_items,     0, NULL, NULL))
	NOFAIL(clEnqueueWriteBuffer(ctx->queue, passives, CL_FALSE, 0, sizeof(db_passives),       db_passives,  0, NULL, NULL))
	NOFAIL(clEnqueueWriteBuffer(ctx->queue, cfg,      CL_FALSE, 0, cfgsiz,                    cfg_input,    0, NULL, NULL))
	NOFAIL(clEnqueueWriteBuffer(ctx->queue, idmap,    CL_FALSE, 0, idsiz,                     g->idmap,     0, NULL, NULL))
	NOFAIL(clEnqueueWriteBuffer(ctx->queue, imem,     CL_FALSE, 0, isiz,                      g->ideals,    0, NULL, NULL))
	NOFAIL(clEnqueueWriteBuffer(ctx->queue, cmem,     CL_FALSE, 0, csiz,                      g->counts,    0, NULL, NULL))
	NOFAIL(clEnqueueWriteBuffer(ctx->queue, amem,     CL_FALSE, 0, asiz,                      g->adjacency, 0, NULL, NULL))

	printf("Starting: %u,%u\n", global_size, local_size);
	printf("Args: db_size=%d, passive_size=%d, cfg_size=%d, id_size=%d, ideals_size=%d, count_size=%d, adjacency_size=%d, max_neighbor=%d, combo_len=%d\n",
		sizeof(db_items),
		sizeof(db_passives),
		cfgsiz, 
		idsiz, 
		isiz, 
		csiz,
		asiz, 
		neighbors,
		combo_len
		);

	error = clEnqueueNDRangeKernel(ctx->queue, ctx->kernel_LE, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		printf("ENQUE_ERROR: %d\n", error);
		fail(0, __LINE__, __FUNCTION__);
	}

	NOFAIL(clFinish(ctx->queue))
	NOFAIL(clEnqueueReadBuffer(ctx->queue, outmem, CL_TRUE, 0, outsiz, results, 0, NULL, NULL))

	return results;
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
	//NOFAIL(clGetDeviceIDs(ctx->platform, CL_DEVICE_TYPE_CPU, 1, &ctx->device, &devices))
	
	properties[1]=(cl_context_properties)ctx->platform;

	// Note that nVidia's OpenCL requires the platform property
	if ((ctx->context = clCreateContext(properties, 1, &ctx->device, &context_error, NULL, &error)) == NULL)
		fail(0, __LINE__, __FUNCTION__);

	if ((ctx->queue = clCreateCommandQueue(ctx->context, ctx->device, 0, &error)) == NULL)
		fail(0, __LINE__, __FUNCTION__);

	src       = load(source_file, &srcsize);
	srcptr[0] = src;
	
	if ((ctx->program = clCreateProgramWithSource(ctx->context, 1, srcptr, &srcsize, &error)) == NULL)
		fail(0, __LINE__, __FUNCTION__);

	if (clBuildProgram(ctx->program, 0, NULL, build_flags, NULL, NULL) != CL_SUCCESS)
	{
		char *msg;
		size_t len;
		NOFAIL(clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, 0, 0, &len))
		msg = (char*) malloc(len);
		NOFAIL(clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, len, msg, NULL))
		printf("======== Compile error: ========\n%s\n", msg);
		free(msg);
		fail(0, __LINE__, __FUNCTION__);
	}

	if ((ctx->kernel_LE = clCreateKernel(ctx->program, "kernel_LE", &error)) == NULL)
		fail(0, __LINE__, __FUNCTION__);


	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof ctx->cfg_compute_units, &ctx->cfg_compute_units, NULL))
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof ctx->cfg_max_const_storage, &ctx->cfg_max_const_storage, NULL))
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_ENDIAN_LITTLE, sizeof ctx->cfg_little_endian, &ctx->cfg_little_endian, NULL))
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof ctx->cfg_max_workgroup_size, &ctx->cfg_max_workgroup_size, NULL))
	NOFAIL(clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof ctx->cfg_max_const_args, &ctx->cfg_max_const_args, NULL))

	

	printf("CL_INFO: compute_units=%d, const_size=%lu, work_group_size=%d, little_endian=%d\n",
                 (int)ctx->cfg_compute_units,
                 (long)ctx->cfg_max_const_storage,
                 (int)ctx->cfg_max_workgroup_size,
                 (int)ctx->cfg_little_endian
                 );
    
	return ctx;
}
