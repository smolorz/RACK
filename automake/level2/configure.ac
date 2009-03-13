#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

m4_define([ProjectName], [rack_level2])
m4_define([PrefixDefault], [/usr/local/rack_level2])
m4_define([SourceFile], [GNUmakefile.am])

AC_INIT([rack_level2],m4_normalize(m4_include([config/version])),[])

PROJNAME=ProjectName
AC_SUBST(PROJNAME)

CONFIG_RACK_CUR_VERSION_MAJOR=`expr $PACKAGE_VERSION : '\([0-9]*\)'`
CONFIG_RACK_CUR_VERSION_MINOR=`expr $PACKAGE_VERSION : '[0-9]*\.\([0-9]*\)'`
CONFIG_RACK_CUR_REVISION_LEVEL=`expr $PACKAGE_VERSION : '[0-9]*\.[0-9]*\.\([0-9]*\)'`
test "x$CONFIG_RACK_CUR_REVISION_LEVEL" = "x" && CONFIG_RACK_CUR_REVISION_LEVEL=0

AC_CONFIG_HEADERS([config/]ProjectName[_config_pre.h])
AC_CONFIG_SRCDIR(SourceFile)
AC_PREFIX_DEFAULT(PrefixDefault)
AC_CONFIG_AUX_DIR([config/autoconf])
AC_CONFIG_MACRO_DIR([config/m4])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_PROG_INSTALL

CFLAGS=${CFLAGS:--s}

dnl ======================================================================
dnl checking cc
dnl ======================================================================

AC_ARG_WITH(CC,
    AS_HELP_STRING([--with-cc=compiler], [use specific C compiler]),
    [case "$withval" in
    "" | y | ye | yes | n | no)
  AC_MSG_ERROR([*** You must supply an argument to --with-cc.]) ;;
    esac
    CC="$withval"])
AC_PROG_CC

AC_DEFINE_UNQUOTED(CONFIG_RACK_CUR_BUILD_STRING, "$build", [Build system alias])
RACK_BUILD_STRING="$build"
AC_DEFINE_UNQUOTED(CONFIG_RACK_CUR_HOST_STRING, "$host", [Host system alias])
RACK_HOST_STRING="$host"
AC_DEFINE_UNQUOTED(CONFIG_RACK_CUR_COMPILER, "`$CC -v 2>&1 | tail -n 1`" , [Compiler])

AM_INIT_AUTOMAKE([foreign no-exeext no-dist-gzip dist-bzip2])
AM_MAINTAINER_MODE
AC_PROG_LIBTOOL
AM_PROG_AS
AC_SUBST(LD)

AC_PATH_PROG(JAR, jar,)
AC_SUBST(JAR)

AC_C_INLINE
AC_HEADER_STDC

dnl ======================================================================
dnl checking target arch
dnl ======================================================================

AC_MSG_CHECKING([for target architecture])
case "$host" in
  i*86*-*)
    OWN_TARGET_ARCH=i386
    wanted_kernel_arch=CONFIG_X86
    ;;
  *)
    AC_MSG_ERROR([ *** Sorry - Unsupported architecture])
    ;;
esac
AC_MSG_RESULT([${OWN_TARGET_ARCH}])

dnl ======================================================================
dnl  Loading the project configuration.
dnl ======================================================================

OWN_KCONFIG_FILE=$srcdir/defconfig
kconfig_type=default

AC_ARG_WITH(kconfig-file,
    AS_HELP_STRING([--with-kconfig-file=<kconfig-file>], [KConfig parameter file]),
    [
        case "$withval" in
        "" | y | ye | yes | n | no)
          AC_MSG_ERROR([You must supply an argument to --with-kconfig-file.])
            ;;
        esac
        OWN_KCONFIG_FILE="$withval"
        kconfig_type=supplied
    ])

AC_MSG_CHECKING([for project's Kconfig file])
if test \! -f $OWN_KCONFIG_FILE; then
    AC_MSG_ERROR([Kconfig-file ${OWN_KCONFIG_FILE} not found -- --with-kconfig-file sets Kconfig file])
fi

. ${OWN_KCONFIG_FILE}
AC_MSG_RESULT([${OWN_KCONFIG_FILE} ($kconfig_type)])

dnl ======================================================================
dnl  RACK
dnl ======================================================================

AC_ARG_WITH(rack-dir,
    AS_HELP_STRING([--with-rack-dir=<RACK>], [Rack installation path]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-rack-dir.])
      ;;
    esac
    CONFIG_RACKDIR="$withval"
    ])

dnl -----------------------------------------------------------------
dnl  try to find rack-config
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([for rack-config])
RACK_CONFIG="${CONFIG_RACKDIR}/bin/rack-config"
if test -x "${RACK_CONFIG}"; then
  AC_MSG_RESULT([${RACK_CONFIG}])
else
  AC_MSG_ERROR([no rack-config found in ${CONFIG_RACKDIR}/bin])
fi

