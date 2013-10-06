
#define C_TIME_FRAME   0
#define C_AD_RATIO     1
#define C_ENEMY_ARMOR  2
#define C_LEVEL        3
#define C_MAX_COST     4

#define CFG_SIZE       5
////////////////////////////////////////////

#define AS_OFFSET         (-0.04)
#define AS_LVPER          (0.0322)
#define AS_BASE           (0.625/(1+AS_OFFSET))

#define CALC_ATTACKSPEED(LV,GEAR) \
	min(2.5, AS_BASE * (1+(AS_LVPER*(LV-1)) + GEAR))

#define CALC_CRIT(B,C)    (1.0 + (1.0 + B) * min(1.0, C))

#define CALC_ARMOR_MITIGATION(ARMOR, APP, APF)   \
	(100/(100+((ARMOR*(1-APP))-APF)))

#define CALC_AD(AD,HP,HPRATIO)        (70+AD+(HP*HPRATIO))

#define FORMULA_DPS(ARMOR,LV,X)   (                                           \
	CALC_ARMOR_MITIGATION(ARMOR,X[F_ARMORPEN_PERCENT], X[F_ARMORPEN_FLAT]) *  \
	CALC_AD(X[F_AD], X[F_HP], X[F_HP2AD]) *                                   \
	CALC_CRIT(X[F_CRIT_BONUS], X[F_CRIT_CHANCE]) *                            \
	CALC_ATTACKSPEED(LV, X[F_ATTACK_SPEED])                                   \
)

#define FORMULA_ABILITY(ARMOR, SCALE, X)  (                                   \
	CALC_ARMOR_MITIGATION(ARMOR,X[F_ARMORPEN_PERCENT], X[F_ARMORPEN_FLAT]) *  \
	CALC_AD(X[F_AD], X[F_HP], X[F_HP2AD]) * SCALE                             \
)

#define FORMULA_TOTAL(CFG, X) (  \
	FORMULA_ABILITY((CFG)[C_ENEMY_ARMOR], (CFG)[C_AD_RATIO], X) +                 \
	(FORMULA_DPS((CFG)[C_ENEMY_ARMOR], (CFG)[C_LEVEL], X) * (CFG)[C_TIME_FRAME])  \
)
