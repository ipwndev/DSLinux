/*
 * Copyright 1998 by Albert Cahalan; all rights resered.         
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version  
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */

#ifndef PROCPS_PS_H
#define PROCPS_PS_H

#include "../proc/readproc.h"
#include <asm/page.h>  /* looks safe for glibc, we need PAGE_SIZE */

#if 0
#define trace(args...) printf(## args)
#else
#define trace(args...)
#endif


/***************** GENERAL DEFINE ********************/


/* selection list */
#define SEL_RUID 1
#define SEL_EUID 2
#define SEL_SUID 3
#define SEL_FUID 4
#define SEL_RGID 5
#define SEL_EGID 6
#define SEL_SGID 7
#define SEL_FGID 8
#define SEL_PGRP 9
#define SEL_PID  10
#define SEL_TTY  11
#define SEL_SESS 12
#define SEL_COMM 13

/* Since an enum could be smashed by a #define, it would be bad. */
#define U98 0 /* Unix98 standard */    /* This must be 0 */
#define XXX 1 /* Common extension */
#define DEC 2 /* Digital Unix */
#define AIX 3 /* AIX */
#define SCO 4 /* SCO */
#define LNX 5 /* Linux original :-) */
#define BSD 6 /* FreeBSD and OpenBSD */
#define SUN 7 /* SunOS 5 (Solaris) */
#define HPU 8 /* HP-UX */
#define SGI 9 /* Irix */

/*
 * Must not overflow the output buffer:
 *    32 pages for env+cmd
 *    8 kB pages on the Alpha
 *    5 chars for "\001 "
 *    plus some slack for other stuff
 * That is about 1.3 MB on the Alpha
 *
 * This isn't good enough for setuid. If anyone cares, mmap() over the
 * last page with something unwriteable.
 */

/* maximum escape expansion is 6, for &quot; */
#define ESC_STRETCH 6
/* output buffer size */
/* IDIOTS */
/* #define OUTBUF_SIZE (32*PAGE_SIZE*ESC_STRETCH + 8*PAGE_SIZE) */
#define OUTBUF_SIZE 32768
/* spaces used to right-justify things */
#define SPACE_AMOUNT (int)(PAGE_SIZE)

/******************* PS DEFINE *******************/

/* personality control flags */
#define PER_BROKEN_o      0x0001
#define PER_BSD_h         0x0002
#define PER_BSD_m         0x0004
#define PER_CUMUL_MARKED  0x0008
#define PER_FORCE_BSD     0x0010
#define PER_GOOD_o        0x0020
#define PER_OLD_m         0x0040
#define PER_SUN_MUTATE_a  0x0080
#define PER_ZAP_ADDR      0x0100
#define PER_SANE_USER     0x0200
#define PER_IRIX_l        0x0400

/* Simple selections by bit mask
 * Warning: these bit positions are hard-coded into the table in
 * table_accept(), so do not change them.
 */
#define SS_B_x 0x01
#define SS_B_g 0x02
#define SS_U_d 0x04
#define SS_U_a 0x08
#define SS_B_a 0x10

/* predefined format flags such as:  -l -f l u s -j */
#define FF_Uf 0x0001 /* -f */
#define FF_Uj 0x0002 /* -j */
#define FF_Ul 0x0004 /* -l */
#define FF_Bj 0x0008 /* j */
#define FF_Bl 0x0010 /* l */
#define FF_Bs 0x0020 /* s */
#define FF_Bu 0x0040 /* u */
#define FF_Bv 0x0080 /* v */
#define FF_LX 0x0100 /* X */
#define FF_Lm 0x0200 /* m */  /* overloaded: threads, sort, format */

/* predefined format modifier flags such as:  -l -f l u s -j */
#define FM_c 0x0001 /* -c */
#define FM_j 0x0002 /* -j */  /* only set when !sysv_j_format */
#define FM_y 0x0004 /* -y */
#define FM_L 0x0008 /* -L */
#define FM_P 0x0010 /* -P */
#define FM_M 0x0020 /* -M */

/* sorting & formatting */
/* U,B,G is Unix,BSD,Gnu and then there is the option itself */
#define SF_U_O      1
#define SF_U_o      2
#define SF_B_O      3
#define SF_B_o      4
#define SF_B_m      5       /* overloaded: threads, sort, format */
#define SF_G_sort   6
#define SF_G_format 7

/* headers */
#define HEAD_SINGLE 0  /* default, must be 0 */
#define HEAD_NONE   1
#define HEAD_MULTI  2

/* Justification control for flags field. */
#define JUST_MASK 0x0f
     /* AIXHACK   0 */
#define USER      1  /* left if text, right if numeric */
#define LEFT      2
#define RIGHT     3
#define UNLIMITED 4
#define WCHAN     5  /* left if text, right if numeric */
#define SIGNAL    6  /* right in 9, or 16 if screen_cols>107 */

#define CUMUL     16  /* mark cumulative (Summed) headers with 'C' */


/********************** GENERAL TYPEDEF *******************/

/* Other fields that might be useful:
 *
 * char *name;     user-defined column name (format specification)
 * int reverse;    sorting in reverse (sort specification)
 *
 * name in place of u
 * reverse in place of n
 */

