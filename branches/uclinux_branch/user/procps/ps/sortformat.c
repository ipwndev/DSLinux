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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* username lookups */
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "../proc/readproc.h"
#include "common.h"

static sf_node *sf_list = NULL;         /* deferred sorting and formatting */
static int broken;                      /* use gross Unix98 parsing? */
static int have_gnu_sort = 0;           /* if true, "O" must be format */
static int already_parsed_sort = 0;     /* redundantly set in & out of fn */
static int already_parsed_format = 0;


#define parse_sort_opt <-- arrgh! do not use this -->
#define gnusort_parse  <-- arrgh! do not use this -->


/****************  Parse single format specifier *******************/
static format_node *do_one_spec(const char *spec, const char *override){
  const format_struct *fs;
  const macro_struct *ms;

  fs = search_format_array(spec);
  if(fs){
    int w1, w2;
    format_node *thisnode;
    thisnode = malloc(sizeof(format_node));
    w1 = fs->width;
    if(override){
      w2 = strlen(override);
      thisnode->width = (w1>w2)?w1:w2;
      thisnode->name = malloc(strlen(override)+1);
      strcpy(thisnode->name, override);
    }else{
      thisnode->width = w1;
      thisnode->name = malloc(strlen(fs->head)+1);
      strcpy(thisnode->name, fs->head);
    }
    thisnode->pr = fs->pr;
    thisnode->pad = fs->pad;
    thisnode->vendor = fs->vendor;
    thisnode->flags = fs->flags;
    thisnode->next = NULL;
    return thisnode;
  }

  /* That failed, so try it as a macro. */
  ms = search_macro_array(spec);
  if(ms){
    format_node *list = NULL;
    format_node *newnode;
    const char *walk;
    int dist;
    char buf[16]; /* trust strings will be short (from above, not user) */
    walk = ms->head;
    while(*walk){
      dist = strcspn(walk, ", ");
      strncpy(buf,walk,dist);
      buf[dist] = '\0';
      newnode = do_one_spec(buf,override); /* call self, assume success */
      newnode->next = list;
      list = newnode;
      walk += dist;
      if(*walk) walk++;
    }
    return list;
  }

  fprintf(stderr,"spec value %s not found\n", spec);
  _exit(1);
}


/* must wrap user format in default */
static void O_wrap(sf_node *sfn, int otype){
  format_node *fnode;
  format_node *endp;
  char *trailer;

  trailer = (otype=='b') ? "END_BSD" : "END_SYS5" ;

  fnode =  do_one_spec("pid",NULL);
  if(!fnode)fprintf(stderr,"Seriously crashing. Goodbye cruel world.\n");
  endp = sfn->f_cooked; while(endp->next) endp = endp->next;  /* find end */
  endp->next = fnode;
  
  fnode =  do_one_spec(trailer,NULL);
  if(!fnode)fprintf(stderr,"Seriously crashing. Goodbye cruel world.\n");
  endp = fnode; while(endp->next) endp = endp->next;  /* find end */
  endp->next = sfn->f_cooked;
  sfn->f_cooked = fnode;
}

/*
 * Used to parse option O lists. Option O is shared between
 * sorting and formatting. Users may expect one or the other.
 * The "broken" flag enables a really bad Unix98 misfeature.
 * Put each completed format_node onto the list starting at ->f_cooked
 */
