#ifndef _LATTICE_KERNEL_H_
#define _LATTICE_KERNEL_H_

#include "types.h"
#include "poset/lattice_types.h"

typedef struct
{
    uint_t max_neighbors;
    uint_t linext_width;
    count_t linext_count;
} lattice_info;

#define LATTICE  __global ideal_t *ideals, __global count_t *counts, __global index_t *adjacency

#ifndef __OPENCL_VERSION__
#include "poset/lattice.h"
#include "opencl_host/function.h"
void linext_print(ideal_t *le_buf, size_t le_len);
void linext_nth(ideal_lattice *il, ideal_t *le_buf, count_t offset, size_t index);
opencl_kernel_arg *linext__bind__(opencl_function *func, bool_t copy_output, ideal_lattice *g, size_t pass_size);
opencl_allocinfo linext__allocnfo__(opencl_function *func, ideal_lattice *g);
#endif

#endif
