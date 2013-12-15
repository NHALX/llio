#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "opencl_host/dummy.h"
#include "league/database/database.h"
#include "league/ll_formulas.h"
#include "poset/lattice.h"
#include "opencl_host/host.h"
#include "opencl_bind.h"
#include "kernel/metric_area.h"





opencl_workset
clbp_bind(opencl_function *f, ideal_lattice *l, item_t *items, size_t  items_len, llf_criteria *cfg_input, opencl_kernel_arg **output_ptr)
{
	opencl_kernel_arg *linext, *output;
	opencl_context *x = f->ctx;
    opencl_workset wset[2];
    opencl_workset work;
    lattice_info info;

    info.linext_width  = l->linext_width;
    info.max_neighbors = l->max_neighbors;
    info.linext_count  = l->linext_count;

    wset[0] = opencl_workcfg(x, l->linext_count, linext__allocnfo__(&f[0], l));
    wset[1] = opencl_workcfg(x, l->linext_count, metric_area__allocnfo__(&f[1], items_len));
    // TODO: this doesn't really handle the minimum resource requirement right
    work = (wset[0].local_size < wset[1].local_size)
        ? wset[0]
        : wset[1];

    linext = linext__bind__(&f[0], FALSE, l, work.pass_size);
    output = metric_area__bind__(&f[1], TRUE,
        &info,
        linext, items, items_len, cfg_input, 
        work.pass_size, 
        work.local_size);

    //output->io_flags |= A_OUT;
    *output_ptr = output;
    return work;
}

opencl_function *
clbp_init(opencl_context *ctx)
{
#define C_DEFINES "-ID:/GitRoot/llio/src/"

    char *kernels[CLBP_KERNEL_N] = { "linext", "metric_area" };
    char *files[CLBP_KERNEL_N] = {
        "D:/GitRoot/llio/src/poset/kernel/lattice_kernel.c",
        "D:/GitRoot/llio/src/league/build_path/kernel/metric_area.c",
    };
    
    return opencl_buildfilev(ctx, kernels, CLBP_KERNEL_N, files, CLBP_KERNEL_N, C_DEFINES);
}