static const char *format_parse(sf_node *sfn){
  char *buf;                   /* temp copy of arg to hack on */
  char *sep_loc;               /* separator location: " \t,\n" */
  char *walk;
  const char *err;       /* error code that could or did happen */
  format_node *fnode;
  int items;
  int need_item;

  /*** prepare to operate ***/
  buf = malloc(strlen(sfn->sf)+1);
  strcpy(buf, sfn->sf);
  
  /*** sanity check and count items ***/
  need_item = 1; /* true */
  items = 0;
  walk = buf;
  err = "Improper format specifier list.";
  do{
    switch(*walk){
    case ' ': case ',': case '\t': case '\n': case '\0':
    /* Linux extension: allow \t and \n as delimiters */
      if(need_item){
        free(buf);
        return "Improper format list";
      }
      need_item=1;
      break;
    case '=':
      if(broken) goto out;
      /* fall through */
    default:
      if(need_item) items++;
      need_item=0;
    }
  } while (*++walk);
out:
  if(!items){
    free(buf);
    return "Empty format list.";
  }
#ifdef STRICT_LIST
  if(need_item){    /* can't have trailing deliminator */
    free(buf);
    return "Improper format list.";
  }
#else
  if(need_item){    /* allow 1 trailing deliminator */
    *--walk='\0';  /* remove the trailing deliminator */
  }
#endif
  /*** actually parse the list ***/
  walk = buf;
  while(items--){
    format_node *endp;
    char *equal_loc;
    sep_loc = strpbrk(walk," ,\t\n");
    /* if items left, then sep_loc is not in header override */
    if(items && sep_loc) *sep_loc = '\0';
    equal_loc = strpbrk(walk,"=");
    if(equal_loc){   /* if header override */
      *equal_loc = '\0';
      equal_loc++;
    }
    fnode =  do_one_spec(walk,equal_loc);
    if(!fnode){
      free(buf);
      return "Unknown user-defined format specifier.";
    }
    endp = fnode; while(endp->next) endp = endp->next;  /* find end */
    endp->next = sfn->f_cooked;
    sfn->f_cooked = fnode;
    walk = sep_loc + 1; /* point to next item, if any */
  }
  free(buf);
  already_parsed_format = 1;
  return NULL;
}




/*
 * Used to parse option AIX field descriptors.
 * Put each completed format_node onto the list starting at ->f_cooked
 */
static const char *aix_format_parse(sf_node *sfn){
  char *buf;                   /* temp copy of arg to hack on */
  char *walk;
  int items;

trace("aix_format_parse called\n");

  /*** sanity check and count items ***/
  items = 0;
  walk = sfn->sf;
  /* state machine */ {
  int c;
  initial:
    c = *walk++;
    if(c=='%')    goto get_desc;
    if(!c)        goto looks_ok;
  /* get_text: */
    items++;
  get_more_text:
    c = *walk++;
    if(c=='%')    goto get_desc;
    if(c)         goto get_more_text;
    goto looks_ok;
  get_desc:
    items++;
    c = *walk++;
    if(c)         goto initial;
    return "Improper AIX field descriptor.";
  looks_ok:
  }

  /*** sanity check passed ***/
  buf = malloc(strlen(sfn->sf)+1);
  strcpy(buf, sfn->sf);
  walk = sfn->sf;
  
  while(items--){
    format_node *fnode;  /* newly allocated */
    format_node *endp;   /* for list manipulation */

    if(*walk == '%'){
      const aix_struct *aix;
      walk++;
      if(*walk == '%') goto double_percent;
      aix = search_aix_array(*walk);
      walk++;
      if(!aix){
        free(buf);
        return "Unknown AIX field descriptor.";
      }
      fnode =  do_one_spec(aix->spec, aix->head);
      if(!fnode){
        free(buf);
        return "AIX field descriptor processing bug.";
      }
    } else {
      int len;
      len = strcspn(walk, "%");
      memcpy(buf,walk,len);
      if(0){
double_percent:
        len = 1;
        buf[0] = '%';
      }
      buf[len] = '\0';
      walk += len;
      fnode = malloc(sizeof(format_node));
      fnode->width = len;
      fnode->name = malloc(len+1);
      strcpy(fnode->name, buf);
      fnode->pr = NULL;     /* checked for */
      fnode->pad = 0;
      fnode->vendor = AIX;
      fnode->flags = 0;
      fnode->next = NULL;
    }
    
    endp = fnode; while(endp->next) endp = endp->next;  /* find end */
    endp->next = sfn->f_cooked;
    sfn->f_cooked = fnode;
  }
  free(buf);
  already_parsed_format = 1;
  return NULL;
}

