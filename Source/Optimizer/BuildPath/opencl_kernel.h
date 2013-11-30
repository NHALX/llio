#ifndef _OPENCL_KERNEL_H_
#define _OPENCL_KERNEL_H_

#include "../../types.h"
#include "lattice_types.h"

#define DB __global item_t db_items[]


#define LINEXT_WIDTH_MAX 64
#define ERROR_INVENTORY -1
#define ERROR_IGNORED   -2
#define ERROR_CORRUPT   -3
#define ERROR_NONE       0


// TODO: enforce alignment on client/host
typedef struct
{
	uchar_t le_buf[LINEXT_WIDTH_MAX];
	uint_t  le_len;
	uint_t  le_index;
} linext_t;


typedef struct { float_t metric; ulong_t index; } result_t;

typedef struct
{
	uint_t max_neighbors;
	uint_t linext_width;
	count_t linext_offset;
	count_t linext_count;
} buildpath_info;




int k_linext_nth(
	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	uint  max_neighbors,
	count_t nth_extension,
	linext_t *le);

result_t k_buildpath(
	DB,
	llf_criteria *cfg,
	__local ideal_t *passv_scratch,

	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	buildpath_info *info,

	size_t global_id,
    size_t local_id);

void k_linext_print(linext_t *le);

#endif
