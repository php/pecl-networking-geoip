--TEST--
Checking geoip_country_code_by_name_v6 (if IPv6 DB installed)
--SKIPIF--
<?php if (!extension_loaded("geoip") || !defined("GEOIP_COUNTRY_EDITION_V6") || !geoip_db_avail(GEOIP_COUNTRY_EDITION_V6)) print "skip"; ?>
--INI--
--FILE--
<?php

var_dump( geoip_country_code_by_name_v6('2001:4860:4860::8888') );

?>
--EXPECT--
string(2) "US"
