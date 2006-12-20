# Configure paths for libvideogfx
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_LIBVIDEOGFX([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]]))
dnl Test for LIBVIDEOGFX, and define LIBVIDEOGFX_CFLAGS and LIBVIDEOGFX_LIBS
dnl
AC_DEFUN(AM_PATH_LIBVIDEOGFX,
[dnl 
dnl Get the cflags and libraries from the libvideogfx-config script
dnl
AC_ARG_WITH(libvideogfx-prefix,[  --with-libvideogfx-prefix=PFX  Prefix where LibVideoGFX is installed (optional)],
            libvideogfx_prefix="$withval", libvideogfx_prefix="")
AC_ARG_WITH(libvideogfx-exec-prefix,[  --with-libvideogfx-exec-prefix=PFX Exec prefix where LibVideoGFX is installed (optional)],
            libvideogfx_exec_prefix="$withval", libvideogfx_exec_prefix="")
AC_ARG_ENABLE(libvideogfx-test, [  --disable-libvideogfx-test      Do not try to compile and run a test LibVideoGFX program],
		    , enable_libvideogfx_test=yes)

  if test x$g_cim_exec_prefix != x ; then
     g_cim_config_args="$g_cim_config_args --prefix=$g_cim_config_exec_prefix"
     if test x${G_CIM_CONFIG+set} != xset ; then
        G_CIM_CONFIG=$g_cim_exec_prefix/bin/g-cim-config
     fi
  fi
  if test x$g_cim_prefix != x ; then
     g_cim_config_args="$g_cim_config_args --prefix=$g_cim_config_prefix"
     if test x${G_CIM_CONFIG+set} != xset ; then
        G_CIM_CONFIG=$g_cim_prefix/bin/g-cim-config
     fi
  fi

  for module in . $4
  do
      case "$module" in
         g-cim-io) 
             g_cim_config_args="$g_cim_config_args g-cim-io"
         ;;
         g-cim-xml) 
             g_cim_config_args="$g_cim_config_args g-cim-xml"
         ;;
         g-cim-objects) 
             g_cim_config_args="$g_cim_config_args g-cim-objects"
         ;;
      esac
  done

  AC_PATH_PROG(G_CIM_CONFIG, g-cim-config, no)
  min_g_cim_version=ifelse([$1], ,1.2.0,$1)
  AC_MSG_CHECKING(for GCim - version >= $min_g_cim_version)
  no_g_cim=""
  if test "$G_CIM_CONFIG" = "no" ; then
    no_g_cim=yes
  else
    G_CIM_CFLAGS=`$G_CIM_CONFIG $g_cim_config_args --cflags`
    G_CIM_LIBS=`$G_CIM_CONFIG $g_cim_config_args --libs`

    g_cim_config_major_version=`$G_CIM_CONFIG $g_cim_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    g_cim_config_minor_version=`$G_CIM_CONFIG $g_cim_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    g_cim_config_micro_version=`$G_CIM_CONFIG $g_cim_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_g_cim_test" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
       CFLAGS="$CFLAGS $G_CIM_CFLAGS"
      LIBS="$LIBS $G_CIM_LIBS"
dnl
dnl Now check if the installed GCim is sufficiently new. (Also sanity
dnl checks the results of g-cim-config to some extent
dnl
      rm -f conf.g-cim-test
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>

#include <g-cim/g-cim.h>

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.g-cim-test");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_g_cim_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_g_cim_version");
     exit(1);
   }

  if ((g_cim_major_version != $g_cim_config_major_version) ||
      (g_cim_minor_version != $g_cim_config_minor_version) ||
      (g_cim_micro_version != $g_cim_config_micro_version))
    {
      printf("\n*** 'g-cim-config --version' returned %d.%d.%d, but GCim (%d.%d.%d)\n", 
             $g_cim_config_major_version, $g_cim_config_minor_version, $g_cim_config_micro_version,
             g_cim_major_version, g_cim_minor_version, g_cim_micro_version);
      printf ("*** was found! If g-cim-config was correct, then it is best\n");
      printf ("*** to remove the old version of GCim. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If g-cim-config was wrong, set the environment variable G_CIM_CONFIG\n");
      printf("*** to point to the correct copy of g-cim-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
  else if ((g_cim_major_version != G_CIM_MAJOR_VERSION) ||
	   (g_cim_minor_version != G_CIM_MINOR_VERSION) ||
           (g_cim_micro_version != G_CIM_MICRO_VERSION))
    {
      printf("*** GCim header files (version %d.%d.%d) do not match\n",
	     G_CIM_MAJOR_VERSION, G_CIM_MINOR_VERSION, G_CIM_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     g_cim_major_version, g_cim_minor_version, g_cim_micro_version);
    }
  else
    {
      if ((g_cim_major_version > major) ||
        ((g_cim_major_version == major) && (g_cim_minor_version > minor)) ||
        ((g_cim_major_version == major) && (g_cim_minor_version == minor) && (g_cim_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GCim (%d.%d.%d) was found.\n",
               g_cim_major_version, g_cim_minor_version, g_cim_micro_version);
        printf("*** You need a version of GCim newer than %d.%d.%d.\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the g-cim-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GCim, but you can also set the G_CIM_CONFIG environment to point to the\n");
        printf("*** correct copy of g-cim-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_g_cim=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_g_cim" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$G_CIM_CONFIG" = "no" ; then
       echo "*** The g-cim-config script installed by GCim could not be found"
       echo "*** If GCim was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the G_CIM_CONFIG environment variable to the"
       echo "*** full path to g-cim-config."
     else
       if test -f conf.g-cim-test ; then
        :
       else
          echo "*** Could not run g-cim test program, checking why..."
          CFLAGS="$CFLAGS $G_CIM_CFLAGS"
          LIBS="$LIBS $G_CIM_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <stdlib.h>

#include <g-cim/g-cim.h>

#ifndef G_CIM_CHECK_VERSION
#define G_CIM_CHECK_VERSION(major, minor, micro) \
    (G_CIM_MAJOR_VERSION > (major) || \
     (G_CIM_MAJOR_VERSION == (major) && G_CIM_MINOR_VERSION > (minor)) || \
     (G_CIM_MAJOR_VERSION == (major) && G_CIM_MINOR_VERSION == (minor) && \
      G_CIM_MICRO_VERSION >= (micro)))
#endif
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GCim or finding the wrong"
          echo "*** version of GCim. If it is not finding GCim, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GCim was incorrectly installed"
          echo "*** or that you have moved GCim since it was installed. In the latter case, you"
          echo "*** may want to edit the g-cim-config script: $G_CIM_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     G_CIM_CFLAGS=""
     G_CIM_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(G_CIM_CFLAGS)
  AC_SUBST(G_CIM_LIBS)
  rm -f conf.g-cim-test
])
