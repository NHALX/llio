import ll_inibin
import item
import item_passive
from collections import *

def out_mathematica():
    for k,v in passivedb.items():

        name = v[0]
        x    = v[1][0][1]
        print("{Id -> passive%d, Name -> \"Passve: %s\", Cost -> 0,  Targets -> %s, Attack -> %s, CritChance -> %s, CritDamage -> %s, AttackSpeed -> %s, HP -> %s}" %
              (k, name, "{}", str(x.AD), str(x.CritChance), str(x.CritDamage), str(x.AttackSpeed), str(x.HP)))


    
    
def print_items(
    output_name, 
    output_item, 
    itemdb_unsorted, filter_func=lambda _: True):
    
    filtered = list(filter(filter_func, itemdb_unsorted.items()))
    viable   = OrderedDict(sorted(filtered, key=lambda t: t[1].id))

    
    passv_keys = list(item_passive.passive_unique.keys())
    
    for v in viable.values():
        output_name(v.Name)
        
    for v in viable.values():
        
        if v.id in item_passive.passive_stackable:
            (_,[x]) = item_passive.passive_stackable[v.id]
            v += x
        
        if str(v.id) in item_passive.passive_unique:
            passive_key = str(v.id)
            passive_index = passv_keys.index(passive_key)
            
        elif str(v.id) in item_passive.passive_named:
            passive_key = item_passive.passive_named[str(v.id)]
            passive_index = passv_keys.index(passive_key)
            
        else:
            passive_key = "(NULL)"
            passive_index = passv_keys.index(passive_key)
            
        output_item(viable, v, passive_key, passive_index)
        
        #output graph
        #edges = v.buildTreeToEdges(itemdb)
        #output_depgraph(edges)
    
    #for v in passive_unique.values():
    #    output_passive(v)
            
    #for v in viable.values():
    #    output_btree(viable, v)

            
itemdir = "D:/Dargon/output_dump/DATA/Items/"
spelldir = "D:/Dargon/output_dump/DATA/Spells/"
buffs    = "D:/Dargon/output_dump/DATA/Buffs/"
strings = ll_inibin.fontconfig_en_US("D:/Dargon/output_dump/")
itemdb  = item.gen_itemdb(strings, itemdir)
null_item = item.Item(0)
null_item.Name = "(NULL-ITEM)"
null_item.valid = True
null_item.Cost  = 0
null_item.BuildFrom = []
null_item.BuildTo = []

itemdb[0] = null_item


outdir = "../"

def item_fields(v,passive_index):
    return (v.id, passive_index, item.total_cost(itemdb,v.id), v.Cost, len(v.BuildFrom))

def stats_fields(v):
    return (v.AD, v.CritChance, v.CritDamage, v.AttackSpeed, v.ArmorPen, v.ArmorPenPercent, v.HP, v.HPtoAD,
            v.HPRegen,v.LifeSteal,v.FlatLifeSteal,v.SpellVamp,0,0,0,0)

def static_output():
    item_format = """  
     {{%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff}, 
      {%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff,%ff}, 
      %d,%d,%d,%d,%d,         
      %s,
      {0}} /*%s*/"""
      
    with open(outdir+"database.h", "w") as database_h:            
        with open(outdir+"db_items.c", "w") as db_items:
            with open(outdir+"db_names.c", "w") as db_names:
                with open(outdir+"db_info.h", "w") as db_info_h:
                    items = []
                    names = []
                    max_width = 0
                    
                    def fmt_item(db, item, passive_key, passive_index):
                        nonlocal max_width
                        edges = item.BuildFrom
                        max_width = max(len(edges), max_width)
                        build = []
                        if edges:
                            s = ["%d" % list(db.keys()).index(e) for e in edges]
                            build = "{%s}" % ", ".join(s)
                        else:
                            build = "{0}"
                            
                        #if passive_key != None:
                        #    passv = Stats(0)
                        passv = item_passive.passive_unique[passive_key]  
                        items.append(item_format % 
                          (stats_fields(item) + 
                           stats_fields(passv) + 
                           item_fields(item, passive_index) +
                           (build,) +
                           (item.Name,))
                        )
                       
                    fmt_name = lambda n: names.append("\"%s\"" % n)

                        
                    #viable, item_passive.passive_unique, passive_index, v
                    print_items(fmt_name,fmt_item,itemdb,lambda x: x[1].valid)
                             
                    db_info_h.write("#define PASSIVE_NULL %d\n" % list(item_passive.passive_unique.keys()).index("(NULL)"))
                    db_info_h.write("#define BUILDTREE_WIDTH %d\n" % max_width)
                    db_info_h.write("#define DB_LEN %d\n" % len(items))
                    
                    db_items.write("#include \"db_layout.h\"\n")
                    db_items.write("const item_t db_items[DB_LEN] = {\n")
                    db_items.write(",\n".join(items))
                    db_items.write("\n};\n\n");
                    
                    db_names.write("#include \"db_layout.h\"\n")
                    db_names.write("const char *db_names[DB_LEN] = {\n")
                    db_names.write(",\n".join(names))
                    db_names.write("\n};\n\n");
                    
                    print(max_width)
                    database_h.write("#include \"db_layout.h\"\n")
                    database_h.write("extern const item_t db_items[DB_LEN];\n")
                    database_h.write("extern const char *db_names[DB_LEN];\n")
  
