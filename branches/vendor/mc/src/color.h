#ifndef __COLOR_H
#define __COLOR_H

void init_colors (void);
void toggle_color_mode (void);
void configure_colors_string (char *color_string);

extern int hascolors;
extern int use_colors;
extern int disable_colors;

extern int attr_pairs [];

#ifdef HAVE_SLANG
#       define MY_COLOR_PAIR(x) COLOR_PAIR(x)
#else
#   define MY_COLOR_PAIR(x) (COLOR_PAIR(x) | attr_pairs [x])
#endif

#define PORT_COLOR(co,bw) (use_colors?co:bw)

#define NORMAL_COLOR       (PORT_COLOR (MY_COLOR_PAIR (1), 0))

#ifdef HAVE_SLANG
#    define DEFAULT_COLOR  (PORT_COLOR (MY_COLOR_PAIR (1),0))
#   else
#     define DEFAULT_COLOR A_NORMAL
#endif

#define SELECTED_COLOR        (PORT_COLOR (MY_COLOR_PAIR (2),A_REVERSE))
#define MARKED_COLOR          (PORT_COLOR (MY_COLOR_PAIR (3),A_BOLD))

#ifdef HAVE_SLANG
#define MARKED_SELECTED_COLOR (PORT_COLOR (MY_COLOR_PAIR (4),(SLtt_Use_Ansi_Colors ? A_BOLD_REVERSE : A_REVERSE | A_BOLD)))
#else
#define MARKED_SELECTED_COLOR (PORT_COLOR (MY_COLOR_PAIR (4),A_REVERSE | A_BOLD))
#endif

#define ERROR_COLOR           (PORT_COLOR (MY_COLOR_PAIR (5),0))
#define MENU_ENTRY_COLOR      (PORT_COLOR (MY_COLOR_PAIR (6),A_REVERSE))
#define REVERSE_COLOR         (PORT_COLOR (MY_COLOR_PAIR (7),A_REVERSE))
#define Q_SELECTED_COLOR      (PORT_COLOR (SELECTED_COLOR, 0))
#define Q_UNSELECTED_COLOR    REVERSE_COLOR

#define COLOR_NORMAL       (PORT_COLOR (MY_COLOR_PAIR (8),A_REVERSE))
#define COLOR_FOCUS        (PORT_COLOR (MY_COLOR_PAIR (9),A_BOLD))
#define COLOR_HOT_NORMAL   (PORT_COLOR (MY_COLOR_PAIR (10),0))
#define COLOR_HOT_FOCUS    (PORT_COLOR (MY_COLOR_PAIR (11),0))
			   
/* Add this to color panel, on BW all pairs are normal */
#define STALLED_COLOR      (PORT_COLOR (MY_COLOR_PAIR (12),0))
			   
#define VIEW_UNDERLINED_COLOR (PORT_COLOR (MY_COLOR_PAIR (12),A_UNDERLINE))
#define MENU_SELECTED_COLOR   (PORT_COLOR (MY_COLOR_PAIR (13),A_BOLD))
#define MENU_HOT_COLOR        (PORT_COLOR (MY_COLOR_PAIR (14),0))
#define MENU_HOTSEL_COLOR     (PORT_COLOR (MY_COLOR_PAIR (15),0))

#define HELP_NORMAL_COLOR  (PORT_COLOR (MY_COLOR_PAIR (16),A_REVERSE))
#define HELP_ITALIC_COLOR  (PORT_COLOR (MY_COLOR_PAIR (17),A_REVERSE))
#define HELP_BOLD_COLOR    (PORT_COLOR (MY_COLOR_PAIR (18),A_REVERSE))
#define HELP_LINK_COLOR    (PORT_COLOR (MY_COLOR_PAIR (19),0))
#define HELP_SLINK_COLOR   (PORT_COLOR (MY_COLOR_PAIR (20),A_BOLD))
			   
/*
 * This should be selectable independently. Default has to be black background
 * foreground does not matter at all.
 */
#define GAUGE_COLOR        (PORT_COLOR (MY_COLOR_PAIR (21),0))
#define INPUT_COLOR        (PORT_COLOR (MY_COLOR_PAIR (22),0))
#define INPUT_COLOR_DEF    (PORT_COLOR (MY_COLOR_PAIR (23),0))

#define DIRECTORY_COLOR    (PORT_COLOR (MY_COLOR_PAIR (24),0))
#define EXECUTABLE_COLOR   (PORT_COLOR (MY_COLOR_PAIR (25),0))
#define LINK_COLOR         (PORT_COLOR (MY_COLOR_PAIR (26),0))
#define DEVICE_COLOR       (PORT_COLOR (MY_COLOR_PAIR (27),0))
#define SPECIAL_COLOR      (PORT_COLOR (MY_COLOR_PAIR (28),0))
#define CORE_COLOR         (PORT_COLOR (MY_COLOR_PAIR (29),0))

/* colors for specific files */
#define HIDDEN_COLOR       (PORT_COLOR (MY_COLOR_PAIR (30),0))
#define TEMP_COLOR         (PORT_COLOR (MY_COLOR_PAIR (31),0))
#define DOC_COLOR          (PORT_COLOR (MY_COLOR_PAIR (32),0))
#define ARCH_COLOR         (PORT_COLOR (MY_COLOR_PAIR (33),0))
#define SRC_COLOR          (PORT_COLOR (MY_COLOR_PAIR (34),0))
#define MEDIA_COLOR        (PORT_COLOR (MY_COLOR_PAIR (35),0))
#define GRAPH_COLOR        (PORT_COLOR (MY_COLOR_PAIR (36),0))
#define DBASE_COLOR        (PORT_COLOR (MY_COLOR_PAIR (37),0))


/* For the default color any unused index may be chosen. */
#define DEFAULT_COLOR_INDEX   38

/*
 * editor colors - only 3 for normal, search->found, and select, respectively
 * Last is defined to view color.
 */
#define EDITOR_NORMAL_COLOR_INDEX    42
#define EDITOR_NORMAL_COLOR          (PORT_COLOR (MY_COLOR_PAIR (42), 0))
#define EDITOR_BOLD_COLOR            (PORT_COLOR (MY_COLOR_PAIR (43),A_BOLD))
#define EDITOR_MARKED_COLOR          (PORT_COLOR (MY_COLOR_PAIR (44),A_REVERSE))
#define EDITOR_UNDERLINED_COLOR      VIEW_UNDERLINED_COLOR


#define STAT_COLOR_INDEX		50
#define STAT_MOUNTPOINT_COLOR		(PORT_COLOR (MY_COLOR_PAIR (50), 0))
#define STAT_AVAIL_COLOR		(PORT_COLOR (MY_COLOR_PAIR (51), A_BOLD))
#define STAT_TOTAL_COLOR		(PORT_COLOR (MY_COLOR_PAIR (52), 0))
#define STAT_SELECTED_COLOR		SELECTED_COLOR


extern int sel_mark_color  [4];
extern int dialog_colors   [4];
extern int alarm_colors    [4];


#ifdef HAVE_SLANG
#   define CTYPE char *
#else
#   define CTYPE int
#endif

void mc_init_pair (int index, CTYPE foreground, CTYPE background);
int try_alloc_color_pair (char *fg, char *bg);
void dealloc_color_pairs (void);

#endif /* __COLOR_H */

