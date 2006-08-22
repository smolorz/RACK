#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

m4_define([ProjectName], [rack])
m4_define([PrefixDefault], [/usr/local/rack])
m4_define([SourceFile], [main/module/rack_module.cpp])

AC_INIT([rack],m4_normalize(m4_include([config/version])),[])

PROJNAME=ProjectName
AC_SUBST(PROJNAME)

CONFIG_RACK_VERSION_MAJOR=`expr $PACKAGE_VERSION : '\([0-9]*\)'`
CONFIG_RACK_VERSION_MINOR=`expr $PACKAGE_VERSION : '[0-9]*\.\([0-9]*\)'`
CONFIG_RACK_REVISION_LEVEL=`expr $PACKAGE_VERSION : '[0-9]*\.[0-9]*\.\([0-9]*\)'`
test "x$CONFIG_RACK_REVISION_LEVEL" = "x" && CONFIG_RACK_REVISION_LEVEL=0

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

AC_DEFINE_UNQUOTED(CONFIG_RACK_BUILD_STRING, "$build", [Build system alias])
RACK_BUILD_STRING="$build"
AC_DEFINE_UNQUOTED(CONFIG_RACK_HOST_STRING, "$host", [Host system alias])
RACK_HOST_STRING="$host"
AC_DEFINE_UNQUOTED(CONFIG_RACK_COMPILER, "`$CC -v 2>&1 | tail -n 1`" , [Compiler])

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
dnl  checking target arch
dnl ======================================================================

AC_MSG_CHECKING([for target architecture])
case "$host" in
  i*86*-*)
    RACK_TARGET_ARCH=i386
    wanted_kernel_arch=CONFIG_X86
    ;;
  arm-*)
    RACK_TARGET_ARCH=arm
    wanted_kernel_arch=CONFIG_ARM
    ;;
  powerpc-*|ppc-*)
    RACK_TARGET_ARCH=ppc
    wanted_kernel_arch=CONFIG_PPC
    ;;
  *)
    AC_MSG_ERROR([ *** Sorry - Unsupported architecture])
    ;;
esac
AC_MSG_RESULT([${RACK_TARGET_ARCH}])
AC_SUBST(RACK_TARGET_ARCH)

dnl ======================================================================
dnl  Loading the RACK configuration.
dnl ======================================================================

RACK_KCONFIG_FILE=$srcdir/defconfig
kconfig_type=default

AC_ARG_WITH(kconfig-file,
    AS_HELP_STRING([--with-kconfig-file=<kconfig-file>], [KConfig parameter file]),
    [
        case "$withval" in
        "" | y | ye | yes | n | no)
          AC_MSG_ERROR([You must supply an argument to --with-kconfig-file.])
            ;;
        esac
        RACK_KCONFIG_FILE="$withval"
        kconfig_type=supplied
    ])

AC_MSG_CHECKING([for RACK Kconfig file])
if test \! -f $RACK_KCONFIG_FILE; then
    AC_MSG_ERROR([Kconfig-file $RACK_KCONFIG_FILE not found -- --with-kconfig-file sets Kconfig file])
fi

. $RACK_KCONFIG_FILE
AC_MSG_RESULT([$RACK_KCONFIG_FILE ($kconfig_type)])

dnl ======================================================================
dnl  LINUX kernel with XENOMAI
dnl ======================================================================

AC_ARG_WITH(linux-dir,
    AS_HELP_STRING([--with-linux-dir=<Linux-Sources>], [Linux source directory]),
    [case "${withval}" in
        y | ye | yes)
            AC_MSG_ERROR([You must supply an argument to --with-linux-dir.])
      ;;
    esac
    CONFIG_RACK_LINUXDIR="$withval"
    ])

if test "${CONFIG_RACK_LINUXDIR}" \!= "" ; then
    dnl -----------------------------------------------------------------
    dnl check config file
    dnl -----------------------------------------------------------------

    AC_MSG_CHECKING([for Linux config file])
    if test \! -f ${CONFIG_RACK_LINUXDIR}/.config ; then
    AC_MSG_ERROR([*** Please configure your kernel])
    fi
    AC_MSG_RESULT([${CONFIG_RACK_LINUXDIR}/.config])

    dnl -----------------------------------------------------------------
    dnl get linux version
    dnl -----------------------------------------------------------------

    AC_MSG_CHECKING([for Linux version])
    LINUX_VERSION="` cat ${CONFIG_RACK_LINUXDIR}/.config | \
                    grep "# Linux kernel version:" | \
                    ${AWK} -F ": " '{print $2}'`"
    AC_MSG_RESULT([${LINUX_VERSION}])

    dnl -----------------------------------------------------------------
    dnl check version
    dnl -----------------------------------------------------------------

    case "${LINUX_VERSION}" in
        2.6.*)
            ;;
        *)
            AC_MSG_ERROR([*** Unsupported kernel version $LINUX_VERSION])
            ;;
    esac

    dnl -----------------------------------------------------------------
    dnl loading linux config
    dnl -----------------------------------------------------------------

    . ${CONFIG_RACK_LINUXDIR}/.config

    dnl -----------------------------------------------------------------
    dnl  checking needed Xenomai and skins in linux config
    dnl -----------------------------------------------------------------

    AC_MSG_CHECKING([for Xenomai])
        case "${CONFIG_XENOMAI}" in
            y) AC_MSG_RESULT([${CONFIG_XENOMAI}]) ;;
            *) AC_MSG_ERROR([*** Please enable Xenomai]) ;;
        esac

    AC_MSG_CHECKING([for Xenomai native skin])
        case "${CONFIG_XENO_SKIN_NATIVE}" in
            y | m) AC_MSG_RESULT([${CONFIG_XENO_SKIN_NATIVE}]) ;;
            *) AC_MSG_ERROR([*** Please enable native skin]) ;;
        esac

    AC_MSG_CHECKING([for Xenomai RTDM skin])
        case "${CONFIG_XENO_SKIN_RTDM}" in
            y | m) AC_MSG_RESULT([${CONFIG_XENO_SKIN_RTDM}]) ;;
            *) AC_MSG_ERROR([*** Please enable RTDM skin]) ;;
        esac
fi

AM_CONDITIONAL(CONFIG_RACK_BUILD_KMOD,[test "${CONFIG_RACK_LINUXDIR}" \!= ""])

dnl ======================================================================
dnl  XENOMAI userspace
dnl ======================================================================

AC_ARG_WITH(xeno-user-dir,
    AS_HELP_STRING([--with-xeno-user-dir=<Xenomai>], [Xenomai userspace installation path]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-xeno-user-dir.])
      ;;
    esac
    CONFIG_RACK_XENOMAIDIR="$withval"
    ])

dnl -----------------------------------------------------------------
dnl  try to find Xenomai config
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([for Xenomai userspace installation])
XENO_USER_CONFIG="${CONFIG_RACK_XENOMAIDIR}/bin/xeno-config"
if test -x "${XENO_USER_CONFIG}"; then
  XENO_USER_DIR="`cd ${CONFIG_RACK_XENOMAIDIR} && pwd`"
  AC_MSG_RESULT([${XENO_USER_DIR}])
else
  AC_MSG_ERROR([no Xenomai found in ${CONFIG_RACK_XENOMAIDIR}])
fi

dnl -----------------------------------------------------------------
dnl  get Xenomai user prefix
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([for Xenomai userspace PREFIX])
XENO_USER_PREFIX="`cd \`${XENO_USER_CONFIG} --prefix\` && pwd`"
AC_MSG_RESULT([${XENO_USER_PREFIX}])

dnl -----------------------------------------------------------------
dnl  checking Xenomai version
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([for Xenomai version])
XENO_VERSION_PRE="`${XENO_USER_CONFIG} --version`"

case "$XENO_VERSION_PRE" in
  2.1* | 2.0.9* | 2.2*)
    XENO_VERSION=xeno-21x
    ;;
  *)
    AC_MSG_ERROR([*** Unsupported Xenomai version $XENO_VERSION_PRE in $XENO_USER_DIR])
  ;;
esac
AC_MSG_RESULT([${XENO_VERSION_PRE}])

dnl -----------------------------------------------------------------
dnl  get some more information about Xenomai
dnl -----------------------------------------------------------------

XENO_LIBRARIES="`${XENO_USER_CONFIG} --library-dir`"

dnl -----------------------------------------------------------------
dnl export xenomai values
dnl -----------------------------------------------------------------

AC_SUBST(XENO_USER_PREFIX)
AC_SUBST(XENO_LIBRARIES)
AC_DEFINE(CONFIG_XENO_21X, 1, [We build for Xenomai 2.1 or newer])

dnl ======================================================================
dnl  check support
dnl ======================================================================

dnl ----- RTnet ---------------------------------------------------------------

AC_ARG_ENABLE(rtnet-support,
    AS_HELP_STRING([--enable-rtnet-support], [enable rtnet support]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_RTNET_SUPPORT=y ;;
        *) CONFIG_RACK_RTET_SUPPORT=n ;;
    esac])

AC_ARG_WITH(rtnet-dir,
    AS_HELP_STRING([--with-rtnet-dir=<rtnet-dir>], [Install directory of RTnet]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-rtnet-dir.])
      ;;
    esac
    CONFIG_RACK_RTNETDIR="$withval"
    ])

dnl ----------------------------------------------------------------------------

AC_ARG_ENABLE(rtcan-support,
    AS_HELP_STRING([--enable-rtcan-support], [enable rtcan support]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_RTCAN_SUPPORT=y ;;
        *) CONFIG_RACK_RTCAN_SUPPORT=n ;;
    esac])

AC_ARG_WITH(rtcan-dir,
    AS_HELP_STRING([--with-rtcan-dir=<can-dir>], [Source path of rtcan]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-rtcan-dir.])
      ;;
    esac
    CONFIG_RACK_RTCANDIR="$withval"
    ])

dnl ----------------------------------------------------------------------------

AC_ARG_ENABLE(libraw1394-support,
    AS_HELP_STRING([--enable-libraw1394-support], [enable libraw1394 support]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_LIBRAW1394_SUPPORT=y ;;
        *) CONFIG_RACK_LIBRAW1394_SUPPORT=n ;;
    esac])

AC_ARG_WITH(libraw1394-dir,
    AS_HELP_STRING([--with-libraw1394=<libraw1394-dir>], [Install directory of libraw1394]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-libraw1394.])
      ;;
    esac
    CONFIG_RACK_LIBRAW1394DIR="$withval"
    ])

dnl ----------------------------------------------------------------------------

AC_ARG_ENABLE(libdc1394-support,
    AS_HELP_STRING([--enable-libdc1394-support], [enable libdc1394 support]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_LIBDC1394_SUPPORT=y ;;
        *) CONFIG_RACK_LIBDC1394_SUPPORT=n ;;
    esac])

AC_ARG_WITH(libdc1394-dir,
    AS_HELP_STRING([--with-libdc1394=<libdc1394-dir>], [Install directory of libdc1394]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-libdc1394.])
      ;;
    esac
    CONFIG_RACK_LIBDC1394DIR="$withval"
    ])

dnl ----------------------------------------------------------------------------

AC_ARG_ENABLE(libjpeg-support,
    AS_HELP_STRING([--enable-libjpeg-support], [enable libjpeg support]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_LIBJPEG_SUPPORT=y ;;
        *) CONFIG_RACK_LIBJPEG_SUPPORT=n ;;
    esac])

AC_ARG_WITH(libjpeg-dir,
    AS_HELP_STRING([--with-libjpeg=<libjpeg-dir>], [Install directory of libjpeg]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-libjpeg.])
      ;;
    esac
    CONFIG_RACK_LIBJPEGDIR="$withval"
    ])

dnl ----------------------------------------------------------------------------

AC_ARG_ENABLE(libjng-support,
    AS_HELP_STRING([--enable-libpng-support], [enable libpng support]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_LIBPNG_SUPPORT=y ;;
        *) CONFIG_RACK_LIBPNG_SUPPORT=n ;;
    esac])

AC_ARG_WITH(libpng-dir,
    AS_HELP_STRING([--with-libpng=<libpng-dir>], [Install directory of libpng]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-libpng.])
      ;;
    esac
    CONFIG_RACK_LIBPNGDIR="$withval"
    ])

dnl ----------------------------------------------------------------------------

AC_ARG_ENABLE(opencv-support,
    AS_HELP_STRING([--enable-opencv-support], [enable opencv support]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_OPENCV_SUPPORT=y ;;
        *) CONFIG_RACK_OPENCV_SUPPORT=n ;;
    esac])

AC_ARG_WITH(opencv-dir,
    AS_HELP_STRING([--with-opencv=<opencv-dir>], [Install directory of opencv]),
    [case "${withval}" in
        "" | y | ye | yes | n | no)
            AC_MSG_ERROR([You must supply an argument to --with-opencv.])
      ;;
    esac
    CONFIG_RACK_OPENCVDIR="$withval"
    ])

dnl ======================================================================
dnl    JAVA Part
dnl ======================================================================

RACK_JAVA_JARDIR="/gui"

AC_SUBST(RACK_JAVA_JARDIR)

dnl -----------------------------------------------------------------
dnl  rack java
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build java])
AC_ARG_ENABLE(java,
    AS_HELP_STRING([--enable-java], [building java]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_JAVA=y ;;
        *) CONFIG_RACK_JAVA=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_JAVA:-n}])
AM_CONDITIONAL(CONFIG_RACK_JAVA,[test "$CONFIG_RACK_JAVA" = "y"])
if test x"$CONFIG_RACK_JAVA" = "xy"; then
    AC_DEFINE(CONFIG_RACK_JAVA,1,[building java])
fi

AC_SUBST(CONFIG_RACK_JAVA)

dnl -----------------------------------------------------------------
dnl  rack java gui
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build java gui])
AC_ARG_ENABLE(java-gui,
    AS_HELP_STRING([--enable-java-gui], [building java gui]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_JAVA_GUI=y ;;
        *) CONFIG_RACK_JAVA_GUI=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_JAVA_GUI:-n}])
AM_CONDITIONAL(CONFIG_RACK_JAVA_GUI,[test "$CONFIG_RACK_JAVA_GUI" = "y"])
if test x"$CONFIG_RACK_JAVA_GUI" = "xy"; then
    AC_DEFINE(CONFIG_RACK_JAVA_GUI,1,[building java gui])
fi

AC_SUBST(CONFIG_RACK_JAVA_GUI)

dnl -----------------------------------------------------------------
dnl  javac & jar
dnl -----------------------------------------------------------------

if test x"$CONFIG_RACK_JAVA" = "xy"; then

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
dnl  ...
dnl -----------------------------------------------------------------

