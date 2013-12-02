#include "opencl_host/host.h"
#include "kernel/metric_ADPS.h"
#include "poset/kernel/lattice_kernel.h"
#include "poset/lattice.h"

#define CLBP_KERNEL_N 2


typedef struct {
	opencl_context    ctx;
	opencl_kernel_params args[CLBP_KERNEL_N];
    opencl_kernel_arg *lattice_info[2];
    opencl_kernel_arg *output;
} clbp_context;


#define CLBP_CONTEXT_INIT 	{ 0 }

opencl_workset clbp_bind(clbp_context *ctx, ideal_lattice *g, item_t *items, size_t items_len,	llf_criteria *cfg_input);
cl_ulong clbp_run(clbp_context *ctx, lattice_info info, opencl_workset *work);
void clbp_init(clbp_context *ctx);
void clbp_free(clbp_context *ctx);
