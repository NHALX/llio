import functools
import os
import sys
import traceback
import copy

import inibin

    
class Stats:

    def boost_physicaldmg(self):
        return self.Attack > 0 or self.AttackSpeed > 0 or self.CritChance > 0
        
    def boost_magicdmg(self):
        return self.AbilityPower > 0 
        
    def __init__(self,AD=0,HP=0,HPtoAD=0,MaxHP=0,ArmorPen=0,ArmorPenPercent=0,LifeSteal=0,FlatLifeSteal=0,CritDamage=0,
                 CritChance=0,AttackSpeed=0,Movement=0,MovementBonus=0,CDR=0,OnHit=0,OnHitNeutral=0,DamageBonusNeutral=0,SplashDamageInner=0,SplashDamageOuter=0,BounceDamage=0):

        self.AD=AD
        self.HP=HP
        self.HPtoAD=HPtoAD
        self.MaxHP=MaxHP
        self.ArmorPen=ArmorPen
        self.ArmorPenPercent=ArmorPenPercent
        self.LifeSteal=LifeSteal
        self.CritDamage=CritDamage
        self.CritChance=CritChance
        self.AttackSpeed=AttackSpeed
        self.Movement=Movement
        self.MovementBonus=MovementBonus
        self.CDR = CDR
        self.OnHit=OnHit
        self.OnHitNeutral=OnHitNeutral
        self.DamageBonusNeutral=DamageBonusNeutral
        self.SplashDamageInner=SplashDamageInner
        self.SplashDamageOuter=SplashDamageOuter
        self.FlatLifeSteal=FlatLifeSteal
        self.BounceDamage=BounceDamage
    
    def __radd__(self,value):
        return __add(self,value)
        
    def __add__(self,value): #intersection of two objects
        z = copy.copy(self)
        
        xs = vars(self)
        ys = vars(value)
        zs = vars(z)
        
        setx = set(xs.keys())
        sety = set(ys.keys())
        
        # TODO: handle stats that are merged multiplicatively
        for k in (setx.union(sety)):
            if k in ys and k in xs:
                zs[k] += ys[k]
            elif k in ys:
                zs[k] = ys[k]
            
        return z
        

@functools.total_ordering
class Item(Stats):
    def __str__(self):
        return str(self.Name)
        
    def __hash__(self):
        return self.id
        
    def __eq__(self,other):
        return self.id == other.id
    
    def __lt__(self,other):
        return self.id < other.id
    
    def __init__(self, id):
        super().__init__()
        self.id = id

    def buildTreeToEdges(self, db):
        edges=[]
        def f(item, edges):
            for x in item.BuildFrom:
                edges.append((db[x], item))
                f(db[x], edges)
        
        f(self, edges)
        return edges
        
def newGraph():
    G = nx.DiGraph()
    G.add_node(0)
    return G




# TODO: this function is broken
def graph(cfg, G, output):
    
    item = Item(cfg, output[1])
    
    if item.valid == False:
        print(item.valid)
        return G
    
    #print("id: ", item.id, item.cost)
    #print("builds from:", item.buildFrom)   
    #print("builds into:", item.buildInto)
    
    G.add_node(item.id)
    
    if not item.buildFrom:
        #G.add_edge(0, item.id, label=item.cost)
        pass
    else:
        for v in item.buildFrom:

            file = "DATA/Items/%s.inibin" % v
            try:
                graph(cfg, G, parse(file))
                G.add_edge(v, item.id, label=item.cost)
            except:

                print("warning: graph: file not found", file)
                continue
            
    return G
    
#data = parse("%s" % sys.argv[1])




def total_cost(itemdb, x):
    item = itemdb[x]
    cost = item.Cost 
    
    for x in item.BuildFrom:
        cost += total_cost(itemdb, x)
        
    return cost
    
        

def gen_itemdb(strings, itemdir):  
    itemdb = {}
    
    for file in os.listdir(itemdir):
        fname = os.path.splitext(file)
        
        if fname[1].lower() != ".inibin":
            continue

        try:
            item = Item(id=None)
            inibin.getFields(strings, int(fname[0]), itemdir + file, item);
            
            if item.RuneValid == False and item.Name:
                itemdb[item.id] = item
            
        except Exception as e:
            tb = sys.exc_info()[2]
            print("Exception", e, "at: ", tb.tb_lineno  )
            traceback.print_exc()
            pass
    
    return itemdb
    
        #try:
        #    graph(g, data)
        #except:
        #    pass
            #print("Graph Exception")
            #dump(data)
        
#nx.draw_graphviz(g)
#plt.savefig("path.png")

#nx.write_dot(g,'graph.dot')

filter_physical = lambda x: x.boost_physicaldmg()
filter_passive  = lambda x: x.Passive != None