dnl ======================================================================
dnl    project modules
dnl ======================================================================

dnl -----------------------------------------------------------------
dnl  drivers - camera dcam
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build camera dcam])
AC_ARG_ENABLE(camera-dcam,
    AS_HELP_STRING([--enable-camera-dcam], [building camera dcam]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_CAMERA_DCAM=y ;;
        *) CONFIG_RACK_CAMERA_DCAM=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_CAMERA_DCAM:-n}])
AM_CONDITIONAL(CONFIG_RACK_CAMERA_DCAM,[test "$CONFIG_RACK_CAMERA_DCAM" = "y"])
if test "$CONFIG_RACK_CAMERA_DCAM" = "y"; then
    AC_DEFINE(CONFIG_RACK_CAMERA_DCAM,1,[building camera dcam])
    CONFIG_RACK_LIBRAW1394_SUPPORT=y
    CONFIG_RACK_LIBDC1394_SUPPORT=y
fi

dnl -----------------------------------------------------------------
dnl  drivers - camera file
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build camera file])
AC_ARG_ENABLE(camera-file,
    AS_HELP_STRING([--enable-camera-file], [building camera file]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_CAMERA_FILE=y ;;
        *) CONFIG_RACK_CAMERA_FILE=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_CAMERA_FILE:-n}])
AM_CONDITIONAL(CONFIG_RACK_CAMERA_FILE,[test "$CONFIG_RACK_CAMERA_FILE" = "y"])
if test "$CONFIG_RACK_CAMERA_FILE" = "y"; then
    AC_DEFINE(CONFIG_RACK_CAMERA_FILE,1,[building camera file])
fi

dnl -----------------------------------------------------------------
dnl  drivers - camera jpeg
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build camera jpeg])
AC_ARG_ENABLE(camera-jpeg,
    AS_HELP_STRING([--enable-camera-jpeg], [building camera jpeg]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_CAMERA_JPEG=y ;;
        *) CONFIG_RACK_CAMERA_JPEG=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_CAMERA_JPEG:-n}])
AM_CONDITIONAL(CONFIG_RACK_CAMERA_JPEG,[test "$CONFIG_RACK_CAMERA_JPEG" = "y"])
if test "$CONFIG_RACK_CAMERA_JPEG" = "y"; then
    AC_DEFINE(CONFIG_RACK_CAMERA_JPEG,1,[building camera jpeg])
    CONFIG_RACK_LIBJPEG_SUPPORT=y
fi

dnl -----------------------------------------------------------------
dnl  drivers - camera v4l
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build camera v4l])
AC_ARG_ENABLE(camera-v4l,
    AS_HELP_STRING([--enable-camera-v4l], [building camera v4l]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_CAMERA_V4L=y ;;
        *) CONFIG_RACK_CAMERA_V4L=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_CAMERA_V4L:-n}])
AM_CONDITIONAL(CONFIG_RACK_CAMERA_V4L,[test "$CONFIG_RACK_CAMERA_V4L" = "y"])
if test "$CONFIG_RACK_CAMERA_V4L" = "y"; then
    AC_DEFINE(CONFIG_RACK_CAMERA_V4L,1,[building camera v4l])
fi

dnl -----------------------------------------------------------------
dnl  drivers - chassis pioneer
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build chassis pioneer])
AC_ARG_ENABLE(chassis-pioneer,
    AS_HELP_STRING([--enable-chassis-pioneer], [building chassis pioneer]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_CHASSIS_PIONEER=y ;;
        *) CONFIG_RACK_CHASSIS_PIONEER=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_CHASSIS_PIONEER:-n}])
AM_CONDITIONAL(CONFIG_RACK_CHASSIS_PIONEER,[test "$CONFIG_RACK_CHASSIS_PIONEER" = "y"])
if test "$CONFIG_RACK_CHASSIS_PIONEER" = "y"; then
    AC_DEFINE(CONFIG_RACK_CHASSIS_PIONEER,1,[building chassis pioneer])
fi

dnl -----------------------------------------------------------------
dnl  drivers - chassis sim
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build chassis sim])
AC_ARG_ENABLE(chassis-sim,
    AS_HELP_STRING([--enable-chassis-sim], [building chassis sim]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_CHASSIS_SIM=y ;;
        *) CONFIG_RACK_CHASSIS_SIM=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_CHASSIS_SIM:-n}])
AM_CONDITIONAL(CONFIG_RACK_CHASSIS_SIM,[test "$CONFIG_RACK_CHASSIS_SIM" = "y"])
if test "$CONFIG_RACK_CHASSIS_SIM" = "y"; then
    AC_DEFINE(CONFIG_RACK_CHASSIS_SIM,1,[building chassis sim])
fi

dnl -----------------------------------------------------------------
dnl  drivers - gps nmea
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build gps nmea])
AC_ARG_ENABLE(gps-nmea,
    AS_HELP_STRING([--enable-gps-nmea], [building gps nmea]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_GPS_NMEA=y ;;
        *) CONFIG_RACK_GPS_NMEA=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_GPS_NMEA:-n}])
AM_CONDITIONAL(CONFIG_RACK_GPS_NMEA,[test "$CONFIG_RACK_GPS_NMEA" = "y"])
if test "$CONFIG_RACK_GPS_NMEA" = "y"; then
    AC_DEFINE(CONFIG_RACK_GPS_NMEA,1,[building gps nmea])
fi

dnl -----------------------------------------------------------------
dnl  drivers - ladar hokuyo urg
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build ladar hokuyo urg])
AC_ARG_ENABLE(ladar-hokuyo-urg,
    AS_HELP_STRING([--enable-ladar-hokuyo-urg], [building ladar hokuyo urg]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_LADAR_HOKUYO_URG=y ;;
        *) CONFIG_RACK_LADAR_HOKUYO_URG=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_LADAR_HOKUYO_URG:-n}])
AM_CONDITIONAL(CONFIG_RACK_LADAR_HOKUYO_URG,[test "$CONFIG_RACK_LADAR_HOKUYO_URG" = "y"])
if test "$CONFIG_RACK_LADAR_HOKUYO_URG" = "y"; then
    AC_DEFINE(CONFIG_RACK_LADAR_HOKUYO_URG,1,[building ladar hokuyo urg])
fi

dnl -----------------------------------------------------------------
dnl  drivers - ladar ibeo
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build ladar ibeo])
AC_ARG_ENABLE(ladar-ibeo,
    AS_HELP_STRING([--enable-ladar-ibeo], [building ladar ibeo]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_LADAR_IBEO=y ;;
        *) CONFIG_RACK_LADAR_IBEO=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_LADAR_IBEO:-n}])
AM_CONDITIONAL(CONFIG_RACK_LADAR_IBEO,[test "$CONFIG_RACK_LADAR_IBEO" = "y"])
if test "$CONFIG_RACK_LADAR_IBEO" = "y"; then
    AC_DEFINE(CONFIG_RACK_LADAR_IBEO,1,[building ladar ibeo])
    CONFIG_RACK_RTCAN_SUPPORT=y
fi

dnl -----------------------------------------------------------------
dnl  drivers - ladar sick lms200
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build ladar sick lms200 driver])
AC_ARG_ENABLE(ladar-sick-lms200,
    AS_HELP_STRING([--enable-ladar-sick-lms200], [building ladar sick lms200 laser scanner driver]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_LADAR_SICK_LMS200=y ;;
        *) CONFIG_RACK_LADAR_SICK_LMS200=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_LADAR_SICK_LMS200:-n}])
