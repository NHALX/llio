#ifndef _COMBINATORICS_COMBINATION_H_
#define _COMBINATORICS_COMBINATION_H_
#include "types.h"
#include "factorial.h"

//#define COMBO_WIDTH_MAX   NK_MULTISET_MAX_K
//#define SET_SIZE 6 // TODO: remove this


void combo_unrank(ulong_t rank, uint_t n, uint_t k, setmax_t *combo);

#endif
