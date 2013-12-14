#include "combinatorics/factorial.h"

// TODO: remove this opencl stuff
#ifdef __OPENCL_VERSION__
__constant
#endif
ulong_t factorial[NK_MULTISET_MAX_N][NK_MULTISET_MAX_K] =
    {{1,1,1,1,1,1},{2,3,4,5,6,7},{3,6,10,15,21,28},{4,10,20,35,56,84},{5,15,35,70,126,210},{6,21,56,126,252,462},{7,28,84,210,462,924},{8,36,120,330,792,1716},{9,45,165,495,1287,3003},{10,55,220,715,2002,5005},{11,66,286,1001,3003,8008},{12,78,364,1365,4368,12376},{13,91,455,1820,6188,18564},{14,105,560,2380,8568,27132},{15,120,680,3060,11628,38760},{16,136,816,3876,15504,54264},{17,153,969,4845,20349,74613},{18,171,1140,5985,26334,100947},{19,190,1330,7315,33649,134596},{20,210,1540,8855,42504,177100},{21,231,1771,10626,53130,230230},{22,253,2024,12650,65780,296010},{23,276,2300,14950,80730,376740},{24,300,2600,17550,98280,475020},{25,325,2925,20475,118755,593775},{26,351,3276,23751,142506,736281},{27,378,3654,27405,169911,906192},{28,406,4060,31465,201376,1107568},{29,435,4495,35960,237336,1344904},{30,465,4960,40920,278256,1623160},{31,496,5456,46376,324632,1947792},{32,528,5984,52360,376992,2324784},{33,561,6545,58905,435897,2760681},{34,595,7140,66045,501942,3262623},{35,630,7770,73815,575757,3838380},{36,666,8436,82251,658008,4496388},{37,703,9139,91390,749398,5245786},{38,741,9880,101270,850668,6096454},{39,780,10660,111930,962598,7059052},{40,820,11480,123410,1086008,8145060},{41,861,12341,135751,1221759,9366819},{42,903,13244,148995,1370754,10737573},{43,946,14190,163185,1533939,12271512},{44,990,15180,178365,1712304,13983816},{45,1035,16215,194580,1906884,15890700},{46,1081,17296,211876,2118760,18009460},{47,1128,18424,230300,2349060,20358520},{48,1176,19600,249900,2598960,22957480},{49,1225,20825,270725,2869685,25827165},{50,1275,22100,292825,3162510,28989675},{51,1326,23426,316251,3478761,32468436},{52,1378,24804,341055,3819816,36288252},{53,1431,26235,367290,4187106,40475358},{54,1485,27720,395010,4582116,45057474},{55,1540,29260,424270,5006386,50063860},{56,1596,30856,455126,5461512,55525372},{57,1653,32509,487635,5949147,61474519},{58,1711,34220,521855,6471002,67945521},{59,1770,35990,557845,7028847,74974368},{60,1830,37820,595665,7624512,82598880},{61,1891,39711,635376,8259888,90858768},{62,1953,41664,677040,8936928,99795696},{63,2016,43680,720720,9657648,109453344},{64,2080,45760,766480,10424128,119877472},{65,2145,47905,814385,11238513,131115985},{66,2211,50116,864501,12103014,143218999},{67,2278,52394,916895,13019909,156238908},{68,2346,54740,971635,13991544,170230452},{69,2415,57155,1028790,15020334,185250786},{70,2485,59640,1088430,16108764,201359550},{71,2556,62196,1150626,17259390,218618940},{72,2628,64824,1215450,18474840,237093780},{73,2701,67525,1282975,19757815,256851595},{74,2775,70300,1353275,21111090,277962685},{75,2850,73150,1426425,22537515,300500200},{76,2926,76076,1502501,24040016,324540216},{77,3003,79079,1581580,25621596,350161812},{78,3081,82160,1663740,27285336,377447148},{79,3160,85320,1749060,29034396,406481544},{80,3240,88560,1837620,30872016,437353560},{81,3321,91881,1929501,32801517,470155077},{82,3403,95284,2024785,34826302,504981379},{83,3486,98770,2123555,36949857,541931236},{84,3570,102340,2225895,39175752,581106988},{85,3655,105995,2331890,41507642,622614630},{86,3741,109736,2441626,43949268,666563898},{87,3828,113564,2555190,46504458,713068356},{88,3916,117480,2672670,49177128,762245484},{89,4005,121485,2794155,51971283,814216767},{90,4095,125580,2919735,54891018,869107785},{91,4186,129766,3049501,57940519,927048304},{92,4278,134044,3183545,61124064,988172368},{93,4371,138415,3321960,64446024,1052618392},{94,4465,142880,3464840,67910864,1120529256},{95,4560,147440,3612280,71523144,1192052400},{96,4656,152096,3764376,75287520,1267339920},{97,4753,156849,3921225,79208745,1346548665},{98,4851,161700,4082925,83291670,1429840335},{99,4950,166650,4249575,87541245,1517381580},{100,5050,171700,4421275,91962520,1609344100},{101,5151,176851,4598126,96560646,1705904746},{102,5253,182104,4780230,101340876,1807245622},{103,5356,187460,4967690,106308566,1913554188},{104,5460,192920,5160610,111469176,2025023364},{105,5565,198485,5359095,116828271,2141851635},{106,5671,204156,5563251,122391522,2264243157},{107,5778,209934,5773185,128164707,2392407864},{108,5886,215820,5989005,134153712,2526561576},{109,5995,221815,6210820,140364532,2666926108},{110,6105,227920,6438740,146803272,2813729380},{111,6216,234136,6672876,153476148,2967205528},{112,6328,240464,6913340,160389488,3127595016},{113,6441,246905,7160245,167549733,3295144749},{114,6555,253460,7413705,174963438,3470108187},{115,6670,260130,7673835,182637273,3652745460},{116,6786,266916,7940751,190578024,3843323484},{117,6903,273819,8214570,198792594,4042116078},{118,7021,280840,8495410,207288004,4249404082},{119,7140,287980,8783390,216071394,4465475476},{120,7260,295240,9078630,225150024,4690625500},{121,7381,302621,9381251,234531275,4925156775},{122,7503,310124,9691375,244222650,5169379425},{123,7626,317750,10009125,254231775,5423611200},{124,7750,325500,10334625,264566400,5688177600},{125,7875,333375,10668000,275234400,5963412000},{126,8001,341376,11009376,286243776,6249655776},{127,8128,349504,11358880,297602656,6547258432},{128,8256,357760,11716640,309319296,6856577728},{129,8385,366145,12082785,321402081,7177979809},{130,8515,374660,12457445,333859526,7511839335},{131,8646,383306,12840751,346700277,7858539612},{132,8778,392084,13232835,359933112,8218472724},{133,8911,400995,13633830,373566942,8592039666},{134,9045,410040,14043870,387610812,8979650478},{135,9180,419220,14463090,402073902,9381724380},{136,9316,428536,14891626,416965528,9798689908},{137,9453,437989,15329615,432295143,10230985051},{138,9591,447580,15777195,448072338,10679057389},{139,9730,457310,16234505,464306843,11143364232},{140,9870,467180,16701685,481008528,11624372760},{141,10011,477191,17178876,498187404,12122560164},{142,10153,487344,17666220,515853624,12638413788},{143,10296,497640,18163860,534017484,13172431272},{144,10440,508080,18671940,552689424,13725120696},{145,10585,518665,19190605,571880029,14297000725},{146,10731,529396,19720001,591600030,14888600755},{147,10878,540274,20260275,611860305,15500461060},{148,11026,551300,20811575,632671880,16133132940},{149,11175,562475,21374050,654045930,16787178870},{150,11325,573800,21947850,675993780,17463172650},{151,11476,585276,22533126,698526906,18161699556},{152,11628,596904,23130030,721656936,18883356492},{153,11781,608685,23738715,745395651,19628752143},{154,11935,620620,24359335,769754986,20398507129},{155,12090,632710,24992045,794747031,21193254160},{156,12246,644956,25637001,820384032,22013638192},{157,12403,657359,26294360,846678392,22860316584},{158,12561,669920,26964280,873642672,23733959256},{159,12720,682640,27646920,901289592,24635248848},{160,12880,695520,28342440,929632032,25564880880},{161,13041,708561,29051001,958683033,26523563913},{162,13203,721764,29772765,988455798,27512019711},{163,13366,735130,30507895,1018963693,28530983404},{164,13530,748660,31256555,1050220248,29581203652},{165,13695,762355,32018910,1082239158,30663442810},{166,13861,776216,32795126,1115034284,31778477094},{167,14028,790244,33585370,1148619654,32927096748},{168,14196,804440,34389810,1183009464,34110106212},{169,14365,818805,35208615,1218218079,35328324291},{170,14535,833340,36041955,1254260034,36582584325},{171,14706,848046,36890001,1291150035,37873734360},{172,14878,862924,37752925,1328902960,39202637320},{173,15051,877975,38630900,1367533860,40570171180},{174,15225,893200,39524100,1407057960,41977229140},{175,15400,908600,40432700,1447490660,43424719800},{176,15576,924176,41356876,1488847536,44913567336},{177,15753,939929,42296805,1531144341,46444711677},{178,15931,955860,43252665,1574397006,48019108683},{179,16110,971970,44224635,1618621641,49637730324},{180,16290,988260,45212895,1663834536,51301564860},{181,16471,1004731,46217626,1710052162,53011617022},{182,16653,1021384,47239010,1757291172,54768908194},{183,16836,1038220,48277230,1805568402,56574476596},{184,17020,1055240,49332470,1854900872,58429377468},{185,17205,1072445,50404915,1905305787,60334683255},{186,17391,1089836,51494751,1956800538,62291483793},{187,17578,1107414,52602165,2009402703,64300886496},{188,17766,1125180,53727345,2063130048,66364016544},{189,17955,1143135,54870480,2118000528,68482017072},{190,18145,1161280,56031760,2174032288,70656049360},{191,18336,1179616,57211376,2231243664,72887293024},{192,18528,1198144,58409520,2289653184,75176946208},{193,18721,1216865,59626385,2349279569,77526225777},{194,18915,1235780,60862165,2410141734,79936367511},{195,19110,1254890,62117055,2472258789,82408626300},{196,19306,1274196,63391251,2535650040,84944276340},{197,19503,1293699,64684950,2600334990,87544611330},{198,19701,1313400,65998350,2666333340,90210944670},{199,19900,1333300,67331650,2733664990,92944609660},{200,20100,1353400,68685050,2802350040,95746959700},{201,20301,1373701,70058751,2872408791,98619368491},{202,20503,1394204,71452955,2943861746,101563230237},{203,20706,1414910,72867865,3016729611,104579959848},{204,20910,1435820,74303685,3091033296,107670993144},{205,21115,1456935,75760620,3166793916,110837787060},{206,21321,1478256,77238876,3244032792,114081819852},{207,21528,1499784,78738660,3322771452,117404591304},{208,21736,1521520,80260180,3403031632,120807622936},{209,21945,1543465,81803645,3484835277,124292458213},{210,22155,1565620,83369265,3568204542,127860662755},{211,22366,1587986,84957251,3653161793,131513824548},{212,22578,1610564,86567815,3739729608,135253554156},{213,22791,1633355,88201170,3827930778,139081484934},{214,23005,1656360,89857530,3917788308,142999273242},{215,23220,1679580,91537110,4009325418,147008598660},{216,23436,1703016,93240126,4102565544,151111164204},{217,23653,1726669,94966795,4197532339,155308696543},{218,23871,1750540,96717335,4294249674,159602946217},{219,24090,1774630,98491965,4392741639,163995687856},{220,24310,1798940,100290905,4493032544,168488720400},{221,24531,1823471,102114376,4595146920,173083867320},{222,24753,1848224,103962600,4699109520,177782976840},{223,24976,1873200,105835800,4804945320,182587922160},{224,25200,1898400,107734200,4912679520,187500601680},{225,25425,1923825,109658025,5022337545,192522939225},{226,25651,1949476,111607501,5133945046,197656884271},{227,25878,1975354,113582855,5247527901,202904412172},{228,26106,2001460,115584315,5363112216,208267524388},{229,26335,2027795,117612110,5480724326,213748248714},{230,26565,2054360,119666470,5600390796,219348639510},{231,26796,2081156,121747626,5722138422,225070777932},{232,27028,2108184,123855810,5845994232,230916772164},{233,27261,2135445,125991255,5971985487,236888757651},{234,27495,2162940,128154195,6100139682,242988897333},{235,27730,2190670,130344865,6230484547,249219381880},{236,27966,2218636,132563501,6363048048,255582429928},{237,28203,2246839,134810340,6497858388,262080288316},{238,28441,2275280,137085620,6634944008,268715232324},{239,28680,2303960,139389580,6774333588,275489565912},{240,28920,2332880,141722460,6916056048,282405621960},{241,29161,2362041,144084501,7060140549,289465762509},{242,29403,2391444,146475945,7206616494,296672379003},{243,29646,2421090,148897035,7355513529,304027892532},{244,29890,2450980,151348015,7506861544,311534754076},{245,30135,2481115,153829130,7660690674,319195444750},{246,30381,2511496,156340626,7817031300,327012476050},{247,30628,2542124,158882750,7975914050,334988390100},{248,30876,2573000,161455750,8137369800,343125759900},{249,31125,2604125,164059875,8301429675,351427189575},{250,31375,2635500,166695375,8468125050,359895314625},{251,31626,2667126,169362501,8637487551,368532802176},{252,31878,2699004,172061505,8809549056,377342351232},{253,32131,2731135,174792640,8984341696,386326692928},{254,32385,2763520,177556160,9161897856,395488590784},{255,32640,2796160,180352320,9342250176,404830840960},{256,32896,2829056,183181376,9525431552,414356272512}}
    ; // TODO: can this be uint instead?

ulong_t NK_MULTISET(setmax_t n, setmax_t k)
{
    if (n <= 0)
        return 0;

    if (k <= 0)
        return 1;

    return factorial[n - 1][k - 1];
}
