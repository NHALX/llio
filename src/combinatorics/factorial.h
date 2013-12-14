#ifndef _COMBINATORICS_FACTORIAL_H_
#define _COMBINATORICS_FACTORIAL_H_

#include "types.h"
#define NK_MULTISET_MAX_N 256
#define NK_MULTISET_MAX_K 6
typedef uchar_t setmax_t;

// factorial[n-1,k-1] == (n+k-1)!/(k!(n-1)!). Note that the -1 in the indices is due to C array indexing being zero based.
// NK_MULTISET[n_, k_] := (n + k - 1)!/(k! (n - 1)!)

ulong_t NK_MULTISET(setmax_t n, setmax_t k);

#endif
