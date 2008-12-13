--TEST--
Checking timezone info
--SKIPIF--
<?php if (!extension_loaded("geoip")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php

var_dump(geoip_time_zone_by_country_and_region('CA','QC'));

?>
--EXPECT--
string(16) "America/Montreal"
