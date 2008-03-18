AC_PREREQ([2.58])
AC_INIT(esound, 0.2.38)

ESD_MAJOR_VERSION=0
ESD_MINOR_VERSION=2
ESD_MICRO_VERSION=38
ESD_VERSION=$ESD_MAJOR_VERSION.$ESD_MINOR_VERSION.$ESD_MICRO_VERSION

AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE(1.6)
AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)

ACLOCAL_AMFLAGS="-I m4 $ACLOCAL_AMFLAGS"

AC_CONFIG_SRCDIR([esd.h])

dnl set version info for libesd.so to package rev - $MAJOR + $MINOR : $MICRO : $MINOR
ESD_VERSION_INFO=`expr $ESD_MINOR_VERSION + $ESD_MAJOR_VERSION`:$ESD_MICRO_VERSION:$ESD_MINOR_VERSION

AC_SUBST(ESD_MAJOR_VERSION)
AC_SUBST(ESD_MINOR_VERSION)
AC_SUBST(ESD_MICRO_VERSION)
AC_SUBST(ESD_VERSION)
AC_SUBST(ESD_VERSION_INFO)

pkgdocdir="\$(datadir)/doc/esound"
AC_SUBST(pkgdocdir)

AC_PROG_CC
AC_PROG_CPP
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL


dnl Check for tools

AC_PROG_MAKE_SET

AC_PROG_INSTALL

dnl Check for compiler environment

AC_C_CONST
AC_C_BIGENDIAN
AC_C_INLINE

dnl Check for system libs needed

AC_FUNC_ALLOCA
AC_CHECK_FUNCS(setenv putenv fchown fchmod gethostbyname2)
AC_CHECK_FUNC(connect,,[AC_CHECK_LIB(socket,connect)])
AC_CHECK_FUNC(gethostbyname,,[AC_CHECK_LIB(nsl,gethostbyname)])
AC_CHECK_FUNC(hstrerror,,[AC_CHECK_LIB(resolv,hstrerror)])
AC_CHECK_FUNC(nanosleep,,[
  AC_CHECK_LIB(rt,nanosleep,,[AC_CHECK_LIB(posix4,nanosleep)])])
AC_CHECK_FUNCS(usleep)
AC_CHECK_FUNCS(strtok_r)

use_inet_pton=no
AC_CHECK_FUNC(inet_pton,use_inet_pton=yes,[AC_CHECK_LIB(resolv,inet_pton)])
       if test "x$use_inet_pton" = "xyes"; then
               case $ac_cv_func_inet_pton/$ac_cv_lib_resolv_inet_pton in
               no/no) ;;
               *) AC_DEFINE(HAVE_INET_PTON, 1, [Defined if inet_pton() works])
               esac
       else
               AC_CHECK_FUNC(inet_aton,,[AC_CHECK_LIB(resolv,inet_aton)])
               case $ac_cv_func_inet_aton/$ac_cv_lib_resolv_inet_aton in
               no/no) ;;
               *) AC_DEFINE(HAVE_INET_ATON, 1, [Defined if inet_aton() works])
               esac
       fi

case $ac_cv_func_nanosleep/$ac_cv_lib_rt_nanosleep/$ac_cv_lib_posix4_nanosleep in
no/no/no) ;;
*) AC_DEFINE(HAVE_NANOSLEEP, 1, [Defined if nanosleep() works])
esac

dnl Check if INADDR_LOOPBACK exists
  