dnl ======================================================================
dnl  get common RACK values
dnl ======================================================================

dnl ---- rack CC ---------------------------------------------------------------

AC_MSG_CHECKING([for RACK CC])
RACK_CC="`${RACK_CONFIG} --cc`"
AC_MSG_RESULT([${RACK_CC}])
AC_SUBST(RACK_CC)

dnl ---- rack target arch ------------------------------------------------------

AC_MSG_CHECKING([for RACK target arch])
RACK_TARGET_ARCH="`${RACK_CONFIG} --arch`"
AC_MSG_RESULT([${RACK_TARGET_ARCH}])
AC_SUBST(RACK_TARGET_ARCH)

dnl ---- rack global cppflags --------------------------------------------------

AC_MSG_CHECKING([for RACK global cppflags])
RACK_GLOBAL_CPPFLAGS="`${RACK_CONFIG} --global-cppflags`"
AC_MSG_RESULT([${RACK_GLOBAL_CPPFLAGS}])

dnl ---- rack global ldflags ---------------------------------------------------

AC_MSG_CHECKING([for RACK global ldflags])
RACK_GLOBAL_LDFLAGS="`${RACK_CONFIG} --global-ldflags`"
AC_MSG_RESULT([${RACK_GLOBAL_LDFLAGS}])

dnl ---- rack supports ...------------------------------------------------------

RACK_SUPPORTS="`${RACK_CONFIG} --supports`"

dnl ======================================================================
dnl  get all supported packages / values
dnl ======================================================================

dnl ---- rack - level 2 ... X  support -----------------------------------------

dnl ... (see level 1 support) ...

dnl ---- rack - level 1 support ------------------------------------------------

AC_MSG_CHECKING([for RACK_LEVEL1 support])
RACK_LEVEL1_SUPPORT="`${RACK_CONFIG} --supports rack_level1`"
AC_MSG_RESULT([${RACK_LEVEL1_SUPPORT}])

RACK_LEVEL1_VERSION="`${RACK_CONFIG} --rack_level1-version`"
RACK_LEVEL1_PREFIX="`${RACK_CONFIG} --rack_level1-prefix`"
RACK_LEVEL1_CPPFLAGS="`${RACK_CONFIG} --cppflags rack_level1`"
RACK_LEVEL1_LDFLAGS="`${RACK_CONFIG} --ldflags rack_level1`"
RACK_LEVEL1_LIBS="`${RACK_CONFIG} --libs rack_level1`"

if test x"${RACK_LEVEL1_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for RACK_LEVEL1 version])
	AC_MSG_RESULT([${RACK_LEVEL1_VERSION}])
	AC_MSG_CHECKING([for RACK_LEVEL1 prefix])
	AC_MSG_RESULT([${RACK_LEVEL1_PREFIX}])
	AC_MSG_CHECKING([for RACK_LEVEL1 cppflags])
	AC_MSG_RESULT([${RACK_LEVEL1_CPPFLAGS}])
	AC_MSG_CHECKING([for RACK_LEVEL1 ldflags])
	AC_MSG_RESULT([${RACK_LEVEL1_LDFLAGS}])
	AC_MSG_CHECKING([for RACK_LEVEL1 libs])
	AC_MSG_RESULT([${RACK_LEVEL1_LIBS}])
fi

AC_SUBST(RACK_LEVEL1_VERSION)
AC_SUBST(RACK_LEVEL1_PREFIX)
AC_SUBST(RACK_LEVEL1_CPPFLAGS)
AC_SUBST(RACK_LEVEL1_LDFLAGS)
AC_SUBST(RACK_LEVEL1_LIBS)

AM_CONDITIONAL(RACK_LEVEL1_SUPPORT,[test x"${RACK_LEVEL1_SUPPORT}" = "xy"])


dnl ---- rack support ----------------------------------------------------------

AC_MSG_CHECKING([for RACK support])
RACK_SUPPORT="`${RACK_CONFIG} --supports rack`"
AC_MSG_RESULT([${RACK_SUPPORT}])

RACK_VERSION="`${RACK_CONFIG} --rack-version`"
RACK_PREFIX="`${RACK_CONFIG} --rack-prefix`"
RACK_CPPFLAGS="`${RACK_CONFIG} --cppflags rack`"
RACK_LDFLAGS="`${RACK_CONFIG} --ldflags rack`"
RACK_LIBS="`${RACK_CONFIG} --libs rack`"

if test x"${RACK_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for RACK version])
	AC_MSG_RESULT([${RACK_VERSION}])
	AC_MSG_CHECKING([for RACK prefix])
	AC_MSG_RESULT([${RACK_PREFIX}])
	AC_MSG_CHECKING([for RACK cppflags])
	AC_MSG_RESULT([${RACK_CPPFLAGS}])
	AC_MSG_CHECKING([for RACK ldflags])
	AC_MSG_RESULT([${RACK_LDFLAGS}])
	AC_MSG_CHECKING([for RACK libs])
	AC_MSG_RESULT([${RACK_LIBS}])
