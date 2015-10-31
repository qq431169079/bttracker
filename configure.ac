#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Autoconf setup.
AC_PREREQ([2.59])
AC_INIT([bttracker], [0.0.1])
AC_CONFIG_SRCDIR([src/bttracker.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for programs.
AC_PROG_CC_C99
AC_PROG_RANLIB
AM_PROG_CC_C_O
AM_PROG_AR

# Checks for libraries.
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.32.3])
PKG_CHECK_MODULES([HIREDIS], [hiredis >= 0.13.1])

# Hiredis library:
# This library is not distributed with a .pc file, so we cannot use
# PKG_CHECK_MODULES

# Checks for header files.
AC_CHECK_HEADERS([syslog.h sys/time.h arpa/inet.h netinet/in.h sys/socket.h netdb.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDC
AC_HEADER_STDBOOL

AC_TYPE_INT8_T
AC_TYPE_INT64_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT
