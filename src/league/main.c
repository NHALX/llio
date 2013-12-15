#include <stdio.h>

#include "opencl_host/function.h"
#include "league/database/db_search.h"
#include "league/database/database.h"
#include "league/build_path/opencl_bind.h"
#include "league/build_combo/item_filter.h"
#include "league/build_combo/build_combo.h"
#include "poset/lattice.h"
#include "combinatorics/factorial.h"

extern void unittest_league();
#define UTBC_DEF "-DUNIT_TEST -ID:/GitRoot/llio/src/"
#define UTBC_SRC  "#include \"league/build_combo/kernel/k_build_combo.c\""
#define INVENTORY_N 5

void
run(opencl_function *build_combo, opencl_function *build_path)
{
    item_t *items;
    size_t items_n;
    setmax_t combo[INVENTORY_N];
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

    items = item_filter(FC_SUSTAIN|FC_PHYSICAL_DAMAGE, &items_n);
    build_comboGPU(build_combo, &cfg, items, items_n, INVENTORY_N, combo);
    for (size_t i = 0; i < INVENTORY_N; ++i)
        combo[i] = db_find(items[combo[i]].id);
   
    printf("combo: \n\t%s,\n\t%s,\n\t%s,\n\t%s,\n\t%s\n", 
        db_names[combo[0]], 
        db_names[combo[1]], 
        db_names[combo[2]], 
        db_names[combo[3]], 
        db_names[combo[4]]
        );
}


int
main()
{
    opencl_context gpu;
    opencl_function *build_combo, *build_path;

    glbinit_lattice();
    unittest_league();

    opencl_init(&gpu, 1);
    
    build_combo = opencl_build(&gpu, "build_combo", UTBC_SRC, UTBC_DEF);
    build_path = clbp_init(&gpu);

    run(build_combo, build_path);
    
    opencl_function_free(build_combo, 1);
    opencl_function_free(build_path, CLBP_KERNEL_N);
    opencl_free(&gpu);
}