fi

AC_SUBST(RACK_VERSION)
AC_SUBST(RACK_PREFIX)
AC_SUBST(RACK_CPPFLAGS)
AC_SUBST(RACK_LDFLAGS)
AC_SUBST(RACK_LIBS)

AM_CONDITIONAL(RACK_SUPPORT,[test x"${RACK_SUPPORT}" = "xy"])
               
dnl ---- xenomai support -------------------------------------------------------

AC_MSG_CHECKING([for Xenomai support])
XENOMAI_SUPPORT="`${RACK_CONFIG} --supports xenomai`"
AC_MSG_RESULT([${XENOMAI_SUPPORT}])

XENOMAI_CPPFLAGS="`${RACK_CONFIG} --cppflags xenomai`"
XENOMAI_LDFLAGS="`${RACK_CONFIG} --ldflags xenomai`"
XENOMAI_LIBS="`${RACK_CONFIG} --libs xenomai`"

if test x"${XENOMAI_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for Xenomai cppflags])
	AC_MSG_RESULT([${XENOMAI_CPPFLAGS}])
	AC_MSG_CHECKING([for Xenomai ldflags])
	AC_MSG_RESULT([${XENOMAI_LDFLAGS}])
	AC_MSG_CHECKING([for Xenomai libs])
	AC_MSG_RESULT([${XENOMAI_LIBS}])
fi

AC_SUBST(XENOMAI_CPPFLAGS)
AC_SUBST(XENOMAI_LDFLAGS)
AC_SUBST(XENOMAI_LIBS)

AM_CONDITIONAL(XENOMAI_SUPPORT,[test x"${XENOMAI_SUPPORT}" = "xy"])

dnl ---- rtnet support ---------------------------------------------------------

AC_MSG_CHECKING([for RTnet support])
RTNET_SUPPORT="`${RACK_CONFIG} --supports rtnet`"
AC_MSG_RESULT([${RTNET_SUPPORT}])

RTNET_CPPFLAGS="`${RACK_CONFIG} --cppflags rtnet`"

if test x"${RTNET_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for RTnet cppflags])
	AC_MSG_RESULT([${RTNET_CPPFLAGS}])
fi

AC_SUBST(RTNET_CPPFLAGS)

AM_CONDITIONAL(RTNET_SUPPORT,[test x"${RTNET_SUPPORT}" = "xy"])

dnl ---- rtcan support ---------------------------------------------------------

AC_MSG_CHECKING([for CAN support])
RTCAN_SUPPORT="`${RACK_CONFIG} --supports rtcan`"
AC_MSG_RESULT([${RTCAN_SUPPORT}])

RTCAN_CPPFLAGS="`${RACK_CONFIG} --cppflags rtcan`"

if test x"${RTCAN_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for rtcan cppflags])
	AC_MSG_RESULT([${RTCAN_CPPFLAGS}])
fi

AC_SUBST(RTCAN_CPPFLAGS)

AM_CONDITIONAL(RTCAN_SUPPORT,[test x"${RTCAN_SUPPORT}" = "xy"])

dnl ---- libraw1394 support ----------------------------------------------------

AC_MSG_CHECKING([for libraw1394 support])
LIBRAW1394_SUPPORT="`${RACK_CONFIG} --supports libraw1394`"
AC_MSG_RESULT([${LIBRAW1394_SUPPORT}])

LIBRAW1394_CPPFLAGS="`${RACK_CONFIG} --cppflags libraw1394`"
LIBRAW1394_LDFLAGS="`${RACK_CONFIG} --ldflags libraw1394`"
LIBRAW1394_LIBS="`${RACK_CONFIG} --libs libraw1394`"

if test x"${LIBRAW1394_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for libraw1394 cppflags])
	AC_MSG_RESULT([${LIBRAW1394_CPPFLAGS}])
	AC_MSG_CHECKING([for libraw1394 ldflags])
	AC_MSG_RESULT([${LIBRAW1394_LDFLAGS}])
	AC_MSG_CHECKING([for libraw1394 libs])
	AC_MSG_RESULT([${LIBRAW1394_LIBS}])
fi

AC_SUBST(LIBRAW1394_CPPFLAGS)
AC_SUBST(LIBRAW1394_LDFLAGS)
AC_SUBST(LIBRAW1394_LIBS)

AM_CONDITIONAL(LIBRAW1394_SUPPORT,[test x"${LIBRAW1394_SUPPORT}" = "xy"])

dnl ---- libdc1394 support -----------------------------------------------------

AC_MSG_CHECKING([for libdc1394 support])
LIBDC1394_SUPPORT="`${RACK_CONFIG} --supports libdc1394`"
AC_MSG_RESULT([${LIBDC1394_SUPPORT}])

