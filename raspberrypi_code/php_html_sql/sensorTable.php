<!DOCTYPE html>
<html>
<head>
	<meta charset = "UTF-8">
	<meta http-equiv = "refresh" content = "30">
	<style type = "text/css">
		.spec{
			text-align:center;
		}
		.con{
			text-align:left;
		}
		</style>
</head>

<body>
	<h1 align = "center">iot Database</h1>
	<div class = "spec">
		# <b>The sensor value description</b>
		<br></br>
	</div>

	<table border = '1' style = "width = 30%" align = "center">
	<tr align = "center">
		<th>NUM</th>
		<th>DATE</th>
		<th>TIME</th>
		<th>Vout</th>
		<th>cds0</th>
		<th>cds1</th>
		<th>cds2</th>
		<th>cds3</th>
		<th>PAN</th>
		<th>TILT</th>
		<th>BUZZ</th>
	</tr>

	<?php
		$conn = mysqli_connect("10.10.52.214", "iot", "pwiot");
		mysqli_select_db($conn, "ESS");
		$result = mysqli_query($conn, "select * from sensor");
		while($row = mysqli_fetch_array($result))
		{
			echo "<tr align = center>";
			echo '<th>'.$row['num'].'</td>';
			echo '<th>'.$row['date'].'</td>';
			echo '<th>'.$row['time'].'</td>';
			echo '<th>'.$row['Vout'].'</td>';
			echo '<th>'.$row['cds0'].'</td>';
			echo '<th>'.$row['cds1'].'</td>';
			echo '<th>'.$row['cds2'].'</td>';
			echo '<th>'.$row['cds3'].'</td>';
			echo '<th>',$row['pan'].'</td>';
	      		echo '<th>',$row['tilt'].'</td>';
			echo '<th>'.$row['buzz'].'</td>';
			echo "</tr>";

		}
		
		mysqli_close($conn);
	?>
	</table>
</body>
</html>
