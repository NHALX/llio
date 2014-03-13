 
        google.setOnLoadCallback(drawAll);
		var canvas_dps;
		var canvas_ehp;
		
		function drawAll() 
		{
        
			canvas_dps = document.getElementById('mygraph');
		//	canvas_ehp = document.getElementById('mygraph_ehp');
		
			draw_chart_dps(canvas_dps);
//			draw_chart_ehp(canvas_ehp);
//			draw_chart_ehp2(canvas_ehp);
//			draw_chart_buildtree(document.getElementById('chart_div'));
            
		}
		
var items = ["item_1","item_2","item_3","item_4","item_5","item_6"];

function metricText(stats, cost)
{
	var ehp = league_ehp(stats);
	var dps = league_dps(stats);
	var optimal_dps = league_dps(league_distro2stat(league_optimalGoldDistroDPS(cost)));
	var optimal_ehp = league_ehp(league_distro2stat(league_optimalGoldDistroEHP(cost)));
	
	ehp.forEach(function(x, i, arr){
		arr[i] = '\n  ' + ehp[i].toFixed(2) + '(' + ((ehp[i]/optimal_ehp[i])*100).toFixed(0) + '% Optimal)';
		});
	
	return 'DPS: '     + dps.toFixed(2) + '(' + ((dps/optimal_dps)*100).toFixed(0) + '% Optimal)\n' + 
		   'Sustain: ' + 'undefined\n' + 
		   'EHP:'      + ehp  + '\n' + 
		   'Cost: '    + cost + '\n';
}

function sumItems()
{
	var combined_stats = document.getElementById("combined_stats");
	var metrics = document.getElementById("metrics");
	var stats = {};
	var total_cost = 0;
	
	for (var i = 0; i < items.length; i++) 
	{
		var text = document.getElementById(items[i]);
		
		if (text.firstChild != null && text.firstChild.data != null)
		{
			var itm = db_items[text.firstChild.data];

			stats = NICOLA.merge(itm['stats'], stats);
			stats = NICOLA.merge(itm['passive_stats'], stats);
			total_cost += itm['total_cost'];
			
//			label.firstChild.data = c['id']; .innerHTML
		}
	}
	
	combined_stats.firstChild.data = NICOLA.showobject(stats) 
	metrics.firstChild.data = metricText(stats,total_cost);
	
	if (canvas_dps)
        draw_chart_dps(canvas_dps, stats, total_cost);
    if (canvas_ehp)
        draw_chart_ehp(canvas_ehp, stats, total_cost);
}

function formAddItem()
{
	var item_selector = document.getElementById("item_selector");

	for (var i = 0; i < items.length; i++) 
	{
		var text = document.getElementById(items[i]);
		
		if (text.firstChild == null)
		{
			var txt = document.createTextNode(item_selector.options[item_selector.selectedIndex].text);
			var node = document.createElement("href")
			node.style = "padding: 0; border: none; background: none;";
			node.appendChild(document.createTextNode("\t\u2716"));
			node.onclick = function()
			{ 
				text.removeChild(txt);
				text.removeChild(node); 
				sumItems();
			}
			
			text.appendChild(txt);
			text.appendChild(node);
			break;
		}
	}
    sumItems();
	/*document.theForm.submit();*/
}
