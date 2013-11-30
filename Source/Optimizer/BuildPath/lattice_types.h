#ifndef _LATTICE_NODE_H_
#define _LATTICE_NODE_H_

#include "../../types.h"
#define INDEX2(N,I,J)			(((I)*(N))+(J))

#define IDEAL_T_MAX UCHAR_T_MAX
#define INVALID_NEIGHBOR INDEX_T_MAX

#define INDEX_T_MAX UINT_T_MAX

typedef uchar_t  ideal_t;
typedef uint_t   index_t;
typedef ulong_t  count_t;

#endif
