#include <assert.h>
#include "league/build_combo/build_combo.h"
#include "combinatorics/combination.h"
#include "combinatorics/factorial.h"
#include "league/unit_test/find_max.h"

#define UTBC_DEF "-DUNIT_TEST -ID:/GitRoot/llio/src/"
#define UTBC_SRC  "#include \"league/build_combo/build_combo.c\""

#define ITEM_COUNT 3

const item_t db_test[ITEM_COUNT] = 
{
     {{0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f}, 
      {0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f}, 
      0,0,0,0,0,         
      {0},
      {0}} /*(NULL-ITEM)*/,

     {{0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f}, 
      {0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f}, 
      1031,0,720,720,0,         
      {0},
      {0}} /*Chain Vest*/,

     {{45.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f}, 
      {0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f,0.000000f}, 
      1038,0,1550,1550,0,         
      {0},
      {0}} /*B. F. Sword*/,
};

void
unittest_buildcombo()
{
#define K 2
    setmax_t combo[K];
    opencl_workset workset;
    opencl_context gpu;
    opencl_function *func;
    opencl_kernel_arg *output;
    ulong_t ncombo;
    result_t *results, best;
    llf_criteria cfg = { 0 };
    cfg.time_frame = 3;
    cfg.ad_ratio = 3.4f;
    cfg.ap_ratio = 0;
    cfg.level = 18;
    cfg.enemy_armor = 100;
    cfg.enemy_mr = 100;
    cfg.build_maxcost = 15000;
    cfg.build_maxinventory = 6;

    opencl_init(&gpu, 1);

    func = opencl_build(&gpu, "build_combo", UTBC_SRC, UTBC_DEF);
    ncombo = NK_MULTISET(ITEM_COUNT, K);
    workset.iterations = 1;
    workset.local_size = 1;
    workset.pass_size  = (size_t)ncombo;
    workset.total      = ncombo;
    
    output = build_combo__bind__(func, K, db_test, ITEM_COUNT, &cfg, workset.pass_size, workset.local_size);
    results = output->buf_data;
    opencl_run(func, 1, TRUE, 0, &workset);
    best = FindMax(results, output->buf_size / sizeof *results);
    combo_unrank(best.index, ITEM_COUNT, K, combo);

    assert(combo[0] == 2 && combo[1] == 2);

    opencl_function_free(func, 1);
    opencl_free(&gpu);
}