AM_CONDITIONAL(CONFIG_RACK_LADAR_SICK_LMS200,[test "$CONFIG_RACK_LADAR_SICK_LMS200" = "y"])
if test "$CONFIG_RACK_LADAR_SICK_LMS200" = "y"; then
    AC_DEFINE(CONFIG_RACK_LADAR_SICK_LMS200,1,[building ladar sick lms200 laser scanner driver])
fi

dnl -----------------------------------------------------------------
dnl  drivers - ladar sick cms3000
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build ladar sick cms3000 driver])
AC_ARG_ENABLE(ladar-sick-cms3000,
    AS_HELP_STRING([--enable-ladar-sick-cms3000], [building ladar sick cms3000 laser scanner 
driver]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_LADAR_SICK_CMS3000=y ;;
        *) CONFIG_RACK_LADAR_SICK_CMS3000=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_LADAR_SICK_CMS3000:-n}])
AM_CONDITIONAL(CONFIG_RACK_LADAR_SICK_CMS3000,[test "$CONFIG_RACK_LADAR_SICK_CMS3000" = "y"])
if test "$CONFIG_RACK_LADAR_SICK_CMS3000" = "y"; then
    AC_DEFINE(CONFIG_RACK_LADAR_SICK_CMS3000,1,[building ladar sick cms3000 laser scanner 
driver])
fi

dnl -----------------------------------------------------------------
dnl  drivers - Odometry
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build odometry chassis])
AC_ARG_ENABLE(odometry-chassis,
    AS_HELP_STRING([--enable-odometry-chassis], [building odometry chassis]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_ODOMETRY_CHASSIS=y ;;
        *) CONFIG_RACK_ODOMETRY_CHASSIS=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_ODOMETRY_CHASSIS:-n}])
AM_CONDITIONAL(CONFIG_RACK_ODOMETRY_CHASSIS,[test "$CONFIG_RACK_ODOMETRY_CHASSIS" = "y"])
if test "$CONFIG_RACK_ODOMETRY_CHASSIS" = "y"; then
    AC_DEFINE(CONFIG_RACK_ODOMETRY_CHASSIS,1,[building odometry chassis])
fi

dnl -----------------------------------------------------------------
dnl  navigation - Pilot Joystick
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build pilot joystick])
AC_ARG_ENABLE(pilot-joystick,
    AS_HELP_STRING([--enable-pilot-joystick], [building pilot joystick]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_PILOT_JOYSTICK=y ;;
        *) CONFIG_RACK_PILOT_JOYSTICK=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_PILOT_JOYSTICK:-n}])
AM_CONDITIONAL(CONFIG_RACK_PILOT_JOYSTICK,[test "$CONFIG_RACK_PILOT_JOYSTICK" = "y"])
if test "$CONFIG_RACK_PILOT_JOYSTICK" = "y"; then
    AC_DEFINE(CONFIG_RACK_PILOT_JOYSTICK,1,[building pilot joystick])
fi

dnl -----------------------------------------------------------------
dnl  navigation - Position
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build position])
AC_ARG_ENABLE(position,
    AS_HELP_STRING([--enable-position], [building position]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_POSITION=y ;;
        *) CONFIG_RACK_POSITION=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_POSITION:-n}])
AM_CONDITIONAL(CONFIG_RACK_POSITION,[test "$CONFIG_RACK_POSITION" = "y"])
if test "$CONFIG_RACK_POSITION" = "y"; then
    AC_DEFINE(CONFIG_RACK_POSITION,1,[building position])
fi

dnl -----------------------------------------------------------------
dnl  perception - scan 2d
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build Scan2D])
AC_ARG_ENABLE(scan-2d,
    AS_HELP_STRING([--enable-scan-2d], [building Scan2D]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_SCAN_2D=y ;;
        *) CONFIG_RACK_SCAN_2D=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_SCAN_2D:-n}])
AM_CONDITIONAL(CONFIG_RACK_SCAN_2D,[test "$CONFIG_RACK_SCAN_2D" = "y"])
if test "$CONFIG_RACK_SCAN_2D" = "y"; then
    AC_DEFINE(CONFIG_RACK_SCAN_2D,1,[building Scan2D])
fi

dnl -----------------------------------------------------------------
dnl  perception - scan 2d merge
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build Scan2D Merge])
AC_ARG_ENABLE(scan-2d-merge,
    AS_HELP_STRING([--enable-scan-2d-merge], [building Scan2D Merge]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_SCAN_2D_MERGE=y ;;
        *) CONFIG_RACK_SCAN_2D_MERGE=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_SCAN_2D_MERGE:-n}])
AM_CONDITIONAL(CONFIG_RACK_SCAN_2D_MERGE,[test "$CONFIG_RACK_SCAN_2D_MERGE" = "y"])
if test "$CONFIG_RACK_SCAN_2D_MERGE" = "y"; then
    AC_DEFINE(CONFIG_RACK_SCAN_2D_MERGE,1,[building Scan2D Merge])
fi

dnl -----------------------------------------------------------------
dnl  perception - scan 2d sim
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build Scan2D Sim])
AC_ARG_ENABLE(scan-2d-sim,
    AS_HELP_STRING([--enable-scan-2d-sim], [building Scan2D Sim]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_SCAN_2D_SIM=y ;;
        *) CONFIG_RACK_SCAN_2D_SIM=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_SCAN_2D_SIM:-n}])
AM_CONDITIONAL(CONFIG_RACK_SCAN_2D_SIM,[test "$CONFIG_RACK_SCAN_2D_SIM" = "y"])
if test "$CONFIG_RACK_SCAN_2D_SIM" = "y"; then
    AC_DEFINE(CONFIG_RACK_SCAN_2D_SIM,1,[building Scan2D Sim])
fi

dnl -----------------------------------------------------------------
dnl  skel - dummy abc
dnl -----------------------------------------------------------------

AC_MSG_CHECKING([build Dummy Abc])
AC_ARG_ENABLE(dummy-abc,
    AS_HELP_STRING([--enable-dummy-abc], [building Dummy Abc]),
    [case "$enableval" in
        y | yes) CONFIG_RACK_DUMMY_ABC=y ;;
        *) CONFIG_RACK_DUMMY_ABC=n ;;
    esac])
AC_MSG_RESULT([${CONFIG_RACK_DUMMY_ABC:-n}])
AM_CONDITIONAL(CONFIG_RACK_DUMMY_ABC,[test "$CONFIG_RACK_DUMMY_ABC" = "y"])
if test "$CONFIG_RACK_DUMMY_ABC" = "y"; then
    AC_DEFINE(CONFIG_RACK_DUMMY_ABC,1,[building Dummy Abc])
fi

dnl -----------------------------------------------------------------
dnl  ...
dnl -----------------------------------------------------------------

dnl ======================================================================
dnl  directory / library checks
dnl ======================================================================

RACK_KMOD_CFLAGS=""
RACK_SUPPORTS=""

dnl ----- Xenomai --------------------------------------------------------
dnl
dnl  depends on nothing
dnl

XENOMAI_CPPFLAGS="`${XENO_USER_CONFIG} --xeno-cflags`"
XENOMAI_LDFLAGS="`${XENO_USER_CONFIG} --xeno-ldflags`"
XENOMAI_LIBS="-lnative -lrtdm"

