import collections
import inibin
from item import (Stats, gen_itemdb, total_cost, Item)
from collections import *
import sys
import string

KillStacks   = 0
EnemyMaxHP   = 3000
SelfLV       = 18
SelfIsMelee  = True
SelfIsRanged = not SelfIsMelee
FarmStack30  = 30


        
Magic="Magic"
TrueDamage="TrueDamage"

Stackable=0


DamageType = collections.namedtuple('DamageType', ['Physical', 'Magic', 'TrueDamage'])
#Vanquish       = Stats(OnHit=(Magic,20), OnHitNeutral=(Magic,20))
#Maim           = lambda n: Stats(OnHitNeutral=(TrueDamage,n))


# TODO: Deal with Sheen/Trinityforce, Statikk shiv, Sotd
passive_stackable = {
3072 : ("The Bloodthirster",        [Stats(AD=FarmStack30,LifeSteal=0.002*FarmStack30)]),
1055 : ("Doran's Blade",            [Stats(FlatLifeSteal=5)]),
}

passive_named = {
"3184" : "Rage",           # "Entropy"
"3123" : "GrievousWounds", # "Executioner's Calling"
"3022" : "Icy",            # "Frozen Mallet"
"3044" : "Rage",           # "Phage"
"1062" : "Prospector",     # "Prospector's Blade"
"3074" : "Cleave",         # "Ravenous Hydra"
"3077" : "Cleave"          # "Tiamat"
}

_passive_unique = {
"(NULL)"         : Stats(),
"Rage"           : Stats(), #[Situational Movement Speed]
"GrievousWounds" : Stats(), #[Enemy.HealEffectiveness-=0.5]
"Icy"            : Stats(MovementBonus=-(0.40-(0.10*SelfIsRanged))), #[Enemy.Movement*=1-(0.40-(0.10*Self.IsRanged))]
"Butcher"        : Stats(DamageBonusNeutral=0.1), 
"Prospector"     : Stats(HP=200), 
"Cleave"         : Stats(SplashDamageInner=0.6*SelfIsMelee,SplashDamageOuter=0.2*SelfIsMelee),    

"3005"           : Stats(HPtoAD=0.015),                                        # "Atma's Impaler"
"3134"           : Stats(ArmorPen=10,CDR=0.10),                                # "The Brutalizer"
"3166"           : Stats(AD=SelfLV*2,ArmorPen=10,CDR=0.05,Movement=25),        # "Bonetooth Necklace"
"3031"           : Stats(CritDamage=0.5),                                      # "Infinity Edge"
"3035"           : Stats(ArmorPenPercent=0.35),                                # "Last Whisper"
"3101"           : Stats(CDR=0.1),                                             # "Stinger"
"3141"           : Stats(AD=5*KillStacks,MovementBonus=0.15*(KillStacks==20)), # "Sword of the Occult"
"3142"           : Stats(ArmorPen=20),                                         # "Youmuu's Ghostblade"

#3071 : ("The Black Cleaver",        [(Unique, Stats(ArmorPen=10,ArmorPenPercent="0.05*min(5,AttackStacks)"))]),
#3153 : ("Blade of the Ruined King", [(Unique, Stats(OnHit=(Magic,"EnemyHP*0.05", OnHitNeutral=(Magic,60)))]), # Neutral="min(60,Enemy.HP*0.05)"
#3186 : ("Kitae's Bloodrazer",       [Stats(OnHit=(Magic,EnemyMaxHP*0.025))]),
#3185 : ("The Lightbringer",         [(Unique, Vanquish)]),
#3106 : ("Madred's Razors",          [(Unique, Maim(60))]),
#1039 : ("Hunter's Machete",         [(Unique, Butcher),(Unique,Maim(10))]),
#3154 : ("Wriggle's Lantern",        [(Unique, Maim(100))]),
#3085 : ("Runaan's Hurricane",       [(Unique, Stats(BounceDamage="10+0.05*Self.AD"))]),
#3181 : ("Sanguine Blade",           [(Unique, Stats(AD="6*min(5,AttackStacks)", LifeSteal="0.01*min(5,AttackStacks)"))]),

}
passive_unique  = OrderedDict(sorted(_passive_unique.items(), key=lambda t: t[0]))


def out_mathematica():
    for k,v in passivedb.items():

        name = v[0]
        x    = v[1][0][1]
        print("{Id -> passive%d, Name -> \"Passve: %s\", Cost -> 0,  Targets -> %s, Attack -> %s, CritChance -> %s, CritDamage -> %s, AttackSpeed -> %s, HP -> %s}" %
              (k, name, "{}", str(x.AD), str(x.CritChance), str(x.CritDamage), str(x.AttackSpeed), str(x.HP)))


    
    
