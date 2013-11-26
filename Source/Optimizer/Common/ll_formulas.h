#ifndef _LL_FORMULAS_H_
#define _LL_FORMULAS_H_

typedef struct
{
	c_uint time_frame;
	c_uint ad_ratio;
	c_uint ap_ratio;
	c_uint level;

	c_uint enemy_armor;
	c_uint enemy_mr;
	
	c_uint build_maxcost;
	c_uint build_maxinventory;

}  llf_criteria;


#define AS_OFFSET         (-0.04f)
#define AS_LVPER          (0.0322f)
#define AS_BASE           (0.625f/(1.0f+AS_OFFSET))

typedef c_ushort stat_t;

#ifdef _MSC_VER
#define inline __inline 
#endif

static inline float llf_attackspeed(c_uint LV, stat_t GEAR)
{
	float level = (float)(LV - 1);
	float bonus = ((float)GEAR) / 100;

	return min(2.5f, AS_BASE * (1.0f + (AS_LVPER*level) + bonus));
}

static inline float llf_crit(c_uint B, c_uint C)
{
	c_uint bonus = 200 + B;
	c_uint chance = min((c_uint)100, C);
	return 1.0f + ((float)(bonus*chance) / 10000.0f);
}

static inline float llf_armor_mitigation(c_uint ARMOR, stat_t APP, stat_t APF)
{
	float reduceA = ((float)(ARMOR*(100 - APP))) / 100;
	return 100 / (100 + (reduceA - (float)APF));
}

static inline float llf_AD(stat_t AD, stat_t HP, stat_t HPRATIO)
{
	c_uint base = 70 + AD;
	c_uint bonus = HP * HPRATIO;

	return ((float)base) + ((float)bonus / 100);
}

static inline float llf_autoattack_DPS(float mitigation, float ad, c_uint LV, stats_t *X)
{
	float crit = llf_crit(F_CRIT_BONUS(*X), F_CRIT_CHANCE(*X));
	float aspd = llf_attackspeed(LV, F_ATTACK_SPEED(*X));

	return mitigation * ad * crit * aspd;
}

static inline float llf_ability_damage(float mit, float ad, c_uint ratio)
{
	return mit * ad * ((float)ratio / 100);
}


static inline float llf_dmgtotal(__constant llf_criteria *CFG, stats_t *X)
{
	float mit = llf_armor_mitigation(CFG->enemy_armor, F_ARMORPEN_PERCENT(*X), F_ARMORPEN_FLAT(*X));
	float ad = llf_AD(F_AD(*X), F_HP(*X), F_HP2AD(*X));
	float dps = llf_autoattack_DPS(mit, ad, CFG->level, X);
	float ability = llf_ability_damage(mit, ad, CFG->ad_ratio);

	return ability + (dps * CFG->time_frame);
}


#define CALC_ATTACKSPEED(LV,GEAR) \
	min(2.5f, AS_BASE * (1.0f + (AS_LVPER*(((float)LV) - 1.0f)) + ((float)GEAR)))

#define CALC_CRIT(B,C)    (1.0f + (1.0f + ((float)B)) * min(1.0f, ((float)C)))

#define CALC_ARMOR_MITIGATION(ARMOR, APP, APF)   \
	(100 / (100 + ((((float)ARMOR)*(1 - ((float)APP))) - ((float)APF))))

#define CALC_AD(AD,HP,HPRATIO)        (70.0f+((float)AD)+(((float)HP)*((float)HPRATIO)))

#define FORMULA_DPS(ARMOR,LV,X)   (                                           \
	CALC_ARMOR_MITIGATION(ARMOR, F_ARMORPEN_PERCENT(X), F_ARMORPEN_FLAT(X)) * \
	CALC_AD(F_AD(X), F_HP(X), F_HP2AD(X)) *                                   \
	CALC_CRIT(F_CRIT_BONUS(X), F_CRIT_CHANCE(X)) *                            \
	CALC_ATTACKSPEED(LV, F_ATTACK_SPEED(X))                                   \
	)

#define FORMULA_ABILITY(ARMOR, SCALE, X)  (                                   \
	CALC_ARMOR_MITIGATION(ARMOR, F_ARMORPEN_PERCENT(X), F_ARMORPEN_FLAT(X)) * \
	CALC_AD(F_AD(X), F_HP(X), F_HP2AD(X)) * ((float)SCALE)                    \
	)

#define FORMULA_TOTAL(CFG, X) (  \
	FORMULA_ABILITY((CFG)->enemy_armor, (CFG)->ad_ratio, X) + \
	(FORMULA_DPS((CFG)->enemy_armor, (CFG)->level, X) * ((float)(CFG)->time_frame))  \
	)

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