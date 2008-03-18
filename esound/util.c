#include "config.h"
#include "esd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Run-time check for IPv6 support */
int 
have_ipv6(void) {
#if defined ENABLE_IPV6
  int s;
  s = socket(AF_INET6, SOCK_STREAM, 0);
  if(s != -1) {
    close(s);
    return (1);
  }
#endif
  return (0);
}

const char*
esd_get_socket_dirname (void) 
{
	const char *audiodev;
	static char *dirname = NULL;

	if (dirname == NULL) {
		if (!(audiodev = getenv("AUDIODEV"))) {
			audiodev = "";
		} else {
			char *newdev = strrchr(audiodev, '/');
			if (newdev != NULL) {
				audiodev = newdev++;
			}
		}
		dirname = malloc(strlen(audiodev) + sizeof("/tmp/.esd"));
		strcpy(dirname, "/tmp/.esd");
		strcat(dirname, audiodev);
	}

	return dirname;
}

const char*
esd_get_socket_name (void) 
{
	const char *dirname;
	static char *name = NULL;

	if (name == NULL) {
		dirname = esd_get_socket_dirname();
		name = malloc(strlen(dirname) + sizeof("/socket"));
		strcpy(name, dirname);
		strcat(name, "/socket");
	}

	return name;
}
