AC_PREREQ(2.61)
AC_INIT([simcom-demo], 1.0.0)
AM_INIT_AUTOMAKE([-Wall gnu foreign])
AM_MAINTAINER_MODE
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AS
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([QMI], [qmi])
AC_SUBST([QMI_CFLAGS])
AC_SUBST([QMI_LIBS])

PKG_CHECK_MODULES([QMIFRAMEWORK], [qmi-framework])
AC_SUBST([QMIFRAMEWORK_CFLAGS])
AC_SUBST([QMIFRAMEWORK_LIBS])

AC_ARG_WITH([kernel],
      AC_HELP_STRING([--with-kernel=@<:@dir@:>@],
         [Specify the location of the Linux kernel headers]),
      [kerneldir=$withval],
      with_kernel=no)

if test "x$with_kernel" != "xno"; then
   CFLAGS="${CFLAGS} -I${kerneldir}/include -I${kerneldir}/arch/arm/include"
fi
AC_ARG_WITH([glib],
      AC_HELP_STRING([--with-glib],
         [enable glib, building HLOS systems which use glib]))

if (test "x${with_glib}" = "xyes"); then
        AC_DEFINE(ENABLE_USEGLIB, 1, [Define if HLOS systems uses glib])
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GThread >= 2.16 is required))
        PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GLib >= 2.16 is required))
        GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
        GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

        AC_SUBST(GLIB_CFLAGS)
        AC_SUBST(GLIB_LIBS)
fi

AM_CONDITIONAL(USE_GLIB, test "x${with_glib}" = "xyes")
AS_CASE([$host],
    [arm*], [ARM=yes],
    [ARM=no]
)

AM_CONDITIONAL(ARM, [test "x$ARM" = "xyes"])
AC_SUBST([CFLAGS])
AC_SUBST([CC])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
