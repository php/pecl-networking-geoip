/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Olivier Hill   <ohill@php.net>                               |
  |         Matthew Fonda                                                |
  +----------------------------------------------------------------------+
  Please contact support@maxmind.com with any comments
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <GeoIP.h>
#include <GeoIPCity.h>
#ifdef PHP_WIN32
#include <GeoIP_internal.h>
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_geoip.h"

/* For PHP 8 */
#ifndef TSRMLS_CC
#define TSRMLS_CC
#endif

ZEND_DECLARE_MODULE_GLOBALS(geoip)

ZEND_BEGIN_ARG_INFO_EX(arginfo_geoip_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_geoip_database_opt, 0, 0, 0)
	ZEND_ARG_INFO(0, database)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_geoip_database, 0, 0, 1)
	ZEND_ARG_INFO(0, database)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_geoip_host, 0, 0, 1)
	ZEND_ARG_INFO(0, host)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_geoip_region, 0, 0, 2)
	ZEND_ARG_INFO(0, country_code)
	ZEND_ARG_INFO(0, region_code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_geoip_directory, 0, 0, 1)
	ZEND_ARG_INFO(0, directory)
ZEND_END_ARG_INFO()

static int le_geoip;

/* {{{ */
zend_function_entry geoip_functions[] = {
	PHP_FE(geoip_database_info,           arginfo_geoip_database_opt)   
#define GEOIPDEF(php_func, c_func, db_type) \
	PHP_FE(php_func,	                  arginfo_geoip_host)
#include "geoip.def"
#undef GEOIPDEF
	PHP_FE(geoip_continent_code_by_name,   arginfo_geoip_host)
	PHP_FE(geoip_org_by_name,              arginfo_geoip_host)
	PHP_FE(geoip_record_by_name,           arginfo_geoip_host)
	PHP_FE(geoip_id_by_name,               arginfo_geoip_host)
	PHP_FE(geoip_region_by_name,           arginfo_geoip_host)
	PHP_FE(geoip_isp_by_name,              arginfo_geoip_host)
	PHP_FE(geoip_db_avail,	               arginfo_geoip_database)
	PHP_FE(geoip_db_get_all_info,	       arginfo_geoip_void)
	PHP_FE(geoip_db_filename,	           arginfo_geoip_database)
#if LIBGEOIP_VERSION >= 1004001
	PHP_FE(geoip_region_name_by_code,      arginfo_geoip_region)
	PHP_FE(geoip_time_zone_by_country_and_region,	arginfo_geoip_region)
#endif
#ifdef HAVE_CUSTOM_DIRECTORY
    PHP_FE(geoip_setup_custom_directory,   arginfo_geoip_directory)
#endif
	PHP_FE(geoip_asnum_by_name,            arginfo_geoip_host)
	PHP_FE(geoip_domain_by_name,           arginfo_geoip_host)
#if LIBGEOIP_VERSION >= 1004008
	PHP_FE(geoip_netspeedcell_by_name,     arginfo_geoip_host)
#endif
#ifdef PHP_FE_END
	PHP_FE_END
#else
	{NULL, NULL, NULL}
#endif
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
	PHP_GEOIP_VERSION, /* version number of the extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_GEOIP
ZEND_GET_MODULE(geoip)
#endif

#ifdef HAVE_CUSTOM_DIRECTORY
/* {{{ geoip_change_custom_directory() helper function
 */
static void geoip_change_custom_directory(char *value)
{
#if LIBGEOIP_VERSION >= 1004007
	GeoIP_cleanup();
#else
	int i;
	if (GeoIPDBFileName != NULL) {
		for (i = 0; i < NUM_DB_TYPES; i++) {
			if (GeoIPDBFileName[i]) {
				free(GeoIPDBFileName[i]);
			}
		}
		free(GeoIPDBFileName);
		GeoIPDBFileName = NULL;
	}
#endif

	GeoIP_setup_custom_directory(value);
	GeoIP_db_avail(GEOIP_COUNTRY_EDITION);
}
/* }}} */
#endif

#ifdef HAVE_CUSTOM_DIRECTORY
/* {{{ PHP_INI_MH 
+ */
static PHP_INI_MH(OnUpdateDirectory)
{
	if (stage == PHP_INI_STAGE_RUNTIME || stage == PHP_INI_STAGE_HTACCESS) {
		GEOIP_G(set_runtime_custom_directory) = 1;
#if PHP_MAJOR_VERSION >= 7
		geoip_change_custom_directory(new_value->val);
#else
		geoip_change_custom_directory(new_value);
#endif
		return SUCCESS;
	}
	
#if PHP_MAJOR_VERSION >= 7
	return OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
#else
	return OnUpdateString(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
#endif
}
/* }}} */
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
#ifdef HAVE_CUSTOM_DIRECTORY
	STD_PHP_INI_ENTRY("geoip.custom_directory", NULL, PHP_INI_ALL, OnUpdateDirectory, custom_directory, zend_geoip_globals, geoip_globals)
#endif
PHP_INI_END()
/* }}} */

/* {{{ php_geoip_init_globals
 */
static void php_geoip_init_globals(zend_geoip_globals *geoip_globals)
{
	geoip_globals->custom_directory = NULL;
	geoip_globals->set_runtime_custom_directory = 0;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(geoip)
{
	ZEND_INIT_MODULE_GLOBALS(geoip, php_geoip_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	/* @TODO: Do something for custom_directory before initialization here */

#ifdef HAVE_CUSTOM_DIRECTORY
	GeoIP_setup_custom_directory(GEOIP_G(custom_directory));
#endif
	/* This will initialize file structure */
	GeoIP_db_avail(GEOIP_COUNTRY_EDITION);
	
	/* For database type constants */
	REGISTER_LONG_CONSTANT("GEOIP_COUNTRY_EDITION",     GEOIP_COUNTRY_EDITION,     CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_REGION_EDITION_REV0", GEOIP_REGION_EDITION_REV0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_CITY_EDITION_REV0",   GEOIP_CITY_EDITION_REV0,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_ORG_EDITION",         GEOIP_ORG_EDITION,         CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_ISP_EDITION",         GEOIP_ISP_EDITION,         CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_CITY_EDITION_REV1",   GEOIP_CITY_EDITION_REV1,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_REGION_EDITION_REV1", GEOIP_REGION_EDITION_REV1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_PROXY_EDITION",       GEOIP_PROXY_EDITION,       CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_ASNUM_EDITION",       GEOIP_ASNUM_EDITION,       CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_NETSPEED_EDITION",    GEOIP_NETSPEED_EDITION,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_DOMAIN_EDITION",      GEOIP_DOMAIN_EDITION,      CONST_CS | CONST_PERSISTENT);
#if LIBGEOIP_VERSION >= 1004008
	REGISTER_LONG_CONSTANT("GEOIP_NETSPEED_EDITION_REV1",GEOIP_NETSPEED_EDITION_REV1,CONST_CS | CONST_PERSISTENT);
#endif

	/* For netspeed constants */
	REGISTER_LONG_CONSTANT("GEOIP_UNKNOWN_SPEED",       GEOIP_UNKNOWN_SPEED,       CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_DIALUP_SPEED",        GEOIP_DIALUP_SPEED,        CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_CABLEDSL_SPEED",      GEOIP_CABLEDSL_SPEED,      CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GEOIP_CORPORATE_SPEED",     GEOIP_CORPORATE_SPEED,     CONST_CS | CONST_PERSISTENT);
	
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
#ifdef HAVE_CUSTOM_DIRECTORY
	/* If we have a custom directory (and have support from   */
	/* libgeoip, we reset the extension to default directory) */
	if (GEOIP_G(set_runtime_custom_directory)) {
		geoip_change_custom_directory(GEOIP_G(custom_directory));
		GEOIP_G(set_runtime_custom_directory) = 0;
	}
#endif
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(geoip)
{
	char buf[32];
	
	php_info_print_table_start();
	php_info_print_table_header(2, "geoip support", "enabled");
	php_info_print_table_row(2, "geoip extension version", PHP_GEOIP_VERSION);
	snprintf(buf, sizeof(buf), "%d", LIBGEOIP_VERSION);
	php_info_print_table_row(2, "geoip library version", buf);
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto boolean geoip_db_avail( [ int database ] ) */
PHP_FUNCTION(geoip_db_avail)
{
	zend_long edition;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &edition) == FAILURE) {
		return;
	}
	
	if (edition < 0 || edition >= NUM_DB_TYPES)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Database type given is out of bound.");
		return;
	}
	
	RETURN_BOOL(GeoIP_db_avail(edition));    
}
/* }}} */

/* {{{ proto string geoip_db_filename( [ int database ] ) */
PHP_FUNCTION(geoip_db_filename)
{
	zend_long edition;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &edition) == FAILURE) {
		return;
	}
	
	if (edition < 0 || edition >= NUM_DB_TYPES)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Database type given is out of bound.");
		return;
	}
	
	if (NULL != GeoIPDBFileName[edition])
#if PHP_MAJOR_VERSION >= 7
		RETURN_STRING(GeoIPDBFileName[edition]);	
#else
		RETURN_STRING(GeoIPDBFileName[edition], 1);	
#endif
}
/* }}} */

/* {{{ proto array geoip_db_get_all_info( ) */
PHP_FUNCTION(geoip_db_get_all_info)
{
	int i;
	
	array_init(return_value);

	for (i=0; i < NUM_DB_TYPES; i++)
	{
		if (NULL != GeoIPDBDescription[i])
		{
#if PHP_MAJOR_VERSION >= 7
			zval real_row;
			zval *row = &real_row;

			array_init(row);
#else
			zval *row;
			ALLOC_INIT_ZVAL(row);
			array_init(row);
#endif

			add_assoc_bool(row, "available", GeoIP_db_avail(i));
			if (GeoIPDBDescription[i]) {
#if PHP_MAJOR_VERSION >= 7
				add_assoc_string(row, "description", (char *)GeoIPDBDescription[i]);
#else
				add_assoc_string(row, "description", (char *)GeoIPDBDescription[i], 1);
#endif
			}
			if (GeoIPDBFileName[i]) {
#if PHP_MAJOR_VERSION >= 7
				add_assoc_string(row, "filename", GeoIPDBFileName[i]);
#else
				add_assoc_string(row, "filename", GeoIPDBFileName[i], 1);
#endif
			}

			add_index_zval(return_value, i, row);
		}
	}
}
/* }}} */

/* {{{ proto string geoip_database_info( [ int database ] )
   Returns GeoIP Database information */
PHP_FUNCTION(geoip_database_info)
{
	GeoIP * gi;
	char * db_info;
	zend_long edition = GEOIP_COUNTRY_EDITION;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &edition) == FAILURE) {
		return;
	}

	if (edition < 0 || edition >= NUM_DB_TYPES)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Database type given is out of bound.");
		return;
	}
	
	if (GeoIP_db_avail(edition)) {
		gi = GeoIP_open_type(edition, GEOIP_STANDARD);
	} else {
		if (NULL != GeoIPDBFileName[edition])
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[edition]);
		else
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available.");
		return;
	}
	
	db_info = GeoIP_database_info(gi);
	GeoIP_delete(gi);

