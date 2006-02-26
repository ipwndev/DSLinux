/* retawq/main.c - main routine, user interaction
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

#include "stuff.h"
#include "init.h"
#include "resource.h"
#include "parser.h"

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if CONFIG_TG == TG_X
#include <X11/keysym.h>
#elif CONFIG_TG == TG_GTK
#include <gdk/gdkkeysyms.h>
#endif

declare_local_i18n_buffer


/** Strings */

static char strbuf[STRBUF_SIZE], strbuf2[STRBUF_SIZE], strbuf3[STRBUF_SIZE];
#if CONFIG_DEBUG
static char debugstrbuf[STRBUF_SIZE];
#endif

static const char strPercsDash[] = "%s - ", strDotSlash[] = "./",
  strSlashSlash[] = "//";
#define strDot (strDoubleDot + 1) /* ugly :-) */

static const char strBracedNewWindow[] = N_("(new window)"),
  strBack[] = N_("Back"), strForward[] = N_("Forward"),
  strReload[] = N_("Reload"), strEnforcedReload[] = N_("Enforced Reload"),
  strSearch[] = N_("Search"), strOpenInNewWindow[] = N_("Open in New Window"),
  strSource[] = N_("source"), strSourceCode[] = N_("Source Code"),
  strSave[] = N_("Save"), strEnableThisElement[] = N_("Enable this Element"),
  strContext[] = N_("Context"), strShowElementInfo[] = N_("Show Element Info"),
  strOpenLinkInNewWindow[] = N_("Open Link in New Window"),
  strSelectionEmpty[] = N_("This selection list is empty!"),
  strAddBookmark[] = N_("Add Bookmark"), strSaveAs[] = N_("Save As..."),
  strSaveLinkAs[] = N_("Save Link As..."), strUriColonSpace[] = N_("URL: "),
  strSubmitThisForm[] = N_("Submit this Form");
#if OPTION_COOKIES
static const char strCookies[] = N_("(cookies) ");
#endif
#if TGC_IS_GRAPHICS || CONFIG_CUSTOM_CONN
static const char strUcDownload[] = N_("Download");
#endif
#if CONFIG_SESSIONS || CONFIG_CONSOLE
static const char strDone[] = N_("Done");
#endif

static const char* const strAek[MAX_AEK + 1] =
{ strUnknown, strLink, N_("text field"), N_("password field"),
  N_(strCheckbox), N_("radio button"), N_("submit button"), N_("reset button"),
  N_("file field"), N_("text-area field"), N_("selection"), N_(strButton),
  N_(strImage), N_("hidden field")
};

static const char strUcClose[] = N_("Close"), strUcQuit[] = N_("Quit");

#if CONFIG_KEYMAPS
static const char strMouseFlip[] = "mouse-flip", strMouseOff[] = "mouse-off",
  strMouseOn[] = "mouse-on";
#endif

#if CONFIG_FTP
static const char strAnonymous[] = "anonymous";
#endif


/** Keys'n'curses */

#if TGC_IS_GRAPHICS

#if CONFIG_TG == TG_X

#define KEY_ESCAPE (XK_Escape)
#define KEY_CANCEL (XK_Cancel)
#define KEY_LEFT (XK_Left)
#define KEY_RIGHT (XK_Right)
#define KEY_END (XK_End)
#define KEY_NPAGE (XK_Page_Down)
#define KEY_PPAGE (XK_Page_Up)
#define KEY_DOWN (XK_Down)
#define KEY_UP (XK_Up)
#define KEY_HOME (XK_Home)
#define KEY_ENTER (XK_Return)
#define KEY_BACKSPACE (XK_BackSpace)
#define KEY_DC (XK_Delete)
#define KEY_IC (XK_Insert)

#elif CONFIG_TG == TG_GTK

FAIL!
#define KEY_ESCAPE (GDK_Escape)
#define KEY_CANCEL (GDK_Cancel)
#define KEY_LEFT (GDK_Left)
#define KEY_RIGHT (GDK_Right)
#define KEY_END (GDK_End)
#define KEY_NPAGE (GDK_Page_Down)
#define KEY_PPAGE (GDK_Page_Up)
#define KEY_DOWN (GDK_Down)
#define KEY_UP (GDK_Up)
#define KEY_HOME (GDK_Home)
#define KEY_ENTER (GDK_Return)
#define KEY_BACKSPACE (GDK_BackSpace)
#define KEY_DC (GDK_Delete)
#define KEY_IC (GDK_Insert)

#endif

/* stubs'n'dummies - for now... */
#define COLS (80)
#define LINES (25)
#define A_NORMAL (0)
#define A_BOLD (0)
#define A_UNDERLINE (0)
#define A_REVERSE (0)
#define OK (0)
#define ERR (-1)
typedef unsigned long chtype;
typedef chtype attr_t;
typedef struct { short _curx, _cury; } tTextualWindowContents;
static my_inline int move(int y __cunused, int x __cunused) { return(OK); }
static my_inline int clrtoeol(void) { return(OK); }
static my_inline int addnstr(const char* s __cunused, int n __cunused)
{ return(OK); }
static my_inline int mvaddnstr(int y, int x, const char* s, int n)
{ move(y, x); addnstr(s, n); return(OK); }
static my_inline int addch(const chtype c __cunused) { return(OK); }
static my_inline int mvaddch(int y, int x, const chtype c)
{ move(y, x); addch(c); return(OK); }
static my_inline int addchnstr(const chtype* s __cunused, int n __cunused)
{ return(OK); }
static my_inline int COLOR_PAIR(int x __cunused) { return(0); }
static my_inline int color_set(short x __cunused, void* d __cunused)
{ return(OK); }
/* static my_inline int attron(attr_t a __cunused) { return(OK); }
   static my_inline int attroff(attr_t a __cunused) { return(OK); }
   static one_caller int refresh(void) { return(OK); } */
static my_inline chtype inch(void) { return(0); }

#elif TGC_IS_CURSES

#if CONFIG_TG != TG_XCURSES
#undef KEY_ESCAPE
#define KEY_ESCAPE '\033'
#endif

#ifdef ACS_HLINE
#define __MY_HLINE (ACS_HLINE)
#else
#define __MY_HLINE ('-')
#endif

#ifdef ACS_VLINE
#define __MY_VLINE (ACS_VLINE)
#else
#define __MY_VLINE ('|')
#endif

#ifdef ACS_ULCORNER
#define __MY_UL (ACS_ULCORNER)
#else
#define __MY_UL ('+')
#endif

#ifdef ACS_URCORNER
#define __MY_UR (ACS_URCORNER)
#else
#define __MY_UR ('+')
#endif

#ifdef ACS_LLCORNER
#define __MY_LL (ACS_LLCORNER)
#else
#define __MY_LL ('+')
#endif

#ifdef ACS_LRCORNER
#define __MY_LR (ACS_LRCORNER)
#else
#define __MY_LR ('+')
#endif

#ifdef ACS_LTEE
#define __MY_LTEE (ACS_LTEE)
#else
#define __MY_LTEE ('+')
#endif

#ifdef ACS_RTEE
#define __MY_RTEE (ACS_RTEE)
#else
#define __MY_RTEE ('+')
#endif

#define VMIDDLE (MAX(((LINES - 2) / 2), 1))

static tBoolean __must_reset_cursor = falsE;
static __my_inline void must_reset_cursor(void) { __must_reset_cursor = truE; }

#endif

#ifndef KEY_TAB
#define KEY_TAB '\t'
#endif

#if OPTION_CED == 0
#define is_bad_uchar(c) ( ((c) < 32) || ((c) >= 127) )
#else
#define is_bad_uchar(c) ( ((c) < 32) || ((c) == 127) )
#endif


/** Keymaps */

/* begin-autogenerated */
my_enum1 enum
{ pccUnknown = 0, pccDocumentBottom = 1, pccDocumentEnforceHtml = 2,
  pccDocumentEnforceSource = 3, pccDocumentInfo = 4, pccDocumentReload = 5,
  pccDocumentReloadEnforced = 6, pccDocumentSave = 7, pccDocumentSearch = 8,
  pccDocumentSearchBackward = 9, pccDocumentSearchNext = 10,
  pccDocumentSearchPrevious = 11, pccDocumentTop = 12, pccDownload = 13,
  pccDownloadFromElement = 14, pccDump = 15, pccElementEnable = 16,
  pccElementInfo = 17, pccElementNext = 18, pccElementOpen = 19,
  pccElementOpenSplit = 20, pccElementPrevious = 21, pccExecextShell = 22,
  pccExecextShellFlip = 23, pccFormReset = 24, pccFormSubmit = 25,
  pccGoBookmarks = 26, pccGoHome = 27, pccGoSearch = 28, pccGoUri = 29,
  pccGoUriPreset = 30, pccJump = 31, pccJumpPreset = 32, pccLineDown = 33,
  pccLineUp = 34, pccLocalFileDirOpen = 35, pccMenuContextual = 36,
  pccMenuUriHistory = 37, pccMenuWindowlist = 38, pccMouseFlip = 39,
  pccMouseOff = 40, pccMouseOn = 41, pccPageDown = 42, pccPageUp = 43,
  pccQuit = 44, pccScreenSplit = 45, pccScreenSwitch = 46,
  pccScreenUnsplit = 47, pccScrollBarsFlip = 48, pccSessionResume = 49,
  pccSessionSave = 50, pccStop = 51, pccViewBack = 52, pccViewForward = 53,
  pccWindowClose = 54, pccWindowNew = 55, pccWindowNewFromDocument = 56,
  pccWindowNewFromElement = 57, pccWindowNext = 58, pccWindowPrevious = 59
} my_enum2(unsigned char) tProgramCommandCode;

typedef struct
{ tKey key;
  tProgramCommandCode pcc;
} tKeymapCommandEntry;

static const tKeymapCommandEntry keymap_command_defaultkeys[] =
{ { '!', pccExecextShell },
  { '&', pccExecextShellFlip },
  { '.', pccStop },
  { '/', pccDocumentSearch },
  { '1', pccScreenUnsplit },
  { '2', pccScreenSplit },
  { '?', pccDocumentSearchBackward },
  { 'C', pccWindowClose },
  { 'D', pccDownloadFromElement },
  { 'E', pccElementEnable },
  { 'G', pccGoUriPreset },
  { 'H', pccDocumentEnforceHtml },
  { 'I', pccDocumentInfo },
  { 'J', pccJumpPreset },
  { 'L', pccLineUp },
  { 'M', pccSessionResume },
  { 'N', pccWindowNewFromDocument },
  { 'O', pccWindowNewFromElement },
  { 'Q', pccQuit },
  { 'R', pccDocumentReloadEnforced },
  { 'S', pccSessionSave },
  { 'W', pccWindowPrevious },
  { 'Y', pccMouseFlip },
  { '\\', pccDocumentEnforceSource },
  { 'b', pccGoBookmarks },
  { 4, pccDownload },
  { 15, pccElementOpenSplit },
  { 23, pccMenuWindowlist },
  { KEY_DOWN, pccElementNext },
  { KEY_LEFT, pccViewBack },
  { KEY_RIGHT, pccViewForward },
  { KEY_UP, pccElementPrevious },
  { 'd', pccDump },
  { KEY_DC, pccLineDown },
  { 'e', pccGoSearch },
  { KEY_END, pccDocumentBottom },
  { KEY_ENTER, pccElementOpen },
  { 'g', pccGoUri },
  { 'h', pccGoHome },
  { KEY_HOME, pccDocumentTop },
  { 'i', pccElementInfo },
  { KEY_IC, pccLineUp },
  { 'j', pccJump },
  { 'l', pccLineDown },
  { 'm', pccMenuContextual },
  { 'n', pccWindowNew },
  { 'o', pccElementOpen },
  { KEY_NPAGE, pccPageDown },
  { KEY_PPAGE, pccPageUp },
  { 'r', pccDocumentReload },
  { 's', pccDocumentSave },
  { ' ', pccPageDown },
  { KEY_TAB, pccScreenSwitch },
  { 'u', pccMenuUriHistory },
  { 'w', pccWindowNext }
};

#if CONFIG_KEYMAPS

static const struct
{ const char* str; /* (sorted in strcmp() order) */
  tProgramCommandCode pcc; /* REMOVEME? */
} keymap_command_str2pcc[] =
{ { "document-bottom", pccDocumentBottom },
  { "document-enforce-html", pccDocumentEnforceHtml },
  { "document-enforce-source", pccDocumentEnforceSource },
  { "document-info", pccDocumentInfo },
  { "document-reload", pccDocumentReload },
  { "document-reload-enforced", pccDocumentReloadEnforced },
  { "document-save", pccDocumentSave },
  { "document-search", pccDocumentSearch },
  { "document-search-backward", pccDocumentSearchBackward },
  { "document-search-next", pccDocumentSearchNext },
  { "document-search-previous", pccDocumentSearchPrevious },
  { "document-top", pccDocumentTop },
  { "download", pccDownload },
  { "download-from-element", pccDownloadFromElement },
  { "dump", pccDump },
  { "element-enable", pccElementEnable },
  { "element-info", pccElementInfo },
  { "element-next", pccElementNext },
  { "element-open", pccElementOpen },
  { "element-open-split", pccElementOpenSplit },
  { "element-previous", pccElementPrevious },
  { "execext-shell", pccExecextShell },
  { "execext-shell-flip", pccExecextShellFlip },
  { "form-reset", pccFormReset },
  { "form-submit", pccFormSubmit },
  { "go-bookmarks", pccGoBookmarks },
  { "go-home", pccGoHome },
  { "go-search", pccGoSearch },
  { "go-url", pccGoUri },
  { "go-url-preset", pccGoUriPreset },
  { "jump", pccJump },
  { "jump-preset", pccJumpPreset },
  { "line-down", pccLineDown },
  { "line-up", pccLineUp },
  { "local-file-dir-open", pccLocalFileDirOpen },
  { "menu-contextual", pccMenuContextual },
  { "menu-url-history", pccMenuUriHistory },
  { "menu-windowlist", pccMenuWindowlist },
  { strMouseFlip, pccMouseFlip },
  { strMouseOff, pccMouseOff },
  { strMouseOn, pccMouseOn },
  { "page-down", pccPageDown },
  { "page-up", pccPageUp },
  { strQuit, pccQuit },
  { "screen-split", pccScreenSplit },
  { "screen-switch", pccScreenSwitch },
  { "screen-unsplit", pccScreenUnsplit },
  { "scroll-bars-flip", pccScrollBarsFlip },
  { "session-resume", pccSessionResume },
  { "session-save", pccSessionSave },
  { "stop", pccStop },
  { "view-back", pccViewBack },
  { "view-forward", pccViewForward },
  { "window-close", pccWindowClose },
  { "window-new", pccWindowNew },
  { "window-new-from-document", pccWindowNewFromDocument },
  { "window-new-from-element", pccWindowNewFromElement },
  { "window-next", pccWindowNext },
  { "window-previous", pccWindowPrevious }
};

#endif

my_enum1 enum
{ liacUnknown = 0, liacAreaSwitch = 1, liacCancel = 2, liacMouseFlip = 3,
  liacMouseOff = 4, liacMouseOn = 5, liacPass2user = 6, liacToEnd = 7,
  liacToLeft = 8, liacToRight = 9, liacToStart = 10
} my_enum2(unsigned char) tLineInputActionCode;

typedef struct
{ tKey key;
  tLineInputActionCode liac;
} tKeymapLineinputEntry;

static const tKeymapLineinputEntry keymap_lineinput_defaultkeys[] =
{ { 1, liacAreaSwitch },
  { 21, liacPass2user },
  { KEY_DOWN, liacToEnd },
  { KEY_LEFT, liacToLeft },
  { KEY_RIGHT, liacToRight },
  { KEY_UP, liacToStart },
  { KEY_END, liacToEnd },
  { KEY_CANCEL, liacCancel },
  { KEY_HOME, liacToStart },
  { KEY_NPAGE, liacToEnd },
  { KEY_PPAGE, liacToStart }
};

#if CONFIG_KEYMAPS

static const struct
{ const char* str; /* (sorted in strcmp() order) */
  tLineInputActionCode liac; /* REMOVEME? */
} keymap_lineinput_str2liac[] =
{ { "area-switch", liacAreaSwitch },
  { "cancel", liacCancel },
  { strMouseFlip, liacMouseFlip },
  { strMouseOff, liacMouseOff },
  { strMouseOn, liacMouseOn },
  { "pass2user", liacPass2user },
  { "to-end", liacToEnd },
  { "to-left", liacToLeft },
  { "to-right", liacToRight },
  { "to-start", liacToStart }
};

#endif
/* end-autogenerated */

static const_after_init tKeymapCommandEntry* keymap_command_keys = NULL;
static const_after_init size_t keymap_command_keys_num = 0,
  keymap_command_keys_maxnum = 0;

static int __init keymap_command_sorter(const void* _a, const void* _b)
{ const tKeymapCommandEntry *a = (const tKeymapCommandEntry*) _a,
    *b = (const tKeymapCommandEntry*) _b;
  tKey ak = a->key, bk = b->key;
  return(my_numcmp(ak, bk));
}

static tBoolean __init keymap_command_key_do_register(tKey key,
  tProgramCommandCode pcc)
/* returns whether it worked */
{ size_t count;
  for (count = 0; count < keymap_command_keys_num; count++) /* IMPROVEME? */
  { if (keymap_command_keys[count].key == key) return(falsE); } /* redefined */

  if (keymap_command_keys_num >= keymap_command_keys_maxnum)
  { keymap_command_keys_maxnum += 32;
    keymap_command_keys = (tKeymapCommandEntry*)
      memory_reallocate(keymap_command_keys, keymap_command_keys_maxnum *
      sizeof(tKeymapCommandEntry), mapKeymap);
  }
  keymap_command_keys[keymap_command_keys_num].key = key;
  keymap_command_keys[keymap_command_keys_num].pcc = pcc;
  keymap_command_keys_num++;
  return(truE);
}

static const_after_init tKeymapLineinputEntry* keymap_lineinput_keys = NULL;
static const_after_init size_t keymap_lineinput_keys_num = 0,
  keymap_lineinput_keys_maxnum = 0;

static int __init keymap_lineinput_sorter(const void* _a, const void* _b)
{ const tKeymapLineinputEntry *a = (const tKeymapLineinputEntry*) _a,
    *b = (const tKeymapLineinputEntry*) _b;
  tKey ak = a->key, bk = b->key;
  return(my_numcmp(ak, bk));
}

static tBoolean __init keymap_lineinput_key_do_register(tKey key,
  tLineInputActionCode liac)
/* returns whether it worked */
{ size_t count;
  for (count = 0; count < keymap_lineinput_keys_num; count++) /* IMPROVEME? */
  { if (keymap_lineinput_keys[count].key == key) return(falsE); } /*redefined*/

  if (keymap_lineinput_keys_num >= keymap_lineinput_keys_maxnum)
  { keymap_lineinput_keys_maxnum += 16;
    keymap_lineinput_keys = (tKeymapLineinputEntry*)
      memory_reallocate(keymap_lineinput_keys, keymap_lineinput_keys_maxnum *
      sizeof(tKeymapLineinputEntry), mapKeymap);
  }
  keymap_lineinput_keys[keymap_lineinput_keys_num].key = key;
  keymap_lineinput_keys[keymap_lineinput_keys_num].liac = liac;
  keymap_lineinput_keys_num++;
  return(truE);
}

#if CONFIG_KEYMAPS

static tKey __init keystr2key(const char* keystr)
{ tKey key = '\0';
  unsigned char ch = *keystr;
  if ( (ch != '\0') && (keystr[1] == '\0') ) key = ch; /* most likely case? */
  else if (!strcmp(keystr, "delete")) key = KEY_DC;
  else if (!strcmp(keystr, "end")) key = KEY_END;
  /* else if (!strcmp(keystr, "enter")) key = KEY_ENTER; */
  else if (!strcmp(keystr, "insert")) key = KEY_IC;
  else if (!strcmp(keystr, "home")) key = KEY_HOME;
  else if (!strcmp(keystr, "page-down")) key = KEY_NPAGE;
  else if (!strcmp(keystr, "page-up")) key = KEY_PPAGE;
  else if (!strcmp(keystr, "escape")) key = KEY_CANCEL;
  else if (!strcmp(keystr, "space")) key = ' ';
  else if (!strcmp(keystr, "tab")) key = '\t';
  else if (!strncmp(keystr, "ctrl-", 5))
  { char cc = keystr[5]; /* "control character" */
    if ( (my_islower(cc)) && (keystr[6] == '\0') ) key = cc - 'a' + 1;
  }
  else if (!strncmp(keystr, "cursor-", 7))
  { const char* tmp = keystr + 7;
    if (!strcmp(tmp, "down")) key = KEY_DOWN;
    else if (!strcmp(tmp, "left")) key = KEY_LEFT;
    else if (!strcmp(tmp, "right")) key = KEY_RIGHT;
    else if (!strcmp(tmp, "up")) key = KEY_UP;
  }
#ifdef KEY_F
  else if (!strncmp(keystr, "fn-", 3))
  { const char* tmp = keystr + 3;
    if (my_isdigit(*tmp))
    { int num;
      my_atoi(tmp, &num, &tmp, 99);
      if ( (num >= 0) && (num <= 63) && (*tmp == '\0') ) key = KEY_F(num);
    }
  }
#endif
  return(key);
}

static one_caller tMbsIndex keymap_cmdstr_lookup(const char* str)
{ my_binary_search(0, ARRAY_ELEMNUM(keymap_command_str2pcc) - 1,
    strcmp(str, keymap_command_str2pcc[idx].str), return(idx))
}

unsigned char __init keymap_command_key_register(const char* keystr,
  const char* cmdstr)
/* return value: 0=fine, 1=bad key identifier; 2=repeatedly defined key; 3=bad
   command; IMPROVEME: binary search for <keystr>? */
{ tKey key = keystr2key(keystr);
  tMbsIndex idx;
  tProgramCommandCode pcc;

  if (key == '\0') return(1);

  /* interpret the command */
  idx = keymap_cmdstr_lookup(cmdstr);
  if (idx < 0) return(3);
  pcc = keymap_command_str2pcc[idx].pcc;

  /* register */
  if (!keymap_command_key_do_register(key, pcc)) return(2);
  return(0);
}

static one_caller tMbsIndex keymap_listr_lookup(const char* str)
{ my_binary_search(0, ARRAY_ELEMNUM(keymap_lineinput_str2liac) - 1,
    strcmp(str, keymap_lineinput_str2liac[idx].str), return(idx))
}

unsigned char __init keymap_lineinput_key_register(const char* keystr,
  const char* listr)
/* return value: 0=fine, 1=bad key identifier; 2=repeatedly defined key; 3=bad
   command; IMPROVEME: binary search for <keystr>? */
{ tKey key = keystr2key(keystr);
  tMbsIndex idx;
  tLineInputActionCode liac;

  if (key == '\0') return(1);

  /* interpret the action */
  idx = keymap_listr_lookup(listr);
  if (idx < 0) return(3);
  liac = keymap_lineinput_str2liac[idx].liac;

  /* register */
  if (!keymap_lineinput_key_do_register(key, liac)) return(2);
  return(0);
}

#endif /* #if CONFIG_KEYMAPS */

#if TGC_IS_GRAPHICS

#define is_khm_command (truE) /* only one key-handling mode available... */

#else

enum
{ khmCommand = 0, khmLineInput = 1
#if CONFIG_MENUS
  , khmMenu = 2
#endif
};
typedef unsigned char tKeyHandlingMode;
static tKeyHandlingMode key_handling_mode = khmCommand;
#define is_khm_command (key_handling_mode == khmCommand)

#define is_bottom_occupied (key_handling_mode == khmLineInput)

#endif


/** Helper functions */

static const char* my_getcwd(void)
{
#if !HAVE_GETCWD
  return(strSlash); /* CHECKME! */
#else
  static const char* cwd = NULL;
  if (cwd == NULL) /* not yet calculated */
  { char buf[2060]; /* 1<<11 + 42*epsilon */
    if ( (getcwd(buf, sizeof(buf) - 5) != buf) || (*buf == '\0') )
      cwd = strSlash;
    else
    { const size_t len = strlen(buf);
      if (buf[len - 1] != chDirsep) { buf[len] = chDirsep; buf[len+1] = '\0'; }
      cwd = my_strdup(buf);
    }
  }
  return(cwd);
#endif
}

#if TGC_IS_GRAPHICS
static my_inline __sallocator char* __callocator
  my_strdup_ucfirst(const char* str)
/* duplicates a string, changing the first character to uppercase */
{ char *retval = my_strdup(str), ch = *retval;
  *retval = my_toupper(ch);
  return(retval);
}
#endif

#if OPTION_TLS
static tBoolean tls_errtext(const tResource* resource, /*@out@*/ char* dest)
{ tBoolean retval;
  tTlsError te = resource->tls_error;
  if (!is_tls_error_expressive(te)) retval = falsE;
  else
  { sprint_safe(dest, _("TLS error - %s"), _(strTlsError[te])); retval=truE; }
  return(retval);
}
#endif

#if 1 /* !CONFIG_JAVASCRIPT */
#define javascript_handle_event(kind, _ae) do { } while (0)
#else
static void javascript_handle_event(tJavascriptEventKind kind,
  tActiveElementNumber _ae)
{ tWindowView* view = current_window->current_view;
  const tJavascriptEventHandler* eh = view->request->resource->aebase[_ae].eh;
  while (eh != NULL)
  { if (eh->kind == kind)
    { const tJavascriptCode* code = eh->code;
      if (code != NULL) javascript_execute(view, _ae, code);
      break;
    }
    eh = eh->next;
  }
}
#endif


/** Remaining work mechanism */

struct tRemainingWork;
typedef tBoolean (*tRemainingWorkCallback)(const struct tRemainingWork*);

typedef struct tRemainingWork
{ struct tRemainingWork* next;
  tRemainingWorkCallback callback;
  void *data1, *data2, *data3; /* private data for the callback handler */
} tRemainingWork;

static tRemainingWork *rw_head = NULL, *rw_tail = NULL;

static void remaining_work_store(tRemainingWork* rw)
{ if (rw_head == NULL) rw_head = rw;
  if (rw_tail != NULL) rw_tail->next = rw;
  rw_tail = rw;
}

static tRemainingWork* remaining_work_create(tRemainingWorkCallback callback)
{ tRemainingWork* retval = memory_allocate(sizeof(tRemainingWork), mapOther);
  retval->callback = callback; remaining_work_store(retval); return(retval);
}

static void remaining_work_do(void)
{ tRemainingWork* rw = rw_head;
  if (rw == NULL) return; /* the most likely case */
  rw_head = rw_tail = NULL; /* detach */
  while (rw != NULL)
  { tRemainingWork* next = rw->next;
    if ((rw->callback)(rw)) remaining_work_store(rw); /* keep it for retry */
    else memory_deallocate(rw);
    rw = next;
  }
}


/** Generic graphics stuff */

#if TGC_IS_GRAPHICS

static const char strCancel[] = N_("Cancel");
static const_after_init char *istrYesUc, *istrNoUc;

#if CONFIG_TG == TG_X

/* some minor, not-yet-complete abstraction for the future... */
typedef void tGraphicsWidget;
typedef void tGraphicsWindow;
typedef void tGraphicsLowlevelWindow;
typedef GC tGraphicsContext;
typedef XEvent tGraphicsEvent;
typedef XFontStruct tGraphicsFont;

Display* xws_display = NULL;

#else

/* some minor, not-yet-complete abstraction for the future... */
typedef GtkWidget tGraphicsWidget;
typedef GtkWindow tGraphicsWindow;
typedef GdkWindow tGraphicsLowlevelWindow;
typedef GdkGC tGraphicsContext;
typedef GdkEvent tGraphicsEvent;
typedef GdkFont tGraphicsFont;

static const char strGtkDestroy[] = "destroy", strGtkDelete[] = "delete_event",
  strGtkConfigure[] = "configure_event", strGtkClicked[] = "clicked",
  strGtkButton[] = "button_press_event", strGtkActivate[] = "activate",
  strGtkKey[] = "key_press_event";

#endif

static const tGraphicsFont* default_font;

#endif /* #if TGC_IS_GRAPHICS */


/** Line input I */

my_enum1 enum
{ liekFail = 0, liekCancel = 1, liekKey = 2
} my_enum2(unsigned char) tLineInputEventKind;

typedef void (*tLineInputCallback)(void*, tLineInputEventKind, tKey key);

my_enum1 enum
{ liafNone = 0, liafFocusable = 0x01, liafEditable = 0x02,
  liafDisguising = 0x04, liafAllowEmptyText = 0x08
} my_enum2(unsigned char) tLineInputAreaFlags;

typedef struct
{ char* text;
  short len, maxlen, pos, first; /* CHECKME: size_t? */
  short row, colmin, colcurr, colmax, usable;
  tLineInputAreaFlags flags;
} tLineInputArea;
typedef unsigned char tLineInputAreaIndex;

my_enum1 enum
{ prrfNone = 0, prrfRedrawAll = 0x01, prrfRedrawOne = 0x02, prrfSource = 0x04,
  prrfHtml = 0x08, prrfReload = 0x10, prrfEnforcedReload = 0x20,
  prrfIsHttpRedirection = 0x40, prrfUseSfbuf = 0x80, prrfPost = 0x100,
  prrfWantUriAnchor = 0x200, prrfUpsfp4 = 0x400
} my_enum2(unsigned short) tPrrFlags; /* for prepare_resource_request() */
#define prrfIsRedirection (prrfIsHttpRedirection)

static tSinkingData sinking_data;

static my_inline void sinking_data_reset(void)
{ my_memclr_var(sinking_data);
}

static void sinking_data_shift(/*@out@*/ tSinkingData** _s)
{ tSinkingData* s = *_s = __memory_allocate(sizeof(tSinkingData),
    mapSinkingData);
  my_memcpy(s, &sinking_data, sizeof(tSinkingData));
  sinking_data_reset();
}

static my_inline void sinking_data_mcleanup(void)
{ sinking_data_cleanup(&sinking_data); sinking_data_reset();
}

static struct
{ tLineInputArea area[2];
#if TGC_IS_GRAPHICS
  tWindow* window;
  GtkEntry* entry;
#endif
  tLineInputCallback callback;
  void* callback_data;
  tPrrFlags prrf; /* rarely needed */
  tLineInputAreaIndex curr, num_areas;
} lid; /* aka "line input data" */

#define lid_area(what) (lid.area[lid.curr].what)

my_enum1 enum
{ eligGoToUri = 0, eligSaveAs = 1
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  , eligDownloadUri = 2, eligDownloadFilename = 3
#endif
#if CONFIG_EXTRA & EXTRA_DUMP
  , eligDumpFilename = 4
#endif
  , eligDocumentSearch = 5, eligDocumentSearchBackward = 6
#if CONFIG_USER_QUERY
  , eligUsername = 7, eligPassword = 8
#endif
#if !TGC_IS_GRAPHICS
  , eligFormText = 9, eligFormPassword = 10, eligFormFilename = 11
#endif
#if CONFIG_SESSIONS
  , eligSessionSave = 12, eligSessionResume = 13
#endif
#if CONFIG_JUMPS
  , eligJump = 14
#endif
#if OPTION_EXECEXT & EXECEXT_SHELL
  , eligExecextShell = 15
#endif
} my_enum2(unsigned char) tEditableLineInputGoal;

#if CONFIG_USER_QUERY
#define is_disguising_elig(elig) \
  ( ((elig) == eligFormPassword) || ((elig) == eligPassword) )
#else
#define is_disguising_elig(elig) ((elig) == eligFormPassword)
#endif

#define is_emptyok_elig(elig) ( ((elig) == eligFormText) || \
  ((elig) == eligFormPassword) || ((elig) == eligFormFilename) )

my_enum1 enum
{ cgQuit = 0, cgClose = 1, cgOverwriteFile = 2,
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  cgOverwriteDownload = 3,
#endif
#if CONFIG_EXTRA & EXTRA_DUMP
  cgOverwriteDump = 4,
#endif
#if CONFIG_SESSIONS
  cgOverwriteSession = 5,
#endif
  cgSubmit = 6, cgReset = 7, cgRepost = 8, cgHtml = 9, cgEnable = 10
#if CONFIG_FTP && OPTION_TLS
  , cgFtpsDataclear = 11
#endif
} my_enum2(unsigned char) tConfirmationGoal;

static const char* khmli_filename = NULL;
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
static const char* khmli_download_uri = NULL;
#endif

#if CONFIG_EXTRA & EXTRA_DUMP

my_enum1 enum
{ dsfNone = 0, dsfMustSource = 0x01, dsfMustHtml = 0x02, dsfBeepWhenDone = 0x04
} my_enum2(unsigned char) tDumpStyleFlags;

my_enum1 enum
{ dscNone = 0, dscDocument = 0x01, dscLinklist = 0x02, dscImagelist = 0x04
} my_enum2(unsigned char) tDumpStyleContents;

typedef struct
{ tCoordinate width;
  unsigned char kb_hashmarks;
  tDumpStyleFlags flags;
} tDumpStyle;

#if 0
static tDumpStyle khmli_dump_style;
#endif

#endif /* #if CONFIG_EXTRA & EXTRA_DUMP */

#if CONFIG_MENUS & MENUS_UHIST
#define URI_HISTORY_LEN (20)
static const char* uri_history[URI_HISTORY_LEN];
static unsigned short uri_history_index = 0;
#endif

my_enum1 enum
{ wrtRedraw = 0, wrtRedrawRecursive = 1, wrtSearch = 2, wrtSearchBackward = 3,
  wrtToEnd = 4
} my_enum2(unsigned char) tWindowRedrawingTask;

/* some prototypes */
static void show_message(const char*, tBoolean);
static void __line_input_estart(tEditableLineInputGoal, const char*,
  const char*, tUserQuery*);
#define line_input_estart(a, b, c, d, e, f, g) __line_input_estart(a, b, c, d)


/** URI parsing */

static const struct
{ const char* scheme; /* (sorted in strcmp() order) */
  tResourceProtocol rp;
} scheme2rp[] =
{ { strAbout, rpAbout },
#if CONFIG_DEBUG
  { strCvs, rpCvs },
#endif
  { strFile, rpLocal },
  { strFinger, __rpFinger },
  { strFtp, __rpFtp },
  { strFtps, __rpFtps },
  { strGopher, __rpGopher },
  { strHttp, rpHttp },
  { strHttps, __rpHttps },
#if CONFIG_DEBUG
  { strInfo, rpInfo },
#endif
  { strJavascript, __rpJavascript },
  { strLocal, rpLocal },
  { strLocalCgi, __rpLocalCgi },
  { strMailto, __rpMailto },
  { strNews, __rpNntp },
  { strNntp, __rpNntp },
  { strPop, __rpPop },
  { strPops, __rpPops }
};

static one_caller tMbsIndex do_lookup_rp(const char* scheme)
{ my_binary_search(0, ARRAY_ELEMNUM(scheme2rp) - 1,
    streqcase3(scheme, scheme2rp[idx].scheme), return(idx))
  /* case-insensitivity: RFC3986 (6.2.2.1) and RFC2616 (3.2.3) -- or was that
     casein-sensitivity? :-) */
}

static one_caller tResourceProtocol lookup_rp(const char* scheme)
{ const tMbsIndex idx = do_lookup_rp(scheme);
  return ( (idx >= 0) ? scheme2rp[idx].rp : rpUnknown );
}

static char* uri_parse_part(const char* uri, unsigned char part)
{ static const char separator[] = ":/?#";
  const char* const sep = separator + part;
  char uch, sch; /* URI character, separator character */
  /* The following code does very much the same as strpbrk(), but that function
     isn't portable. */
  while ( (uch = *uri) != '\0' )
  { const char* temp = sep;
    while ( (sch = *temp++) != '\0' )
    { if (uch == sch) return(unconstify(uri)); }
    uri++;
  }
  return(NULL);
}

