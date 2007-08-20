--TEST--
Calling geoip_db_filename() with a non-existant database type within bound.
--SKIPIF--
<?php if (!extension_loaded("geoip")) print "skip"; ?>
--FILE--
<?php

var_dump( geoip_db_filename(14) );

?>
--EXPECT--
NULL
