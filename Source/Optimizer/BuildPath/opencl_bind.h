#include "opencl_host.h"
#include "opencl_kernel.h"
#include "lattice.h"
#define SEPARATE_KERNELS

#ifdef SEPARATE_KERNELS
#define CLBP_KERNEL_N 3
#else
#define CLBP_KERNEL_N 1
#endif


typedef struct {
	opencl_context    ctx;
	opencl_kernel_params args[CLBP_KERNEL_N];
} clbp_context;


#define CLBP_CONTEXT_INIT 	{ 0 }

opencl_workset
clbp_bind(clbp_context *ctx,
	ideal_lattice *g,
	item_t *items,
	size_t items_len,
	llf_criteria *cfg_input,
	opencl_kernel_arg **output,
	opencl_kernel_arg **bpinfo);

cl_ulong clbp_run(clbp_context *ctx, opencl_kernel_arg *bpi, buildpath_info info, opencl_kernel_arg *output, opencl_workset *work);
cl_kernel *clbp_init(clbp_context *ctx);
void clbp_free(clbp_context *ctx);