static void setup_authority(/*const*/ char** _uri,
  /*@out@*/ /*const*/ char** _authority)
{ /*const*/ char *uri = *_uri, *pos = uri_parse_part(uri, 1), *authority;
  if (pos != NULL) { authority = my_strndup(uri, pos - uri); uri = pos; }
  else { authority = my_strdup(uri); TO_EOS(uri); }
  *_uri = uri; *_authority = authority;
}

static __sallocator char* __callocator finalize_path(const char* _path)
/* transforms a path to a standard representation by removing "superfluous"
   components */
{ char ch, *path = my_strdup(_path), *temp, *dest, **component;
  size_t num, maxnum, count, orig_len = strlen(path);
  tBoolean found_tilde = falsE, is_abs = cond2boolean(*path == chDirsep),
    is_dir = cond2boolean( (orig_len > 0) && (path[orig_len - 1] == chDirsep)),
    is_first;

  /* split the path into components */
  temp = path; component = NULL; num = maxnum = 0;
  splitloop:
  while (*temp == chDirsep) temp++;
  if (*temp != '\0') /* got a component */
  { if (num >= maxnum)
    { maxnum += 16;
      component = memory_reallocate(component, maxnum * sizeof(char*),
        mapOther);
    }
    component[num++] = temp;
    while ( (*temp != chDirsep) && (*temp != '\0') ) temp++;
    if (*temp != '\0') { *temp++ = '\0'; goto splitloop; } /* look for more */
  }

  /* remove superfluous components */
#define rc(x) *(component[(x)]) = '\0'
  for (count = 0; count < num; count++)
  { const char* str = component[count];
    if (!strcmp(str, strDot)) rc(count);
    else if (!found_tilde)
    { if (!strcmp(str, strDoubleDot))
      { rc(count);
        if (count > 0) /* try to remove "the preceding" component */
        { size_t cnt = count - 1;
          while (1)
          { if (*(component[cnt]) != '\0') { rc(cnt); break; } /* done */
            if (cnt == 0) break; /* no removable preceding component */
            cnt--;
          }
        }
      }
      else if (!strcmp(str, "~"))
      { /* e.g. in FTP it should be possible to do "ftp://foo.org/~/../blah",
           which can only be interpreted by the server, not by us */
        found_tilde = truE;
      }
    }
  }
#undef rc

  /* build the new path */
  dest = path; is_first = truE;
  if (is_abs) *dest++ = chDirsep;
  for (count = 0; count < num; count++)
  { const char* src = component[count];
    if (*src == '\0') continue; /* removed */
    if (is_first) is_first = falsE;
    else *dest++ = chDirsep;
    while ( (ch = *src++) != '\0' ) *dest++ = ch;
  }
  if (is_dir)
  { if ( (dest <= path) || (*(dest - 1) != chDirsep) ) *dest++ = chDirsep; }
  *dest = '\0';

  __dealloc(component);
  return(path);
}

#define __cleanup_path \
  if (must_dealloc_path) { memory_deallocate(path); must_dealloc_path = falsE;}

static __my_inline void uri_set_error(tUriData* uri, tResourceError re)
{ if (uri->re == reFine) uri->re = re;
}

static __my_inline tUriData* uri_allocate(void)
{ return((tUriData*) memory_allocate(sizeof(tUriData), mapUriData));
}

static tUriData* uri_parse(const char* _uri, const tUriData* const
  orig_referrer, /*@out@*/ const char** _fragment, const char* extra_query,
  unsigned char special_flags)
/* Please note the number and position of underscore characters in the function
   name: it's not "u_rip_....()" although maybe it should be... */
/* RFC3986 says: ^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?
   Simple in theory, but in practice it's much more complicated. Just to
   mention a few examples:
   - Colons can appear in four places in a URI: right after the scheme string,
     as a separator in "username:password", as a separator within a numerical
     IPv6 hostname, and as a portnumber indicator.
   - There aren't only absolute URIs to be handled, but also relative and
     abbreviated URIs.
   To distinguish all these correctly, we have to do much more than a trivial
   pattern matching. Let's thank all specification developers for such
   bogosities. (I've looked at several implementations, and none of them seems
   to handle all cases the right way. And the endless comments in this function
   might also be an indication of bad URI design concepts, similar to the HTTP
   cookie mess...)
*/
{ static const char strWwwDot[] = "www.", strFtpDot[] = "ftp.",
    strNewsDot[] = "news.", strGopherDot[] = "gopher.";
  tUriData* retval = uri_allocate();
  const tUriData* referrer = orig_referrer;
  tResourceProtocol rp, wouldbe_rp;
  char *uristart, *uri, *pos;
  char *scheme, *authority, *hostname, *port, *path, *query, *fragment;
  const char *username, *password;
  char *complete_query, *spfbuf_uri;
  tPortnumber portnumber;
  tResourceError uri_error;
  tBoolean must_dealloc_path, must_dealloc_complete_query, ua, iph, appfrag;

  username = password = scheme = authority = hostname = port = path = query =
    fragment = NULL;
  portnumber = 0; must_dealloc_path = must_dealloc_complete_query = falsE;
  rp = rpUnknown;

  /* Prepare the URI */

  { char* dest = uristart = uri = __memory_allocate(strlen(_uri)+1, mapString);
    const char* src = _uri;
    const unsigned char* test = (const unsigned char*) _uri;
    unsigned char testch;
    while ( (testch = *test++) != '\0' )
    { const char urich = *src++; /* "My first name is Robert." :-) */
      *dest++ = (is_control_char(testch) ? '_' : urich);
    }
    *dest = '\0';
  }

  if (orig_referrer != NULL)
  { const char ch = *uri;
    if ( (ch == '\0') || (ch == '#') ) /* RFC3986, 4.4 */
    { const char* ref_uri = orig_referrer->uri;
      if ( (ref_uri != NULL) && (*ref_uri != '\0') )
      { char* new_uri;
        my_spf(NULL, 0, &new_uri, strPercsPercs, ref_uri, uri);
        memory_deallocate(uristart); uristart = uri = my_spf_use(new_uri);
      }
    }
  }

  /* Look what's explicitly given */

  pos = uri_parse_part(uri, 0);
  if ( (pos != NULL) && (*pos == ':') && (pos > uri) )
  { /* RFC3986 (3.1) says: scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
       Test whether it actually _is_ a scheme name or rather an abbreviated URI
       like "foo.org:42" or a numerical IPv6 hostname beginning like
       "[42:43...]". To avoid problems, we don't allow any dots in a scheme
       name - RFC3986 allows dots, but nobody seems to use them. This way, only
       abbreviated URIs like "localhost:42" could still cause problems, and we
       check explicitly for "localhost". These are ugly special-case hacks, but
       all more elegant algorithms seem to be less correct. */
    const char* temp;
    char ch;
    size_t len;
    if ( (pos[1] == '/') && (pos[2] == '/') )
    { /* Found the sequence "://"; accept anything in front of this as a scheme
         string, no matter what it actually is. This mechanism allows users to
         override the below plausibility tests, so they can use even the
         strangest schemes if they want to. (Non-standard URI schemes will be
         configurable in a later version, and "all power to the user" is often
         a good idea anyway.)
      */
      goto do_scheme;
    }
    temp = uri; ch = *temp++;
    if (!my_isalpha(ch)) goto no_scheme;
    while (temp < pos)
    { ch = *temp++;
      if ( (!my_isalnum(ch)) && (ch != '+') && (ch != '-') ) goto no_scheme;
    }
    len = pos - uri;
    if ( (len == 9) && (strneqcase(uri, strLocalhost, 9)) ) goto no_scheme;
    do_scheme: scheme = uri; *pos++ = '\0'; uri = pos; /* scheme given */
    no_scheme: {}
  }

  if ( (uri[0] == '/') && (uri[1] == '/') ) /* authority given */
  { uri += 2; setup_authority(&uri, &authority);
  }

  path = uri; /* a path is always given (but may be empty, of course) */

  pos = uri_parse_part(uri, 2);
  if ( (pos != NULL) && (*pos == '?') ) /* query given */
  { *pos++ = '\0'; query = uri = pos; }

  pos = uri_parse_part(uri, 3);
  if ( (pos != NULL) && (*pos == '#') ) /* fragment given */
  { *pos++ = '\0'; fragment = uri = pos; }

  /* Interpret the whole stuff */

  /* calculate protocol and possibly authority: */
  if (scheme != NULL) rp = lookup_rp(scheme);
  else if (orig_referrer != NULL) rp = orig_referrer->rp;
  else if (authority == NULL)
  { /* The URI can only be an abbreviated thing like "www.foo.org..." or a
       local path. */
    if ( (*path == chDirsep) || ( (path[0] == '.') && (path[1] == chDirsep) )
         || ( (path[0] == '~') && (path[1] == chDirsep) ) )
    { /* local scheme, no authority necessary */
      set_local: rp = rpLocal;
    }
    else if (strneqcase(path, strWwwDot, 4))
    { set_http: rp = rpHttp;
      handle_abbr_authority: setup_authority(&path, &authority);
    }
    else if (strneqcase(path, strFtpDot, 4))
    { set_ftp: rp = __rpFtp; goto handle_abbr_authority; }
    else if ( (strneqcase(path, strNewsDot, 5)) /* CHECKME! ||
              ( (strneqcase(path, strNews, 4)) &&
                (uri_parse_part(path, 1) == path + 4) ) */ )
    { rp = __rpNntp; goto handle_abbr_authority; }
    else if (strneqcase(path, strGopherDot, 7))
    { rp = __rpGopher; goto handle_abbr_authority; }
    else /* last resorts */
    { struct stat statbuf;
#if CONFIG_CUSTOM_CONN
      if (special_flags & 1) goto set_ftp;
#endif
      /* might be a local regular file or directory */
      if (my_stat(path, &statbuf) == 0)
      { const mode_t mode = statbuf.st_mode;
        if (S_ISREG(mode) || S_ISDIR(mode)) goto set_local;
      }
      /* nothing else worked, so assume it's an HTTP resource */
      goto set_http;
    }
  }

  /* split the authority: */
  if (authority != NULL)
  { pos = my_strchr(authority, '@');
    if (pos == NULL) hostname = authority;
    else
    { *pos++ = '\0'; hostname = pos; username = authority; /* username given */
      pos = my_strchr(username, ':');
      if (pos != NULL) { *pos++ = '\0'; password = pos; } /* password given */
    }

    pos = hostname; /* default start position for the below my_strchr() call */
    if (*hostname == '[')
    { /*looks like the beginning of a numeric IPv6 address notation (RFC2732)*/
      char* temp;
      if ( (hostname[1] != '\0') &&
           ( ( temp = my_strchr(hostname + 2, ']') ) != NULL ) )
      { /* yup, it is */
        *temp++ = '\0'; pos = temp; hostname++;
        retval->udf |= udfTryIpv6;
      }
      /* "else": it isn't; we assume that users know what they're doing; we
         don't know :-) so we pass the stuff unchanged */
    }
    pos = my_strchr(pos, ':');
    if (pos != NULL) { *pos++ = '\0'; port = pos; } /* port given */
    if (*hostname == '\0') hostname = NULL;
    else
    { if ( (rp == rpUnknown) && (scheme == NULL) && (orig_referrer == NULL) )
      { /* no explicit scheme, no referrer, but an explicit hostname; seems
           that the user entered something like "//www.foo.org" manually; very
           rare, but valid and thus should be supported */
        if (strneqcase(hostname, strWwwDot, 4)) rp = rpHttp;
        else if (strneqcase(hostname, strFtpDot, 4)) rp = __rpFtp;
        else if ( (strneqcase(hostname, strNewsDot, 5)) ||
                  (streqcase(hostname, strNews)) )
          rp = __rpNntp;
      }
      hostname = my_strdup_tolower(hostname);
        /* case-insensitivity: RFC3986 (6.2.2.1) and RFC2616 (3.2.3) */
    }
  }

#if OPTION_EXECEXT & EXECEXT_SHELL
  if ( (rp == rpUnknown) && (special_flags & 4) ) rp = rpExecextShell;
#endif
  wouldbe_rp = rp; rp = rp_data[wouldbe_rp].final_rp;
  ua = cond2boolean(rp_data[wouldbe_rp].flags & rpfUsesAuthority);
  iph = cond2boolean(rp_data[wouldbe_rp].flags & rpfIsPathHierarchical);

  /* calculate the portnumber: */
  if (ua)
  { if ( (port == NULL) || (*port == '\0') )
      portnumber = rp2portnumber(wouldbe_rp);
    else
    { int _portnumber;
      my_atoi(port, &_portnumber, NULL, 99999);
      if ( (_portnumber <= 0) || (_portnumber > 65535) )
      { uri_set_error(retval, rePortnumber); portnumber = 0; }
      else
      { portnumber = (tPortnumber) htons((unsigned short) _portnumber);
        retval->udf |= udfGotExplicitPortnumber;
      }
    }
#if CONFIG_CUSTOM_CONN
    if ( (special_flags & 1) && (!(retval->udf & udfGotExplicitPortnumber)) )
    { if (rp == rpFtp) portnumber = htons(21);
#if OPTION_TLS
      else if (rp == rpFtps) portnumber = htons(990);
#endif
    }
#endif
  }

#if CONFIG_FTP
  if (is_ftplike(wouldbe_rp))
  { /* check for ";type=X" ending of FTP path */
    const char *l_username, *l_password;
    size_t len = strlen(path);
    if ( (len >= 7) && (strneqcase(path + len - 7, ";type=", 6)) )
    { char typech = *(path + len - 1);
      if (typech == 'a') retval->udf |= udfFtpTypeA;
      else if (typech == 'd') retval->udf |= udfFtpTypeD;
      else if (typech == 'i') retval->udf |= udfFtpTypeI;
      else goto no_ftp_type;
      *(path + len - 7) = '\0';
      no_ftp_type: {}
    }
    /* handle username and password */
    l_username = strAnonymous; l_password = "guest";
    if ( (wouldbe_rp == rpFtp) && ((username == NULL) || (password == NULL)) )
    { const tConfigLogin* l = ( (hostname != NULL) ? config.ftp_login : NULL );
      tPortnumber portnumber1 = portnumber;
      while (l != NULL)
      { tPortnumber portnumber2 = l->hosts_portnumber;
        if ( ( (portnumber2 == 0) || (portnumber2 == portnumber1) ) &&
             (my_pattern_matcher(l->hosts_pattern, hostname)) ) /* found */
        { l_username = l->user; l_password = l->password; break; }
        l = l->next;
      }
    }
    if (username == NULL) username = l_username;
    if ( (password == NULL) && (username != NULL) && (l_username != NULL) &&
         (!strcmp(username, l_username)) )
      password = l_password;
  }
#endif

  /* Complete the data */

  if ( (query != NULL) && (*query == '\0') ) query = NULL;
  if ( (fragment != NULL) && (*fragment == '\0') ) fragment = NULL;

  if ( (query != NULL) && (extra_query != NULL) )
  { my_spf(NULL, 0, &complete_query, "%s&%s", query, extra_query);
    must_dealloc_complete_query = truE;
  }
  else if (query != NULL) complete_query = query;
  else if (extra_query != NULL) complete_query = unconstify(extra_query);
  else complete_query = NULL;

  if ( (wouldbe_rp == rpLocal) || (wouldbe_rp == __rpLocalCgi) )
  { if ( (path[0] == '~') && (path[1] == chDirsep) )
    { my_spf(NULL, 0, &path, strPercsPercs, get_homepath(), path + 2);
      must_dealloc_path = truE;
    }
  }

  if ( (referrer != NULL) && (rp != referrer->rp) )
    referrer = NULL; /* can't use it */

  if (referrer != NULL)
  { const char* refpath;
    /* duplicate any missing information by taking it from the referrer: */
    if (ua)
    { if (hostname == NULL) hostname = my_strdup(referrer->hostname);
      else if (strcmp(hostname, referrer->hostname))
      { referrer = NULL; goto done_with_referrer; } /* can't use it */
    }
    if ( (iph) && (*path != chDirsep) &&
         ( (refpath = referrer->path) != NULL ) )
    { const char* temp = my_strrchr(refpath, chDirsep);
      if (temp == NULL) /* "can't happen" */
      { __cleanup_path path = unconstify(strSlash);
      }
      else
      { size_t refpathsize = temp - refpath + 1, pathsize = strlen(path) + 1,
          size = refpathsize + pathsize;
        char* dest = __memory_allocate(size, mapString);
        my_memcpy(dest, refpath, refpathsize);
        my_memcpy(dest + refpathsize, path, pathsize);
        __cleanup_path path = dest; must_dealloc_path = truE;
      }
    }
  }
  done_with_referrer: {}

  if ( (*path != chDirsep) && (is_locallike(wouldbe_rp)) )
  { char *spfbuf, *pathtemp = path;
    const char* prefix = my_getcwd();
    while (!strncmp(pathtemp, strDotSlash, 2)) pathtemp += 2; /* REMOVEME! */
    my_spf(NULL, 0, &spfbuf, strPercsPercs, prefix, pathtemp);
    __cleanup_path path = my_spf_use(spfbuf); must_dealloc_path = truE;
  }

  if ( (iph) && (*path == '\0') )
  { __cleanup_path path = unconstify(strSlash); }

  if (iph)
  { char* finpath = finalize_path(path);
    __cleanup_path path = finpath; must_dealloc_path = truE;
  }

  switch (rp)
  { case rpAbout:
      if (*path == '\0') { __cleanup_path path = unconstify(strRetawq); }
      break;
#if OPTION_NEWS
    case rpNntp:
      if (hostname == NULL)
      { const char* n = config.news_server;
        if (n != NULL) hostname = my_strdup(n);
      }
      break;
#endif
  }

  /* Compose the final URI and store everything */

  if (ua) sprint_safe(strbuf, strPercd, ntohs(portnumber));
  appfrag = cond2boolean( (special_flags & 2) && (fragment != NULL) );

  my_spf(NULL, 0, &spfbuf_uri
    ,
/*A*/ "%s:" /* scheme */
/*B*/ "%s%s%s%s" /* double slash, hostname, colon, portnumber (if ua) */
/*C*/ "%s%s" /* slash (conditional), path */
/*D*/ "%s%s" /* question mark, query (if any query) */
/*E*/ "%s%s" /* hashmark, fragment (if appfrag) */
    ,
/*A*/ ( ( (scheme != NULL) && (!is_rp_nice(wouldbe_rp)) ) ? scheme :
/*A*/   rp_data[wouldbe_rp].scheme ),
/*B*/ (ua ? strSlashSlash : strEmpty),
/*B*/ ( (ua && (hostname != NULL)) ? hostname : strEmpty ),
/*B*/ (ua ? ":" : strEmpty), (ua ? strbuf : strEmpty), /* portnumber */
/*C*/ ( (ua && (*path != chDirsep)) ? strDirsep : strEmpty ), path,
/*D*/ ( (complete_query != NULL) ? strQm : strEmpty ),
/*D*/ ( (complete_query != NULL) ? complete_query : strEmpty ),
/*E*/ (appfrag ? strHm : strEmpty), (appfrag ? fragment : strEmpty)
    );

  retval->rp = rp;
  retval->uri = my_spf_use(spfbuf_uri); retval->hostname = hostname;
  if (must_dealloc_path) { retval->path = path; must_dealloc_path = falsE; }
  else retval->path = my_strdup(path);
  if (complete_query != NULL)
  { if (must_dealloc_complete_query)
    { retval->query = complete_query; must_dealloc_complete_query = falsE; }
    else retval->query = my_strdup(complete_query);
  }
  if (username != NULL) retval->username = my_strdup(username);
  if (password != NULL) retval->password = my_strdup(password);
  if (ua) retval->portnumber = portnumber;

  if (_fragment != NULL) *_fragment = (fragment ? my_strdup(fragment) : NULL);

  if (rp == rpUnknown) uri_error = reProtocol;
  else if (rp == rpDisabled) uri_error = reProtDisabled;
#if CONFIG_JAVASCRIPT
  else if (rp == rpJavascript) uri_error = reProtDisabled;
#endif
  else if (ua && (hostname == NULL)) uri_error = reHostname;
  else goto no_uri_error;
  uri_set_error(retval, uri_error);
  no_uri_error: {}

  memory_deallocate(uristart); __dealloc(authority);
  if (must_dealloc_path) memory_deallocate(path);
  if (must_dealloc_complete_query) memory_deallocate(complete_query);
  return(retval);
}

#undef __cleanup_path


/** Windowing */

my_enum1 enum
{ wkUnknown = 0, wkBrowser = 1
#if DO_WK_INFO
  , wkInfo = 2
#endif
#if DO_WK_CUSTOM_CONN
  , wkCustomConn = 3
#endif
#if DO_WK_EDITOR
  , wkEditor = 4
#endif
#if DO_WK_FILEMGR
  , wkFilemgr = 5
#endif
#if DO_WK_MAILMGR
  , wkMailmgr = 6
#endif
} my_enum2(unsigned char) tWindowKind;

struct tWindowSpec;

typedef struct tWindow /* a "virtual window" */
{ struct tWindow *prev, *next; /* list of all windows */
  const struct tWindowSpec* spec; /* specification (operations etc.) */
  void* wksd; /* "window-kind-specific data", "tWkFooData*" */
} tWindow;

my_enum1 enum
{ wsfNone = 0, wsfWantKeyFirst = 0x01
} my_enum2(unsigned char) tWindowSpecFlags;

struct tBrowserDocument;

typedef struct tWindowSpec
{ void (*create)(tWindow*);
  void (*remove)(tWindow*);
  void (*redraw)(const tWindow*);
  tBoolean (*is_precious)(const tWindow*);
  tBoolean (*handle_key)(tWindow*, tKey);
  tBoolean (*handle_pcc)(tWindow*, tProgramCommandCode);
  struct tBrowserDocument* (*find_currdoc)(const tWindow*);
  const char* (*get_info)(tWindow*, /*@out@*/ tBoolean* /*is_error*/);
  const char* (*get_menu_entry)(tWindow*);
#if CONFIG_MENUS & MENUS_CONTEXT
  void (*setup_cm)(tWindow*, short*, short*, tActiveElementNumber);
#define WSPEC_CM(func) (func),
#else
#define WSPEC_CM(func) /* nothing */
#endif
#if CONFIG_SESSIONS
  void (*save_session)(tWindow*, int /*fd*/);
#define WSPEC_SESSION(func) (func),
#else
#define WSPEC_SESSION(func) /* nothing */
#endif
#if CONFIG_DO_TEXTMODEMOUSE
  void (*handle_mouse)(tWindow*, tCoordinate, tCoordinate, int);
#define WSPEC_MOUSE(func) (func),
#else
#define WSPEC_MOUSE(func) /* nothing */
#endif
  char ui_char, session_char; /* for user interface and for session files */
  tWindowKind kind;
  tWindowSpecFlags flags;
} tWindowSpec; /* "window (kind) specification" */

static /*@null@*/ tWindow *windowlisthead = NULL, *windowlisttail = NULL;

#if TGC_IS_GRAPHICS
static unsigned int window_counter = 0;
#else
static /*@relnull@*/ tWindow* visible_window_x[2] = { NULL, NULL };
typedef unsigned char tVisibleWindowIndex; /* 0..1 */
static tVisibleWindowIndex current_window_index_x = 0;
#define current_window_x (visible_window_x[current_window_index_x])
#endif /* #if TGC_IS_GRAPHICS */

my_enum1 enum
{ wvfNone = 0, wvfScreenFull = 0x01, wvfDontDrawContents = 0x02,
  wvfScrollingUp = 0x04, wvfHandledRedirection = 0x08, wvfPost = 0x10
} my_enum2(unsigned char) tWindowViewFlags; /* CHECKME: rename! */
/* wvfScreenFull implies two things: 1. the user may scroll down; 2. we needn't
   redraw the document when the resource handler got more content for it. */

my_enum1 enum
{ bddmAutodetect = 0, bddmSource = 1, bddmHtml = 2 /* , bddmHex = 3 */
} my_enum2(unsigned char) tBrowserDocumentDisplayMode;

typedef struct
{ /* tBoolean (*handle_pcc)(struct tBrowserDocument*, tProgramCommandCode); */
  tBoolean (*find_coords)(const struct tBrowserDocument*, /*@out@*/ short*,
    /*@out@*/ short*, /*@out@*/ short*, /*@out@*/ short*);
  void (*display_meta1)(struct tBrowserDocument*);
  void (*display_meta2)(struct tBrowserDocument*);
} tBrowserDocumentOps;

typedef signed int tLinenumber; /* ("signed" to simplify calculations) */

#if TGC_IS_CURSES
typedef struct tActiveElementCoordinates
{ struct tActiveElementCoordinates* next;
  tLinenumber y;
  short x1, x2;
} tActiveElementCoordinates;
/* Such a list usually consists of only one entry, sometimes two (if a
   linebreak occurs within an active element). In very small table columns, the
   list might become longer - that's the main point with this structure. */
#endif

typedef struct
{ char* current_text; /* for text/password/... fields */
#if TGC_IS_GRAPHICS
  tGraphicsWidget* widget;
#else
  tActiveElementCoordinates* aec;
#endif
  tActiveElementFlags flags; /* only contains aefChangeable flags */
} tActiveElement;

typedef struct tBrowserDocument
{ const tBrowserDocumentOps* ops;
  void* container; /* e.g. "tWindowView*" for window kind "browser window" */
  tResourceRequest* request;
  tCantent* cantent; /* what's displayed for this document */
  const char *title, *minor_html_title, *last_info, *anchor;
  tActiveElement* active_element;
  tActiveElementNumber aenum, aemax;
#if !TGC_IS_GRAPHICS
  tActiveElementNumber aecur, former_ae;
#endif
  tLinenumber origin_y; /* line of content in top-left corner of e.g. window */
#if MIGHT_USE_SCROLL_BARS
  tLinenumber sbdh; /* "document height" */
  int sbvi; /* "validity indicator" */
#endif
  unsigned char redirections; /* counter to avoid infinite loops */
  tBrowserDocumentDisplayMode bddm;
  tWindowViewFlags flags;
} tBrowserDocument;

#define window_currdoc(window) (((window)->spec->find_currdoc)(window))
#define current_document_x (window_currdoc(current_window_x))

static char* sfbuf; /* "sf": "submit, form" */
static char sfconv[4];
static size_t sfbuf_size, sfbuf_maxsize;

/* prototype */
static void document_display(tBrowserDocument*, const tWindowRedrawingTask);

static void sfbuf_reset(void)
{ sfbuf = NULL; sfbuf_size = sfbuf_maxsize = 0;
}

static /* __sallocator -- not an "only" reference... */ tWindow* __callocator
  window_create(/*@notnull@*/ const tWindowSpec* spec)
{ tWindow* retval = (tWindow*) memory_allocate(sizeof(tWindow), mapWindow);
#if CONFIG_DEBUG
  sprint_safe(strbuf, "creating window %p (%d)\n", retval, spec->kind);
  debugmsg(strbuf);
#endif
  if (windowlisthead == NULL)
  { windowlisthead
#if !TGC_IS_GRAPHICS
      = visible_window_x[0]
#endif
      = retval;
  }
  if (windowlisttail != NULL)
  { retval->prev = windowlisttail; windowlisttail->next = retval; }
  windowlisttail = retval; retval->spec = spec; retval->spec->create(retval);
#if TGC_IS_GRAPHICS
  window_counter++;
#endif
  return(retval);
}

static void window_remove(tWindow* window)
{
#if CONFIG_DEBUG
  sprint_safe(strbuf, "removing window %p (%d)\n", window, window->spec->kind);
  debugmsg(strbuf);
#endif
  window->spec->remove(window);
  if (windowlisthead == window) windowlisthead = window->next;
  if (windowlisttail == window) windowlisttail = window->prev;
  if (window->prev != NULL) window->prev->next = window->next;
  if (window->next != NULL) window->next->prev = window->prev;
  memory_deallocate(window);
#if TGC_IS_GRAPHICS
  window_counter--;
  if (window_counter == 0) do_quit();
#endif
}

static __my_inline void window_redraw(tWindow* window)
{ window->spec->redraw(window);
}

static void window_redraw_all(void)
/* redraws all visible windows */
{
#if TGC_IS_CURSES
  window_redraw(visible_window_x[0]);
  if (visible_window_x[1] != NULL) window_redraw(visible_window_x[1]);
#endif
}

/* prototypes */
static tWindow* wk_browser_create(void);
static void wk_browser_prr(tWindow*, const char*, tPrrFlags,
  const tBrowserDocument*);
static void wk_browser_reload(tWindow*);
#if DO_WK_INFO
static tWindow* wk_info_create(const char*, tBrowserDocumentDisplayMode,
  /*@out@*/ tBrowserDocument** _document);
static void wk_info_collect(tWindow*, const char*);
static void wk_info_finalize(tWindow*);
static void wk_info_set_message(tWindow*, const char*, tBoolean);
#endif

static tBoolean handle_command_code(tProgramCommandCode);

static tBoolean window_is_precious(const tWindow* window)
{ tBoolean (*func)(const tWindow*) = window->spec->is_precious;
  return( (func != NULL) ? ((func)(window)) : truE );
}

static void window_hide(tWindow* window, unsigned char removal)
{ /* <removal>: 0 = must not; 1 = may; 2 = must */
#if TGC_IS_GRAPHICS
  gtk_widget_hide(GTK_WIDGET(window->ww));
  if (removal) window_remove(window);
#else
  if (window == visible_window_x[0])
  { visible_window_x[0] = visible_window_x[1];
    if (visible_window_x[0] == NULL)
    { /* Make sure we always have at least one window on the screen */
      tWindow* w = window->next;
      if (w == NULL) w = windowlisthead;
      if ( (w == NULL) || (w == window) ) w = wk_browser_create();
      visible_window_x[0] = w;
    }
    goto xy;
  }
  else if (window == visible_window_x[1])
  { xy:
    if ( (removal == 2) || ( (removal == 1) && (!window_is_precious(window)) ))
      window_remove(window);
    visible_window_x[1] = NULL; current_window_index_x = 0;
    window_redraw_all();
  }
  /* "else": nothing to do */
#endif
}

#if TGC_IS_GRAPHICS
#define window_is_visible(window) (truE)
#else
static tBoolean window_is_visible(tWindow* window)
{ tBoolean retval;
  if (window == NULL) retval = falsE; /* "should not happen" */
  else if ((window == visible_window_x[0]) || (window == visible_window_x[1]))
    retval = truE;
  else retval = falsE;
  return(retval);
}
#endif

#define is_browser_window(window) ((window)->spec->kind == wkBrowser)

static tWindow* require_browser_window(void)
{ tWindow* window = current_window_x;
  if (!is_browser_window(window))
    window = current_window_x = wk_browser_create();
  return(window);
}

static void redraw_message_line(void)
{
#if TGC_IS_CURSES
  tWindow* window = current_window_x;
  tBoolean is_error;
  const char* str;
  const char* (*func)(tWindow*, tBoolean*) = window->spec->get_info;
  if (func != NULL) str = (func)(window, &is_error);
  else { str = NULL; is_error = falsE; }
  show_message(null2empty(str), is_error);
#endif
}

static tBoolean window_contents_minmaxrow(const tWindow* window,
  /*@out@*/ short* _minrow, /*@out@*/ short* _maxrow)
{ tBoolean retval;
  short minrow = 0, maxrow = MAX(LINES - 3, 0);
  if (window == visible_window_x[0])
  { if (visible_window_x[1] != NULL) /* only use top-half of screen */
      maxrow = VMIDDLE - 1;
    it_worked: *_minrow = minrow; *_maxrow = maxrow; retval = truE;
  }
  else if (window == visible_window_x[1]) /* only use bottom-half of screen */
  { minrow = VMIDDLE + 1; goto it_worked; }
  else retval = falsE; /* "should not happen" */
  return(retval);
}


/** Message display */

static void show_message(const char* msg, tBoolean is_error)
{
#if TGC_IS_GRAPHICS
  tGraphicsWidget* widget = current_window->message;
  gtk_label_set_text(GTK_LABEL(widget), msg);
#else
  if (!is_bottom_occupied)
  { if (is_error) my_set_color(cpnRed);
    (void) mvaddnstr(LINES - 1, 0, msg, COLS - 1);
    if (is_error) my_set_color(cpnDefault);
    (void) clrtoeol(); must_reset_cursor();
  }
#endif
  debugmsg("UI message: "); debugmsg(msg); debugmsg(strNewline);
}

static void show_message_osfailure(int errnum, const char* text)
{ if (text == NULL) text = _("Failed!"); /* some stupidistic default text */
  if (errnum > 0)
  { const char* e = my_strerror(errnum);
    if (strlen(e) > STRBUF_SIZE / 2) e = _(strUnknown); /* can't be serious */
    sprint_safe(strbuf, _("%s - error #%d, %s"), text, errnum, e);
    text = strbuf; /* CHECKME: use strErrorTrail[]! */
  }
  show_message(text, truE);
}

#if CONFIG_CONSOLE
static __my_inline void cc_output_str(const char* str)
{ my_write_str(fd_stdout, str); /* FIXME for window! */
}
#define cc_output_errstr(str) cc_output_str(str) /* (_currently_ the same) */
#endif

static void inform_about_error(int errnum, const char* text)
{ if (is_environed) show_message_osfailure(errnum, text);
#if CONFIG_CONSOLE
  else if (program_mode == pmConsole)
  { cc_output_errstr(text);
    if (errnum > 0)
    { char buf[1000];
      const char* errstr = my_strerror(errnum);
      if (strlen(errstr) > 200) errstr = _(strUnknown);
      sprint_safe(buf, _(strErrorTrail), errnum, errstr);
      cc_output_errstr(buf);
    }
    cc_output_errstr(strNewline);
  }
#endif
  else fatal_error(errnum, text);
}

#if ((CONFIG_MENUS & MENUS_ALL) != MENUS_ALL) || (!CONFIG_SESSIONS) || (!CONFIG_JUMPS) || (!CONFIG_DO_TEXTMODEMOUSE) || (!OPTION_EXECEXT) || (!MIGHT_USE_SCROLL_BARS) || (!(CONFIG_EXTRA & EXTRA_DOWNLOAD)) || (!(CONFIG_EXTRA & EXTRA_DUMP)) || (!DO_WK_INFO) /* (-: */
static void fwdact(const char* str)
{ char* spfbuf;
  if (*str == 'F') str++;
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, _(strFwdact), str, strEmpty);
  *spfbuf = my_toupper(*spfbuf);
  show_message(spfbuf, truE);
  my_spf_cleanup(strbuf, spfbuf);
}
#endif

#if (CONFIG_MENUS & MENUS_ALL) != MENUS_ALL
static void menus_were_disabled(void)
{ fwdact(_("Fthis kind of menus"));
}
#endif

#if TGC_IS_CURSES
static my_inline void terminal_too_small(void)
{ show_message(_("Terminal too small"), truE);
}
#endif


/** Resource requests */

static void request_dhm_control(__sunused void* _request __cunused, __sunused
  void* data __cunused, __sunused tDhmControlCode dcc __cunused)
/* just a dummy, at least for now */
{ /* tResourceRequest* request = (tResourceRequest*) _request; */
}

static __sallocator tResourceRequest* __callocator request_create(void)
{ tResourceRequest* retval = (tResourceRequest*)
    memory_allocate(sizeof(tResourceRequest), mapResourceRequest);
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "request_create(): %p\n", retval);
  debugmsg(debugstrbuf);
#endif
  dhm_init(retval, request_dhm_control, "request");
  return(retval);
}

