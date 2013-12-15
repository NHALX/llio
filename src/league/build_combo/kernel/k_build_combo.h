#ifndef _LEAGUE_BUILD_COMBO_KERNEL_H_
#define _LEAGUE_BUILD_COMBO_KERNEL_H_

#ifndef __OPENCL_VERSION__

#include "opencl_host/function.h"
#include "league/database/db_layout.h"
#include "league/ll_formulas.h"
#include "league/common.h"
#include "combinatorics/factorial.h"

opencl_allocinfo build_combo__allocnfo__(opencl_function *func, size_t items_n);
opencl_kernel_arg *build_combo__bind__(opencl_function *X, uint_t combo_n, const item_t *items, uint_t items_n, llf_criteria *cfg, size_t pass_size, size_t local_size);

result_t build_combo_m(const item_t db_items[], llf_criteria *cfg, ulong_t rank, uint_t db_len, uint_t combo_len, setmax_t *unique);
#endif

#endif
