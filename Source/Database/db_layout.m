
itemDatabaseFields = {"F_ID", "F_PASSIVE", "F_COST", "F_AD", "F_CRIT_CHANCE", "F_CRIT_BONUS", "F_ATTACK_SPEED", "F_ARMORPEN_FLAT", "F_ARMORPEN_PERCENT", "F_HP", "F_HP2AD"}
itemDatabaseRules  = ArrayRules [itemDatabaseFields]  /. (({x_} -> y_) :> y -> x)
itemPassiveSyncOffset = 3