#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(db_info);
#else
	RETVAL_STRING(db_info, 1);
#endif
	free(db_info);
}
/* }}} */

/* {{{ */
#if PHP_MAJOR_VERSION >= 7
#define GEOIPDEF(php_func, c_func, db_type) \
	PHP_FUNCTION(php_func) \
	{ \
		GeoIP * gi; \
		char * hostname = NULL; \
		const char * return_code; \
		size_t arglen; \
		\
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) { \
			return; \
		} \
		\
		if (GeoIP_db_avail(db_type)) { \
			gi = GeoIP_open_type(db_type, GEOIP_STANDARD); \
		} else { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[db_type]); \
			return; \
		} \
		\
		return_code = c_func(gi, hostname); \
		GeoIP_delete(gi); \
		if (return_code == NULL) { \
			RETURN_FALSE; \
		} \
		RETURN_STRING((char*)return_code); \
		\
	}
#else
#define GEOIPDEF(php_func, c_func, db_type) \
	PHP_FUNCTION(php_func) \
	{ \
		GeoIP * gi; \
		char * hostname = NULL; \
		const char * return_code; \
		int arglen; \
		\
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) { \
			return; \
		} \
		\
		if (GeoIP_db_avail(db_type)) { \
			gi = GeoIP_open_type(db_type, GEOIP_STANDARD); \
		} else { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[db_type]); \
			return; \
		} \
		\
		return_code = c_func(gi, hostname); \
		GeoIP_delete(gi); \
		if (return_code == NULL) { \
			RETURN_FALSE; \
		} \
		RETURN_STRING((char*)return_code, 1); \
		\
	}