static void request_remove(tResourceRequest* request)
{
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "request_remove(): %p\n", request);
  debugmsg(debugstrbuf);
#endif
  dhm_notify(request, dhmnfRemoval);
  if (request->resource != NULL) dhm_detach(request->resource);
  if (request->uri_data != NULL) uri_detach(request->uri_data);
  sinking_data_deallocate(&(request->sinking_data));
  memory_deallocate(request);
}

static void request_set_error(tResourceRequest* request, tResourceError error)
{ if (request->state != rrsError)
  { request->state = rrsError; request->error = error; }
}

static void request_copy_error(tResourceRequest* request,
  const tResource* resource)
/* CHECKME! */
{ if ( (resource->state == rsError) && (request->state != rrsError) )
  { request->state = rrsError; request->error = resource->error; }
}

static void request_queue(tResourceRequest* request,
  tResourceRequestAction action)
/* feeds the request to the resource handler for kickoff */
{ request->action = action; request->state = rrsPreparedByMain;
#if CONFIG_DEBUG
  { const tUriData* u = request->uri_data;
    char* spfbuf;
    my_spf(strbuf, STRBUF_SIZE, &spfbuf,
      "Queueing request: %d - *%s* - *%s* - *%s*\n", u->rp, null2empty(u->uri),
      null2empty(u->query), null2empty(u->post));
    debugmsg(spfbuf); my_spf_cleanup(strbuf, spfbuf);
  }
#endif
  resource_request_start(request);
}

typedef struct
{ const char* uri; /*I*/
  const tBrowserDocument* referrer; /*I*/
  tResourceRequest* result; /*O*/
  const char* uri_anchor; /*O*/ /* (if prrfWantUriAnchor) */
  tPrrFlags prrf; /*I*/
} tPrrData; /* I=input, O=output */

static my_inline void prr_setup(/*@out@*/ tPrrData* data, const char* uri,
  tPrrFlags prrf)
{ my_memclr(data, sizeof(tPrrData)); data->uri = uri; data->prrf = prrf;
}

static void prr_setdown(tPrrData* data)
/* removes things which were allocated by prepare_resource_request() but
   not "consumed" by its caller */
{ tResourceRequest* request = data->result;
  if (request != NULL) request_remove(request);
  __dealloc(data->uri_anchor);
}

static void prepare_resource_request(tPrrData* data)
{ tResourceRequest* request = data->result = request_create();
  const tBrowserDocument* _referrer = data->referrer;
  const tResourceRequest* referrer = (_referrer ? _referrer->request : NULL);
  const tUriData* ref_ud = (referrer ? referrer->uri_data : NULL);
  const char *sfbuf_final, *extra_query;
  const tPrrFlags prrf = data->prrf;
  tUriData* uri_data;

  if (sfbuf == NULL) sfbuf_final = NULL;
  else
  { sfbuf_final = sfbuf;
    if (*sfbuf_final == '&') sfbuf_final++;
    if (*sfbuf_final == '\0') sfbuf_final = NULL; /* no "real" data */
  }
  if ( (!(prrf & prrfPost)) && (prrf & prrfUseSfbuf) && (sfbuf_final != NULL) )
  { /* not a post request, but still some submit-form data, thus query */
    extra_query = sfbuf_final;
  }
  else extra_query = NULL;

  uri_data = uri_parse(data->uri, ref_ud, ( (prrf & prrfWantUriAnchor) ?
    (&(data->uri_anchor)) : NULL ), extra_query, ( (prrf & prrfUpsfp4) ? 4:0));
  uri_attach(request->uri_data, uri_data);
  if (uri_data->re != reFine) request_set_error(request, uri_data->re);

  if (prrf & prrfPost)
  { request->flags |= rrfPost;
    if ( (prrf & prrfUseSfbuf) && (sfbuf_final != NULL) )
      request->uri_data->post = my_strdup(sfbuf_final);
  }
  if (sfbuf != NULL) { memory_deallocate(sfbuf); sfbuf_reset(); }

  if ( (prrf & prrfIsRedirection) ||
       ( (_referrer != NULL) && (_referrer->redirections > 0) ) )
  { request->flags |= rrfIsRedirection; }
}

static const char* calculate_reqresmsg(const tResourceRequest* request,
  const tResource* resource, unsigned char flags, /*@out@*/tBoolean* _is_error)
{ const char* retval;
  *_is_error = falsE; /* default */
  if (resource != NULL)
  { const tCantent* cantent = resource->cantent;
    const tResourceState rs = resource->state;
    const tServerStatusCode ssc = resource->server_status_code;
    if (rs == rsError)
    { const tResourceError re = resource->error;
#if OPTION_TLS
      if ((re == reTls) && (tls_errtext(resource, strbuf2))) retval = strbuf2;
      else
#endif
      { retval = _(strResourceError[re]); }
      *_is_error = truE;
    }
#if OPTION_TLS
    else if (resource_in_tls(resource, truE)) retval = _("TLS handshaking");
#endif
    else if (rs == rsReading)
    { const size_t count = resource->bytecount;
      if (count < 2) retval = _("Waiting for reply");
      else
      { const size_t nominal = resource->nominal_contentlength;
        const char* temp;
        if ( (nominal != UNKNOWN_CONTENTLENGTH) && (count < nominal) )
        { sprint_safe(strbuf3, _("of %d "), localized_size(nominal));
          temp = strbuf3;
        }
        else temp = strEmpty;
        sprint_safe(strbuf2, _("Received %d %sbytes"), localized_size(count),
          temp);
        retval = strbuf2;
      }
    }
    else if ( (!(flags & 1)) && (rs == rsComplete) && ( (cantent->content ==
      NULL) || (cantent->content->used <= 0) ) )
    { retval = _("Document empty"); }
    else if (resource_ui_conn_ip(resource, strbuf2, STRBUF_SIZE / 2))
    { sprint_safe(strbuf3, _("Connecting to %s"), strbuf2); retval = strbuf3; }
    else retval = _(strResourceState[rs]);
#if CONFIG_DEBUG
    sprint_safe(strbuf,
      "%s [res=%p,rs=%d,re=%d,rch=%d,rf=%d,ths=%d,ssc=%d,ssi=*%s*]", retval,
      resource, rs, resource->error, resource->handshake, resource->flags,
      resource->tlheaderstate, ssc, ssc2info(resource->protocol, ssc));
    retval = strbuf;
#else
    if (ssc != 0)
    { const tResourceProtocol rp = resource->protocol;
      const char *ssi = ssc2info(rp, ssc), *ssi_sep = ( (ssi == strEmpty) ?
        strEmpty : strSpacedDash ), *whose;
#if OPTION_LOCAL_CGI
      if (rp == rpLocalCgi) whose = _("script status: ");
      else
#endif
      { whose = _("server status: "); }
      sprint_safe(strbuf, "%s (%s%d%s%s)", retval, whose, ssc, ssi_sep, ssi);
      retval = strbuf;
    }
#endif
  }
  else if (request != NULL)
  { const tResourceRequestState rrs = request->state;
    switch (rrs)
    { case rrsError:
        retval = _(strResourceError[request->error]); *_is_error = truE; break;
      case rrsUnknown: retval = strEmpty; break;
      default: retval = _(strResourceRequestState[rrs]); break;
    }
  }
  else retval = strEmpty;
  return(retval);
}


/** Browser documents */

/* Browser documents: screen coordinates */

#if TGC_IS_GRAPHICS

static my_inline tLinenumber document_section_height(const tBrowserDocument*
  document)
{ return(LINES); /* FIXME! */
}

static my_inline void calculate_minmaxrow(const tBrowserDocument* document
  __cunused, /*@out@*/ short* min, /*@out@*/ short* max)
{ *min = 0; *max = LINES - 1; /* FIXME! */
}

#else

static __my_inline tBoolean document_find_coords(const tBrowserDocument*
  document, /*@out@*/ short* _x1, /*@out@*/ short* _y1, /*@out@*/ short* _x2,
  /*@out@*/ short* _y2)
{ return((document->ops->find_coords)(document, _x1, _y1, _x2, _y2));
}

static tBoolean document_section_height(const tBrowserDocument* document,
  /*@out@*/ tLinenumber* _height)
{ short x1, y1, x2, y2;
  const tBoolean retval = document_find_coords(document, &x1, &y1, &x2, &y2);
  if (retval) *_height = y2 - y1 + 1;
  return(retval);
}

#if (CONFIG_TG == TG_XCURSES) && (MIGHT_USE_SCROLL_BARS)
static tBoolean document_minmaxcol(__sunused const tBrowserDocument* document
  __cunused, /*@out@*/ short* _mincol, /*@out@*/ short* _maxcol)
{ *_mincol = 0; *_maxcol = COLS - 1; /* FIXME for framesets etc.! */
  return(truE);
}
#endif

static tBoolean document_minmaxrow(const tBrowserDocument* document,
  /*@out@*/ short* _minrow, /*@out@*/ short* _maxrow)
{ short x1, y1, x2, y2;
  tBoolean retval = document_find_coords(document, &x1, &y1, &x2, &y2);
  if (retval) { *_minrow = y1; *_maxrow = y2; }
  return(retval);
}

static tBoolean document_line2row(const tBrowserDocument* document,
  tLinenumber y, /*@out@*/ short* _row)
{ short x1, y1, x2, y2;
  tBoolean retval = document_find_coords(document, &x1, &y1, &x2, &y2);
  if (retval) { *_row = y1 + ((short) (y - document->origin_y)); }
  return(retval);
}

#endif


/* Browser documents: active elements */

static void deallocate_aec(/*const*/ tActiveElementCoordinates** _aec)
{ const tActiveElementCoordinates* aec = *_aec;
  if (aec == NULL) return; /* nothing to do */
  while (aec != NULL)
  { const tActiveElementCoordinates* aec_next = aec->next;
    memory_deallocate(aec); aec = aec_next;
  }
  *_aec = NULL;
}

static void __init_ae(const tActiveElementBase* aeb, tActiveElement* ae,
  tBoolean is_first_init
#if TGC_IS_GRAPHICS
  , tWindowView* view
#endif
  )
{ const tActiveElementKind aek = aeb->kind;
  const char* text;

  if (is_first_init) my_memclr(ae, sizeof(tActiveElement));
  else dealloc(ae->current_text);
  ae->flags = aeb->flags & aefChangeable;

  if ((aek == aekFormText) || (aek == aekFormPassword) || (aek == aekFormFile))
  { if ( (text = aeb->render) != NULL ) ae->current_text = my_strdup(text);
  }
  else if (aek == aekFormSelect)
  { const tHtmlInputLength count = aeb->maxlength;
    if (count > 0) /* non-empty selection list */
    { char* t = ae->current_text = memory_allocate((count + 7) / 8, mapOther);
      const tHtmlOption* o = (const tHtmlOption*) aeb->render;
      tHtmlInputLength c = 0;
      while (o != NULL)
      { if (o->flags & hofSelected) my_bit_set(t, c);
        o = o->next; c++;
      }
    }
  }

#if TGC_IS_GRAPHICS
  if (is_first_init) /* possibly create a widget */
  { tGraphicsWidget* w;
    tHtmlInputLength maxlength;
    switch (aek)
    { case aekFormText: case aekFormPassword: case aekFormTextarea:
        text = aeb->render;
        maxlength = (has_input_length(aek) ? aeb->maxlength : 0);
        /* FIXME: don't show passwords! */
        /* w = gtk_text_new(NULL, NULL); */
        w = ( (maxlength > 0) ? gtk_entry_new_with_max_length(maxlength) :
          gtk_entry_new() );
        if ( (text != NULL) && (*text != '\0') )
          gtk_entry_set_text(GTK_ENTRY(w), text);
        break;
      case aekFormCheckbox: w = gtk_check_button_new(); break;
      case aekFormRadio: w = gtk_radio_button_new(NULL); break;/*IMPLEMENTME!*/
      case aekFormFile: w = gtk_file_selection_new(strEmpty); break;
      default: w = NULL; break;
    }
    /* FIXME: check whether disabled, readonly, ...! */
    if (w != NULL)
    { pack_box(view->window->contents, w); show_widget(w); ae->widget = w; }
  }
#endif
}

#if TGC_IS_GRAPHICS
#define init_ae(a, b, c, d) __init_ae(a, b, c, d)
#else
#define init_ae(a, b, c, d) __init_ae(a, b, c)
#endif

#if !TGC_IS_GRAPHICS

static const tActiveElementCoordinates*
  find_visible_aec(const tBrowserDocument* document, tActiveElementNumber _ae)
/* tries to find visible coordinates for the given active element */
{ const tActiveElementCoordinates *retval = NULL,
    *aec = document->active_element[_ae].aec;
  tLinenumber yvis1, yvis2;
  short minrow, maxrow;

  if (aec == NULL) goto out; /* no coordinates, nothing visible */
  if (!document_minmaxrow(document, &minrow, &maxrow)) goto out;
  yvis1 = document->origin_y; yvis2 = yvis1 + (maxrow - minrow);
  while (aec != NULL)
  { tLinenumber y = aec->y;
    if ( (y >= yvis1) && (y <= yvis2) ) { retval = aec; break; }
    aec = aec->next;
  }
  out:
  return(retval);
}

static __my_inline tBoolean is_ae_visible(const tBrowserDocument* document,
  tActiveElementNumber _ae)
{ return(cond2boolean(find_visible_aec(document, _ae) != NULL));
}

static tActiveElementNumber next_visible_ae(const tBrowserDocument* document,
  tActiveElementNumber _ae)
{ tActiveElementNumber retval = INVALID_AE, l = document->aenum;
  while (++_ae < l)
  { if (is_ae_visible(document, _ae)) { retval = _ae; break; } }
  return(retval);
}

static tActiveElementNumber previous_visible_ae(const tBrowserDocument*
  document, tActiveElementNumber _ae)
{ tActiveElementNumber retval = INVALID_AE;
  while (--_ae >= 0)
  { if (is_ae_visible(document, _ae)) { retval = _ae; break; } }
  return(retval);
}

static void do_activate_element(tBrowserDocument* document,
  tActiveElementNumber _ae, tBoolean active)
{ const tActiveElementCoordinates* aec = document->active_element[_ae].aec;
  attr_t attribute;
  short minrow, maxrow, row;
  tLinenumber y1, y2;
  if (aec == NULL) goto finish;
  if (!document_section_height(document, &y2)) goto finish;
  y1 = document->origin_y; y2 += y1 - 1;
  attribute = (active ? A_REVERSE : A_UNDERLINE);
  document_minmaxrow(document, &minrow, &maxrow);
  while (aec != NULL)
  { tLinenumber y = aec->y;
    short x1, x2, count;
    if (y < y1) goto do_next;
    else if (y > y2) break; /* done */
    if (!document_line2row(document, y, &row)) break; /* "should not happen" */
    x1 = aec->x1; x2 = aec->x2; count = x2 - x1 + 1; (void) move(row, x1);
    while (count-- > 0)
      (void) addch((inch() & ~(A_REVERSE | A_UNDERLINE)) | attribute);
    do_next:
    aec = aec->next;
  }
  finish:
  if (active) document->aecur = document->former_ae = _ae;
  else document->aecur = INVALID_AE;
}

static void activate_element(tBrowserDocument* document,
  tActiveElementNumber _ae)
{ tActiveElementNumber old = document->aecur;
  if (old != INVALID_AE)
    do_activate_element(document, old, falsE); /* deactivate old */
  do_activate_element(document, _ae, truE); /* activate new */
  must_reset_cursor();
}

#endif /* #if TGC_IS_GRAPHICS */


/* Browser documents: general */

static my_inline void document_init(tBrowserDocument* document,
  const tBrowserDocumentOps* ops, void* container, tResourceRequest* request)
{ document->ops = ops; document->container = container;
  document->request = request;
#if !TGC_IS_GRAPHICS
  document->aecur = document->former_ae = INVALID_AE;
#endif
}

static void document_tear(const tBrowserDocument* document)
/* IMPORTANT: the <document> itself must be deallocated by the _caller_ (if
   necessary at all)! */
{ tResourceRequest* request = document->request;
  tCantent* cantent = document->cantent;
  if (request != NULL) request_remove(request);
  if (cantent != NULL) cantent_put(cantent);
  __dealloc(document->title); __dealloc(document->minor_html_title);
  __dealloc(document->last_info); __dealloc(document->anchor);
  if (document->aenum > 0)
  { /*const*/ tActiveElement* aes = document->active_element;
    tActiveElementNumber _ae;
    for (_ae = 0; _ae < document->aenum; _ae++)
    { __dealloc(aes[_ae].current_text);
#if TGC_IS_GRAPHICS
      /* FIXME: handle widget! */
#else
      deallocate_aec(&(aes[_ae].aec));
#endif
    }
    memory_deallocate(aes);
  }
}


/* Browser documents: HTML forms */

static void __sfbuf_add(const char ch)
/* append character unconverted */
{ if (sfbuf_maxsize <= sfbuf_size)
  { sfbuf_maxsize += 100;
    sfbuf = memory_reallocate(sfbuf, sfbuf_maxsize, mapString);
  }
  sfbuf[sfbuf_size++] = ch;
}

static my_inline void __sfbuf_add_str(const char* str)
/* append string unconverted */
{ char ch;
  while ( (ch = *str++) != '\0' ) __sfbuf_add(ch);
}

static void sfbuf_add_str(const char* str)
/* append string converted */
{ char ch;
  while ( (ch = *str++) != '\0' )
  { /* append character converted */
    { if (my_isalnum(ch)) __sfbuf_add(ch);
      else if (ch == ' ') __sfbuf_add('+');
      else if (ch == '\n') __sfbuf_add_str("%0d%0a");
      else if (ch != '\r')
      { sfconv[1] = strHexnum[(ch >> 4) & 15]; sfconv[2] = strHexnum[ch & 15];
        __sfbuf_add_str(sfconv);
      }
    }
  }
}

static tHtmlFormNumber calc_hfn(const tCantent* cantent,
  tActiveElementNumber _ae)
/* searches for the number of the HTML form to which the <_ae> belongs */
{ tHtmlFormNumber retval = INVALID_HTML_FORM_NUMBER, hfn, hfnum;
  const tHtmlForm* form;
  if ( (_ae == INVALID_AE) || (cantent == NULL) ||
       ( (hfnum = cantent->hfnum) <= 0 ) )
  { goto out; } /* can't find anything */
  form = cantent->form;
  for (hfn = 0; hfn < hfnum; hfn++)
  { tActiveElementNumber a1 = form[hfn].first_ae, a2 = form[hfn].last_ae;
    if ((a1 != INVALID_AE) && (a2 != INVALID_AE) && (_ae >= a1) && (_ae <= a2))
    { retval = hfn; goto out; } /* found */
  }
  out:
  return(retval);
}

static void cnsniaf(void)
{ show_message(_("Can't submit - not inside a form?"), truE);
}

static void document_form_submit(/*@notnull@*/ const tBrowserDocument*document)
{ const tCantent* cantent = document->cantent;
  const tActiveElementBase* aebase;
  tActiveElementNumber _ae, a1, a2;
  const tActiveElement* aes;
#if TGC_IS_GRAPHICS
  const tActiveElementNumber aecur = INVALID_AE; /* FIXME! */
  const tHtmlFormNumber hfn = INVALID_HTML_FORM_NUMBER; /* FIXME! */
#else
  const tActiveElementNumber aecur = document->aecur;
  const tHtmlFormNumber hfn = calc_hfn(cantent, aecur);
#endif
  const tHtmlOption* o;
  tHtmlOptionNumber onum;
  tPrrFlags prrf;

  if ( (cantent == NULL) || (hfn == INVALID_HTML_FORM_NUMBER) )
  { cant_submit: cnsniaf(); return; }
  a1 = cantent->form[hfn].first_ae; a2 = cantent->form[hfn].last_ae;
  if ( (a1 == INVALID_AE) || (a2 == INVALID_AE) ) goto cant_submit;

  javascript_handle_event(jekSubmit, aecur); sfbuf_reset();
  aebase = cantent->aebase; aes = document->active_element;
  for (_ae = a1; _ae <= a2; _ae++)
  { tActiveElementKind kind;
    const char *name, *value, *bitfield;
    tBoolean is_first, is_multiple;
    if (aebase[_ae].flags & aefDisabled) continue;
    kind = aebase[_ae].kind;
    if ( (!is_form_aek(kind)) || ( (name = aebase[_ae].data) == NULL )
        || (*name == '\0') )
    { continue; } /* non-form element or no element name - unusable here */
    switch (kind)
    {case aekFormText: case aekFormPassword: case aekFormTextarea:
      value = aes[_ae].current_text;
      append:
      if (value != NULL)
      { __sfbuf_add('&'); sfbuf_add_str(name);
        __sfbuf_add('='); sfbuf_add_str(value);
      }
      break;
     case aekFormCheckbox:
      if (aes[_ae].flags & aefCheckedSelected) { value = strOn; goto append; }
      break;
     case aekFormRadio:
      if (aes[_ae].flags & aefCheckedSelected)
      { value = aebase[_ae].render; goto append; }
      break;
     case aekFormSubmit: case aekFormImage:
      if (_ae == aecur)
      { if (kind == aekFormImage)
        { __sfbuf_add('&'); sfbuf_add_str(name); __sfbuf_add_str(".x=0&");
          sfbuf_add_str(name); __sfbuf_add_str(".y=0"); continue;
        }
        else if (aebase[_ae].flags & aefButtonTag) value = aebase[_ae].render;
        else value = strOn;
        goto append;
      }
      break;
     case aekFormHidden:
      value = aebase[_ae].render; goto append; /*@notreached@*/ break;
     case aekFormSelect:
      bitfield = aes[_ae].current_text;
      o = (const tHtmlOption*) aebase[_ae].render; onum = 0; is_first = truE;
      is_multiple = cond2boolean(aebase[_ae].flags & aefMultiple);
      while (o != NULL)
      { if (my_bit_test(bitfield, onum)) /* option is selected */
        { if (!is_first) __sfbuf_add(',');
          else
          { __sfbuf_add('&'); sfbuf_add_str(name); __sfbuf_add('=');
            is_first = falsE;
          }
          sfbuf_add_str(o->value);
          if (!is_multiple) break; /* done with collecting */
        }
        o = o->next; onum++;
      }
      break;
     /* case aekFormReset: break; -- nothing to do, element can't succeed */
     /* case aekFormFile: break; -- IMPLEMENTME! */
    }
  }
  __sfbuf_add('\0');
  prrf = prrfRedrawOne | prrfUseSfbuf;
  if (cantent->form[hfn].flags & hffMethodPost) prrf |= prrfPost;
  wk_browser_prr(require_browser_window(), cantent->form[hfn].action_uri, prrf,
    document);
}

static void current_document_form_submit(void)
{ const tBrowserDocument* document = current_document_x;
  if (document != NULL) document_form_submit(document);
}

static void document_form_reset(tBrowserDocument* document)
{ const tCantent* cantent = document->cantent;
  const tActiveElementBase* aebase;
  tActiveElementNumber _ae, a1, a2;
  tActiveElement* aes;
#if TGC_IS_GRAPHICS
  tActiveElementNumber aecur = INVALID_AE; /* FIXME! */
  tHtmlFormNumber hfn = INVALID_HTML_FORM_NUMBER; /* FIXME! */
#else
  tActiveElementNumber aecur = document->aecur;
  tHtmlFormNumber hfn = calc_hfn(cantent, aecur);
#endif
  if ( (cantent == NULL) || (hfn == INVALID_HTML_FORM_NUMBER) )
  { cant_reset: show_message(_("Can't reset - not inside a form?"), truE);
    return;
  }
  a1 = cantent->form[hfn].first_ae; a2 = cantent->form[hfn].last_ae;
  if ( (a1 == INVALID_AE) || (a2 == INVALID_AE) ) goto cant_reset;
  javascript_handle_event(jekReset, aecur);
  aebase = cantent->aebase; aes = document->active_element;
  for (_ae = a1; _ae <= a2; _ae++)
    init_ae(&(aebase[_ae]), &(aes[_ae]), falsE, document);
  document_display(document, wrtRedraw);
}

static void current_document_form_reset(void)
{ tBrowserDocument* document = current_document_x;
  if (document != NULL) document_form_reset(document);
}


/* Browser documents: displaying on screen */

#if (TGC_IS_WINDOWING) || (CONFIG_EXTRA & EXTRA_DUMP) || (DO_PAGER)
#include "renderer.c"
#endif

static void wrc_rd_setup(tRendererData* rd, /*@notnull@*/ tBrowserDocument*
  document, short width)
{ const tCantent* cantent = document->cantent;
  const tBrowserDocumentDisplayMode bddm = document->bddm;
  my_memclr(rd, sizeof(tRendererData));
  rd->ra = raLayout; rd->document = document; rd->line_width = width;
  rd->flags = rdfAttributes | rdfAlignment | rdfAe;
#if MIGHT_USE_COLORS
  rd->flags |= rdfColors;
#endif
#if TGC_IS_GRAPHICS
  rd->flags |= rdfGraphical;
#endif
#if TGC_IS_PIXELING
  rd->flags |= rdfPixeling;
#endif
#if CONFIG_HTML & HTML_FRAMES
  if (!(config.flags & cfHtmlFramesSimple)) rd->flags |= rdfFrames;
#endif
  if (bddm != bddmAutodetect) /* the user explicitly wants something */
  { if (bddm == bddmHtml) { set_html: rd->flags |= rdfHtml; }
    goto kind_done;
  }
  if ( (cantent != NULL) && (cantent->kind == rckHtml) )
    goto set_html; /* resource kind knowledge, no explicit user preference */
  kind_done: {}
}

my_enum1 enum
{ wrfNone = 0, wrfFound = 0x01
} my_enum2(unsigned char) tWindowRedrawingFlags; /* CHECKME: rename! */

typedef struct
{ tLinenumber origin_y, currline, highest_backward_line;
  size_t searchlen;
  short row, maxrow, mincol;
  tWindowRedrawingTask task;
  tWindowRedrawingFlags flags;
} tWindowRedrawingData; /* CHECKME: rename! */

static void __document_display_line(tWindowRedrawingData* wrd,
  tRendererData* data)
/* actually draws a line on the screen */
{ const tRendererElement* element = data->element;
  (void) move(wrd->row, wrd->mincol);
  while (element != NULL)
  { const tRendererText* text = element->text;
    const tRendererAttr* attr = element->attr;
    size_t count = 0, textcount = element->textcount;
    if (element->is_spacer) { while (count++ < textcount) addch(' '); }
    else
    { while (count < textcount)
      { tRendererAttr a = 0;
        if (text != NULL) a |= (tRendererAttr) text[count];
        if (attr != NULL) a |= attr[count];
        if (a == 0) /* "should not happen" */
          a = (tRendererAttr) ((unsigned char) ' ');
        addch(a); count++;
      }
    }
    element = element->next;
  }
  (void) clrtoeol(); wrd->row++;
  if (wrd->row > wrd->maxrow) data->flags |= rdfCallerDone;
}

static void document_display_line__standard(tRendererData* data)
{ tWindowRedrawingData* wrd = data->line_callback_data;
  if (wrd->currline < wrd->origin_y)
  { wrd->currline++;
    if (wrd->currline >= wrd->origin_y) data->flags &= ~rdfVirtual;
  }
  else { wrd->currline++; __document_display_line(wrd, data); }
}

static /*@null@*/ const char* search_string = NULL;

static tBoolean search_in_line(const tRendererElement* element,
  size_t searchlen)
/* returns whether it found something */
{ tBoolean retval = falsE;
  while (element != NULL)
  { const char* text;
    size_t textcount = element->textcount;
    if ( (!(element->is_spacer)) && ( (text = element->text) != NULL ) &&
         ( (textcount = element->textcount) >= searchlen ) &&
         (my_strncasestr(text, search_string, textcount)) )
    { retval = truE; break; }
    element = element->next;
  }
  return(retval);
}

static void document_display_line__search(tRendererData* data)
{ tWindowRedrawingData* wrd = data->line_callback_data;
  if (wrd->currline <= wrd->origin_y)
  { wrd->currline++;
    if (wrd->currline > wrd->origin_y) data->flags &= ~rdfVirtual;
  }
  else if (wrd->flags & wrfFound)
  { wrd->currline++; do_redraw: __document_display_line(wrd, data); }
  else
  { wrd->currline++;
    if (search_in_line(data->element, wrd->searchlen))
    { if (data->document != NULL) data->document->origin_y = wrd->currline - 1;
      wrd->flags |= wrfFound; goto do_redraw;
    }
  }
}

static void document_display_line__search_backward(tRendererData* data)
{ tWindowRedrawingData* wrd = data->line_callback_data;
  wrd->currline++;
  if (wrd->currline < wrd->origin_y)
  { if (search_in_line(data->element, wrd->searchlen))
    { wrd->highest_backward_line = wrd->currline - 1; wrd->flags |= wrfFound; }
  }
  else data->flags |= rdfCallerDone;
}

static void document_display_line__count(tRendererData* data)
{ tWindowRedrawingData* wrd = data->line_callback_data;
  wrd->currline++;
}

static void document_do_display(/*@notnull@*/ tBrowserDocument* document,
  const short mincol, const short minrow, const short maxcol,
  const short maxrow, const tWindowRedrawingTask task)
{ tRendererData data;
  tWindowRedrawingData wrd;
  const tBoolean is_search =
    cond2boolean( (task == wrtSearch) || (task == wrtSearchBackward) );
#if !TGC_IS_GRAPHICS
  tActiveElementNumber _ae;
#endif

  if (task != wrtRedrawRecursive) { (document->ops->display_meta1)(document); }
  wrc_rd_setup(&data, document, maxcol - mincol + 1);
  my_memclr_var(wrd); data.line_callback_data = &wrd;
  data.line_callback = document_display_line__standard; /* default callback */
  if (is_search)
  { wrd.searchlen = strlen(search_string);
    if (task == wrtSearch) data.line_callback = document_display_line__search;
    else if (task == wrtSearchBackward)
      data.line_callback = document_display_line__search_backward;
  }
  else if (task == wrtToEnd) data.line_callback = document_display_line__count;

#if !TGC_IS_GRAPHICS
  _ae = document->aecur;
  if (_ae != INVALID_AE) do_activate_element(document, _ae, falsE);
#endif

  wrd.row = minrow; wrd.maxrow = maxrow; wrd.mincol = mincol; wrd.task = task;
  wrd.origin_y = document->origin_y;
  switch (task)
  { case wrtSearch: case wrtToEnd: data.flags |= rdfVirtual; break;
    case wrtSearchBackward: /* nothing */ break;
    default: if (wrd.origin_y > 0) { data.flags |= rdfVirtual; } break;
  }

  renderer_run(&data);

  if (task == wrtRedraw) { /* nothing (test most likely case first) */ }
  else if (is_search)
  { const tBoolean did_find = cond2boolean(wrd.flags & wrfFound);
    if ( (did_find) && (task == wrtSearchBackward) )
      document->origin_y = wrd.highest_backward_line;
    /* IMPROVEME: don't redraw always, only if necessary! */
    document_do_display(document, mincol, minrow, maxcol, maxrow,
      wrtRedrawRecursive); /* risk a recursion :-) */
    if (did_find) show_message(_("Found!"), falsE);
    else show_message(_("Not found!"), truE);
    goto out; /* rest was done in recursively called instance */
  }
  else if (task == wrtToEnd)
  { const short height = maxrow - minrow;
    const tLinenumber num = wrd.currline;
    document->origin_y = ( (num > height) ? (num - height) : 0 );
    document_do_display(document, mincol, minrow, maxcol, maxrow,
      wrtRedrawRecursive);
    goto out;
  }
  if (wrd.row > wrd.maxrow) document->flags |= wvfScreenFull;
  else document->flags &= ~wvfScreenFull;
  while (wrd.row <= wrd.maxrow)
  { (void) move(wrd.row, mincol); (void) clrtoeol(); wrd.row++; }

#if !TGC_IS_GRAPHICS
  if (_ae == INVALID_AE) _ae = document->former_ae;
  if (_ae != INVALID_AE)
  { if (!is_ae_visible(document, _ae))
    { if (document->flags & wvfScrollingUp)
        _ae = previous_visible_ae(document, _ae);
      else _ae = next_visible_ae(document, _ae);
    }
  }
  else
  { _ae = next_visible_ae(document, -1); /* find the first visible ae */
    if ( (_ae != INVALID_AE) && (document->flags & wvfScrollingUp) )
    { /* proceed to the _last_ visible active element */
      tActiveElementNumber _ae2, l = document->aenum;
      for (_ae2 = _ae + 1; _ae2 < l; _ae2++)
      { if (is_ae_visible(document, _ae2)) _ae = _ae2; }
    }
  }
  if (_ae != INVALID_AE) activate_element(document, _ae);
#endif
  (document->ops->display_meta2)(document);
  document->flags &= ~wvfScrollingUp;
#if 0
  /* some texts for the very next versions...
     _("looking for fragment"), _("fragment not found")
     _("disk cache file"), _("Do not edit manually while retawq is running!"),
     _("Upload local file: "), _("Upload to FTP URL: "),
     _("Resume upload? [(r)esume, (o)verwrite%s]"), _(", (s)kip"),
     _("File size: local %d, remote %d")
  */
#endif
  out: {}
}

static void document_display(tBrowserDocument* document,
  const tWindowRedrawingTask task)
{ short mincol, minrow, maxcol, maxrow;
  if (document_find_coords(document, &mincol, &minrow, &maxcol, &maxrow))
  { document_do_display(document, mincol, minrow, maxcol, maxrow, task);
    must_reset_cursor();
  }
  /* "else": "should not happen" */
}


/** Downloads */

#if CONFIG_EXTRA & (EXTRA_DOWNLOAD | EXTRA_DUMP)

static void print_automated_uri(const char* uri)
{ if (lfdmbs(2))
  { my_write_str(fd_stderr, strUriColonSpace);
      /* (no _() here - this output shall be machine-parseable!) */
    my_write_str(fd_stderr, uri); my_write_str(fd_stderr, strNewline);
  }
}

#endif

#if CONFIG_EXTRA & EXTRA_DOWNLOAD

static const char strCantDownload[] = N_("can't download to file");
static const tBrowserDocument* khmli_download_referrer = NULL;
static tResourceRequest* pm_request; /* for pmDownload */

static tBoolean rwd_cb_reqrem(const tRemainingWork* rw)
{ tResourceRequest* request = (tResourceRequest*) (rw->data1);
  request_remove(request); return(falsE);
}

static void schedule_request_removal(tResourceRequest* request)
{ /* Sometimes we can't remove a request directly because then a dhm-related
     call chain could try to access deallocated memory on unwinding. Thus: */
  tRemainingWork* rw = remaining_work_create(rwd_cb_reqrem);
  rw->data1 = request;
}

static void download_request_callback(void* _request,
  tDhmNotificationFlags flags)
{ tResourceRequest* request = (tResourceRequest*) _request;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf,"download_request_callback(%p, %d)\n",request,flags);
  debugmsg(debugstrbuf);
#endif
  if (flags & (dhmnfDataChange | dhmnfMetadataChange))
  { const tBoolean doing_auto = cond2boolean( (program_mode == pmDownload) &&
      (request == pm_request) );
    tResource* resource = request->resource;
    tResourceError re;
    if (resource != NULL)
    { re = resource->error;
      if (re != reFine)
      { if (doing_auto)
        { const char* text;
#if OPTION_TLS
          if ( (re == reTls) && (tls_errtext(resource, strbuf)) ) text=strbuf;
          else
#endif
          { handle_re: text = _(strResourceError[re]); }
          fatal_error(0, text);
        }
        req_rem: schedule_request_removal(request);
      }
      else if (resource->flags & rfFinal)
      { if (doing_auto) do_quit(); /* done */
        goto req_rem;
      }
    }
    else if ( (re = request->error) != reFine )
    { if (doing_auto) goto handle_re;
      else goto req_rem;
    }
  }
  else if (flags & dhmnfAttachery) /* a resource was attached to the request */
  { dhm_notification_setup(request->resource, download_request_callback,
      request, dhmnfDataChange | dhmnfMetadataChange, dhmnSet);
  }
}

