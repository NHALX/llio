
#ifdef __OPENCL_VERSION__
//#define STRUCT_ALIGNMENT __attribute__((aligned(16)))
#define STRUCT_ALIGNMENT 
#else
#define STRUCT_ALIGNMENT 
#endif

struct string_ctx
{
	unsigned char *s_ptr;
	unsigned int   s_len;
	unsigned int   s_index;
};


typedef struct STRUCT_ALIGNMENT { float metric; ulong index; } result_t;
typedef uint    mask_t;
