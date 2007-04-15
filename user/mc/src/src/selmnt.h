#ifndef __SELMNT_H__
#define __SELMNT_H__

extern int mc_cd_mountpoint_and_dir;

typedef struct mountp_list { 
    int special;
    char *handler;
    
    struct mountp_list *prev;
    struct mountp_list *next;
//    int type;
//    char *typename;

    char *name; // name of entry device, [/dev/]'hda8'
    char *path; // local path after mountpoint dir, [/mnt/hda/8]'/mp3' (plan to get last from history by regexp)
    char *mpoint; // mountpoint, '/mnt/hda/8'
    int total; // total size of FS
    int avail;  // free size of FS
} Mountp;

Mountp *init_mountp ( WPanel *panel );
void select_mnt_left ( void );
void select_mnt_right ( void );

#endif /* __SELMNT_H__ */