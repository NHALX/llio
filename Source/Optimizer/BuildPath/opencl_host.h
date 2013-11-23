#include "x_types.h"
#include "../Common/ll_formulas.h"

#define PLATFORM_MAX    32
#define KERNEL_ARG_LEN  11
#define KERNEL_OUTPUT_0 10

struct gpu_context
{
	cl_platform_id platform[PLATFORM_MAX];
	size_t platform_n;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;

	cl_kernel kernel_LE;

	cl_int   cfg_compute_units;
	cl_ulong cfg_max_const_storage;
	cl_uint  cfg_max_const_args;
	cl_bool  cfg_little_endian;
	size_t   cfg_max_workgroup_size;
	int profiling;
};

typedef struct
{
	cl_uint max_neighbors;
	cl_uint   linext_width;
	c_count_t linext_offset;
	c_count_t linext_count;
} buildpath_info;



enum gpu_arg_t { GA_MEM, GA_VAL, GA_TMP };
#define A_IN    0x01
#define A_OUT   0x10
#define A_INOUT 0x11

// TODO: the size_t precision of nth_extension is a problem.
// the system wont be able to queue the full workload anyway
// so an offset should be added later to split the work up 

struct gpu_arg
{
	char *symbol;
	enum gpu_arg_t type;
	void  *buf_data;
	size_t buf_size;
	void  *arg;
	size_t arg_size;
	cl_mem_flags cl_flags;
	int          io_flags;
	
	union {
		cl_mem  cl_mem;
		cl_uint cl_uint;
		buildpath_info buildpath_info;
	} u;
};

#define MEM(X, SYM, IOFLAGS, CLFLAGS, PTR, SIZE) do { \
	X = (struct gpu_arg){ SYM, GA_MEM, PTR, SIZE, 0, sizeof(cl_mem), CLFLAGS, IOFLAGS, 0 }; \
	X.arg = &X.u; \
} while (0)

#define TMP(X, SYM, SIZE) do { \
	memset(&X, 0, sizeof X); \
	X.symbol = SYM; \
	X.type = GA_TMP; \
	X.arg_size = SIZE; \
} while (0)

#define VAL(X, SYM, TYPE,V) do { \
	memset(&X, 0, sizeof X); \
	X.symbol    = SYM; \
	X.type      = GA_VAL; \
	X.u.##TYPE  = V; \
	X.arg       = &X.u; \
	X.arg_size  = sizeof(TYPE); \
	} while (0)


void opencl_setargs(struct gpu_context *ctx, struct gpu_arg args[KERNEL_ARG_LEN], struct ideal_lattice *g, c_itemid_t *idmap, size_t idmap_len, cl_float cfg_input[CFG_SIZE], buildpath_info info, size_t global_size, size_t local_size);
void opencl_init(struct gpu_context *, int profiling, char *kernel_function, char *source_file, char *build_flags);
cl_ulong opencl_run(struct gpu_context *ctx, struct gpu_arg *args, size_t argc, buildpath_info info, size_t global_size, size_t local_size);
