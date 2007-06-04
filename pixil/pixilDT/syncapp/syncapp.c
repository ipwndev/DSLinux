/* This is a simple sync agent - it opens up a 
   socket for listening, sends out anything that comes
   in on stdin and outputs the status on stdout.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef struct
{
    unsigned short id;
    unsigned short size;
}
pixil_sync_net_t;

#define PIXIL_SYNC_ID 0x1234

static int server = 0;
static int client = 0;

int
network_setup(unsigned long port)
{

    struct sockaddr_in saddr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
	goto net_error;

    if (listen(sock, 1) == -1)
	goto net_error;

    fprintf(stderr, "MESSAGE:  Network has been set up for listening\n");
    return sock;

  net_error:
    fprintf(stderr, "ERROR:  Network error [%s]\n", strerror(errno));
    close(sock);
    return -1;
}

int
write_net(int fd, char *buf, int len)
{
    pixil_sync_net_t *msg;
    char *out = (char *) calloc(sizeof(pixil_sync_net_t) + len, 1);

    if (!fd || !buf)
	return -1;

    msg = (pixil_sync_net_t *) out;
    msg->id = PIXIL_SYNC_ID;
    msg->size = len;

    memcpy(out + sizeof(pixil_sync_net_t), buf, len);
    return write(fd, out, len + sizeof(pixil_sync_net_t));
}


int
read_net(int fd, char **buffer)
{
    pixil_sync_net_t msg;
    int ret;
    char *p;
    int i;

    if (!fd)
	return -1;

    ret = read(fd, &msg, sizeof(pixil_sync_net_t));

    if (ret <= 0) {
	if (ret == -EAGAIN)
	    return 0;
	fprintf(stderr, "ERROR:  [%s]\n", strerror(errno));
	close(fd);

	return -1;
    }

    if (msg.id != PIXIL_SYNC_ID) {
	fprintf(stderr, "ERROR:  Invalid sync message.  Ignoring\n");
	return 0;
    }

    if (msg.size <= 0)
	return msg.size;

    *buffer = (char *) calloc(msg.size + 1, 1);

    if (!*buffer) {
	fprintf(stderr, "Error:  Unable to allocate a buffer of size %d\n",
		msg.size + 1);

	return -1;
    }

    ret = read(fd, *buffer, msg.size);

    if (ret == -1) {
	fprintf(stderr, "ERROR:  [%s]\n", strerror(errno));
	close(fd);
	free(*buffer);
	return -1;
    }

    return msg.size;
}

int
send_message(int incoming, int network)
{
    char size[10];
    char ch;
    char *ptr = size, *buffer = 0;
    int ret, len;

    bzero(size, sizeof(size));

    ret = read(incoming, &ch, 1);

    while (1) {
	if (ret <= 0)
	    return -1;

	if (ch == ';')
	    break;
	*ptr++ = ch;

	ret = read(incoming, &ch, 1);

    }

    len = atoi(size);
    if (len == 0)
	return 0;
    if (len == -1)
	return -1;		/* Error from the other side o' the pipe */

    buffer = (char *) calloc(len + 1, 1);
    ret = read(incoming, buffer, len);
    if (ret < 0)
	return -1;

    ret = write_net(network, buffer, len);
    return (ret == len) ? 0 : -1;
}

int
get_message(int network, int outgoing)
{
    char *buffer = 0;
    char size[10];
    int len = read_net(network, &buffer);

    if (len == 0)
	return 0;		/* Ignore that read */

    if (len == -1) {
	/* A true, honest to god error.  Bail out now */
	write(outgoing, "-1;", 3);
	return -1;
    }

    sprintf(size, "%d;", len);

    write(outgoing, &size, strlen(size));
    write(outgoing, buffer, len);

    //write(outgoing, "\n", 1);

    free(buffer);
    return 0;
}

static void
close_syncapp(void) {
  if (client) close(client);
  if (server) close(server);

  client = server = 0;
}

void
sighandler(int sig)
{
    exit(0);
}

int
main(int argc, char **argv)
{

    int ret = -1;
    int flags;

    atexit(close_syncapp);      /* Lazy, but the best way to ensure that the sockets get closed */
    signal(SIGTERM, sighandler);

    server = network_setup(2000);

    if (server == -1)
	return -1;

    fprintf(stderr, "Desktop sync application started\n");

    /* Make sure that the stdin doesn't block on read */

    flags = fcntl(fileno(stdin), F_GETFL);
    fcntl(fileno(stdin), F_SETFL, flags | O_NONBLOCK);

    while (1) {
	fd_set fdset;
	int max;

	FD_ZERO(&fdset);

	FD_SET(fileno(stdin), &fdset);	/* stdin */
	FD_SET(server, &fdset);	/* network */

	if (client > 0)
	    FD_SET(client, &fdset);	/* client */

	max = client > server ? client : server;

	ret = select(max + 1, &fdset, 0, 0, 0);
	if (ret <= 0)
	    break;

	/* Message incoming on the pipe */
	if (FD_ISSET(fileno(stdin), &fdset)) {
	    send_message(fileno(stdin), client);
	}

	if (client > 0 && FD_ISSET(client, &fdset)) {
	    if (get_message(client, fileno(stdout)) == -1) {
		fprintf(stderr, "Shut down client %d\n", client);
		client = 0;
	    }
	}

	if (FD_ISSET(server, &fdset)) {
	    struct sockaddr_in saddr;
	    int size = sizeof(saddr);

	    int lo = accept(server, (struct sockaddr *) &saddr, &size);

	    /* Only 1 client at a time */

	    if (lo > 0) {
		if (client)
		    close(lo);
		else {
		    client = lo;
		    fprintf(stderr, "MESSAGE: New client connected on %d\n",
			    client);
		}
	    } else
		goto exit_main;
	}
    }

    fprintf(stderr, "Closing down the desktop sync application\n");
    ret = 0;

  exit_main:
    if (server)
	close(server);
    if (client)
	close(client);

    /* Bye bye */
    return ret;
}