/****************  Parse single sort specifier *******************/
static sort_node *do_one_sort_spec(const char *spec){
  const format_struct *fs;
  int reverse = 0;
  if(*spec == '-'){
    reverse = 1;
    spec++;
  }
  fs = search_format_array(spec);
  if(fs){
    sort_node *thisnode;
    thisnode = malloc(sizeof(format_node));
    thisnode->sr = fs->sr;
    thisnode->reverse = reverse;
    thisnode->next = NULL;
    return thisnode;
  }
  return NULL;   /* bad, spec not found */
}


/*
 * Used to parse long sorting options.
 * Put each completed sort_node onto the list starting at ->s_cooked
 */
static const char *long_sort_parse(sf_node *sfn){
  char *buf;                   /* temp copy of arg to hack on */
  char *sep_loc;               /* separator location: " \t,\n" */
  char *walk;
  const char *err;       /* error code that could or did happen */
  sort_node *snode;
  int items;
  int need_item;

  /*** prepare to operate ***/
  buf = malloc(strlen(sfn->sf)+1);
  strcpy(buf, sfn->sf);
  
  /*** sanity check and count items ***/
  need_item = 1; /* true */
  items = 0;
  walk = buf;
  err = "Improper sort specifier list.";
  do{
    switch(*walk){
    case ' ': case ',': case '\t': case '\n': case '\0':
      if(need_item){
        free(buf);
        return "Improper sort list";
      }
      need_item=1;
      break;
    default:
      if(need_item) items++;
      need_item=0;
    }
  } while (*++walk);
  if(!items){
    free(buf);
    return "Empty sort list.";
  }
#ifdef STRICT_LIST
  if(need_item){    /* can't have trailing deliminator */
    free(buf);
    return "Improper sort list.";
  }
#else
  if(need_item){    /* allow 1 trailing deliminator */
    *--walk='\0';  /* remove the trailing deliminator */
  }
#endif
  /*** actually parse the list ***/
  walk = buf;
  while(items--){
    sort_node *endp;
    sep_loc = strpbrk(walk," ,\t\n");
    if(sep_loc) *sep_loc = '\0';
    snode = do_one_sort_spec(walk);
    if(!snode){
      free(buf);
      return "Unknown sort specifier.";
    }
    endp = snode; while(endp->next) endp = endp->next;  /* find end */
    endp->next = sfn->s_cooked;
    sfn->s_cooked = snode;
    walk = sep_loc + 1; /* point to next item, if any */
  }
  free(buf);
  already_parsed_sort = 1;
  return NULL;
}






/************ pre-parse short sorting option *************/
/* Errors _must_ be detected so that the "O" option can try to
 * reparse as formatting codes.
 */
static const char *verify_short_sort(const char *arg){
  const char *all = "CGJKMNPRSTUcfgjkmnoprstuvy+-";
  char checkoff[256];
  int i;
  const char *walk;
  int tmp;
  if(strspn(arg,all) != strlen(arg)) return "Bad sorting code.";
  for(i=256; i--;) checkoff[i] = 0;
  walk = arg;
  for(;;){
    tmp = *walk;
    switch(tmp){
    case '\0':
      return NULL;   /* looks good */
    case '+':
    case '-':
      tmp = *(walk+1);
      if(!tmp || tmp=='+' || tmp=='-') return "Bad sorting code.";
      break;
    case 'P':
      if(forest_type) return "PPID sort and forest output conflict.";
      /* fall through */
    default:
      if(checkoff[tmp]) return "Bad sorting code.";   /* repeated */
      /* ought to check against already accepted sort options */
      checkoff[tmp] = 1;
      break;
    }
    walk++;
  }
}



