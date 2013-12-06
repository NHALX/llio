import collections
import ll_inibin
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