static void download_queue(tResourceRequest* request, int fd)
/* some common stuff for slightly different kinds of downloads */
{ request->flags |= rrfDownload; make_fd_cloexec(fd);
  sinking_data_mcleanup(); sinking_data.download_fd = fd;
  sinking_data.flags |= sdfIsDownloadFdValid;
  sinking_data_shift(&(request->sinking_data));
  request_queue(request, rraEnforcedReload);
}

static void content_download_to_fd(const char* uri,
  const tBrowserDocument* referrer, int fd)
{ tPrrData prr_data;
  tResourceRequest* request;
  prr_setup(&prr_data, uri, prrfNone); prr_data.referrer = referrer;
  prepare_resource_request(&prr_data); request = prr_data.result;
  if (program_mode == pmDownload)
  { pm_request = request; print_automated_uri(request->uri_data->uri); }
  if (request->state == rrsError)
  { inform_about_error(0, _(strResourceError[request->error]));
    my_close(fd); goto out;
  }
  prr_data.result = NULL;
  dhm_notification_setup(request, download_request_callback, request,
    dhmnfDataChange | dhmnfMetadataChange | dhmnfAttachery, dhmnSet);
  download_queue(request, fd);
  out:
  prr_setdown(&prr_data);
}

static void __content_download(const char* filename, tBoolean may_overwrite)
{ const char* uri = khmli_download_uri;
  const tBrowserDocument* referrer = khmli_download_referrer;
  int fd, cflags = O_CREAT | O_TRUNC | O_WRONLY;
  struct stat statbuf;
  khmli_download_referrer = NULL;
  if (!may_overwrite) cflags |= O_EXCL;
  fd = my_create(filename, cflags, S_IRUSR | S_IWUSR);
  if (fd < 0)
  { failed: inform_about_error(errno, _(strCantDownload)); return; }
  if (my_fstat(fd, &statbuf) != 0)
  { int e; close_bad: e = errno; my_close(fd); errno = e; goto failed; }
  if (!S_ISREG(statbuf.st_mode))
  { errno = ( (S_ISDIR(statbuf.st_mode)) ? EISDIR : 0 );
    goto close_bad;
  }
  content_download_to_fd(uri, referrer, fd);
}

static __my_inline void content_download(tBoolean may_overwrite)
{ __content_download(khmli_filename, may_overwrite);
}

#endif /* #if CONFIG_EXTRA & EXTRA_DOWNLOAD */


/** Dumps */

#if CONFIG_EXTRA & EXTRA_DUMP

typedef struct
{ char* line;
  int fd;
} tContentDumpData;

static void content_dump_line(tRendererData* data)
{ const tContentDumpData* cdd = data->line_callback_data;
  const tRendererElement* element = data->element;
  char* line = cdd->line;
  size_t count = 0, cnt;
  while (element != NULL)
  { const size_t textcount = element->textcount;
    if (textcount > 0)
    { if (element->is_spacer)
      { for (cnt = 0; cnt < textcount; cnt++) line[count++] = ' '; }
      else
      { const tRendererText* text = element->text;
        for (cnt = 0; cnt < textcount; cnt++) line[count++] = text[cnt];
      }
    }
    element = element->next;
  }
  line[count++] = '\n';
  if (my_write(cdd->fd, line, count) != (ssize_t) count)
  { inform_about_error(errno, _("can't dump line"));
    data->flags |= rdfCallerDone;
  }
}

static void content_dump_to_fd(tBrowserDocument* document, int fd,
  const tDumpStyle* dump_style)
{ tRendererData data;
  tContentDumpData cdd;
  const size_t line_width = 80;
  const tDumpStyleFlags flags = ( (dump_style != NULL) ? (dump_style->flags) :
    dsfNone );
  my_memclr_var(data);
  data.line_callback = content_dump_line; data.line_callback_data = &cdd;
  data.ra = raLayout; data.document = document; data.line_width = line_width;
  data.flags = rdfAlignment;
  if (flags & dsfMustHtml) { do_html: data.flags |= rdfHtml; }
  else if (!(flags & dsfMustSource))
  { if (document->bddm == bddmHtml) goto do_html;
    if ( (document->cantent != NULL) && (document->cantent->kind == rckHtml) )
      goto do_html;
  }
  cdd.fd = fd; cdd.line = __memory_allocate(line_width + 5, mapString);
  renderer_run(&data); memory_deallocate(cdd.line);
}

static void content_dump(tBrowserDocument* document, const char* filename,
  tBoolean may_overwrite, const tDumpStyle* dump_style)
{ int fd, cflags = O_CREAT | O_TRUNC | O_WRONLY;
  struct stat statbuf;
  if (!may_overwrite) cflags |= O_EXCL;
  fd = my_create(filename, cflags, S_IRUSR | S_IWUSR);
  if (fd < 0)
  { failed: inform_about_error(errno, _("can't dump to file")); return; }
  if (my_fstat(fd, &statbuf) != 0)
  { int e; close_bad: e = errno; my_close(fd); errno = e; goto failed; }
  if (!S_ISREG(statbuf.st_mode))
  { errno = ( (S_ISDIR(statbuf.st_mode)) ? EISDIR : 0 );
    goto close_bad;
  }
  content_dump_to_fd(document, fd, dump_style); my_close(fd);
}

static void dump_request_callback(void* _request, tDhmNotificationFlags flags)
{ tResourceRequest* request = (tResourceRequest*) _request;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "dump_request_callback(%p, %d)\n", request, flags);
  debugmsg(debugstrbuf);
#endif
  if (flags & (dhmnfDataChange | dhmnfMetadataChange))
  { tResource* resource = request->resource;
    tResourceError re;
    if (resource != NULL)
    { re = resource->error;
      if (re != reFine)
      { const char* text;
#if OPTION_TLS
        if ( (re == reTls) && (tls_errtext(resource, strbuf)) ) text = strbuf;
        else
#endif
        { handle_re: text = _(strResourceError[re]); }
        fatal_error(0, text);
      }
      else if (resource->flags & rfFinal)
      { tBrowserDocument document;
        tDumpStyle ds;
        my_memclr_var(document); document.cantent = resource->cantent;
        my_memclr_var(ds); content_dump_to_fd(&document, fd_stdout, &ds);
        do_quit(); /* done */
      }
    }
    else if ( (re = request->error) != reFine ) goto handle_re;
  }
  else if (flags & dhmnfAttachery) /* a resource was attached to the request */
  { dhm_notification_setup(request->resource, dump_request_callback, request,
      dhmnfDataChange | dhmnfMetadataChange, dhmnSet);
  }
}

static one_caller void content_dump_auto(const char* uri)
{ tPrrData prr_data;
  tResourceRequest* request;
  prr_setup(&prr_data, uri, prrfNone); prepare_resource_request(&prr_data);
  request = prr_data.result; prr_data.result = NULL;
  print_automated_uri(request->uri_data->uri);
  if (request->state == rrsError)
    fatal_error(0, _(strResourceError[request->error]));
  dhm_notification_setup(request, dump_request_callback, request,
    dhmnfDataChange | dhmnfMetadataChange | dhmnfAttachery, dhmnSet);
  request_queue(request, rraEnforcedReload); prr_setdown(&prr_data);
}

#endif /* #if CONFIG_EXTRA & EXTRA_DUMP */


/** Sessions */

#if CONFIG_SESSIONS

static const char strSessionMarker[] = "|#|x:";
#define smlp (3) /* session marker letter position */
#define psm(buf, ch) /* prepare session marker */ \
  do { strcpy((buf), strSessionMarker); *((buf) + smlp) = (ch); } while (0)
#define bdp(buf) ((buf) + strlen(strSessionMarker)) /* buffer data position */

/* CHECKME: maybe this shouldn't be global... */
static char flagsbuf[50], linenumbuf[50], widthbuf[50], winbuf[50],
  htmltitlemarker[10], urimarker[10];

static void session_save(const char* filename, tBoolean may_overwrite)
{ int fd, cflags = O_CREAT | O_TRUNC | O_WRONLY;
  struct stat statbuf;
  tWindow* window;

  /* create the file */
  if (!may_overwrite) cflags |= O_EXCL;
  fd = my_create(filename, cflags, S_IRUSR | S_IWUSR);
  if (fd < 0) { failed: show_message_osfailure(errno, NULL); return; }
  if (my_fstat(fd, &statbuf) != 0)
  { int e; close_bad: e = errno; my_close(fd); errno = e; goto failed; }
  if (!S_ISREG(statbuf.st_mode))
  { errno = ( (S_ISDIR(statbuf.st_mode)) ? EISDIR : 0 );
    goto close_bad;
  }

  /* save the session */
  my_write_str(fd,
    _("# Session file for retawq (http://retawq.sourceforge.net/)\n"));
  psm(flagsbuf, 'f'); psm(linenumbuf, 'l'); psm(htmltitlemarker, 'h');
  psm(urimarker, 'u'); psm(widthbuf, 'w'); *(bdp(widthbuf)) = *strTG;
  winbuf[0] = 'W'; winbuf[1] = ':';
  window = windowlisthead;
  while (window != NULL)
  { void (*func)(tWindow*, int) = window->spec->save_session;
    if (func != NULL) (func)(window, fd);
    window = window->next;
  }

  /* finish */
  my_close(fd);
}

static tLinenumber session_linenum;
static tWindow* session_window;
#if TGC_IS_CURSES
static tWindow* session_viswin[2];
#endif
#define SESSIONPART_MAXNUM (10)
static const char* session_part[SESSIONPART_MAXNUM];
static unsigned char session_numparts;

static void session_resume_split(char* line)
{ char* temp;
  session_numparts = 0;
  loop:
  temp = my_strstr(line, "|#|");
  if (temp != NULL)
  { char ch = temp[3];
    if (!my_islower(ch)) { incandloop: line = temp + 1; goto loop; }
    if (temp[4] != ':') goto incandloop;
    *temp = '\0';
    session_part[session_numparts++] = temp + 3;
    if (session_numparts < SESSIONPART_MAXNUM)
    { line = temp + 5; goto loop; } /* look for more */
  }
}

static __my_inline void session_resume_finish_window(void)
{
#if 0 /* FIXME! */
  if ( (session_window != NULL) && (session_currview != NULL) )
    session_window->current_view = session_currview;
#endif
}

static void session_resume_window(void)
{ session_resume_finish_window();
  session_window = wk_browser_create();
#if 0 /* FIXME! */
  session_currview = NULL;
#endif
}

static void session_resume_line(const char* line, const char* limit)
{ size_t len = limit - line;
  char buf[STRBUF_SIZE], *temp;
  unsigned char count;
  session_linenum++;
  if (len <= 0) return; /* empty line */
  if (len > STRBUF_SIZE - 5) return; /* very long line, can't be serious */
  my_memcpy(buf, line, len); buf[len] = '\0'; /* (for simplicity) */
  if (buf[len - 1] == '\r') /* (e.g. manually written file?) */
    buf[len - 1] = '\0';
  debugmsg("session_resume_line(): *"); debugmsg(buf); debugmsg("*\n");
  temp = buf;
  while ( (*temp == ' ') || (*temp == '\t') ) temp++; /* skip whitespace */
  if (*temp == '#') return; /* comment line */
  if (!strncmp(temp, "W:", 2))
  { session_resume_window();
#if TGC_IS_CURSES
    session_resume_split(temp + 2);
    for (count = 0; count < session_numparts; count++)
    { const char *str = session_part[count], *data = str + 2;
      int num;
      debugmsg("session part W: *"); debugmsg(str); debugmsg("*\n");
      if (*str != 'v') continue; /* unknown part */
      if (*data == '\0') continue; /* empty part */
      my_atoi(data, &num, &data, 9);
      if ( (num >= 0) && (num <= 1) && (*data == '\0') )
        session_viswin[num] = session_window;
    }
#endif
  }
  else if (!strncmp(temp, "U:", 2))
  { const char *uri, *flagstr, *html_title;
    session_resume_split(temp + 2);
    uri = flagstr = html_title = NULL;
    for (count = 0; count < session_numparts; count++)
    { const char *str = session_part[count], *data = str + 2;
      debugmsg("session part U: *"); debugmsg(str); debugmsg("*\n");
      if (*data == '\0') continue; /* empty part */
      switch (*str)
      { case 'u': uri = data; break;
        case 'f': flagstr = data; break;
        case 'h': html_title = data; break;
        /* "default": unknown part; IMPLEMENTME: print warning/error? */
      }
    }
    if (uri != NULL) /* got useful information */
    { /* tResourceRequest* request; */
      tPrrFlags prrf = prrfRedrawOne;
      /* tBoolean is_currview = falsE; */
      if (flagstr != NULL)
      { const char* f = flagstr;
        char ch;
        while ( (ch = *f++) != '\0' )
        { if (ch == 's') prrf |= prrfSource;
          else if (ch == 'h') prrf |= prrfHtml;
          /* else if (ch == 'c') is_currview = truE; */
        }
      }
      if (session_window == NULL) session_resume_window();
      wk_browser_prr(session_window, uri, prrf, NULL);
#if 0 /* FIXME! */
      if (request != NULL)
      { if (is_currview) session_currview = view;
        if (html_title != NULL)
          my_strdedup(document->minor_html_title, html_title);
      }
#endif
    }
  }
  /* "else": IMPLEMENTME: bad line, print a message, stop resumption! */
}

static void session_resume(const char* filename)
{ size_t size;
  void* filebuf;
  char *buf, *end, *temp;
  if (*filename == '\0') filename = config.session_default;
  debugmsg("session_resume(): *"); debugmsg(filename); debugmsg("*\n");
  switch (my_mmap_file_readonly(filename, &filebuf, &size))
  { case 0: show_message_osfailure(errno,NULL); return; /*@notreached@*/ break;
    case 1: goto out; /*@notreached@*/ break;
  }
  buf = (char*) filebuf; end = buf + size - 1;
  session_window = NULL;
#if TGC_IS_CURSES
  session_viswin[0] = session_viswin[1] = NULL;
#endif
#if 0 /* FIXME! */
  session_currview = NULL;
#endif
  session_linenum = 0;
  temp = buf;
  while (temp <= end)
  { if (*temp == '\n') { session_resume_line(buf, temp); buf = temp + 1; }
    temp++;
  }
  my_munmap(filebuf, size);
  session_resume_finish_window();
#if TGC_IS_CURSES
  { tWindow *viswin0 = session_viswin[0], *viswin1 = session_viswin[1];
    if ( (viswin0 == NULL) && (viswin1 != NULL) )
    { viswin0 = viswin1; viswin1 = NULL; }
    if (viswin0 != NULL) visible_window_x[0] = viswin0;
    if ( (viswin1 != NULL) && (viswin1 != visible_window_x[0]) )
      visible_window_x[1] = viswin1;
  }
#endif
  window_redraw_all();

  out:
  show_message(_(strDone), falsE);
}

#endif /* #if CONFIG_SESSIONS */


/** Contextual menus (that's a misnomer nowadays; should be something like
   "general/basic menu handling", but I don't want to change the nice "cm_"
   prefix all over the place) */

#if CONFIG_MENUS

#if TGC_IS_CURSES

#if CONFIG_MENUS & MENUS_BAR
static tBoolean doing_mbar = falsE;
#endif

static void my_hline(short x1, short x2, short y)
/* draws a horizontal line */
{ (void) move(y, x1);
  while (x1 <= x2) { (void) addch(__MY_HLINE); x1++; }
}

static void my_vline(short y1, short y2, short x)
/* draws a vertical line */
{ while (y1 <= y2) { (void) mvaddch(y1, x, __MY_VLINE); y1++; }
}

static void draw_box(short x1, short y1, short x2, short y2)
{ my_set_color(cpnBlue); /* attron(A_BOLD); */
  my_hline(x1 + 1, x2 - 1, y1); my_hline(x1 + 1, x2 - 1, y2);
  my_vline(y1 + 1, y2 - 1, x1); my_vline(y1 + 1, y2 - 1, x2);
  (void) mvaddch(y1, x1, __MY_UL); (void) mvaddch(y1, x2, __MY_UR);
  (void) mvaddch(y2, x1, __MY_LL); (void) mvaddch(y2, x2, __MY_LR);
  /* attroff(A_BOLD); */ my_set_color(cpnDefault);
}

typedef void* tCmCallbackData;
typedef void (*tCmCallbackFunction)(tCmCallbackData);
typedef signed int tCmNumber; /* ("signed" for simplicity only) */

typedef struct
{ const char* render;
  tCmCallbackFunction function;
  tCmCallbackData data;
} tCmEntry;

static tCmEntry* cm_info;
static tCmNumber cm_num, cm_maxnum, cm_current,
  cm_topmost, cm_onscreen_num; /* what's displayed on the screen */
static tCoordinate cm_x1, cm_y1, cm_x2, cm_y2;

static char* cm_select_bitfield;
static tBoolean cm_select_is_multiple;
static tHtmlOptionNumber cm_select_old; /* formerly selected option */

static void cm_init(void)
{ cm_info = NULL;
  cm_num = cm_maxnum = cm_current = cm_topmost = 0;
}

static void __cm_add(const char* str, tCmCallbackFunction func,
  tCmCallbackData data)
{ if (cm_num >= cm_maxnum)
  { cm_maxnum += 20;
    cm_info = memory_reallocate(cm_info, cm_maxnum * sizeof(tCmEntry),
      mapOther);
  }
  cm_info[cm_num].render = my_strdup(str);
  cm_info[cm_num].function = func; cm_info[cm_num].data = data;
  cm_num++;
}

#define cm_add(str, func, val) \
  __cm_add(unconstify_or_(str), func, (tCmCallbackData) MY_INT_TO_POINTER(val))

static my_inline void cm_add_separator(void)
{ if (cm_num > 0) __cm_add(strMinus, NULL, NULL);
}

static void cm_activate(tBoolean active)
/* indicates on screen what's currently selected ("menu cursor") */
{ short x, x0 = cm_x1 + 1, y = (cm_y1 + 1) + (cm_current - cm_topmost);
  (void) move(y, x0);
  for (x = x0; x < cm_x2; x++)
  { chtype ch = inch();
    if (active) ch |= A_REVERSE;
    else ch &= ~A_REVERSE;
    (void) addch(ch);
  }
  (void) move(LINES - 1, 0);
}

static void __cm_draw(void)
/* draws the menu texts */
{ tCmNumber num;
  short x = cm_x1 + 1, width = cm_x2 - cm_x1 - 1;
  for (num = 0; num < cm_onscreen_num; num++)
  { short y = cm_y1 + num + 1;
    const char* str = cm_info[cm_topmost + num].render;
    if ( (*str == '-') && (str[1] == '\0') ) my_hline(x, x + width - 1, y);
    else
    { short count = width - strlen(str);
      /*(void)*/ mvaddnstr(y, x, str, width);
      while (count-- > 0) (void) addch(' ');
    }
  }
}

static one_caller tBoolean cm_draw(short x, short y, const char* title)
/* calculates and draws the menu rectangle; returns whether it worked */
{ short titlelen = ( (title != NULL) ? strlen(title) : 0 ),
    titlewidth = ( (titlelen > 0) ? (titlelen + 2) : 0 ),
    width = titlewidth, cols = COLS, lines = LINES, maxwidth = cols - 10;
  tCmNumber num, onscreen_maxnum = lines - 4;
  if ( (maxwidth <= 5) || (onscreen_maxnum <= 2) ) return(falsE);
  for (num = 0; num < cm_num; num++)
  { short len = strlen(cm_info[num].render);
    if (width < len) width = len;
  }
  if (width > maxwidth) width = maxwidth;
  cm_onscreen_num = MIN(cm_num, onscreen_maxnum);

  if ( (x < 0) || (x >= cols) ) x = 0;
  if ( (y < 0) || (y >= lines) ) y = 0;

  if (x + width + 1 >= cols - 1) x = cols - width - 3;
  if (y + cm_onscreen_num + 1 >= lines - 2) y = lines - cm_onscreen_num - 4;

  cm_x1 = x; cm_y1 = y; cm_x2 = x + width + 1; cm_y2 = y + cm_onscreen_num + 1;
  draw_box(cm_x1, cm_y1, cm_x2, cm_y2);
  if (titlelen > 0)
  { short maxlen = cm_x2 - cm_x1 - 3, len = MIN(maxlen, titlelen),
      offset = (maxlen - len) / 2;
    if (offset < 0) offset = 0;
    my_set_color(cpnBlue);
    /*(void)*/ mvaddnstr(cm_y1, cm_x1 + 2 + offset, title, len);
    my_set_color(cpnDefault);
  }
  __cm_draw(); cm_activate(truE);
  return(truE);
}

static void cm_start(short x, short y, const char* title, const char* errstr)
{ if (cm_num > 0)
  { if (cm_draw(x, y, title)) key_handling_mode = khmMenu;
    else terminal_too_small();
  }
  else
  { /* The menu would be empty, so we won't show it. */
    if (errstr != NULL) show_message(errstr, truE);
  }
}

static void cm_handle_command_code(tCmCallbackData data)
{ (void) handle_command_code((tProgramCommandCode) MY_POINTER_TO_INT(data));
}

#if CONFIG_MENUS & MENUS_CONTEXT

static void cm_setup_contextual(short x, short y, tActiveElementNumber _ae)
/* sets up a contextual menu with ae-related entries (unless <_ae> is invalid)
   and some general entries */
{ tWindow* window = current_window_x; /* FIXME! */
  void (*func)(tWindow*, short*, short*, tActiveElementNumber) =
    window->spec->setup_cm;
  cm_init();
  if (func != NULL) (func)(window, &x, &y, _ae);
  cm_add(strUcClose, cm_handle_command_code, pccWindowClose);
  cm_add(strUcQuit, cm_handle_command_code, pccQuit);
  cm_start(x, y, _(strContext), NULL);
}

#endif /* #if CONFIG_MENUS & MENUS_CONTEXT */

#if CONFIG_MENUS & MENUS_HTML

static void cm_handle_select_tag(tCmCallbackData data)
/* callback function for menus for the HTML <select> tag */
{ tHtmlOptionNumber num = (tHtmlOptionNumber) MY_POINTER_TO_INT(data);
  if (cm_select_is_multiple) my_bit_flip(cm_select_bitfield, num);
  else
  { my_bit_clear(cm_select_bitfield, cm_select_old);
    my_bit_set(cm_select_bitfield, num);
  }
  window_redraw(current_window_x);
}

static void cm_setup_select_tag(const tActiveElementBase* aebase,
  const tBrowserDocument* document, tActiveElementNumber _ae)
/* sets up a menu for the HTML <select> tag */
{ tActiveElement* aes = document->active_element;
  const tHtmlOption* o = (const tHtmlOption*) (aebase[_ae].render);
  const tActiveElementCoordinates* aec = find_visible_aec(document, _ae);
  tHtmlOptionNumber num = 0;
  short x, y;
  cm_init();
  cm_select_is_multiple = cond2boolean(aebase[_ae].flags & aefMultiple);
  cm_select_bitfield = aes[_ae].current_text;
  cm_select_old = 0;
  while (o != NULL)
  { tBoolean is_selected = cond2boolean(my_bit_test(cm_select_bitfield, num));
    char ch, *spfbuf;
    if (is_selected) ch = '+';
    else if (o->flags & hofDisabled) ch = '-'; /* CHECKME! */
    else ch = ' ';
    my_spf(strbuf, STRBUF_SIZE, &spfbuf, "%c%s", ch, o->render);
    __cm_add(spfbuf, cm_handle_select_tag,
      (tCmCallbackData) MY_INT_TO_POINTER(num));
    my_spf_cleanup(strbuf, spfbuf);
    if (is_selected) cm_select_old = num;
    o = o->next; num++;
  }
  if ( (aec != NULL) && (document_line2row(document, aec->y, &y)) )
  { x = aec->x1; y++; }
  else { x = y = 0; }
  cm_start(x, y, _("Options"), _(strSelectionEmpty));
}

#endif /* #if CONFIG_MENUS & MENUS_HTML */

#if CONFIG_MENUS & MENUS_UHIST

static void cm_handle_uri_history(tCmCallbackData data)
{ tWindow* window = require_browser_window();
  unsigned short i = (unsigned short) MY_POINTER_TO_INT(data);
  wk_browser_prr(window, uri_history[i], prrfRedrawOne, NULL);
}

static void cm_setup_uri_history(void)
{ unsigned short count = 0, i = uri_history_index;
  cm_init();
  while (count < URI_HISTORY_LEN)
  { const char* text;
    if (i > 0) i--;
    else i = URI_HISTORY_LEN - 1;
    text = uri_history[i];
    if (text == NULL) break; /* reached the "end" of the list */
    __cm_add(text, cm_handle_uri_history,
      (tCmCallbackData) MY_INT_TO_POINTER(i));
    count++;
  }
  cm_start(0, 0, _("History"), _("No URL history available yet!"));
}

#endif /* #if CONFIG_MENUS & MENUS_UHIST */

#if CONFIG_MENUS & MENUS_WLIST

static void cm_handle_window_list(tCmCallbackData data)
{ tWindow* window = (tWindow*) data;
  if (current_window_x != window) /* CHECKME! */
  { current_window_x = window;
    window_redraw(current_window_x);
  }
}

static void cm_setup_window_list(void)
{ tWindow* window = windowlisthead;
  cm_init();
  while (window != NULL)
  { const char* (*func)(tWindow*) = window->spec->get_menu_entry;
    const char* text = ( (func != NULL) ? ((func)(window)) : NULL );
    if (text != NULL)
      __cm_add(text, cm_handle_window_list, (tCmCallbackData) window);
    window = window->next;
  }
  cm_start(0, 0, _("Window List"), _("No open window!"));
}

#endif /* #if CONFIG_MENUS & MENUS_WLIST */

static void cm_remove(void)
/* This is only for "internal" use by cm_....() functions; external users
   should call cm_cancel() instead. */
{ tCmNumber num;
  for (num = 0; num < cm_num; num++) __dealloc(cm_info[num].render);
  memory_deallocate(cm_info);
#if CONFIG_MENUS & MENUS_BAR
  doing_mbar = falsE;
#endif
  key_handling_mode = khmCommand; window_redraw_all();
}

static void cm_change_current(tCmNumber num)
/* moves the "menu cursor" */
{ tCmNumber old_topmost;
  if (cm_current == num) return; /* no "real" change, nothing to do */
  cm_activate(falsE); /* deactivate old */
  cm_current = num;

  old_topmost = cm_topmost;
  if (cm_topmost > num) cm_topmost = num;
  else if (cm_topmost <= num - cm_onscreen_num)
    cm_topmost = num - cm_onscreen_num + 1;
  if (cm_topmost != old_topmost) __cm_draw(); /* need to "scroll" */
  cm_activate(truE);
}

static void cm_line_up(void)
{ if (cm_current > 0) cm_change_current(cm_current - 1);
  else if (cm_num > 0) cm_change_current(cm_num - 1);
}

static void cm_line_down(void)
{ if (cm_current < cm_num - 1) cm_change_current(cm_current + 1);
  else cm_change_current(0);
}

static void cm_page_up(void)
{ tCmNumber num = cm_current - (cm_y2 - cm_y1 - 1);
  if (num < 0) num = 0;
  cm_change_current(num);
}

static void cm_page_down(void)
{ tCmNumber num = cm_current + (cm_y2 - cm_y1 - 1);
  if (num > cm_num - 1) num = cm_num - 1;
  cm_change_current(num);
}

static __my_inline void cm_cancel(void)
/* The user cancelled the menu explicitly or resized the terminal. */
{ cm_remove();
}

static void cm_apply(void)
/* The user pressed some KEY_ENTER variant. */
{ tCmCallbackFunction function = cm_info[cm_current].function;
  if (function != NULL)
  { tCmCallbackData data = cm_info[cm_current].data;
    cm_remove();
    (function)(data);
  }
}

#if CONFIG_DO_TEXTMODEMOUSE
static void cm_handle_mouseclick(tCoordinate x, tCoordinate y)
{ if ( (x > cm_x1) && (x < cm_x2) && (y > cm_y1) && (y < cm_y2) )
  { /* The user clicked inside the menu box. */
    tCmNumber num = ((tCmNumber) (y - cm_y1 - 1)) + cm_topmost;
    cm_change_current(num);
    cm_apply();
  }
  else cm_remove();
}
#endif

#endif /* #if TGC_IS_CURSES */

#endif /* #if CONFIG_MENUS */


/** Line input II */

#if TGC_IS_GRAPHICS

static tBoolean current_window_exists(void)
/* returns whether current_window still exists; in graphics mode, it's possible
   that a user starts a dialog which is related to a document window, then
   closes the window and finally clicks the "action" button of the dialog, so
   the callback function would refer to a non-existing window. This problem
   could be solved by using _modal_ dialogs, but I hate modality because it
   takes away flexibility from users - often unnecessarily. */
{ if (current_window != NULL)
  { const tWindow* w = windowlisthead;
    while (w != NULL)
    { if (w == current_window) return(truE);
      w = w->next;
    }
  }
  return(falsE);
}

/* "require current_window" */
#define __rcw(solution) if (!current_window_exists()) solution;
#define rcw __rcw(return) /* the simple solution */
#define rcwc __rcw(current_window = wk_browser_create()) /* sometimes nicer */

#else

#define __rcw(solution) /* nothing; current_window _is_ valid  */
#define rcw
#define rcwc

#endif

#if TGC_IS_GRAPHICS

#if 0
typedef struct
{ tWindow* window;
  GtkEntry* entry;
  tEditableLineInputGoal elig;
} tLigData;

static one_caller __sallocator tLigData* __callocator
  lig_data_create(tWindow* window, GtkWidget* _entry,
  tEditableLineInputGoal elig)
{ tLigData* retval = (tLigData*) __memory_allocate(sizeof(tLigData),
    mapLineInput);
  GtkEntry* entry = ( (_entry != NULL) ? GTK_ENTRY(_entry) : NULL );
  retval->window = window; retval->entry = entry; retval->elig = elig;
  return(retval);
}

static void graphics_handle_lig_action(tGraphicsWidget* w __cunused,
  gpointer _data)
{ const tLigData* data = (tLigData*) _data;
  tEditableLineInputGoal elig = data->elig;
  current_window = data->window;
  if (is_confirmation_lig(_lig)) handle_lig_confirmation(_lig);
  else
  { GtkEntry* entry = data->entry;
    const char* text = ( (entry != NULL) ? gtk_entry_get_text(entry) : NULL );
    handle_lig_other(_lig, text);
  }
  current_window = NULL;
}
#endif

static void graphics_window_destroy(tGraphicsWidget* w __cunused,
  gpointer data)
/* low-level callback for window destruction triggered by buttons; for
   _document_ windows, use window_hide()/_remove() instead! */
{ gtk_widget_destroy((tGraphicsWidget*) data); /* (yes!) */
}

static GtkWidget* dialog_create_button(GtkDialog* dialog, const char* text)
{ GtkWidget* button = gtk_button_new_with_label(text);
  pack_box(dialog->action_area, button);
  show_widget(button);
  return(button);
}

#define connect_button(b, h, d) connect_widget(b, strGtkClicked, h, d)

#if 0
static void start_line_input_FAIL(tLineInputGoal _lig, const char* msg,
  const char* initstr, const char* wtitle, const char* wact,
  const char* wcancel)
{ GtkDialog* dialog = GTK_DIALOG(gtk_dialog_new());
  GtkWidget *button, *inputbox;
  if (wtitle != NULL) gtk_window_set_title(GTK_WINDOW(dialog), wtitle);
  if (is_confirmation_lig(_lig))
  { GtkWidget* label = gtk_label_new(msg);
    show_widget(label); pack_box(dialog->vbox, label);
    inputbox = NULL;
  }
  else
  { GtkWidget *hbox = gtk_hbox_new(FALSE, 0), *label = gtk_label_new(msg);
    inputbox = gtk_entry_new();
    if (initstr != NULL) gtk_entry_set_text(inputbox, (gchar*) initstr);
    show_widget(hbox); show_widget(label); show_widget(inputbox);
    pack_box(hbox, label); pack_box(hbox, inputbox);
    pack_box(dialog->vbox, hbox);
  }

  button = dialog_create_button(dialog, wact);
  connect_button(button, graphics_handle_lig_action,
    lig_data_create(current_window, inputbox, _lig));
  connect_button(button, graphics_window_destroy, dialog);

  button = dialog_create_button(dialog, wcancel);
  connect_button(button, graphics_window_destroy, dialog);

  show_widget(dialog);
}
#endif

static void __line_input_start(/*@notnull@*/ const char* msg,
  const char* initstr, tLineInputCallback func, void* data,
  tLineInputAreaFlags liaf)
{ /* FIXME! */
}

typedef struct
{ GtkFileSelection* sel;
  tWindow* window;
} tFileselData;

static one_caller __sallocator tFileselData* __callocator
  filesel_data_create(GtkWidget* sel, tWindow* window)
{ tFileselData* retval = (tFileselData*) __memory_allocate(sizeof(*retval),
    mapGtk);
  retval->sel = GTK_FILE_SELECTION(sel); retval->window = window;
  return(retval);
}

static void filesel_handle(GtkFileSelection* widget __cunused, gpointer _data)
{ const tFileselData* data = (tFileselData*) _data;
  const char* file = gtk_file_selection_get_filename(data->sel);
  if ( (file != NULL) && (*file != '\0') )
    prepare_resource_request(data->window, file, prrfRedrawOne, NULL, NULL);
}

#else /* #if TGC_IS_GRAPHICS */

static void line_input_redraw(void)
{ tLineInputAreaIndex count;
  for (count = 0; count < lid.num_areas; count++)
  { tLineInputArea* lia = &(lid.area[count]);
    const tLineInputAreaFlags liaf = lia->flags;
    const short len = lia->len, first = lia->first, usable = lia->usable,
      row = lia->row, colmin = lia->colmin;
    short cnt = len - first;
    const char* src = lid.area[count].text + first;
    if (cnt > usable) cnt = usable;
    (void) move(row, colmin);
    if (cnt > 0)
    { if (liaf & liafDisguising) { while (cnt-- > 0) (void) addch('*'); }
      else (void) addnstr(src, cnt);
    }
    (void) clrtoeol(); /* IMPROVEME! */
  }
  (void) move(lid_area(row), lid_area(colcurr));
}

static void line_input_layout(void)
{ tLineInputAreaIndex numa = lid.num_areas, count;
  const short line = LINES - 1, usable = COLS - 1;
  short col = 0;
  for (count = 0; count < numa; count++)
  { lid.area[count].row = line;
    if (lid.area[count].flags & liafEditable)
      lid.area[count].flags |= liafFocusable;
  }

  if (usable < 5 * numa)
  { key_handling_mode = khmCommand; terminal_too_small();
    (lid.callback)(lid.callback_data, liekFail, 0);
    return;
  }

  for (count = 0; count < numa; count++)
  { short len = lid.area[count].len, maxlen = usable - col - 5*(numa-count-1),
      used;
    if (len > maxlen) len = maxlen;
    used = ((lid.area[count].flags & liafEditable) ? maxlen : len);
    lid.area[count].colmin = lid.area[count].colcurr = col;
    lid.area[count].colmax = col + used; lid.area[count].usable = used;
    col += used;
  }

  line_input_redraw();
}

