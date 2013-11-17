(* ::Package:: *)

(* ::Text:: *)
(*Code based on: "Ef\[FiLigature]cient computation of rank probabilities in posets" by Karel De Loof.*)


BeginPackage["LatticePoset`"]
idealLattice::usage          = "idealLattice[g] - Generates lattice of ideals from poset relationships defined by graph G"
allLinearExtensions::usage   = "allLinearExtensions[l] - Finds all linear extensions from lattice of ideals 'l'"
countLinearExtensions::usage = "countLinearExtensions[l] - The number of unique linear extensions"
nthLinearExtension::usage    = "nthLinearExtension[l, n] - Unrank the nth linear extension"
randomLinearExtension::usage = "randomLinearExtension[l] - Generates a random linear extension"
export::usage                = "export[l] - Export the lattice of ideals as an array of C structs"
MaxCombo
MaxOutDegree
CVertex
CAdjacency
CIdeals
CCount
Begin["`Private`"]


(* ::Text:: *)
(*Constructing the lattice of ideals.*)
(*See: Notes/LatticePoset_1_construction.png*)


minimalElements[G_?(VertexCount[#] == 0&)] := {}
minimalElements[G_]                        := VertexList[Subgraph[G, _?(VertexInDegree[G,#] == 0&)]];

init[{G_,L_}, x_]:= 
	{System`EdgeAdd[G, {} \[DirectedEdge] {x}],
	Append[L, {x}]}

stuff[{g_,l_}, x_] := With[{i2=Union[i,{x}]}, 
	If[Not[MemberQ[l,i2]], 
		{System`EdgeAdd[g, i \[DirectedEdge] i2], Append[l,i2]},
		{System`EdgeAdd[g, i \[DirectedEdge] i2], l}
	]
]

idealLatticeInternal[P_] := Block[{G,L,i},
	{G,L} = Fold[init, {System`Graph[{{}},{}, VertexLabels->"Name"],{}}, minimalElements[P]];

	While[Length[L] != 0, 
		i = L[[1]];
		L = L[[2;;]];
		{G,L} = Fold[stuff, {G,L}, minimalElements[VertexDelete[P,i]]]
	];

	G
]

testGraph = System`Graph[{2\[DirectedEdge]4,1\[DirectedEdge]3,1\[DirectedEdge]4,3\[DirectedEdge]5}, VertexLabels->"Name"];
lattice = idealLatticeInternal[testGraph]

(*
AdjacencyMatrix[testGraph] // MatrixForm
IncidenceMatrix[testGraph] // MatrixForm
genIncidenceMatrix[EdgeList[testGraph]] // MatrixForm
*)
(*
testMedium = idealLattice[System`Graph[{1\[DirectedEdge]2,3\[DirectedEdge]2,4\[DirectedEdge]3,5\[DirectedEdge]6,7\[DirectedEdge]6}]];
testLarge = idealLattice[System`Graph[{1\[DirectedEdge]2,3\[DirectedEdge]2,4\[DirectedEdge]3,5\[DirectedEdge]6,7\[DirectedEdge]6,8\[DirectedEdge]9,10\[DirectedEdge]8,11\[DirectedEdge]8,12\[DirectedEdge]9,13\[DirectedEdge]12,14\[DirectedEdge]12,15\[DirectedEdge]9,16\[DirectedEdge]15,17\[DirectedEdge]15}]];
*)
(*System`Graph[{2\[DirectedEdge]4,1\[DirectedEdge]3,1\[DirectedEdge]4,3\[DirectedEdge]5}, VertexLabels->"Name"]*)


(* ::Input:: *)
(**)


(* Discover all LE by depth first search *)
imSucc[g_,v_] := Cases[IncidenceList[g, v, 1], (v \[DirectedEdge] _)][[All, 2]]
setProps[g_,prop_] := Fold[SetProperty[{#1,{#2}}, VertexLabels -> PropertyValue[{#1,#2}, prop]]&, g, VertexList[g]] 
(********)

dfsLE[g_,root_,end_,le_,all2_] := Module[{walkSub,extend},
	extend[v_]                := Append[le,First@Complement[v,root]];
	walkSub[sle_,end] := Append[sle, extend[end]];
	walkSub[sle_,v_]  := dfsLE[g,v,end,extend[v], sle];
	Fold[walkSub, all2, imSucc[g,root]]
]

allLinearExtensions[g_] := With[{dst = First@VertexList[g,_?(VertexOutDegree[g,#] == 0&)]}, dfsLE[g,{},dst,{},{}]]

allLinearExtensions[lattice]


(* ::Text:: *)
(*Counting the number of linear extensions/paths beneath a node.*)
(*See: Notes/LatticePoset_2_comparison.png*)


AssignWrapper[g_] := With[{
	graph = Fold[SetProperty[{#1,{#2,#2}}, {Visited -> False,LEF -> 0}]&, g, VertexList[g]],
	src   = {},
	dst   = First@VertexList[g,_?(VertexOutDegree[g,#] == 0&)]},
	
	Assign[dst,SetProperty[{graph,{dst}}, LEF -> 1], src][[1]]
]

Assign[end_,graph_,vertex_] := Module[{f,iGraph,rGraph,rE},

		f[{g_,e_}, v2_] := Piecewise[{
			{{g,1}, v2 == end},
			{{g, e + PropertyValue[{g,v2}, LEF]}, PropertyValue[{g,v2}, Visited]}
			},
			With[{r=Assign[end,g,v2]}, {r[[1]], e + r[[2]]}]
		];
		iGraph      = SetProperty[{graph,{vertex}}, Visited -> True];
		{rGraph,rE} = Fold[f, {iGraph,0}, imSucc[iGraph,vertex]];
		{SetProperty[{rGraph,{vertex}}, LEF -> rE], rE}
	]

(**************)

idealLattice[g_]          := AssignWrapper[idealLatticeInternal[g]]
countLinearExtensions[l_] := PropertyValue[{l,{}}, LEF]



countLinearExtensions[idealLattice[testGraph]]



(* ::Text:: *)
(*Unranking paths in a graph/lattice: *)
(*http://cs.stackexchange.com/questions/16433/unranking-paths-in-a-graph-lattice *)
(**)
(*See: Notes/LatticePoset_3_unrank.png*)


nthPath[g_, {}, n_, xs_] := xs
nthPath[g_, vs_, n_, xs_] := Block[{counts,mkInt,intervals,next,int},
	counts     = Map[PropertyValue[{g,#}, LEF]&, vs];
	mkInt[{a_,is_},c_] := {a+c, Append[is,Interval[{a,a+c-1}]]};
	intervals  = Fold[mkInt, {0,{}}, counts][[2]];
	{next,int} = First @ Select[Thread[{vs,intervals}], IntervalMemberQ[#[[2]], n]&];
	nthPath[g, imSucc[g,next], n - Min[int], Append[xs,next]]
]
nthLinearExtension[g_, n_] := Block[{path},
	path = nthPath[g, imSucc[g,{}], n, {{}}];
	Map[First@Complement[path[[#+1]],path[[#]]]&, Range[Length[path]-1]]
]


widthTest = idealLattice[testGraph];
setProps[widthTest,LEF]

Map[nthLinearExtension[widthTest,#]&, Range[0,8]] // MatrixForm


(* ::Text:: *)
(*Random generation of linear extensions*)


randomLinearExtension[g_] := Block[{v={},le={}, t, c, r},
	While[imSucc[g,v] != {},
		t = PropertyValue[{g,v}, LEF];
		c = 0;
		r = RandomInteger[{1,t}];
		Do [
			c += PropertyValue[{g,v2}, LEF];
			If [r <= c, 
				AppendTo[le, First@Complement[v2,v]];
				v = v2;
				Break[]
				]	
		, {v2,imSucc[g,v]} ];
	];
	le
]


lattice = idealLattice[testGraph];
nLE     = countLinearExtensions[lattice]

SeedRandom[Method -> {"ExtendedCA", "Skip" -> 8}]

(* http://math.stackexchange.com/questions/509751/coupon-collectors-problem-worst-case-time *)
trialsFromP[n_, p_] := Block[{u,t},
	u = Log[1/Log[1/p]];
	t = n * Log[n] + n *u;
	t
	]

samples = Ceiling[trialsFromP[nLE,0.999]]
all = Table[randomLinearExtension[lattice], {samples}];
uniq = DeleteDuplicates[all]
Length[uniq]




(* ::Text:: *)
(*Exporting data as C structs.*)


bitmask[list_, subset_] := Module[{f},
  f[_] = 0;
  (f[#] = 1)& /@ subset;
  f /@ list]

maxOutDegree[g_] := Max[Map[VertexOutDegree[g,#]&,VertexList[g]]]
maxCombo[g_]     := Reverse@First@VertexList[g,_?(VertexOutDegree[g,#] == 0&)]

exportIdealLattice[g_,comboMax_, maxOutDegree_]:= 
	Map[With[{
		label  = FromDigits[bitmask[comboMax, Reverse[#]], 2],
		count  = PropertyValue[{g,#}, LEF],
		edges  = Map[\[Alpha] \[Function] VertexIndex[g, \[Alpha]], imSucc[g,#]],
		ideals = Map[\[Alpha] \[Function] First@Complement[\[Alpha],#], imSucc[g,#]]},
		{label, count, PadRight[edges,maxOutDegree,Undefined], PadRight[ideals,maxOutDegree,Undefined]}

		]&, VertexList[g]]

(*
test = idealLattice[System`Graph[{1\[DirectedEdge]2,3\[DirectedEdge]2,4\[DirectedEdge]3,5\[DirectedEdge]6,7\[DirectedEdge]6,8\[DirectedEdge]9}]];
test = lattice;
countLinearExtensions[test] *)

export[g_] := Block[{a=maxCombo[g],b=maxOutDegree[g],dat},
	dat = exportIdealLattice[g,a,b];
	{MaxCombo     -> Length[a],
	 MaxOutDegree -> b,
	 CVertex      -> dat[[All, 1]],
	 CCount        -> dat[[All, 2]],
	 CAdjacency   -> dat[[All, 3]],
	 CIdeals      -> dat[[All, 4]]
	}
]


End[]
EndPackage[]