#endif
#include "geoip.def"
#undef GEOIPDEF
/* }}} */

/* {{{ proto string geoip_continent_code_by_name( string hostname )
   Returns the Continent name found in the GeoIP Database */
PHP_FUNCTION(geoip_continent_code_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	int id;
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_COUNTRY_EDITION)) {
		gi = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_COUNTRY_EDITION]);
		return;
	}

	id = GeoIP_id_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (id == 0) {
		RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING((char *)GeoIP_country_continent[id]);
#else
	RETURN_STRING((char *)GeoIP_country_continent[id], 1);
#endif
}
/* }}} */

/* {{{ proto string geoip_org_by_name( string hostname )
   Returns the Organization Name found in the GeoIP Database */
PHP_FUNCTION(geoip_org_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	char * org;
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}
	
	if (GeoIP_db_avail(GEOIP_ORG_EDITION)) {
		gi = GeoIP_open_type(GEOIP_ORG_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_ORG_EDITION]);
		return;
	}

	org = GeoIP_org_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (org == NULL) {
		RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(org);
#else
	RETVAL_STRING(org, 1);
#endif
	free(org);
}
/* }}} */

/* {{{ proto string geoip_asnum_by_name( string hostname )
   Returns the Domain Name found in the GeoIP Database */
PHP_FUNCTION(geoip_asnum_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	char * org;
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_ASNUM_EDITION)) {
		gi = GeoIP_open_type(GEOIP_ASNUM_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_ASNUM_EDITION]);
		return;
	}

	org = GeoIP_name_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (org == NULL) {
		RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(org);
#else
	RETVAL_STRING(org, 1);
#endif
	free(org);
}
/* }}} */

