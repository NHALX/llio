#ifndef _VECTOR_H_
#define _VECTOR_H_

#ifndef __OPENCL_VERSION__
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#endif

// Use arrays since __constant c_short8 is bugged on my nvidia card
#define ARRAY_STATS 
// NOTE: that the db input needs to be changed for % based stats if you set this
//#define INTEGER_STATS

#ifndef __OPENCL_VERSION__
typedef cl_ushort c_ushort; typedef cl_short c_short;
typedef cl_uint  c_uint;    typedef cl_float c_float; typedef cl_float8 c_float8;
typedef cl_short c_itemid_t;
#else
typedef ushort c_ushort; typedef short c_short; 
typedef uint   c_uint;   typedef float c_float; typedef float8 c_float8;
typedef short  c_itemid_t;
#endif

#ifdef ARRAY_STATS
#define PART(X,I) ((X)[I])
#else
#ifdef __OPENCL_VERSION__
#define PART(X,I) ((X).s##I)
#else
#define PART(X,I) ((X).s[I])
#endif
#endif

#if defined (ARRAY_STATS) || !defined (__OPENCL_VERSION__)
#define VECTOR_VEC_N 8

#define VECTOR_ADD(A,B) do { for (size_t I = 0; I < VECTOR_VEC_N; ++I) \
	{ PART(A, I) += PART(B, I); }  } while (0)

#define VECTOR_SUB(A,B) do { for (size_t I = 0; I < VECTOR_VEC_N; ++I) \
	{ PART(A, I) -= PART(B, I);	} } while (0)

#define VECTOR_COPY(A,B) do { for (size_t I = 0; I < VECTOR_VEC_N; ++I) \
	{ PART(A, I) = PART(B, I); } } while (0)
#else  
#define VECTOR_ADD(A,B)		A += B
#define VECTOR_SUB(A,B)		A -= B
#define VECTOR_COPY(A,B)    A = B
#endif

#ifdef ARRAY_STATS
#ifdef INTEGER_STATS

typedef c_short stat_t;
#else
typedef c_float stat_t;
#endif
#define VECTOR_SIZEOF      (sizeof(stat_t)*VECTOR_VEC_N)
#define VECTOR(SYM)		   stat_t (SYM)[VECTOR_VEC_N]
#define VECTOR_ZERO_INIT	{ 0,0,0,0,0,0,0,0 }
#else
#ifdef INTEGER_STATS
typedef c_short stat_t;
#define VECTOR_SIZEOF      (sizeof(c_short8))
#define VECTOR(SYM)		   c_short8  SYM
#define VECTOR_ZERO_INIT   (c_short8){ 0,0,0,0,0,0,0,0 }
#else
typedef c_float stat_t;
#define VECTOR_SIZEOF      (sizeof(c_float8))
#define VECTOR(SYM)		   c_float8  SYM
#define VECTOR_ZERO_INIT   (c_float8){ 0,0,0,0,0,0,0,0 }
#endif
#endif
#endif