AC_TRY_COMPILE([
    #include<sys/types.h>
    #include<netinet/in.h>],
    [unsigned long ul = INADDR_LOOPBACK;],,
    [AC_DEFINE(INADDR_LOOPBACK, (u_int32_t)0x7f000001, [Defined as the address of the loopback interface])])
    

dnl ========================================================================
dnl Begin IPv6 checks
dnl ========================================================================
AC_MSG_CHECKING([whether to enable ipv6])
AC_ARG_ENABLE(ipv6, [  --enable-ipv6 enable IPv6 extensions], ,enable_ipv6=yes)

if test $enable_ipv6 = yes; then
  AC_TRY_COMPILE([
    #include <sys/types.h>
    #include <sys/socket.h>], [
    socket(AF_INET6, SOCK_STREAM, 0)
    ],
    have_ipv6=yes,
    have_ipv6=no
  )
  AC_MSG_RESULT($have_ipv6)

  if test $have_ipv6=yes; then
    have_getaddrinfo=no
    AC_CHECK_FUNC(getaddrinfo, have_getaddrinfo=yes)
    if test $have_getaddrinfo = no; then
      for lib in bsd socket inet; do
        AC_CHECK_LIB($lib,getaddrinfo,[LIBS="$LIBS -l$lib";have_getaddrinfo=yes;break])
      done
    fi

    if test $have_getaddrinfo = yes; then
      AC_DEFINE(ENABLE_IPV6, 1, [Defined if ipv6 is enabled])
    fi


    dnl Check for inet_ntop. We need it only for debugging
    AC_CHECK_FUNC(inet_ntop, AC_DEFINE(HAVE_INET_NTOP, 1, [Defined if inet_ntop() works]))
  fi
else
  AC_MSG_RESULT(no)
fi

dnl ========================================================================
dnl End of IPv6 checks
dnl ========================================================================

dnl Check if SUN_LEN exists

AC_TRY_LINK([
    #include<sys/types.h>
    #include<sys/un.h>], 
    [struct sockaddr_un su; int i = SUN_LEN(&su);],
    [AC_DEFINE(HAVE_SUN_LEN, 1, [Defined if SUN_LEN() exists])], )

dnl check for getopt_long, substitute the distributed versions if not

AC_CHECK_FUNC(getopt_long,[have_getopt_long=yes],[have_getopt_long=no])
AM_CONDITIONAL(NEED_GETOPT, test "x$have_getopt_long" = xno)

dnl Needed for FIONBIO on Solaris at least

AC_CHECK_HEADERS(sys/filio.h sys/ioctl.h)

dnl see if we want to enable insanely verbose debugging

AC_ARG_ENABLE(debugging,
  AC_HELP_STRING([--enable-debugging],[enable verbose diagnostic info [default=no]]),
  [enable_debuggin=${enableval}],
  [enable_debugging=no])

if test "x$enable_debugging" = "xyes"; then
  AC_DEFINE(ESDBG, 1, [Defined if debugging is enabled])
fi

dnl see if we intentionally want to disable local sound

AC_ARG_ENABLE(local_sound,
  AC_HELP_STRING([--enable-local-sound],[turn on local sound playing [default=yes]]),
  [enable_local_sound=${enableval}],
  [enable_local_sound=yes])
AC_ARG_ENABLE(oss,
  AC_HELP_STRING([--enable-oss],[use OSS if available [default=yes]]),
  [enable_oss=${enableval}],
  [enable_oss=yes])
AC_ARG_ENABLE(alsa,
  AC_HELP_STRING([--enable-alsa],[use ALSA if available [default=yes]]),
  [enable_alsa=${enableval}],
  [enable_alsa=yes])
AC_ARG_ENABLE(arts,
  AC_HELP_STRING([--enable-arts],[use arts if available [default=no]]),
  [enable_arts=${enableval}],
  [enable_arts=no])

dnl build the esddsp wrapper for systems, 
dnl that support LD_PRELOAD and RTLD_NEXT only
dsp_ok=no DL_LIB=

AC_MSG_CHECKING([if your platform supports esddsp])
case "$host_os" in
  linux* | freebsd* | kfreebsd*-gnu | bsdi4* )
    dsp_ok=yes
    ;;
esac
AC_MSG_RESULT($dsp_ok)

if test "x$dsp_ok" = xyes; then
   AC_CHECK_FUNC(dlopen, DL_LIB=,
      AC_CHECK_LIB(dl, dlopen, DL_LIB=-ldl))
   ESDDSP=esddsp
   LIBESDDSP=libesddsp.la
fi
AM_CONDITIONAL(BUILD_ESDDSP, test "x$dsp_ok" = xyes)

if test "x$enable_local_sound" = "xyes"; then
   found_sound=no

   dnl Check for audio header files needed

   echo "---------------------------------------------------------------------"
   echo "--- Checking to see which audio header files your system uses.";
   echo "--- Most of these checks should fail. Do not be alarmed.";
   AC_CHECK_HEADERS(soundcard.h sys/soundcard.h machine/soundcard.h sys/audio.h)
   AC_CHECK_HEADERS(sys/audioio.h sys/audio.io.h sun/audioio.h)
   AC_CHECK_HEADERS(dmedia/audio.h sys/soundlib.h sys/asoundlib.h alsa/asoundlib.h)
   AC_CHECK_HEADERS(CoreAudio/CoreAudio.h)
   # mme_api.h directly includes other files from the mme subdir
   xCPPFLAGS="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS -I/usr/include/mme"
   AC_CHECK_HEADERS(mme/mme_api.h)
   CPPFLAGS="$xCPPFLAGS"
   AM_PATH_ARTS(, HAVE_ARTS=yes, HAVE_ARTS=no)

   dnl Define the driver needed based on the first header file found

   if test x"$enable_oss" = xyes ; then
     if test "${ac_cv_header_sys_soundcard_h}" = "yes" || \
        test "${ac_cv_header_soundcard_h}" = "yes" || \
        test "${ac_cv_header_machine_soundcard_h}" = "yes"; then

        dnl Platform mklinux/powerpc needs special care and feeding
        found_sound=yes
        AC_DEFINE(DRIVER_OSS, 1, [Defined if OSS backend is enabled])
     fi
   fi

   if test "${ac_cv_header_sys_audio_h}" = "yes"; then
      case ${host_os} in
      hpux*)
          found_sound=yes
          AC_DEFINE(DRIVER_HPUX, 1, [Defined if HPUX backend is enabled])
      ;;
      aix*)
          found_sound=yes
          AC_DEFINE(DRIVER_AIX, 1, [Defined if AIX backend is enabled])
      esac
   fi

   if test "${ac_cv_header_sys_audioio_h}" = "yes" || \
      test "${ac_cv_header_sys_audio_io_h}" = "yes" || \
      test "${ac_cv_header_sun_audioio_h}" = "yes"; then
      case ${host_os} in
      solaris*)
         found_sound=yes
         AC_DEFINE(DRIVER_SOLARIS, 1, [Defined if Solaris backend is enabled])
         ;;
      esac
   fi

   case ${host_os} in
   os2*)
      found_sound=yes
      AC_DEFINE(DRIVER_DART, 1, [Defined if OS/2 backend is enabled])
      ;;
   esac

   if test "${ac_cv_header_dmedia_audio_h}" = "yes"; then
      case ${host_os} in
      irix*)
         found_sound=yes
         AC_DEFINE(DRIVER_IRIX, 1, [Defined if IRIX backend is enabled])
      esac
   fi

   if test "${ac_cv_header_mme_mme_api_h}" = "yes"; then
      case ${host_os} in
      osf*)
	found_sound=yes
	AC_DEFINE(DRIVER_OSF, 1, [Defined if OSF backend is enabled])
	CPPFLAGS="$CPPFLAGS -I/usr/include/mme"
	SOUND_LIBS=-lmme
      esac
   fi

   if test "x$enable_alsa" = "xyes"; then
      if test "${ac_cv_header_sys_soundlib_h}" = "yes"; then
         found_sound=yes
         AC_DEFINE(DRIVER_ALSA, 1, [Defined if ALSA backend is enabled (deprecated)])
      fi
      if test "${ac_cv_header_alsa_asoundlib_h}" = "yes"; then
         found_sound=yes
         AC_DEFINE(DRIVER_ALSA_09, 1, [Defined if ALSA-1.0 backend is enabled])
      fi
      if test "${ac_cv_header_sys_asoundlib_h}" = "yes"; then
         found_sound=yes
         AC_DEFINE(DRIVER_NEWALSA, 1, [Defined if old ALSA backend is enabled (deprecated)])
      fi
   fi

   if test "${ac_cv_header_CoreAudio_CoreAudio_h}" = "yes"; then
      found_sound=yes
      AC_DEFINE(DRIVER_COREAUDIO, 1, [Defined if CoreAudio backend is enabled])
      SOUND_LIBS="-Wl,-framework,CoreAudio"
   fi

   if test "x$enable_arts" = "xyes" ; then
     if test "x$HAVE_ARTS" = "xyes"; then
       found_sound=yes
       CFLAGS="$CFLAGS $ARTSC_CFLAGS"
       LIBS="$LIBS $ARTS_LIBS"
       AC_DEFINE(DRIVER_ARTS, 1, [Defined if Arts backend is enabled])
     fi
   fi

   if test "$found_sound" = "no"; then
      AC_MSG_ERROR([Could not find a support sound driver])
   fi

   dnl Check for additional audio libs needed

   echo "---------------------------------------------------------------------"
   echo "--- Checking to see which audio libraries are required for linking.";
   echo "--- Most of these checks should also fail. Do not be alarmed.";
   AC_CHECK_FUNC(_oss_ioctl,,[AC_CHECK_LIB(ossaudio,_oss_ioctl)])
   AC_CHECK_FUNC(ALnewconfig,,[AC_CHECK_LIB(audio,ALnewconfig)])
   if test "x$enable_alsa" = "xyes"; then
      AC_CHECK_FUNC(snd_cards,,[AC_CHECK_LIB(sound,snd_cards)])
      AC_CHECK_FUNC(snd_cards,,[AC_CHECK_LIB(asound,snd_cards)])
      AC_CHECK_FUNC(snd_pcm_pause,,[AC_CHECK_LIB(asound,snd_pcm_pause)])

      dnl Check if ALSA uses new API
      saved_cflags="$CFLAGS"
      CFLAGS="$CFLAGS -Werror"
      echo -n "checking for alsa new PCM API... "
      AC_TRY_LINK([
	  #include <alsa/asoundlib.h>
	  ], [
	  snd_pcm_t *pcm;
	  snd_pcm_hw_params_t *params;
	  unsigned int val;
	  int dir;
	  /* intentionally not uses pointers, trying to replicate bug 129709 */
	  snd_pcm_hw_params_set_rate_near(pcm, params, val, &dir);
	  ],[ echo "no" ], AC_DEFINE(DRIVER_ALSA_09_NEW_PCM_API, 1, [Defined if alsa-0.9 new pcm api is detected]) [ echo "yes" ],
	  ])

      CFLAGS="$saved_cflags"
   fi


