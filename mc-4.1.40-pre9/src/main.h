#ifndef __MAIN_H
#define __MAIN_H

/* Toggling functions */
void toggle_eight_bit (void);
void toggle_clean_exec (void);
void toggle_show_backup (void);
void toggle_show_hidden (void);
void toggle_show_mini_status (void);
void toggle_align_extensions (void);
void toggle_mix_all_files (void);
void toggle_fast_reload (void);
void toggle_confirm_delete (void);

enum {
    RP_NOCLEAR,
    RP_CLEAR
};

#define UP_OPTIMIZE 0
#define UP_RELOAD   1
#define UP_ONLY_CURRENT  2

#define UP_KEEPSEL (char *) -1

extern int quote;
extern volatile int quit;

/* Execute functions: the base and the routines that use it */
void do_execute (const char *shell, const char *command, int internal_command);
#define execute_internal(command,args) do_execute (command, args, 1)

/* Execute functions that use the shell to execute */
void shell_execute (char *command, int flags);
void execute (char *command);

/* This one executes a shell */
void exec_shell ();

void subshell_chdir (char *command);

/* See main.c for details on these variables */
extern int panel_scroll_pages;
extern int mark_moves_down;
extern int auto_menu;
extern int pause_after_run;
extern int auto_save_setup;
extern int use_internal_view;
extern int use_internal_edit;
#ifdef USE_INTERNAL_EDIT
extern int option_word_wrap_line_length;
extern int edit_key_emulation;
extern int option_tab_spacing;
extern int option_fill_tabs_with_spaces;
extern int option_return_does_auto_indent;
extern int option_backspace_through_tabs;
extern int option_fake_half_tabs;
extern int option_save_mode;
extern int option_backup_ext_int;
extern int option_auto_para_formatting;
extern int option_typewriter_wrap;
extern int edit_confirm_save;
extern int option_syntax_highlighting;
extern int option_edit_save_pos;
#endif	/* ! USE_INTERNAL_EDIT */
extern int view_save_line;
extern int fast_reload_w;
extern int clear_before_exec;
extern int mou_auto_repeat;
extern char *other_dir;
extern int mouse_move_pages;
extern int mouse_move_pages_viewer;
#ifdef HAVE_CHARSETS
extern int source_codepage;
extern int display_codepage;
#else /* ! HAVE_CHARSETS */
extern int full_eight_bits;
extern int eight_bit_clean;
#endif /* ! HAVE_CHARSETS */
extern int confirm_view_dir;
extern int fast_refresh;
extern int navigate_with_arrows;
extern int advanced_chfns;
extern int drop_menus;
extern int cd_symlinks;
extern int show_all_if_ambiguous;
extern int slow_terminal;
extern int update_prompt;	/* To comunicate with subshell */
extern int confirm_delete;
extern int confirm_execute;
extern int confirm_exit;
extern int confirm_overwrite;
extern int iconify_on_exec;
extern int force_colors;
extern int boot_current_is_left;
extern int acs_vline;
extern int acs_hline;
extern int use_file_to_check_type;
extern int searching;
extern int vfs_use_limit;
extern int alternate_plus_minus;
extern int only_leading_plus_minus;
extern int ftpfs_directory_timeout;
extern int output_starts_shell;
extern int midnight_shutdown;
extern char search_buffer [256];
extern char cmd_buf [512];
extern int colorize_executables;
extern char *ftmp;
extern char *fdoc;
extern char *farch;
extern char *fsrc;
extern char *fmedia;
extern char *fgraph;
extern char *fdbase;

/* The menu panels */
extern int is_right;		/* If the selected menu was the right */
#define MENU_PANEL (is_right ? right_panel : left_panel)
#define SELECTED_IS_PANEL (get_display_type (is_right ? 1 : 0) == view_listing)

/* Useful macros to avoid too much typing */
#define cpanel get_current_panel()
#define opanel get_other_panel()

typedef void (*key_callback) ();
/* FIXME: We have to leave this type ambiguous, because `key_callback'
   is used both for functions that take an argument and ones that don't.
   That ought to be cleared up. */

/* The keymaps are of this type */
typedef struct {
    int   key_code;
    key_callback fn;
} key_map;

void update_panels (int force_update, char *current_file);
void create_panels (void);
void repaint_screen (void);
void outrefresh_screen (void);
void suspend_cmd (void);
void do_update_prompt (void);

/* clock functions */
void clock_interrupt_handler (int sig);
void set_clock_type (int ct);
int get_clock_type (void);
void clock_init (void);
void clock_cancel (void);
void clock_resume (void);

/* own beep */
void mc_beep (void);


extern char control_file [];
extern char *shell;

/* FIXME: remove this when using slang */
extern const int status_using_ncurses;

void clr_scr (void);
void restore_console (void);

enum cd_enum {
    cd_parse_command,
    cd_exact
};

int do_cd           (char *new_dir, enum cd_enum cd_type);	/* For find.c */
void change_panel   (void);
void init_sigchld   (void);	/* For subshell.c */
int load_prompt     (int fd, void *unused);
void save_cwds_stat (void);
void copy_prog_name (void);
int quiet_quit_cmd  (void);	/* For cmd.c and command.c */
int quit_cmd        (void);


void untouch_bar    (void);
void touch_bar      (void);
void load_hint      (void);

void update_xterm_title (unsigned char *str);
void update_xterm_title_path (void);

void print_vfs_message(char *msg, ...);

extern char *prompt;
extern char *mc_home;

int maybe_cd (int char_code, int move_up_dir);
void do_possible_cd (char *dir);

#ifdef WANT_WIDGETS
extern WButtonBar *the_bar;
extern WLabel     *the_prompt;
extern WLabel     *the_hint;
extern Dlg_head   *midnight_dlg;
extern WLabel      *process_status;
#include "menu.h"
extern WMenu      *the_menubar;

extern Dlg_head *midnight_dlg;

void init_menu (void);
void done_menu (void);
void menubar_arrange(WMenu* menubar);

/* Back hack to define the following routines only if the client code
 * has included panel.h
 */
#ifdef __PANEL_H
void directory_history_add   (WPanel *panel, char *s);
int  do_panel_cd             (WPanel *panel, char *new_dir, enum cd_enum cd_type);
void update_one_panel_widget (WPanel *panel, int force_update, char *current_file);
#endif

#endif

void edition_pre_exec (void);
void edition_post_exec (void);

#ifdef OS2_NT
#    define MC_BASE ""
#else
#    define MC_BASE "/.mc/"
#endif
#endif