/************ parse short sorting option *************/
/* must verify with short_sort_parse() before calling this */
static const char *short_sort_parse(sf_node *sfn){
  int direction = 0;
  const char *walk;
  int tmp;
  sort_node *snode;
  sort_node *endp;
  const struct shortsort_struct *ss;
  walk = sfn->sf;
  for(;;){
    tmp = *walk;
    switch(tmp){
    case '\0':
      already_parsed_sort = 1;
      return NULL;
    case '+':
      direction = 0;
      break;
    case '-':
      direction = 1;
      break;
    default:
      ss = search_shortsort_array(tmp);
      if(!ss) return "Unknown sort specifier.";
      snode = do_one_sort_spec(ss->spec);
      if(!snode) return "Unknown sort specifier.";
      snode->reverse = direction;
      endp = snode; while(endp->next) endp = endp->next;  /* find end */
      endp->next = sfn->s_cooked;
      sfn->s_cooked = snode;
      direction = 0;
      break;
    }
    walk++;
  }
}

/******************* high-level below here *********************/


/*
 * Used to parse option O lists. Option O is shared between
 * sorting and formatting. Users may expect one or the other.
 * The "broken" flag enables a really bad Unix98 misfeature.
 * Recursion is to preserve original order.
 */
static const char *parse_O_option(sf_node *sfn){
  const char *err;     /* error code that could or did happen */
  const char *err2;

  if(sfn->next){
    err = parse_O_option(sfn->next);
    if(err) return err;
  }

  switch(sfn->sf_code){
    case SF_B_o: case SF_G_format: case SF_U_o: /*** format ***/
      err = format_parse(sfn);
      trace("Foo!\n");
      if(err && strchr(sfn->sf,'%')) err = aix_format_parse(sfn);
      trace("Bar!\n");
      if(!err) already_parsed_format = 1;
      trace("Baz!\n");
      break;
    case SF_U_O:                                /*** format ***/
      /* Can have -l -f f u... set already_parsed_format like DEC does */
      if(already_parsed_format) return "option -O can not follow other format options.";
      err = format_parse(sfn);
      if(err && strchr(sfn->sf,'%')) err = aix_format_parse(sfn);
      if(err) return err;
      already_parsed_format = 1;
      O_wrap(sfn,'u'); /* must wrap user format in default */
      break;
    case SF_B_O:                                /***  both  ***/
      if(have_gnu_sort || already_parsed_sort) err = "Multiple sort options.";
      else err = verify_short_sort(sfn->sf);
      if(!err){ /* success as sorting code */
        short_sort_parse(sfn);
        already_parsed_sort = 1;
        return NULL;
      }
      if(already_parsed_format){
        err = "option O is neither first format nor sort order.";
        break;
      }
      err2 = format_parse(sfn);
      if(!err2){ /* success as format code */
        already_parsed_format = 1;
        O_wrap(sfn,'b'); /* must wrap user format in default */
        return NULL;
      }
      if(strchr(sfn->sf,'%')) err2 = aix_format_parse(sfn);
      if(!err2){ /* success as format code */
        already_parsed_format = 1;
        O_wrap(sfn,'b'); /* must wrap user format in default */
        return NULL;
      }
      break;
    case SF_G_sort: case SF_B_m:                 /***  sort  ***/
      if(already_parsed_sort) err = "Multiple sort options.";
      else err = long_sort_parse(sfn);
      already_parsed_sort = 1;
      break;
    default:                                    /***  junk  ***/
      return "Bug: parse_O_option got weirdness!";
  }
  return err; /* could be NULL */
}


