#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([remy], [0.1], [remy@mit.edu])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/remy.cc])
AC_CONFIG_HEADERS([config.h])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

AC_PATH_PROG([PROTOC], [protoc], [])
AS_IF([test x"$PROTOC" = x],
  [AC_MSG_ERROR([cannot find protoc, the Protocol Buffers compiler])])

# Checks for libraries.
PKG_CHECK_MODULES([protobuf], [protobuf])

# Add compiler flags
CXX11_FLAGS="-std=c++11 -pthread"
AC_SUBST([CXX11_FLAGS])

PICKY_CXXFLAGS="-Wall -Wpedantic -Wextra -Weffc++ -Werror"
AC_SUBST([PICKY_CXXFLAGS])

AC_ARG_ENABLE([graph],
  [AS_HELP_STRING([--enable-graph],
    [Enable live graph (ratatouille) @<:@no@:>@])],
  [build_graph="yes"],
  [build_graph="no"])
AM_CONDITIONAL([BUILD_GRAPH], [test x"$build_graph" != xno])

# Checks for libraries.
AM_COND_IF( [BUILD_GRAPH],
[
  PKG_CHECK_MODULES([GL], [gl])
  PKG_CHECK_MODULES([GLFW], [glfw3])
  PKG_CHECK_MODULES([GLEW], [glew])
  PKG_CHECK_MODULES([GLU], [glu])
  PKG_CHECK_MODULES([PANGOCAIRO], [pangocairo])
  PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0])
] )

# Checks for header files.
AC_LANG_PUSH(C++)

AC_CHECK_HEADERS([boost/accumulators/accumulators.hpp \
boost/accumulators/statistics/median.hpp \
boost/accumulators/statistics/stats.hpp boost/functional/hash.hpp], [], [AC_MSG_ERROR([Missing boost headers (need to install libboost-dev?)])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_LANG_POP(C++)

# Checks for library functions.

AC_CONFIG_FILES([Makefile protobufs/Makefile graph/Makefile src/Makefile tests/Makefile scripts/Makefile])
AC_OUTPUT
