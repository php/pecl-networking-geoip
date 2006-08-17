/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2006 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Olivier Hill                                                 |
  |         Matthew Fonda                                                |
  +----------------------------------------------------------------------+
  Please contact support@maxmind.com with any comments
*/


#define EXTENSION_VERSION "0.1.4-dev"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <GeoIP.h>
#include <GeoIPCity.h>

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_geoip.h"



ZEND_DECLARE_MODULE_GLOBALS(geoip)

static int le_geoip;

/* {{{ */
function_entry geoip_functions[] = {
	PHP_FE(geoip_database_info,   NULL)   
	PHP_FE(geoip_country_code_by_name,   NULL)
	PHP_FE(geoip_country_name_by_name,   NULL)
	PHP_FE(geoip_org_by_name,   NULL)
	PHP_FE(geoip_record_by_name,   NULL)
	PHP_FE(geoip_id_by_name,   NULL)
	PHP_FE(geoip_region_by_name,   NULL)
	{NULL, NULL, NULL}   
};
/* }}} */

/* {{{ geoip_module_entry
 */
zend_module_entry geoip_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"geoip",
	geoip_functions,
	PHP_MINIT(geoip),
	PHP_MSHUTDOWN(geoip),
	PHP_RINIT(geoip),      
	PHP_RSHUTDOWN(geoip),   
	PHP_MINFO(geoip),
#if ZEND_MODULE_API_NO >= 20010901
	EXTENSION_VERSION, /* version number of the extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_GEOIP
ZEND_GET_MODULE(geoip)
#endif


#ifdef GEOIPDATADIR
#define GEOIPDATABASE GEOIPDATADIR "/GeoIP.dat"
#else
#define GEOIPDATABASE "GeoIP.dat"
#endif


/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("geoip.database_standard", GEOIPDATABASE, PHP_INI_ALL, OnUpdateString, database_standard, zend_geoip_globals, geoip_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_geoip_init_globals
 */
static void php_geoip_init_globals(zend_geoip_globals *geoip_globals)
{
	geoip_globals->database_standard = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(geoip)
{
	ZEND_INIT_MODULE_GLOBALS(geoip, php_geoip_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	_GeoIP_setup_dbfilename();
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(geoip)
{
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(geoip)
{
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(geoip)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(geoip)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "geoip support", "enabled");
	php_info_print_table_row(2, "geoip extension version", EXTENSION_VERSION);
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto string geoip_database_info()
   Returns GeoIP Database information */
PHP_FUNCTION(geoip_database_info)
{
	GeoIP * gi;
	char * db_info;

	if(ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	gi = GeoIP_new(GEOIP_COUNTRY_EDITION);
	db_info = GeoIP_database_info(gi);
	GeoIP_delete(gi);

	RETURN_STRING(db_info, 1);
}
/* }}} */

/* {{{ proto string geoip_country_code_by_name( string hostname )
   Return the Country Code found in the GeoIP Database */
PHP_FUNCTION(geoip_country_code_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	const char * country_code;
	int arglen;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_COUNTRY_EDITION)) {
		gi = GeoIP_new(GEOIP_COUNTRY_EDITION);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available.");
		return;
	}
	
	country_code = GeoIP_country_code_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (country_code == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Host %s not found", hostname);
		return;
	}
	RETURN_STRING((char*)country_code, 1);
}
/* }}} */

/* {{{ proto string geoip_country_name_by_name( string hostname )
   Returns the Country name found in the GeoIP Database */
PHP_FUNCTION(geoip_country_name_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	const char * country_name;
	int arglen;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_COUNTRY_EDITION)) {
		gi = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available.");
		return;
	}

	country_name = GeoIP_country_name_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (country_name == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Host %s not found", hostname);
		return;
	}
	RETURN_STRING((char*)country_name, 1);
}
/* }}} */

/* {{{ proto string geoip_org_by_name( string hostname )
   Returns the Organization Name found in the GeoIP Database */
PHP_FUNCTION(geoip_org_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	const char * org;
	int arglen;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}
	
	if (GeoIP_db_avail(GEOIP_ORG_EDITION)) {
		gi = GeoIP_open_type(GEOIP_ORG_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available");
		return;
	}

	org = GeoIP_org_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (org == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Host %s not found", hostname);
		return;
	}
	RETURN_STRING((char*)org, 1);
}
/* }}} */

/* {{{ proto array geoip_record_by_name( string hostname )
   Returns the detailed City information found in the GeoIP Database */
PHP_FUNCTION(geoip_record_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	int arglen;
	GeoIPRecord * gir;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_CITY_EDITION_REV1)) {
		gi = GeoIP_open_type(GEOIP_CITY_EDITION_REV1, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available");
		return;
	}
	gir = GeoIP_record_by_name(gi, hostname);

	GeoIP_delete(gi);
	
	if (NULL == gir) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Host %s not found", hostname);
		return;
	}
	
	array_init(return_value);
	add_assoc_string(return_value, "country_code", (gir->country_code == NULL) ? "" : gir->country_code, 1);
	add_assoc_string(return_value, "region", (gir->region == NULL) ? "" : gir->region, 1);
	add_assoc_string(return_value, "city", (gir->city == NULL) ? "" : gir->city, 1);
	add_assoc_string(return_value, "postal_code", (gir->postal_code == NULL) ? "" : gir->postal_code, 1);
	add_assoc_double(return_value, "latitude", gir->latitude);
	add_assoc_double(return_value, "longitude", gir->longitude);
	add_assoc_long(return_value, "dma_code", gir->dma_code);
	add_assoc_long(return_value, "area_code", gir->area_code);
	
	GeoIPRecord_delete(gir);
}
/* }}} */

/* {{{ proto integer geoip_id_by_name( string hostname )
   Returns the Net Speed found in the GeoIP Database */
PHP_FUNCTION(geoip_id_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	int arglen;
	int netspeed;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_NETSPEED_EDITION)) {
		gi = GeoIP_open_type(GEOIP_NETSPEED_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available");
		return;
	}

	netspeed = GeoIP_id_by_name(gi, hostname);
	GeoIP_delete(gi);
	RETURN_LONG(netspeed);
}
/* }}} */

/* {{{ proto array geoip_region_by_name( string hostname )
   Returns the Country Code and region found in the GeoIP Database */
PHP_FUNCTION(geoip_region_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	int arglen;
	GeoIPRegion * region;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_REGION_EDITION_REV0) || GeoIP_db_avail(GEOIP_REGION_EDITION_REV1)) {
		if (GeoIP_db_avail(GEOIP_REGION_EDITION_REV1)) {
			gi = GeoIP_open_type(GEOIP_REGION_EDITION_REV1, GEOIP_STANDARD);
		} else {
			gi = GeoIP_open_type(GEOIP_REGION_EDITION_REV0, GEOIP_STANDARD);
		}
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available");
		return;
	}

	region = GeoIP_region_by_name(gi, hostname);
	GeoIP_delete(gi);

	if (NULL == region) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Host %s not found", hostname);
		return;
	}

	array_init(return_value);
	add_assoc_string(return_value, "country_code", region->country_code, 1);
	add_assoc_string(return_value, "region", region->region, 1);
	
	GeoIPRegion_delete(region);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */