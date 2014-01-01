#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Extra macros.
m4_include([m4/ax_pthread.m4])

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
AX_PTHREAD

# Checks for libraries.
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.32.3])

# Checks for header files.
AC_CHECK_HEADERS([syslog.h sys/time.h arpa/inet.h netinet/in.h sys/socket.h netdb.h])

# Hiredis is not distributed with a .pc file, so use PKG_CHECK_MODULES is
# out of question
AC_CHECK_HEADERS([hiredis/hiredis.h])

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
AC_CHECK_FUNCS([gettimeofday strdup])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT
