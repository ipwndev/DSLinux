#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config.h>

#include "global.h"
#include "color.h"	
#include "util.h"	
#include "mountlist.h"
#include "panel.h"	
#include "main.h"	

#include "fsusage.h"
#include "wtools.h"

#include "cmd.h"
#include "tty.h"

#include "selmnt.h"

//extern int selmnt_first_time;

int mc_cd_mountpoint_and_dir = 0;

extern struct mount_entry *mount_list;
extern void listbox_select_by_number (WListbox *l, int n);
extern int _do_panel_cd (WPanel *panel, char *new_dir, enum cd_enum cd);

#ifdef __BEOS__
struct mount_entry *read_filesystem_list (int need_fs_type, int all_fs);

struct mount_entry *read_filesystem_list (int need_fs_type, int all_fs)
{
//	int				i, fd;
//	char				*tp, dev[_POSIX_NAME_MAX], dir[_POSIX_PATH_MAX];

	static struct mount_entry	*me = NULL;

	if (me)
	{
		if (me->me_devname) free(me->me_devname);
		if (me->me_mountdir) free(me->me_mountdir);
		if (me->me_type) free(me->me_type);
		return (NULL);
	}
	else
		me = (struct mount_entry *)malloc(sizeof(struct mount_entry));

	
	me->me_devname = strdup("//");
	me->me_mountdir = strdup("//");
	me->me_type = strdup("unknown");

	return (me);
}
#endif /*__BEOS__*/
//------------------ functions

/* Returns the number of the item selected */
int run_custom_listbox (Listbox *l)
{
    int val;
    
    run_dlg (l->dlg);
    if (l->dlg->ret_value == B_CANCEL)
	val = -1;
    else
	val = l->list->pos;
    destroy_dlg (l->dlg);
    free (l);
    return val;
}

//------------------ create own listbox window

static int custom_listbox_callback (Dlg_head * h, int Par, int Msg)
{
    switch (Msg)
    {
        case DLG_DRAW:
            attrset (COLOR_NORMAL);
            dlg_erase (h);
            draw_box (h, 0, 0, h->lines, h->cols);
            attrset (COLOR_HOT_NORMAL);
    	    if (h->title){
		dlg_move (h, 0, (h->cols-strlen (h->title))/2);
		addstr (h->title);
	    }
            break;
    }
    return 0;
}

Listbox *create_custom_listbox_window (WPanel *panel, int cols, int lines, char *title, char *help)
{
    int x, y, w, h;
    int widget_x, widget_y;
    int xpos, ypos, len;
    Listbox  *listbox = xmalloc (sizeof (Listbox), "create_custom_listbox_window");

//    Adjust sizes

    widget_x = panel->widget.x ;
    widget_y = panel->widget.y ;

    y = widget_y;
    h = lines+2;
    
    if (h <= y || y > LINES - 6)
    {
        h = min(h, y - 1);
        y -= h;
    }
    else
    {
        y++;
        h = min(h, LINES - y);
    }

    x = widget_x;

    if ((w = cols + 4) + x > COLS)
    {
        w = min(w,COLS);
    }

    x = widget_x + (panel->widget.cols - w)/2;
// additional checking
    x = x > 0 ? x : widget_x + 3;
    y = widget_y + (panel->widget.lines - h)/2;

//////////////////////////////
// Create components

    listbox->dlg = create_dlg (y, x, h, w, dialog_colors, custom_listbox_callback, 
    help, "listbox", DLG_NONE);

    x_set_dialog_title (listbox->dlg, title);

    listbox->list = listbox_new (1, 1, w - 2, h - 2, listbox_finish, 0, "li");

    add_widget (listbox->dlg, listbox->list);
    listbox_refresh(listbox->dlg);

    return listbox;
}

//------------------ show mounts list
static int show_mnt (Mountp *mntpoints, WPanel *panel)
{
    Mountp *mz, *mhi;
    int menu_lines, i, maxlen=5, count = 0, u;
    Listbox* listbox;
    mz = mntpoints;

    if (!mz)
        return 0;

    while (mz->prev)           /* goto first */
        mz = mz->prev;

    mhi = mz;

    while (mhi)
    {
        if (
	    (i = strlen (mhi->mpoint) + 
		 (mc_cd_mountpoint_and_dir ? 5:3) + 
		 (mhi->path && mc_cd_mountpoint_and_dir ? strlen(mhi->path):0)) 
		> maxlen)
    	    maxlen = i;
        count++;
        mhi = mhi->next;
    }

// count = number of elements
// maxlen = maximum long of mountpoint menu item name

menu_lines = count;

    /* Create listbox */
    listbox = create_custom_listbox_window ( panel, maxlen, menu_lines," Mountpoints ", "Mountpoints");

    i=0;

    while (mz) {
	static char buffer[4000];
//	sprintf( buffer, "%s (%d/%d)", mz->mpoint, mz->total/1024, mz->avail/1024 ); //for example!
//	sprintf( buffer, "%s", mz->mpoint);

    if (mc_cd_mountpoint_and_dir && mz->path) 
	sprintf( buffer, "%s [%s]", mz->mpoint, mz->path);
    else
	sprintf( buffer, "%s", mz->mpoint);
    
	LISTBOX_APPEND_TEXT( listbox, get_hotkey(i), buffer, NULL );

	mz = mz->next;
	i++;
    }

    /* Select the default entry */
    listbox_select_by_number( listbox->list, 0 );

    i = run_listbox( listbox );

// we must return dirnum
    return (i);
    
}