#static_output()

def txt_output():
    item_format = """%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%s,0,\"%s\""""
      
    with open(outdir+"db_items.txt", "w") as db_items:

                items = []
                names = []
                max_width = 0
                
                def fmt_item(db, item, passive_key, passive_index):
                    nonlocal max_width
                    edges = item.BuildFrom
                    max_width = max(len(edges), max_width)
                    build = []
                    if edges:
                        s = ["%d" % list(db.keys()).index(e) for e in edges]
                        build = "{%s}" % "|".join(s)
                    else:
                        build = "{}"
                        
                    #if passive_key != None:
                    #    passv = Stats(0)
                    passv = item_passive.passive_unique[passive_key]  
                    items.append(item_format % 
                      (stats_fields(item) + 
                       stats_fields(passv) + 
                       item_fields(item, passive_index) +
                       (build,) +
                       (item.Name,))
                    )
                   
                fmt_name = lambda n: names.append("\"%s\"" % n)

                    
                #viable, item_passive.passive_unique, passive_index, v
                print_items(fmt_name,fmt_item,itemdb,lambda x: x[1].valid)
                         
                db_items.write(",\n".join(items))
                    


def v2str(obj):
    s = []
    for k,v in obj.__dict__.items():
        if k != "Name" and k != "id" and k != "Cost" and k != "BuildFrom" and k != "BuildInto" and k != "valid" and k != "Passive": #TODO: fix this hack
            s.append("\t\t\t%s:%s" % (str(k),str(v)))
        
    return "{\n" + ",\n".join(s) + "\n\t\t}"
    
    
import sys
def js_output(file):
    item_format = """
    "%s": { 
        stats: %s,
        passiv_stats: %s,
        id: %d,
        passiv_index: %d,
        total_cost: %d,
        cost: %d,
        build_len: %d,
        build_tree: %s
    }"""
    
    with open(file, "w") as db_items:
                items = []
                names = []
                max_width = 0
                
                def fmt_item(db, item, passive_key, passive_index):
                    nonlocal max_width
                    edges = item.BuildFrom
                    max_width = max(len(edges), max_width)
                    build = []
                    if edges:
                        s = ["%d" % list(db.keys()).index(e) for e in edges]
                        build = "[%s]" % ",".join(s)
                    else:
                        build = "[]"
                        
                    #if passive_key != None:
                    #    passv = Stats(0)
                    passv = item_passive.passive_unique[passive_key]  
                    items.append(item_format % 
                      ((item.Name, v2str(item), v2str(passv)) + item_fields(item, passive_index) + (build,))
                    )
                   
                fmt_name = lambda n: names.append("\"%s\"" % n)

                    
                #viable, item_passive.passive_unique, passive_index, v
                print_items(fmt_name,fmt_item,itemdb,lambda x: x[1].valid)
                db_items.write("var False = 0;\nvar True = 1;\nvar db_items = {")
                db_items.write(",\n".join(items))
                db_items.write("};")
                db_items.write("""
 function get_item_names(){ 
    var names = [];  
    for(var i in db_items)
    { 
        names.push(i);
    }
    return names;
}
""")

js_output("C:\\Users\\Media Markt\\Downloads\\graph-1.3.2\\data\\db_items.js")                