#include "types.h"
#include "combinatorics/factorial.h"
#include "combinatorics/combination.h"

static int largest(ulong_t i, uint_t n, uint_t k, ulong_t *offset)
{
    ulong_t x = 0;

    do    x = NK_MULTISET(n, k);
    while (x > i && --n >= 0);

    *offset = x;
    return n;
}

void combo_unrank(ulong_t rank, uint_t n, uint_t k, setmax_t *combo)
{
    ulong_t offset;
    uint_t combo_len = k;

    rank %= NK_MULTISET(n, k); // force wrap around if rank exceeds highest index

    for (size_t i = 0; i < combo_len && k > 0; ++i)
    {
        combo[i] = largest(rank, n, k, &offset);
        rank -= offset;
        k -= 1;
    }
}