LIBDC1394_CPPFLAGS="`${RACK_CONFIG} --cppflags libdc1394`"
LIBDC1394_LDFLAGS="`${RACK_CONFIG} --ldflags libdc1394`"
LIBDC1394_LIBS="`${RACK_CONFIG} --libs libdc1394`"

if test x"${LIBDC1394_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for libdc1394 cppflags])
	AC_MSG_RESULT([${LIBDC1394_CPPFLAGS}])
	AC_MSG_CHECKING([for libdc1394 ldflags])
	AC_MSG_RESULT([${LIBDC1394_LDFLAGS}])
	AC_MSG_CHECKING([for libdc1394 libs])
	AC_MSG_RESULT([${LIBDC1394_LIBS}])
fi

AC_SUBST(LIBDC1394_CPPFLAGS)
AC_SUBST(LIBDC1394_LDFLAGS)
AC_SUBST(LIBDC1394_LIBS)

AM_CONDITIONAL(LIBDC1394_SUPPORT,[test x"${LIBDC1394_SUPPORT}" = "xy"])

dnl ---- libjpeg support -------------------------------------------------------

AC_MSG_CHECKING([for libjpeg support])
LIBJPEG_SUPPORT="`${RACK_CONFIG} --supports libjpeg`"
AC_MSG_RESULT([${LIBJPEG_SUPPORT}])

LIBJPEG_CPPFLAGS="`${RACK_CONFIG} --cppflags libjpeg`"
LIBJPEG_LDFLAGS="`${RACK_CONFIG} --ldflags libjpeg`"
LIBJPEG_LIBS="`${RACK_CONFIG} --libs libjpeg`"

if test x"${LIBJPEG_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for libjpeg cppflags])
	AC_MSG_RESULT([${LIBJPEG_CPPFLAGS}])
	AC_MSG_CHECKING([for libjpeg ldflags])
	AC_MSG_RESULT([${LIBJPEG_LDFLAGS}])
	AC_MSG_CHECKING([for libjpeg libs])
	AC_MSG_RESULT([${LIBJPEG_LIBS}])
fi

AC_SUBST(LIBJPEG_CPPFLAGS)
AC_SUBST(LIBJPEG_LDFLAGS)
AC_SUBST(LIBJPEG_LIBS)

AM_CONDITIONAL(LIBJPEG_SUPPORT,[test x"${LIBJPEG_SUPPORT}" = "xy"])

dnl ---- libpng support --------------------------------------------------------

AC_MSG_CHECKING([for libpng support])
LIBPNG_SUPPORT="`${RACK_CONFIG} --supports libpng`"
AC_MSG_RESULT([${LIBPNG_SUPPORT}])

LIBPNG_CPPFLAGS="`${RACK_CONFIG} --cppflags libpng`"
LIBPNG_LDFLAGS="`${RACK_CONFIG} --ldflags libpng`"
LIBPNG_LIBS="`${RACK_CONFIG} --libs libpng`"

if test x"${LIBPNG_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for libpng cppflags])
	AC_MSG_RESULT([${LIBPNG_CPPFLAGS}])
	AC_MSG_CHECKING([for libpng ldflags])
	AC_MSG_RESULT([${LIBPNG_LDFLAGS}])
	AC_MSG_CHECKING([for libpng libs])
	AC_MSG_RESULT([${LIBPNG_LIBS}])
fi

AC_SUBST(LIBPNG_CPPFLAGS)
AC_SUBST(LIBPNG_LDFLAGS)
AC_SUBST(LIBPNG_LIBS)

AM_CONDITIONAL(LIBPNG_SUPPORT,[test x"${LIBPNG_SUPPORT}" = "xy"])

dnl ---- opencv support -------------------------------------------------------

AC_MSG_CHECKING([for opencv support])
OPENCV_SUPPORT="`${RACK_CONFIG} --supports opencv`"
AC_MSG_RESULT([${OPENCV_SUPPORT}])

OPENCV_CPPFLAGS="`${RACK_CONFIG} --cppflags opencv`"
OPENCV_LDFLAGS="`${RACK_CONFIG} --ldflags opencv`"
OPENCV_LIBS="`${RACK_CONFIG} --libs opencv`"

if test x"${OPENCV_SUPPORT}" = xy; then
	AC_MSG_CHECKING([for opencv cppflags])
	AC_MSG_RESULT([${OPENCV_CPPFLAGS}])
	AC_MSG_CHECKING([for opencv ldflags])
	AC_MSG_RESULT([${OPENCV_LDFLAGS}])
	AC_MSG_CHECKING([for opencv libs])
	AC_MSG_RESULT([${OPENCV_LIBS}])
fi

AC_SUBST(OPENCV_CPPFLAGS)
AC_SUBST(OPENCV_LDFLAGS)
AC_SUBST(OPENCV_LIBS)

AM_CONDITIONAL(OPENCV_SUPPORT,[test x"${OPENCV_SUPPORT}" = "xy"])

dnl ======================================================================
dnl  common checks
dnl ======================================================================

