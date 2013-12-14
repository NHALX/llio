#ifndef _LEAGUE_BUILD_COMBO_H_
#define _LEAGUE_BUILD_COMBO_H_

#ifndef __OPENCL_VERSION__

#include "opencl_host/function.h"
#include "league/database/db_layout.h"
#include "league/ll_formulas.h"
#include "league/common.h"

opencl_allocinfo build_combo__allocnfo__(opencl_function *func, size_t items_n);
opencl_kernel_arg *build_combo__bind__(opencl_function *X, uint_t combo_n, const item_t *items, uint_t items_n, llf_criteria *cfg, size_t pass_size, size_t local_size);
#endif

#endif
