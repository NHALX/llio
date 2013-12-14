#include <assert.h>
#include "combinatorics/combination.h"

static_assert(NK_MULTISET_MAX_K > 2, "NK_MULTISET_MAX_K less than 2");

void unittest_combination()
{

    setmax_t reference[6][2] = { { 0, 0 }, { 1, 0 }, {1, 1}, {2, 0}, {2, 1}, {2, 2} };
    setmax_t combo[6][2];

    for (size_t i = 0; i < 6; ++i)
    {
        combo_unrank(i, 3, 2, combo[i]);

        assert(combo[i][0] == reference[i][0] &&
               combo[i][1] == reference[i][1]);
    }
}