/* {{{ proto string geoip_domain_by_name( string hostname )
   Returns the Domain Name found in the GeoIP Database */
PHP_FUNCTION(geoip_domain_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	char * org;
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_DOMAIN_EDITION)) {
		gi = GeoIP_open_type(GEOIP_DOMAIN_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_DOMAIN_EDITION]);
		return;
	}

	org = GeoIP_name_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (org == NULL) {
		RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(org);
#else
	RETVAL_STRING(org, 1);
#endif
	free(org);
}
/* }}} */

#if LIBGEOIP_VERSION >= 1004008
/* {{{ proto string geoip_netspeedcell_by_name( string hostname )
   Returns the Net Speed found in the GeoIP Database */
PHP_FUNCTION(geoip_netspeedcell_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	char * org;
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_NETSPEED_EDITION_REV1)) {
		gi = GeoIP_open_type(GEOIP_NETSPEED_EDITION_REV1, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_NETSPEED_EDITION_REV1]);
		return;
	}

	org = GeoIP_name_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (org == NULL) {
		RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(org);
#else
	RETVAL_STRING(org, 1);
#endif
	free(org);
}
/* }}} */
#endif

/* {{{ proto array geoip_record_by_name( string hostname )
   Returns the detailed City information found in the GeoIP Database */
PHP_FUNCTION(geoip_record_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif
	GeoIPRecord * gir;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_CITY_EDITION_REV1) || GeoIP_db_avail(GEOIP_CITY_EDITION_REV0)) {
		if (GeoIP_db_avail(GEOIP_CITY_EDITION_REV1)) {
			gi = GeoIP_open_type(GEOIP_CITY_EDITION_REV1, GEOIP_STANDARD);
		} else {
			gi = GeoIP_open_type(GEOIP_CITY_EDITION_REV0, GEOIP_STANDARD);
		}
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_CITY_EDITION_REV0]);
		return;
	}
	gir = GeoIP_record_by_name(gi, hostname);

	GeoIP_delete(gi);
	
	if (NULL == gir) {
		RETURN_FALSE;
	}
	
	array_init(return_value);
#if PHP_MAJOR_VERSION >= 7
# if LIBGEOIP_VERSION >= 1004003
	add_assoc_string(return_value, "continent_code", (gir->continent_code == NULL) ? "" : gir->continent_code);
# endif
	add_assoc_string(return_value, "country_code", (gir->country_code == NULL) ? "" : gir->country_code);
	add_assoc_string(return_value, "country_code3", (gir->country_code3 == NULL) ? "" : gir->country_code3);
	add_assoc_string(return_value, "country_name", (gir->country_name == NULL) ? "" : gir->country_name);
	add_assoc_string(return_value, "region", (gir->region == NULL) ? "" : gir->region);
	add_assoc_string(return_value, "city", (gir->city == NULL) ? "" : gir->city);
	add_assoc_string(return_value, "postal_code", (gir->postal_code == NULL) ? "" : gir->postal_code);
#else
# if LIBGEOIP_VERSION >= 1004003
	add_assoc_string(return_value, "continent_code", (gir->continent_code == NULL) ? "" : gir->continent_code, 1);
# endif
	add_assoc_string(return_value, "country_code", (gir->country_code == NULL) ? "" : gir->country_code, 1);
	add_assoc_string(return_value, "country_code3", (gir->country_code3 == NULL) ? "" : gir->country_code3, 1);
	add_assoc_string(return_value, "country_name", (gir->country_name == NULL) ? "" : gir->country_name, 1);
	add_assoc_string(return_value, "region", (gir->region == NULL) ? "" : gir->region, 1);
	add_assoc_string(return_value, "city", (gir->city == NULL) ? "" : gir->city, 1);
	add_assoc_string(return_value, "postal_code", (gir->postal_code == NULL) ? "" : gir->postal_code, 1);