dnl ---- RACK_LEVEL2 ... X  version ---------------------------------------------

dnl ... (see level 1 version) ...

dnl ---- RACK level 1 version ----------------------------------------------------

AC_MSG_CHECKING([for RACK_LEVEL1 version])
RACK_LEVEL1_VERSION_PRE="$RACK_LEVEL1_VERSION"

case "$RACK_LEVEL1_VERSION_PRE" in
  0.0.*) RACK_LEVEL1_VERSION_SUPPORT=rack_level1-0.0x ;;
  *)     AC_MSG_ERROR([
******************** 
Unsupported RACK_LEVEL1 version in directory $RACK_LEVEL1_PREFIX
Needed: 0.0.1 or greater
Available: $RACK_LEVEL1_VERSION_PRE
********************])  ;;
esac

AC_MSG_RESULT([${RACK_LEVEL1_VERSION_PRE}])

AC_DEFINE(CONFIG_RACK_LEVEL1_0X, 1, [We build for Rack-Level1 0.0.1 or newer])

dnl ---- RACK version ----------------------------------------------------

AC_MSG_CHECKING([for RACK version])
RACK_VERSION_PRE="$RACK_VERSION"

case "$RACK_VERSION_PRE" in
  0.1.*|0.2.*) RACK_VERSION_SUPPORT=rack-0.1x ;;
  *)     AC_MSG_ERROR([
******************** 
Unsupported RACK version in directory $RACK_PREFIX
Needed: 0.1.0 or greater
Available: $RACK_VERSION_PRE
********************])  ;;
esac

AC_MSG_RESULT([${RACK_VERSION_PRE}])

AC_DEFINE(CONFIG_RACK_0X, 1, [We build for Rack 0.1.0 or newer])

dnl -----------------------------------------------------------------
dnl  checking target architectures
dnl -----------------------------------------------------------------

if test x"${RACK_TARGET_ARCH}" != x"${OWN_TARGET_ARCH}"; then
    AC_MSG_ERROR([
********************
Target architectures are not the same
RACK        : ${RACK_TARGET_ARCH}
This Project: ${OWN_TARGET_ARCH}
********************])
fi

dnl ======================================================================
dnl  current project
dnl ======================================================================

RACK_SUPPORTS="${PROJNAME} ${RACK_SUPPORTS}"
AC_SUBST(RACK_SUPPORTS)

RACK_CUR_CPPFLAGS="-I\${top_srcdir} -I\${top_builddir}"
RACK_CUR_LDFLAGS="-L\$(top_builddir)/main/lib/.libs"
RACK_CUR_LIBS="-l${PROJNAME}"

AC_SUBST(RACK_CUR_LIBS)

dnl ======================================================================
dnl  set flags / libs
dnl ======================================================================

dnl ---- cppflags --------------------------------------------------------------

GLOBAL_CPPFLAGS_ADD=""
RACK_GLOBAL_CPPFLAGS="${GLOBAL_CPPFLAGS_ADD} ${RACK_GLOBAL_CPPFLAGS}"
AC_SUBST(RACK_GLOBAL_CPPFLAGS)

dnl --- !!! Add level 2 ..X CPPFLAGS here !!! ---

CPPFLAGS="\
${XENOMAI_CPPFLAGS} \
${RACK_GLOBAL_CPPFLAGS} \
${RACK_CUR_CPPFLAGS} \
${RACK_LEVEL1_CPPFLAGS} \
${RACK_CPPFLAGS} \
${CPPFLAGS}"

AC_MSG_CHECKING([for cppflags])
AC_MSG_RESULT([${CPPFLAGS}])

dnl ---- ldflags ---------------------------------------------------------------

GLOBAL_LDFLAGS_ADD=""
RACK_GLOBAL_LDFLAGS="${GLOBAL_LDFLAGS_ADD} ${RACK_GLOBAL_LDFLAGS}"
AC_SUBST(RACK_GLOBAL_LDFLAGS)

dnl --- !!! Add level 2 ..X LDFLAGS here !!! ---

LDFLAGS="\
${XENOMAI_LDFLAGS} \
${RACK_GLOBAL_LDFLAGS} \
${RACK_CUR_LDFLAGS} \
${RACK_LEVEL1_LDFLAGS} \
${RACK_LDFLAGS} \
${LDFLAGS}"

AC_MSG_CHECKING([for ldflags])
AC_MSG_RESULT([${LDFLAGS}])

dnl ---- libs ------------------------------------------------------------------

dnl --- !!! Add level 2 ..X LIBS here !!! ---

LIBS="\
${XENOMAI_LIBS} \
${RACK_LEVEL1_LIBS} \
${RACK_LIBS}"

RACK_LEVEL2_LIBS=${RACK_CUR_LIBS}
AC_SUBST(RACK_LEVEL2_LIBS)

AC_MSG_CHECKING([for common libs])
AC_MSG_RESULT([${LIBS}])

dnl ======================================================================
dnl  clean component lists 
dnl ======================================================================

