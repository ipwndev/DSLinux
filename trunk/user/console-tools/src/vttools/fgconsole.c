/*
 * fgconsole.c - aeb - 960123 - Print foreground console
 * ydi, 1997-09-22: use getfd()
 */
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/vt.h>

#include <lct/local.h>
#include <lct/console.h>

static void usage(char *progname)
{
  printf(_("Usage: %s [vt_number]\n"
	   "Print foreground console\n"), progname);
  OPTIONS_ARE();

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

void main(int argc, char **argv)
{
  struct vt_stat vtstat;
  int fd;

  setuplocale();
  
  simple_options (argc, argv, usage, strip_path (argv[0]));

  if (-1 == (fd = get_console_fd(NULL))) exit (1);
  
  if (ioctl(fd, VT_GETSTATE, &vtstat)) 
    {
      perror("fgconsole: VT_GETSTATE");
      exit(1);
    }
  printf("%d\n", vtstat.v_active);
}
