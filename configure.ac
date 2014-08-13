#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([libregex], [1.0.0], [jackie.CPlusPlus@gmail.com])
AC_CONFIG_SRCDIR([src/regcomp.c])
AC_CONFIG_HEADERS([include/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Setup AM
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset regcomp strchr])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 include/Makefile
                 demo/Makefile
                 doc/Makefile])

AC_OUTPUT
