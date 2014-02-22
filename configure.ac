#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([remy], [0.1], [remy@mit.edu])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/remy.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

AC_PATH_PROG([PROTOC], [protoc], [])
AS_IF([test x"$PROTOC" = x],
  [AC_MSG_ERROR([cannot find protoc, the Protocol Buffers compiler])])

# Checks for libraries.
PKG_CHECK_MODULES([protobuf], [protobuf])

# Add picky CXXFLAGS
CPPFLAGS="-std=c++11 -pthread"
PICKY_CXXFLAGS="-pedantic -Wall -Wextra -Weffc++ -Werror"
AC_SUBST([PICKY_CXXFLAGS])

# Checks for libraries.
PKG_CHECK_MODULES([GL], [gl])
PKG_CHECK_MODULES([GLFW], [glfw3])
PKG_CHECK_MODULES([GLEW], [glew])
PKG_CHECK_MODULES([GLU], [glu])
PKG_CHECK_MODULES([PANGOCAIRO], [pangocairo])

# Checks for header files.
AC_LANG_PUSH(C++)

AC_CHECK_HEADERS([algorithm array cassert boost/accumulators/accumulators.hpp \
boost/accumulators/statistics/median.hpp \
boost/accumulators/statistics/stats.hpp boost/functional/hash.hpp \
boost/random/exponential_distribution.hpp boost/random/mersenne_twister.hpp \
boost/random/poisson_distribution.hpp \
boost/random/uniform_int_distribution.hpp \
boost/random/uniform_real_distribution.hpp fcntl.h future cmath queue \
cstdio string sys/stat.h sys/types.h ctime tuple unistd.h unordered_map \
utility vector], [], [AC_MSG_ERROR([Missing header file])])

AC_CHECK_LIB(jemalloc, malloc, [], [AC_MSG_ERROR([Need jemalloc])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_LANG_POP(C++)

# Checks for library functions.

AC_CONFIG_FILES([Makefile protobufs/Makefile graph/Makefile src/Makefile])
AC_OUTPUT
