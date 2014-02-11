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
            ("ManaRegenPerLevel",1248483905),
            #("AttackSpeedOffset", 0x1ed6b674),
            ("AttackSpeedDelay", 0x829c7b37),
            ("AttackSpeedPerLevel", 0x2de86566)]
            
abilities = [# AbilityMapping
            ("Name",1805538005),
            ("FlavorText",3431853604),
            ("Description",2634919164),
            ("Range",3806805222),
            ("ImageFileName",2059614685),

            ("Effect1AmountLV1",466816973),
            ("Effect1AmountLV2",4059954766),
            ("Effect1AmountLV3",3358125263),
            ("Effect1AmountLV4",2656295760),
            ("Effect1AmountLV5",1954466257),

            ("Effect2AmountLV1",3898289358),
            ("Effect2AmountLV2",3196459855),
            ("Effect2AmountLV3",2494630352),
            ("Effect2AmountLV4",1792800849),
            ("Effect2AmountLV5",1090971346),
            
            ("Effect3AmountLV1", -1260172849),
            ("Effect3AmountLV2", -1962002352),
            ("Effect3AmountLV3", 1631135441),
            ("Effect3AmountLV4", 929305938),
            ("Effect3AmountLV5", 227476435),
            
            ("Effect4AmountLV1", -2123667760),
            ("Effect4AmountLV2", 1469470033),
            ("Effect4AmountLV3", 767640530),
            ("Effect4AmountLV4", 65811027),
            ("Effect4AmountLV5", -636018476),
            
            ("Effect5AmountLV1", 1307804625),
            ("Effect5AmountLV2", 605975122),
            ("Effect5AmountLV3", -95854381),
            ("Effect5AmountLV4", -797683884),
            ("Effect5AmountLV5", -1499513387),

            ("ManaLV1",3771724453),
            ("ManaLV2",3771724454),
            ("ManaLV3",3771724455),
            ("ManaLV4",3771724456),
            ("ManaLV5",3771724457),

            ("CooldownLV1", 0x9cb7f6ce),
            ("CooldownLV2", 0x9cb7f6cf),
            ("CooldownLV3", 0x9cb7f6d0),
            ("CooldownLV4", 0x9cb7f6d1),
            ("CooldownLV5", 0x9cb7f6d2),
            
            ("FlatCooldown", 0x494f9223),
            ("Target", 0x9a34c4ef),
            ("TargetFlags", 0x6d0a0522),
            ("SpellSlot", 0x681c7297),
            
            ("Scaling1", 844968125),
            ("Scaling2", -1783890251),
            
            #("AppliesOnHit", 0x20004c61)
            ]

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
champion_table = dict(champion)
abilities_table = dict(abilities)

value_key = dict([(v,k) for (k,v) in champion + abilities + items + runes])

assert len(item_table) == len(items)
assert len(rune_table) == len(runes)