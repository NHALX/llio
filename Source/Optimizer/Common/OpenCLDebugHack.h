#ifndef _OPENCLDEBUG_HACK_H_
#define _OPENCLDEBUG_HACK_H_

#include <math.h>
#include <stdio.h>

#define DEBUG_PRINTF(...) printf(__VA_ARGS__)

#define __kernel
#define __global
#define __constant
#define __local

#define INFINITY 9999999
#define CLK_LOCAL_MEM_FENCE 0
#define barrier(X)  ;

static int __global_id, __local_id, __global_size, __local_size, __group_id;

static int get_global_id(int x)  { return __global_id; }
static int get_local_id(int x)   { return __local_id; }
static int get_group_id(int x)  { return __group_id; }
static int get_global_size(int x){ return __global_size; }
static int get_local_size(int x) { return __local_size; }

typedef struct { int x; int y; } int2;
typedef int mint;
typedef struct { float x; float y; } float2;
typedef unsigned long long ulong;
/*
static float min(float a, float b)
{
    return (a < b) ? a : b;
}
*/
#define round(X) ((int)X)

#define MAINLOOP(I, FUNCTION, GLOBAL_SIZE, LOCAL_SIZE)                                     \
    for (I = 0,                                                                         \
         __global_size = GLOBAL_SIZE,                                                   \
         __local_size = LOCAL_SIZE,                                                     \
         __group_id = 0,                                                                \
         __global_id = 0;                                                               \
                                                                                        \
         I < GLOBAL_SIZE/LOCAL_SIZE;                                                    \
                                                                                        \
         ++I, ++__group_id)                                                             \
    {                                                                                   \
        for (__local_id  = 0; __local_id < __local_size; ++__local_id, ++__global_id)   \
        { FUNCTION; }                                                                   \
    }                                                                                   \
    
#endif
