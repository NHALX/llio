
        google.load("visualization", "1");
		google.load('visualization', '1', {packages:['orgchart']});
        // Set callback to run when API is loaded
        google.setOnLoadCallback(drawAll);
		
		function drawAll() 
		{
			drawVisualization()
			drawChart()
		}
		
		function drawChart() {
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
			var chart = new google.visualization.OrgChart(document.getElementById('chart_div'));
			chart.draw(data, {allowHtml:false});
      	}
		
        // Called when the Visualization API is loaded.
        function drawVisualization() {
            // Create and populate a data table.
            var data = new google.visualization.DataTable();
            data.addColumn('datetime', 'time');
            data.addColumn('number', 'Function A');
            data.addColumn('number', 'Function B');

            function functionA(x) {
                return Math.sin(x / 25) * Math.cos(x / 25) * 50 + (Math.random()-0.5) * 10;
            }

            function functionB(x) {
                return Math.sin(x / 50) *50 + Math.cos(x / 7) * 75 + (Math.random()-0.5) * 20 + 20;
            }

            // create data
            var d = new Date(2010, 9, 23, 20, 0, 0);
            for (var i = 0; i < 100; i++) {
                data.addRow([new Date(d), functionA(i), functionB(i)]);
                d.setMinutes(d.getMinutes() + 1);
            }

            // specify options
            var options = {
                "width":  "100%",
                "height": "350px"
            };

            // Instantiate our graph object.
            var graph = new links.Graph(document.getElementById('mygraph'));

            // Draw our graph with the created data and options
            graph.draw(data, options);
        }