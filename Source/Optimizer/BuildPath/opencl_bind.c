#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../Common/OpenCLDebugHack.h"
#include "../../Database/database.h"
#include "../Common/ll_formulas.h"
#include "x_types.h"
#include "lattice.h"
#include "opencl_host.h"
#include "opencl_bind.h"






#define ROUND_UP(N,M) ((N + M - 1) / M * M)

opencl_workset
ConfigureWorkload(opencl_context *ctx, c_count_t linext_count)
{
	size_t hint;
	size_t local_size, pass_size, saturation;
	cl_ulong iterations;
	opencl_workset work;

	NOFAIL(clGetKernelWorkGroupInfo(ctx->kernel_LE, ctx->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof hint, &hint, 0));

	saturation = 128;
	local_size = hint; //ctx->cfg_max_workgroup_size; 
	pass_size = (ctx->cfg_compute_units * saturation * local_size);

	if ((c_count_t)pass_size > linext_count)
		pass_size = (size_t)ROUND_UP(linext_count, 2);

	if (local_size > pass_size)
		local_size = ROUND_UP(pass_size, 2);

	linext_count = ROUND_UP(linext_count, local_size);
	pass_size = ROUND_UP(pass_size, local_size);
	iterations = (cl_ulong)ceil((double)linext_count / (double)pass_size);

	printf("OpenCL: n=%llu, pass_size=%u, local_size=%u, iterations=%llu\n", linext_count, pass_size, local_size, iterations);

	assert(iterations * (cl_ulong)pass_size >= linext_count);
	assert(local_size % 2 == 0); // NOTE: the reduction algorithm in the kernel needs a power of 2 local size

	work.iterations = iterations;
	work.local_size = local_size;
	work.pass_size = pass_size;
	work.total = linext_count;

	return work;
}


opencl_workset
clbp_bindmem(opencl_context *ctx,
opencl_kernel_arg args[KERNEL_ARG_LEN],
struct ideal_lattice *g,
	c_itemid_t *idmap, size_t idmap_len,
	llf_criteria *cfg_input)
{
	size_t outlen;
	opencl_workset work;

#define CONST_ARGS_START 0
#define CONST_ARGS_END   4
#define BPINFO_ARG_INDEX 8
	// NOTE: update KERNEL_ARG_LEN, KERNEL_OUTPUT_0 in opencl_host.h when making changes.

	CONST_MEM(args[0], "db_items", 0, db_items, sizeof db_items);
	CONST_MEM(args[1], "db_passives", 0, db_passives, sizeof db_passives);
	CONST_MEM(args[2], "db_buildtree", 0, db_buildtree, sizeof db_buildtree);
	CONST_MEM(args[3], "cfg_input", 0, cfg_input, sizeof(*cfg_input));
	GLOBAL_MEM(args[4], "id_map", A_IN, CL_MEM_READ_ONLY, idmap, idmap_len*sizeof(*idmap));
	GLOBAL_MEM(args[5], "ideals", A_IN, CL_MEM_READ_ONLY, g->ideals, g->vertex_count*sizeof(*g->ideals)*g->max_neighbors);
	GLOBAL_MEM(args[6], "counts", A_IN, CL_MEM_READ_ONLY, g->counts, g->vertex_count*sizeof(*g->counts));
	GLOBAL_MEM(args[7], "neighbors", A_IN, CL_MEM_READ_ONLY, g->neighbors, g->vertex_count*sizeof(*g->neighbors)*g->max_neighbors);
	IGNORE(args[8]);

	work = ConfigureWorkload(ctx, g->linext_count);
	outlen = work.pass_size / work.local_size;

	LOCAL_MEM(args[9], "scratch", work.local_size * sizeof(c_result_t));
	GLOBAL_MEM(args[10], "output", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof(c_result_t)* outlen);

	assert(10 == KERNEL_OUTPUT_0);
	assert(8 == BPINFO_ARG_INDEX);
	return work;
}

void
clbp_bindval(opencl_context *ctx, opencl_kernel_arg args[KERNEL_ARG_LEN], buildpath_info info)
{
	VAL(args[BPINFO_ARG_INDEX], "max_neighbors", buildpath_info, info);

	NOFAIL(clSetKernelArg(ctx->kernel_LE, BPINFO_ARG_INDEX,
		args[BPINFO_ARG_INDEX].arg_size,
		args[BPINFO_ARG_INDEX].arg));
}

