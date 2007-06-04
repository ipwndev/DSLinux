#ifndef SYSCONF_PLUGIN_H
#define SYSCONF_PLUGIN_H

void sysconf_ipc_write(int id, char *, int);
int sysconf_ipc_find(char *);

NxApp *sysconf_get_instance(void);

#endif
