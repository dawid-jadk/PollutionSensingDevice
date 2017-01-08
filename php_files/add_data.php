<?php
define('HOST','EDIT HERE');
define('USER','EDIT HERE');
define('PASS','EDIT HERE');
define('DB','EDIT HERE');
$con = mysqli_connect(HOST,USER,PASS,DB);
$sql = "INSERT INTO pollution_values (gas1_value, gas2_value, gas3_value, time_value, location_lat_value, location_lng_value) VALUES ('".$_GET["gas1"]."', '".$_GET["gas2"]."', '".$_GET["gas3"]."', '".$_GET["time"]."', '".$_GET["lat"]."', '".$_GET["lng"]."')";
mysqli_query($con,$sql);
mysqli_close($con);
?>