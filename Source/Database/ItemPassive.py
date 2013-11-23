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
"3071"           : Stats(ArmorPen=10,ArmorPenPercent=0.05),                    # "The Black Cleaver" Assumption: 1 stack

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
    output_btree,
    itemdb_unsorted, filter_func=lambda _: True):
    
    filtered = list(filter(filter_func, itemdb_unsorted.items()))
    viable   = OrderedDict(sorted(filtered, key=lambda t: t[1].id))

    
    passv_keys = list(passive_unique.keys())
    
    for v in viable.values():
        output_name(v.Name)
        
    for v in viable.values():
        
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
            
    for v in viable.values():
        output_btree(viable, v)

            
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
itemDatabaseFields = {"F_ID", "F_PASSIVE", "F_COST", "F_UPGRADE_COST", "F_SLOT_MERGE", "F_AD", "F_CRIT_CHANCE", "F_CRIT_BONUS", "F_ATTACK_SPEED", "F_ARMORPEN_FLAT", "F_ARMORPEN_PERCENT", "F_HP", "F_HP2AD"}
itemDatabaseRules  = ArrayRules [itemDatabaseFields]  /. (({x_} -> y_) :> y -> x)
itemPassiveSyncOffset = 5
"""

# TODO: bag constraints
item_columns = lambda btree_width: """
#ifndef __OPENCL_VERSION__
#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#endif

#define F_ID                         0
#define F_PASSIVE                    1
#define F_COST                       2 
#define F_UPGRADE_COST               3
#define F_SLOT_MERGE                 4
#define F_AD                         5
#define F_CRIT_CHANCE                6
#define F_CRIT_BONUS                 7
#define F_ATTACK_SPEED               8 
#define F_ARMORPEN_FLAT              9
#define F_ARMORPEN_PERCENT           10
#define F_HP                         11
#define F_HP2AD                      12
#define ITEM_WIDTH                   13
#define ITEM_PASSIVE_SYNC_OFFSET     F_AD /* Passive layout matches item layout after first few columns are dropped */
#define ITEM_SIZEOF                  (ITEM_WIDTH*sizeof(cl_float))
#define PASSIVE_WIDTH                (ITEM_WIDTH-ITEM_PASSIVE_SYNC_OFFSET)
#define PASSIVE_NULL                 %d
#define BUILDTREE_WIDTH              %d
""" % (list(passive_unique.keys()).index("(NULL)"), btree_width)

def item_fields(v,passive_index):
    return (v.id, passive_index, total_cost(itemdb,v.id), v.Cost, len(v.BuildFrom), 
    v.AD, v.CritChance, v.CritDamage, v.AttackSpeed, v.ArmorPen, v.ArmorPenPercent, v.HP, v.HPtoAD)

def passive_fields(v):
    return (v.AD, v.CritChance, v.CritDamage, v.AttackSpeed, v.ArmorPen, v.ArmorPenPercent, v.HP, v.HPtoAD)

def static_output():
    with open("database.h", "w") as source_h:            
        with open("database.c", "w") as source:
            with open("db_layout.h", "w") as layout:
                items = []
                passives = []
                names = []
                btree = []
                max_width = 0
                
                def fmt_btree(db, item):
                    nonlocal max_width
                    #edges = item.buildTreeToEdges(itemdb)
                    edges = item.BuildFrom
                    max_width = max(len(edges), max_width)
                    if edges:
                        #str = ["{%s,%s}" % (a.id,b.id) for (a,b) in edges]
                        str = ["%d" % list(db.keys()).index(e) for e in edges]
                        line = "{%s} /*%s*/" % (", ".join(str), item.Name)
                        btree.append(line)
                    else:
                        btree.append("{0} /*%s*/" % item.Name)
                    
                fmt_name = lambda n: names.append("\"%s\"" % n)

                fmt_item = lambda v, passive_index: items.append(
                    "  {%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff} /*%s*/" % (item_fields(v, passive_index) + (v.Name,))
                   )
                 
                fmt_passive = lambda v: passives.append(
                    "  {%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff}" % passive_fields(v)
                    )
                    
                print_items(fmt_name,fmt_item,fmt_passive,fmt_btree,itemdb,lambda x: x[1].valid)
                         
                layout.write(item_columns(max_width))
                source.write("#include \"database.h\"")
                source.write("\nconst cl_float db_items[DB_LEN][ITEM_WIDTH] = {\n")
                source.write(",\n".join(items))
                source.write("\n};\n\n");
                source.write("const cl_float db_passives[DB_LEN][ITEM_WIDTH-ITEM_PASSIVE_SYNC_OFFSET] = {\n")
                source.write(",\n".join(passives))
                source.write("\n};\n\n");
                source.write("const char *db_names[DB_LEN] = {\n")
                source.write(",\n".join(names))
                source.write("\n};\n\n");
                source.write("const cl_short db_buildtree[DB_LEN][BUILDTREE_WIDTH] = {\n%s\n};\n" % ",\n".join(btree))
                print(max_width)
                source_h.write(item_columns(max_width) + """
#define DB_LEN %d
extern const cl_float db_items[DB_LEN][ITEM_WIDTH];
extern const cl_float db_passives[DB_LEN][ITEM_WIDTH-ITEM_PASSIVE_SYNC_OFFSET];
extern const char *db_names[DB_LEN];
extern const cl_short db_buildtree[DB_LEN][BUILDTREE_WIDTH];
""" % len(items))
#extern const cl_short db_buildtree[DB_LEN][BUILDTREE_WIDTH][2];
 
def dat_output():
    
    with open("names.dat", "w") as names:
        with open("items.dat", "w") as items:
            with open("passives.dat", "w") as passives:
                with open("db_layout.h", "w") as layout:
                    with open("db_layout.m", "w") as mathematica:
                        
                        mathematica.write(mathematica_code)
                        btree = []
                        max_width = 0
                        def fmt_btree(db, item):
                            nonlocal max_width
                            #edges = item.buildTreeToEdges(itemdb)
                            edges = item.BuildFrom
                            max_width = max(len(edges), max_width)
                            if edges:
                                #str = ["{%s,%s}" % (a.id,b.id) for (a,b) in edges]
                                str = ["%d" % list(db.keys()).index(e) for e in edges]
                                line = "{%s} /*%s*/" % (", ".join(str), item.Name)
                                btree.append(line)
                            else:
                                btree.append("{0} /*%s*/" % item.Name)
                        
                        fmt_name = lambda n: names.write(n+"\n")

                        fmt_item = lambda v, passive_index: items.write("%d %d %d %d %d %d %f %f %f %f %f %f %f\n" % item_fields(v,passive_index))
                         
                        fmt_passive = lambda v: passives.write("%d %f %f %f %f %f %f %f\n" % passive_fields(v))
                            
                        print_items(fmt_name,fmt_item,fmt_passive,fmt_btree,itemdb,lambda x: x[1].valid)
                        layout.write(item_columns(max_width))
 


   
def build_tree_mathematica():
    lines = []
    for item in itemdb.values():
        edges = item.buildTreeToEdges(itemdb)
        if edges:
            str = ["{\"%s\",\"%s\"}" % t for t in edges]
            line = "\t%-35s -> {%s}" % (("\"%s\"" % item.Name), ", ".join(str))
            lines.append(line)

    sys.stdout.write("buildTree = {\n%s\n};\n" % ",\n".join(lines))


    
static_output()
#dat_output()
