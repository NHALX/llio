#ifndef _LEAGUE_BUILD_COMBO_H_
#define _LEAGUE_BUILD_COMBO_H_
#include "league/common.h"
#include "league/ll_formulas.h"
#include "opencl_host/function.h"
#include "combinatorics/factorial.h"

void build_comboGPU(opencl_function *func, llf_criteria *cfg, const item_t *items, size_t item_n, size_t inventory_n, setmax_t *combo);
void build_comboCPU(opencl_function *func, llf_criteria *cfg, const item_t *items, size_t item_n, size_t inventory_n, setmax_t *combo);

#endif
