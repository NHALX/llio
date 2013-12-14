#include "opencl_host/host.h"
#include "kernel/metric_ADPS.h"
#include "poset/kernel/lattice_kernel.h"
#include "poset/lattice.h"

#define CLBP_KERNEL_N 2

opencl_function *clbp_init(opencl_context *ctx);
opencl_workset clbp_bind(opencl_function *f, ideal_lattice *l, item_t *items, size_t  items_len, llf_criteria *cfg_input, opencl_kernel_arg **output_ptr);