/************ Main parser calls this to save lists for later **********/
/* store data for later and return 1 if arg looks non-standard */
int defer_sf_option(const char *arg, int source){
  sf_node *sfn;
  char buf[16];
  int dist;
  const format_struct *fs;
  int need_item = 1;

  sfn = malloc(sizeof(sf_node));
  sfn->sf = malloc(strlen(arg)+1);
  strcpy(sfn->sf, arg);
  sfn->sf_code = source;
  sfn->s_cooked = NULL;
  sfn->f_cooked = NULL;
  sfn->next = sf_list;
  sf_list = sfn;

  if(source == SF_G_sort) have_gnu_sort = 1;

  /* Now try to find an excuse to ignore broken Unix98 parsing. */
  if(source != SF_U_o) return 1;    /* Wonderful! Already non-Unix98. */
  do{
    switch(*arg){
    case ' ': case ',': case '\0':  /* no \t\n\r support in Unix98 */
      if(need_item) return 1;       /* something wrong */
      need_item=1;
      break;
    case '=':
      if(need_item) return 1;       /* something wrong */
      return 0;                     /* broken Unix98 parsing is required */
    default:
      if(!need_item) break;
      need_item=0;
      dist = strcspn(arg,", =");
      if(dist>15) return 1;         /* something wrong, sort maybe? */
      strncpy(buf,arg,dist);   /* no '\0' on end */
      buf[dist] = '\0';        /* fix that problem */
      fs = search_format_array(buf);
      if(!fs) return 1;             /* invalid spec, macro or sort maybe? */
      if(fs->vendor) return 1;      /* Wonderful! Legal non-Unix98 spec. */
    }
  } while (*++arg);

  return 0;                         /* boring, Unix98 is no change */
}

/* Since ps is not long-lived, the memory leak can be ignored. */
void reset_sortformat(void){
  sf_list = NULL;          /* deferred sorting and formatting */
  format_list = NULL;      /* digested formatting options */
  have_gnu_sort = 0;
  already_parsed_sort = 0;
  already_parsed_format = 0;
}


/* Search format_list for findme, then insert putme after findme. */
static int fmt_add_after(const char *findme, format_node *putme){
  format_node *walk;
  if(!strcmp(format_list->name, findme)){
    putme->next = format_list->next;
    format_list->next = putme;
    return 1; /* success */
  }
  walk = format_list;
  while(walk->next){
    if(!strcmp(walk->next->name, findme)){
      putme->next = walk->next->next;
      walk->next->next = putme;
      return 1; /* success */
    }
    walk = walk->next;
  }
  return 0; /* fail */
}

/* Search format_list for findme, then delete it. */
static int fmt_delete(const char *findme){
  format_node *walk;
  format_node *old;
  if(!strcmp(format_list->name, findme)){
    old = format_list;
    format_list = format_list->next;
    free(old);
    return 1; /* success */
  }
  walk = format_list;
  while(walk->next){
    if(!strcmp(walk->next->name, findme)){
      old = walk->next;
      walk->next = walk->next->next;
      free(old);
      return 1; /* success */
    }
    walk = walk->next;
  }
  return 0; /* fail */
}


