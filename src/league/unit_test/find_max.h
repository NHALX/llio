#include "league/common.h"

static result_t
FindMax(result_t *rs, size_t result_n)
{
    result_t max = { 0, 0 };
    size_t i;

    for (i = 0; i < result_n; ++i)
    {
        if (max.metric < rs[i].metric)
            max = rs[i];
    }

    return max;
}
