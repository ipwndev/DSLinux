/*
 * vcstime.c
 *
 * Show time in upper right hand corner of the console screen
 * aeb, 951202, following a suggestion by Miguel de Icaza.
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#if TM_IN_SYS_TIME
# include <sys/time.h>
#else
# include <time.h>
#endif

#include <lct/utils.h>
#include <lct/local.h>

static void usage(char *progname)
{
  printf(_("Usage: %s\n"
	   "Show time in upper right hand corner of the console screen\n"), progname);
  OPTIONS_ARE();

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

void
fatal(char *s) {
    perror(s);
    exit(1);
}

unsigned char
number_of_columns() {
    int fda;
    unsigned char rc[2];

    if((fda = open("/dev/vcsa", O_RDONLY)) < 0)
	fatal("/dev/vcsa");
    if(read(fda, rc, 2) != 2)
	fatal("/dev/vcsa");
    close(fda);
    return rc[1];
}

int
main(int argc, char **argv)
{
    int fd;
    int cols = number_of_columns();
    time_t tid;
    struct tm *t;
    char tijd[10];

    setuplocale();
  
    simple_options (argc, argv, usage, strip_path(argv[0]));

    if((fd = open("/dev/vcs", O_WRONLY)) < 0)
	fatal("/dev/vcs");

    while(1) {
	lseek(fd, cols-10, 0);
	tid = time(0);
	t = localtime(&tid);
	sprintf(tijd, " %02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
	write(fd, tijd, 9);
	usleep(500000L);	/* or sleep(1); */
    }
}
