#ifndef _LL_FORMULAS_H_
#define _LL_FORMULAS_H_

typedef struct
{
	uint_t  time_frame;
	float_t ad_ratio;
	float_t ap_ratio;
	uint_t  level;

	uint_t enemy_armor;
	uint_t enemy_mr;
	
	uint_t build_maxcost;
	uint_t build_maxinventory;

}  llf_criteria;


#define AS_OFFSET         (-0.04f)
#define AS_LVPER          (0.0322f)
#define AS_BASE           (0.625f/(1.0f+AS_OFFSET))


#ifdef _MSC_VER
#define inline __inline 
#endif

#ifdef INTEGER_STATS
static inline float llf_crit(c_uint B, c_uint C)
{
	c_uint bonus = 100 + B;
	c_uint chance = min((c_uint)100, C);
	return 1.0f + ((float)(bonus*chance) / 10000.0f);
}
static inline float llf_armor_mitigation(c_uint ARMOR, stat_t APP, stat_t APF)
{
	float reduceA = ((float)(ARMOR*(100 - APP))) / 100;
	return 100 / (100 + (reduceA - (float)APF));
}
#define PERCENT(X) ((float)X/100)
#else
#define PERCENT(X) ((float)X)

static inline float llf_crit(stat_t B, stat_t C)
{
	return 1.0f + (1.0f + ((float)B)) * min(1.0f, ((float)C));
}
static inline float llf_armor_mitigation(uint_t ARMOR, stat_t APP, stat_t APF)
{
	return 100 / (100 + ((((float)ARMOR)*(1 - ((float)APP))) - ((float)APF)));
}
#endif


static inline float llf_ability_damage(float mit, float ad, float ratio)
{
	return mit * ad * ratio;
}


static inline float llf_AD(stat_t AD, stat_t HP, stat_t HPRATIO)
{
	return ((float)(70 + AD)) + ((float)HP * PERCENT(HPRATIO));
}

static inline float llf_attackspeed(uint_t LV, stat_t GEAR)
{
	float level = (float)(LV - 1);
	float bonus = PERCENT(GEAR);

	return min(2.5f, AS_BASE * (1.0f + (AS_LVPER*level) + bonus));
}

static inline float llf_autoattack_DPS(float mitigation, float ad, uint_t LV, VECTOR(*X))
{
	float crit = llf_crit(F_CRIT_BONUS(*X), F_CRIT_CHANCE(*X));
	float aspd = llf_attackspeed(LV, F_ATTACK_SPEED(*X));

	return mitigation * ad * crit * aspd;
}

static inline float llf_dmgtotal(llf_criteria *CFG, VECTOR(*X))
{
	float mit = llf_armor_mitigation(CFG->enemy_armor, F_ARMORPEN_PERCENT(*X), F_ARMORPEN_FLAT(*X));
	float ad = llf_AD(F_AD(*X), F_HP(*X), F_HP2AD(*X));
	float dps = llf_autoattack_DPS(mit, ad, CFG->level, X);
	float ability = llf_ability_damage(mit, ad, CFG->ad_ratio);

	return ability + (dps * CFG->time_frame);
}


static inline float llf_sustain(llf_criteria *cfg, VECTOR(*X))
{
    float mit = llf_armor_mitigation(cfg->enemy_armor, F_ARMORPEN_PERCENT(*X), F_ARMORPEN_FLAT(*X));
    float ad = llf_AD(F_AD(*X), F_HP(*X), F_HP2AD(*X));
    float crit = llf_crit(F_CRIT_BONUS(*X), F_CRIT_CHANCE(*X));
    float hit = mit * ad * crit;
    float hit_rate = 10.0f / 60.0f;
    float cast_dmg = 0;
    float cast_rate = 0;

    return F_HPRegen(*X) +
        (F_LIFESTEAL_FLAT(*X) * hit_rate) +
        (F_LIFESTEAL_PERCENT(*X) * hit_rate * hit) +
        (F_SPELLVAMP(*X) * cast_rate * cast_dmg);
}



#endif

//#define LLF_CFG_LEN      5
////////////////////////////////////////////
/*
#define C_TIME_FRAME   0
#define C_AD_RATIO     1
#define C_ENEMY_ARMOR  2
#define C_LEVEL        3
#define C_MAX_COST     4

#define AS_OFFSET         (-0.04f)
#define AS_LVPER          (0.0322f)
#define AS_BASE           (0.625f/(1.0f+AS_OFFSET))

#define CALC_ATTACKSPEED(LV,GEAR) \
	min(2.5f, AS_BASE * (1.0f+(AS_LVPER*(LV-1.0f)) + GEAR))

#define CALC_CRIT(B,C)    (1.0f + (1.0f + B) * min(1.0f, C))

#define CALC_ARMOR_MITIGATION(ARMOR, APP, APF)   \
	(100/(100+((ARMOR*(1-APP))-APF)))

#define CALC_AD(AD,HP,HPRATIO)        (70+AD+(HP*HPRATIO))

#define FORMULA_DPS(ARMOR,LV,X)   (                                           \
	CALC_ARMOR_MITIGATION(ARMOR,F_ARMORPEN_PERCENT(X), F_ARMORPEN_FLAT(X)) *  \
	CALC_AD(F_AD(X), F_HP(X), F_HP2AD(X)) *                                   \
	CALC_CRIT(F_CRIT_BONUS(X), F_CRIT_CHANCE(X)) *                            \
	CALC_ATTACKSPEED(LV, F_ATTACK_SPEED(X))                                   \
)

#define FORMULA_ABILITY(ARMOR, SCALE, X)  (                                   \
	CALC_ARMOR_MITIGATION(ARMOR,F_ARMORPEN_PERCENT(X), F_ARMORPEN_FLAT(X)) *  \
	CALC_AD(F_AD(X), F_HP(X), F_HP2AD(X)) * SCALE                             \
)

#define FORMULA_TOTAL(CFG, X) (  \
	FORMULA_ABILITY((CFG)[C_ENEMY_ARMOR], (CFG)[C_AD_RATIO], X) +                 \
	(FORMULA_DPS((CFG)[C_ENEMY_ARMOR], (CFG)[C_LEVEL], X) * (CFG)[C_TIME_FRAME])  \
)
*/