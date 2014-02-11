import ll_inibin
import ll_hashkeys
import sys
import os.path

def generic_tostr(obj):
    s = []
    for k,v in obj.__dict__.items():
        s.append("%s=%s" % (str(k),str(v)))
        
    return "\n".join(s)

class Show:
    def __str__(self): return generic_tostr(self)
    def __repr__(self): return generic_tostr(self)
    
ranks = [1,2,3,4,5]

class Abilitiy(Show):
        
    def __init__(self, db):
        def get(key):
            if ll_hashkeys.abilities_table[key] in db:
                return db[ll_hashkeys.abilities_table[key]]
            else:
                return None
                
        
        self.Name = get("Name")
        self.Effect1Amount = [get("Effect1AmountLV" + str(i)) for i in ranks]
        self.Effect2Amount = [get("Effect2AmountLV" + str(i)) for i in ranks]
        self.Mana          = [get("ManaLV" + str(i)) for i in ranks]
        self.Cooldown      = [get("CooldownLV" + str(i)) for i in ranks]
        
class ChampStats(Show):
    def __init__(self, db):
        def get(key):
            return db[ll_hashkeys.champion_table[key]]
            
        self.HealthBase = get("Health")
        self.HealthLv   = get("HealthPerLevel")
        self.AttackBase = get("Damage")
        self.AttackLv   = get("DamagePerLevel")
        self.ArmorBase  = get("Armor")
        self.ArmorLv    = get("ArmorPerLevel")
        self.MagicResistBase  = get("Armor")
        self.MagicResistLv    = get("ArmorPerLevel")
        
        self.HealthRegenBase = get("HealthRegen")
        self.HealthRegenLv   = get("HealthRegenPerLevel")
        self.AttackSpeedDelay = get("AttackSpeedDelay")
        self.AttackSpeedLv    = get("AttackSpeedPerLevel")/100
            
class Champion(Show):
    
    def __init__(self, db):
        def get(key):
            return db[ll_hashkeys.champion_table[key]]
            
        self.Name  = get("Name")
        self.Stats = ChampStats(db)
        
        self.AbilityRef = [get("FirstAbilityReference"), 
                        get("SecondAbilityReference"), 
                        get("ThirdAbilityReference"),
                        get("FourthAbilityReference")]
        
        self.Ability = []
        
        for k in self.AbilityRef:
            file = os.path.abspath(os.path.dirname(path) + "../../../Spells/" + k + ".inibin")
            db   = ll_inibin.parse(file)[2]
            
            x = Abilitiy(db)
            self.Ability.append(x)

            
            
def dump(db):
    for k,v in db.items():
        print("%s=%s" % (str(k),str(v)))
        
        
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: character <filename>")
    else:
        path = sys.argv[1]
        db   = ll_inibin.parse(path)[2]
        c    = Champion(db)

        
        #for k,v in c.__dict__.items():
        #    print("%s=%s" % (str(k),str(v)))
        for k,v in c.Stats.__dict__.items():
            print(("champion%s[%s] = %s;" % (os.path.splitext(os.path.basename(path))[0], k, v)))
