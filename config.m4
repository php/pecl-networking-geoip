dnl $Id$
dnl config.m4 for extension geoip

dnl Contributed by Jonathan Whiteman of cyberflowsolutions.com

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(geoip, for geoip support,
dnl Make sure that the comment is aligned:
[  --with-geoip             Include GeoIP support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(geoip, whether to enable geoip support,
dnl Make sure that the comment is aligned:
dnl [  --enable-geoip           Enable geoip support])

if test "$PHP_GEOIP" != "no"; then
  dnl Write more examples of tests here...

  # --with-geoip -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/GeoIP.h"  # you most likely want to change this
  if test -r $PHP_GEOIP/$SEARCH_FOR; then # path given as parameter
    GEOIP_DIR=$PHP_GEOIP
  else # search default path list
    AC_MSG_CHECKING([for geoip files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        GEOIP_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$GEOIP_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the geoip distribution])
  fi

  # --with-geoip -> add include path
  PHP_ADD_INCLUDE($GEOIP_DIR/include)

  # --with-geoip -> check for lib and symbol presence
  LIBNAME=GeoIP # you may want to change this
  LIBSYMBOL=GeoIP_open # you most likely want to change this

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $GEOIP_DIR/lib, GEOIP_SHARED_LIBADD)
    AC_DEFINE(HAVE_GEOIPLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong geoip lib version or lib not found])
  ],[
    -L$GEOIP_DIR/lib -lm -ldl
  ])

  PHP_SUBST(GEOIP_SHARED_LIBADD)

  PHP_NEW_EXTENSION(geoip, geoip.c, $ext_shared)
fi

