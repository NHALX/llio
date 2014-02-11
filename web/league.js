// JavaScript Document

function itemMergeAllStats(a,b)
{
	var stats1 = NICOLA.merge(a['stats'], b['stats']);
	return stats1;
	//var stats2 = NICOLA.merge(a['passiv_stats'], b['passiv_stats']);
	//return NICOLA.merge(stats1,stats2);
}