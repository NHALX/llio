#ifndef _X_TYPES_H_
#define _X_TYPES_H_

#ifndef NO_OPENCL
#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#define C_IDEAL_T_MAX CL_UCHAR_MAX
#define C_INDEX_T_MAX CL_UINT_MAX
typedef cl_uint  c_index_t;
typedef cl_uchar c_ideal_t;
typedef cl_ulong c_count_t;
typedef cl_short c_itemid_t;
typedef struct { cl_float metric; cl_ulong index; }    c_result_t;
#else
#include <limits.h>
#include <stdint.h>

#define C_IDEAL_T_MAX UCHAR_MAX
#define C_INDEX_T_MAX UINT_MAX
typedef uintptr_t     c_index_t;
typedef unsigned char c_ideal_t;
typedef uint64_t      c_count_t;
typedef int16_t       c_itemid_t;
#endif

#endif
