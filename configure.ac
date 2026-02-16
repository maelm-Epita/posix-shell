AC_INIT([42sh], [1.0])

AM_INIT_AUTOMAKE([subdir-objects] [foreign])
AM_SILENT_RULES([no])

AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_CC

AC_ARG_ENABLE([debug], 
              AS_HELP_STRING([--enable-debug], [Build with debug flags (-g -O0)]),
              [enable_debug=$enableval], [enable_debug=no])
AC_ARG_ENABLE([coverage], 
              AS_HELP_STRING([--enable-coverage], [Build with coverage flags (--coverage -g -O0)]),
              [enable_coverage=$enableval], [enable_coverage=no])
AC_ARG_ENABLE([asan], 
              AS_HELP_STRING([--enable-asan], [Build with asan flags (-g -O0 -fsanitize=address)]),
              [enable_asan=$enableval], [enable_asan=no])

WERROR_FLAG="-Werror"

AS_IF([test "x$enable_debug" = "xyes"], [
  CFLAGS="-g -O0"
  WERROR_FLAG=""
])

AS_IF([test "x$enable_coverage" = "xyes"], [
  CFLAGS="--coverage -O0 -g"
  LDFLAGS="--coverage"
  WERROR_FLAG=""
])

AS_IF([test "x$enable_asan" = "xyes"], [
  CFLAGS="-g -O0 -fsanitize=address -fno-omit-frame-pointer"
  LDFLAGS="-fsanitize=address"
  WERROR_FLAG=""
])

AC_SUBST([WERROR_FLAG])


#AX_COMPILER_FLAGS([], [], [], [-std=c99 -pedantic -Werror -Wall -Wextra -Wvla])
AC_CONFIG_FILES([
                 Makefile
                 src/Makefile
                 src/ast/Makefile
                 src/parser/Makefile
                 src/lexer/Makefile
                 src/execution/Makefile
                 src/io/Makefile
                 src/shell/Makefile
                 src/token/Makefile
                 src/expansion/Makefile
                 src/builtins/Makefile
                 src/variables/Makefile
                 src/functions/Makefile
                 src/utils/Makefile
		 src/matching/Makefile
		 tests/Makefile
                 ])

AC_OUTPUT
