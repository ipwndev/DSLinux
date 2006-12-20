#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

typedef struct
{
    unsigned short id;
    unsigned short size;
}
pixil_sync_net_t;

#define PIXIL_SYNC_ID 0x1234

int g_fd = 0;

int
pl_init(int argc, char **argv)
{
  char *hostname = argv[0];
  int port = atoi(argv[1]);
  
  struct hostent *h = 0;
  struct sockaddr_in saddr;
  
  if (g_fd)
    return g_fd;

  g_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (g_fd < 0) {
    printf("TCPIP:  Error [%s]\n", strerror(errno));
    return -1;
  }
  
  h = gethostbyname(hostname);
  if (!h) {
    printf("TCPIP: Unable to find the host %s [%s]\n", hostname,
	   strerror(errno));
    goto exit_init;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr = *((struct in_addr *) h->h_addr);

    if (connect(g_fd, (struct sockaddr *) &saddr, sizeof(saddr)) == 0) {
	printf("TCPIP: Connected to [%s:%d]\n", hostname, port);
	return g_fd;
    }
    
    printf("TCPIP: Unable to connect to %s [%s]\n", hostname, strerror(errno));

  exit_init:
    close(g_fd);
    g_fd = 0;

    return -1;
}

int 
pl_close(void) {
  if (g_fd) close(g_fd);
  g_fd = 0;
  return 0;
}

int
pl_getfd(void) {
  return g_fd;
}
  
int
pl_write(char *buffer, int size)
{
    pixil_sync_net_t *msg;
    char *out = (char *) calloc(sizeof(pixil_sync_net_t) + size, 1);
    char *p;
    int i;

    if (!g_fd || !buffer)
	return -1;

    msg = (pixil_sync_net_t *) out;

    msg->id = PIXIL_SYNC_ID;
    msg->size = (unsigned short) size;

    memcpy(out + sizeof(pixil_sync_net_t), buffer, size);
    return write(g_fd, out, size + sizeof(pixil_sync_net_t));
}

int
pl_read(char **buffer)
{

    pixil_sync_net_t msg;
    int ret;

    if (!g_fd)
	return -1;

    ret = read(g_fd, &msg, sizeof(pixil_sync_net_t));
    if (ret <= 0) {
	if (ret == -EAGAIN)
	    return 0;
	return -1;
    }

    if (msg.id != PIXIL_SYNC_ID)
	return 0;

    if (msg.size) {
	*buffer = (char *) calloc(msg.size, 1);
	if (!*buffer)
	    return -1;

	ret = read(g_fd, *buffer, msg.size);
    }

    return msg.size;
}