#endif
	add_assoc_double(return_value, "latitude", gir->latitude);
	add_assoc_double(return_value, "longitude", gir->longitude);
#if LIBGEOIP_VERSION >= 1004005
	add_assoc_long(return_value, "dma_code", gir->metro_code);
#else
	add_assoc_long(return_value, "dma_code", gir->dma_code);
#endif
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
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif
	int netspeed;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}

	if (GeoIP_db_avail(GEOIP_NETSPEED_EDITION)) {
		gi = GeoIP_open_type(GEOIP_NETSPEED_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_NETSPEED_EDITION]);
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
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif
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
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_REGION_EDITION_REV0]);
		return;
	}

	region = GeoIP_region_by_name(gi, hostname);
	GeoIP_delete(gi);

	if (NULL == region) {
		RETURN_FALSE;
	}

	array_init(return_value);
#if PHP_MAJOR_VERSION >= 7
	add_assoc_string(return_value, "country_code", region->country_code);
	add_assoc_string(return_value, "region", region->region);
#else
	add_assoc_string(return_value, "country_code", region->country_code, 1);
	add_assoc_string(return_value, "region", region->region, 1);
#endif

	GeoIPRegion_delete(region);
}
/* }}} */

/* {{{ proto string geoip_isp_by_name( string hostname )
   Returns the ISP Name found in the GeoIP Database */
PHP_FUNCTION(geoip_isp_by_name)
{
	GeoIP * gi;
	char * hostname = NULL;
	char * isp;
#if PHP_MAJOR_VERSION >= 7
	size_t arglen;
#else
	int arglen;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &arglen) == FAILURE) {
		return;
	}
	
	if (GeoIP_db_avail(GEOIP_ISP_EDITION)) {
		gi = GeoIP_open_type(GEOIP_ISP_EDITION, GEOIP_STANDARD);
	}   else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Required database not available at %s.", GeoIPDBFileName[GEOIP_ISP_EDITION]);
		return;
	}

	isp = GeoIP_name_by_name(gi, hostname);
	GeoIP_delete(gi);
	if (isp == NULL) {
		RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	RETVAL_STRING(isp);
#else
	RETVAL_STRING(isp, 1);
#endif
	free(isp);
}

#if LIBGEOIP_VERSION >= 1004001
/* {{{ proto string geoip_region_name_by_code( string country_code, string region_code )
   Returns the region name for some country code and region code combo */
PHP_FUNCTION(geoip_region_name_by_code)
{
	char * country_code = NULL;
	char * region_code = NULL;
	const char * region_name;
#if PHP_MAJOR_VERSION >= 7
	size_t countrylen, regionlen;
#else
	int countrylen, regionlen;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &country_code, &countrylen, &region_code, &regionlen) == FAILURE) {
		return;
	}

	if (!countrylen || !regionlen) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "You need to specify the country and region codes.");
		RETURN_FALSE;
	}
	
	region_name = GeoIP_region_name_by_code(country_code, region_code);
	if (region_name == NULL) {
		RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING((char*)region_name);
#else
	RETURN_STRING((char*)region_name, 1);
#endif
}
/* }}} */
#endif

#if LIBGEOIP_VERSION >= 1004001
/* {{{ proto string geoip_time_zone_by_country_and_region( string country, string region )
   Returns the time zone for some country code and region code combo */
PHP_FUNCTION(geoip_time_zone_by_country_and_region)
{
	char * country = NULL;
	char * region = NULL;
	const char * timezone;
#if PHP_MAJOR_VERSION >= 7
	size_t countrylen, arg2len;
#else
	int countrylen, arg2len;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &country, &countrylen, &region, &arg2len) == FAILURE) {
		return;
	}

	if (!countrylen) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "You need to specify at least the country code.");
		RETURN_FALSE;
	}
	
	timezone = GeoIP_time_zone_by_country_and_region(country, region);
	if (timezone == NULL) {
		RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING((char*)timezone);
#else
	RETURN_STRING((char*)timezone, 1);
#endif
}
/* }}} */
#endif

#ifdef HAVE_CUSTOM_DIRECTORY
/* {{{ proto void geoip_setup_custom_directory( string directory )
   Sets the custom directory for GeoIP databases */
PHP_FUNCTION(geoip_setup_custom_directory)
{
	char * dir = NULL;
#if PHP_MAJOR_VERSION >= 7
	size_t dirlen;
#else
	int dirlen;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &dir, &dirlen) == FAILURE) {
		return;
	}

	GEOIP_G(set_runtime_custom_directory) = 1;
	geoip_change_custom_directory(dir);
}
/* }}} */
#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