RACK_SUPPORTS="xenomai ${RACK_SUPPORTS}"

AC_SUBST(XENOMAI_CPPFLAGS)
AC_SUBST(XENOMAI_LDFLAGS)
AC_SUBST(XENOMAI_LIBS)

dnl *** little hack ***
CONFIG_RACK_XENOMAI_SUPPORT=y

AM_CONDITIONAL(CONFIG_RACK_XENOMAI_SUPPORT,
               [test x"${CONFIG_RACK_XENOMAI_SUPPORT}" = "xy"])
               
dnl ----- rtnet ----------------------------------------------------------------
dnl
dnl  depends on xenomai
dnl  provides headers: yes
dnl  provides libs:    no
dnl

AC_MSG_CHECKING([rtnet support is needed])
AC_MSG_RESULT([${CONFIG_RACK_RTNET_SUPPORT}])

RTNET_CPPFLAGS=""
    
if test x"${CONFIG_RACK_RTNET_SUPPORT}" = x"y"; then

    cppflags_backup="${CPPFLAGS}"
    if test x${CONFIG_RACK_RTNETDIR} != "x"; then
        AC_MSG_CHECKING([rtnet directory])
        if test -d ${CONFIG_RACK_RTNETDIR}; then
            AC_MSG_RESULT([${CONFIG_RACK_RTNETDIR}])

            RTNET_CPPFLAGS="-I${CONFIG_RACK_RTNETDIR}/include"

            CPPFLAGS="${XENOMAI_CPPFLAGS} ${CPPFLAGS}"
            CPPFLAGS="${RTNET_CPPFLAGS} ${CPPFLAGS}"
        else
            AC_MSG_ERROR(cannot find ${CONFIG_RACK_RTNETDIR})
        fi
    fi

    AC_CHECK_HEADER(rtnet.h,,
                    [AC_MSG_ERROR(cannot find rtnet.h)]
                   )

    RACK_SUPPORTS="rtnet ${RACK_SUPPORTS}"
    CPPFLAGS="${cppflags_backup}"
fi

AC_SUBST(RTNET_CPPFLAGS)
AM_CONDITIONAL(CONFIG_RACK_RTNET_SUPPORT,
               [test x"${CONFIG_RACK_RTNET_SUPPORT}" = "xy"])

dnl ----- rt_sja1000 -----------------------------------------------------------
dnl
dnl  depends on xenomai
dnl  provides headers: yes
dnl  provides libs:    no
dnl

AC_MSG_CHECKING([rtcan support is needed])
AC_MSG_RESULT([${CONFIG_RACK_RTCAN_SUPPORT}])

RTCAN_CPPFLAGS=""
    
if test x"${CONFIG_RACK_RTCAN_SUPPORT}" = x"y"; then

    cppflags_backup="${CPPFLAGS}"

    if test x${CONFIG_RACK_RTCANDIR} != "x"; then
        AC_MSG_CHECKING([rtcan directory])
        if test -d ${CONFIG_RACK_RTCANDIR}; then
            AC_MSG_RESULT([${CONFIG_RACK_RTCANDIR}])

            RTCAN_CPPFLAGS="-I${CONFIG_RACK_RTCANDIR}/include"

            CPPFLAGS="${XENOMAI_CPPFLAGS} ${CPPFLAGS}"
            CPPFLAGS="${RTCAN_CPPFLAGS} ${CPPFLAGS}"
        else
            AC_MSG_ERROR(cannot find ${CONFIG_RACK_RTCANDIR})
        fi
    fi

    AC_CHECK_HEADER(rtcan.h,,
                    [AC_MSG_ERROR(cannot find rtcan.h)]
                   )

    RACK_SUPPORTS="rtcan ${RACK_SUPPORTS}"
    CPPFLAGS="${cppflags_backup}"
fi

AC_SUBST(RTCAN_CPPFLAGS)
AM_CONDITIONAL(CONFIG_RACK_RTCAN_SUPPORT,
               [test x"${CONFIG_RACK_RTCAN_SUPPORT}" = "xy"])

dnl ----- libraw1394 -----------------------------------------------------------
dnl
dnl  depends on nothing
dnl  provides headers: yes
dnl  provides libs:    yes
dnl

AC_MSG_CHECKING([libraw1394 support])
AC_MSG_RESULT([${CONFIG_RACK_LIBRAW1394_SUPPORT}])

LIBRAW1394_CPPFLAGS=""
LIBRAW1394_LDFLAGS=""
LIBRAW1394_LIBS=""

if test x"${CONFIG_RACK_LIBRAW1394_SUPPORT}" = x"y"; then

    cppflags_backup="${CPPFLAGS}"
    ldflags_backup="${LDFLAGS}"
    libs_backup="${LIBS}"
    
    if test x${CONFIG_RACK_LIBRAW1394DIR} != "x"; then
        AC_MSG_CHECKING([libraw1394 directory])
        if test -d ${CONFIG_RACK_LIBRAW1394DIR}; then
            AC_MSG_RESULT([${CONFIG_RACK_LIBRAW1394DIR}])

            LIBRAW1394_CPPFLAGS="-I${CONFIG_RACK_LIBRAW1394DIR}/include"
            LIBRAW1394_LDFLAGS="-L${CONFIG_RACK_LIBRAW1394DIR}/lib"
            LIBRAW1394_LIBS="-lraw1394"

            CPPFLAGS="${LIBRAW1394_CPPFLAGS} ${CPPFLAGS}"
            LDFLAGS="${LIBRAW1394_LDFLAGS} ${LDFLAGS}"
            LIBS="$LIBRAW394_LIBS"

        else
            AC_MSG_ERROR(cannot find ${CONFIG_RACK_LIBRAW1394DIR})
        fi
    fi

    AC_CHECK_HEADER([libraw1394/raw1394.h],
                    AC_CHECK_LIB(raw1394,
                                 raw1394_new_handle,,
                                 [AC_MSG_ERROR(cannot find function raw1394_new_handle)]
                                ),
                    [AC_MSG_ERROR(cannot find libraw1394 header files)]
                   )

    RACK_SUPPORTS="libraw1394 ${RACK_SUPPORTS}"
    CPPFLAGS="${cppflags_backup}"
    LDFLAGS="${ldflags_backup}"
    LIBS="${libs_backup}"
fi

AC_SUBST(LIBRAW1394_CPPFLAGS)
AC_SUBST(LIBRAW1394_LDFLAGS)
AC_SUBST(LIBRAW1394_LIBS)
AM_CONDITIONAL(CONFIG_RACK_LIBRAW1394_SUPPORT,
               [test x"${CONFIG_RACK_LIBRAW1394_SUPPORT}" = "xy"])

dnl ----- libdc1394_control ----------------------------------------------------
dnl
dnl  depends on libraw1394
dnl  provides headers: yes
dnl  provides libs:    yes
dnl

AC_MSG_CHECKING([libdc1394 support])
AC_MSG_RESULT([${CONFIG_RACK_LIBDC1394_SUPPORT}])

LIBDC1394_CPPFLAGS=""
LIBDC1394_LDFLAGS=""
LIBDC1394_LIBS=""

