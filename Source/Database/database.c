#include "database.h"
const item_t  db_items[DB_LEN] = {
  {{0,0,0,0,0,0,0,0},0,0,0,0,0,0} /*(NULL-ITEM)*/,
  {{0,0,0,0,0,0,0,0},1001,0,325,325,0,0} /*Boots of Speed*/,
  {{0,0,0,0,0,0,0,0},1004,0,180,180,0,0} /*Faerie Charm*/,
  {{0,0,0,0,0,0,0,0},1005,0,390,390,0,0} /*Meki Pendant*/,
  {{0,0,0,0,0,0,0,0},1006,0,180,180,0,0} /*Rejuvenation Bead*/,
  {{0,0,0,0,0,0,0,0},1007,0,435,435,0,0} /*Regrowth Pendant*/,
  {{0,0,0,0,0,0,380,0},1011,0,1000,1000,0,0} /*Giant's Belt*/,
  {{0,15,0,0,0,0,0,0},1018,0,730,730,0,0} /*Cloak of Agility*/,
  {{0,0,0,0,0,0,0,0},1026,0,860,860,0,0} /*Blasting Wand*/,
  {{0,0,0,0,0,0,0,0},1027,0,400,400,0,0} /*Sapphire Crystal*/,
  {{0,0,0,0,0,0,180,0},1028,0,475,475,0,0} /*Ruby Crystal*/,
  {{0,0,0,0,0,0,0,0},1029,0,300,300,0,0} /*Cloth Armor*/,
  {{0,0,0,0,0,0,0,0},1031,0,720,720,0,0} /*Chain Vest*/,
  {{0,0,0,0,0,0,0,0},1033,0,400,400,0,0} /*Null-Magic Mantle*/,
  {{10,0,0,0,0,0,0,0},1036,0,400,400,0,0} /*Long Sword*/,
  {{25,0,0,0,0,0,0,0},1037,0,875,875,0,0} /*Pickaxe*/,
  {{45,0,0,0,0,0,0,0},1038,0,1550,1550,0,0} /*B. F. Sword*/,
  {{0,0,0,0,0,0,0,0},1039,0,300,300,0,0} /*Hunter's Machete*/,
  {{0,0,0,12,0,0,0,0},1042,0,400,400,0,0} /*Dagger*/,
  {{0,0,0,30,0,0,0,0},1043,0,900,900,0,0} /*Recurve Bow*/,
  {{0,8,0,0,0,0,0,0},1051,0,400,400,0,0} /*Brawler's Gloves*/,
  {{0,0,0,0,0,0,0,0},1052,0,435,435,0,0} /*Amplifying Tome*/,
  {{10,0,0,0,0,0,0,0},1053,0,800,400,1,0} /*Vampiric Scepter*/,
  {{0,0,0,0,0,0,100,0},1054,0,440,440,0,0} /*Doran's Shield*/,
  {{10,0,0,0,0,0,80,0},1055,0,475,475,0,0} /*Doran's Blade*/,
  {{0,0,0,0,0,0,60,0},1056,0,400,400,0,0} /*Doran's Ring*/,
  {{0,0,0,0,0,0,0,0},1057,0,720,720,0,0} /*Negatron Cloak*/,
  {{0,0,0,0,0,0,0,0},1058,0,1600,1600,0,0} /*Needlessly Large Rod*/,
  {{20,0,0,0,0,0,0,0},1062,14,950,950,0,0} /*Prospector's Blade*/,
  {{0,0,0,0,0,0,0,0},1063,0,950,950,0,0} /*Prospector's Ring*/,
  {{0,0,0,0,0,0,0,0},1080,0,700,40,3,0} /*Spirit Stone*/,
  {{0,0,0,0,0,0,0,0},2003,0,35,35,0,0} /*Health Potion*/,
  {{0,0,0,0,0,0,0,0},2004,0,35,35,0,0} /*Mana Potion*/,
  {{0,0,0,0,0,0,0,0},2009,0,0,0,0,0} /*Total Biscuit of Rejuvenation*/,
  {{0,0,0,0,0,0,0,0},2037,0,350,350,0,0} /*Elixir of Fortitude*/,
  {{0,0,0,0,0,0,0,0},2038,0,250,250,0,0} /*Elixir of Agility*/,
  {{0,0,0,0,0,0,0,0},2039,0,250,250,0,0} /*Elixir of Brilliance*/,
  {{0,0,0,0,0,0,0,0},2040,0,500,500,0,0} /*Ichor of Rage*/,
  {{0,0,0,0,0,0,0,0},2041,0,345,345,0,0} /*Crystalline Flask*/,
  {{0,0,0,0,0,0,0,0},2042,0,400,400,0,0} /*Oracle's Elixir*/,
  {{0,0,0,0,0,0,0,0},2043,0,125,125,0,0} /*Vision Ward*/,
  {{0,0,0,0,0,0,0,0},2044,0,75,75,0,0} /*Sight Ward*/,
  {{0,0,0,0,0,0,360,0},2045,0,1550,125,2,0} /*Ruby Sightstone*/,
  {{0,0,0,0,0,0,0,0},2047,0,250,250,0,0} /*Oracle's Extract*/,
  {{0,0,0,0,0,0,0,0},2048,0,500,500,0,0} /*Ichor of Illumination*/,
  {{0,0,0,0,0,0,180,0},2049,0,950,475,1,0} /*Sightstone*/,
  {{0,0,0,0,0,0,0,0},2050,0,0,0,0,0} /*Explorer's Ward*/,
  {{0,0,0,0,0,0,180,0},2051,0,1025,370,2,0} /*Guardian's Horn*/,
  {{0,0,0,0,0,0,0,0},2052,0,0,0,0,0} /*Poro-Snax*/,
  {{0,0,0,0,0,0,0,0},3001,0,2560,980,2,0} /*Abyssal Scepter*/,
  {{0,0,0,0,0,0,0,0},3003,0,2700,1140,2,0} /*Archangel's Staff*/,
  {{20,0,0,0,0,0,0,0},3004,0,2100,1000,2,0} /*Manamune*/,
  {{0,15,0,0,0,0,0,0},3005,1,2300,780,2,0} /*Atma's Impaler*/,
  {{0,0,0,20,0,0,0,0},3006,0,900,175,2,0} /*Berserker's Greaves*/,
  {{0,0,0,0,0,0,0,0},3007,0,2700,1140,2,0} /*Archangel's Staff (Crystal Scar)*/,
  {{20,0,0,0,0,0,0,0},3008,0,2100,1000,2,0} /*Manamune (Crystal Scar)*/,
  {{0,0,0,0,0,0,0,0},3009,0,1000,675,1,0} /*Boots of Swiftness*/,
  {{0,0,0,0,0,0,200,0},3010,0,1200,325,2,0} /*Catalyst the Protector*/,
  {{0,0,0,0,0,0,0,0},3020,0,1100,775,1,0} /*Sorcerer's Shoes*/,
  {{30,0,0,0,0,0,700,0},3022,13,3300,835,2,0} /*Frozen Mallet*/,
  {{0,0,0,0,0,0,0,0},3023,0,1900,735,2,0} /*Twin Shadows*/,
  {{0,0,0,0,0,0,0,0},3024,0,1350,230,2,0} /*Glacial Shroud*/,
  {{0,0,0,0,0,0,0,0},3025,0,3250,700,2,0} /*Iceborn Gauntlet*/,
  {{0,0,0,0,0,0,0,0},3026,0,2750,1310,2,0} /*Guardian Angel*/,
  {{0,0,0,0,0,0,450,0},3027,0,2800,740,2,0} /*Rod of Ages*/,
  {{0,0,0,0,0,0,0,0},3028,0,880,120,3,0} /*Chalice of Harmony*/,
  {{0,0,0,0,0,0,450,0},3029,0,2800,740,2,0} /*Rod of Ages (Crystal Scar)*/,
  {{70,25,0,0,0,0,0,0},3031,2,3800,645,3,0} /*Infinity Edge*/,
  {{40,0,0,0,0,0,0,0},3035,3,2300,1025,2,0} /*Last Whisper*/,
  {{0,0,0,0,0,0,0,0},3037,0,300,120,1,0} /*Mana Manipulator*/,
  {{0,0,0,0,0,0,0,0},3040,0,2700,2700,0,0} /*Seraph's Embrace*/,
  {{0,0,0,0,0,0,0,0},3041,0,1235,800,1,0} /*Mejai's Soulstealer*/,
  {{20,0,0,0,0,0,0,0},3042,0,2100,2100,0,0} /*Muramana*/,
  {{20,0,0,0,0,0,200,0},3044,15,1465,590,2,0} /*Phage*/,
  {{0,30,0,50,0,0,0,0},3046,0,2800,495,3,0} /*Phantom Dancer*/,
  {{0,0,0,0,0,0,0,0},3047,0,1000,375,2,0} /*Ninja Tabi*/,
  {{0,0,0,0,0,0,250,0},3050,0,2550,900,2,0} /*Zeke's Herald*/,
  {{0,0,0,0,0,0,350,0},3056,0,2835,800,3,0} /*Ohmwrecker*/,
  {{0,0,0,0,0,0,0,0},3057,0,1200,365,2,0} /*Sheen*/,
  {{0,0,0,0,0,0,0,0},3060,0,2360,890,2,0} /*Banner of Command*/,
  {{0,0,0,0,0,0,400,0},3065,0,2625,375,2,0} /*Spirit Visage*/,
  {{0,0,0,0,0,0,200,0},3067,0,850,375,1,0} /*Kindlegem*/,
  {{0,0,0,0,0,0,450,0},3068,0,2650,930,2,0} /*Sunfire Cape*/,
  {{0,0,0,0,0,0,250,0},3069,0,2100,550,2,0} /*Shurelya's Reverie*/,
  {{0,0,0,0,0,0,0,0},3070,0,700,120,2,0} /*Tear of the Goddess*/,
  {{50,0,0,0,0,0,200,0},3071,4,3000,1188,2,0} /*The Black Cleaver*/,
  {{100,0,0,0,0,0,0,0},3072,0,3200,850,2,0} /*The Bloodthirster*/,
  {{0,0,0,0,0,0,0,0},3073,0,700,120,2,0} /*Tear of the Goddess (Crystal Scar)*/,
  {{75,0,0,0,0,0,0,0},3074,11,3300,600,2,0} /*Ravenous Hydra (Melee Only)*/,
  {{0,0,0,0,0,0,0,0},3075,0,2200,1180,2,0} /*Thornmail*/,
  {{40,0,0,0,0,0,0,0},3077,11,1900,265,4,0} /*Tiamat (Melee Only)*/,
  {{30,10,0,30,0,0,250,0},3078,0,3843,3,3,0} /*Trinity Force*/,
  {{0,0,0,0,0,0,0,0},3082,0,1000,400,2,0} /*Warden's Mail*/,
  {{0,0,0,0,0,0,1000,0},3083,0,2830,995,4,0} /*Warmog's Armor*/,
  {{0,0,0,0,0,0,850,0},3084,0,2455,980,2,0} /*Overlord's Bloodmail*/,
  {{0,0,0,70,0,0,0,0},3085,0,2400,700,3,0} /*Runaan's Hurricane (Ranged Only)*/,
  {{0,10,0,18,0,0,0,0},3086,0,1175,375,2,0} /*Zeal*/,
  {{0,20,0,40,0,0,0,0},3087,0,2500,525,2,0} /*Statikk Shiv*/,
  {{0,0,0,0,0,0,0,0},3089,0,3300,840,2,0} /*Rabadon's Deathcap*/,
  {{0,0,0,0,0,0,0,0},3090,0,3500,1045,3,0} /*Wooglet's Witchcap*/,
  {{0,0,0,42,0,0,0,0},3091,0,2400,700,3,0} /*Wit's End*/,
  {{0,0,0,0,0,0,0,0},3092,0,1600,535,2,0} /*Shard of True Ice*/,
  {{0,10,0,0,0,0,0,0},3093,0,800,400,1,0} /*Avarice Blade*/,
  {{0,0,0,0,0,0,0,0},3096,0,700,340,2,0} /*Philosopher's Stone*/,
  {{0,0,0,0,0,0,0,0},3097,0,650,170,2,0} /*Emblem of Valor*/,
  {{0,0,0,0,0,0,0,0},3098,0,765,330,1,0} /*Kage's Lucky Pick*/,
  {{0,0,0,0,0,0,520,0},3099,0,2110,485,3,0} /*Soul Shroud*/,
  {{0,0,0,0,0,0,0,0},3100,0,3000,940,2,0} /*Lich Bane*/,
  {{0,0,0,40,0,0,0,0},3101,5,1250,450,2,0} /*Stinger*/,
  {{0,0,0,0,0,0,450,0},3102,0,2750,875,2,0} /*Banshee's Veil*/,
  {{40,0,0,0,0,0,300,0},3104,0,2962,800,2,0} /*Lord Van Damm's Pillager*/,
  {{0,0,0,0,0,0,200,0},3105,0,1900,375,3,0} /*Aegis of the Legion*/,
  {{0,0,0,0,0,0,0,0},3106,0,700,100,2,0} /*Madred's Razors*/,
  {{0,0,0,0,0,0,300,0},3107,0,400,400,0,0} /*Runic Bulwark*/,
  {{0,0,0,0,0,0,0,0},3108,0,820,385,1,0} /*Fiendish Codex*/,
  {{0,0,0,0,0,0,0,0},3109,0,2080,1000,3,0} /*Force of Nature*/,
  {{0,0,0,0,0,0,0,0},3110,0,2900,550,2,0} /*Frozen Heart*/,
  {{0,0,0,0,0,0,0,0},3111,0,1200,475,2,0} /*Mercury's Treads*/,
  {{0,0,0,0,0,0,0,0},3112,0,2080,1000,3,0} /*Orb of Winter*/,
  {{0,0,0,50,0,0,0,0},3114,0,1785,550,3,0} /*Malady*/,
  {{0,0,0,50,0,0,0,0},3115,0,2920,850,2,0} /*Nashor's Tooth*/,
  {{0,0,0,0,0,0,500,0},3116,0,2900,605,3,0} /*Rylai's Crystal Scepter*/,
  {{0,0,0,0,0,0,0,0},3117,0,1000,675,1,0} /*Boots of Mobility*/,
  {{20,18,0,0,0,0,0,0},3122,0,1840,710,2,0} /*Wicked Hatchet*/,
  {{25,20,0,0,0,0,0,0},3123,12,1900,700,2,0} /*Executioner's Calling*/,
  {{30,0,0,0,0,0,0,0},3124,0,2600,865,2,0} /*Guinsoo's Rageblade*/,
  {{40,0,0,30,0,0,0,0},3126,0,3250,775,3,0} /*Madred's Bloodrazor*/,
  {{0,0,0,0,0,0,0,0},3128,0,3100,680,2,0} /*Deathfire Grasp*/,
  {{0,0,0,0,0,0,0,0},3131,0,2150,850,2,0} /*Sword of the Divine*/,
  {{0,0,0,0,0,0,325,0},3132,0,825,350,1,0} /*Heart of Gold*/,
  {{25,0,0,0,0,0,0,0},3134,6,1337,537,2,0} /*The Brutalizer*/,
  {{0,0,0,0,0,0,0,0},3135,0,2295,1000,2,0} /*Void Staff*/,
  {{0,0,0,0,0,0,200,0},3136,0,1485,575,2,0} /*Haunting Guise*/,
  {{0,0,0,0,0,0,180,0},3138,0,1275,800,1,0} /*Leviathan*/,
  {{60,0,0,0,0,0,0,0},3139,0,3700,600,2,0} /*Mercurial Scimitar*/,
  {{0,0,0,0,0,0,0,0},3140,0,1550,830,1,0} /*Quicksilver Sash*/,
  {{10,0,0,0,0,0,0,0},3141,7,1200,800,1,0} /*Sword of the Occult*/,
  {{30,15,0,0,0,0,0,0},3142,8,2700,563,2,0} /*Youmuu's Ghostblade*/,
  {{0,0,0,0,0,0,500,0},3143,0,3000,1000,2,0} /*Randuin's Omen*/,
  {{25,0,0,0,0,0,0,0},3144,0,1400,200,2,0} /*Bilgewater Cutlass*/,
  {{0,0,0,0,0,0,0,0},3145,0,1200,330,2,0} /*Hextech Revolver*/,
  {{45,0,0,0,0,0,0,0},3146,0,3400,800,2,0} /*Hextech Gunblade*/,
  {{0,0,0,0,0,0,300,0},3151,0,2900,980,2,0} /*Liandry's Torment*/,
  {{0,0,0,0,0,0,0,0},3152,0,2550,585,2,0} /*Will of the Ancients*/,
  {{25,0,0,40,0,0,0,0},3153,0,3200,1000,3,0} /*Blade of the Ruined King*/,
  {{25,0,0,0,0,0,0,0},3154,0,2000,500,2,0} /*Wriggle's Lantern*/,
  {{25,0,0,0,0,0,0,0},3155,0,1350,550,2,0} /*Hexdrinker*/,
  {{60,0,0,0,0,0,0,0},3156,0,3200,975,2,0} /*Maw of Malmortius*/,
  {{0,0,0,0,0,0,0,0},3157,0,3260,500,2,0} /*Zhonya's Hourglass*/,
  {{0,0,0,0,0,0,0,0},3158,0,1000,675,1,0} /*Ionian Boots of Lucidity*/,
  {{20,0,0,0,0,0,0,0},3159,0,1250,150,2,0} /*Grez's Spectral Lantern*/,
  {{0,0,0,0,0,0,0,0},3165,0,2200,435,3,0} /*Morellonomicon*/,
  {{5,0,0,0,0,0,0,0},3166,9,800,800,0,0} /*Bonetooth Necklace*/,
  {{5,0,0,0,0,0,0,0},3167,0,800,800,0,0} /*Bonetooth Necklace*/,
  {{5,0,0,0,0,0,0,0},3168,0,800,800,0,0} /*Bonetooth Necklace*/,
  {{5,0,0,0,0,0,0,0},3169,0,800,800,0,0} /*Bonetooth Necklace*/,
  {{0,0,0,0,0,0,0,0},3170,0,2300,420,2,0} /*Moonflair Spellblade (Ranged Only)*/,
  {{5,0,0,0,0,0,0,0},3171,0,800,800,0,0} /*Bonetooth Necklace*/,
  {{25,0,0,50,0,0,0,0},3172,0,2850,725,2,0} /*Zephyr*/,
  {{0,0,0,0,0,0,0,0},3173,0,1100,400,1,0} /*Eleisa's Miracle*/,
  {{0,0,0,0,0,0,0,0},3174,0,2600,900,2,0} /*Athene's Unholy Grail*/,
  {{5,0,0,0,0,0,0,0},3175,0,800,800,0,0} /*Head of Kha'Zix*/,
  {{0,0,0,0,0,0,400,0},3176,0,2175,500,2,0} /*Arcane Helix*/,
  {{0,0,0,40,0,0,250,0},3178,0,1950,575,2,0} /*Ionic Spark*/,
  {{0,0,0,0,0,0,350,0},3180,0,2520,600,2,0} /*Odyn's Veil*/,
  {{65,0,0,0,0,0,0,0},3181,0,2850,500,2,0} /*Sanguine Blade*/,
  {{0,0,0,0,0,0,0,0},3183,0,10000359,9999999,2,0} /*Priscilla's Blessing*/,
  {{70,0,0,0,0,0,275,0},3184,15,3615,600,2,0} /*Entropy*/,
  {{50,0,0,0,0,0,0,0},3185,0,2425,300,2,0} /*The Lightbringer*/,
  {{30,0,0,40,0,0,0,0},3186,0,2475,700,2,0} /*Kitae's Bloodrazor*/,
  {{0,0,0,0,0,0,300,0},3187,0,1920,200,3,0} /*Hextech Sweeper*/,
  {{0,0,0,0,0,0,0,0},3188,0,2400,720,2,0} /*Blackfire Torch*/,
  {{0,0,0,0,0,0,300,0},3190,0,2500,600,1,0} /*Locket of the Iron Solari*/,
  {{0,0,0,0,0,0,0,0},3191,0,1160,125,3,0} /*Seeker's Armguard*/,
  {{0,0,0,0,0,0,220,0},3196,0,1000,1000,1,0} /*Augment: Power*/,
  {{0,0,0,0,0,0,0,0},3197,0,1000,1000,1,0} /*Augment: Gravity*/,
  {{0,0,0,0,0,0,0,0},3198,0,1000,1000,1,0} /*Augment: Death*/,
  {{0,0,0,0,0,0,0,0},3200,0,0,0,0,0} /*The Hex Core*/,
  {{0,0,0,0,0,0,0,0},3206,0,2000,100,2,0} /*Spirit of the Spectral Wraith*/,
  {{0,0,0,0,0,0,500,0},3207,0,2000,450,2,0} /*Spirit of the Ancient Golem*/,
  {{35,0,0,0,0,0,0,0},3209,0,2000,500,3,0} /*Spirit of the Elder Lizard*/,
  {{0,0,0,0,0,0,200,0},3211,0,1400,205,2,0} /*Spectre's Cowl*/,
  {{0,0,0,0,0,0,0,0},3222,0,2500,920,2,0} /*Mikael's Crucible*/,
  {{0,0,0,20,0,0,0,0},3250,0,1375,475,1,0} /*Enchantment: Homeguard*/,
  {{0,0,0,20,0,0,0,0},3251,0,1650,750,1,0} /*Enchantment: Captain*/,
  {{0,0,0,20,0,0,0,0},3252,0,1550,650,1,0} /*Enchantment: Furor*/,
  {{0,0,0,20,0,0,0,0},3253,0,1375,475,1,0} /*Enchantment: Distortion*/,
  {{0,0,0,20,0,0,0,0},3254,0,1375,475,1,0} /*Enchantment: Alacrity*/,
  {{0,0,0,0,0,0,0,0},3255,0,1575,475,1,0} /*Enchantment: Homeguard*/,
  {{0,0,0,0,0,0,0,0},3256,0,1850,750,1,0} /*Enchantment: Captain*/,
  {{0,0,0,0,0,0,0,0},3257,0,1750,650,1,0} /*Enchantment: Furor*/,
  {{0,0,0,0,0,0,0,0},3258,0,1575,475,1,0} /*Enchantment: Distortion*/,
  {{0,0,0,0,0,0,0,0},3259,0,1575,475,1,0} /*Enchantment: Alacrity*/,
  {{0,0,0,0,0,0,0,0},3260,0,1475,475,1,0} /*Enchantment: Homeguard*/,
  {{0,0,0,0,0,0,0,0},3261,0,1750,750,1,0} /*Enchantment: Captain*/,
  {{0,0,0,0,0,0,0,0},3262,0,1650,650,1,0} /*Enchantment: Furor*/,
  {{0,0,0,0,0,0,0,0},3263,0,1475,475,1,0} /*Enchantment: Distortion*/,
  {{0,0,0,0,0,0,0,0},3264,0,1475,475,1,0} /*Enchantment: Alacrity*/,
  {{0,0,0,0,0,0,0,0},3265,0,1675,475,1,0} /*Enchantment: Homeguard*/,
  {{0,0,0,0,0,0,0,0},3266,0,1950,750,1,0} /*Enchantment: Captain*/,
  {{0,0,0,0,0,0,0,0},3267,0,1850,650,1,0} /*Enchantment: Furor*/,
  {{0,0,0,0,0,0,0,0},3268,0,1675,475,1,0} /*Enchantment: Distortion*/,
  {{0,0,0,0,0,0,0,0},3269,0,1675,475,1,0} /*Enchantment: Alacrity*/,
  {{0,0,0,0,0,0,0,0},3270,0,1475,475,1,0} /*Enchantment: Homeguard*/,
  {{0,0,0,0,0,0,0,0},3271,0,1750,750,1,0} /*Enchantment: Captain*/,
  {{0,0,0,0,0,0,0,0},3272,0,1650,650,1,0} /*Enchantment: Furor*/,
  {{0,0,0,0,0,0,0,0},3273,0,1475,475,1,0} /*Enchantment: Distortion*/,
  {{0,0,0,0,0,0,0,0},3274,0,1475,475,1,0} /*Enchantment: Alacrity*/,
  {{0,0,0,0,0,0,0,0},3275,0,1475,475,1,0} /*Enchantment: Homeguard*/,
  {{0,0,0,0,0,0,0,0},3276,0,1750,750,1,0} /*Enchantment: Captain*/,
  {{0,0,0,0,0,0,0,0},3277,0,1650,650,1,0} /*Enchantment: Furor*/,
  {{0,0,0,0,0,0,0,0},3278,0,1475,475,1,0} /*Enchantment: Distortion*/,
  {{0,0,0,0,0,0,0,0},3279,0,1475,475,1,0} /*Enchantment: Alacrity*/,
  {{0,0,0,0,0,0,0,0},3280,0,1475,475,1,0} /*Enchantment: Homeguard*/,
  {{0,0,0,0,0,0,0,0},3281,0,1750,750,1,0} /*Enchantment: Captain*/,
  {{0,0,0,0,0,0,0,0},3282,0,1650,650,1,0} /*Enchantment: Furor*/,
  {{0,0,0,0,0,0,0,0},3283,0,1475,475,1,0} /*Enchantment: Distortion*/,
  {{0,0,0,0,0,0,0,0},3284,0,1475,475,1,0} /*Enchantment: Alacrity*/
};