/************ Build a SysV format backwards. ***********/
#define PUSH(foo) (fn=do_one_spec(foo, NULL), fn->next=format_list, format_list=fn)
static const char *generate_sysv_list(void){
  format_node *fn;
  if((format_modifiers & FM_y) && !(format_flags & FF_Ul))
    return "Modifier -y without format -l makes no sense.";
  if(prefer_bsd){
    if(format_flags) PUSH("cmd");
    else PUSH("args");
    PUSH("bsdtime");
    if(!(format_flags & FF_Ul)) PUSH("stat");
  }else{
    if(format_flags & FF_Uf) PUSH("cmd");
    else PUSH("ucmd");
    PUSH("time");
  }
  PUSH("tname");  /* Unix98 says "TTY" here, yet "tty" produces "TT". */
  if(format_flags & FF_Uf) PUSH("stime");
  if(format_flags & FF_Ul) PUSH("wchan");
  if(format_flags & FF_Ul){
    if(personality & PER_IRIX_l){ /* add "rss" then ':' here */
      PUSH("sgi_rss");
      fn = malloc(sizeof(format_node));
      fn->width = 1;
      fn->name = malloc(2);
      strcpy(fn->name, ":");
      fn->pr = NULL;     /* checked for */
      fn->pad = 0;
      fn->vendor = AIX;   /* yes, for SGI weirdness */
      fn->flags = 0;
      fn->next = format_list;
      format_list=fn;
    }
    PUSH("sz");
  }
  if(format_flags & FF_Ul){
    if(format_modifiers & FM_y) PUSH("rss");
    else if(personality & (PER_ZAP_ADDR|PER_IRIX_l)) PUSH("sgi_p");
    else PUSH("addr");
  }
  if(format_modifiers & FM_c){
    PUSH("pri"); PUSH("class");
  }else if(format_flags & FF_Ul){
    PUSH("ni"); PUSH("opri");
  }
  if((format_modifiers & FM_L) && (format_flags & FF_Uf)) PUSH("nlwp");
  if( (format_flags & (FF_Uf|FF_Ul)) && !(format_modifiers & FM_c) ) PUSH("c");
  if(format_modifiers & FM_P) PUSH("psr");
  if(format_modifiers & FM_L) PUSH("lwp");
  if(format_modifiers & FM_j){
    PUSH("sid");
    PUSH("pgid");
  }
  if(format_flags & (FF_Uf|FF_Ul)) PUSH("ppid");
  PUSH("pid");
  if(format_flags & FF_Uf){
    if(personality & PER_SANE_USER) PUSH("user");
    else PUSH("uid_hack");
  }else if(format_flags & FF_Ul){
    PUSH("uid");
  }
  if(format_flags & FF_Ul){
    PUSH("s");
    if(!(format_modifiers & FM_y)) PUSH("f");
  }
  if(format_modifiers & FM_M){
    PUSH("label");  /* Mandatory Access Control */
  }
  format_modifiers = 0;
  return NULL;
}


/**************************************************************************
 * Used to parse option O lists. Option O is shared between
 * sorting and formatting. Users may expect one or the other.
 * The "broken" flag enables a really bad Unix98 misfeature.
 */
