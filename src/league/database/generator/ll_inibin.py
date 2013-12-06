import mmap
import struct
import array
import sys
import os
import functools
import itertools
import re
import math

import traceback
import ll_hashkeys
#import networkx as nx
#import matplotlib.pyplot as plt
from collections import namedtuple




def readByte(mm, index):
    x = mm[index]
    
    if type(x) == type(''):
        return ord(x)
    else:
        return mm[index]
    
def readString(mm, offset):
    s = ''
    
    while readByte(mm,offset) != 0x0:
        s += (chr(readByte(mm,offset)))
        offset += 1

    return s
    
def readValues(mm, index, ptype, count):
    fmt     = ptype*count
    results = struct.unpack_from('='+fmt, mm, index)
    return (results, struct.calcsize(fmt))

def readVector(mm, index, ptype, count):
    fmt     = ptype*count*3
    results = struct.unpack_from('='+fmt, mm, index)
    
    def grouper(n, iterable, fillvalue=None):
        "grouper(3, 'ABCDEFG', 'x') --> ABC DEF Gxx"
        args = [iter(iterable)] * n
        return itertools.zip_longest(*args, fillvalue=fillvalue)
    

    return (list(grouper(3, results)), struct.calcsize(fmt))
    
    
def readSegmentKeys(mm, index):
    (count,) = struct.unpack_from("=H", mm, index)
    fmt = 'I'*count
    results = struct.unpack_from('='+fmt, mm, index+2)
    return (list(results), struct.calcsize(fmt)+2)

def readBitfield(mm, index, count):
    extra = count % 8
    n     = count // 8 
    results = []
        
    def extract(bits, bc):
        for b in range(0, bc):
            results.append(bool(bits & 0x1))
            bits >>= 1
            
    for i in range(0, n):
        extract(readByte(mm,index+i), 8)
        
    if extra:
        extract(readByte(mm,index+n), extra)
        return (results, n + 1)
    else:
        return (results, n)
        


# The order of this list matters, because the types are laid out in a certain order in the file format
def _value(ptype): return lambda x, n: readValues(x[0],x[2],ptype,n)
def _vec3(ptype):  return lambda x, n: readVector(x[0],x[2],ptype,n)
def _offset(ctx, n):
    (xs,size) = _value('H')(ctx, n)
    return ([ readString(ctx[0], ctx[1]+x) for x in xs], size) 

def _u8div10(ctx, n):
    (vs,size) = _value('B')(ctx, n)
    # TODO: Decide if i really want this converted to float
    return ([ float(v)/10 for v in vs], size) 

    
_bitfield = lambda x, n: readBitfield(x[0], x[2], n)
        
        
_FMTS = [ ("FMT_U32"      , (0x0001, _value('I'))),
          ("FMT_FLOAT"    , (0x0002, _value('f'))),
          ("FMT_U8_PT"    , (0x0004, _u8div10)),
          ("FMT_U16"      , (0x0008, _value('H'))),
          ("FMT_U8"       , (0x0010, _value('B'))),
          ("FMT_BITFIELD" , (0x0020, _bitfield)),
          ("FMT_RGBA"     , (0x0400, _value('I'))),
          ("FMT_UNKNOWN1" , (0x0080, _vec3('f'))),
          ("FMT_OFFSET"   , (0x1000, _offset))
        ]

        


def fontconfig_en_US(root):
    with open(root + "/DATA/Menu/fontconfig_en_US.txt", "r+b") as f: 
        mm = mmap.mmap(f.fileno(), 0)
        
        r  = re.compile(("^tr \"game_item_displayname_(?P<key>\S+)\" = \"(?P<value>.+)\"\r\n").encode('utf-8'), flags=re.MULTILINE)
        d  = {}
        for x in r.finditer(mm):
            k    = b"game_item_displayname_" + x.group(1)
            d[k.decode('utf-8')] = x.group(2).decode('utf-8')
            
        return d # m.groupdict()
    
def parse(filename):

    with open(filename, "r+b") as f: 
        offset = 0
        mm = mmap.mmap(f.fileno(), 0)
        filesz = len(mm)
        
        #version = int(mm[0])
        #length  = mm[1:3]
        
        (version, symtabsz, format) = struct.unpack_from("=BhH", mm, offset)
        offset     += 5
        symbase     = filesz-symtabsz
        header_info = (filename, version, format, symtabsz, filesz)
        output      = []
        
        for (fmt_key,fmt_val) in _FMTS:
            if (format & fmt_val[0]) != 0:
                (keys,size)  = readSegmentKeys(mm,offset)
                offset += size
                (vals,size2) = fmt_val[1]((mm,symbase,offset), len(keys))
                offset += size2
                output += list(zip(keys,[fmt_key]*len(keys),vals))
                
        return (header_info, output)
        
    