def print_items(
    output_name, 
    output_item, 
    output_passive,
    itemdb_unsorted, filter_func=lambda _: True):
    itemdb  = OrderedDict(sorted(itemdb_unsorted.items(), key=lambda t: t[1].id))
    viable  = list(filter(filter_func, itemdb.values()))
    
    passv_keys = list(passive_unique.keys())
    
    for v in viable:
        output_name(v.Name)
        
    for v in viable:
        
        if v.id in passive_stackable:
            (_,[x]) = passive_stackable[v.id]
            v += x
        
        if str(v.id) in passive_unique:
            passive_index = passv_keys.index(str(v.id))
            
        elif str(v.id) in passive_named:
            passive_index = passv_keys.index(passive_named[str(v.id)])
            
        else:
            passive_index = passv_keys.index("(NULL)")
            
        output_item(v, passive_index)
        
        #output graph
        #edges = v.buildTreeToEdges(itemdb)
        #output_depgraph(edges)
    
    for v in passive_unique.values():
        output_passive(v)
            


            
itemdir = "D:/Dargon/output_dump/DATA/Items/"
spelldir = "D:/Dargon/output_dump/DATA/Spells/"
buffs    = "D:/Dargon/output_dump/DATA/Buffs/"
strings = inibin.fontconfig_en_US("D:/Dargon/output_dump/")
itemdb  = gen_itemdb(strings, itemdir)
null_item = Item(0)
null_item.Name = "(NULL-ITEM)"
null_item.valid = True
null_item.Cost  = 0
null_item.BuildFrom = []
null_item.BuildTo = []
itemdb[0] = null_item
# TODO: parameterize this
mathematica_code = """
itemDatabaseFields = {"F_ID", "F_PASSIVE", "F_COST", "F_UPGRADE_COST", "F_AD", "F_CRIT_CHANCE", "F_CRIT_BONUS", "F_ATTACK_SPEED", "F_ARMORPEN_FLAT", "F_ARMORPEN_PERCENT", "F_HP", "F_HP2AD"}
itemDatabaseRules  = ArrayRules [itemDatabaseFields]  /. (({x_} -> y_) :> y -> x)
itemPassiveSyncOffset = 4
"""

item_columns = """
#define F_ID                         0
#define F_PASSIVE                    1
#define F_COST                       2 
#define F_UPGRADE_COST               3
#define F_AD                         4
#define F_CRIT_CHANCE                5
#define F_CRIT_BONUS                 6
#define F_ATTACK_SPEED               7 
#define F_ARMORPEN_FLAT              8
#define F_ARMORPEN_PERCENT           9
#define F_HP                         10
#define F_HP2AD                      11
#define ITEM_WIDTH                   12
#define ITEM_PASSIVE_SYNC_OFFSET     F_AD /* Passive layout matches item layout after first few columns are dropped */
#define PASSIVE_WIDTH                (ITEM_WIDTH-ITEM_PASSIVE_SYNC_OFFSET)
#define PASSIVE_NULL                 %d
""" % list(passive_unique.keys()).index("(NULL)")

def item_fields(v,passive_index):
    return (v.id, passive_index, total_cost(itemdb,v.id), v.Cost, v.AD, v.CritChance, v.CritDamage, v.AttackSpeed, v.ArmorPen, v.ArmorPenPercent, v.HP, v.HPtoAD)

def passive_fields(v):
    return (v.AD, v.CritChance, v.CritDamage, v.AttackSpeed, v.ArmorPen, v.ArmorPenPercent, v.HP, v.HPtoAD)

def static_output():
    
    with open("database.h", "w") as source:
        source.write(item_columns)
        items = []
        passives = []
        fmt_name = lambda n: n

        fmt_item = lambda v, passive_index: items.append(
            "  {%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff}" % item_fields(v, passive_index)
           )
         
        fmt_passive = lambda v: passives.append(
            "  {%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff}" % passive_fields(v)
            )
            
        print_items(fmt_name,fmt_item,fmt_passive,itemdb,lambda x: x.valid)
                    
        source.write("\nstatic cl_float db_items[][ITEM_WIDTH] = {\n")
        source.write(",\n".join(items))
        source.write("\n};\n\n");
        source.write("static cl_float db_passives[][ITEM_WIDTH-ITEM_PASSIVE_SYNC_OFFSET] = {\n")
        source.write(",\n".join(passives))
        source.write("\n};\n\n");
        
def dat_output():
    
    with open("names.dat", "w") as names:
        with open("items.dat", "w") as items:
            with open("passives.dat", "w") as passives:
                with open("db_layout.h", "w") as layout:
                    with open("db_layout.m", "w") as mathematica:
                        layout.write(item_columns)
                        mathematica.write(mathematica_code)
                        
                        fmt_name = lambda n: names.write(n+"\n")

                        fmt_item = lambda v, passive_index: items.write(
                            "%d %d %d %d %d %f %f %f %f %f %f %f\n" % item_fields(v,passive_index))
                         
                        fmt_passive = lambda v: passives.write(
                            "%d %f %f %f %f %f %f %f\n" % passive_fields(v)
                            )
                            
                        print_items(fmt_name,fmt_item,fmt_passive,itemdb,lambda x: x.valid)

 


   
def build_tree():
    lines = []
    for item in itemdb.values():
        edges = item.buildTreeToEdges(itemdb)
        if edges:
            str = ["{\"%s\",\"%s\"}" % t for t in edges]
            line = "\t%-35s -> {%s}" % (("\"%s\"" % item.Name), ", ".join(str))
            lines.append(line)

    sys.stdout.write("buildTree = {\n%s\n};\n" % ",\n".join(lines))
    
static_output()
dat_output()
build_tree()