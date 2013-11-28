#include "opencl_host.h"

opencl_workset clbp_bindmem(
	opencl_context *ctx,
	opencl_kernel_params *args,
	struct ideal_lattice *g,
	item_t *items,
	c_itemid_t *idmap, size_t idmap_len,
	llf_criteria *cfg_input,
	opencl_kernel_arg **output,
	opencl_kernel_arg **bpinfo);

void clbp_bindval(opencl_context *ctx, opencl_kernel_arg *arg, buildpath_info info);