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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysmacros.h>
#include "devname.h"

#include <asm/page.h>
#ifndef PAGE_SIZE
#define PAGE_SIZE (sizeof(long)*1024)
#endif

/* Who uses what:
 *
 * tty_to_dev   oldps, w (there is a fancy version in ps)
 * dev_to_tty   oldps, top, ps
 */

typedef struct c_syntax_requires_useless_crap {
  struct c_syntax_requires_useless_crap *next;
  unsigned int major_number;
  char name[4];
} tty_map_node;

static tty_map_node *tty_map = NULL;

/* Load /proc/tty/drivers for device name mapping use. */
static void load_drivers(void){
  char buf[10000];
  char *p;
  int fd;
  int bytes;
  fd = open("/proc/tty/drivers",O_RDONLY);
  if(fd == -1) goto fail;
  bytes = read(fd, buf, 9999);
  if(bytes == -1) goto fail;
  buf[bytes] = '\0';
  p = buf;
  while(( p = strstr(p, " /dev/tty") )){
    tty_map_node *tmn;
    int len;
    p += 9;
    len = strspn(p, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if(!len) continue;
    if(len>3) continue;
    if((len=1) && (*p=='S')) continue;
    tmn = malloc(sizeof(tty_map_node));
    tmn->next = tty_map;
    tty_map = tmn;
    memset(tmn->name, '\0', 4);
    strncpy(tmn->name, p, len);
    p += len;
    tmn->major_number = atoi(p);
  }
fail:
  if(fd != -1) close(fd);
  if(!tty_map) tty_map = (tty_map_node *)-1;
}

/* Try to guess the device name from /proc/tty/drivers info. */
static int driver_name(char * const buf, int maj, int min){
  struct stat sbuf;
  tty_map_node *tmn;
  if(!tty_map) load_drivers();
  if(tty_map == (tty_map_node *)-1) return 0;
  tmn = tty_map;
  for(;;){
    if(!tmn) return 0;
    if(tmn->major_number == maj) break;
    tmn = tmn->next;
  }
  sprintf(buf, "/dev/tty%s%d", tmn->name, min);  /* like "/dev/ttyZZ255" */
  if(stat(buf, &sbuf) < 0) return 0;
  if(min != minor(sbuf.st_rdev)) return 0;
  if(maj != major(sbuf.st_rdev)) return 0;
  return 1;
}

/* Try to guess the device name (we only need pre-2.2 devices) */
static int guess_name(char * const buf, int maj, int min){
  struct stat sbuf;
  int t0, t1;
  int tmpmin = min;
  switch(maj){
  case   4:
    if(min<64){
      sprintf(buf, "/dev/tty%d", min);
      break;
    }
    if(min<128){
      sprintf(buf, "/dev/ttyS%d", min-64);
      break;
    }
    tmpmin = min & 0x3f;  /* FALL THROUGH */
  case   3:      /* /dev/[pt]ty[p-za-o][0-9a-z] is 936 */
    t0 = "pqrstuvwxyzabcde"[tmpmin>>4];
    t1 = "0123456789abcdef"[tmpmin&0x0f];
    sprintf(buf, "/dev/tty%c%c", t0, t1);
    break;
  case  17:  sprintf(buf, "/dev/ttyH%d",  min); break;
  case  19:  sprintf(buf, "/dev/ttyC%d",  min); break;
  case  23:  sprintf(buf, "/dev/ttyD%d",  min); break;
  case  24:  sprintf(buf, "/dev/ttyE%d",  min); break;
  case  32:  sprintf(buf, "/dev/ttyX%d",  min); break;
  case  43:  sprintf(buf, "/dev/ttyI%d",  min); break;
  case  46:  sprintf(buf, "/dev/ttyR%d",  min); break;
  case  48:  sprintf(buf, "/dev/ttyL%d",  min); break;
  case  57:  sprintf(buf, "/dev/ttyP%d",  min); break;
  case  71:  sprintf(buf, "/dev/ttyF%d",  min); break;
  case  75:  sprintf(buf, "/dev/ttyW%d",  min); break;
  case  78:  sprintf(buf, "/dev/ttyM%d",  min); break;
  case 105:  sprintf(buf, "/dev/ttyV%d",  min); break;
  /* 136 ... 143 are /dev/pts/0, /dev/pts/1, /dev/pts/2 ... */
  case 136 ... 143:  sprintf(buf, "/dev/pts/%d",  min+(maj-136)*256); break;
  default: return 0;
  }
  if(stat(buf, &sbuf) < 0) return 0;
  if(min != minor(sbuf.st_rdev)) return 0;
  if(maj != major(sbuf.st_rdev)) return 0;
  return 1;
}

/* Linux 2.2 can give us filenames that might be correct.
 * Useful names could be in /proc/PID/fd/2 (stderr, seldom redirected)
 * and in /proc/PID/fd/255 (used by bash to remember the tty).
 */
static int fd_name(char * const buf, int maj, int min, int pid, int fd){
  struct stat sbuf;
  char path[32];
  int count;
  sprintf(path, "/proc/%d/fd/%d", pid, fd);  /* often permission denied */
  count = readlink(path,buf,PAGE_SIZE);
  if(count == -1) return 0;
  buf[count] = '\0';
  if(stat(buf, &sbuf) < 0) return 0;
  if(min != minor(sbuf.st_rdev)) return 0;
  if(maj != major(sbuf.st_rdev)) return 0;
  return 1;
}

/* number --> name */
int dev_to_tty(char *ret, int chop, int dev, int pid, unsigned int flags) {
  static char buf[PAGE_SIZE];
  char *tmp = buf;
  int i = 0;
  int c;
  if((short)dev == (short)-1) goto fail;
  if(    fd_name(tmp, major(dev), minor(dev), pid, 2  )) goto abbrev;
  if(    fd_name(tmp, major(dev), minor(dev), pid, 255)) goto abbrev;
  if( guess_name(tmp, major(dev), minor(dev)          )) goto abbrev;
  if(driver_name(tmp, major(dev), minor(dev)          )) goto abbrev;
fail:
  strcpy(ret, "?");
  return 1;
abbrev:
  if((flags&ABBREV_DEV) && !strncmp(tmp,"/dev/",5) && tmp[5]) tmp += 5;
  if((flags&ABBREV_TTY) && !strncmp(tmp,"tty",  3) && tmp[3]) tmp += 3;
  if((flags&ABBREV_PTS) && !strncmp(tmp,"pts/", 4) && tmp[4]) tmp += 4;
  tmp[chop] = '\0';
  for(;;){
    c = *tmp;
    tmp++;
    if(!c) break;
    i++;
    if(c<=' ') c = '?';
    if(c>126)  c = '?';
    *ret = c;
    ret++;
  }
  *ret = '\0';
  return i;
}

/* name --> number */
int tty_to_dev(char *name) {
  struct stat sbuf;
  static char buf[32];
  if(stat(name, &sbuf) >= 0) return sbuf.st_rdev;
  snprintf(buf,32,"/dev/%s",name);
  if(stat(buf, &sbuf) >= 0) return sbuf.st_rdev;
  snprintf(buf,32,"/dev/tty%s",name);
  if(stat(buf, &sbuf) >= 0) return sbuf.st_rdev;
  snprintf(buf,32,"/dev/pts/%s",name);
  if(stat(buf, &sbuf) >= 0) return sbuf.st_rdev;
  return -1;
}
