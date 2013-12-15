#ifndef _OPENCL_KERNEL_H_
#define _OPENCL_KERNEL_H_

#include "types.h"
#include "league/common.h"
#include "league/ll_formulas.h"
#include "poset/lattice_types.h"
#include "poset/kernel/lattice_kernel.h"

#define LINEXT_WIDTH_MAX 64
#define ERROR_INVENTORY -1
#define ERROR_IGNORED   -2
#define ERROR_CORRUPT   -3
#define ERROR_NONE       0


#define DB __constant item_t db_items[]



#ifndef __OPENCL_VERSION__
opencl_kernel_arg *
metric_area__bind__(opencl_function *x, bool_t copy_output,
    lattice_info *info,
    opencl_kernel_arg *linext,
    item_t *items, size_t items_n,
    llf_criteria *cfg,
    size_t pass_size,
    size_t local_size);

opencl_allocinfo metric_area__allocnfo__(opencl_function *, size_t items_n);
result_t metric_area(ideal_t *linext, const item_t *items, llf_criteria *cfg, lattice_info *info, count_t nth_extension);
#else
__kernel void metric_area(ulong_t linext_offset, lattice_info info, __global ideal_t *linext, DB, llf_criteria cfg, __local ideal_t *pasv_scratch, __local result_t* scratch, __global result_t* result);
#endif
#endif
