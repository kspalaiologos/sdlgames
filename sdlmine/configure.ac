AC_PREREQ([2.71])
AC_INIT([sdlmine], [1.0], [kspalaiologos@gmail.com])
AC_CONFIG_SRCDIR([minesweeper.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux])
AC_COPYRIGHT([Copyright (C) 2023 Kamila Szewczyk; licensed under the terms of the GNU GPLv3 license.])

if test "`uname -s`" = "NetBSD"; then
	AM_CONDITIONAL(NETBSD, true)
	CFLAGS="-I/usr/pkg/include -L/usr/pkg/lib -Wl,-R/usr/pkg/lib"
else
	AM_CONDITIONAL(NETBSD, false)
fi

AM_INIT_AUTOMAKE([foreign subdir-objects])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(m, sqrt, , AC_MSG_ERROR([libm missing.]))
AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR([libpthread missing.]))
AC_CHECK_LIB(SDL2, SDL_Init, , AC_MSG_ERROR([libSDL2 missing.]))
AC_CHECK_LIB(SDL2_ttf, TTF_Init, , AC_MSG_ERROR([libSDL2_ttf missing.]))
AC_CHECK_LIB(SDL2_mixer, Mix_Init, , AC_MSG_ERROR([libSDL2_mixer missing.]))

# Checks for header files.
AC_CHECK_HEADERS([SDL2/SDL.h], , AC_MSG_ERROR([SDL2 missing.]))

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX([17], [ext], [])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

./gen_assets.sh