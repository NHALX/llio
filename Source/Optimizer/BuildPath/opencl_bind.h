#include "opencl_host.h"

#define KERNEL_ARG_LEN  11
#define KERNEL_OUTPUT_0 10

opencl_workset clbp_bindmem(opencl_context *ctx, opencl_kernel_arg args[KERNEL_ARG_LEN], struct ideal_lattice *g, c_itemid_t *idmap, size_t idmap_len, llf_criteria *cfg_input);
void clbp_bindval(opencl_context *ctx, opencl_kernel_arg args[KERNEL_ARG_LEN], buildpath_info info);
