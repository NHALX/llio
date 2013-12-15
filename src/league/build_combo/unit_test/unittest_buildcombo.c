#include <assert.h>
#include "league/build_combo/kernel/k_build_combo.h"
#include "combinatorics/combination.h"
#include "combinatorics/factorial.h"
#include "league/unit_test/find_max.h"
#include "league/build_combo/build_combo.h"

#define UTBC_DEF "-DUNIT_TEST -ID:/GitRoot/llio/src/"
#define UTBC_SRC  "#include \"league/build_combo/kernel/k_build_combo.c\""

#define ITEM_COUNT 3
#define INVENTORY 2

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
    setmax_t combo[INVENTORY];

    opencl_context gpu;
    opencl_function *func;
   
    llf_criteria cfg = { 0 };
    cfg.time_frame = 3;
    cfg.ad_ratio = 3.4f;
    cfg.ap_ratio = 0;
    cfg.level = 18;
    cfg.enemy_armor = 100;
    cfg.enemy_mr = 100;
    cfg.build_maxcost = 15000;
    cfg.build_maxinventory = 6;
    cfg.metric_type = METRIC_ALL_IN;

    opencl_init(&gpu, 1);

    func = opencl_build(&gpu, "build_combo", UTBC_SRC, UTBC_DEF);
    build_comboGPU(func, &cfg, db_test, ITEM_COUNT, INVENTORY, combo);

    assert(combo[0] == 2 && combo[1] == 2);

    opencl_function_free(func, 1);
    opencl_free(&gpu);

    combo[0] = 0;
    combo[1] = 0;

    build_comboCPU(func, &cfg, db_test, ITEM_COUNT, INVENTORY, combo);

    assert(combo[0] == 2 && combo[1] == 2);
}
