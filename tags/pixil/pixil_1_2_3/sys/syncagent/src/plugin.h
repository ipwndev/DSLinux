#ifndef PLUGIN_H_
#define PLUGIN_H_

typedef struct plugin_struct
{
    char name[32];
    void *handle;

    int (*init) (int, char **);
  int (*close) (void);
  int (*getfd)(void);
    int (*read) (char **);
    int (*write) (char *, int);

    struct plugin_struct *next;
}
plugin_t;

plugin_t *load_plugin(char *filename);
void free_plugin(plugin_t *);

#endif
