--TEST--
Checking timezone info
--SKIPIF--
<?php if (!extension_loaded("geoip")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php

var_dump(geoip_time_zone_by_country_and_region('CA','BC'));
var_dump(geoip_time_zone_by_country_and_region('JP','01'));
?>
--EXPECT--
string(17) "America/Vancouver"
string(10) "Asia/Tokyo"
