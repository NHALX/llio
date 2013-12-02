#ifndef _OPENCL_KERNEL_H_
#define _OPENCL_KERNEL_H_

#include "types.h"
#include "poset/lattice_types.h"
#include "poset/kernel/lattice_kernel.h"

#define LINEXT_WIDTH_MAX 64
#define ERROR_INVENTORY -1
#define ERROR_IGNORED   -2
#define ERROR_CORRUPT   -3
#define ERROR_NONE       0

// TODO: enforce alignment on client/host
typedef struct { float_t metric; ulong_t index; } result_t;

#define DB __constant item_t db_items[]



#ifndef __OPENCL_VERSION__
opencl_kernel_arg *metric_ADPS__bind__(opencl_context *x, cl_kernel k, opencl_kernel_params *a, opencl_kernel_arg *linext, item_t *items, size_t items_n, llf_criteria *cfg, opencl_kernel_arg **output, size_t pass_size, size_t local_size);
opencl_allocinfo metric_ADPS__allocnfo__(opencl_context *ctx, cl_kernel k, size_t items_n);
result_t metric_ADPS(__global ideal_t *linext, DB, llf_criteria *cfg, lattice_info *info, count_t nth_extension);
#else
__kernel void metric_ADPS(lattice_info info, __global ideal_t *linext, DB, llf_criteria cfg, __local ideal_t *pasv_scratch, __local result_t* scratch, __global result_t* result);
#endif
#endif
