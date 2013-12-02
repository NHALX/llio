#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#include "../../Database/database.h"
#include <stdlib.h>
#undef DEBUG_PRINTF
#define DEBUG_PRINTF
#else
#define DEBUG_PRINTF 
#include "../../Database/db_layout.h"
#endif

#include "factorial.h"
#include "../Common/ll_formulas.h"

#define COMBO_WIDTH_MAX 6
#define SET_SIZE 6 // TODO: remove this


//////////////////////////////////////////////////////////////
void reduce(int2 dps, __local int2* scratch, __global int2* result) 
{
    int offset;
    int local_index;

    // Perform parallel reduction
    local_index = get_local_id(0);
    scratch[local_index] = dps;
    barrier(CLK_LOCAL_MEM_FENCE);
  
    for (offset = get_local_size(0) / 2;
        offset > 0;
        offset = offset / 2) 
    {
        if (local_index < offset) {
            int2 other = scratch[local_index + offset];
            int2 mine = scratch[local_index];
            scratch[local_index] = (mine.x > other.x) ? mine : other;
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    if (local_index == 0) {
        result[get_group_id(0)] = scratch[0];
    }
}



int2 calculate_dps( 
    __constant float items[][ITEM_WIDTH], 
	__constant float passives[][PASSIVE_WIDTH], 
	unsigned int base_lexoindex,
    __local unsigned char * combo,
	unsigned int block_size,
    mint combo_input_width, // Widths greater than 6 are passively ignored in calculations, but the full width is still used to traverse the array
	__constant float *cfg)
{
    int processed;
	int combo_width;
	int passive_unique[COMBO_WIDTH_MAX];
	int i,j;
    int2 best_dps = {0, -1};

	   

	processed   = 0;
	combo_width = min(COMBO_WIDTH_MAX, (int)combo_input_width);

	for (; processed < block_size; ++processed, combo += combo_width) 
    {
    	float merged[ITEM_WIDTH];
		float damage;
        int2 dps;
        
		for (j = 0; j < ITEM_WIDTH; ++j) // initialize
            merged[j] = 0;

        for (i = 0; i < combo_width; ++i) // copy passives, sum main stats
        {
            int id = combo[i];
			
			passive_unique[i] = items[id][F_PASSIVE];

            for (j = 0; j < ITEM_WIDTH; ++j)
            {        
                merged[j] += items[id][j]; // TODO: handle stats that are merged multiplicatively
            }
        }
       
		for (i = 0; i < combo_width; ++i) // remove duplicate passives
        {
			int test = passive_unique[i];
			for (j = i+1; j < combo_width; ++j)
				passive_unique[j] = (test == passive_unique[j]) ? PASSIVE_NULL : passive_unique[j];
		}

		for (i = 0; i < combo_width; ++i) // merge passives
        {
			for (j = 0; j < PASSIVE_WIDTH; ++j)
			{
				int id = passive_unique[i];
                merged[j+ITEM_PASSIVE_SYNC_OFFSET] += passives[id][j]; // TODO: handle stats that are merged multiplicatively
			}
		}
		damage = round(FORMULA_TOTAL(cfg,merged));
		
		dps.x = (merged[F_COST] > cfg[C_MAX_COST]) ? -INFINITY : damage;
        dps.y = base_lexoindex + processed;

		DEBUG_PRINTF("index=%d,dps=%d,passives=%d,%d,%d,%d,%d,%d,combo=%d,%d,%d,%d,%d,%d\n", dps.y, dps.x, 
			passive_unique[0], passive_unique[1], passive_unique[2], passive_unique[3], passive_unique[4], passive_unique[5],
			combo[0], 
			combo[1], 
			combo[2], 
			combo[3], 
			combo[4], 
			combo[5]
		);

        best_dps = (best_dps.x > dps.x) ? best_dps : dps;  
    }

	return best_dps;
} 


__kernel void get_combo(__global unsigned char * dst, __global unsigned char * combos, mint combo_width, mint index)
{
    int i;

    for (i = 0; i < combo_width; ++i)
    {
        dst[i] = combos[index+i];
    }
}
 
//////////////////////////////////////////////////////////////

void next_combo(unsigned char *ar, unsigned char n, unsigned int k)
{
	unsigned int i, lowest_i;

	for (i=lowest_i=0; i < k; ++i)
		lowest_i = (ar[i] < ar[lowest_i]) ? i : lowest_i;

	++ar[lowest_i];

	i = (ar[lowest_i] >= n) 
		? 0           // 0 -> all combinations have been exhausted, reset to first combination.
		: lowest_i+1; // _ -> base incremented. digits to the right of it are now zero.
		
	for (; i<k; ++i)
		ar[i] = 0;	
}




int largest(ulong i, unsigned int n, unsigned int k, ulong *offset)
{
    ulong x = 0;

	do    x = NK_MULTISET(n,k); 
	while (x > i && --n >= 0);

	*offset = x;
	return n;
}

void id2combo(ulong id, unsigned int n, unsigned int k, unsigned char *combo)
{
    ulong offset;
	unsigned int i;
	unsigned combo_len = k;

	id %= NK_MULTISET(n,k); // force wrap around if id exceeds highest index

    for (i = 0; i < combo_len && k > 0; ++i)
    {
        combo[i] = largest(id, n, k, &offset);
        id -= offset;
        k  -= 1;
    }    
}




void 
gen_multicombo(unsigned int base_lexoindex, mint n,  __local unsigned char *buf,  unsigned int block_size)
{
	unsigned int i, j;
    unsigned char arr[SET_SIZE];


    id2combo(base_lexoindex, n, SET_SIZE, arr);
   
    for (j = 0; j < SET_SIZE; ++j)
        buf[j] = arr[j];

    DEBUG_PRINTF("StartingCombo: %d,%d,%d,%d,%d\n", arr[0], arr[1], arr[2], arr[3], arr[4]);

    buf += SET_SIZE;
    i    = 1;
    
    while (i++ < block_size) 
    {
		next_combo(arr, n, SET_SIZE);
		DEBUG_PRINTF("%d,%d,%d,%d,%d --> %d\n", arr[0], arr[1], arr[2], arr[3], arr[4], buf);
        for (j = 0; j < SET_SIZE; ++j)
            buf[j] = arr[j];
            
        buf+=SET_SIZE;
    }
}



__kernel void
maxdps(__constant float *cfg,
   __constant float items[][ITEM_WIDTH], 
   __constant float passives[][PASSIVE_WIDTH], 
   mint start_offset, 
   mint n, 
   __local unsigned char *combo_storage,  
   mint combo_local_count,
   __local int2* scratch,
   __global int2* result)
{
	const unsigned int block_size     = combo_local_count / get_local_size(0);
    const unsigned int id             = (get_group_id(0)*combo_local_count) + (get_local_id(0) * block_size);
	const unsigned int offset         = get_local_id(0)*block_size;
    __local unsigned char *buf        = combo_storage + (offset*SET_SIZE);
	const unsigned int base_lexoindex = start_offset + id;

	int2 best_dps;

    DEBUG_PRINTF("Group=%d,Gen:id=%d,bs=%d,offset=%d,ptr=%d\n", get_group_id(0), id, block_size, offset, combo_storage);

	gen_multicombo(base_lexoindex, n, buf, block_size);
	best_dps = calculate_dps(items, passives, base_lexoindex, buf, block_size, SET_SIZE, cfg);
	DEBUG_PRINTF("best_dps=%d,best_index=%d\n",best_dps.x, best_dps.y);

    reduce(best_dps, scratch, result);
}    

//////////////////////////////////////////////////////////////
// For debugging as regular C code:
#ifndef __OPENCL_VERSION__

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define VALUE_MAX 60
static_assert(VALUE_MAX < 255, "VALUE_MAX is limited to byte sized precision");

void
test()
{
    printf("global_id:%d,local_id:%d,group_id:%d,global_size:%d,local_size:%d\n", get_global_id(0), get_local_id(0), get_group_id(0), get_global_size(0), get_local_size(0));
}



int
main()
{
	int2 best = {0,0};
    unsigned int i;
	///////////////
	#define ROUNDTONEAREST(X, N) (ceil((float)(X) / (float)N) * N)

	#define LOCAL_SIZE 2
	#define LOCAL_STORAGE_COUNT (LOCAL_SIZE*5)

    const ulong total = NK_MULTISET(VALUE_MAX, SET_SIZE);
	unsigned int global_size = ROUNDTONEAREST(((float)total/(float)LOCAL_STORAGE_COUNT)*(float)LOCAL_SIZE, LOCAL_SIZE);
	const unsigned int ngroups = global_size/LOCAL_SIZE;
	unsigned char *combo_storage = (unsigned char *) calloc(ngroups*LOCAL_STORAGE_COUNT,SET_SIZE);

    ////////////////
    
    int2 scratch[LOCAL_SIZE];
    int2 *result= (int2*) calloc(ngroups, sizeof(int2));
	float cfg[] = { 6, 3.4, 100, 18, 13000 };

    ////////////////
        
    // this produces wrong results because the barrier syncs don't happen    
	MAINLOOP(i, maxdps(
		cfg,
		db_items, 
		db_passives, 
		0, 
		VALUE_MAX, 
		combo_storage + (get_group_id(0)*SET_SIZE*LOCAL_STORAGE_COUNT), 
		LOCAL_STORAGE_COUNT,
		scratch,
		result
	), 
	global_size, LOCAL_SIZE)

    //MAINLOOP(i,maxreduce_dps(db_items, db_passives, (unsigned char *)combo,NINPUTS,COMBO_WIDTH,scratch,result,6, 3.4, 100, 18, 13000), global_size, LOCAL_SIZE)
    //MAINLOOP(i,test(),GLOBAL_SIZE,LOCAL_SIZE)
    /////////////
    

	/*
	{
		unsigned char *ptr = combo_storage;
		unsigned char *end = ptr+(SET_SIZE*total);
		unsigned int k = 0;
		while (ptr < end)
		{
			printf("i=%02d, {%d,%d,%d,%d,%d}\n", k++, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
			ptr += SET_SIZE;
		}
	}
	*/

	for (i = 0; i < ngroups; ++i)
    {
		best = (best.x < result[i].x) ? result[i] : best;

		//unsigned char dst[SET_SIZE];
		//int index = (int)result[i].y;
		//unsigned char* p = (unsigned char *)combo_storage;

		//get_combo(dst, (unsigned char *)combo_storage, SET_SIZE, index);
        
    }

	printf("result=%d,%d,\n", best.x, best.y);

	printf("global_size_was:%d\n", global_size);
    return 0;
}
#endif

