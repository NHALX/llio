# ChampionMapping
def name(base, set): 
    return list(zip(["%s_%d" % (base, x) for x in range(0,len(set))], set))

BuildTo   = name("BuildTo", range(1652767513, 1652767523))
BuildFrom = name("BuildFrom", range(973644272, 973644282))


champion = [("Id",2921476548),
            ("Name",82690155),
            ("Description",3747042364),
            ("Lore",4243215483),
            ("ImageFileName",3606610482),
            ("Tags",4146314945),
            ("FirstAbilityReference",404599689),
            ("SecondAbilityReference",404599690),
            ("ThirdAbilityReference",404599691),
            ("FourthAbilityReference",404599692),
            ("PassiveAbilityName",3401798261),
            ("PassiveAbilityFlavorText",743602011),
            ("PassiveImageFileName",3810483779),
            ("ChampionValid",82690155),

            ("Damage",1880118880),
            ("DamagePerLevel",1139868982),
            ("Health",742042233),
            ("HealthPerLevel",3306821199),
            ("Mana",742370228),
            ("ManaPerLevel",1003217290),
            ("MovementSpeed",1081768566),
            ("Armor",2599053023),
            ("ArmorPerLevel",1608827366),
            ("MagicResist",1395891205),
            ("MagicResistPerLevel",4032178956),
            ("HealthRegen",4128291318),
            ("HealthRegenPerLevel",3062102972),
            ("ManaRegen",619143803),
            ("ManaRegenPerLevel",1248483905)]

abilities = [# AbilityMapping
            ("Name",1805538005),
            ("FlavorText",3431853604),
            ("Description",2634919164),
            ("Range",3806805222),
            ("ImageFileName",2059614685),

            ("Effect1AmountLevel1",466816973),
            ("Effect1AmountLevel2",4059954766),
            ("Effect1AmountLevel3",3358125263),
            ("Effect1AmountLevel4",2656295760),
            ("Effect1AmountLevel5",1954466257),

            ("Effect2AmountLevel1",3898289358),
            ("Effect2AmountLevel2",3196459855),
            ("Effect2AmountLevel3",2494630352),
            ("Effect2AmountLevel4",1792800849),
            ("Effect2AmountLevel5",1090971346),

            ("StartManaCost",3771724453),
            ("EndManaCost",3771724457) ]

items = BuildTo + BuildFrom + [
# ItemMapping

("ItemValid",1169232412),

("Id",1250849294),
("Name",2120726765),
#("StartBuildsInto_0",1652767513),
#("StartBuildsInto_1",1652767514),
#("StartBuildsInto_2",1652767515),
#("StartBuildsInto_3",1652767516),
#("StartBuildsInto_4",1652767517),
#("StartBuildsInto_5",1652767518),
#("StartBuildsInto_6",1652767519),
#("EndBuildsInto",1652767523),
#("StartBuiltFrom_0",973644272),
#("StartBuiltFrom_1",973644273),
#("EndBuiltFrom",973644282),
("Description",3747042364),
("ImageFileName",4278916085),
("GoldCost",2478814953),

("AbilityPower",3524488927),
("Armor",2125415132),
("Damage",899933219),
("Health",1993475205),
("HealthRegenPerSecond",3412476166),
("Mana",2522004106),
("ManaRegenPerSecond",1108872257),
("MagicResist",1673461700),

("PercentAttackSpeed",4117873288),
("PercentCriticalStrikeChance",3756168283),
("PercentCriticalDamage",921972736),
("PercentLifeSteal",1581007476),
("PercentMovementSpeed",1249439471),
("PercentManaSteal",1675658961),
("ArmorPenetration",1666342795),

("ResellValue",3760373072),#("PassiveGoldEverySecond",3760373072),
("UniqueEffect",0x202b1bfb),
("ActivateSpell",0xfe0efbd3) ]

runes = [# RuneMapping
        ("RuneValid",3086385998),
        ("ImageFileName",2414666880),
        ("Name",2120726765),
        ("Tier",3448275386),
        ("Color",973892934),
        ("Description",3747042364),

        # TODO: Need to make new class for Rune Modifiers
        ("AbilityPower",3524488927),
        ("AbilityPowerPerLevel",2904354520),
        ("Armor",2125415132),
        ("ArmorPenetration",1666342795),
        ("AttackDamage",899933219),
        ("AttackDamagePerLevel",3408390072),
        ("CriticalDamage",921972736),
        ("GoldEveryTenSeconds",3596459249),
        ("Health",1993475205),
        ("HealthPerLevel",2503536726),
        ("HealthRegenPerSecond",3412476166),
        ("HealthRegenPerSecondPerLevel",3470517503),
        ("MagicPenetration",1105291097),
        ("MagicResist",1673461700),
        ("MagicResistPerLevel",1201590105),
        ("Mana",2522004106),
        ("ManaPerLevel",2481029467),
        ("ManaRegenPerSecond",1108872257),
        ("PercentAttackSpeed",4117873288),
        ("PercentCooldownReduction",331225348),
        ("PercentCooldownReductionPerLevel",3683445259),
        ("PercentCriticalChance",3756168283),
        ("PercentCriticalDamage",921972736),
        ("PercentDodge",250160022),
        ("PercentExperienceGained",1975167751),
        ("PercentMovementSpeed",1249439471),
        ("PercentTimeDeadReduction",450607838),
        ]

item_table = dict(items)
rune_table = dict(runes)

value_key = dict([(v,k) for (k,v) in champion + abilities + items + runes])

assert len(item_table) == len(items)
assert len(rune_table) == len(runes)