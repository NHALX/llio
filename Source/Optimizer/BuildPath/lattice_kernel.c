#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#include <limits.h>
#endif

#include "lattice_kernel.h"

int NthLinearExt(LATTICE, lattice_info *info, __global ideal_t *output, size_t global_id);

#ifndef __OPENCL_VERSION__
//////////////// HOST SIDE ONLY ////////////////

void
linext_print(ideal_t *le_buf, size_t le_len)
{
    unsigned int i;

    printf("%c", '{');
    for (i = 0; i < le_len; ++i)
    {
        printf("%d", le_buf[i]);

        if (i + 1 < le_len)
            printf("%c", ',');
    }

    printf("%c\n", '}');
}

void
linext_nth(ideal_lattice *il, ideal_t *le_buf, count_t index)
{
    lattice_info info;
    info.linext_count = il->linext_count;
    info.linext_offset = index;
    info.linext_width = il->linext_width;
    info.max_neighbors = il->max_neighbors;

    NthLinearExt(il->ideals, il->counts, il->neighbors, &info, le_buf, 0);
}


opencl_kernel_arg *
linext__bind__(opencl_context *ctx, cl_kernel k, opencl_kernel_params *a, ideal_lattice *g, opencl_kernel_arg **bpinfo, size_t pass_size)
{
    const size_t glblN_linext = g->linext_width * sizeof(*g->ideals);
    ka_mglobal(ctx, k, ka_push(a), "ideals", A_IN, CL_MEM_READ_ONLY, g->ideals, g->edge_count*sizeof(*g->ideals));
    ka_mglobal(ctx, k, ka_push(a), "counts", A_IN, CL_MEM_READ_ONLY, g->counts, g->vertex_count*sizeof(*g->counts));
    ka_mglobal(ctx, k, ka_push(a), "neighbors", A_IN, CL_MEM_READ_ONLY, g->neighbors, g->edge_count*sizeof(*g->neighbors));
    *bpinfo = ka_ignore(ctx, k, ka_push(a));
    return ka_mem(ctx, k, ka_push(a), GA_MEM, "linexts", 0, CL_MEM_READ_WRITE, 0, glblN_linext*pass_size);
}

opencl_allocinfo
linext__allocnfo__(opencl_context *ctx, cl_kernel k, ideal_lattice *g)
{
    cl_ulong klmem;
    opencl_allocinfo nfo = { 0 };

    NOFAIL(clGetKernelWorkGroupInfo(k, ctx->device,
        CL_KERNEL_LOCAL_MEM_SIZE, sizeof klmem, &klmem, 0));

    nfo.fixed.global += g->edge_count * (sizeof *g->ideals + sizeof *g->neighbors);
    nfo.fixed.global += g->vertex_count * sizeof *g->counts;

    nfo.scale_pass.global += g->linext_width * sizeof(*g->ideals);
    nfo.scale_workgroup.local += klmem;
    return nfo;
}

#else

//////////////// CLIENT SIDE ////////////////

__kernel void linext(
    __global ideal_t *ideals,
    __global count_t *counts,
    __global index_t *adjacency,
    lattice_info info,
    __global ideal_t *output)
{
    NthLinearExt(ideals, counts, adjacency, &info, output, get_global_id(0));
}

#endif

// TODO: make all lattice code seperate from rest of project
//////////////// BOTH HOST/CLIENT SIDE ////////////////
#define GET_EDGES(P,S,I) (&P[I*S])

int NthLinearExt(LATTICE, lattice_info *info, __global ideal_t *output, size_t global_id)
{
    int node_index;
    __global index_t *neighbors;
    __global ideal_t *iedges;
    size_t i;
    count_t nth_extension = info->linext_offset + global_id;
    size_t le_index = info->linext_width;

    output += info->linext_width * global_id;

    if (nth_extension < info->linext_count)
    {
        node_index = 0; // TODO: make sure this is a valid assumption
        while (le_index > 0)
        {
            count_t interval[2];
            count_t prev_count;
            ideal_t ideal;

            ideal = 0;
            neighbors = GET_EDGES(adjacency, info->max_neighbors, node_index);
            iedges    = GET_EDGES(ideals, info->max_neighbors, node_index);
            node_index = -1;

            for (prev_count = 0, i = 0; i < info->max_neighbors; ++i)
            {
                if (neighbors[i] == INVALID_NEIGHBOR)
                    continue; // TODO: break instead?

                node_index = neighbors[i];

                interval[0] = prev_count;
                prev_count += counts[node_index];
                interval[1] = prev_count - 1;

                if (nth_extension >= interval[0] && nth_extension <= interval[1])
                {
                    ideal = iedges[i];
                    break;
                }
            }

            if (ideal == 0) // error, nothing found
                return 1;

            output[--le_index] = ideal;
            nth_extension -= interval[0];
        }

        return 0;
    }
    else
    {
        for (i = 0; i < info->linext_width; i++)
            output[i] = 0;

        return 1;
    }
}