typedef union sel_union {
  pid_t pid;
  uid_t uid;
  gid_t gid;
  dev_t tty;
  char  cmd[8];  /* this is _not_ \0 terminated */
} sel_union;

typedef struct selection_node {
  struct selection_node *next;
  sel_union *u;  /* used if selection type has a list of values */
  int n;         /* used if selection type has a list of values */
  int typecode;
} selection_node;

typedef struct sort_node {
  struct sort_node *next;
  int (*sr)(const proc_t* P, const proc_t* Q); /* sort function */
  int reverse;   /* can sort backwards */
  int typecode;
} sort_node;

typedef struct format_node {
  struct format_node *next;
  char *name;                             /* user can override default name */
  int (*pr)(void);                         /* print function */
/*  int (* const sr)(const proc_t* P, const proc_t* Q); */ /* sort function */
  int width;
  int pad;
  int vendor;                             /* Vendor that invented this */
  int flags;
  int typecode;
} format_node;

typedef struct format_struct {
  const char *spec; /* format specifier */
  const char *head; /* default header in the POSIX locale */
  int (* const pr)(void); /* print function */
  int (* const sr)(const proc_t* P, const proc_t* Q); /* sort function */
  const int width;
  const int pad;    /* could be second width */
  const int vendor; /* Where does this come from? */
  const int flags;
} format_struct;

/* though ps-specific, needed by general file */
typedef struct macro_struct {
  const char *spec; /* format specifier */
  const char *head; /* default header in the POSIX locale */
} macro_struct;

/**************** PS TYPEDEF ***********************/

typedef struct aix_struct {
  const int   desc; /* 1-character format code */
  const char *spec; /* format specifier */
  const char *head; /* default header in the POSIX locale */
} aix_struct;

typedef struct shortsort_struct {
  const int   desc; /* 1-character format code */
  const char *spec; /* format specifier */
} shortsort_struct;

/* Save these options for later: -o o -O O --format --sort */
typedef struct sf_node {
  struct sf_node *next;  /* next arg */
  format_node *f_cooked;  /* convert each arg alone, then merge */
  sort_node   *s_cooked;  /* convert each arg alone, then merge */
  char *sf;
  int sf_code;
} sf_node;


/*********************** GENERAL GLOBALS *************************/

/* escape.c */
extern int escape_strlist(char *dst, const char **src, size_t n);
extern int escape_str(char *dst, const char *src, size_t n);
extern int octal_escape_str(char *dst, const char *src, size_t n);
extern int simple_escape_str(char *dst, const char *src, size_t n);
extern int html_escape_str(char *dst, const char *src, size_t n);

/********************* UNDECIDED GLOBALS **************/

/* compare.c */
extern void reset_sort_options(void);
extern int mult_lvl_cmp(void* a, void* b);
extern int node_mult_lvl_cmp(void* a, void* b);
extern void dump_keys(void);
extern const char *parse_sort_opt(const char* opt);
extern const char *parse_long_sort(const char* opt);

/* output.c */
extern void show_one_proc(proc_t* p);
extern void print_format_specifiers(void);
extern const aix_struct *search_aix_array(const int findme);
extern const shortsort_struct *search_shortsort_array(const int findme);
extern const format_struct *search_format_array(const char *findme);
extern const macro_struct *search_macro_array(const char *findme);
extern void init_output(void);

/* global.c */
extern void reset_global(void);

/* global.c */
extern int             all_processes;
extern char           *bsd_j_format;
extern char           *bsd_l_format;
extern char           *bsd_s_format;
extern char           *bsd_u_format;
extern char           *bsd_v_format;
extern int             bsd_c_option;
extern int             bsd_e_option;
extern uid_t           cached_euid;
extern pid_t           cached_pid;
extern dev_t           cached_tty;
extern char            forest_prefix[4 * 32*1024 + 100];
extern int             forest_type;
extern unsigned        format_flags;     /* -l -f l u s -j... */
extern format_node    *format_list; /* digested formatting options */
extern unsigned        format_modifiers; /* -c -j -y -P -L... */
extern int             header_gap;
extern int             header_type; /* none, single, multi... */
extern int             include_dead_children;
extern int             lines_to_next_header;
extern int             max_line_width;
extern const char     *namelist_file;
extern int             negate_selection;
extern unsigned        personality;
extern int             prefer_bsd;
extern int             running_only;
extern int             screen_cols;
extern int             screen_rows;
extern selection_node *selection_list;
extern unsigned        simple_select;
extern sort_node      *sort_list;
extern char           *sysv_f_format;
extern char           *sysv_fl_format;
extern char           *sysv_j_format;
extern char           *sysv_l_format;
extern int             unix_f_option;
extern int             use_aix_codes;
extern int             user_is_number;
extern int             wchan_is_number;

/************************* PS GLOBALS *********************/

/* sortformat.c */
extern int defer_sf_option(const char *arg, int source);
extern const char *process_sf_options(int localbroken);
extern void reset_sortformat(void);

/* select.c */
extern int want_this_proc(proc_t *buf);
extern const char *select_bits_setup(void);

/* help.c */
extern const char *usage_message;
extern const char *help_message;

/* global.c */
extern void self_info(void);

/* parser.c */
extern int arg_parse(int argc, char *argv[]);

/* output.c */
extern unsigned long seconds_since_boot;

#endif
