#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(ruse, 2.0, jan.moren@oist.jp)
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

AC_ARG_ENABLE([pss], 
        AS_HELP_STRING([--enable-pss] [Use PSS rather than RSS for resident memory estimation]),  
        [case "${enableval}" in
            yes) pss=true ;;
            no)  pss=false ;;
            *) AC_MSG_ERROR([bad value ${enableval} for --enable-pss]) ;;
        esac],
        [pss=false])


if [test "x$pss" = "xtrue"]
then
    AC_DEFINE([ENABLE_PSS], [], ["use PSS for memory estimation"])
    AC_MSG_NOTICE([using PSS for memory estimation])
else
    AC_MSG_NOTICE([using RSS for memory estimation])
fi
AC_ARG_WITH([extras],
	[AS_HELP_STRING([--with-extras], [Build extra utils useful for testing.])],
	[],
	[with_extras=no])

AS_IF([test "x$with_extras" != xno],
	[
	AM_CONDITIONAL(WITH_EXTRAS, true)
	], [
	AM_CONDITIONAL(WITH_EXTRAS, false)
	])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 util/Makefile])
AC_OUTPUT