RTNET_COMPONENTS=""
RTCAN_COMPONENTS=""
LIBRAW1394_COMPONENTS=""
LIBDC1394_COMPONENTS=""
LIBJPEG_COMPONENTS=""
LIBPNG_COMPONENTS=""
OPENCV_COMPONENTS=""

dnl ======================================================================
dnl    JAVA Part
dnl ======================================================================

dnl -----------------------------------------------------------------
dnl  java gui
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build java gui])
AC_ARG_ENABLE(java-gui,
    AS_HELP_STRING([--enable-java-gui], [building java gui]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_JAVA_GUI=y ;;
        *) CONFIG_RACK_JAVA_GUI=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_JAVA_GUI:-n}])

dnl -----------------------------------------------------------------
dnl  javac & jar
dnl -----------------------------------------------------------------

if test x"$CONFIG_RACK_JAVA_GUI" = "xy"; then

    AC_MSG_CHECKING([javac])
    JAVAC="`which javac`"
    if test \! -x ${JAVAC}; then
        AC_MSG_ERROR([*** can't find javac])
    else
        AC_MSG_RESULT([${JAVAC}])
    fi

    AC_MSG_CHECKING([jar])
    JAR="`which jar`"
    if test \! -x ${JAR}; then
        AC_MSG_ERROR([*** can't find jar])
    else
        AC_MSG_RESULT([${JAR}])
    fi
fi

dnl -----------------------------------------------------------------

AM_CONDITIONAL(CONFIG_RACK_JAVA_GUI,[test "$CONFIG_RACK_JAVA_GUI" = "y"])
if test "$CONFIG_RACK_JAVA_GUI" = "y"; then
    AC_DEFINE(CONFIG_RACK_JAVA_GUI,1,[building java gui])
fi
AC_SUBST(CONFIG_RACK_JAVA_GUI)

dnl -----------------------------------------------------------------
dnl get rack jar files
dnl -----------------------------------------------------------------

dnl ----- rack_level2 ... X --------------------------------------

dnl ...

dnl ----- rack_level1 --------------------------------------------

RACK_LEVEL1_JARFILE="${RACK_LEVEL1_PREFIX}/gui/rack_level1.jar"

if test "${CONFIG_RACK_JAVA_GUI}" = "y"; then
	AC_MSG_CHECKING([for rack_level1 jar file])
	if test \! -f ${RACK_LEVEL1_JARFILE}; then
		AC_MSG_ERROR([*** Can't find RACK_LEVEL1's jar file])
	fi
	AC_MSG_RESULT([${RACK_LEVEL1_JARFILE}])
fi

dnl ----- rack --------------------------------------------

RACK_JARFILE="${RACK_PREFIX}/gui/rack.jar"

if test "${CONFIG_RACK_JAVA_GUI}" = "y"; then
	AC_MSG_CHECKING([for rack jar file])
	if test \! -f ${RACK_JARFILE}; then
		AC_MSG_ERROR([*** Can't find RACK's jar file])
	fi
	AC_MSG_RESULT([${RACK_JARFILE}])
fi

dnl --- !!! Add level 2 ..X jarfiles here !!! ---

RACK_CLASSPATH_ADD="\
${RACK_LEVEL1_JARFILE}:\
${RACK_JARFILE}:\
\$(JAVAROOT):\
\$(srcdir)/\$(JAVAROOT)"

AC_SUBST(RACK_CLASSPATH_ADD)

dnl -----------------------------------------------------------------
dnl  drivers - dummy java gui
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build dummy driver java gui])
AC_ARG_ENABLE(dummy-driver-java-gui,
    AS_HELP_STRING([--enable-dummy-driver-java-gui], [building dummy driver java gui]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_DUMMY_DRV_JAVA_GUI=y ;;
        *) CONFIG_RACK_DUMMY_DRV_JAVA_GUI=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_DUMMY_DRV_JAVA_GUI:-n}])
AM_CONDITIONAL(CONFIG_RACK_DUMMY_DRV_JAVA_GUI,[test "$CONFIG_RACK_DUMMY_DRV_JAVA_GUI" = "y"])
if test "$CONFIG_RACK_DUMMY_DRV_JAVA_GUI" = "y"; then
    AC_DEFINE(CONFIG_RACK_DUMMY_DRV_JAVA_GUI,1,[building dummy driver java gui])
fi

dnl -----------------------------------------------------------------
dnl  navigation - dummy java gui
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build dummy navigation java gui])
AC_ARG_ENABLE(dummy-navigation-java-gui,
    AS_HELP_STRING([--enable-dummy-navigation-java-gui], [building dummy navigation java gui]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_DUMMY_NAV_JAVA_GUI=y ;;
        *) CONFIG_RACK_DUMMY_NAV_JAVA_GUI=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_DUMMY_NAV_JAVA_GUI:-n}])
