#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([MuScalpel], [1.0], [alexandru.marginean.13@ucl.ac.uk])
AM_INIT_AUTOMAKE(pippo,2.6)
AC_OUTPUT(Makefile src/Makefile doc/Makefile examples/Makefile man/Makefile scripts/Makefile tools/Makefile)
AC_CONFIG_SRCDIR([.])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT(../install)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd mkdir strdup strstr])


AC_CANONICAL_HOST
AC_DEFINE([OS],[],[Current operating system.])
AC_DEFINE([Linux], 0, [Linux])
AC_DEFINE([MacOS], 1, [MacOS])
AC_DEFINE([Windows], 2, [Windows])

# Detect the target system
case "${host_os}" in
	    linux*)
	AC_DEFINE(OS, Linux)
	        ;;
		    cygwin*|mingw*)
	AC_DEFINE(OS, Windows)
	        ;;
		    darwin*)
	AC_DEFINE(OS, MacOS)
	        ;;
		    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
	        ;;
esac


AM_CONDITIONAL([Linux], [test x$OS = x$Linux])

AC_OUTPUT
