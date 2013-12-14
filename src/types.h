#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef __OPENCL_VERSION__
#include "types_kernel.h"
#else
#include "types_host.h"
#endif

typedef uint_t bool_t;
#define TRUE   ((bool_t)1)
#define FALSE  ((bool_t)0)

#endif