def dump_keys(output):
      
    print("| %-10s | %-40s | %-20s" % ("KEY", "VALUE", "TYPE"))
    
    for (key,type,value) in output:
        s_val = str(value)

        if key in ll_hashkeys.value_key:
            name = ll_hashkeys.value_key[key]
        else:
            name = "Unknown"
            
        print("| 0x%08x %-20s | %-40s | %-20s" % (key, name, s_val, type))
        
def dump(file):
    
    ((filename, version, format, symtabsz, filesz), output) = parse(file)
    
    print("< file: %s >\n"
              "< format-ver: %d >\n"
              "< types-mask: 0x%04x >\n"
              "< sizes: (symtab=%d, total=%d) >\n" % 
              (filename, version, format, symtabsz, filesz))

    dump_keys(output)

#TODO: all the fields we are using arent even being loaded (like armor pen percent)
def getFields(strings, id, file, item):
    data = parse(file)
    _setItemStats(item, id, strings, data[1])
    return item
    
def _setItemStats(item, fileid, strings, output):

    index = lambda i, xs: [x[i] for x in xs]
    query = lambda d, xs: [d[x] for x in xs]

    db = dict(zip(index(0, output), index(2, output))) 
    def get_i(str):
        x = get(str)
        if x:
            return int(x)
        else:
            return 0
            
    def get_f(str, scale):
        x = get(str)
        if x:
            #return round(float(x) * scale)
            return float(x)
        else:
            return 0
        
    def rune_get(str):
        if ll_hashkeys.rune_table[str] in db:
            return db[ll_hashkeys.rune_table[str]]
        else:
            return None
            
    def get(str):
        if ll_hashkeys.item_table[str] in db:
            return db[ll_hashkeys.item_table[str]]
        else:
            return None

    item.id = fileid
    
    #item.id    = get_i("Id") #int(item.db[ll_hashkeys.table["Id"]])
    #if item.id != fileid:
    #    print("Warning: fileid mismatch", item.id, fileid)
        
    #item.Cost  = int(item.db[ll_hashkeys.table["GoldCost"]])
    item.valid     = bool(get("ItemValid"))
    item.RuneValid = bool(rune_get("RuneValid"))
    
    nkey = get("Name")
    if nkey in strings:
        item.Name  = strings[nkey]
    else:
        item.Name  = None
        
    #TODO: sync these
    #self.ArmorPenPercent=ArmorPenPercent
    #self.Movement=Movement
    #self.MovementBonus=MovementBonus
    #self.CDR = CDR
    #self.OnHit=OnHit
    #self.OnHitNeutral=OnHitNeutral
    #self.DamageBonusNeutral=DamageBonusNeutral
    #self.SplashDamage=SplashDamage
    #self.FlatLifeSteal=FlatLifeSteal
    #self.BounceDamage=BounceDamage
    item.HPRegen       = get_f("HealthRegenPerSecond", 100) # TODO: check that this is right field
    item.LifeSteal     = get_f("PercentLifeSteal", 100)
    item.FlatLifeSteal = 0 #TODO: key is unknown
    item.SpellVamp     = 0 #TODO: key is unknown
    
    item.HP            = get_i("Health")
    item.AD            = get_i("Damage")
    #item.RuneArmorPen      = get_f("ArmorPenetration")
    item.AttackSpeed   = get_f("PercentAttackSpeed", 100)
    item.CritChance    = get_f("PercentCriticalStrikeChance", 100)
    item.CritDamage    = get_f("PercentCriticalDamage", 100)
    item.MovementSpeed = get_f("PercentMovementSpeed", 100)
    item.MP            = get_i("Mana")
    #item.MP_5          = get_f("ManaRegenPerSecond")
    item.AbilityPower  = get_i("AbilityPower")
    #item.MagicPen      = get_f("MagicPenetration")
    #item.CDR           = get_f("PercentCooldownReduction")
    item.Armor         = get_i("Armor")
    item.MagicRest     = get_i("MagicResist")
    #item.Tenacity      = get()
    item.Cost          = get_i("GoldCost")
    #item.ResellValue   = get_f("ResellValue")
    item.Passive       = get("UniqueEffect")
    
    keys      = set(index(0, output))
    kfrom     = set(index(1, ll_hashkeys.BuildFrom))
    kto       = set(index(1, ll_hashkeys.BuildTo))
    process   = lambda x: list(filter(lambda y: y != 0, map(int, query(db, keys.intersection(x)))))
    
    item.BuildFrom = process(kfrom)
    item.BuildInto = process(kto)
    
    
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: inibin <filename>")
    else:
        dump(sys.argv[1])