const char *process_sf_options(int localbroken){
  const char *err;
  sf_node *sf_walk;
  int option_source;  /* true if user-defined */
  if(personality & PER_BROKEN_o) localbroken = 1;
  if(personality & PER_GOOD_o)   localbroken = 0;
  broken = localbroken;
  if(sf_list){
    err = parse_O_option(sf_list);
    if(err) return err;
  }

  if(format_list) printf("Bug: must reset the list first!\n");

  /* merge formatting info of sf_list into format_list here */
  sf_walk = sf_list;
  while(sf_walk){
    format_node *fmt_walk;
    fmt_walk = sf_walk->f_cooked;
    sf_walk->f_cooked = NULL;
    while(fmt_walk){   /* put any nodes onto format_list in opposite way */
      format_node *travler;
      travler = fmt_walk;
      fmt_walk = fmt_walk->next;
      travler->next = format_list;
      format_list = travler;
    }
    sf_walk = sf_walk->next;
  }

  /* merge sorting info of sf_list into sort_list here */
  sf_walk = sf_list;
  while(sf_walk){
    sort_node *srt_walk;
    srt_walk = sf_walk->s_cooked;
    sf_walk->s_cooked = NULL;
    while(srt_walk){   /* put any nodes onto sort_list in opposite way */
      sort_node *travler;
      travler = srt_walk;
      srt_walk = srt_walk->next;
      travler->next = sort_list;
      sort_list = travler;
    }
    sf_walk = sf_walk->next;
  }

  if(format_list){
    if(format_flags) return "Conflicting format options.";
    option_source = 1;
  }else{
    format_node *fmt_walk;
    format_node *fn;
    const char *spec;
    switch(format_flags){

    default:             return "Conflicting format options.";

    /* These can be NULL, which enables SysV list generation code. */
    case 0:              spec=NULL;           break;
    case FF_Uf | FF_Ul:  spec=sysv_fl_format; break;
    case FF_Uf:          spec=sysv_f_format;  break;
    case FF_Ul:          spec=sysv_l_format;  break;

    /* These are NOT REACHED for normal -j processing. */
    case FF_Uj:          spec=sysv_j_format;  break; /* Debian & Digital */
    case FF_Uj | FF_Ul:  spec="RD_lj";        break; /* Debian */
    case FF_Uj | FF_Uf:  spec="RD_fj";        break; /* Debian */

    /* These are true BSD options. */
    case FF_Bj:          spec=bsd_j_format;   break;
    case FF_Bl:          spec=bsd_l_format;   break;
    case FF_Bs:          spec=bsd_s_format;   break;
    case FF_Bu:          spec=bsd_u_format;   break;
    case FF_Bv:          spec=bsd_v_format;   break;

    /* These are old Linux options. Option m is overloaded. */
    case FF_LX:          spec="OL_X";         break;
    case FF_Lm:          spec="OL_m";         break;

    }  /* end switch(format_flags) */

    option_source = 0;
    if(!format_flags && !format_modifiers){   /* was default */
      char *tmp;
      tmp = getenv("PS_FORMAT");  /* user override kills default */
      if(tmp && *tmp){
        spec = tmp;
        option_source = 2;
      }
    }

    if(spec){
      fn = do_one_spec(spec, NULL); /* use override "" for no headers */
      fmt_walk = fn;
      while(fmt_walk){   /* put any nodes onto format_list in opposite way */
        format_node *travler;
        travler = fmt_walk;
        fmt_walk = fmt_walk->next;
        travler->next = format_list;
        format_list = travler;
      }
    }else{
      err = generate_sysv_list();
      if(err) return err;
      option_source = 3;
    }
  }
  if(format_modifiers){  /* generate_sysv_list() may have cleared some bits */
    format_node *fn;
    if(option_source) return "Can't use output modifiers with user-defined output";
    if(format_modifiers & FM_j){
      fn = do_one_spec("pgid", NULL);
      if(!fmt_add_after("PPID", fn)) if(!fmt_add_after("PID", fn))
        return "Internal error, no PID or PPID for -j option.";
      fn = do_one_spec("sid", NULL);
      if(!fmt_add_after("PGID", fn)) return "Lost my PGID!";
    }
    if(format_modifiers & FM_y){
      /* TODO: check for failure to do something, and complain if so */
      fmt_delete("F");
      fn = do_one_spec("rss", NULL);
      if(fmt_add_after("ADDR", fn)) fmt_delete("ADDR");
    }
    if(format_modifiers & FM_c){
      fmt_delete("%CPU"); fmt_delete("CPU"); fmt_delete("CP"); fmt_delete("C");
      fmt_delete("NI");
      fn = do_one_spec("class", NULL);
      if(!fmt_add_after("PRI", fn))
        return "Internal error, no PRI for -c option.";
      fmt_delete("PRI"); /* we want a different one */
      fn = do_one_spec("pri", NULL);
      if(!fmt_add_after("CLS", fn)) return "Lost my CLS!";
    }
  }
  if(!option_source){  /* OK to really muck with stuff */
    format_node *fn;
    /* Do personality-specific translations not covered by format_flags.
     * Generally, these only get hit when personality overrides unix output.
     * That (mostly?) means the Digital and Debian personalities.
     */
    if((personality & PER_ZAP_ADDR) && (format_flags & FF_Ul)){
      fn = do_one_spec("sgi_p", NULL);
      if(fmt_add_after("ADDR", fn)) fmt_delete("ADDR");
    }
    if((personality & PER_SANE_USER) && (format_flags & FF_Uf)){
      fn = do_one_spec("user", NULL);
      if(fmt_add_after("UID", fn)) fmt_delete("UID");
    }
  }

  /* Could scan for duplicates (format and sort) here. Digital does. */
  return NULL;
}

