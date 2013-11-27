
#ifdef __OPENCL_VERSION__
//#define STRUCT_ALIGNMENT __attribute__((aligned(16)))
#define STRUCT_ALIGNMENT 
#else
#define STRUCT_ALIGNMENT 
#endif

#define LINEXT_WIDTH_MAX 64
#define ERROR_INVENTORY -1
#define ERROR_IGNORED   -2
#define ERROR_NONE       0

//#ifdef __OPENCL_VERSION__
typedef struct 
{
	uchar le_buf[LINEXT_WIDTH_MAX];
	uint  le_len;
	uint  le_index;
} linext_t;


typedef struct STRUCT_ALIGNMENT { float metric; ulong index; } result_t;

// TODO: merge these definitions into original locations - db_layout.h and lattice.h?
typedef uint   index_t;
typedef uchar  ideal_t;
typedef ulong  count_t;
typedef ushort itemid_t;

#define INVALID_NEIGHBOR UINT_MAX

struct buildpath_info
{
	uint    max_neighbors;
	uint    linext_width;
	count_t linext_offset;
	count_t linext_count;
};