//------------------ init Mountp
// First - we need helper, that must identify our last dir in this mountpoint
// from dir_history

static char *path_from_history (WPanel *panel, char *mountpoint)
{
    Hist *hd;

        hd = panel->dir_history;

    if ( !hd ) 
        return NULL;

    if ( ! strcmp (mountpoint,PATH_SEP_STR) ) 
        return NULL;

    while (hd->next)
        hd = hd->next;

    do {
    if (strcmp(mountpoint, strdup(hd->text)))
        if (!strncmp(mountpoint, strdup(hd->text), strlen(mountpoint)))
	    return hd->text;

	    hd = hd->prev;
    }  while (hd->prev);


    return NULL;
}


Mountp *init_mountp ( WPanel *panel )
{
    int lockm = 0;
    Mountp *mounts = NULL;
    struct mount_entry *temp = NULL;
    struct fs_usage fs_use;
    
    temp = read_filesystem_list (0,0);

    while (temp) {
	    if (!mounts) {
		get_fs_usage (temp->me_mountdir, &fs_use);

        	mounts = malloc (sizeof (Mountp)+1);
        	memset (mounts, 0, sizeof (Mountp));

//		mounts->type = temp->me_dev;
//		mounts->typename = temp->me_type;
		mounts->mpoint = strdup (temp->me_mountdir);
		mounts->path = mc_cd_mountpoint_and_dir ? path_from_history (panel, temp->me_mountdir) : NULL;
		mounts->name = strdup (temp->me_devname);
		mounts->avail = getuid () ? fs_use.fsu_bavail/2 : fs_use.fsu_bfree/2;
		mounts->total = fs_use.fsu_blocks/2;
                lockm = 1;
	    }

        if (mounts->next)
        {
//	    if(mounts->next->typename) { free (mounts->next->typename); mounts->typename = 0; }
	    if(mounts->next->mpoint) { free (mounts->next->mpoint); mounts->next->mpoint = 0; }
	    if(mounts->next->name) { free (mounts->next->name); mounts->next->name = 0; }
        }
        else
        {
	    mounts->next = malloc (sizeof (Mountp)+1);
    	    memset (mounts->next, 0, sizeof (Mountp));
            mounts->next->prev = mounts;
        }

        if (lockm != 1)
        {
	    get_fs_usage (temp->me_mountdir, &fs_use);

            mounts = mounts->next;
	    mounts->mpoint = strdup (temp->me_mountdir);
	    mounts->name = strdup (temp->me_devname);
	    mounts->path = mc_cd_mountpoint_and_dir ? path_from_history (panel, temp->me_mountdir) : NULL;
	    mounts->avail = getuid () ? fs_use.fsu_bavail/2 : fs_use.fsu_bfree/2;
	    mounts->total = fs_use.fsu_blocks/2;
        }

        lockm = 0;
        temp = temp->me_next;
    }
    free (temp);

    return mounts; /* must be freed! */
}


static int select_mountpoint( WPanel *panel )
{
    int i, inta;
    char *s = 0;
    Mountp *mount_ls = init_mountp(panel);

/* HERE we WILL add some special functions, like fast ftp or network client call */


/*	
Network link - netlink_cmd
FTP link - ftplink_cmd
*/

/* HERE we FINISH add some special functions */
    
    if (mount_ls)
    {
        if (mount_ls->prev || mount_ls->next)
        {
            inta = show_mnt (mount_ls, panel);

    if (inta != -1)
    {
	while (mount_ls->prev)           /* goto first */
    	    mount_ls = mount_ls->prev;

	for (i=0; i<inta; i++)
	    if (mount_ls->next)
		mount_ls = mount_ls->next;
// resulting dirname
	    if (mc_cd_mountpoint_and_dir && mount_ls->path) {
		s = mount_ls->path;
	    } else {
		if (mount_ls->mpoint)
		    s = mount_ls->mpoint;
	    }
    }

            if (s) /* execute CD */
            {
                int r;
                r = _do_panel_cd (panel, s, cd_exact);
                free (s);
            }
	    else /* execute command */ 
	    {
	    }
        }
    }
    free (mount_ls);
    return 1;
}

//------------------ programming interfaces
void select_mnt_left ( void )
{
    switch_to_listing ( 0 );
    select_mountpoint ( left_panel );
}


void select_mnt_right ( void )
{
    switch_to_listing ( 1 );
    select_mountpoint ( right_panel );
}