static void line_input_resize(void)
{ short count;
  for (count = 0; count < lid.num_areas; count++)
  { lid.area[count].pos = 0; lid.area[count].first = 0; } /* KISS */
  line_input_layout();
}

static one_caller void __line_input_to_eos(void)
/* ("eos": "end of string", as usual) */
{ if (lid_area(len) >= lid_area(usable))
  { lid_area(first) = lid_area(len) - lid_area(usable);
    lid_area(colcurr) = lid_area(colmax);
  }
  else
  { lid_area(first) = 0;
    lid_area(colcurr) = lid_area(colmin) + lid_area(len);
  }
}

static void line_input_to_eos(void)
{ if (lid_area(pos) < lid_area(len))
  { lid_area(pos) = lid_area(len); __line_input_to_eos(); line_input_redraw();}
}

static one_caller tMbsIndex do_lookup_lineinput_key(tKey key)
{ if (keymap_lineinput_keys_num <= 0) return(INVALID_INDEX); /*"can't happen"*/
  my_binary_search(0, keymap_lineinput_keys_num - 1,
    my_numcmp(key, keymap_lineinput_keys[idx].key), return(idx))
}

static tLineInputActionCode lookup_lineinput_key(tKey key)
{ tMbsIndex idx = do_lookup_lineinput_key(key);
  return( (idx < 0) ? liacUnknown : keymap_lineinput_keys[idx].liac );
}

#if CONFIG_DO_TEXTMODEMOUSE
static void mouse_flip(unsigned char what)
{ static tBoolean is_on = cond2boolean(!OFWAX);
  tBoolean new_state;
  switch (what)
  { default: /* "can't happen" */ /*@fallthrough@*/
    case 0: new_state = cond2boolean(!is_on); break;
    case 1: new_state = falsE; break;
    case 2: new_state = truE; break;
  }
  if (is_on != new_state) /* must do something */
  { is_on = new_state;
    mousemask(is_on ? TEXTMODEMOUSE_MASK : 0, NULL);
    show_message(is_on ? _("mouse on") : _("mouse off"), falsE);
  }
}
#else
static const char strFtmm[] = N_("Ftext-mode mouse");
#define mouse_flip(dummy) fwdact(_(strFtmm))
#endif

static one_caller void line_input_handle_key(tKey key)
{ tLineInputActionCode liac = lookup_lineinput_key(key);
  switch (liac)
  { case liacCancel:
      (lid.callback)(lid.callback_data, liekCancel, 0);
      return; /*@notreached@*/ break;
    case liacAreaSwitch:
      if (lid.num_areas == 2) { lid.curr = 1 - lid.curr; line_input_redraw(); }
      return; /*@notreached@*/ break;
    case liacMouseFlip: case liacMouseOff: case liacMouseOn:
      mouse_flip(liac - liacMouseFlip); return; /*@notreached@*/ break;
  }

  if (lid_area(flags) & liafEditable) /* the callback will wanna do the work */
  { do_call: (lid.callback)(lid.callback_data, liekKey, key); return; }

  switch (liac)
  { case liacToLeft:
      if (lid_area(first) > 0)
      { lid_area(first)--; do_redraw: line_input_redraw(); }
      break;
    case liacToRight:
      if (lid_area(first) + lid_area(usable) < lid_area(len))
      { lid_area(first)++; goto do_redraw; }
      break;
    case liacToStart:
      if (lid_area(first) > 0) { lid_area(first) = 0; goto do_redraw; }
      break;
    case liacToEnd:
      if (lid_area(first) + lid_area(usable) < lid_area(len))
      { lid_area(first) = lid_area(len) - lid_area(usable); goto do_redraw; }
      break;
    default:
      if (lid.curr == lid.num_areas - 1) goto do_call;
      break;
  }
}

static void lid_prepare_text(const char* text, tLineInputAreaIndex idx)
{ size_t len = strlen(text);
  if (len > 1024) len = 1024; /* can't be serious */
  if (len > 0)
  { char* t = __memory_allocate(len,mapString); /* not storing trailing '\0' */
    my_memcpy(t, text, len); lid.area[idx].text = t;
  }
  lid.area[idx].len = lid.area[idx].maxlen = len;
}

#define line_input_start(a, b, c, d, e, f, g, h) __line_input_start(a,b,c,d,e)

static void __line_input_start(/*@notnull@*/ const char* msg,
  const char* initstr, tLineInputCallback func, void* data,
  tLineInputAreaFlags liaf)
{ tLineInputAreaIndex count = 0, curr;
  my_memclr_var(lid); lid.callback = func; lid.callback_data = data;
  lid_prepare_text(msg, count++);
  if (liaf & liafEditable)
  { if (initstr != NULL) lid_prepare_text(initstr, count);
    count++;
  }
  curr = count - 1; lid.area[curr].flags = liaf; lid.curr = curr;
  lid.num_areas = count; key_handling_mode = khmLineInput;
  line_input_layout();
  if ( (key_handling_mode == khmLineInput) && (liaf & liafEditable) )
    line_input_to_eos(); /* IMPROVEME! */
}

#endif /* #if TGC_IS_GRAPHICS */

static void line_input_ensure_room(void)
{ if (lid_area(len) >= lid_area(maxlen))
  { lid_area(maxlen) += 100;
    lid_area(text) = memory_reallocate(lid_area(text), lid_area(maxlen),
      mapString);
  }
}

static void line_input_paste_char(char ch)
{ char* text;
  short count;
  if (lid_area(len) >= 1234) return; /* text becomes "too long" */
  /* update the string */
  line_input_ensure_room();
  text = lid_area(text);
  for (count = lid_area(len); count > lid_area(pos); count--)
    text[count] = text[count - 1];
  text[lid_area(pos)++] = ch; lid_area(len)++;
  /* update the screen */
  if (lid_area(colcurr) < lid_area(colmax)) lid_area(colcurr)++;
  else lid_area(first)++;
  line_input_redraw();
}

static void line_input_finish(void)
{
#if TGC_IS_CURSES
  key_handling_mode = khmCommand;
#endif
  redraw_message_line();
  __dealloc(lid.area[0].text); __dealloc(lid.area[1].text);
}

static const char* uri2filename(const tUriData* u)
/* returns an allocated filename for the URI or NULL */
{ const char *retval = NULL, *temp;
  const tResourceProtocol rp = u->rp;
  if ( (is_rp_nice(rp)) && (rp_data[rp].flags & rpfIsPathHierarchical) &&
       ( (temp = u->path) != NULL ) )
  { const char* slash = my_strrchr(temp, chDirsep);
    if (slash == NULL) retval = temp;
    else
    { slash++;
      if (*slash != '\0') retval = slash;
      /* "else": it's probably a directory */
    }
  }
  if (retval != NULL)
  { char ch, *ptr = __memory_allocate(strlen(retval) + 1, mapString),
      *ptr2 = ptr;
    while ( (ch = *retval++) != '\0' )
    { if ( (!my_isalnum(ch)) && (ch != '.') && (ch != '-') ) ch = '_';
      *ptr++ = ch;
    }
    *ptr = '\0'; retval = ptr2;
  }
  return(retval);
}

static const char* uristr2filename(const char* str)
{ tUriData* u = uri_parse(str, NULL, NULL, NULL, 0);
  const char* filename = uri2filename(u);
  uri_put(u);
  return(filename);
}

#if CONFIG_EXTRA & EXTRA_DOWNLOAD
static void ask_download_to(void)
{ const char* filename = uristr2filename(khmli_download_uri);
  line_input_estart(eligDownloadFilename, _("Download to: "), filename, NULL,
    _(strUcDownload), _(strUcDownload), _(strCancel));
  __dealloc(filename);
}
#endif

static void start_save_as(/*@notnull@*/ const tBrowserDocument* document,
  const char* filename, tBoolean may_overwrite)
/* IMPROVEME: show better messages! */
{ const tResourceRequest* request = document->request;
  tResource* resource = ( (request != NULL) ? request->resource : NULL );
  tCantent* cantent = document->cantent;
  struct stat statbuf;
  int fd, cflags;
  if (cantent == NULL)
  { errno = 0; bad: show_message_osfailure(errno, _("Can't save")); return; }
  cflags = O_CREAT | O_TRUNC | O_WRONLY;
  if (!may_overwrite) cflags |= O_EXCL;
  fd = my_create(filename, cflags, S_IRUSR | S_IWUSR);
  if (fd < 0) goto bad;
  if (my_fstat(fd, &statbuf) != 0)
  { int e; close_bad: e = errno; my_close(fd); errno = e; goto bad; }
  if (!S_ISREG(statbuf.st_mode))
  { errno = ( (S_ISDIR(statbuf.st_mode)) ? EISDIR : 0 );
    goto close_bad;
  }
  resource_start_saving(resource, cantent, fd);
  show_message(_("Saving"), falsE);
}

static void interpret_as_html(void)
{ tBrowserDocument* document = current_document_x;
  if (document != NULL)
  { document->bddm = bddmHtml; document_display(document, wrtRedraw); }
}

static one_caller void reload_document(void)
/* CHECKME: that's unclean! */
{ tWindow* window = current_window_x;
  if (is_browser_window(window)) wk_browser_reload(window);
}

static one_caller void ask_yesno_act(tConfirmationGoal cg)
{ tBrowserDocument* document;
  switch (cg)
  { case cgQuit: do_quit(); /*@notreached@*/ break;
    case cgClose: rcw window_hide(current_window_x, 2); break;
#if TGC_IS_CURSES
    case cgOverwriteFile:
      document = current_document_x;
      if (document != NULL) start_save_as(document, khmli_filename, truE);
      break;
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
    case cgOverwriteDownload: content_download(truE); break;
#endif
#if CONFIG_EXTRA & EXTRA_DUMP
    case cgOverwriteDump:
      document = current_document_x;
      if (document != NULL) content_dump(document, khmli_filename, truE, NULL);
      break;
#endif
#if CONFIG_SESSIONS
    case cgOverwriteSession: session_save(khmli_filename, truE); break;
#endif
#endif
    case cgSubmit: rcw current_document_form_submit(); break;
    case cgReset: current_document_form_reset(); break;
    case cgRepost: rcw reload_document(); break;
    case cgHtml: rcw interpret_as_html(); break;
    case cgEnable: rcw document = current_document_x;
#if TGC_IS_GRAPHICS
      /* IMPLEMENTME! */
#else
      if ( (document != NULL) && (document->aecur != INVALID_AE) )
        document->cantent->aebase[document->aecur].flags &= ~aefDisabled;
#endif
      break;
  }
}

typedef struct
{
#if CONFIG_USER_QUERY
  tUserQuery* query;
#endif
  tConfirmationGoal cg;
} tAskYesnoData;

#if CONFIG_USER_QUERY

static void user_query_removal(void* _query, tDhmNotificationFlags flags)
{ tUserQuery* query = (tUserQuery*) _query;
  query->mif |= mifObjectVanished;
  (lid.callback)(lid.callback_data, liekFail, 0);
}

static void user_query_push(tUserQuery* query)
{ dhm_notification_off(query->resource, user_query_removal, query);
  (query->callback)(query);
}

#endif

static void ask_yesno_callback(void* _data, tLineInputEventKind liek, tKey key)
{ tAskYesnoData* data = (tAskYesnoData*) _data;
#if CONFIG_USER_QUERY
  tUserQuery* query = data->query;
#endif
  tConfirmationGoal cg = data->cg;
  switch (liek)
  { case liekFail:
#if CONFIG_USER_QUERY
      if (query != NULL) query->mif |= mifQueryFailed;
#endif
      goto do_cancel0; /*@notreached@*/ break;
    case liekCancel:
      do_cancel: {}
#if CONFIG_USER_QUERY
      if (query != NULL) query->mif |= mifUserCancelled;
#endif
      do_cancel0: line_input_finish(); memory_deallocate(data);
#if CONFIG_USER_QUERY
      if (query != NULL) { handle_conf_query: user_query_push(query); }
#endif
      break;
#if TGC_IS_CURSES
    case liekKey:
      if ( (key == config.char_yes) || (key == KEY_ENTER) )
      { line_input_finish(); memory_deallocate(data);
#if CONFIG_USER_QUERY
        if (query != NULL) { goto handle_conf_query; } else
#endif
        { ask_yesno_act(cg); }
      }
      else if (key == config.char_no) goto do_cancel;
      break;
#endif
  }
}

static void ask_yesno(const char* msg, tConfirmationGoal cg, tUserQuery* query)
{ tAskYesnoData* data = memory_allocate(sizeof(tAskYesnoData), mapLineInput);
#if TGC_IS_GRAPHICS
#define buf msg /* (no string change necessary) */
#else
  char tempbuf[1024], *spfbuf;
  my_spf(tempbuf, sizeof(tempbuf), &spfbuf, "%s (%c/%c) ", msg,
    config.char_yes, config.char_no);
#define buf spfbuf
#endif
#if CONFIG_USER_QUERY
  data->query = query;
#endif
  data->cg = cg;
  line_input_start(buf, NULL, ask_yesno_callback, data, liafNone,
    _("Confirmation Required"), istrYesUc, istrNoUc);
#undef buf
#if !TGC_IS_GRAPHICS
  my_spf_cleanup(tempbuf, spfbuf);
#endif
}

typedef struct
{
#if CONFIG_USER_QUERY
  tUserQuery* query;
#endif
  tEditableLineInputGoal elig;
} tEstartData;

#if CONFIG_USER_QUERY

static void do_start_user_query_login(tUserQuery* query)
{ tResource* resource = query->resource;
  tMissingInformationFlags mif = query->mif;
  tBoolean pr = cond2boolean(mif & mifProxyrelated);
  const char *ps = (pr ? _("proxy") : _("server")), *lf, *what, *sep,
    *old_username, *userat; /* user_at? use_rat? :-) */
  char schemebuf[MAXSCHEMESTRSIZE], * spfbuf;
  tEditableLineInputGoal elig;
  if (mif & mifPriorLoginAttemptFailed)
  { lf = (pr ? _("Proxy login failed") : _(strLoginFailed));
    sep = strSpacedDash;
  }
  else lf = sep = strEmpty;

  rp2scheme(resource->protocol, schemebuf); old_username = userat = strEmpty;
  if (mif & mifUsername) { elig = eligUsername; what = _("username"); }
  else if (mif & mifPassword)
  { const char* u = resource->uri_data->username;
    if (u != NULL) { old_username = u; userat = "@"; }
    elig = eligPassword; what = _("password");
  }
  else
  { /* bug */
    return;
  }

  my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("%s%s%s %s for %s %s%s%s:%d: "), lf,
    sep, schemebuf, what, ps, old_username, userat, query->hostname,
    ntohs(query->portnumber));
  line_input_estart(elig, spfbuf, NULL, query, _("User Input"), _("Apply"),
    _(strCancel));
  my_spf_cleanup(strbuf, spfbuf);
}

void user_query_queue(tUserQuery* query)
{ tResource* resource = query->resource;
#if TGC_IS_CURSES
  if (key_handling_mode == khmLineInput)
  { /* IMPLEMENTME: add a tRemainingWork entry instead of failing! */
    query->mif |= mifQueryFailed; return;
  }
#endif
#if CONFIG_FTP && OPTION_TLS
  if (query->mif & mifFtpsDataclear)
  { const char* hostname = query->hostname;
    char schemebuf[MAXSCHEMESTRSIZE];
    if (strlen(hostname) > STRBUF_SIZE / 4) /* can't be serious */
      hostname = "[?]";
    rp2scheme(resource->protocol, schemebuf);
    sprint_safe(strbuf,
      _("%s server %s:%d can't protect data - allow cleartext?"),
      schemebuf, hostname, ntohs(query->portnumber));
    ask_yesno(strbuf, cgFtpsDataclear, query);
  }
  else
#endif
  { do_start_user_query_login(query); }
#if TGC_IS_CURSES
  if (key_handling_mode == khmLineInput)
  { dhm_notification_setup(resource, user_query_removal, query, dhmnfRemoval,
      dhmnSet);
  }
#endif
}

static void start_user_query_username(void)
/* go from "asking for password" back to "ask for username"; CHECKME: dirty! */
{ const tEstartData* data = (const tEstartData*) (lid.callback_data);
  tUserQuery* query = data->query;
  memory_deallocate(data); line_input_finish();
  query->mif &= ~mifPassword; query->mif |= mifUsername;
  do_start_user_query_login(query);
}

#endif /* #if CONFIG_USER_QUERY */

#if CONFIG_JUMPS

static /*@null@*/ const char* previous_jump = NULL;

static tConfigJump* lookup_jump_shortcut(const char* shortcut)
{ tConfigJump* j = config.jumps;
  while (j != NULL)
  { const char* name = j->name;
    if ( (name != NULL) && (!strcmp(name, shortcut)) ) break; /* found */
    j = j->next;
  }
  return(j);
}

static one_caller void handle_lig_jump(const char* text)
{ char *text2 SHUT_UP_COMPILER(NULL), *uri SHUT_UP_COMPILER(NULL);
  const tConfigJump* j;
  tBoolean cleanup_text2, cleanup_uri;
  cleanup_text2 = cleanup_uri = falsE; my_strdedup(previous_jump, text);
  if (my_strchr(text, ' ') == NULL) /* no arguments given, just the shortcut */
  { j = lookup_jump_shortcut(text);
    if (j != NULL)
    { uri = j->uri;
      if ( (uri != NULL) && (*uri != '\0') )
      { prr:
        wk_browser_prr(require_browser_window(), uri, prrfRedrawOne, NULL);
      }
      else { bad_uri: show_message(_(strResourceError[reUri]), truE); }
    }
    else
    { char* spfbuf;
      not_found:
      my_spf(strbuf, STRBUF_SIZE, &spfbuf,
        _("Can't find the jump shortcut \"%s\""), text);
      show_message(spfbuf, truE);
      my_spf_cleanup(strbuf, spfbuf);
    }
  }
  else /* need to extract shortcut and replace markers with argument values */
  { char *ptr, *ptr2;
    unsigned short count;
    tBoolean is_last;
    text2 = my_strdup(text); cleanup_text2 = truE;
    ptr = text2; ptr2 = my_strchr(ptr, ' ');
    if (ptr2 == NULL) /* "can't happen" */
    { not_found2:
      text = ptr; goto not_found;
    }
    *ptr2 = '\0';
    j = lookup_jump_shortcut(ptr);
    if (j == NULL) goto not_found2;
    uri = j->uri;
    if ( (uri == NULL) || (*uri == '\0') ) goto bad_uri;
    uri = my_strdup(uri); cleanup_uri = truE;
    /* Try to replace all markers in the URI with arguments: */
    is_last = falsE;
    for (count = 0; count < j->argcount; count++)
    { const char *marker, *match, *src;
      char *new_uri, *dest;
      size_t markerlen, size;
      if (is_last) break; /* no more arguments given */
      spaceloop:
      ptr = ptr2 + 1;
      if (*ptr == '\0') break;
      ptr2 = my_strchr(ptr, ' ');
      if (ptr2 != NULL)
      { if (ptr2 == ptr) goto spaceloop; /* skip accidental space sequence */
        *ptr2 = '\0';
      }
      else
      { ptr2 = ptr;
        while (*ptr2 != '\0') ptr2++; /* set it onto the trailing '\0' byte */
        is_last = truE;
      }
      marker = j->arg[count];
      if ( (marker == NULL) || (*marker == '\0') ) /* "can't happen" */
        continue;
      match = my_strstr(uri, marker);
      if (match == NULL)
      { char* spfbuf;
        my_spf(strbuf, STRBUF_SIZE, &spfbuf,
          _("Can't find marker \"%s\" in URL pattern"), marker);
        show_message(spfbuf, truE);
        my_spf_cleanup(strbuf, spfbuf);
        goto cleanup;
      }
      markerlen = strlen(marker);
      size = strlen(uri) - markerlen + strlen(ptr) + 1;
      new_uri = __memory_allocate(size, mapString);
      src = uri; dest = new_uri;
      while (src < match) *dest++ = *src++;
      while (ptr < ptr2) *dest++ = *ptr++;
      src += markerlen;
      while ( (*dest++ = *src++) != '\0' ) { /* nothing */ }
      memory_deallocate(uri); uri = new_uri;
    }
    goto prr;
  }
  cleanup:
  if (cleanup_text2) memory_deallocate(text2);
  if (cleanup_uri) memory_deallocate(uri);
}

#endif /* #if CONFIG_JUMPS */

#if OPTION_EXECEXT & EXECEXT_SHELL

static const char strNoShellCmd[] = N_("No shell command given"),
  strExecextShellColon[] = "execext-shell:";

static const char* execext_shell_header(void)
{ char* spfbuf;
  my_spf(strbuf2, STRBUF_SIZE, &spfbuf, "V:%s\n\n", strSoftwareId);
  return(spfbuf);
}

static void execext_shell_request_callback(void* _document,
  tDhmNotificationFlags flags)
{ tBrowserDocument* document = (tBrowserDocument*) _document;
  if (flags & (dhmnfDataChange | dhmnfMetadataChange))
  { const tResourceRequest* request = document->request;
    const tResource* resource = request->resource;
    tResourceError re;
    if (resource != NULL)
    { const char* msg;
      tBoolean is_err;
      if ( (re = resource->error) != reFine )
      { handle_re: msg = _(strResourceError[re]); is_err = truE;
        set_msg:
        wk_info_set_message((tWindow*)(document->container), msg, is_err);
      }
      else if (resource->flags & rfFinal)
      { msg = _(strResourceState[rsComplete]); is_err = falsE; goto set_msg; }
    }
    else if ( (re = request->error) != reFine ) goto handle_re;
    document_display(document, wrtRedraw);
  }
  else if (flags & dhmnfAttachery) /* a resource was attached to the request */
  { tResource* resource = document->request->resource;
    dhm_notification_setup(resource, execext_shell_request_callback, _document,
      dhmnfDataChange | dhmnfMetadataChange, dhmnSet);
    cantent_put(document->cantent);
    cantent_attach(document->cantent, resource->cantent);
  }
}

static one_caller void handle_lig_execext_shell(const char* text)
{ tExecextShellData data;
  tExecextShellFlags esf = esfNone;
  const tBrowserDocument* document;
  const tCantent* cantent;
  const tContentblock *cont SHUT_UP_COMPILER(NULL), *cont2;
  const char* spfbuf_header = strEmpty;
  size_t header_len, content_size, writedata_size;
  tBoolean must_dealloc_command, want_header, want_content, interpret_percents;
  must_dealloc_command = want_header = want_content = interpret_percents=falsE;
  if (*text == ':') /* options given */
  { char ch;
    while ( (ch = *++text) != '\0' )
    { switch (ch)
      { case 'o': esf |= esfReadStdout; break;
        case 'e': esf |= esfReadStderr; break;
        case 'H': esf |= esfEnforceHtml; break;
        case 'h': want_header = truE; break;
        case 'c': want_content = truE; break;
        case 'i': interpret_percents = truE; break;
        case ' ': text++; goto end_of_options; /*@notreached@*/ break;
      }
    }
    end_of_options: {}
  }
  if (*text == '\0') { show_message(_(strNoShellCmd), truE); return; }
#if !DO_WK_INFO
  esf &= ~(esfReadStdout | esfReadStderr);
    /* feature not available; CHECKME: show error message instead? */
#endif
  my_memclr_var(data); data.esf = esf; document = current_document_x;
  cantent = ( (document != NULL) ? document->cantent : NULL );

  if (!interpret_percents) { simple_command: data.command = text; }
  else
  { const char *temp = my_strstr(text, "%U"), *uri;
    char* convcomm;
    if (temp == NULL) goto simple_command;
    if ( (document != NULL) && ( (uri = document->title) != NULL ) && (*uri) )
    { const size_t len_before = temp - text, len_uri = strlen(uri),
        len_after = strlen(temp + 2);
      char* ptr = convcomm = __memory_allocate(len_before + len_uri + len_after
        + 1, mapString);
      if (len_before > 0) { my_memcpy(ptr, text, len_before); ptr+=len_before;}
      my_memcpy(ptr, uri, len_uri); ptr += len_uri;
      if (len_after > 0) { my_memcpy(ptr, temp+2, len_after); ptr+=len_after; }
      *ptr = '\0';
    }
    else /* just remove the "%U" */
    { size_t len_before = temp - text;
      convcomm = __memory_allocate(strlen(text) - 2 + 1, mapString);
      if (len_before > 0) my_memcpy(convcomm, text, len_before);
      strcpy(convcomm + len_before, temp + 2);
    }
    data.command = convcomm; must_dealloc_command = truE;
  }

#if DO_WK_INFO
  if (esf & (esfReadStdout | esfReadStderr))
  { tPrrData prr_data;
    tResourceRequest* request;
    tResourceError re;
    tWindow* iwindow;
    tBrowserDocument* idocument;
    const tBrowserDocumentDisplayMode bddm = ( (esf & esfEnforceHtml) ?
      bddmHtml : bddmSource );
    char* spfbuf;
    prr_setup(&prr_data, strExecextShellColon /* dummy - CHECKME! */,
      prrfUpsfp4);
    prepare_resource_request(&prr_data);
    if ( (re = prr_data.result->error) != reFine )
    { show_message(_(strResourceError[re]), truE);
      prr_setdown(&prr_data); goto cleanup;
    }
    request = data.request = prr_data.result; prr_data.result = NULL;
    prr_setdown(&prr_data);
    my_spf(NULL, 0, &spfbuf, _("Result of \"%s\""), text);
    iwindow = visible_window_x[1 - current_window_index_x] =
      wk_info_create(my_spf_use(spfbuf), bddm, &idocument);
    idocument->request = request;
    dhm_notification_setup(request, execext_shell_request_callback, idocument,
      dhmnfDataChange | dhmnfMetadataChange | dhmnfAttachery, dhmnSet);
    wk_info_finalize(iwindow);
  }
#endif

  header_len = content_size = writedata_size = 0;
  if (want_header)
  { spfbuf_header = execext_shell_header(); header_len = strlen(spfbuf_header);
  }
  if (want_content)
  { if (cantent != NULL)
    { cont2 = cont = cantent->content;
      while (cont2 != NULL) { content_size += cont2->used; cont2=cont2->next; }
    }
    if (content_size <= 0) want_content = falsE; /* no content there */
  }
  writedata_size = header_len + content_size;
  if (writedata_size > 0) /* user wants to write something (header/content) */
  { char* writedata = __memory_allocate(writedata_size, mapWritedata), *temp;
    data.writedata = writedata; data.writedata_size = writedata_size;
    if (header_len > 0) my_memcpy(writedata, spfbuf_header, header_len);
    temp = writedata + header_len; cont2 = cont;
    while (cont2 != NULL)
    { const size_t usedsize = cont2->used;
      if (usedsize > 0)
      { my_memcpy(temp, cont2->data, usedsize); temp += usedsize; }
      cont2 = cont2->next;
    }
  }
  if (spfbuf_header != strEmpty) my_spf_cleanup(strbuf2, spfbuf_header);

  resource_start_execext_shell(&data);
  cleanup:
  if (must_dealloc_command) memory_deallocate(data.command);
  __dealloc(data.writedata);
}

#endif /* #if OPTION_EXECEXT & EXECEXT_SHELL */

static tBoolean test_local_file_overwrite(/*const*/ char** _filename,
  tConfirmationGoal cg, /*@out@*/ tBoolean* _may_overwrite)
/* determines whether a local file may be overwritten right now; if not, this
   function either prepares a "may overwrite?" question for the user or
   produces an error message */
{ tBoolean retval;
  const char* filename = *_filename;
  struct stat statbuf;
  int err;

  retval = *_may_overwrite = falsE;
  err = my_stat(filename, &statbuf);
  if (err != 0)
  { if (err != -1) errno = 0; /* "should not happen" */
    if (errno == ENOENT) { allow: retval = truE; } /* nothing there */
    else { failed: show_message_osfailure(errno, NULL); }
  }
  else /* already there */
  { const mode_t mode = statbuf.st_mode;
    if (!S_ISREG(mode)) /* only overwrite _files_ */
    { errno = ( (S_ISDIR(mode)) ? EISDIR : 0 ); goto failed; }
    else if (config.flags & cfDontConfirmOverwrite)
    { *_may_overwrite = truE; goto allow; }
    else
    { char bstr[1024];
      const char* bstrptr;
      const off_t size = statbuf.st_size;
      __dealloc(khmli_filename); khmli_filename = filename; *_filename = NULL;
      if (size < 0) bstrptr = strEmpty; /* "should not happen" */
      else
      { sprint_safe(bstr,strBracedNumstr,localized_size(size),bytebytes(size));
        bstrptr = bstr;
      }
      sprint_safe(strbuf, _("Overwrite existing%s?"), bstrptr);
      ask_yesno(strbuf, cg, NULL);
    }
  }
  return(retval);
}

static one_caller void line_input_estart_act(tEditableLineInputGoal elig,
  /*const*/ char** _text)
{ const char* text = *_text;
  tBrowserDocument* document;
  tWindowRedrawingTask wrt;
  tUriData* uri_data;
  tResourceError re;
  switch (elig)
  { case eligGoToUri:
      wk_browser_prr(require_browser_window(), text, prrfRedrawOne, NULL);
      break;
    case eligSaveAs:
      { tBoolean may_overwrite;
        document = current_document_x;
        if ( (document != NULL) && (test_local_file_overwrite(_text,
            cgOverwriteFile, &may_overwrite)) )
          start_save_as(document, text, may_overwrite);
      }
      break;
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
    case eligDownloadUri:
      uri_data = uri_parse(text, NULL, NULL, NULL, 0);
      re = uri_data->re; uri_put(uri_data);
      if (re != reFine) { show_message(_(strResourceError[re]),truE); return; }
      __dealloc(khmli_download_uri); khmli_download_uri = text; *_text = NULL;
      ask_download_to();
      break;
    case eligDownloadFilename:
      { tBoolean may_overwrite;
        if(test_local_file_overwrite(_text,cgOverwriteDownload,&may_overwrite))
          __content_download(text, may_overwrite);
      }
      break;
#endif
#if CONFIG_EXTRA & EXTRA_DUMP
    case eligDumpFilename:
      { tBoolean may_overwrite;
        if (test_local_file_overwrite(_text, cgOverwriteDump, &may_overwrite))
        { if ( (document = current_document_x) != NULL )
            content_dump(document, text, may_overwrite, NULL);
        }
      }
      break;
#endif
    case eligDocumentSearch:
      wrt = wrtSearch;
      handle_search:
      __dealloc(search_string); search_string = text; *_text = NULL;
      if ( (document = current_document_x) != NULL )
        document_display(document, wrt);
      break;
    case eligDocumentSearchBackward:
      wrt = wrtSearchBackward; goto handle_search; /*@notreached@*/ break;
#if CONFIG_JUMPS
    case eligJump: handle_lig_jump(text); break;
#endif
#if TGC_IS_CURSES
    case eligFormText: case eligFormPassword: case eligFormFilename:
      { tActiveElement* aes;
        tActiveElementNumber _ae;
        if ( (document = current_document_x) == NULL ) return;
        aes = document->active_element; _ae = document->aecur;
        if (*text != '\0') my_strdedup(aes[_ae].current_text, text);
        else dealloc(aes[_ae].current_text);
        /* IMPLEMENTME: "javascript_handle_event(jekChange, _ae);" if
           appropriate */
        javascript_handle_event(jekBlur, _ae);
        document_display(document, wrtRedraw);
      }
      break;
#endif
#if CONFIG_SESSIONS
    case eligSessionSave:
      { tBoolean may_overwrite;
        if (test_local_file_overwrite(_text,cgOverwriteSession,&may_overwrite))
          session_save(text, may_overwrite);
      }
      break;
    case eligSessionResume: session_resume(text); break;
#endif
#if OPTION_EXECEXT & EXECEXT_SHELL
    case eligExecextShell: handle_lig_execext_shell(text); break;
#endif
  }
}

#if TGC_IS_CURSES
static my_inline void __curleft(void)
{ (void) move(stdscr->_cury, stdscr->_curx - 1);
}
static my_inline void __curright(void)
{ (void) move(stdscr->_cury, stdscr->_curx + 1);
}
#endif

static void line_input_estart_callback(void* _data, tLineInputEventKind liek,
  tKey key)
{ tEstartData* data = (tEstartData*) _data;
#if CONFIG_USER_QUERY
  tUserQuery* query = data->query;
#endif
  tEditableLineInputGoal elig = data->elig;
  tLineInputActionCode liac;
  switch (liek)
  { case liekFail:
#if CONFIG_USER_QUERY
      if (query != NULL) query->mif |= mifQueryFailed;
#endif
      goto do_cancel0; /*@notreached@*/ break;
    case liekCancel:
      do_cancel: {}
#if CONFIG_USER_QUERY
      if (query != NULL) query->mif |= mifUserCancelled;
#endif
      do_cancel0: line_input_finish(); memory_deallocate(data);
#if CONFIG_USER_QUERY
      if (query != NULL) user_query_push(query);
#endif
      break;
#if TGC_IS_CURSES
    case liekKey:
      liac = lookup_lineinput_key(key);
      switch (liac)
      { case liacUnknown: break; /* the most likely case first */
        case liacCancel: goto do_cancel; /*@notreached@*/ break;
        case liacToLeft:
          if (lid_area(pos) > 0)
          { lid_area(pos)--;
            if (lid_area(colcurr) > lid_area(colmin))
            { __curleft(); lid_area(colcurr)--; }
            else { lid_area(first)--; line_input_redraw(); }
          }
          return; /*@notreached@*/ break;
        case liacToRight:
          if (lid_area(pos) < lid_area(len))
          { lid_area(pos)++;
            if (lid_area(colcurr) < lid_area(colmax))
            { __curright(); lid_area(colcurr)++; }
            else { lid_area(first)++; line_input_redraw(); }
          }
          return; /*@notreached@*/ break;
        case liacToStart:
          if (lid_area(pos) > 0)
          { lid_area(pos) = lid_area(first) = 0;
            lid_area(colcurr) = lid_area(colmin);
            line_input_redraw();
          }
          return; /*@notreached@*/ break;
        case liacToEnd: line_input_to_eos(); return; /*@notreached@*/ break;
        case liacPass2user:
#if CONFIG_USER_QUERY
          if ( (elig == eligPassword) && (query != NULL) )
            start_user_query_username();
#endif
          return; /*@notreached@*/ break;
      }

      if (key == KEY_ENTER)
      { if ( (lid_area(len) > 0) || (lid_area(flags) & liafAllowEmptyText) )
        { char* text;
          line_input_ensure_room(); text = lid_area(text);
          text[lid_area(len)] = '\0'; lid_area(text) = NULL;
          line_input_finish(); memory_deallocate(data);
#if CONFIG_DEBUG
          sprint_safe(strbuf, "line input: *%s*\n", text);
          debugmsg(strbuf);
#endif
#if CONFIG_USER_QUERY
          if (query != NULL)
          { tResource* resource = query->resource;
            query->mif &= ~mifSet;
            if (elig == eligUsername)
            { resource->uri_data->username = text; text = NULL;
              query->mif |= mifUsername;
            }
            else if (elig == eligPassword)
            { resource->uri_data->password = text; text = NULL;
              query->mif |= mifPassword;
            }
            user_query_push(query);
          }
          else
#endif
          { line_input_estart_act(elig, &text); }
          __dealloc(text);
        }
      }
      else if (key == KEY_BACKSPACE)
      { if (lid_area(pos) > 0)
        { char* text = lid_area(text);
          short count;
          /* update the string */
          for (count = lid_area(pos); count < lid_area(len); count++)
            text[count - 1] = text[count];
          lid_area(pos)--; lid_area(len)--;
          /* update the screen */
          if (lid_area(colcurr) <= lid_area(colmin)) lid_area(first)--;
          else { lid_area(colcurr)--; line_input_redraw(); }
        }
      }
      else if (key == KEY_DC)
      { if (lid_area(pos) < lid_area(len))
        { char* text = lid_area(text);
          short count;
          /* update the string */
          for (count = lid_area(pos); count < lid_area(len) - 1; count++)
            text[count] = text[count + 1];
          lid_area(len)--;
          /* update the screen */
          line_input_redraw();
        }
      }
      else if ( (key >= ' ') && (key <= 255) && (key != 127) )
      { /* CHECKME: OPTION_CED? */
        line_input_paste_char(key);
      }
      break;
#endif /* #if TGC_IS_CURSES */
  }
}