const stats_t db_passives[DB_LEN] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,10,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,10,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,20,0,0,0},
  {36,0,0,0,10,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,200,0},
  {0,0,0,0,0,0,0,0}
};

const char *db_names[DB_LEN] = {
"(NULL-ITEM)",
"Boots of Speed",
"Faerie Charm",
"Meki Pendant",
"Rejuvenation Bead",
"Regrowth Pendant",
"Giant's Belt",
"Cloak of Agility",
"Blasting Wand",
"Sapphire Crystal",
"Ruby Crystal",
"Cloth Armor",
"Chain Vest",
"Null-Magic Mantle",
"Long Sword",
"Pickaxe",
"B. F. Sword",
"Hunter's Machete",
"Dagger",
"Recurve Bow",
"Brawler's Gloves",
"Amplifying Tome",
"Vampiric Scepter",
"Doran's Shield",
"Doran's Blade",
"Doran's Ring",
"Negatron Cloak",
"Needlessly Large Rod",
"Prospector's Blade",
"Prospector's Ring",
"Spirit Stone",
"Health Potion",
"Mana Potion",
"Total Biscuit of Rejuvenation",
"Elixir of Fortitude",
"Elixir of Agility",
"Elixir of Brilliance",
"Ichor of Rage",
"Crystalline Flask",
"Oracle's Elixir",
"Vision Ward",
"Sight Ward",
"Ruby Sightstone",
"Oracle's Extract",
"Ichor of Illumination",
"Sightstone",
"Explorer's Ward",
"Guardian's Horn",
"Poro-Snax",
"Abyssal Scepter",
"Archangel's Staff",
"Manamune",
"Atma's Impaler",
"Berserker's Greaves",
"Archangel's Staff (Crystal Scar)",
"Manamune (Crystal Scar)",
"Boots of Swiftness",
"Catalyst the Protector",
"Sorcerer's Shoes",
"Frozen Mallet",
"Twin Shadows",
"Glacial Shroud",
"Iceborn Gauntlet",
"Guardian Angel",
"Rod of Ages",
"Chalice of Harmony",
"Rod of Ages (Crystal Scar)",
"Infinity Edge",
"Last Whisper",
"Mana Manipulator",
"Seraph's Embrace",
"Mejai's Soulstealer",
"Muramana",
"Phage",
"Phantom Dancer",
"Ninja Tabi",
"Zeke's Herald",
"Ohmwrecker",
"Sheen",
"Banner of Command",
"Spirit Visage",
"Kindlegem",
"Sunfire Cape",
"Shurelya's Reverie",
"Tear of the Goddess",
"The Black Cleaver",
"The Bloodthirster",
"Tear of the Goddess (Crystal Scar)",
"Ravenous Hydra (Melee Only)",
"Thornmail",
"Tiamat (Melee Only)",
"Trinity Force",
"Warden's Mail",
"Warmog's Armor",
"Overlord's Bloodmail",
"Runaan's Hurricane (Ranged Only)",
"Zeal",
"Statikk Shiv",
"Rabadon's Deathcap",
"Wooglet's Witchcap",
"Wit's End",
"Shard of True Ice",
"Avarice Blade",
"Philosopher's Stone",
"Emblem of Valor",
"Kage's Lucky Pick",
"Soul Shroud",
"Lich Bane",
"Stinger",
"Banshee's Veil",
"Lord Van Damm's Pillager",
"Aegis of the Legion",
"Madred's Razors",
"Runic Bulwark",
"Fiendish Codex",
"Force of Nature",
"Frozen Heart",
"Mercury's Treads",
"Orb of Winter",
"Malady",
"Nashor's Tooth",
"Rylai's Crystal Scepter",
"Boots of Mobility",
"Wicked Hatchet",
"Executioner's Calling",
"Guinsoo's Rageblade",
"Madred's Bloodrazor",
"Deathfire Grasp",
"Sword of the Divine",
"Heart of Gold",
"The Brutalizer",
"Void Staff",
"Haunting Guise",
"Leviathan",
"Mercurial Scimitar",
"Quicksilver Sash",
"Sword of the Occult",
"Youmuu's Ghostblade",
"Randuin's Omen",
"Bilgewater Cutlass",
"Hextech Revolver",
"Hextech Gunblade",
"Liandry's Torment",
"Will of the Ancients",
"Blade of the Ruined King",
"Wriggle's Lantern",
"Hexdrinker",
"Maw of Malmortius",
"Zhonya's Hourglass",
"Ionian Boots of Lucidity",
"Grez's Spectral Lantern",
"Morellonomicon",
"Bonetooth Necklace",
"Bonetooth Necklace",
"Bonetooth Necklace",
"Bonetooth Necklace",
"Moonflair Spellblade (Ranged Only)",
"Bonetooth Necklace",
"Zephyr",
"Eleisa's Miracle",
"Athene's Unholy Grail",
"Head of Kha'Zix",
"Arcane Helix",
"Ionic Spark",
"Odyn's Veil",
"Sanguine Blade",
"Priscilla's Blessing",
"Entropy",
"The Lightbringer",
"Kitae's Bloodrazor",
"Hextech Sweeper",
"Blackfire Torch",
"Locket of the Iron Solari",
"Seeker's Armguard",
"Augment: Power",
"Augment: Gravity",
"Augment: Death",
"The Hex Core",
"Spirit of the Spectral Wraith",
"Spirit of the Ancient Golem",
"Spirit of the Elder Lizard",
"Spectre's Cowl",
"Mikael's Crucible",
"Enchantment: Homeguard",
"Enchantment: Captain",
"Enchantment: Furor",
"Enchantment: Distortion",
"Enchantment: Alacrity",
"Enchantment: Homeguard",
"Enchantment: Captain",
"Enchantment: Furor",
"Enchantment: Distortion",
"Enchantment: Alacrity",
"Enchantment: Homeguard",
"Enchantment: Captain",
"Enchantment: Furor",
"Enchantment: Distortion",
"Enchantment: Alacrity",
"Enchantment: Homeguard",
"Enchantment: Captain",
"Enchantment: Furor",
"Enchantment: Distortion",
"Enchantment: Alacrity",
"Enchantment: Homeguard",
"Enchantment: Captain",
"Enchantment: Furor",
"Enchantment: Distortion",
"Enchantment: Alacrity",
"Enchantment: Homeguard",
"Enchantment: Captain",
"Enchantment: Furor",
"Enchantment: Distortion",
"Enchantment: Alacrity",
"Enchantment: Homeguard",
"Enchantment: Captain",
"Enchantment: Furor",
"Enchantment: Distortion",
"Enchantment: Alacrity"
};

