import ll_inibin

#0xf1e4103f onhit ad bonus?

same_files = [
#"D:\Dargon\output_dump\DATA\Spells\IreliaHitenStyle.inibin",
#"D:\dargon\output_dump\DATA\Spells\OlafRecklessStrike.inibin"
#    "D:\Dargon\output_dump\DATA\Spells\IreliaGatotsu.inibin",
#    "D:\Dargon\output_dump\DATA\Spells\Parley.inibin",
#    "D:\Dargon\output_dump\DATA\Spells\EzrealMysticShot.inibin"
"D:\Dargon\output_dump\DATA\Spells\IreliaEquilibriumStrike.inibin",
"D:\Dargon\output_dump\DATA\Spells\Disintegrate.inibin"
    ]

diff_files = [
   # "D:\Dargon\output_dump\DATA\Spells\IreliaEquilibriumStrike.inibin",
    #"D:\Dargon\output_dump\DATA\Spells\IreliaHitenStyle.inibin",
   # "D:\Dargon\output_dump\DATA\Spells\IreliaTranscendentBlades.inibin",
]   

same = [ ll_inibin.parse(x)[2] for x in same_files ]
diff = [ ll_inibin.parse(f)[2] for f in diff_files ]

def merge(operation, xs):
    
    i = set(xs[0].items())

    for x in xs[1:]:
        i = getattr(i, operation)(set(x.items()))
        
    return i
    
common = merge("intersection", same)
notval = set()

if len(diff_files) > 0:
    
    for x in diff:
        y = set(x.items())
        notval = notval.union(y.difference(common))
        
    notval = set([x[0] for x in notval])
    common = set([x[0] for x in common])

    result = common.intersection(notval)
else:
    result = set([x[0] for x in common])
    
print("\n".join([hex(x) for x in result]))

for x in result:
    for s in same:
        print(hex(x), "same", s[x])
        
    for d in diff:
        print(hex(x), "diff", d[x])