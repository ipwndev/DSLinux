/* Ugly?  Yes.  This is a very simple version of the agent, which doesn't do a lot more
   than introduce the agent architecture.  We don't bother messing with any of the sync
   code that already exists
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <ctype.h>

#include <ipc/colosseum.h>
#include <sync/msg_defs.h>

#include "plugin.h"

static int g_src = 0;
static plugin_t *g_plugin = 0;

static int parse_msgid(char *buffer) {
  int id = atoi(buffer);
  return id;
}

static void respond_ok(int id) {
  ClSendMessage(id, "200^", 4);
}

static void respond_error(int id, char *error) {
  char *str = (char *) calloc(strlen(error) + 5, 1);
  sprintf(str, "100^%s", error);

  ClSendMessage(id, error, strlen(error));
  free(str);
}

/* Buffer format:
   1000^plugin^arg1^arg2... 
*/

int handle_connect(char *buffer) {
  
  int argc = 0, ret = -1;

  char **argv = 0;
  char *end, *start = strchr(buffer, '^');
  char *plugin;
  char *ptr;

  if (!start) return -1;
  end = strchr(start + 1, '^');

  plugin = start + 1;

  if (end) {
    *end = 0;
    start = end + 1;
    
    while(1) {
      end = strchr(start, '^');
      if (!argv) argv = (char **) calloc(argc + 1, sizeof(char *));
      else argv = (char **) realloc(argv, (argc + 1 ) * sizeof(char *));
      argv[argc++] = start;
  
      if (!end) break;
      *end = 0;
      start = end + 1;
    }
  }

  /* Go lowercase on the plugin name */

  ptr = plugin;

  while(*ptr) 
    *ptr++ = tolower(*ptr);

  g_plugin = load_plugin(plugin);

  if (g_plugin) 
    ret = g_plugin->init(argc, argv);
  
  if (argv) free(argv);

  if (ret == -1 && g_plugin) {
    free_plugin(g_plugin);
    g_plugin = 0;
  }

  return (ret > 0) ? 0 : -1;
}

void handle_disconnect(void) {
  if (!g_plugin) return;
  g_plugin->close();

  free_plugin(g_plugin);
  g_plugin = 0;
}

int
handle_incoming(plugin_t * plugin)
{
    char *buffer = 0;
    int len = plugin->read(&buffer);
    
    if (len <= 0)
	return len;

    if (g_src)
      ClSendMessage(g_src, buffer, len);
    
    free(buffer);

    return 0;
}

int
handle_outgoing(plugin_t * plugin)
{
    int ret, size = CL_MAX_MSG_LEN;
    int msg_id;

    unsigned short src;

    char *buffer = (char *) calloc(CL_MAX_MSG_LEN, 1);
    if (!buffer)
	return -1;

    ret = ClGetMessage(buffer, &size, &src);

    if (ret < 0) {
      free(buffer);
      return -1;
    }

    /* Check to see if it is destined for us */
    msg_id = parse_msgid(buffer);

    if (msg_id == CONNECT) {
      printf("SYNAGENT:  Incoming connect request\n");

      if (handle_connect(buffer) == 0) 
	respond_ok(src);
      else
	respond_error(src, "Unable to connect to the remote agent\n");
    }

    else if (msg_id == DISCONNECT) {
      printf("SYNAGENT:  Incomming disconnect request\n");
      handle_disconnect();
      respond_ok(src);
    }
    else if (plugin) {
      g_src = src;
      plugin->write(buffer, size);
    }
    else  printf("SYNAGENT:  I don't know what to do with the incoming message [%d] [%s]\n", ret, buffer);
    free(buffer);
    return 0;
}

int
main(int argc, char **argv)
{

  int clsock, flags;

    clsock = ClRegister("syncagent", &flags);

    if (clsock < 0) {
	printf("SYNCAGENT:  Colosseum returned %d\n", clsock);
	exit(-1);
    }

    while (1) {
	fd_set fdset;
	int ret, max = clsock;

	FD_ZERO(&fdset);

	if (g_plugin) {
	  FD_SET(g_plugin->getfd(), &fdset);
	  if (g_plugin->getfd() > clsock) max = g_plugin->getfd();
	}

	FD_SET(clsock, &fdset);

	ret = select(max + 1, &fdset, 0, 0, 0);

	if (ret < 0) break;

	if (g_plugin && FD_ISSET(g_plugin->getfd(), &fdset))
	  if (handle_incoming(g_plugin) == -1)
	    break;

	if (FD_ISSET(clsock, &fdset))
	  if (handle_outgoing(g_plugin) == -1)
	    break;
    }

    printf("SYNCAGENT:  Shutting down\n");
    
    if (g_plugin->getfd()) 
      handle_disconnect();
      
    ClClose();    
    return 0;
}
