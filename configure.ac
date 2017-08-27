#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(ruse, 1.0, jan.moren@oist.jp)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/proc.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC_C99


# Checks for libraries.
AC_CHECK_LIB([m], [cos], [], [AC_MSG_ERROR([libm not found.])])
AC_CHECK_LIB([rt], [clock_gettime], [], [AC_MSG_ERROR([librt not found.])])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h libgen.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime getpagesize strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 util/Makefile])
AC_OUTPUT