if test x"${CONFIG_RACK_LIBDC1394_SUPPORT}" = x"y"; then

    cppflags_backup="${CPPFLAGS}"
    ldflags_backup="${LDFLAGS}"
    libs_backup="${LIBS}"

    if test x${CONFIG_RACK_LIBDC1394DIR} != "x"; then
        AC_MSG_CHECKING([libdc1394 directory])
        if test -d ${CONFIG_RACK_LIBDC1394DIR}; then
            AC_MSG_RESULT([${CONFIG_RACK_LIBDC1394DIR}])

            LIBDC1394_CPPFLAGS="-I${CONFIG_RACK_LIBDC1394DIR}/include"
            LIBDC1394_LDFLAGS="-L${CONFIG_RACK_LIBDC1394DIR}/lib"
            LIBDC1394_LIBS="-ldc1394_control"

            CPPFLAGS="${LIBRAW1394_CPPFLAGS} ${CPPFLAGS}"
            LDFLAGS="${LIBRAW1394_LDFLAGS} ${LDFLAGS}"
            LIBS="$LIBRAW394_LIBS"

            CPPFLAGS="${LIBDC1394_CPPFLAGS} ${CPPFLAGS}"
            LDFLAGS="${LIBDC1394_LDFLAGS} ${LDFLAGS}"
            LIBS="$LIBDC1394_LIBS"
        else
            AC_MSG_ERROR(cannot find ${CONFIG_RACK_LIBDC1394DIR})
        fi
    fi

    AC_CHECK_HEADER([libdc1394/dc1394_control.h],
                    AC_CHECK_LIB(dc1394_control,
                                 dc1394_get_camera_nodes,,
                                 [AC_MSG_ERROR(cannot find function dc1394_get_camera_nodes)]
                                ),
                    [AC_MSG_ERROR(cannot find libdc1394 header files)]
                   )

    RACK_SUPPORTS="libdc1394 ${RACK_SUPPORTS}"
    CPPFLAGS="${cppflags_backup}"
    LDFLAGS="${ldflags_backup}"
    LIBS="${libs_backup}"
fi

AC_SUBST(LIBDC1394_CPPFLAGS)
AC_SUBST(LIBDC1394_LDFLAGS)
AC_SUBST(LIBDC1394_LIBS)
AM_CONDITIONAL(CONFIG_RACK_LIBDC1394_SUPPORT,
               [test x"${CONFIG_RACK_LIBDC1394_SUPPORT}" = "xy"])

dnl ----- libjpeg --------------------------------------------------------------
dnl
dnl  depends on nothing
dnl  provides headers: yes
dnl  provides libs:    yes
dnl

AC_MSG_CHECKING([libjpeg support])
AC_MSG_RESULT([${CONFIG_RACK_LIBJPEG_SUPPORT}])

LIBJPEG_CPPFLAGS=""
LIBJPEG_LDFLAGS=""
LIBJPEG_LIBS=""

if test x"${CONFIG_RACK_LIBJPEG_SUPPORT}" = x"y"; then

    cppflags_backup="${CPPFLAGS}"
    ldflags_backup="${LDFLAGS}"
    libs_backup="${LIBS}"

    if test x${CONFIG_RACK_LIBJPEGDIR} != "x"; then
        AC_MSG_CHECKING([libjpeg directory])
        if test -d ${CONFIG_RACK_LIBJPEGDIR}; then
            AC_MSG_RESULT([${CONFIG_RACK_LIBJPEGDIR}])

            LIBJPEG_CPPFLAGS="-I${CONFIG_RACK_LIBJPEGDIR}/include"
            LIBJPEG_LDFLAGS="-L${CONFIG_RACK_LIBJPEGDIR}/lib"
            LIBJPEG_LIBS="-ljpeg"

            CPPFLAGS="${LIBJPEG_CPPFLAGS} ${CPPFLAGS}"
            LDFLAGS="${LIBJPEG_LDFLAGS} ${LDFLAGS}"
            LIBS="$LIBJPEG_LIBS"
        else
            AC_MSG_ERROR(cannot find ${CONFIG_RACK_LIBJPEGDIR})
        fi
    fi

    AC_CHECK_HEADER(jpeglib.h,
                    AC_CHECK_LIB(jpeg,
                                 jpeg_start_compress,,
                                 [AC_MSG_ERROR(cannot find function jpeg_create_compress)]
                                ),
                    [AC_MSG_ERROR(cannot find libjpeg header files)]
                   )

    RACK_SUPPORTS="libjpeg ${RACK_SUPPORTS}"
    CPPFLAGS="${cppflags_backup}"
    LDFLAGS="${ldflags_backup}"
    LIBS="${libs_backup}"
fi

AC_SUBST(LIBJPEG_CPPFLAGS)
AC_SUBST(LIBJPEG_LDFLAGS)
AC_SUBST(LIBJPEG_LIBS)
AM_CONDITIONAL(CONFIG_RACK_LIBJPEG_SUPPORT,
               [test x"${CONFIG_RACK_LIBJPEG_SUPPORT}" = "xy"])

dnl ----- libpng ---------------------------------------------------------------
dnl
dnl  depends on nothing
dnl  provides headers: yes
dnl  provides libs:    yes
dnl

AC_MSG_CHECKING([libpng support])
AC_MSG_RESULT([${CONFIG_RACK_LIBPNG_SUPPORT}])

if test x"${CONFIG_RACK_LIBPNG_SUPPORT}" = x"y"; then

    cppflags_backup="${CPPFLAGS}"
    ldflags_backup="${LDFLAGS}"
    libs_backup="${LIBS}"

    LIBPNG_CPPFLAGS=""
    LIBPNG_LDFLAGS=""
    LIBPNG_LIBS=""

    if test x${CONFIG_RACK_LIBPNGDIR} != "x"; then
        AC_MSG_CHECKING([libpng directory])
        if test -d ${CONFIG_RACK_LIBPNGDIR}; then
            AC_MSG_RESULT([${CONFIG_RACK_LIBPNGDIR}])

            LIBPNG_CPPFLAGS="-I${CONFIG_RACK_LIBPNGDIR}/include"
            LIBPNG_LDFLAGS="-L${CONFIG_RACK_LIBPNGDIR}/lib"
            LIBPNG_LIBS="-lpng12"

            CPPFLAGS="${LIBPNG_CPPFLAGS} ${CPPFLAGS}"
            LDFLAGS="${LIBPNG_LDFLAGS} ${LDFLAGS}"
            LIBS="$LIBPNG_LIBS"
        else
            AC_MSG_ERROR(cannot find ${CONFIG_RACK_LIBPNGDIR})
        fi
    fi

    AC_CHECK_HEADER(libpng/png.h,
                    AC_CHECK_LIB(png12,
                                 png_write_end,,
                                 [AC_MSG_ERROR(cannot find function png_write_end)]
                                ),
                    [AC_MSG_ERROR(cannot find libpng header files)]
                   )

    RACK_SUPPORTS="libpng ${RACK_SUPPORTS}"

    AC_SUBST(LIBPNG_CPPFLAGS)
    AC_SUBST(LIBPNG_LDFLAGS)
    AC_SUBST(LIBPNG_LIBS)

    CPPFLAGS="${cppflags_backup}"
    LDFLAGS="${ldflags_backup}"
    LIBS="${libs_backup}"
fi

AM_CONDITIONAL(CONFIG_LIBPNG,
               [test x"${LIBPNG_LIBS}" != x""])

AM_CONDITIONAL(CONFIG_RACK_LIBPNG_SUPPORT,
               [test x"${CONFIG_RACK_LIBPNG_SUPPORT}" = "xy"])

dnl ----- opencv ---------------------------------------------------------------
dnl
dnl  depends on nothing
dnl  provides headers: yes
dnl  provides libs:    yes
dnl