else
   AC_DEFINE(DRIVER_NONE, 1, [Defined if no backend is enabled])
fi

dnl offer daemon in serverdir

AC_ARG_WITH(esd-dir, [  --with-esd-dir=PATH     specify location of esound daemon ])

if test "x$with_esd_dir" = "x" ; then
  ESD_DIR='$(prefix)/bin'
  ESD_PC_DIR='${prefix}/bin'
else
  ESD_DIR=$with_esd_dir
  ESD_PC_DIR=$with_esd_dir
fi
AC_SUBST(ESD_DIR)
AC_SUBST(ESD_PC_DIR)

dnl make sure we have the library, -laudiofile

build_esdplay=false

PKG_CHECK_MODULES(AUDIOFILE, [audiofile >= 0.1.5])

CFLAGS="$CFLAGS $AUDIOFILE_CFLAGS"
LIBS="$LIBS $AUDIOFILE_LIBS"

build_esdplay=true

AM_CONDITIONAL(BUILD_ESDPLAY, $build_esdplay)

echo "---------------------------------------------------------------------"
echo "--- Checking for the documentation generation tools.  NOTE: These "
echo "--- are NOT required for proper compilation of the esound package."

AC_CHECK_PROG(JW, jw, jw, false)
AM_CONDITIONAL(HAVE_JW, test "x$JW" != xfalse)