AM_CONDITIONAL(CONFIG_RACK_DUMMY_NAV_JAVA_GUI,[test "$CONFIG_RACK_DUMMY_NAV_JAVA_GUI" = "y"])
if test "$CONFIG_RACK_DUMMY_NAV_JAVA_GUI" = "y"; then
    AC_DEFINE(CONFIG_RACK_DUMMY_NAV_JAVA_GUI,1,[building dummy navigation java gui])
fi

dnl -----------------------------------------------------------------
dnl  perception - dummy java gui
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build dummy perception java gui])
AC_ARG_ENABLE(dummy-perception-java-gui,
    AS_HELP_STRING([--enable-dummy-perception-java-gui], [building dummy perception java gui]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_DUMMY_PER_JAVA_GUI=y ;;
        *) CONFIG_RACK_DUMMY_PER_JAVA_GUI=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_DUMMY_PER_JAVA_GUI:-n}])
AM_CONDITIONAL(CONFIG_RACK_DUMMY_PER_JAVA_GUI,[test "$CONFIG_RACK_DUMMY_PER_JAVA_GUI" = "y"])
if test "$CONFIG_RACK_DUMMY_PER_JAVA_GUI" = "y"; then
    AC_DEFINE(CONFIG_RACK_DUMMY_PER_JAVA_GUI,1,[building dummy perception java gui])
fi


dnl -----------------------------------------------------------------
dnl  ...
dnl -----------------------------------------------------------------

dnl ======================================================================
dnl    project modules
dnl ======================================================================

dnl -----------------------------------------------------------------
dnl  drivers
dnl -----------------------------------------------------------------

dnl ------- driver dummy --------------

AC_MSG_CHECKING([build driver dummy])
AC_ARG_ENABLE(driver-dummy,
    AS_HELP_STRING([--enable-driver-dummy], [building driver dummy]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_DUMMY_DRV=y ;;
        *) CONFIG_RACK_DUMMY_DRV=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_DUMMY_DRV:-n}])
AM_CONDITIONAL(CONFIG_RACK_DUMMY_DRV,[test "$CONFIG_RACK_DUMMY_DRV" = "y"])
if test "$CONFIG_RACK_DUMMY_DRV" = "y"; then
    AC_DEFINE(CONFIG_RACK_DUMMY_DRV,1,[building driver-dummy])
fi

dnl -----------------------------------------------------------------
dnl  navigation
dnl -----------------------------------------------------------------

dnl ------- navigation dummy --------------

AC_MSG_CHECKING([build navigation dummy])
AC_ARG_ENABLE(navigation-dummy,
    AS_HELP_STRING([--enable-navigation-dummy], [building navigation dummy]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_DUMMY_NAV=y ;;
        *) CONFIG_RACK_DUMMY_NAV=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_DUMMY_NAV:-n}])
AM_CONDITIONAL(CONFIG_RACK_DUMMY_NAV,[test "$CONFIG_RACK_DUMMY_NAV" = "y"])
if test "$CONFIG_RACK_DUMMY_NAV" = "y"; then
    AC_DEFINE(CONFIG_RACK_DUMMY_NAV,1,[building navigation dummy])
fi

dnl -----------------------------------------------------------------
dnl  perception
dnl -----------------------------------------------------------------

dnl ------- perception dummy --------------

AC_MSG_CHECKING([build perception dummy])
AC_ARG_ENABLE(perception-dummy,
    AS_HELP_STRING([--enable-perception-dummy], [building perception dummy]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_DUMMY_PER=y ;;
        *) CONFIG_RACK_DUMMY_PER=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_DUMMY_PER:-n}])
AM_CONDITIONAL(CONFIG_RACK_DUMMY_PER,[test "$CONFIG_RACK_DUMMY_PER" = "y"])
if test "$CONFIG_RACK_DUMMY_PER" = "y"; then
    AC_DEFINE(CONFIG_RACK_DUMMY_PER,1,[building perception dummy])
fi


dnl ======================================================================
dnl  support checks
dnl ======================================================================

AC_MSG_CHECKING([rtnet support])

if test x"${RTNET_COMPONENTS}" = x; then
	AC_MSG_RESULT([not needed])
else
    if test x"${RTNET_SUPPORT}" = xy; then
		AC_MSG_RESULT([supported])
	else
		AC_MSG_ERROR([not supported !
********************
Enable RTnet support in RACK or disable modules 
${RTNET_COMPONENTS}
********************])
    fi
fi

AC_MSG_CHECKING([rtcan support])

if test x"${RTCAN_COMPONENTS}" = x; then
	AC_MSG_RESULT([not needed])
else
    if test x"${RTCAN_SUPPORT}" = xy; then
		AC_MSG_RESULT([supported])
	else
		AC_MSG_ERROR([not supported.
********************
Enable CAN support in RACK or disable modules 
${RTCAN_COMPONENTS}
********************])
    fi
fi

AC_MSG_CHECKING([libraw1394 support])

