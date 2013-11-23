#ifndef _X_TYPES_H_
#define _X_TYPES_H_

#define INDEX2(N,I,J)			(((I)*(N))+(J))

#ifndef NO_OPENCL
#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#define LINEXT_WIDTH_MAX 64
#define C_IDEAL_T_MAX CL_UCHAR_MAX
#define C_INDEX_T_MAX CL_UINT_MAX
typedef cl_uint  c_index_t;
typedef cl_uchar c_ideal_t;
typedef cl_ulong c_count_t;
typedef cl_short c_itemid_t;
typedef struct { cl_float metric; cl_ulong index; }    c_result_t;
typedef struct 
{
	cl_uchar le_buf[LINEXT_WIDTH_MAX];
	cl_uint  le_len;
	cl_uint  le_index;
} c_linext_t;

#else
#include <limits.h>
#include <stdint.h>

#define C_IDEAL_T_MAX UCHAR_MAX
#define C_INDEX_T_MAX UINT_MAX
typedef uintptr_t     c_index_t;
typedef unsigned char c_ideal_t;
typedef uint64_t      c_count_t;
typedef int16_t       c_itemid_t;

typedef struct
{
	unsigned char le_ptr[LINEXT_WIDTH_MAX];
	unsigned int  le_len;
	unsigned int  le_index;
} c_linext_t;

#endif
#endif
