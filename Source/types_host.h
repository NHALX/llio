#ifndef NO_OPENCL
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#define UINT_T_MAX  CL_UINT_MAX
#define UCHAR_T_MAX CL_UCHAR_MAX
//#define IDEAL_T_MAX CL_UCHAR_MAX
//#define INDEX_T_MAX UINT_T_MAX
//#define INVALID_NEIGHBOR INDEX_T_MAX

typedef cl_uchar  uchar_t;
typedef cl_ushort ushort_t; 
typedef cl_short  short_t;
typedef cl_uint   uint_t;   
typedef cl_int    int_t;
typedef cl_float  float_t; 
typedef cl_ulong  ulong_t;
typedef cl_long   long_t;

#else

#include <limits.h>
#include <stdint.h>

#define UINT_T_MAX  UINT_MAX
#define UCHAR_T_MAX UCHAR_MAX

typedef unsigned char  uchar_t;
typedef unsigned short ushort_t; 
typedef short          short_t;
typedef unsigned int   uint_t;   
typedef int            int_t;
typedef float          float_t;
typedef uint64_t       ulong_t;
typedef int64_t        long_t;

#endif