AC_MSG_CHECKING([opencv support])
AC_MSG_RESULT([${CONFIG_RACK_OPENCV_SUPPORT}])

if test x"${CONFIG_RACK_OPENCV_SUPPORT}" = x"y"; then

    cppflags_backup="${CPPFLAGS}"
    ldflags_backup="${LDFLAGS}"
    libs_backup="${LIBS}"

    OPENCV_CPPFLAGS=""
    OPENCV_LDFLAGS=""
    OPENCV_LIBS=""

    if test x${CONFIG_RACK_OPENCVDIR} != "x"; then
        AC_MSG_CHECKING([opencv directory])
        if test -d ${CONFIG_RACK_OPENCVDIR}; then
            AC_MSG_RESULT([${CONFIG_RACK_OPENCVDIR}])

            OPENCV_CPPFLAGS="-I${CONFIG_RACK_OPENCVDIR}/include/opencv"
            OPENCV_LDFLAGS="-L${CONFIG_RACK_OPENCVDIR}/lib"
            OPENCV_LIBS="-lcv -lcvaux -lcxcore -lhighgui"

            CPPFLAGS="${OPENCV_CPPFLAGS} ${CPPFLAGS}"
            LDFLAGS="${OPENCV_LDFLAGS} ${LDFLAGS}"
            LIBS="$OPENCV_LIBS"
        else
            AC_MSG_ERROR(cannot find ${CONFIG_RACK_OPENCVDIR})
        fi
    fi

    AC_CHECK_HEADERS(cv.h cvaux.h cxcore.h highgui.h,
                    AC_CHECK_LIB(cv,
                                 cvCreateData,,
                                 [AC_MSG_ERROR(cannot find function cvCreateData)]
                                )
                    AC_CHECK_LIB(cvaux,
                                 cvFindFace,,
                                 [AC_MSG_ERROR(cannot find function cvFindFace)]
                                )
                    AC_CHECK_LIB(cxcore,
                                 cvCreateSet,,
                                 [AC_MSG_ERROR(cannot find function cvCreateSet)]
                                )
                    AC_CHECK_LIB(highgui,
                                 cvShowImage,,
                                 [AC_MSG_ERROR(cannot find function cvShowImage)]
                                ),
                    [AC_MSG_ERROR(cannot find opencv header files)]
                   )

    RACK_SUPPORTS="opencv ${RACK_SUPPORTS}"

    AC_SUBST(OPENCV_CPPFLAGS)
    AC_SUBST(OPENCV_LDFLAGS)
    AC_SUBST(OPENCV_LIBS)

    CPPFLAGS="${cppflags_backup}"
    LDFLAGS="${ldflags_backup}"
    LIBS="${libs_backup}"
fi

AM_CONDITIONAL(CONFIG_OPENCV,
               [test x"${OPENCV_LIBS}" != x""])

AM_CONDITIONAL(CONFIG_RACK_OPENCV_SUPPORT,
               [test x"${CONFIG_RACK_OPENCV_SUPPORT}" = "xy"])

dnl ======================================================================
dnl  set flags / libs
dnl ======================================================================

dnl ---- cppflags --------------------------------------------------------------

RACK_GLOBAL_CPPFLAGS="-Wall"
AC_SUBST(RACK_GLOBAL_CPPFLAGS)

dnl ---- ldflags ---------------------------------------------------------------

RACK_GLOBAL_LDFLAGS="--export-dynamic"
AC_SUBST(RACK_GLOBAL_LDFLAGS)

dnl ======================================================================
dnl  current project
dnl ======================================================================

RACK_SUPPORTS="${PROJNAME} ${RACK_SUPPORTS}"
AC_SUBST(RACK_SUPPORTS)

RACK_CPPFLAGS="-I\${top_srcdir} -I\${top_builddir} ${XENOMAI_CPPFLAGS} ${RACK_GLOBAL_CPPFLAGS}"
RACK_LDFLAGS="-L\$(top_builddir)/main/lib/.libs ${XENOMAI_LDFLAGS} ${RACK_GLOBAL_LDFLAGS}"
RACK_LIBS="-l${PROJNAME} ${XENOMAI_LIBS}"

AC_SUBST(RACK_CPPFLAGS)
AC_SUBST(RACK_LDFLAGS)
AC_SUBST(RACK_LIBS)


dnl ======================================================================
dnl             doxygen documentation
dnl ======================================================================

if test \! -d $srcdir/doc; then
    unset RACK_MAYBE_DOCDIR
else
    RACK_MAYBE_DOCDIR=doc
fi
AC_SUBST(RACK_MAYBE_DOCDIR)

AC_MSG_CHECKING(for Doxygen documentation)
AC_ARG_ENABLE(dox-doc,
    AS_HELP_STRING([--enable-dox-doc], [Build Doxygen documentation]),
    [case "$enableval" in
    y | yes) CONFIG_RACK_DOC_DOX=y ;;
    *) unset CONFIG_RACK_DOC_DOX ;;
    esac])

if test \! -d $srcdir/doc; then
    if test x$CONFIG_RACK_DOC_DOX = xy ;
    then
        AC_MSG_ERROR([documentation tree is missing.])
    fi
    AC_MSG_RESULT([not present])
else
    AC_MSG_RESULT(${CONFIG_RACK_DOC_DOX:-no})
fi

AC_CHECK_PROG(DOXYGEN, doxygen, doxygen)

if test x${CONFIG_RACK_DOC_DOX} = xy -a x"$DOXYGEN" = x; then
   AC_MSG_ERROR([Missing the Doxygen tools to generate the documentation.])
fi

AC_CHECK_PROG(DOXYGEN_HAVE_DOT, dot, YES, NO)
if test x"$DOXYGEN_HAVE_DOT" = xYES; then
   DOXYGEN_SHOW_INCLUDE_FILES=NO
else
   DOXYGEN_SHOW_INCLUDE_FILES=YES
fi

AM_CONDITIONAL(CONFIG_RACK_DOC_DOX,[test "$CONFIG_RACK_DOC_DOX" = y])
AC_SUBST(DOXYGEN_SHOW_INCLUDE_FILES)
AC_SUBST(DOXYGEN_HAVE_DOT)
AC_SUBST(DOXYGEN)

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
dnl             build for 2.6 kernel
dnl ======================================================================

RACK_MODULE_EXT=.ko
RACK_KBUILD_ENV='src2obj = $(patsubst %.$(1),%.o,$(filter %.$(1),$(call notdir,$(2))))'
RACK_KBUILD_CMD="                                                           \
        @for src in \$(filter-out FORCE, \$^); do                           \
            if test \\! -r \`basename \$\$src\`; then                       \
                \$(LN_S) \$\$src;                                           \
            fi;                                                             \
        done;                                                               \
        \$(MAKE) -C ${CONFIG_RACK_LINUXDIR}                                 \
                CROSS_COMPILE=\$(CROSS_COMPILE)                             \
                ARCH=\$(RACK_TARGET_ARCH)                                   \
                M=\"\`pwd\`\"                                               \
                V=\$(V)                                                     \
                rack_kmod_cflags=\"\$(RACK_KMOD_CFLAGS)\"                   \
                top_srcdir=\"\`cd \$(top_srcdir) && pwd\`\"                 \
                top_builddir=\"\`cd \$(top_builddir) && pwd\`\"             \
                linux-dir=\"${CONFIG_RACK_LINUXDIR}\"                       \
                xeno_user_dir=\"${XENO_USER_PREFIX}\"                       \
                srcdir=\"\`cd \$(srcdir) && pwd\`\"                         \
                build_objs='\$(call src2obj,c,\$^)'                         \
                build_target='\$(basename \$@)'                             \
                modules                                                     \
                "