static void __line_input_estart(tEditableLineInputGoal elig, const char* msg,
  const char* initstr, tUserQuery* query)
{ tLineInputAreaFlags liaf = liafEditable;
  tEstartData* data = __memory_allocate(sizeof(tEstartData), mapLineInput);
#if CONFIG_USER_QUERY
  data->query = query;
#endif
  data->elig = elig;
#if TGC_IS_CURSES
  if (is_disguising_elig(elig)) liaf |= liafDisguising;
  if (is_emptyok_elig(elig)) liaf |= liafAllowEmptyText;
#endif
  __line_input_start(msg, initstr, line_input_estart_callback, data, liaf);
}


/** Keys and command codes */

static one_caller void remap_key(tKey* _key)
{ switch (*_key)
  { case KEY_ESCAPE: *_key = KEY_CANCEL; break;
    case '\015': case '\012': *_key = KEY_ENTER; break;
    case '\010': *_key = KEY_BACKSPACE; break;
    case 127: *_key = KEY_DC; break;
  }
}

static tBoolean generic_handle_command(tProgramCommandCode code)
/* if the window-specific handler didn't like the command... */
{ tBoolean retval = truE;
  const char *temp, *text, *msg, *uri;
  tEditableLineInputGoal elig;
  tBrowserDocument* document;
  tLinenumber scrolloff;
  switch (code)
  {case pccScreenUnsplit:
    if (visible_window_x[1] != NULL)
      window_hide(visible_window_x[1 - current_window_index_x], 1);
    break;
   case pccScreenSplit:
    if (visible_window_x[1] == NULL)
    { if ( (document = current_document_x) != NULL )
      { const tActiveElementNumber _ae = document->aecur;
        if (_ae != INVALID_AE) do_activate_element(document, _ae, falsE);
      }
      visible_window_x[1] = wk_browser_create(); current_window_index_x = 1;
      window_redraw_all();
    }
    break;
   case pccScreenSwitch:
    if (visible_window_x[1] != NULL)
    { current_window_index_x = 1 - current_window_index_x;
      redraw_message_line();
    }
    break;
   case pccWindowNext:
    { tWindow* window = current_window_x;
      tBoolean is_first_try = truE;
      findloop1: window = window->next;
      if (window == NULL) window = windowlisthead;
      if (window != NULL)
      { if (window_is_visible(window))
        { if (is_first_try) { is_first_try = falsE; goto findloop1; }
          else goto out;
        }
        current_window_x = window; window_redraw(current_window_x);
      }
    }
    break;
   case pccWindowPrevious:
    { tWindow* window = current_window_x;
      tBoolean is_first_try = truE;
      findloop2: window = window->prev;
      if (window == NULL) window = windowlisttail;
      if (window != NULL)
      { if (window_is_visible(window))
        { if (is_first_try) { is_first_try = falsE; goto findloop2; }
          else goto out;
        }
        current_window_x = window; window_redraw(current_window_x);
      }
    }
    break;
   case pccMenuWindowlist:
#if CONFIG_MENUS & MENUS_WLIST
     cm_setup_window_list();
#else
     menus_were_disabled();
#endif
     break;
   case pccMenuContextual:
#if CONFIG_MENUS & MENUS_CONTEXT
    { tActiveElementNumber _ae;
      document = current_document_x;
      _ae = ( (document != NULL) ? (document->aecur) : INVALID_AE );
      cm_setup_contextual(0, ( (current_window_index_x == 1) ? (VMIDDLE + 1) :
        0 ), _ae);
    }
#else
     menus_were_disabled();
#endif
    break;
   case pccMenuUriHistory:
#if CONFIG_MENUS & MENUS_UHIST
    cm_setup_uri_history();
#else
    menus_were_disabled();
#endif
    break;
   case pccGoUri:
    text = _("Go to URL: "); temp = NULL;
    go_to_uri:
    line_input_estart(eligGoToUri, text, temp, NULL, _("Enter URL"), _("Go"),
      _(strCancel));
    break;
   case pccGoUriPreset:
    { if ( (document = current_document_x) != NULL )
      { text = _("Edit URL: "); temp = document->title; goto go_to_uri; }
    }
    break;
   case pccGoHome:
    if ( (uri = config.home_uri) != NULL )
    { do_uri: wk_browser_prr(require_browser_window(),uri,prrfRedrawOne,NULL);}
    else show_message(_("No home URL configured"), truE);
    break;
   case pccGoSearch:
    if ( (uri = config.search_engine) != NULL ) goto do_uri;
    else show_message(_("No search engine URL configured"), truE);
    break;
   case pccGoBookmarks:
    if ( (uri = config.bookmarks) != NULL ) goto do_uri;
    else show_message(_("No bookmarks document URL configured"), truE);
    break;
#if CONFIG_JUMPS
   case pccJump:
    temp = NULL;
    handle_jump:
    if (config.jumps != NULL)
    { line_input_estart(eligJump, _("Jump to: "), temp, NULL, _("Jump to URL"),
        _("Jump"), _(strCancel));
    }
    else show_message(_("No jump shortcuts configured"), truE);
    break;
   case pccJumpPreset:
    temp = previous_jump; goto handle_jump; /*@notreached@*/ break;
#else
   case pccJump: case pccJumpPreset: fwdact(_(strFjumps)); break;
#endif
   case pccWindowClose:
    if (config.flags & cfDontConfirmClose) window_hide(current_window_x, 2);
    else ask_yesno(_("Really close this window?"), cgClose, NULL);
    break;
   case pccWindowNew:
    { const tWindow* window = current_window_x;
      if ( (!is_browser_window(window)) || (window_is_precious(window)) )
      { current_window_x = wk_browser_create();
        window_redraw(current_window_x);
      }
    }
    break;
   case pccWindowNewFromDocument:
    { document = current_document_x;
      if ( (document != NULL) && ( (uri = document->title) != NULL ) )
      { current_window_x = wk_browser_create();
        wk_browser_prr(current_window_x, uri, prrfRedrawOne, NULL);
      }
    }
    break;
   case pccDocumentSave:
    { tCantent* cantent;
      if ( ( (document = current_document_x) != NULL ) &&
           ( (cantent = document->cantent) != NULL ) )
      { const char* filename;
        uri = document->title; filename = (uri ? uristr2filename(uri) : NULL);
        line_input_estart(eligSaveAs, _("Save document as: "), filename, NULL,
          _("Save Document"), _(strSave), _(strCancel));
        __dealloc(filename);
      }
      else goto not_handled;
    }
    break;
   case pccDocumentTop:
    if ( (document = current_document_x) != NULL )
    { if (document->origin_y > 0)
      { document->origin_y = 0; document->flags |= wvfScrollingUp;
#if TGC_IS_CURSES
        document->aecur = INVALID_AE;
#endif
        document_display(document, wrtRedraw);
      }
    }
    else goto not_handled;
    break;
   case pccDocumentBottom:
    document = current_document_x;
    if (document != NULL) document_display(document, wrtToEnd);
    else goto not_handled;
    break;
   case pccPageDown:
    document = current_document_x;
    if ( (document != NULL) && (document_section_height(document, &scrolloff)))
    { scroll_down:
      if (document->flags & wvfScreenFull)
      { document->origin_y += scrolloff; document->flags &= ~wvfScrollingUp;
        do_scroll:
#if TGC_IS_CURSES
        document->aecur = INVALID_AE;
#endif
        document_display(document, wrtRedraw);
      }
    }
    break;
   case pccPageUp:
    document = current_document_x;
    if ( (document != NULL) && (document_section_height(document, &scrolloff)))
    { tLinenumber y;
      negate_scroll_up: scrolloff = -scrolloff;
      scroll_up: y = document->origin_y + scrolloff;
      if (y < 0) y = 0;
      if (document->origin_y > y) /* actually need to scroll */
      { document->origin_y = y; document->flags |= wvfScrollingUp;
        goto do_scroll;
      }
    }
    break;
   case pccLineDown:
    if ( (document = current_document_x) != NULL )
    { scrolloff = 1; goto scroll_down; }
    break;
   case pccLineUp:
    if ( (document = current_document_x) != NULL )
    { scrolloff = -1; goto scroll_up; }
    break;
   case pccElementNext:
    if ( (document = current_document_x) != NULL )
    { tActiveElementNumber _ae = document->aecur, _ae2;
      if ( (_ae != INVALID_AE) &&
           ( (_ae2 = next_visible_ae(document, _ae)) != INVALID_AE ) )
      { activate_element(document, _ae2); }
      else if (document_section_height(document, &scrolloff))
        goto scroll_down;
    }
    break;
   case pccElementPrevious:
    if ( (document = current_document_x) != NULL )
    { tActiveElementNumber _ae = document->aecur, _ae2;
      if ( (_ae != INVALID_AE) &&
           ( (_ae2 = previous_visible_ae(document, _ae)) != INVALID_AE ) )
      { activate_element(document, _ae2); }
      else if (document_section_height(document, &scrolloff))
        goto negate_scroll_up;
    }
    break;
   case pccElementOpen: case pccWindowNewFromElement: case pccElementOpenSplit:
    { tActiveElementNumber _ae;
      tCantent* cantent;
      if ( ( (document = current_document_x) != NULL ) &&
           ( (_ae = document->aecur) != INVALID_AE ) &&
           ( (cantent = document->cantent) != NULL ) )
      { const tActiveElementBase* aebase = cantent->aebase;
        tActiveElementKind kind;
        tActiveElement* aes;
        if (aebase[_ae].flags & aefDisabled)
        { show_message(_("This element is disabled!"), truE); goto out; }
        javascript_handle_event(jekFocus, _ae);
        aes = document->active_element; kind = aebase[_ae].kind;
        switch (kind)
        {case aekLink:
          if ( ( (temp = aebase[_ae].data) != NULL) && (*temp != '\0') )
          { tWindow* destwin;
            tPrrFlags prrf = prrfRedrawOne;
            if ( (code == pccElementOpen) &&
                 (is_browser_window(current_window_x)) )
              destwin = current_window_x;
            else
            { destwin = wk_browser_create();
              if (code == pccElementOpenSplit)
              { visible_window_x[1 - current_window_index_x] = destwin;
                prrf = prrfRedrawAll;
              }
              else current_window_x = destwin;
            }
            wk_browser_prr(destwin, temp, prrf, document);
          }
          break;
         case aekFormSubmit: case aekFormImage:
           goto handle_submit; /*@notreached@*/ break;
         case aekFormReset: goto handle_reset; /*@notreached@*/ break;
         case aekFormText:
          elig = eligFormText;
          start_input:
          if (aebase[_ae].flags & aefReadonly)
          { /* IMPLEMENTME: the user should be able to see the _whole_ contents
               of the element somehow, not only what is displayed within the
               normal web page layout! (E.g. the text might be clipped.) */
            readonly:
            show_message(_("This element is read-only!"), truE);
            goto out;
          }
          else
          { sprint_safe(strbuf3, "%s: ", _(strAek[kind]));
            __line_input_estart(elig, strbuf3, aes[_ae].current_text, NULL);
            /* CHECKME: respect the "maxlength" attribute for eligFormText and
               eligFormPassword? */
          }
          break;
         case aekFormPassword:
          elig = eligFormPassword; goto start_input; /*@notreached@*/ break;
         case aekFormFile:
          elig = eligFormFilename; goto start_input; /*@notreached@*/ break;
         case aekFormCheckbox:
          if (aebase[_ae].flags & aefReadonly) goto readonly;
          else
          { aes[_ae].flags ^= aefCheckedSelected;
            document_display(document, wrtRedraw); /* IMPROVEME? */
          }
          break;
         case aekFormRadio:
          if (aebase[_ae].flags & aefReadonly) goto readonly;
          else if (!(aes[_ae].flags & aefCheckedSelected))
          { const tHtmlFormNumber fn = calc_hfn(cantent, _ae);
            const char* name = aebase[_ae].data;
            aes[_ae].flags |= aefCheckedSelected;
            /* Uncheck all other radio buttons of the same name in this form */
            if ( (fn != INVALID_HTML_FORM_NUMBER) && (name != NULL) )
            { tActiveElementNumber a, a1 = cantent->form[fn].first_ae,
                a2 = cantent->form[fn].last_ae;
              if ( (a1 != INVALID_AE) && (a2 != INVALID_AE) )
              { for (a = a1; a <= a2; a++)
                { if ( (a != _ae) && (aebase[a].kind == aekFormRadio) )
                  { const char* n = aebase[a].data;
                    if ( (n != NULL) && (!my_strcasecmp(name, n)) )
                      aes[a].flags &= ~aefCheckedSelected;
                  }
                }
              }
            }
            document_display(document, wrtRedraw); /* IMPROVEME? */
          }
          break;
         case aekFormSelect:
          if (aebase[_ae].maxlength > 0)
          {
#if CONFIG_MENUS & MENUS_HTML
            cm_setup_select_tag(aebase, document, _ae);
#else
            menus_were_disabled();
#endif
          }
          else show_message(_(strSelectionEmpty), truE);
          break;
        } /* switch */
      }
    }
    break;
#if TGC_IS_GRAPHICS
#define connect_sel(b, h, d) \
  connect_object(GTK_OBJECT(GTK_FILE_SELECTION(sel)->b), strGtkClicked, h, d)
   case pccLocalFileDirOpen:
    { GtkWidget* sel = gtk_file_selection_new(_("Open Local File/Directory"));
      connect_sel(ok_button, filesel_handle,
        filesel_data_create(sel, current_window));
      connect_sel(ok_button, graphics_window_destroy, sel);
      connect_sel(cancel_button, graphics_window_destroy, sel);
      show_widget(sel);
    }
    break;
#endif
   case pccDump:
#if CONFIG_EXTRA & EXTRA_DUMP
    if ( (document = current_document_x) != NULL )
    { line_input_estart(eligDumpFilename, _("Dump to: "), NULL, NULL,
        _("Dump to File"), _("Dump"), _(strCancel));
    }
    else goto not_handled;
#else
    fwdact(_("Fdumps"));
#endif
    break;
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
   case pccDownload:
     dealloc(khmli_download_uri); khmli_download_referrer = NULL;
     line_input_estart(eligDownloadUri, _("Download from: "), NULL, NULL,
       _(strUcDownload), _("Proceed"), _(strCancel));
     break;
   case pccDownloadFromElement:
    { const tCantent* cantent;
      const tActiveElementBase* aebase;
      tActiveElementNumber _ae;
      if ( ( (document = current_document_x) != NULL ) &&
#if TGC_IS_GRAPHICS
           ( (_ae = INVALID_AE) != INVALID_AE ) && /* FIXME! */
#else
           ( (_ae = document->aecur) != INVALID_AE ) &&
#endif
           ( (cantent = document->cantent) != NULL ) &&
           ( (aebase = cantent->aebase) != NULL ) )
      { const tActiveElementKind aek = aebase[_ae].kind;
        if (aek == aekLink)
        { temp = aebase[_ae].data;
          if ( (temp != NULL) && (*temp != '\0') )
          { my_strdedup(khmli_download_uri, temp);
            khmli_download_referrer = document;
            ask_download_to();
          }
        }
        else show_message(_("That's not a link"), truE);
      }
      else goto not_handled;
    }
    break;
#else
   case pccDownload: case pccDownloadFromElement:
     fwdact(_("Fdownloads")); break;
#endif
   case pccDocumentEnforceHtml:
    { if ( (document = current_document_x) != NULL )
      { if (config.flags & cfDontConfirmHtml) interpret_as_html();
        else ask_yesno(_("Really interpret as HTML?"), cgHtml, NULL);
      }
      else goto not_handled;
    }
    break;
   case pccFormReset:
    handle_reset:
    /* CHECKME: only ask if actually inside a form, otherwise fail! */
    if (config.flags & cfDontConfirmReset) current_document_form_reset();
    else ask_yesno(_("Really reset this form?"), cgReset, NULL);
    break;
   case pccFormSubmit:
    handle_submit:
    if (config.flags & cfDontConfirmSubmit) current_document_form_submit();
    else if ( (document = current_document_x) != NULL )
    { const tCantent* cantent = document->cantent;
      const tHtmlFormNumber hfn = calc_hfn(cantent, document->aecur);
      const tHtmlForm* form;
      char* spfbuf;
      if ( (cantent == NULL) || (hfn == INVALID_HTML_FORM_NUMBER) )
      { cnsniaf(); goto out; }
      form = &(cantent->form[hfn]);
      my_spf(strbuf, STRBUF_SIZE, &spfbuf,
        _("Really submit this form to \"%s\" (%s)?"), form->action_uri,
        ( (form->flags & hffMethodPost) ? strPost : strGet ));
      ask_yesno(spfbuf, cgSubmit, NULL); my_spf_cleanup(strbuf, spfbuf);
    }
    else goto not_handled;
    break;
   case pccStop:
    { tResourceRequest* request;
      document = current_document_x;
      if ( (document != NULL) && ( (request = document->request) != NULL) )
        resource_request_stop(request);
      else goto not_handled;
    }
    break;
   case pccDocumentSearch:
    elig = eligDocumentSearch; msg = _("Search string: ");
    check_search:
    if ( (document = current_document_x) != NULL )
    { line_input_estart(elig, msg, search_string, NULL, _("Search String"),
        _(strSearch), _(strCancel));
    }
    else goto not_handled;
    break;
   case pccDocumentSearchBackward:
    elig = eligDocumentSearchBackward; msg = _("Search string backwards: ");
    goto check_search; /*@notreached@*/ break;
   case pccDocumentSearchNext:
    if ( (search_string != NULL) && ( (document = current_document_x) != NULL))
      document_display(document, wrtSearch);
    break;
   case pccDocumentSearchPrevious:
    if ( (search_string != NULL) && ( (document = current_document_x) != NULL))
      document_display(document, wrtSearchBackward);
    break;
   case pccElementEnable:
    { tActiveElementNumber _ae;
      const tCantent* cantent;
      document = current_document_x;
      if ( (document != NULL) && ( (_ae = document->aecur) != INVALID_AE ) &&
           ( (cantent = document->cantent) != NULL ) &&
           (cantent->aebase[_ae].flags & aefDisabled) )
      { if (config.flags & cfDontConfirmEnable)
          cantent->aebase[_ae].flags &= ~aefDisabled;
        else ask_yesno(_("Really enable this element?"), cgEnable, NULL);
      }
      else goto not_handled;
    }
    break;
   case pccElementInfo:
    { tActiveElementNumber _ae;
      tCantent* cantent;
      document = current_document_x;
      if ( (document != NULL) && ( (_ae = document->aecur) != INVALID_AE ) &&
           ( (cantent = document->cantent) != NULL ) )
      { const tActiveElementBase* aeb = &(cantent->aebase[_ae]);
        const tActiveElementFlags flags = aeb->flags;
        const char *dat = null2empty(aeb->data),
          *disabl = ( (flags & aefDisabled) ? (_("disabled ")) : strEmpty ),
          *readonl = ( (flags & aefReadonly) ? (_("read-only ")) : strEmpty ),
          *mult = ( (flags & aefMultiple) ? (_("multi-option ")) : strEmpty );
        char* spfbuf;
        my_spf(strbuf, STRBUF_SIZE, &spfbuf, "%s%s%s%s%s%s", disabl, readonl,
          mult, _(strAek[aeb->kind]), ((*dat != '\0') ? ", " : strEmpty), dat);
        show_message(spfbuf, falsE); my_spf_cleanup(strbuf, spfbuf);
      }
      else goto not_handled;
    }
    break;
#if OPTION_EXECEXT & EXECEXT_SHELL
   case pccExecextShell:
    if ( (config.flags & cfExecextShellCustom) &&
         (config.execext_shell == NULL) )
    { show_message(_(strResourceError[reConfigForbids]), truE); }
    else
    { line_input_estart(eligExecextShell, _("Execute shell command: "), NULL,
        NULL, _("Execute Shell Command"), _("Execute"), _(strCancel));
    }
    break;
   case pccExecextShellFlip:
    config.flags ^= cfExecextShellCustom;
    sprint_safe(strbuf, _("execext-shell configuration: %s"), (config.flags &
      cfExecextShellCustom) ? _("customized") : _("standard"));
    show_message(strbuf, falsE);
    break;
#else
   case pccExecextShell: case pccExecextShellFlip:
     fwdact(_(strFexecext)); break;
#endif
#if CONFIG_SESSIONS
   case pccSessionSave:
     line_input_estart(eligSessionSave, _("Save session as: "),
       config.session_default, NULL, _("Save Session"), _(strSave),
       _(strCancel));
     break;
   case pccSessionResume:
     line_input_estart(eligSessionResume, _("Resume session from: "),
       config.session_default, NULL, _("Resume Session"), _("Resume"),
       _(strCancel));
     break;
#else
   case pccSessionSave: case pccSessionResume: fwdact(_(strFsessions)); break;
#endif
   case pccMouseFlip: case pccMouseOff: case pccMouseOn:
     mouse_flip(code - pccMouseFlip); break;
   case pccScrollBarsFlip:
#if MIGHT_USE_SCROLL_BARS
    config.flags ^= cfUseScrollBars; window_redraw_all();
#else
    fwdact(_("Fscroll bars"));
#endif
    break;
   case pccQuit:
    if (config.flags & cfDontConfirmQuit) do_quit();
    else ask_yesno(_("Really quit retawq?"), cgQuit, NULL);
    break;
   default: not_handled: retval = falsE; break; /* command not handled */
  }
  out:
  return(retval);
}

static tBoolean handle_command_code(tProgramCommandCode pcc)
{ tBoolean retval;
  tWindow* window = current_window_x;
  tBoolean (*func)(tWindow*, tProgramCommandCode) = window->spec->handle_pcc;
  retval = ( (func != NULL) ? ((func)(window, pcc)) : falsE );
  if (!retval) retval = generic_handle_command(pcc);
  return(retval);
}

#if TGC_IS_CURSES

#if CONFIG_DO_TEXTMODEMOUSE
static one_caller void handle_key_mouse(void)
{ MEVENT event;
  short x, _y;
  tLinenumber y, yt;
  tVisibleWindowIndex i;
  tWindow* window;
  tBrowserDocument* document;
  const tActiveElement* aes;
  tActiveElementNumber _ae;
  mmask_t mask;
  if (getmouse(&event) != OK) return;
  mask = event.bstate;
  if (!(mask & TEXTMODEMOUSE_MASK)) return; /* "should not happen" */
  x = event.x; _y = event.y; y = (tLinenumber) _y;
  if (!is_bottom_occupied)
  { sprint_safe(strbuf, _("mouse click: %d, %d"), x, _y);
    show_message(strbuf, falsE);
  }
  if (key_handling_mode == khmLineInput)
  { if ( (lid.curr == 1) && (y == lid_area(row)) && (x >= lid_area(colmin)) &&
         (x <= lid_area(colmax)) )
    { short offset = x - lid_area(colcurr);
      if (offset < 0)
      { li_off:
        lid_area(colcurr) += offset; lid_area(pos) += offset;
        (void) move(lid_area(row), lid_area(colcurr));
      }
      else if (offset > 0)
      { short pos = lid_area(pos) + offset;
        if (pos > lid_area(len)) pos = lid_area(len);
        if (pos > lid_area(pos)) { offset = pos - lid_area(pos); goto li_off; }
      }
      /* "else": offset == 0, nothing changes */
    }
    return;
  }
#if CONFIG_MENUS
  else if (key_handling_mode == khmMenu)
  { if (mask & BUTTON1_CLICKED) cm_handle_mouseclick(x, _y);
    return;
  }
#endif
  if (y > LINES - 2)
  { check_other_buttons:
    _ae = INVALID_AE;
    do_check_other_buttons:
    if (mask & (BUTTON2_CLICKED | BUTTON3_CLICKED))
    {
#if CONFIG_MENUS & MENUS_CONTEXT
      cm_setup_contextual(x, _y, _ae);
#else
      menus_were_disabled();
#endif
    }
    return;
  }
  if (visible_window_x[1] == NULL) i = 0;
  else
  { if (y == VMIDDLE) goto check_other_buttons;
    else if (y < VMIDDLE) i = 0;
    else { i = 1; y -= (VMIDDLE + 1); }
  }
  current_window_index_x = i; window = current_window_x;
  document = current_document_x;
  if ( (document == NULL) || (document->aenum <= 0) ) goto check_other_buttons;
  yt = y + document->origin_y; aes = document->active_element;
  for (_ae = 0; _ae < document->aenum; _ae++)
  { const tActiveElementCoordinates* aec = aes[_ae].aec;
    while (aec != NULL)
    { tLinenumber yc = aec->y;
      short x1 = aec->x1, x2 = aec->x2;
      if ( (yt == yc) && (x >= x1) && (x <= x2) )
      { const tActiveElementBase* aeb;
        if (_ae != document->aecur) activate_element(document, _ae);
        if (!(mask & BUTTON1_CLICKED)) goto do_check_other_buttons;
        aeb = &(document->cantent->aebase[_ae]);
        if (aeb->kind == aekLink)
        { javascript_handle_event(jekClick, _ae);
          wk_browser_prr(require_browser_window(), aeb->data, prrfRedrawOne,
            document);
        }
        return; /* done */
      }
      aec = aec->next;
    }
  }
  goto check_other_buttons;
}
#endif /* #if CONFIG_DO_TEXTMODEMOUSE */

static one_caller tMbsIndex lookup_command_key(tKey key)
{ if (keymap_command_keys_num <= 0) return(INVALID_INDEX); /* "can't happen" */
  my_binary_search(0, keymap_command_keys_num - 1,
    my_numcmp(key, keymap_command_keys[idx].key), return(idx))
}

static void handle_key(tKey key)
{
#if CONFIG_DO_TEXTMODEMOUSE
  if (key == KEY_MOUSE) { handle_key_mouse(); return; }
#endif
#if (TGC_IS_CURSES) && (defined(KEY_RESIZE))
  if (key == KEY_RESIZE)
  { /* might happen if the curses library installed its own resize signal
       handler and our MIGHT_SIG_TERMSIZE was 0 */
    if (key_handling_mode == khmLineInput) line_input_resize();
#if CONFIG_MENUS
    else if (key_handling_mode == khmMenu) cm_cancel();
#endif
    window_redraw_all();
    return;
  }
#endif

  remap_key(&key);
  if (is_khm_command)
  { if (key != KEY_CANCEL)
    { tWindow* window = current_window_x;
      const tWindowSpec* spec = window->spec;
      tBoolean (*func)(tWindow*, tKey) = spec->handle_key;
      tBoolean might_try_key = cond2boolean(func != NULL);
      tMbsIndex idx;
      if ( (spec->flags & wsfWantKeyFirst) && (might_try_key) )
      { if ((func)(window, key)) return; /* done */
        else might_try_key = falsE; /* don't try again */
      }
      idx = lookup_command_key(key);
      if (idx >= 0)
      { if (handle_command_code(keymap_command_keys[idx].pcc)) return; }
      else if ( (might_try_key) && ((func)(window, key)) ) return; /* done */
      show_message( ( (idx >= 0) ? _("Keyboard command not handled") :
        _("Key not handled") ), truE);
    }
  }
#if TGC_IS_CURSES
  else if (key_handling_mode == khmLineInput) line_input_handle_key(key);
#if CONFIG_MENUS
  else if (key_handling_mode == khmMenu)
  { switch (key)
    { case KEY_ENTER: cm_apply(); break;
      case KEY_CANCEL: cm_cancel(); break;
      case KEY_UP: cm_line_up(); break;
      case KEY_DOWN: cm_line_down(); break;
      case KEY_PPAGE: cm_page_up(); break;
      case KEY_NPAGE: cm_page_down(); break;
    }
  }
#endif
#endif /* #if TGC_IS_CURSES */
}

#endif /* #if TGC_IS_CURSES */

static tBoolean rwd_cb_redir(const tRemainingWork* rw)
{ /* IMPLEMENTME: with scripting, we must make sure that the window and
     document aren't deallocated before we get here - use dhm! */
  tWindow* window = (tWindow*) (rw->data1);
  const tBrowserDocument* referrer = (const tBrowserDocument*) (rw->data2);
  const char* uri = (const char*) (rw->data3);
  wk_browser_prr(window, uri, prrfRedrawOne | prrfIsHttpRedirection, referrer);
  memory_deallocate(uri); return(falsE);
}

static void test_redirection(tWindow* window, tBrowserDocument* document)
{ const tResourceRequest* request;
  const tResource* resource;
  const char* redirection;
  tCantent* cantent;
  if ( (document->flags & wvfHandledRedirection) ||
       ( (cantent = document->cantent) == NULL ) ||
       ( (redirection = cantent->redirection) == NULL ) )
    return;
  document->flags |= wvfHandledRedirection;
  if (document->bddm == bddmSource) return; /* don't redirect */
  request = document->request;
  resource = ( (request != NULL) ? (request->resource) : NULL );
  if ( (resource != NULL) && (is_httplike(resource->protocol)) )
  { const tServerStatusCode ssc = resource->server_status_code;
    if ( (ssc == 301) || (ssc == 302) || (ssc == 303) || (ssc == 307) )
    { tRemainingWork* rw;
      setup_redirect: debugmsg("redirecting\n");
      rw = remaining_work_create(rwd_cb_redir); rw->data1 = window;
      rw->data2 = document; rw->data3 = my_strdup(redirection);
    }
  }
  else if (cantent->caf & cafHtmlRedirection) goto setup_redirect;
}


/** Custom connections */

#if CONFIG_CUSTOM_CONN

static const char strAccount[] = "account", strAppend[] = "append",
  strAscii[] = "ascii", strAuth[] = "auth", strBinary[] = "binary",
  strCdup[] = "cdup", strDelete[] = "delete", strDownload[] = "download",
  strLcGet[] = "get", strHash[] = "hash", strLabel[] = "label",
  strLcd[] = "lcd", strLs[] = "ls", strMkdir[] = "mkdir", strOpen[] = "open",
  strPass[] = "pass", strPut[] = "put", strPwd[] = "pwd", strQuote[] = "quote",
  strRestart[] = "restart", strRmdir[] = "rmdir", strSite[] = "site",
  strTask[] = "task", strUpload[] = "upload", strUser[] = "user",
  strExcl[] = "!", strSpaceOpeningBrace[] = " (", strClosingBrace[] = ")",
  strTooFewArguments[] = N_("Too few arguments\n");
#define strCd (strLcd + 1)

static int cc_hash_bytes = 1024; /* ("int" simply for my_atoi() compliance) */
static tBoolean cc_do_hash = falsE;

#if CONFIG_CONSOLE
#define cc_on_console (program_mode == pmConsole)
#else
#define cc_on_console (falsE)
#endif

my_enum1 enum
{ cccfNone = 0, cccfConsoleOnly = 0x01, cccfFtpOnly = 0x02, cccfDoQuote = 0x04,
  cccfDontReqConn = 0x08
} my_enum2(unsigned char) tCccFlags;

my_enum1 enum
{ ccccUnknown = -2, ccccAmbiguous = -1, ccccQuit = 0, ccccVersion = 1,
  ccccHelp = 2, ccccHash = 3, ccccLcd = 4, ccccTask = 5, ccccInfo = 6,
  ccccLabel = 7, ccccAuth = 8, ccccDownload = 9, ccccUpload = 10,
  ccccOpen = 11, ccccClose = 12, ccccQuote = 13, ccccUser = 14, ccccPass = 15,
  ccccAccount = 16, ccccCd = 17, ccccCdup = 18, ccccPwd = 19, ccccLs = 20,
  ccccAscii = 21, ccccBinary = 22, ccccDelete = 23, ccccMkdir = 24,
  ccccRmdir = 25, ccccRestart = 26, ccccGet = 27, ccccPut = 28,
  ccccAppend = 29, ccccSite = 30
#if OPTION_EXECEXT & EXECEXT_SHELL
  , ccccShell = 31
#endif
} my_enum2(signed char) tCccCode; /* "custom connection command code" :-) */
#define MAX_CCCC (31)
#define cccc_is_valid(cccc) ((cccc) >= 0)

typedef unsigned char tCccArgnum; /* roughly 0..100 */

typedef struct
{ const char *default_cmdstr, *helpstr;
  tCccFlags flags;
  tCccArgnum minarg, maxarg;
} tCccData;

static const tCccData cccdata[MAX_CCCC + 1] =
{ /* ccccQuit */ { strQuit, N_("quit retawq"), cccfConsoleOnly |
    cccfDontReqConn, 0, 0 },
  /* ccccVersion */ { strVersion, N_("print version information"),
    cccfDontReqConn, 0, 0 },
  /* ccccHelp */ { strHelp, N_("print help information"), cccfDontReqConn, 0,
    100 },
  /* ccccHash */ { strHash, N_("hashmarks on/off"), cccfDontReqConn, 0, 1 },
  /* ccccLcd */ { strLcd, N_("change local directory"), cccfConsoleOnly |
    cccfDontReqConn, 0, 1},
  /* ccccTask */ { strTask, N_("switch to <task-id>"), cccfConsoleOnly |
    cccfDontReqConn, 1, 1 },
  /* ccccInfo */ { strInfo, N_("show information about existing task(s)"),
    cccfConsoleOnly | cccfDontReqConn, 0, 100 },
  /* ccccLabel */ { strLabel, N_("label a task"), cccfConsoleOnly |
    cccfDontReqConn, 1, 2 },
  /* ccccAuth */ { strAuth, N_("start TLS/SSL handshake (p/c/s)"), cccfFtpOnly,
    0, 1 },
  /* ccccDownload */ { strDownload, N_("download via URL"), cccfConsoleOnly |
    cccfDontReqConn, 1, 2 },
  /* ccccUpload */ { strUpload, N_("upload via URL"), cccfConsoleOnly |
    cccfDontReqConn, 1, 2 },
  /* ccccOpen */ { strOpen, N_("open a connection to a server (via URL)"),
    cccfDontReqConn, 1, 1 },
  /* ccccClose */ { strClose, N_("close some connection(s)"),
    cccfDontReqConn, 0, 100 },
  /* ccccQuote */ { strQuote, N_("send a verbatim command"), cccfFtpOnly, 1,
    100 },
  /* ccccUser */ { strUser, N_("send username"), cccfFtpOnly, 0, 1 },
  /* ccccPass */ { strPass, N_("send password"), cccfFtpOnly, 0, 1 },
  /* ccccAccount */ { strAccount, N_("send account"), cccfFtpOnly, 1, 1 },
  /* ccccCd */ { strCd, N_("change remote directory"), cccfFtpOnly, 1, 1 },
  /* ccccCdup */ { strCdup, N_("change to remote parent directory"),
    cccfFtpOnly | cccfDoQuote, 0, 0 },
  /* ccccPwd */ { strPwd, N_("print remote working directory"), cccfFtpOnly |
    cccfDoQuote, 0, 0 },
  /* ccccLs */ { strLs, N_("list contents of remote directory"), cccfNone,
    0, 100 },
  /* ccccAscii */ { strAscii, N_("set file transfer type to ASCII"),
    cccfFtpOnly, 0, 0 },
  /* ccccBinary */ { strBinary, N_("set file transfer type to binary"),
    cccfFtpOnly, 0, 0 },
  /* ccccDelete */ { strDelete, N_("delete remote file"), cccfFtpOnly, 1, 1 },
  /* ccccMkdir */ { strMkdir, N_("make remote directory"), cccfFtpOnly, 1, 1 },
  /* ccccRmdir */ { strRmdir, N_("delete remote directory"), cccfFtpOnly, 1,1},
  /* ccccRestart */ { strRestart, N_("set file transfer restart marker"),
    cccfFtpOnly, 1, 1 },
  /* ccccGet */ { strLcGet, N_("receive one file"), cccfNone, 1, 2 },
  /* ccccPut */ { strPut, N_("upload one file"), cccfNone, 1, 2 },
  /* ccccAppend */ { strAppend, N_("append to remote file"), cccfNone, 1, 2 },
  /* ccccSite */ { strSite, N_("send site-specific command"), cccfFtpOnly |
    cccfDoQuote, 1, 100 },
  /* ccccShell */ { strExcl, N_("execute a shell command"), cccfConsoleOnly |
    cccfDontReqConn, 1, 100 }
};

