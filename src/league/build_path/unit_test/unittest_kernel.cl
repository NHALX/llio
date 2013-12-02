#include "league/build_path/kernel/metric_ADPS.c"
	   
__kernel void kunittest_mem(DB,
	__global itemid_t id[],
	__global stat_t  stat_s0[],
	__global stat_t  stat_s3[],
	__global stat_t  stat_s6[],
	__global VECTOR(*stat_all))
{
	size_t i = get_global_id(0);
	id[i] = db_items[i].id;
	stat_s0[i] = PART(db_items[i].stats, 0);
	stat_s3[i] = PART(db_items[i].stats, 3);
	stat_s6[i] = PART(db_items[i].stats, 6);

	VECTOR_COPY(stat_all[i], db_items[i].stats);
}
  


__kernel void kunittest_mergestats(DB,
	__global itemid_t *xs,
	uint xs_n,
	__global VECTOR(*output))
{
	VECTOR(stats) = VECTOR_ZERO_INIT;
	size_t i;
	size_t id = get_global_id(0);

	xs += id * xs_n;

	for (i = 0; i < xs_n; ++i)
		MergeStats(db_items, &stats, xs[i], true);

	VECTOR_COPY(output[id], stats);
}

__kernel void kunittest_clearsubcomponents(
	__local ideal_t *pasv_scratch,
	DB,
	__global itemid_t *xs_in,
	uint xs_n,
	__global int(*output_inventory)[LINEXT_WIDTH_MAX],
	__global VECTOR(*output_stats))
{
	size_t x;
	int inventory_slots;
	itemid_t xs[LINEXT_WIDTH_MAX];
	VECTOR(stats) = VECTOR_ZERO_INIT;
	size_t id = get_global_id(0);

    pasv_scratch += get_local_id(0) * PASV_SCRATCH_LEN(xs_n);
	ZERO_INIT(pasv_scratch, PASV_SCRATCH_LEN(xs_n));
	xs_in += id * xs_n;
	
	for (x = 0; x < xs_n; ++x) // TODO: why is this here?
		xs[x] = xs_in[x];

	
	for (x = 0, inventory_slots = 0; x < xs_n; ++x)
	{
		inventory_slots += AddItem(db_items, pasv_scratch, &stats, xs[x]);
		output_inventory[id][x] = inventory_slots;
	}

	for (x = xs_n; x < LINEXT_WIDTH_MAX; ++x)
		output_inventory[id][x] = 0;

	VECTOR_COPY(output_stats[id], stats);
}

 
__kernel void kunittest_llformulas(DB,
	__global itemid_t *xs,
	llf_criteria cfg,
	__global float(*output)[7])
{
	size_t id = get_global_id(0);
	VECTOR(stats);
	float mit, ad, crit, aspd;
	 
	VECTOR_COPY(stats, db_items[xs[id]].stats);

	mit = llf_armor_mitigation(cfg.enemy_armor, F_ARMORPEN_PERCENT(stats), F_ARMORPEN_FLAT(stats));
	ad = llf_AD(F_AD(stats), F_HP(stats), F_HP2AD(stats));
	crit = llf_crit(F_CRIT_BONUS(stats), F_CRIT_CHANCE(stats));
	aspd = llf_attackspeed(cfg.level, F_ATTACK_SPEED(stats));

	output[id][0] = llf_dmgtotal(&cfg, &stats);
	output[id][1] = llf_autoattack_DPS(mit, ad, cfg.level, &stats);
	output[id][2] = mit;
	output[id][3] = ad;
	output[id][4] = crit;
	output[id][5] = aspd;
	output[id][6] = llf_ability_damage(mit, ad, cfg.ad_ratio);
}

