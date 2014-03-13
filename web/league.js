// JavaScript Document

function itemMergeAllStats(a,b)
{
	var stats1 = NICOLA.merge(a['stats'], b['stats']);
	return stats1;
	//var stats2 = NICOLA.merge(a['passiv_stats'], b['passiv_stats']);
	//return NICOLA.merge(stats1,stats2);
}

function league_mitigation(ar)
{
	return (100/(100+ar));
}
function league_ehp(stats)
{	
	return [
		stats['HP'] * (1/league_mitigation(stats['Armor'])),
		stats['HP'] * (1/league_mitigation(stats['MagicResist']))
		];
}
function league_dps(stats)
{
	return stats['AD'] * Math.min(2, 1 + stats['CritChance']) * Math.min(5/2, 1 + stats['AttackSpeed']);
}

var league_stat_costs = {'AD':36,'CritChance':5000,'AttackSpeed':3333,'HP':66/25,'Armor':20,'MagicResist':20};

function league_distro2stat(distro)
{
	var g = {};
	for (var i in distro)
		g[i] = distro[i] / league_stat_costs[i];
	return g;
}

function league_optimalGoldDistroEHP(u) 	
{
	if (u <= 0) 
		return {'HP':0,'Armor':0,'MagicResist':0};
	else if (u <= 2000)
		return {'HP':u,'Armor':0,'MagicResist':0};	
	else 
		return {'HP':(2000+u)/2,
				'Armor':-2000 + ((2000+u)/2),
				'MagicResist':0};	
}

function league_optimalGoldDistroDPS(u) 
{
	if (u <= 0) 
		return {'AD':0,'CritChance':0,'AttackSpeed':0};
		
	else if (u >= 20000)
		return {'AD':-10000+u,
				'CritChance':5000,
				'AttackSpeed':5000};
				
	else if (16666 < u && u < 20000)
		return {'AD':u/2,
				'CritChance':-5000+(u/2),
				'AttackSpeed':5000};
				
	else if (u <= 3333)
		return {'AD':u,
				'CritChance':0,
				'AttackSpeed':0};
				
	else if (3333 < u && u <= 6667)
		return {'AD':(3333+u)/2, 
				'CritChance':0, 
				'AttackSpeed':(1/2) * (-3333+u)};
	else
		return {'AD':(8333+u)/3, 
				'CritChance':(1/3) * (-6667+u), 
				'AttackSpeed':(1/3) * (-1666+u)};
}