typedef struct
{ const char* cmdstr; /* (sorted in strcmp() order) */
  tCccCode code;
} tCccXlat; /* "custom connection command translation" */

static const tCccXlat cccxlat[] =
{
#if OPTION_EXECEXT & EXECEXT_SHELL
  { strExcl, ccccShell },
#endif
  { strQm, ccccHelp }, /* abbr. alias for lazy users */
  { strAccount, ccccAccount },
  /* { strAppend, ccccAppend }, */
  { strAscii, ccccAscii },
#if OPTION_TLS
  { strAuth, ccccAuth },
#endif
  { strBinary, ccccBinary },
  { "bye", ccccQuit }, /* alias in "usual text console FTP clients" */
  /* { strCat, ccccCat }, -- print contents of remote file */
  { strCd, ccccCd },
  { strCdup, ccccCdup },
  { strClose, ccccClose },
  { strDelete, ccccDelete },
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  { strDownload, ccccDownload },
#endif
  { "exit", ccccQuit }, /* alias in "usual text console FTP clients" */
  { strLcGet, ccccGet },
  { strHash, ccccHash },
  { strHelp, ccccHelp },
  { strInfo, ccccInfo },
  { strLabel, ccccLabel },
  { strLcd, ccccLcd },
  { strLs, ccccLs },
  { strMkdir, ccccMkdir },
  { strOpen, ccccOpen },
  { strPass, ccccPass },
  /* { strPut, ccccPut }, */
  { strPwd, ccccPwd },
  { strQuit, ccccQuit },
  { strQuote, ccccQuote },
  { strRestart, ccccRestart },
  { strRmdir, ccccRmdir },
  { strSite, ccccSite },
  /* { strStop, ccccStop }, -- e.g. stop sequencer and/or close dconn */
  { strTask, ccccTask },
  /* { strUpload, ccccUpload }, */
  { strUser, ccccUser },
  { strVersion, ccccVersion }
};

#define MAXCMDSTRLEN (8) /* length of the command string "download" */

struct tCustomConnContext;
struct tCcSeqEntry;

my_enum1 enum
{ ccssUnknown = 0, ccssSend = 1, ccssFinalize = 2, ccssCancel = 3
} my_enum2(unsigned char) tCcSeqStep;

typedef void (*tCcSeqCallback)(const struct tCustomConnContext*,
  /*@null@*/ tResource* resource, struct tCcSeqEntry* entry, tCcSeqStep);

typedef struct tCcSeqEntry
{ /* struct tCcSeqEntry* next; */
  tCcSeqCallback callback;
  void* data; /* callback-specific information, e.g. an FTP command string */
  tBoolean did_send;
} tCcSeqEntry; /* "custom connection sequencer entry" */

typedef struct
{ tBoolean (*may_handle)(const struct tCustomConnContext*);
  tBoolean (*handle_cccc)(const struct tCustomConnContext*, tCccCode,
    const char**, tCccArgnum);
  tBoolean (*try_open)(const struct tCustomConnContext*);
  void (*do_open)(const struct tCustomConnContext*, tResourceRequest*);
  void (*do_start)(const struct tCustomConnContext*, unsigned char,
    const void*);
  void (*do_prepare_sequencer)(const struct tCustomConnContext*, const char*,
    tCcSeqCallback);
  void (*getput)(const struct tCustomConnContext*, unsigned char, const char*,
    const char*);
} tCustomConnOps;

typedef struct tCustomConnContext
{ const tCustomConnOps* ops;
  void* data; /* e.g. console task ID number */
} tCustomConnContext;

static one_caller tMbsIndex __ccc_do_lookup(const char* str)
{ my_binary_search(0, ARRAY_ELEMNUM(cccxlat) - 1,
    strcmp(str, cccxlat[idx].cmdstr), return(idx))
}

#define ccc_xlat(idx) ( ((idx) >= 0) ? cccxlat[idx].code : ccccUnknown )

static one_caller tCccCode __ccc_lookup(const char* str)
{ const tMbsIndex idx = __ccc_do_lookup(str);
  return(ccc_xlat(idx));
}

static one_caller tMbsIndex __ccc_do_lookup_prefix(const char* prefix,
  size_t prefixlen)
{ my_binary_search(0, ARRAY_ELEMNUM(cccxlat) - 1,
    strncmp(prefix, cccxlat[idx].cmdstr, prefixlen), return(idx))
}

static tMbsIndex ccc_minidx, ccc_maxidx, /* for use with ccccAmbiguous */
  ccc_abbridx; /* for abbreviated commands */

static one_caller tCccCode __ccc_lookup_prefix(const char* prefix)
{ const size_t prefixlen = strlen(prefix);
  const tMbsIndex idx = __ccc_do_lookup_prefix(prefix, prefixlen);
  if (idx >= 0) /* got a "candidate"; check ambiguity */
  { tMbsIndex minidx, maxidx;
    minidx = maxidx = idx;
    while ( (--minidx >= 0) &&
            (!strncmp(prefix, cccxlat[minidx].cmdstr, prefixlen)) )
    { /* loop */ }
    while ( (++maxidx < (tMbsIndex) (ARRAY_ELEMNUM(cccxlat))) &&
            (!strncmp(prefix, cccxlat[maxidx].cmdstr, prefixlen)) )
    { /* loop */ }
    minidx++; maxidx--;
    if (minidx < maxidx)
    { ccc_minidx = minidx; ccc_maxidx = maxidx; return(ccccAmbiguous); }
    else ccc_abbridx = idx;
  }
  return(ccc_xlat(idx));
}

static tCccCode ccc_lookup(const char* str)
{ tCccCode cccc = __ccc_lookup(str);
  ccc_abbridx = INVALID_INDEX;
  if (cccc == ccccUnknown) cccc = __ccc_lookup_prefix(str);
  return(cccc);
}

static char* ccs_ptr;
static tBoolean ccs_is_first;

static void ccs_append(const char* str)
{ char ch;
  if (ccs_is_first) ccs_is_first = falsE;
  else *ccs_ptr++ = ' ';
  while ( (ch = *str++) != '\0' ) *ccs_ptr++ = ch;
}

static void cc_concat(const char* cmd, const char** arg, tCccArgnum argnum,
  tBoolean append_crlf)
{ tCccArgnum i;
  ccs_ptr = strbuf; ccs_is_first = truE;
  if (cmd != NULL) ccs_append(cmd);
  for (i = 0; i < argnum; i++) ccs_append(arg[i]);
  if (append_crlf) { *ccs_ptr++ = '\r'; *ccs_ptr++ = '\n'; }
  *ccs_ptr = '\0';
}

static __my_inline void cc_do_start(const tCustomConnContext* context,
  unsigned char what, const void* whatever)
{ (context->ops->do_start)(context, what, whatever);
}

static __my_inline void cc_do_start_cmd(const tCustomConnContext* context,
  const char* cmd)
{ cc_do_start(context, 0, cmd);
}

static __my_inline void cc_start_cmd(const tCustomConnContext* context,
  const char* cmd, const char** arg, tCccArgnum argnum)
{ cc_concat(cmd, arg, argnum, truE); cc_do_start_cmd(context, strbuf);
}

static __my_inline void cc_start_rch(const tCustomConnContext* context,
  tResourceCommandHandshake rch)
{ cc_do_start(context, 1, MY_INT_TO_POINTER(rch));
}

#if OPTION_TLS
static __my_inline void cc_start_auth(const tCustomConnContext* context,
  tFtpTlsMethod ftm)
{ cc_do_start(context, 2, MY_INT_TO_POINTER(ftm));
}
#endif

static __my_inline void cc_sequencer_prepare(const tCustomConnContext* context,
  const char* cmd, tCcSeqCallback callback)
{ (context->ops->do_prepare_sequencer)(context, cmd, callback);
}

static void cc_sequencer_cleanup(const tCustomConnContext* context,
  tCcSeqEntry* entry, tResource* resource)
{ tCcSeqCallback callback = entry->callback;
  if (callback != NULL)
  { (callback)(context, resource, entry, (entry->did_send) ? ccssFinalize :
      ccssCancel);
    my_memclr(entry, sizeof(tCcSeqEntry));
  }
}

static void cc_seq_cb_ls(const tCustomConnContext* context,
  /*@null@*/ tResource* resource, tCcSeqEntry* entry, tCcSeqStep step)
{ const char* cmd;
  switch (step)
  { case ccssSend:
      if (resource != NULL) resource->flags |= rfCustomConnBd1;
      cmd = (const char*) entry->data;
      cc_do_start_cmd(context, cmd); memory_deallocate(cmd);
      entry->data = NULL; break;
    case ccssFinalize: case ccssCancel:
      if (resource != NULL) resource->flags &= ~rfCustomConnBdAny;
      __dealloc(entry->data); break;
  }
}

static void cc_seq_cb_retr(const tCustomConnContext* context,
  /*@null@*/ tResource* resource, tCcSeqEntry* entry, tCcSeqStep step)
{ const char* cmd;
  switch (step)
  { case ccssSend:
      if (resource != NULL) resource->flags |= rfDownload | rfCustomConnBd4;
      cmd = (const char*) entry->data;
      cc_do_start_cmd(context, cmd); memory_deallocate(cmd);
      entry->data = NULL; break;
    case ccssFinalize: case ccssCancel:
      if (resource != NULL)
      { sinking_data_deallocate(&(resource->sinking_data));
        resource->flags &= ~(rfDownload | rfCustomConnBdAny);
      }
      __dealloc(entry->data); break;
  }
}

static one_caller void ccc_execute(const tCustomConnContext* context,
  char* inputstr)
/* IMPLEMENTME further! */
{ static const char strCommaSpace[] = ", ";
  tCccCode cccc;
  const tCccData* data;
  tCccFlags cccf;
  enum { MAXNUM = 50 };
  const char *cmdstr, *_arg[MAXNUM], **arg, *sendcmd, *filename;
  char type_ch, numbuf[200], *temp = inputstr;
  tCccArgnum argnum = 0, i;
#if OPTION_TLS
  tFtpTlsMethod ftm;
#endif
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "ccc_execute(): data=%p, cmd=%s\n", context->data,
    inputstr);
  debugmsg(debugstrbuf);
#endif

  /* split arguments */
  splitloop:
  while ( (*temp == ' ') || (*temp == '\t') ) *temp++ = '\0';
  if (*temp != '\0')
  { if (argnum >= MAXNUM)
    { err_tma: cc_output_errstr(_("Too many arguments\n")); return; }
    _arg[argnum++] = temp++;
    while ( (*temp != ' ') && (*temp != '\t') && (*temp != '\0') ) temp++;
    if (*temp != '\0') goto splitloop;
  }
  if (argnum <= 0) return; /* nothing given at all */
  cmdstr = _arg[0]; arg = &(_arg[1]); argnum--;

  /* lookup command */
  cccc = ccc_lookup(cmdstr);
  if (cccc == ccccUnknown) /* try some last resorts (missing space on ?/!) */
  { if (*cmdstr == '?') cccc = ccccHelp;
#if OPTION_EXECEXT & EXECEXT_SHELL
    else if (*cmdstr == '!') cccc = ccccShell;
#endif
    else
    { err_unknown_cmd:
      cc_output_errstr(_("Unknown command. Try \"help\".\n")); return;
    }
    (_arg[0])++; arg = &(_arg[0]); argnum++;
  }
  else if (cccc == ccccAmbiguous)
  { tMbsIndex idx = ccc_minidx;
    tBoolean is_first = truE;
    cc_output_errstr(_("Ambiguous command - "));
    while (idx <= ccc_maxidx)
    { if (is_first) is_first = falsE;
      else cc_output_errstr(strCommaSpace);
      cc_output_errstr(cccxlat[idx].cmdstr); idx++;
    }
    cc_output_errstr("?\n"); return;
  }

  /* handle command */
  data = &(cccdata[cccc]); cccf = data->flags;
  if ( (cccf & cccfConsoleOnly) && (!cc_on_console) )
  { cc_output_errstr(_("Command only available in console mode\n")); return; }
  else if ( (!(cccf & cccfDontReqConn)) &&
            (!((context->ops->may_handle)(context))) )
  { cc_output_errstr(_("Not connected (or no task selected)\n")); return; }
  else if (argnum < data->minarg)
  { err_tfa: cc_output_errstr(_(strTooFewArguments)); return; }
  else if (argnum > data->maxarg) goto err_tma;
  else if (cccf & cccfDoQuote) /* a simple case */
  { cc_start_cmd(context, data->default_cmdstr, arg, argnum); return; }
  switch (cccc)
  { case ccccQuit: do_quit(); /*@notreached@*/ break;
    case ccccVersion:
      /* just in case someone needs the version number after the initial output
         has scrolled away, esp. in console mode... */
      cc_output_str(strCopyright); cc_output_str(strProgramLegalese); break;
    case ccccHelp:
      if (argnum <= 0) /* list all possible commands */
      { tMbsIndex idx;
        const tBoolean on_console = cond2boolean(cc_on_console);
        int maxlinewidth = STRBUF_SIZE / 2, linewidth = 80, maxnum, count,
          cmdwidth = MAXCMDSTRLEN + 1; /* 1 for space character */
        /* IMPLEMENTME: use the actual window/terminal width for linewidth! */
        if (linewidth > maxlinewidth) linewidth = maxlinewidth; /* sani... */
        else if (linewidth < 25) linewidth = 25; /* ...tize */
        if (!on_console) cmdwidth += 2; /* for brackets */
        maxnum = linewidth / (cmdwidth + 1); /* "+1" for separation char */
        count = 0; temp = strbuf;
        for (idx = 0; idx < (tMbsIndex) (ARRAY_ELEMNUM(cccxlat)); idx++)
        { const tCccXlat* xlat = &(cccxlat[idx]);
          const char *src = xlat->cmdstr, *origtemp;
          const tCccCode c = xlat->code;
          const tBoolean cannot_use = cond2boolean( (!on_console) &&
            (cccdata[c].flags & cccfConsoleOnly) );
          char ch;
          if (count > 0) *temp++ = ' ';
          origtemp = temp;
          if (cannot_use) *temp++ = '[';
          while ( (ch = *src++) != '\0' ) *temp++ = ch;
          if (cannot_use) *temp++ = ']';
          count++;
          if (count >= maxnum)
          { *temp++ = '\n'; *temp = '\0'; cc_output_str(strbuf);
            count = 0; temp = strbuf;
          }
          else { while (temp - origtemp < cmdwidth) *temp++ = ' '; /* fill */ }
        }
        if (count > 0) { *temp++ = '\n'; *temp = '\0'; cc_output_str(strbuf); }
      }
      else /* list specific help texts */
      { for (i = 0; i < argnum; i++)
        { const char* str = arg[i];
          const tCccCode c = ccc_lookup(str);
          cc_output_str(str);
          if (cccc_is_valid(c))
          { const tCccData* d = &(cccdata[c]);
            const char* defaultstr = d->default_cmdstr;
            tCccArgnum minarg = d->minarg, maxarg = d->maxarg;
            if (strcmp(str, defaultstr))
            { /* mention the non-alias, non-abbreviated command */
              cc_output_str(strSpaceOpeningBrace);
              if (ccc_abbridx >= 0)
              { const char* s = cccxlat[ccc_abbridx].cmdstr;
                if (strcmp(s, defaultstr))
                { sprint_safe(strbuf, "%s, ", s); cc_output_str(strbuf); }
              }
              cc_output_str(defaultstr);
              cc_output_str(strClosingBrace);
            }
            cc_output_str(strSpacedDash); cc_output_str(_(d->helpstr));
            if (maxarg >= MAXNUM) maxarg = MAXNUM - 1;
            if (minarg != maxarg) sprint_safe(numbuf, "..%d", maxarg);
            else *numbuf = '\0';
            sprint_safe(strbuf, " (%d%s)", minarg, numbuf);
            cc_output_str(strbuf);
          }
          else if (c == ccccAmbiguous)
          { tMbsIndex j;
            cc_output_str(strSpacedDash);
            for (j = ccc_minidx; j <= ccc_maxidx; j++)
            { if (j > ccc_minidx) cc_output_str(strCommaSpace);
              cc_output_str(cccxlat[j].cmdstr);
            }
            cc_output_str(strQm);
          }
          else
          { cc_output_str(strSpacedDash); cc_output_errstr(_(strUnknown)); }
          cc_output_str(strNewline);
        }
      }
      break;
    case ccccHash:
      if (argnum <= 0) /* toggle */
      { const char* val;
        flip_boolean(cc_do_hash);
        show_hashing:
        if (!cc_do_hash) val = _(strOff);
        else { sprint_safe(numbuf, strPercd, cc_hash_bytes); val = numbuf; }
        sprint_safe(strbuf, _("Hashmarks: %s\n"), val);
        cc_output_str(strbuf);
      }
      else
      { int bytes;
        const char* a = arg[0];
        if ( (a[0] == '?') && (a[1] == '\0') ) goto show_hashing;
        my_atoi(a, &bytes, NULL, MY_ATOI_INT_MAX);
        if (bytes < 256) cc_do_hash = falsE;
        else { cc_hash_bytes = bytes; cc_do_hash = truE; }
        goto show_hashing;
      }
      break;
#if OPTION_TLS
    case ccccAuth:
      if (argnum <= 0) ftm = ftmAutodetect;
      else
      { const char *str = arg[0], ch = *str++;
        if ( (!my_islower(ch)) || (*str != '\0') )
        { bad_auth: cc_output_errstr(_(strBadValue));
          cc_output_errstr(strNewline); break;
        }
        switch (ch)
        { case 'p': ftm = ftmAuthTls; break;
          case 'c': ftm = ftmAuthTlsDataclear; break;
          case 's': ftm = ftmAuthSsl; break;
          default: goto bad_auth; /*@notreached@*/ break;
        }
      }
      cc_start_auth(context, ftm); break;
#endif
    case ccccQuote:
      sendcmd = NULL; send_cmd: cc_start_cmd(context, sendcmd, arg, argnum);
      break;
    case ccccUser:
      if (argnum <= 0) { arg[0] = strAnonymous; argnum = 1; }
      sendcmd = strUser; goto send_cmd; /*@notreached@*/ break;
    case ccccPass:
      if (argnum <= 0) goto poke_context; /* must prompt user */
      else { sendcmd = strPass; goto send_cmd; }
      break;
    case ccccAccount: sendcmd = "ACCT"; goto send_cmd; /*@notreached@*/ break;
    case ccccCd: sendcmd = "CWD"; goto send_cmd; /*@notreached@*/ break;
    case ccccLs:
      cc_concat(strList, arg, argnum, truE);
      cc_sequencer_prepare(context, strbuf, cc_seq_cb_ls);
      cc_start_rch(context, rchFtpPasv);
      break;
    case ccccAscii:
      type_ch = 'A'; send_type: sprint_safe(strbuf, "TYPE %c\r\n", type_ch);
      cc_do_start_cmd(context, strbuf); break;
    case ccccBinary: type_ch = 'I'; goto send_type; /*@notreached@*/ break;
    case ccccDelete: sendcmd = "DELE"; goto send_cmd; /*@notreached@*/ break;
    case ccccMkdir: sendcmd = "MKD"; goto send_cmd; /*@notreached@*/ break;
    case ccccRmdir: sendcmd = "RMD"; goto send_cmd; /*@notreached@*/ break;
    case ccccRestart: sendcmd = "REST"; goto send_cmd; /*@notreached@*/ break;
    case ccccGet:
      if (argnum >= 2) filename = arg[1];
      else
      { const char* ptr = arg[0];
        filename = my_strrchr(ptr, '/');
        if (filename == NULL) filename = ptr;
        else { filename++; if (*filename == '\0') goto err_tfa; }
      }
      /* The remainder can only be done by a context-specific handler; e.g. the
         "full" local path might depend on a prior ccccLcd result. */
      (context->ops->getput)(context, 0, arg[0], filename);
      break;
    case ccccOpen:
      if (context->ops->try_open(context)) /* can open a new connection */
      { tUriData* uri = uri_parse(arg[0], NULL, NULL, NULL, 1);
        const tResourceProtocol rp = uri->rp;
        tResourceError re = uri->re;
        tResourceRequest* request;
        char* spfbuf;
        my_spf(strbuf, STRBUF_SIZE, &spfbuf, "%s%s\n", _(strUriColonSpace),
          uri->uri);
        cc_output_str(spfbuf); my_spf_cleanup(strbuf, spfbuf);
        if ( (re == reFine) && (!is_ftplike(rp)) ) re = reProtocol;
        if (re != reFine)
        { cc_output_errstr(strResourceError[re]);
          cc_output_errstr(strNewline); uri_put(uri); return;
        }
        request = request_create(); uri_attach(request->uri_data, uri);
        (context->ops->do_open)(context, request);
      }
      break;
    default: /* try the context-specific handler */
      poke_context:
      if (!((context->ops->handle_cccc)(context, cccc, arg, argnum)))
        goto err_unknown_cmd;
      break;
  }
}

#endif /* #if CONFIG_CUSTOM_CONN */


/** Console mode */

#if CONFIG_CONSOLE

static const char strTooManyTasks[] = N_("Too many tasks\n");

typedef /*signed*/ int tConsoleTaskNum; /* ("signed" for simplicity only) */
#define CONSOLE_TASK_INVALID (-1)

my_enum1 enum
{ ctsUnused = 0, ctsPromptable = 1, ctsBusy = 2, ctsAutomated = 3
} my_enum2(unsigned char) tConsoleTaskState;

typedef struct
{ tCustomConnContext context;
  tCcSeqEntry sequencer_entry;
  tResourceRequest* request;
  const char* label;
  size_t last_hash_bytecount;
  tConsoleTaskState state;
} tConsoleTask;

static tConsoleTask* console_task = NULL;
static tConsoleTaskNum console_task_num = 0,
  console_task_curr = CONSOLE_TASK_INVALID;
static tBoolean console_try_prompt = falsE;
#define console_prompting ( (console_task_curr == CONSOLE_TASK_INVALID) || \
  (console_task[console_task_curr].state == ctsPromptable) )
#define console_ctxidx ( (tConsoleTaskNum) (MY_POINTER_TO_INT(context->data)) )

static const tCustomConnOps console_ops; /* prototype */
static const tCustomConnContext console_dummyctx = { &console_ops,
  MY_INT_TO_POINTER(CONSOLE_TASK_INVALID) };
#define console_currctx ( (console_task_curr == CONSOLE_TASK_INVALID) ? &console_dummyctx : &(console_task[console_task_curr].context) )

my_enum1 enum
{ cpkNormal = 0, cpkPassword = 1, cpkOverwrite = 2
} my_enum2(unsigned char) tConsolePromptKind;
#define console_pk_disguising (console_pk == cpkPassword)

static tConsolePromptKind console_pk = cpkNormal;

static char* console_input = NULL;
static size_t console_input_len = 0, console_input_maxlen = 0;

static const char* console_cwd = NULL;
static char console_oac[4]; /* for the "(o)verwrite/(a)ppend/(c)ancel" keys */

#define task_id2idx(id)  ((id) - 1)
#define task_idx2id(idx) ((idx) + 1)

static void console_print_prompt(void)
{ const char *extra, *spacing;
  if (!console_prompting) return;
  if (console_task_curr == CONSOLE_TASK_INVALID) *strbuf2 = '\0';
  else
  { const char* using_tls;
#if OPTION_TLS
    const tResourceRequest* request = console_task[console_task_curr].request;
    const tResource* resource;
    if ( (request != NULL) && ( (resource = request->resource) != NULL ) &&
         (resource_in_tls(resource, falsE)) )
      using_tls = "|TLS";
    else
#endif
    { using_tls = strEmpty; }
    sprint_safe(strbuf2, "[%d%s]", task_idx2id(console_task_curr), using_tls);
  }
  switch (console_pk)
  { default: extra = strEmpty; break;
    case cpkPassword: extra = strPass; break;
    case cpkOverwrite: extra = console_oac; break;
  }
  spacing = ( (*extra == '\0') ? strEmpty : strSpace );
  sprint_safe(strbuf, "\n%s%s%s%s> ", strbuf2, spacing, extra, spacing);
  cc_output_str(strbuf); console_try_prompt = falsE;
}

static void console_task_print_info(tConsoleTaskNum idx, const char* prefix)
{ const tConsoleTask* task;
  tConsoleTaskState state;
  const tResourceRequest* request;
  const char *message, *label, *statestr, *using_tls;
  char* spfbuf;
  tBoolean has_label, is_error;
  if (idx == CONSOLE_TASK_INVALID) return; /* CHECKME: print error message? */
  task = &(console_task[idx]); request = task->request; state = task->state;
  label = task->label; has_label = cond2boolean(label != NULL);
  switch (state)
  { default: statestr = strEmpty; break;
    case ctsBusy: statestr = _("busy"); break;
    case ctsAutomated: statestr = _("automated"); break;
  }
#if OPTION_TLS
  if ((request->resource!=NULL) && (resource_in_tls(request->resource,falsE)))
    using_tls = " - TLS";
  else
#endif
  { using_tls = strEmpty; }
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, "%s%s %d%s%s%s%s%s%s - \"%s\"\n",
    null2empty(prefix), _("Task"), task_idx2id(idx), (has_label ? " (\"" :
    strEmpty), null2empty(label), (has_label ? "\")" : strEmpty), ( (*statestr
    != '\0') ? strSpacedDash : strEmpty ), statestr, using_tls,
    request->uri_data->uri); /*so much to print, so little time^Wspace... :-)*/
  cc_output_str(spfbuf); my_spf_cleanup(strbuf, spfbuf);
  message = calculate_reqresmsg(request, request->resource, 1, &is_error);
  if (*message != '\0') { cc_output_str(message); cc_output_str(strNewline); }
}

static void console_task_switch(tConsoleTaskNum idx, tBoolean did_create)
/* Task switch? Who's writing an operating system here?! :-) */
{ tConsoleTask* task = &(console_task[idx]);
  const tResourceRequest* request = task->request;
  const tResource* resource = request->resource;
  console_task_curr = idx;
  console_task_print_info(idx, (did_create ? _("Creating: ") :
    _("Switching to: ")));
  if (resource != NULL)
  { task->last_hash_bytecount = resource->bytecount;
    /* (to ensure that the user isn't "flooded" with hashmarks when making a
        task current which transferred lots of data while non-current) */
  }
}

#define CC_MAXTASKNUM (4096) /* just a "sane" limit (rather insane in fact) */

static tConsoleTaskNum console_task_create(void)
{ tConsoleTaskNum idx = 0, tempidx;
  /* try to find a free slot */
  while (idx < console_task_num)
  { if (console_task[idx].state == ctsUnused)
    { tConsoleTask* task;
      tCustomConnContext* context;
      pick_it: task = &(console_task[idx]); task->state = ctsBusy;
      context = &(task->context); context->ops = &console_ops;
      context->data = MY_INT_TO_POINTER(idx); goto out;
    }
    idx++;
  }
  /* no free slot left, allocate more memory; we start with very few (two)
     slots because most users will even only have _one_ task existing at any
     time; don't waste memory... */
  idx = tempidx = console_task_num;
  if (console_task_num < 2) console_task_num = 2;
  else if (console_task_num < CC_MAXTASKNUM) console_task_num <<= 1;
  else goto fail;
  console_task = memory_reallocate(console_task, console_task_num *
    sizeof(tConsoleTask), mapOther);
  while (tempidx < console_task_num)
  { my_memclr_var(console_task[tempidx]); tempidx++; }
  goto pick_it;
  fail: idx = CONSOLE_TASK_INVALID;
  out: return(idx);
}

static void console_task_remove(tConsoleTaskNum idx, tBoolean print_notice)
{ tConsoleTask* task = &(console_task[idx]);
  tResourceRequest* request = task->request;
  tResource* resource = ( (request != NULL) ? request->resource : NULL );
  if (print_notice) console_task_print_info(idx, _("Removing: "));
  cc_sequencer_cleanup(&(task->context), &(task->sequencer_entry), resource);
  request_remove(request); __dealloc(task->label);
  my_memclr_var(console_task[idx]); /* esp. setting state to ctsUnused */
  if (console_task_curr == idx) /* try to find a new "current" task */
  { console_task_curr = CONSOLE_TASK_INVALID; /* IMPLEMENTME: nicer task? */
    console_try_prompt = truE;
  }
}

static tConsoleTaskNum task_str2idx(const char* str)
{ tConsoleTaskNum id, idx;
  const char* tail;
  my_atoi(str, &id, &tail, MY_ATOI_INT_MAX);
  if ( (id <= 0) || (id > console_task_num) || (*tail != '\0') )
  { failed: idx = CONSOLE_TASK_INVALID; goto out; }
  idx = task_id2idx(id);
  if (console_task[idx].state == ctsUnused) goto failed;
  out:
  return(idx);
}

static void say_task_unknown(const char* str)
{ char* spfbuf;
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, "%s - %s\n", str, _(strUnknown));
  cc_output_errstr(spfbuf); my_spf_cleanup(strbuf, spfbuf);
}

static tBoolean console_may_handle(__sunused const tCustomConnContext* context
  __cunused)
{ return(cond2boolean(console_task_curr != CONSOLE_TASK_INVALID));
}

static void console_print_hashmarks(tConsoleTask* task,
  const tResource* resource)
{ size_t old, curr, diff, num, count, c;
  old = task->last_hash_bytecount; curr = resource->bytecount;
  if (curr <= old) return; /* "should not happen" */
  diff = curr - old; num = diff / cc_hash_bytes;
  if (num > 0)
  { task->last_hash_bytecount += num * cc_hash_bytes;
    while (num > 0)
    { char* buf = strbuf;
      if (num < STRBUF_SIZE / 2) count = num;
      else count = STRBUF_SIZE / 2;
      num -= count;
      for (c = 0; c < count; c++) *buf++ = '#';
      *buf = '\0'; cc_output_str(strbuf);
    }
  }
}

static tBoolean rwd_cb_ctr(const tRemainingWork* rw)
{ tConsoleTaskNum task_idx = (tConsoleTaskNum) MY_POINTER_TO_INT(rw->data1);
  console_task_remove(task_idx, cond2boolean(task_idx == console_task_curr));
  if (console_try_prompt) console_print_prompt();
  return(falsE);
}

static void console_schedule_task_removal(tConsoleTaskNum task_idx)
/* cf. schedule_request_removal() */
{ tRemainingWork* rw = remaining_work_create(rwd_cb_ctr);
  rw->data1 = MY_INT_TO_POINTER(task_idx);
}

#if CONFIG_EXTRA & EXTRA_DOWNLOAD

static void console_download_request_callback(void* _idx,
  tDhmNotificationFlags flags)
{ const tConsoleTaskNum task_idx = (tConsoleTaskNum) MY_POINTER_TO_INT(_idx);
  tConsoleTask* task = &(console_task[task_idx]);
  const tResourceRequest* request = task->request;
  tResource* resource = request->resource;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "console_drc(): idx=%d, dhmnf=%d, req=%p, res=%p\n",
    task_idx, flags, request, resource);
  debugmsg(debugstrbuf);
#endif
  if (flags & (dhmnfDataChange | dhmnfMetadataChange))
  { const tBoolean is_curr = cond2boolean(task_idx == console_task_curr);
    tResourceError re;
    if (resource != NULL)
    { re = resource->error;
      if (re != reFine)
      { if (is_curr)
        { const char* text;
#if OPTION_TLS
          if ( (re == reTls) && (tls_errtext(resource, strbuf)) ) text=strbuf;
          else
#endif
          { handle_re: text = _(strResourceError[re]); }
          cc_output_errstr(text); cc_output_errstr(strNewline);
        }
        remove_task: console_schedule_task_removal(task_idx); return;
      }
      else if (resource->flags & rfFinal)
      { if (is_curr) { cc_output_str(_(strDone)); cc_output_str(strNewline); }
        goto remove_task;
      }
      else if ( (cc_do_hash) && (is_curr) )
        console_print_hashmarks(task, resource);
    }
    else if ( (re = request->error) != reFine )
    { if (is_curr) goto handle_re;
      else goto remove_task;
    }
  }
  else if (flags & dhmnfAttachery) /* a resource was attached to the request */
  { dhm_notification_setup(resource, console_download_request_callback,
      _idx, dhmnfDataChange | dhmnfMetadataChange, dhmnSet);
  }
}

static one_caller void console_download(const char* from, const char* to)
{ unsigned char _cleanup = 0; /* "&1": <to>; "&2": <fd>; "&4": <prr_data> */
  tUriData* uri_data = uri_parse(from, NULL, NULL, NULL, 0);
  tResourceError re = uri_data->re;
  int fd SHUT_UP_COMPILER(-1), err, e;
  struct stat statbuf;
  mode_t mode;
  char* spfbuf;
  tPrrData prr_data;
  tResourceRequest* request;
  tConsoleTaskNum task_idx;
  tConsoleTask* task;

  if (re != reFine)
  { cc_output_errstr(_(strResourceError[re])); cc_output_errstr(strNewline);
    goto cleanup;
  }
  if (to == NULL)
  { to = uri2filename(uri_data);
    if (to == NULL)
    { cc_output_errstr(_(strTooFewArguments)); goto cleanup; }
    _cleanup |= 1;
  }

  /* CHECKME: apply finalize_path()? */
  if (*to == '/') { /* absolute path, don't change it */ }
  else if ( (to[0] == '~') && (to[1] == '/') )
  { my_spf(NULL, 0, &spfbuf, strPercsPercs, get_homepath(), to + 2);
    if (_cleanup & 1) memory_deallocate(to);
    to = my_spf_use(spfbuf); _cleanup |= 1;
  }
  else
  { my_spf(NULL, 0, &spfbuf, strPercsPercs, ( (console_cwd != NULL) ?
    console_cwd : my_getcwd() ), to);
    if (_cleanup & 1) memory_deallocate(to);
    to = my_spf_use(spfbuf); _cleanup |= 1;
  }
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, "%s \"%s\" -> \"%s\"\n",
    _(strUcDownload), uri_data->uri, to);
  cc_output_str(spfbuf); my_spf_cleanup(strbuf, spfbuf);

  /* IMPLEMENTME: don't use O_EXCL unconditionally, do my_stat() first, ask
     "may overwrite?" if file already exists! */
  fd = my_create(to, O_CREAT | O_TRUNC | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0)
  { e = ( (fd == -1) ? errno : 0 );
    failed_fd: inform_about_error(e, _(strCantDownload));
    goto cleanup;
  }
  _cleanup |= 2;
  if ( (err = my_fstat(fd, &statbuf)) != 0 )
  { e = ( (err == -1) ? errno : 0 ); goto failed_fd; }
  mode = statbuf.st_mode;
  if (!S_ISREG(mode))
  { e = ( (S_ISDIR(mode)) ? EISDIR : 0 ); goto failed_fd; }

  prr_setup(&prr_data, from, prrfNone); prepare_resource_request(&prr_data);
  _cleanup |= 4; request = prr_data.result; re = request->error;
  if (re != reFine)
  { inform_about_error(0, _(strResourceError[re])); goto cleanup; }

  task_idx = console_task_create();
  if (task_idx == CONSOLE_TASK_INVALID) /* "rare" */
  { cc_output_errstr(_(strTooManyTasks)); goto cleanup; }
  task = &(console_task[task_idx]); task->state = ctsAutomated;
  task->request = request; prr_data.result = NULL;
  console_task_switch(task_idx, truE);
  _cleanup &= ~2; /* we'll _use_ the fd, so don't close it */
  dhm_notification_setup(request, console_download_request_callback,
    MY_INT_TO_POINTER(task_idx), dhmnfDataChange | dhmnfMetadataChange |
    dhmnfAttachery, dhmnSet);
  download_queue(request, fd);
  cleanup:
  uri_put(uri_data);
  if (_cleanup & 1) memory_deallocate(to);
  if (_cleanup & 2) my_close(fd);
  if (_cleanup & 4) prr_setdown(&prr_data);
}

