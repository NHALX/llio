
        google.load("visualization", "1");
		google.load('visualization', '1', {packages:['orgchart']});
		google.load("visualization", "1", {packages:["corechart"]});
        // Set callback to run when API is loaded

		
		function draw_chart_buildtree(canvas) 
		{
			var data = new google.visualization.DataTable();
			data.addColumn('string', 'Name');
			data.addColumn('string', 'Manager');
			data.addColumn('string', 'ToolTip');
			data.addRows([
			  ['LongSword.1', 'Brutalizer', ''],
			  ['LongSword.2', 'Brutalizer', ''],
			  ['BrawlersGlovse', 'AvariceBlade', ''],
			  ['AvariceBlade', 'Ghostblade', ''],
			  ['Brutalizer', 'Ghostblade', ''],
			  ['BFSword', 'InfinityEdge', ''],
  			  ['Pickaxe', 'InfinityEdge', ''],
  			  ['Cape', 'InfinityEdge', '']
			]);
			var chart = new google.visualization.OrgChart(canvas);
			chart.draw(data, {allowHtml:false});
      	}
		
		function chart_layout(text)
		{
		    var data = new google.visualization.DataTable();
            data.addColumn('number', 'Gold');
            data.addColumn('number', 'Theoretical optimum');
			data.addColumn({type:'string', role:'tooltip', 'p': {'html': true}});
			data.addColumn({type:'string', role:'annotation'}); // annotation role col.
			data.addColumn({type:'string', role:'annotationText'}); // annotationText col.
			data.addColumn('number', 'Build: ' + text);
			data.addColumn({type:'string', role:'tooltip', 'p': {'html': true}});

			data.addColumn('number', 'Build: ' + text + ' (cost adjusted)');
			data.addColumn({type:'string', role:'tooltip', 'p': {'html': true}});
			return data;
		}
		
		function chart_tooltip(tag,u,r)
		{/*
			txt = "<b>"+tag+":</b> " + r['y'].toFixed(2) + ' ('+ u.toFixed(0) + "g)<br><br>";
			for (var k in r['distro'])
				txt += "<b>"+k+":</b> " + r['stats'][k].toFixed(2) + ' ('+ r['distro'][k].toFixed(0)  +'g)'+ "<br>" 
			*/
            txt = tag+": " + r['y'].toFixed(2) + ' ('+ u.toFixed(0) + "g)\n";
			for (var k in r['distro'])
				txt += "\n" + k + ": " + r['stats'][k].toFixed(2) + ' ('+ r['distro'][k].toFixed(0)  +'g)'
			return txt; 
		}
		function chart_extra(tag, func, stats, total_cost)
		{
			if (stats == null || total_cost == null)
				return null;
				
			var metric = func(stats);
			var adj_cost = 0;
			for (var j in league_stat_costs)
				adj_cost += stats[j]*league_stat_costs[j];
				
			return [{'y':metric,'x':total_cost,'tooltip':tag+": " + metric.toFixed(2) + ", cost: " + total_cost.toFixed(2)},
					{'y':metric,'x':adj_cost,'tooltip':tag+": " + metric.toFixed(2) + ", cost: " + adj_cost.toFixed(2)}];
		}
			
		function draw_chart_ehp(canvas, stats, total_cost) 
		{
			var data = chart_layout("EHP");
			var cfg = {'options' : {
				
			 hAxis: { viewWindow: {min:0,max:25000} },
         	 title: 'EHP',
			 curveType: 'function',
			 legend: { position: 'bottom' },
			 series: {
			  1: {
				pointSize: 8,
				color: 'red',
			 	/*lineWidth: 5,*/
			 
				annotations: {
				  textStyle: {fontSize: 14, color: 'red' }
				}
			  },
			  
			  2: {pointSize: 4, color: 'green'}
			 }

       		},
			'function' : function (x){
				var distro = league_optimalGoldDistroEHP(x);
				var stats  = league_distro2stat(distro);
				return { 'y'      : league_ehp(stats)[0], 
					     'distro' : distro,
						 'stats'  : stats };
			 	},
			'tooltip' : chart_tooltip,
			'thresholds' : [
				{'limit':0,     'annotation':"HP",        'annotationText':"Gold allocation: 100% Health"},
				{'limit':2000,  'annotation':"HP,AR/MR",  'annotationText':"Gold allocation: 50% Health, 50% Armor/MagicResist"},
				],
			'x_end' : 30000,
			'y_tag' : "EHP"
			};
			
			draw_chart(cfg, canvas, chart_extra("EHP", function(x){return league_ehp(x)[0];}, stats, total_cost));
		}
		
		function draw_chart_dps(canvas, stats, total_cost) 
		{
            var style = {  fontName: 'Terminal', fontSize: 11, color:'white'};
			var cfg = {'options' : {
             backgroundColor: {fill:'transparent'},
             titleTextStyle: style,
             
             colors: ['white','white','white','white','white','white','white'],
             legend: { position: 'right', textStyle: style },
			 hAxis: { viewWindow: {min:0,max:25000}, textStyle: style, gridlines: {color: '#000000', count: 16}},
             vAxis: { textStyle: style,gridlines: {color: '#111111', count: 16} },
             			 tooltip: {textStyle: {color: '#0000ff'}, isHtml: true},
                            
               annotations: {
            
    textStyle: {
      fontName: 'Terminal',
      fontSize: 21,
      bold: false,
      italic: false,/*
      color: '#871b47',     // The color of the text.
      auraColor: '#d799ae', // The color of the text outline.
      opacity: 0.8          // The transparency of the text. */
    }
  }, 
  
         	 title: 'Auto-attack DPS',
			 curveType: 'function',
			
			 tooltip: {isHtml: false},
			 series: {
              0: { lineWidth : 6 },
			  1: {
				pointSize: 20,
				color: 'Blue',
			 	/*lineWidth: 5,*/
                
				annotations: {
				  textStyle: {fontSize: 14, color: 'Blue' }
				}
			  },
			  
			  2: {pointSize: 14, color: 'purple'}
			 }

       		},
			'function' : function (x){
				var distro = league_optimalGoldDistroDPS(x);
				var stats  = league_distro2stat(distro);
				return { 'y'      : league_dps(stats), 
					     'distro' : distro,
						 'stats'  : stats };
			 	},
			'tooltip' : chart_tooltip,
			'thresholds' : [
				{'limit':0,     'annotation':"AD",         'annotationText':"Gold allocation: 100% Attack"},
				{'limit':3333,  'annotation':"AD,AS",      'annotationText':"Gold allocation: 50% Attack, 50% Attack Speed"},
				{'limit':6667,  'annotation':"AD,AS,Crit", 'annotationText':"Gold allocation: 33% Attack, 33% Attack Speed, 33% Crit"},
				{'limit':16666, 'annotation':"AD,Crit",    'annotationText':"Gold allocation: 50% Attack, 50% Crit"},
				{'limit':20000, 'annotation':"AD",         'annotationText':"Gold allocation: 100% Attack" }
				],
			'x_end' : 30000,
			'y_tag' : "DPS"
			};
			

			draw_chart(cfg, canvas, chart_extra("DPS", league_dps, stats, total_cost));
		}
       
        function draw_chart(cfg, canvas, extra) 
		{
       
            var data = chart_layout(cfg['y_tag']);
		
			for (var i = 0; i < cfg['thresholds'].length; i++)
			{
				u = cfg['thresholds'][i]['limit'];

				function annotate(u,r)
				{
					var result;
					
					for (var j = 0; j < cfg['thresholds'].length; j++)
						if (u >= cfg['thresholds'][j]['limit'])
							result = j;
							
					return [cfg['thresholds'][result]['annotation'], 
							cfg['thresholds'][result]['annotationText']];						
				}
				
				var r = cfg['function'](u);
                data.addRow([u, r['y'], cfg['tooltip'](cfg['y_tag'],u,r)].concat(annotate(u,r).concat([null,null,null,null])));	
	
				var end = (i + 1 < cfg['thresholds'].length) 
						? cfg['thresholds'][i+1]['limit']
						: cfg['x_end'];
				
				for (var j = u; j < end; j+=500) {
					r = cfg['function'](j);
					data.addRow([j, r['y'], cfg['tooltip'](cfg['y_tag'],j,r)].concat([null,null,null,null,null,null]));
				}	
			}

			if (extra != null)
			{
				data.addRow([extra[0]['x'], null, null, null, null, extra[0]['y'], extra[0]['tooltip'], null,null]);	
				data.addRow([extra[1]['x'], null, null, null, null, null,null, extra[1]['y'], extra[1]['tooltip']]);	
			}
			

       		var graph = new google.visualization.LineChart(canvas);
            // Draw our graph with the created data and options
            graph.draw(data, cfg['options']);
        }
		
	
	function custom(u,scale)
	{
		if ((scale == 0 || 2000/scale > u) && u > 0)
			return [u, 0, (25*u)/66];
			
		else if ((2000/scale) < u)
		
			return [(1/2) * (2000/scale + u), 
					-(1000/scale) + (u/2), 
					-((25*(2000 + scale*u)*(-2000 + scale*(4000 + u)))/(132*scale*(2000*(1-3*scale) + (-1 + scale)*scale*u)))
					];
		else
			return [0,0,0];
    }
	function draw_chart_ehp2(canvas)
	{
        // Create and populate a data table.
        data = new google.visualization.DataTable();
        data.addColumn('number', 'x');
        data.addColumn('number', 'y');
        data.addColumn('number', 'z');

        // create some nice looking data with sin/cos
        var steps = 20;  // number of datapoints will be steps*steps
        var axisMax = 15000;
        axisStep = axisMax / steps;

		for (var v = 0; v <= 1; v+=0.05) {
        for (var u = 0; u < axisMax; u+=axisStep) 
				data.addRow(custom(u,v));

        for (var u = axisMax; u >= 0; u-=axisStep) 
	        	data.addRow(custom(u,v));
	
        }
		for (var v = 0; v <= 1; v+=0.05) 
			data.addRow(custom(axisMax,v));
		
        // specify options
        options = {width:  "100%", 
                   height: "100%",
				   xCenter: "50%",
                   style: "line",
				   backgroundColor: "#222222",
                   showPerspective: true,
                   showGrid: true,
                   showShadow: false,
                   keepAspectRatio: true,
                   verticalRatio: 0.5,
                   };
		//data.sort([{column: 0}]);
        // Instantiate our graph object.
        graph = new links.Graph3d(canvas);

        // Draw our graph with the created data and options 
        graph.draw(data, options);
      
	}