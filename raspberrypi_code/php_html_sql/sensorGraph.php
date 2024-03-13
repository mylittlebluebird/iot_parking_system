<?php
	$conn = mysqli_connect("10.10.52.214", "iot", "pwiot");
#	mysqli_set_charset($conn, "UTF-8");
	mysqli_select_db($conn, "ESS");
	$query = "select * from sensor ";
	$result = mysqli_query($conn, $query);

	$data = array(array('KSH_SQL','cds0','cds1','cds2','cds3'));

	if($result)
	{
		while($row = mysqli_fetch_array($result))
		{
			array_push($data, array($row['date']."\n".$row['time'], intval($row['cds0']),intval($row['cds1']) ,intval($row['cds2']),intval($row['cds3'])));
		}
	}

	$options = array(
			'title' => 'illumination temperature humidity',
			'width' => 1000, 'height' => 400,
			'curveType' => 'function'
			);

?>

<script src="//www.google.com/jsapi"></script>
<script>
var data = <?=json_encode($data) ?>;
var options = <?= json_encode($options) ?>;

google.load('visualization', '1.0', {'packages':['corechart']});

google.setOnLoadCallback(function() {
	var chart = new google.visualization.LineChart(document.querySelector('#chart_div'));
	chart.draw(google.visualization.arrayToDataTable(data), options);
	});
	</script>
<div id="chart_div"></div>