#endif /* #if CONFIG_EXTRA & EXTRA_DOWNLOAD */

#if OPTION_EXECEXT & EXECEXT_SHELL

static void console_execext_shell_request_callback(void* _idx,
  tDhmNotificationFlags flags)
{ const tConsoleTaskNum task_idx = (tConsoleTaskNum) MY_POINTER_TO_INT(_idx);
  tConsoleTask* task = &(console_task[task_idx]);
  const tResourceRequest* request = task->request;
  tResource* resource = request->resource;
  tResourceError re;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf,
    "console_esrc(): task_idx=%d, flags=%d, req=%p, res=%p\n",
    task_idx, flags, request, resource);
  debugmsg(debugstrbuf);
#endif
  if (flags & (dhmnfDataChange | dhmnfMetadataChange))
  { const tBoolean is_curr = cond2boolean(task_idx == console_task_curr);
    if (resource != NULL)
    { if ( (re = resource->error) != reFine )
      { handle_re: if (is_curr) inform_about_error(0, _(strResourceError[re]));
        do_remove: console_schedule_task_removal(task_idx);
      }
      else if (resource->flags & rfFinal)
      { if (is_curr) { cc_output_str(_(strDone)); cc_output_str(strNewline); }
        goto do_remove;
      }
    }
    else if ( (re = request->error) != reFine ) goto handle_re;
  }
  else if (flags & dhmnfAttachery) /* a resource was attached to the request */
  { resource->custconn_handle = _idx;
    dhm_notification_setup(resource, console_execext_shell_request_callback,
      _idx, dhmnfDataChange | dhmnfMetadataChange, dhmnSet);
  }
}

static one_caller void console_execext_shell(const char* cmd)
/* This is roughly a simplified variant of handle_lig_execext_shell(). */
{ tExecextShellData data;
  tExecextShellFlags esf = esfNone;
  tBoolean want_header = falsE;
  if (*cmd == ':') /* options given */
  { char ch;
    while ( (ch = *++cmd) != '\0' )
    { switch (ch)
      { case 'o': esf |= esfReadStdout; break;
        case 'e': esf |= esfReadStderr; break;
        case 'h': want_header = truE; break;
        case ' ': cmd++; goto end_of_options; /*@notreached@*/ break;
      }
    }
    end_of_options: {}
  }
  if (*cmd == '\0') { inform_about_error(0, _(strNoShellCmd)); return; }
  my_memclr_var(data); data.esf = esf; data.command = cmd;
  if (esf & (esfReadStdout | esfReadStderr))
  { tPrrData prr_data;
    tResourceRequest* request;
    tResourceError re;
    tConsoleTaskNum task_idx;
    tConsoleTask* task;
    prr_setup(&prr_data, strExecextShellColon /* dummy - CHECKME! */,
      prrfUpsfp4);
    prepare_resource_request(&prr_data);
    if ( (re = prr_data.result->error) != reFine )
    { inform_about_error(0, _(strResourceError[re]));
      cleanup_prr: prr_setdown(&prr_data); return;
    }
    task_idx = console_task_create();
    if (task_idx == CONSOLE_TASK_INVALID) /* "rare" */
    { cc_output_errstr(_(strTooManyTasks)); goto cleanup_prr; }
    task = &(console_task[task_idx]); task->state = ctsAutomated;
    task->request = data.request = request = prr_data.result;
    prr_data.result = NULL; prr_setdown(&prr_data);
    dhm_notification_setup(request, console_execext_shell_request_callback,
      MY_INT_TO_POINTER(task_idx), dhmnfDataChange | dhmnfMetadataChange |
      dhmnfAttachery, dhmnSet);
    console_task_switch(task_idx, truE);
  }
  if (want_header) data.writedata = execext_shell_header();
  resource_start_execext_shell(&data);
  if (want_header) my_spf_cleanup(strbuf2, data.writedata);
}

#endif /* #if OPTION_EXECEXT & EXECEXT_SHELL */

static tBoolean console_handle_cccc(__sunused const tCustomConnContext*
  context __cunused, tCccCode cccc, const char** arg, tCccArgnum argnum)
/* console-specific custom connection command handler; IMPLEMENTME further! */
{ static const char strNoTask[] = N_("no task\n");
  tBoolean retval = truE;
  tConsoleTaskNum task_idx;
  tConsoleTask* task;
  const char* str;
  char *spfbuf, *new_cwd;
  switch (cccc)
  { case ccccLcd: /* change/print local current directory */
      /* We don't "actually" change the directory, e.g. by calling chdir(). We
         only change an internal prefix string; that causes less trouble. */
      if (argnum <= 0) new_cwd = my_strdup(my_getcwd());
      else
      { const char* dir = arg[0];
        const size_t dirlen = strlen(dir);
        const tBoolean has_slash = cond2boolean( (dirlen > 0) &&
          (dir[dirlen - 1] == '/') );
        if (*dir == '/')
        { if (has_slash) new_cwd = my_strdup(dir);
          else
          { new_cwd = __memory_allocate(dirlen + 1 + 1, mapString);
            my_memcpy(new_cwd, dir, dirlen);
            new_cwd[dirlen] = '/'; new_cwd[dirlen + 1] = '\0';
          }
        }
        else if ( (dir[0] == '~') && (dir[1] == '/') )
        { my_spf(strbuf, STRBUF_SIZE, &spfbuf, strPercsPercs, get_homepath(),
            dir + 2);
          new_cwd = finalize_path(spfbuf); my_spf_cleanup(strbuf, spfbuf);
        }
        else
        { my_spf(strbuf, STRBUF_SIZE, &spfbuf, "%s%s%s", ((console_cwd != NULL)
            ? console_cwd : my_getcwd()), dir, has_slash ? strEmpty :strSlash);
          new_cwd = finalize_path(spfbuf); my_spf_cleanup(strbuf, spfbuf);
        }
      }
      /* IMPLEMENTME: test whether this directory exists at all? */
      __dealloc(console_cwd); console_cwd = new_cwd;
      my_spf(strbuf, STRBUF_SIZE, &spfbuf,
        _("Local current directory: \"%s\"\n"), console_cwd);
      cc_output_str(spfbuf); my_spf_cleanup(strbuf, spfbuf);
      break;
    case ccccTask: /* switch to a task */
      str = arg[0]; task_idx = task_str2idx(str);
      if (task_idx == CONSOLE_TASK_INVALID)
      { if ( (str[0] == '0') && (str[1] == '\0') )
          console_task_curr = CONSOLE_TASK_INVALID;
        else { stu: say_task_unknown(str); }
      }
      else console_task_switch(task_idx, falsE);
      break;
    case ccccInfo: /* print information about existing tasks */
      if (argnum <= 0) /* show all tasks */
      { tBoolean is_first = truE;
        for (task_idx = 0; task_idx < console_task_num; task_idx++)
        { if (console_task[task_idx].state != ctsUnused)
          { console_task_print_info(task_idx, NULL); is_first = falsE; }
        }
        if (is_first) { say_no_task: cc_output_str(_(strNoTask)); }
      }
      else /* show given tasks */
      { tCccArgnum i;
        for (i = 0; i < argnum; i++)
        { str = arg[i]; task_idx = task_str2idx(str);
          if (task_idx == CONSOLE_TASK_INVALID) goto stu;
          else console_task_print_info(task_idx, NULL);
        }
      }
      break;
    case ccccLabel:
      if (my_isdigit(*(arg[0])))
      { cc_output_errstr(_(strBadValue)); cc_output_errstr(strNewline); break;}
      if (argnum <= 1)
      { task_idx = console_task_curr;
        if (task_idx == CONSOLE_TASK_INVALID) goto say_no_task;
      }
      else
      { str = arg[1]; task_idx = task_str2idx(str);
        if (task_idx == CONSOLE_TASK_INVALID) goto stu;
      }
      task = &(console_task[task_idx]); my_strdedup(task->label, arg[0]);
      break;
    case ccccPass: /* the user wants to be prompted separately */
      console_pk = cpkPassword; break;
    case ccccClose:
      if (argnum <= 0)
      { if (console_task_curr == CONSOLE_TASK_INVALID)
          cc_output_errstr(_(strNoTask));
        else console_task_remove(console_task_curr, truE);
      }
      else
      { tCccArgnum i;
        for (i = 0; i < argnum; i++)
        { str = arg[i]; task_idx = task_str2idx(str);
          if (task_idx == CONSOLE_TASK_INVALID) goto stu; /*CHECKME: proceed?*/
          else console_task_remove(task_idx, truE);
        }
      }
      break;
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
    case ccccDownload:
      console_download(arg[0], ( (argnum >= 2) ? arg[1] : NULL )); break;
#endif
#if OPTION_EXECEXT & EXECEXT_SHELL
    case ccccShell:
      { const char* cmd;
        cc_concat(NULL, arg, argnum, falsE);
        cmd = my_strdup(strbuf); /* IMPROVEME! */
        console_execext_shell(cmd); memory_deallocate(cmd);
      }
      break;
#endif
    default: retval = falsE; break;
  }
  return(retval);
}

static tBoolean console_try_open(__sunused const tCustomConnContext* context
  __cunused)
{ const tBoolean retval = cond2boolean(console_task_num < CC_MAXTASKNUM);
  if (!retval) cc_output_errstr(_(strTooManyTasks));
  return(retval);
}

static void console_request_callback(void* _idx, tDhmNotificationFlags flags)
{ tConsoleTaskNum idx = (tConsoleTaskNum) MY_POINTER_TO_INT(_idx);
  tConsoleTask* task = &(console_task[idx]);
  tResourceRequest* request = task->request;
  tResource* resource = request->resource;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf,
    "console_request_callback(_idx=%p, idx=%d, req=%p, res=%p, flags=%d)\n",
    _idx, idx, request, resource, flags);
  debugmsg(debugstrbuf);
#endif
  if (flags & (dhmnfDataChange | dhmnfMetadataChange))
  { tResourceError re;
    if (resource != NULL)
    { if ( (re = resource->error) != reFine )
      { const char* text;
#if OPTION_TLS
        if ( (re == reTls) && (tls_errtext(resource, strbuf)) ) text = strbuf;
        else
#endif
        { handle_re: text = _(strResourceError[re]); }
        if (idx == console_task_curr) inform_about_error(0, text);
        console_schedule_task_removal(idx); goto out;
      }
    }
    else
    { if ( (re = request->error) != reFine ) goto handle_re;
    }
  }
  else if (flags & dhmnfAttachery) /* a resource was attached to the request */
  { dhm_notification_setup(resource, console_request_callback, _idx,
      dhmnfDataChange | dhmnfMetadataChange, dhmnSet);
    resource->flags |= rfCustomConn; resource->custconn_handle = _idx;
  }
  out:
  if (console_try_prompt) console_print_prompt();
}

static void console_do_open(__sunused const tCustomConnContext* context
  __cunused, tResourceRequest* request)
{ const tConsoleTaskNum idx = console_task_create();
  if (idx == CONSOLE_TASK_INVALID) /* "can't happen" */
  { request_remove(request); return; }
  console_task[idx].request = request; console_task_switch(idx, truE);
  dhm_notification_setup(request, console_request_callback,
    MY_INT_TO_POINTER(idx), dhmnfDataChange | dhmnfMetadataChange |
    dhmnfAttachery, dhmnSet);
  request_queue(request, rraCustomConn);
}

static void console_do_start(const tCustomConnContext* context,
  unsigned char what, const void* whatever)
{ tConsoleTask* task = &(console_task[console_ctxidx]);
  tResource* resource = task->request->resource;
  if ( (what == 2) && (resource->uri_data->rp != rpFtp) )
  { /* a _manual_ "auth" command can only work on "ftp", not "ftps" */
    cc_output_errstr(_(strResourceError[reProtocol]));
    cc_output_errstr(strNewline); return;
  }
  /* IMPLEMENTME: check whether already in TLS mode (if what == 2)! */
  task->state = ctsBusy; resource_custom_conn_start(resource, what, whatever);
}

static void console_do_prepare_sequencer(const tCustomConnContext* context,
  const char* cmd, tCcSeqCallback callback)
{ tConsoleTask* task = &(console_task[console_ctxidx]);
  tCcSeqEntry* entry = &(task->sequencer_entry);
  my_strdedup(entry->data, cmd); entry->callback = callback;
}

static const char *console_from = NULL, *console_to = NULL; /* paths */

static void console_setup_retr(const tCustomConnContext* context,
  const char* from, const char* to, tFileTreatment ft)
{ int fd, err, e;
  struct stat statbuf;
  mode_t mode;
  tConsoleTask* task;
  tResourceRequest* request;
  tResource* resource;
  if (ft == ftAppend) fd = my_open(to, O_WRONLY | O_APPEND);
  else
  { int cflags = O_CREAT | O_TRUNC | O_WRONLY;
    if (ft == ftDontOverwrite) cflags |= O_EXCL;
    fd = my_create(to, cflags, S_IRUSR | S_IWUSR);
  }
#if CONFIG_DEBUG
  e = errno;
  sprint_safe(debugstrbuf, "setup_retr(): ft=%d, fd=%d, errno=%d\n", ft, fd,
    errno);
  debugmsg(debugstrbuf); errno = e;
#endif
  if (fd < 0)
  { e = ( (fd == -1) ? errno : 0 );
    handle_re_file: inform_about_error(e, _(strResourceError[reFile]));
    return;
  }
  if ( (err = my_fstat(fd, &statbuf)) != 0 )
  { e = ( (err == -1) ? errno : 0 ); clre: my_close(fd); goto handle_re_file; }
  mode = statbuf.st_mode;
  if (!S_ISREG(mode)) { e = ( (S_ISDIR(mode)) ? EISDIR : 0 ); goto clre; }
  task = &(console_task[console_ctxidx]); request = task->request;
  resource = request->resource;
  sinking_data_deallocate(&(resource->sinking_data));
  sinking_data_mcleanup(); sinking_data.download_fd = fd;
  sinking_data.flags |= sdfIsDownloadFdValid;
  sinking_data_shift(&(resource->sinking_data));
  cc_concat(strRetr, &from, 1, truE);
  cc_sequencer_prepare(context, strbuf, cc_seq_cb_retr);
  cc_start_rch(context, rchFtpPasv);
}

static one_caller void console_handle_oac(const char* str)
{ const char ch = *str++;
  tFileTreatment ft;
  if ( (ch == '\0') || (*str != '\0') )
  { cc_output_errstr(_(strBadValue)); cc_output_errstr(strNewline); return; }
  if (ch == console_oac[0]) ft = ftMayOverwrite;
  else if (ch == console_oac[1]) ft = ftAppend;
  else { cc_output_str(_("Cancelled\n")); return; }
  console_setup_retr(console_currctx, console_from, console_to, ft);
}

static void console_getput(const tCustomConnContext* context,
  unsigned char what, const char* s1, const char* s2)
{ const char *from, *to;
  char* spfbuf;
  tBoolean must_dealloc_to = falsE;
  struct stat statbuf;
  int err, e;
  if (what == 0) /* get */
  { from = s1; to = s2;
    /* CHECKME: apply finalize_path()? */
    if (*to == '/') { /* absolute path, don't change it */ }
    else if ( (to[0] == '~') && (to[1] == '/') )
    { my_spf(NULL, 0, &spfbuf, strPercsPercs, get_homepath(), to + 2);
      to = my_spf_use(spfbuf); must_dealloc_to = truE;
    }
    else
    { while ( (to[0] == '.') && (to[1] == '/') ) to += 2;
      my_spf(NULL, 0, &spfbuf, strPercsPercs, ( (console_cwd != NULL) ?
        console_cwd : my_getcwd() ), to);
      to = my_spf_use(spfbuf); must_dealloc_to = truE;
    }
    my_spf(strbuf, STRBUF_SIZE, &spfbuf, "\"%s\" -> \"%s\"\n", from, to);
    cc_output_str(spfbuf); my_spf_cleanup(strbuf, spfbuf);
    err = my_stat(to, &statbuf);
    if (err != 0)
    { e = ( (err == -1) ? (errno) : 0 );
      if (e == ENOENT) console_setup_retr(context, from, to,ftDontOverwrite);
      else
      { handle_re_file: inform_about_error(e, _(strResourceError[reFile]));
        goto cleanup;
      }
    }
    else
    { static tBoolean did_init = falsE;
      const char* question;
      const mode_t mode = statbuf.st_mode;
      if (!S_ISREG(mode))
      { e = ( (S_ISDIR(mode)) ? EISDIR : 0 ); goto handle_re_file; }
      question = _("File exists - (o)verwrite, (a)ppend, (c)ancel?\n");
      if (!did_init) /* must initialize console_oac[] */
      { unsigned char c;
        const char* src = question;
        char ch, *dest = console_oac;
        for (c = 0; c <= 2; c++)
        { src = my_strchr(src, '(');
          if (src == NULL) /* "should not happen" */
          { bad_oac: cc_output_errstr(_(strResourceError[reHandshake]));
            cc_output_errstr(strNewline); goto cleanup;
          }
          ch = *++src;
          if (!my_islower(ch)) goto bad_oac;
          *dest++ = ch;
        }
        *dest = '\0'; did_init = truE;
      }
      __dealloc(console_from); console_from = my_strdup(from);
      __dealloc(console_to);
      if (!must_dealloc_to) console_to = my_strdup(to);
      else { console_to = to; must_dealloc_to = falsE; }
      cc_output_str(question); console_pk = cpkOverwrite;
    }
  }
  else /* put */
  { from = s2; to = s1;
  }
  cleanup:
  if (must_dealloc_to) memory_deallocate(to);
}

static const tCustomConnOps console_ops =
{ console_may_handle, console_handle_cccc, console_try_open,
  console_do_open, console_do_start, console_do_prepare_sequencer,
  console_getput
};

static void console_input_append(char ch)
{ if (console_input_len >= console_input_maxlen)
  { console_input_maxlen += 50;
    console_input = memory_reallocate(console_input, console_input_maxlen,
      mapString);
  }
  console_input[console_input_len++] = ch;
}

static void console_handle_res(tResource* resource, unsigned char what,
  void* _x)
{ const tConsoleTaskNum task_idx = (tConsoleTaskNum)
    MY_POINTER_TO_INT(resource->custconn_handle);
  tCustomConnPrintingData* x;
  tConsoleTask* task;
  tConsoleTaskState state;
  const char* text;
  tCcSeqEntry* sequencer_entry;
  tCcSeqCallback seq_cb;
  tBoolean has_seq, stop_seq;
  switch (what)
  { case 0: /* print something */
      if (console_task_curr != task_idx) return; /* user isn't interested */
      x = (tCustomConnPrintingData*) _x; text = x->text;
      if (x->ccpk == ccpkNetresp)
      { /* some text from an untrustable source; might contain arbitrary
           rubbish and maybe isn't '\0'-terminated, so let's be careful... */
        char ch, netresp[1005];
        unsigned char c;
        size_t count = x->len, netresplen = 0;
#define netresp_flush do { __netresp_append('\0'); cc_output_str(netresp); } while (0)
#define __netresp_append(ch) netresp[netresplen++] = (ch)
#define netresp_append(ch) do { if (netresplen > 1000) { netresp_flush; netresplen = 0; } __netresp_append(ch); } while (0)
        while (count-- > 0)
        { ch = *text++; c = (unsigned char) ch;
          if ( (!is_bad_uchar(c)) || (ch == '\n') ) netresp_append(ch);
        }
        if (netresplen > 0) netresp_flush;
#undef netresp_flush
#undef __netresp_append
#undef netresp_append
      }
      else
      { if (x->ccpk == ccpkNetcmd)
        { cc_output_str(_("Sending: "));
          if (strneqcase(text, "pass ", 5)) text = "PASS...\n"; /* disguise */
        }
        cc_output_str(text); /* IMPLEMENTME: coloring! */
      }
      break;
    case 1: /* unbusify */
      task = &(console_task[task_idx]); state = task->state;
      resource->bytecount = task->last_hash_bytecount = 0;
      sequencer_entry = &(task->sequencer_entry);
      seq_cb = sequencer_entry->callback;
      has_seq = cond2boolean(seq_cb != NULL);
      if (resource->flags & rfCustomConnStopSequencer)
      { stop_seq = truE; resource->flags &= ~rfCustomConnStopSequencer; }
      else stop_seq = falsE;
      if (state == ctsBusy)
      { task->state = ctsPromptable;
        resource->state = rsMsgExchange; /* CHECKME! */
        if ( (has_seq) && (!stop_seq) && (!(sequencer_entry->did_send)) )
        { sequencer_entry->did_send = truE;
          (seq_cb)(&(task->context), resource, sequencer_entry, ccssSend);
        }
        else
        { if (has_seq)
            cc_sequencer_cleanup(&(task->context), sequencer_entry, resource);
          if (console_task_curr == task_idx) console_try_prompt = truE;
        }
      }
#if CONFIG_DEBUG
      else
      { /* This "should not happen" unless the server sends a message without a
           prior command, e.g. an "idle timeout" message. No problem, nothing
           to do here. */
        sprint_safe(debugstrbuf, "BUG (maybe): console task state %p,%d,%d\n",
          resource, task_idx, state);
        debugmsg(debugstrbuf); cc_output_errstr(debugstrbuf);
      }
#endif
      break;
    case 2: /* print "connecting to..." or similar */
      if (console_task_curr == task_idx) /* the user might be interested */
      { tResourceRequest* request;
        const char* message;
        tBoolean is_error;
        task = &(console_task[task_idx]); request = task->request;
        message = calculate_reqresmsg(request, resource, 0, &is_error);
        if (*message != '\0')
        { cc_output_str(message); cc_output_str(strNewline); }
      }
      break;
    case 3: /* print a connect-related error message */
      if (console_task_curr == task_idx) /* the user might be interested */
      { const tResourceError* _re = (const tResourceError*) _x;
        cc_output_errstr(_(strResourceError[*_re]));
        cc_output_errstr(strNewline);
      }
      break;
    case 4: /* print hashmarks */
      if ( (cc_do_hash) && (console_task_curr == task_idx) )
      { task = &(console_task[task_idx]);
        console_print_hashmarks(task, resource);
      }
      break;
  }
  if (console_try_prompt) console_print_prompt();
}

static one_caller void console_input_interpret(void)
{ const tConsolePromptKind cpk = console_pk;
  if (console_input_len <= 0) return; /* nothing entered, nothing to do */
  console_input_append('\0'); /* (for simplicity) */
  console_input_len = 0; /* preparation for the next input line */
  my_write_str(fd_stdout, strNewline); console_pk = cpkNormal;
  if (cpk == cpkPassword)
  { const char* arg[1];
    arg[0] = console_input; cc_start_cmd(console_currctx, strPass, arg, 1);
  }
  else if (cpk == cpkOverwrite) console_handle_oac(console_input);
  else ccc_execute(console_currctx, console_input);
  console_try_prompt = truE;
}

static void console_keyboard_handler(__sunused void* data __cunused,
  __sunused tFdObservationFlags flags __cunused)
{ char ch;
  int err = my_read(fd_keyboard_input, &ch, sizeof(ch));
  if (err <= 0) { /* nothing; hope it's just a temporary failure... */ }
  else if (!console_prompting)
  { if (ch == '&') /* the user wants to get at a prompt */
    { console_task_curr = CONSOLE_TASK_INVALID; console_try_prompt = truE; }
  }
  else if (ch == '\n') console_input_interpret();
  else if (ch == config.console_backspace)
  { if (console_input_len > 0)
    { console_input_len--; my_write_str(fd_stdout, "\b \b"); }
  }
  else if (console_input_len <= STRBUF_SIZE / 2) /* (just a "sane" limit) */
  { const unsigned char c = (unsigned char) ch;
    if (!is_bad_uchar(c))
    { /* if ( (ch != ' ') || ( (console_input_len > 0) &&
        (console_input[console_input_len - 1] != ' ') ) ||
        (console_pk == cpkPassword) ) -- CHECKME! */
      { console_input_append(ch);
        if (console_pk_disguising) ch = '*';
        my_write(fd_stdout, &ch, 1);
      }
    }
  }
  if (console_try_prompt) console_print_prompt();
}

#endif /* #if CONFIG_CONSOLE */


/** Main entry point */

#if TGC_IS_CURSES

static void curses_keyboard_handler(__sunused void* data __cunused,
  __sunused tFdObservationFlags flags __cunused)
{ tKey key = getch();
#if CONFIG_PLATFORM == 1
  if (key != 0)
#endif
  { if (key != ERR) handle_key(key); }
}

#endif /* #if TGC_IS_CURSES */

#if CAN_HANDLE_SIGNALS
static void any2main_input_handler(__sunused void* data __cunused,
  __sunused tFdObservationFlags flags __cunused)
{ static char buf[4], count = 0;
  ssize_t err = my_read_pipe(fd_any2main_read, buf + count, 4 - count);
  if (err <= 0) return;
  count += err;
  if (count < 4) return; /* didn't yet get a complete four-byte sequence */
  count = 0; /* preparation for the next "message" */
  if (!strncmp(buf, strQuit, 4)) do_quit_sig(); /* terminated by a signal */
#if (TGC_IS_CURSES) && ( (HAVE_CURSES_RESIZETERM) || (defined(resizeterm)) )
  else if (*buf == 'w') /* resize the terminal */
  { const unsigned char* const temp = (const unsigned char*) buf,
      cols = temp[1], rows = temp[2];
    if ( (COLS != cols) || (LINES != rows) ) /* need resize */
    {
#if CONFIG_MENUS
      if (key_handling_mode == khmMenu) cm_cancel();
#endif
      (void) resizeterm(rows, cols); window_redraw_all();
      if (key_handling_mode == khmLineInput) line_input_resize();
    }
  }
#endif
#if CONFIG_CONSOLE
  else if (!strncmp(buf, strConsoleDiscard, 4)) /* provide a fresh prompt */
  { if (!console_prompting) console_task_curr = CONSOLE_TASK_INVALID;
    else { console_input_len = 0; console_pk = cpkNormal; } /* discard input */
    console_print_prompt();
  }
#endif
}
#endif

#if CONFIG_TG == TG_XCURSES

#if MIGHT_USE_SCROLL_BARS
static void scroll_bar_calc(tRendererData* data)
{ tLinenumber* _h = data->line_callback_data;
  *_h = *_h + 1;
}
#endif

void xcurses_confuser(unsigned char a, void* b, void* c)
/* quick end dirty - you know the Latin abbr. "q.e.d."? :-) */
{ switch (a)
  { case 0: /* paste text */
      if ( (key_handling_mode == khmLineInput) &&
           (lid_area(flags) & liafEditable) )
      { const char* text = (const char*) b;
        unsigned long size = *((const unsigned long*) c);
        while (size-- > 0) line_input_paste_char(*text++);
      }
      break;
#if MIGHT_USE_SCROLL_BARS
    case 1: /* handle a program command code */
      { static const tProgramCommandCode cv[6] = { pccUnknown, pccLineUp,
          pccPageUp, pccUnknown, pccPageDown, pccLineDown };
        const unsigned char code = *((const unsigned char*) b);
        const tProgramCommandCode pcc = cv[code];
        if (pcc != pccUnknown) handle_command_code(pcc);
      }
      break;
    case 2: /* scroll to a specific line */
      { tBrowserDocument* document = current_document_x;
        if (document != NULL)
        { document->origin_y = (tLinenumber) (*((const int*) b));
          document_display(document, wrtRedraw);
        }
      }
      break;
    case 3: /* calculate scroll bar information */
      { tBrowserDocument* document = current_document_x;
        int* v = (int*) b;
        tLinenumber h;
        if ( (document == NULL) || (!document_section_height(document, &h)) )
        { v[0] = v[1] = v[2] = 0; return; }
        v[0] = (int) h; v[1] = document->origin_y;
        if (document->sbvi != COLS) /* must recalculate the document height */
        { tLinenumber height = 0;
          short mincol, maxcol;
          if (document_minmaxcol(document, &mincol, &maxcol))
          { tRendererData data;
            const short width = maxcol - mincol + 1;
            wrc_rd_setup(&data, document, width); data.flags |= rdfVirtual;
            data.line_callback_data = &height;
            data.line_callback = scroll_bar_calc;
            renderer_run(&data); document->sbdh = height; document->sbvi=COLS;
          }
        }
        v[2] = document->sbdh;
      }
      break;
#endif
  }
}

#endif /* #if CONFIG_TG == TG_XCURSES */

#if TGC_IS_WINDOWING
#include "wk.c"
#endif

#if CONFIG_CUSTOM_CONN
void main_handle_custom_conn(tResource* resource, unsigned char what, void* _x)
{
#if CONFIG_CONSOLE
  if (program_mode == pmConsole) console_handle_res(resource, what, _x);
  else
#endif
  { /* handle custom connection window */ }
}
#endif

#if TGC_IS_CURSES
static one_caller void cursor_reset_position(void)
{ if (key_handling_mode == khmLineInput)
    (void) move(lid_area(row), lid_area(colcurr));
  else (void) move( (current_window_index_x == 1) ? (VMIDDLE + 1) : 0, 0);
  __must_reset_cursor = falsE;
}
#endif

#if CONFIG_PLATFORM == 1
static const_after_init long chStdin;
#endif

static one_caller void __init main_initialize(void)
{ size_t count;
#if CAN_HANDLE_SIGNALS
  fd_observe(fd_any2main_read, any2main_input_handler, NULL, fdofRead);
#endif
  sinking_data_reset(); *sfconv = '%'; sfconv[3] = '\0';
#if CONFIG_MENUS & MENUS_UHIST
  my_memclr_arr(uri_history);
#endif
#if TGC_IS_GRAPHICS
  istrYesUc = my_strdup_ucfirst(_(strYes));
  istrNoUc  = my_strdup_ucfirst(_(strNo));
  default_font =
    gdk_font_load("-adobe-helvetica-*-r-normal--*-*-*-*-*-*-iso8859-1");
#endif

#if CONFIG_PLATFORM == 1
  if (is_promptable) chStdin = fgetchid(stdin);
#endif
  if (!is_environed) return; /* forget the rest */
  for (count = 0; count < ARRAY_ELEMNUM(keymap_command_defaultkeys); count++)
  { const tKeymapCommandEntry* x = &(keymap_command_defaultkeys[count]);
    (void) keymap_command_key_do_register(x->key, x->pcc);
  }
  qsort(keymap_command_keys, keymap_command_keys_num,
    sizeof(tKeymapCommandEntry), keymap_command_sorter);
  /* (The qsort() might be slow because the defaultkeys[] was already "quite
      sorted"; don't care much - it's only done once, and only at program
      initialization time, and the array isn't _that_ big:-) */
  for (count = 0; count < ARRAY_ELEMNUM(keymap_lineinput_defaultkeys); count++)
  { const tKeymapLineinputEntry* x = &(keymap_lineinput_defaultkeys[count]);
    (void) keymap_lineinput_key_do_register(x->key, x->liac);
  }
  qsort(keymap_lineinput_keys, keymap_lineinput_keys_num,
    sizeof(tKeymapLineinputEntry), keymap_lineinput_sorter);
}

int main(int argc, const char** argv)
{ /* getting start(l)ed... */
  initialize(argc, argv);
  main_initialize();

  if (is_environed)
  { unsigned char count;
#if CONFIG_SESSIONS
    { const char* filename = config.session_resume;
      if (filename != NULL) session_resume(filename);
    }
    if (windowlisthead == NULL) /* didn't actually resume anything */
#endif
    { if (launch_uri_count == 0)
      { /* The user didn't provide any URI on the command-line, so we use a
           default URI. We don't use the "home" URI because that might cause
           undesired network activity (which might cost money although the user
           maybe launched retawq only to view some local files); the user can
           go there by pressing "h", and I think that's acceptable. :-) */
        launch_uri[launch_uri_count++] = "about:retawq";
      }
    }
    for (count = 0; count < launch_uri_count; count++)
    { tWindow* w = wk_browser_create();
#if TGC_IS_CURSES
      if (count < 2) visible_window_x[count] = w;
#endif
      wk_browser_prr(w, launch_uri[count], prrfNone, NULL);
    }
    window_redraw_all();
  }
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  else if (program_mode == pmDownload)
    content_download_to_fd(config.pm_uri, NULL, 1);
#endif
#if CONFIG_EXTRA & EXTRA_DUMP
  else if (program_mode == pmDump) content_dump_auto(config.pm_uri);
#endif
#if CONFIG_CONSOLE
  else if (program_mode == pmConsole)
  { cc_output_str(strCopyright); cc_output_str(strProgramLegalese);
    if (initial_console_msgs != NULL)
    { cc_output_str(strNewline); cc_output_str(initial_console_msgs); }
    cc_output_str(_("\nEntering console runmode. Use the command \"help\" for a list of commands.\n"));
    console_print_prompt();
  }
  dealloc(initial_console_msgs);
#endif

#if TGC_IS_CURSES

#if CONFIG_PLATFORM != 1
  if (is_environed)
    fd_observe(fd_keyboard_input, curses_keyboard_handler, NULL, fdofRead);
#if CONFIG_CONSOLE
  else if (program_mode == pmConsole)
    fd_observe(fd_keyboard_input, console_keyboard_handler, NULL, fdofRead);
#endif
#endif

  while (1)
  { remaining_work_do();
    if (is_environed)
    { if (__must_reset_cursor) cursor_reset_position();
      (void) refresh();
    }
    i18n_cleanup
    fd_multiplex(); /* try to get some sleep :-) */
    /* Now disturb all the nice abstraction for very special cases... */
#if CONFIG_PLATFORM == 1
    if ( (is_promptable) && (io_pend(chStdin, 0) == 0) )
    {
#if CONFIG_CONSOLE
      if (program_mode == pmConsole)
      { /* console_keyboard_handler(NULL, 0); -- CHECKME: what instead? */ }
      else
#endif
      { curses_keyboard_handler(NULL, 0); }
    }
#elif CONFIG_TG == TG_BICURSES
    { /* This is necessary e.g. if the user just presses the Escape key and
         nothing further - the fd callback isn't executed further, so the
         timeout couldn't be detected. */
      tKey key = my_builtin_getch(falsE);
      if (key != ERR) handle_key(key);
    }
#endif
  }

#elif CONFIG_TG == TG_GTK

  if (is_environed) { need_tglib_cleanup = truE; gtk_main(); }
  else { while (1) { i18n_cleanup fd_multiplex(); } }

#endif /* tg */

  /*@notreached@*/ do_quit(); /*@notreached@*/
  return(0); /* avoid compiler warning... */
}