if test x"${LIBRAW1394_COMPONENTS}" = x; then
	AC_MSG_RESULT([not needed])
else
    if test x"${LIBRAW1394_SUPPORT}" = xy; then
		AC_MSG_RESULT([supported])
	else
		AC_MSG_ERROR([not supported.
********************
Enable libraw1394 support in RACK or disable modules 
${LIBRAW1394_COMPONENTS}
********************])
    fi
fi

AC_MSG_CHECKING([libdc1394 support])

if test x"${LIBDC1394_COMPONENTS}" = x; then
	AC_MSG_RESULT([not needed])
else
    if test x"${LIBDC1394_SUPPORT}" = xy; then
		AC_MSG_RESULT([supported])
	else
		AC_MSG_ERROR([not supported.
********************
Enable libdc1394 support in RACK or disable modules 
${LIBDC1394_COMPONENTS}
********************])
    fi
fi

AC_MSG_CHECKING([libjpeg support])

if test x"${LIBJPEG_COMPONENTS}" = x; then
	AC_MSG_RESULT([not needed])
else
    if test x"${LIBJPEG_SUPPORT}" = xy; then
		AC_MSG_RESULT([supported])
	else
		AC_MSG_ERROR([not supported.
********************
Enable libjpeg support in RACK or disable modules 
${LIBJPEG_COMPONENTS}
********************])
    fi
fi

AC_MSG_CHECKING([libpng support])

if test x"${LIBPNG_COMPONENTS}" = x; then
	AC_MSG_RESULT([not needed])
else
    if test x"${LIBPNG_SUPPORT}" = xy; then
		AC_MSG_RESULT([supported])
	else
		AC_MSG_ERROR([not supported.
********************
Enable libpng support in RACK or disable modules 
${LIBPNG_COMPONENTS}
********************])
    fi
fi

AC_MSG_CHECKING([opencv support])

if test x"${OPENCV_COMPONENTS}" = x; then
	AC_MSG_RESULT([not needed])
else
    if test x"${OPENCV_SUPPORT}" = xy; then
		AC_MSG_RESULT([supported])
	else
		AC_MSG_ERROR([not supported.
********************
Enable opencv support in RACK or disable modules 
${OPENCV_COMPONENTS}
********************])
    fi
fi

dnl ======================================================================
dnl             crosscompiling
dnl ======================================================================

AC_MSG_CHECKING([for CROSS_COMPILE])
if test "${CROSS_COMPILE}" = ""; then
    CROSS_COMPILE="`echo ${CC} | sed s/gcc//`"

    if test "${CROSS_COMPILE}gcc" \!= "${CC}"; then
        CROSS_COMPILE=""
        AC_MSG_RESULT([Cannot identify CROSS_COMPILE - assuming none - set manually if needed])
    else
        if test "${CROSS_COMPILE}" = ""; then
            AC_MSG_RESULT([none])
        else
            AC_MSG_RESULT([${CROSS_COMPILE}])
        fi
    fi
else
    AC_MSG_RESULT([${CROSS_COMPILE}])
fi
AC_SUBST(CROSS_COMPILE)

dnl ======================================================================
dnl             generate output
dnl ======================================================================

AC_CONFIG_FILES([GNUmakefile \
     \
     config/GNUmakefile \
     config/kconfig/GNUmakefile \
     config/kconfig/lxdialog/GNUmakefile \
     \
     drivers/GNUmakefile \
     drivers/dummy/GNUmakefile \
     \
     main/GNUmakefile \
     main/defines/GNUmakefile \
     main/lib/GNUmakefile \
     main/naming/GNUmakefile \
     \
     navigation/GNUmakefile \
     navigation/dummy/GNUmakefile \
     \
     perception/GNUmakefile \
     perception/dummy/GNUmakefile \
     \
     scripts/GNUmakefile \
     scripts/rack-config \
     \
                ])

if test x"$CONFIG_RACK_JAVA_GUI" = x"y"; then
AC_CONFIG_COMMANDS([java-link], [if test ! -e ProjectName; then $LN_S . ProjectName; fi], [LN_S="$LN_S"])
else
AC_CONFIG_COMMANDS([java-unlink], [if test -e ProjectName; then rm -rf ProjectName; fi])
fi

AC_OUTPUT

dnl ======================================================================
dnl generate ${PROJNAME}_config.h
dnl ======================================================================

AC_MSG_NOTICE([creating ]ProjectName[_config.h])
sed -e "s/ PACKAGE/ RACK_CUR_PACKAGE/" -e "s/ VERSION/ RACK_CUR_VERSION/" \
    config/${PROJNAME}_config_pre.h > ${PROJNAME}_config.h.new
if cmp -s ${PROJNAME}_config.h.new ${PROJNAME}_config.h; then
    rm ${PROJNAME}_config.h.new
    AC_MSG_NOTICE(ProjectName[_config.h is unchanged])
else
    mv ${PROJNAME}_config.h.new ${PROJNAME}_config.h
fi
