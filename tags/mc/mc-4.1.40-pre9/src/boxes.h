#ifndef __BOXES_H
#define __BOXES_H

extern char *user_format [];

int display_box (WPanel *p, char **user, char **mini, int *use_msformat, int num);
sortfn *sort_box (sortfn *sort_fn, int *reverse, int *case_sensitive, int *exe_first);
void confirm_box (void);
void display_bits_box ();
void file_highlighting_box (void);
#ifdef USE_VFS
void configure_vfs ();
#endif
void jobs_cmd ();
char *cd_dialog (void);
void symlink_dialog (char *existing, char *new, char **ret_existing, char **ret_new);
#endif