const cl_short db_buildtree[DB_LEN][BUILDTREE_WIDTH] = {
{0} /*(NULL-ITEM)*/,
{0} /*Boots of Speed*/,
{0} /*Faerie Charm*/,
{0} /*Meki Pendant*/,
{0} /*Rejuvenation Bead*/,
{0} /*Regrowth Pendant*/,
{0} /*Giant's Belt*/,
{0} /*Cloak of Agility*/,
{0} /*Blasting Wand*/,
{0} /*Sapphire Crystal*/,
{0} /*Ruby Crystal*/,
{0} /*Cloth Armor*/,
{0} /*Chain Vest*/,
{0} /*Null-Magic Mantle*/,
{0} /*Long Sword*/,
{0} /*Pickaxe*/,
{0} /*B. F. Sword*/,
{0} /*Hunter's Machete*/,
{0} /*Dagger*/,
{0} /*Recurve Bow*/,
{0} /*Brawler's Gloves*/,
{0} /*Amplifying Tome*/,
{14} /*Vampiric Scepter*/,
{0} /*Doran's Shield*/,
{0} /*Doran's Blade*/,
{0} /*Doran's Ring*/,
{0} /*Negatron Cloak*/,
{0} /*Needlessly Large Rod*/,
{0} /*Prospector's Blade*/,
{0} /*Prospector's Ring*/,
{17, 2, 4} /*Spirit Stone*/,
{0} /*Health Potion*/,
{0} /*Mana Potion*/,
{0} /*Total Biscuit of Rejuvenation*/,
{0} /*Elixir of Fortitude*/,
{0} /*Elixir of Agility*/,
{0} /*Elixir of Brilliance*/,
{0} /*Ichor of Rage*/,
{0} /*Crystalline Flask*/,
{0} /*Oracle's Elixir*/,
{0} /*Vision Ward*/,
{0} /*Sight Ward*/,
{45, 10} /*Ruby Sightstone*/,
{0} /*Oracle's Extract*/,
{0} /*Ichor of Illumination*/,
{10} /*Sightstone*/,
{0} /*Explorer's Ward*/,
{4, 10} /*Guardian's Horn*/,
{0} /*Poro-Snax*/,
{8, 26} /*Abyssal Scepter*/,
{84, 8} /*Archangel's Staff*/,
{84, 14} /*Manamune*/,
{12, 102} /*Atma's Impaler*/,
{1, 18} /*Berserker's Greaves*/,
{87, 8} /*Archangel's Staff (Crystal Scar)*/,
{87, 14} /*Manamune (Crystal Scar)*/,
{1} /*Boots of Swiftness*/,
{10, 9} /*Catalyst the Protector*/,
{1} /*Sorcerer's Shoes*/,
{73, 6} /*Frozen Mallet*/,
{105, 13} /*Twin Shadows*/,
{9, 12} /*Glacial Shroud*/,
{78, 61} /*Iceborn Gauntlet*/,
{26, 12} /*Guardian Angel*/,
{57, 8} /*Rod of Ages*/,
{2, 13, 2} /*Chalice of Harmony*/,
{57, 8} /*Rod of Ages (Crystal Scar)*/,
{16, 15, 7} /*Infinity Edge*/,
{15, 14} /*Last Whisper*/,
{2} /*Mana Manipulator*/,
{0} /*Seraph's Embrace*/,
{21} /*Mejai's Soulstealer*/,
{0} /*Muramana*/,
{10, 14} /*Phage*/,
{7, 96, 18} /*Phantom Dancer*/,
{1, 11} /*Ninja Tabi*/,
{81, 22} /*Zeke's Herald*/,
{10, 103, 8} /*Ohmwrecker*/,
{9, 21} /*Sheen*/,
{114, 104} /*Banner of Command*/,
{181, 81} /*Spirit Visage*/,
{10} /*Kindlegem*/,
{12, 6} /*Sunfire Cape*/,
{81, 103} /*Shurelya's Reverie*/,
{9, 2} /*Tear of the Goddess*/,
{130, 10} /*The Black Cleaver*/,
{16, 22} /*The Bloodthirster*/,
{9, 2} /*Tear of the Goddess (Crystal Scar)*/,
{90, 22} /*Ravenous Hydra (Melee Only)*/,
{11, 12} /*Thornmail*/,
{15, 14, 4, 4} /*Tiamat (Melee Only)*/,
{96, 78, 73} /*Trinity Force*/,
{11, 11} /*Warden's Mail*/,
{6, 10, 4, 4} /*Warmog's Armor*/,
{6, 10} /*Overlord's Bloodmail*/,
{18, 19, 18} /*Runaan's Hurricane (Ranged Only)*/,
{20, 18} /*Zeal*/,
{96, 102} /*Statikk Shiv*/,
{8, 27} /*Rabadon's Deathcap*/,
{173, 8, 21} /*Wooglet's Witchcap*/,
{19, 13, 18} /*Wit's End*/,
{105, 69} /*Shard of True Ice*/,
{20} /*Avarice Blade*/,
{2, 4} /*Philosopher's Stone*/,
{11, 4} /*Emblem of Valor*/,
{21} /*Kage's Lucky Pick*/,
{81, 10, 69} /*Soul Shroud*/,
{78, 8} /*Lich Bane*/,
{18, 18} /*Stinger*/,
{181, 10} /*Banshee's Veil*/,
{129, 130} /*Lord Van Damm's Pillager*/,
{104, 10, 13} /*Aegis of the Legion*/,
{11, 17} /*Madred's Razors*/,
{0} /*Runic Bulwark*/,
{21} /*Fiendish Codex*/,
{4, 4, 26} /*Force of Nature*/,
{92, 61} /*Frozen Heart*/,
{1, 13} /*Mercury's Treads*/,
{4, 4, 26} /*Orb of Winter*/,
{18, 18, 21} /*Malady*/,
{108, 114} /*Nashor's Tooth*/,
{8, 21, 6} /*Rylai's Crystal Scepter*/,
{1} /*Boots of Mobility*/,
{7, 14} /*Wicked Hatchet*/,
{102, 14} /*Executioner's Calling*/,
{8, 15} /*Guinsoo's Rageblade*/,
{112, 15, 19} /*Madred's Bloodrazor*/,
{27, 114} /*Deathfire Grasp*/,
{19, 18} /*Sword of the Divine*/,
{10} /*Heart of Gold*/,
{14, 14} /*The Brutalizer*/,
{8, 21} /*Void Staff*/,
{10, 21} /*Haunting Guise*/,
{10} /*Leviathan*/,
{135, 16} /*Mercurial Scimitar*/,
{26} /*Quicksilver Sash*/,
{14} /*Sword of the Occult*/,
{102, 130} /*Youmuu's Ghostblade*/,
{6, 92} /*Randuin's Omen*/,
{14, 22} /*Bilgewater Cutlass*/,
{21, 21} /*Hextech Revolver*/,
{139, 140} /*Hextech Gunblade*/,
{132, 21} /*Liandry's Torment*/,
{105, 140} /*Will of the Ancients*/,
{18, 139, 18} /*Blade of the Ruined King*/,
{112, 22} /*Wriggle's Lantern*/,
{14, 13} /*Hexdrinker*/,
{146, 15} /*Maw of Malmortius*/,
{173, 27} /*Zhonya's Hourglass*/,
{1} /*Ionian Boots of Lucidity*/,
{11, 22} /*Grez's Spectral Lantern*/,
{105, 2, 114} /*Morellonomicon*/,
{0} /*Bonetooth Necklace*/,
{0} /*Bonetooth Necklace*/,
{0} /*Bonetooth Necklace*/,
{0} /*Bonetooth Necklace*/,
{173, 26} /*Moonflair Spellblade (Ranged Only)*/,
{0} /*Bonetooth Necklace*/,
{108, 15} /*Zephyr*/,
{103} /*Eleisa's Miracle*/,
{114, 65} /*Athene's Unholy Grail*/,
{0} /*Head of Kha'Zix*/,
{57, 10} /*Arcane Helix*/,
{19, 10} /*Ionic Spark*/,
{26, 57} /*Odyn's Veil*/,
{16, 22} /*Sanguine Blade*/,
{4, 4} /*Priscilla's Blessing*/,
{73, 16} /*Entropy*/,
{150, 15} /*The Lightbringer*/,
{15, 19} /*Kitae's Bloodrazor*/,
{21, 21, 81} /*Hextech Sweeper*/,
{8, 114} /*Blackfire Torch*/,
{111} /*Locket of the Iron Solari*/,
{11, 21, 11} /*Seeker's Armguard*/,
{177} /*Augment: Power*/,
{177} /*Augment: Gravity*/,
{177} /*Augment: Death*/,
{0} /*The Hex Core*/,
{30, 140} /*Spirit of the Spectral Wraith*/,
{30, 81} /*Spirit of the Ancient Golem*/,
{30, 14, 14} /*Spirit of the Elder Lizard*/,
{10, 26} /*Spectre's Cowl*/,
{103, 65} /*Mikael's Crucible*/,
{53} /*Enchantment: Homeguard*/,
{53} /*Enchantment: Captain*/,
{53} /*Enchantment: Furor*/,
{53} /*Enchantment: Distortion*/,
{53} /*Enchantment: Alacrity*/,
{58} /*Enchantment: Homeguard*/,
{58} /*Enchantment: Captain*/,
{58} /*Enchantment: Furor*/,
{58} /*Enchantment: Distortion*/,
{58} /*Enchantment: Alacrity*/,
{75} /*Enchantment: Homeguard*/,
{75} /*Enchantment: Captain*/,
{75} /*Enchantment: Furor*/,
{75} /*Enchantment: Distortion*/,
{75} /*Enchantment: Alacrity*/,
{117} /*Enchantment: Homeguard*/,
{117} /*Enchantment: Captain*/,
{117} /*Enchantment: Furor*/,
{117} /*Enchantment: Distortion*/,
{117} /*Enchantment: Alacrity*/,
{122} /*Enchantment: Homeguard*/,
{122} /*Enchantment: Captain*/,
{122} /*Enchantment: Furor*/,
{122} /*Enchantment: Distortion*/,
{122} /*Enchantment: Alacrity*/,
{149} /*Enchantment: Homeguard*/,
{149} /*Enchantment: Captain*/,
{149} /*Enchantment: Furor*/,
{149} /*Enchantment: Distortion*/,
{149} /*Enchantment: Alacrity*/,
{56} /*Enchantment: Homeguard*/,
{56} /*Enchantment: Captain*/,
{56} /*Enchantment: Furor*/,
{56} /*Enchantment: Distortion*/,
{56} /*Enchantment: Alacrity*/
};