echo "---------------------------------------------------------------------"

AC_ARG_WITH(libwrap,[  --with-libwrap          use tcp wrappers [default=no]], , wrap_ok=no)

WRAP_LIB=
if test "x$with_libwrap" = "xyes"; then
   esound_save_LIBS="$LIBS"
   LIBS="$LIBS -lwrap"

   AC_MSG_CHECKING([for tcp_wrapper library and headers])

   wrap_ok=no
   AC_TRY_LINK(
[#include <tcpd.h>
#include <syslog.h>
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;],
[struct request_info *request; return hosts_access (request);],
      [AC_DEFINE(USE_LIBWRAP, 1, [Defined if libwrap is used])
       WRAP_LIB="-lwrap"
       wrap_ok=yes],
      [LIBS="$LIBS -lnsl"
       AC_TRY_LINK(
[#include <tcpd.h>
#include <syslog.h>
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;],
[struct request_info *request; return hosts_access (request);],

         [AC_DEFINE(USE_LIBWRAP, 1, [Defined if libwrap is used])
          WRAP_LIB="-lwrap -lnsl"
          wrap_ok=yes])])

   AC_MSG_RESULT($wrap_ok)

   LIBS="$esound_save_LIBS"
fi

AS_COMPILER_FLAG(-Wall, CFLAGS="$CFLAGS -Wall")

# set | sort; # just for double checking the script...

AC_SUBST(WRAP_LIB)
AC_SUBST(SOUND_LIBS)
AC_SUBST(AUDIOFILE_LIBS)
AC_SUBST(DL_LIB)
AC_SUBST(DOCS)
AC_SUBST(DISTDOCS)
AC_SUBST(JW)

AC_OUTPUT([
Makefile
esd-config
esound.spec
esound.pc
esddsp
docs/Makefile
docs/esd.1
docs/esdcat.1
docs/esd-config.1
docs/esdctl.1
docs/esddsp.1
docs/esdfilt.1
docs/esdloop.1
docs/esdmon.1
docs/esdplay.1
docs/esdrec.1
docs/esdsample.1
])
dnl , [case "$CONFIG_FILES" in *esd-config*) chmod +x esd-config;; esac])

echo ""
echo "Esound Configure Settings:"
echo ""
echo "Debugging support: $enable_debugging" 
echo "libwrap support:   $wrap_ok"
echo ""
echo "LIBS=$LIBS"
echo ""