RACK_KBUILD_CLEAN="                                                         \
        @rm -f *.ko *.mod.c .*.cmd *.o;                                     \
        for src in \$^; do                                                  \
            if test -h \`basename \$\$src\`; then                           \
                rm -f \`basename \$\$src\`;                                 \
            fi;                                                             \
        done;                                                               \
        "
RACK_KBUILD_DISTCLEAN="                                                     \
        @for mod in \${OBJS}; do                                            \
            rm -f \`cd \$(top_builddir) && pwd\`/symbols/\`echo \$\$mod |   \
                sed -e's/${RACK_MODULE_EXT}//'\`.mod;                       \
        done;                                                               \
        "

AC_MSG_CHECKING([for kernel module extension])
AC_MSG_RESULT([$RACK_MODULE_EXT])

AC_SUBST(RACK_MODULE_EXT)
AC_SUBST(RACK_KBUILD_ENV)
AC_SUBST(RACK_KBUILD_CMD)
AC_SUBST(RACK_KBUILD_CLEAN)
AC_SUBST(RACK_KBUILD_DISTCLEAN)

bs_kmodext=$RACK_MODULE_EXT
bs_kcompile="make -C $CONFIG_RACK_LINUXDIR ARCH=$RACK_TARGET_ARCH CROSS_COMPILE=$CROSS_COMPILE V=1 M=`pwd` SUBDIRS=`pwd` modules"

dnl ======================================================================
dnl             Installation directory for modules
dnl ======================================================================

AC_MSG_CHECKING([for module installation directory])

AC_ARG_WITH(module-dir,
    AS_HELP_STRING([--with-module-dir=<module-dir>], [Installation directory for modules]),
    [case "${withval}" in
    "" | y | ye | yes | n | no)
        AC_MSG_ERROR([You must supply an argument to --with-module-dir.]) ;;
    esac],
    with_module_dir='${exec_prefix}/modules')
RACK_MODULE_DIR="$with_module_dir"
AC_MSG_RESULT([$RACK_MODULE_DIR])

AC_SUBST(RACK_MODULE_DIR)

dnl ======================================================================
dnl             hac^H^H^Hfix problem with multible defined symbols problem
dnl             shamelessly stolen from RTAI-3.1 tnx to Philippe
dnl ======================================================================

dnl CAUTION: We need to have the CONFIG_RACK_XX symbols always defined when
dnl the configuration header is read, but we want the Autoconf-produced
dnl symbols to be defined only when compiling RACK. This way, we won't
dnl pollute the namespace with the latter when our configuration header is
dnl indirectly included by a client application. To achieve this, we ask
dnl autoheader to produce the following header structure:
dnl #define CONFIG_XX
dnl #define CONFIG_XX ...
dnl #ifdef __IN_RACK__
dnl <Autoconf-defined symbols>
dnl #endif /* __IN_RACK__ */
dnl This is quite a hack since we have to rely on the fact that
dnl all Autoconf-generated symbols are lexicographically sorted
dnl after CONFIG_RACK_XX ones, but, well...

dnl Use a key which will cause the verbatim string to be put after
dnl all CONFIG_RACK_XX symbols, but still before any Autoconf-generated
dnl symbol, hence CONFIG_RACK___.
AH_VERBATIM(CONFIG_RACK___,[#ifdef __IN_RACK__])

dnl Now we can close the conditional section, right after all
dnl Autoconf-generated symbols have been listed.
AH_BOTTOM([#endif /* __IN_RACK__ */])

RACK_KMOD_CFLAGS="-D__IN_RACK__ $RACK_KMOD_CFLAGS"
RACK_USER_CFLAGS="-D__IN_RACK__ $RACK_USER_CFLAGS"

AC_SUBST(RACK_KMOD_CFLAGS)
AC_SUBST(RACK_USER_CFLAGS)

dnl ======================================================================
dnl             create links to Makefiles used by linux-2.6.x
dnl ======================================================================

for DIR in \
    main/tims/driver \
    ; do
      AC_CONFIG_LINKS(${DIR}/Makefile:${DIR}/Makefile.kbuild)
done

dnl ======================================================================
dnl             generate output
dnl ======================================================================

AC_CONFIG_FILES([GNUmakefile \
    \
    config/GNUmakefile \
    config/kconfig/GNUmakefile \
    config/kconfig/lxdialog/GNUmakefile \
    \
    main/GNUmakefile \
    main/argopts/GNUmakefile \
    main/defines/GNUmakefile \
    main/dxfmap/GNUmakefile \
    main/lib/GNUmakefile \
    main/mailbox/GNUmakefile \
    main/module/GNUmakefile \
    main/proxy/GNUmakefile \
    main/canport/GNUmakefile \
    main/serialport/GNUmakefile \
    main/tims/GNUmakefile \
    main/tims/driver/GNUmakefile \
    main/tims/router/GNUmakefile \
    \
    drivers/GNUmakefile \
    drivers/camera/GNUmakefile \
    drivers/chassis/GNUmakefile \
    drivers/gps/GNUmakefile \
    drivers/ladar/GNUmakefile \
    drivers/odometry/GNUmakefile \
    \
    navigation/GNUmakefile \
    navigation/pilot/GNUmakefile \
    navigation/position/GNUmakefile \
    \
    perception/GNUmakefile \
    perception/scan2d/GNUmakefile \
    \
    skel/GNUmakefile \
    skel/dummy/GNUmakefile \
    \
    gui/GNUmakefile \
    gui/main/GNUmakefile \
    gui/drivers/GNUmakefile \
    gui/navigation/GNUmakefile \
    gui/perception/GNUmakefile \
    \
    tools/GNUmakefile \
    \
    examples/GNUmakefile \
    \
    scripts/rack-config \
    scripts/GNUmakefile
])

if test \! x$RACK_MAYBE_DOCDIR = x; then

AC_CONFIG_FILES([ \
    doc/GNUmakefile \
    doc/doxygen/GNUmakefile \
    doc/doxygen/Doxyfile \
])

fi

if test x"$CONFIG_RACK_JAVA" = x"y"; then
AC_CONFIG_COMMANDS([java-link], [if test ! -e ProjectName; then $LN_S . ProjectName; fi], [LN_S="$LN_S"])
else
AC_CONFIG_COMMANDS([java-unlink], [if test -e ProjectName; then rm -rf ProjectName; fi])
fi

AC_OUTPUT

dnl ======================================================================
dnl generate ${PROJNAME}_config.h
dnl ======================================================================

AC_MSG_NOTICE([creating ]ProjectName[_config.h])
sed -e "s/ PACKAGE/ RACK_PACKAGE/" -e "s/ VERSION/ RACK_VERSION/" \
    config/${PROJNAME}_config_pre.h > ${PROJNAME}_config.h.new
if cmp -s ${PROJNAME}_config.h.new ${PROJNAME}_config.h; then
    rm ${PROJNAME}_config.h.new
    AC_MSG_NOTICE(ProjectName[_config.h is unchanged])
else
    mv ${PROJNAME}_config.h.new ${PROJNAME}_config.h
fi
