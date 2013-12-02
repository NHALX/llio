
// Find maximum, NOTE: this requires a power of 2 local size
static
void
Reduce(result_t dps, __local volatile result_t* scratch, __global result_t* result)
{
    int offset;
    int local_index;

    local_index = get_local_id(0);
    scratch[local_index] = dps;
    barrier(CLK_LOCAL_MEM_FENCE);

    // parallel reduction
    for (offset = get_local_size(0) / 2;
        offset > 0;
        offset >>= 1) //offset = offset / 2) 
    {
        if (local_index < offset) {
            result_t other = scratch[local_index + offset];
            scratch[local_index] = (dps.metric > other.metric) ? dps : other;
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (local_index == 0) {
        result[get_group_id(0)] = scratch[0];
    }
}
