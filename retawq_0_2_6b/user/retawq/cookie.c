/* retawq/cookie.c - HTTP state management
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

/* This code is part of the resource management; it's taken out for the only
   reason that source files are smaller this way. Look how it's #include'd from
   resource.c; maybe dirty, but simple, fast (inlined) and functional...
*/

#include <time.h> /* for ctime() */

#define COOKIE_STRICT_ATTRVALUES 0 /* grumble... */

/* We artificially limit the number and size of cookies; but our limits are
   much more generous than the minimum mentioned in RFC2965, 5.3, and really
   necessary to protect against servers that go nuts or even start attacks
   intentionally (RFC2965, 5.3.1). */
#define COOKIE_MAXNUM_PER_HOST (40) /* number of cookies per host */
#define COOKIE_MAXNUM_PORTS (100) /* number of port numbers per cookie */
#define COOKIE_MAXLEN (10 * 1024) /* length of cookie data strings */
#define COOKIE_MAXNUM_SEND (1000) /* number of cookies for "Cookie:" header */

#define is_cookie_whitespace(ch) \
  ( ((ch) == ' ') || ((ch) == '\t') || ((ch) == '\r') || ((ch) == '\n') )

enum
{ coofNone = 0, coofUseExpiry = 0x01, coofExpired = 0x02, coofNotToDisk = 0x04,
  coofSc2 = 0x08, coofPort = 0x10, coofPortvalues = 0x20, coofTlsOnly = 0x40
};
typedef unsigned char tCookieFlags;

typedef unsigned char tCookieVersion;

typedef struct tCookie
{ struct tCookie* next;
  char *name, *value, *path, *portstr, *comment;
  const char* domain;
  tPortnumber* portlist; /* (in network byte order) */
  time_t expiry;
  unsigned char portlistlen;
  tCookieVersion version;
  tCookieFlags flags;
} tCookie;


/* Helper functions */

static one_caller __sallocator /*@out@*/ tCookie* __callocator
  cookie_allocate(void)
{ return((tCookie*) __memory_allocate(sizeof(tCookie), mapOther));
}

static void __cookie_deallocate(const tCookie* cookie)
{ __dealloc(cookie->name); __dealloc(cookie->value); __dealloc(cookie->domain);
  __dealloc(cookie->path); __dealloc(cookie->portstr);
  __dealloc(cookie->comment); __dealloc(cookie->portlist);
}

static my_inline void cookie_deallocate(/*@only@*/ const tCookie* cookie)
{ __cookie_deallocate(cookie);
  memory_deallocate(cookie);
}

static tBoolean has_embedded_dots(const char* str)
{ size_t len = strlen(str);
  if (len < 3) return(falsE); /* can't have _any_ "embedded" thing */
  return(cond2boolean(my_strnchr(str + 1, '.', len - 2) != NULL));
}

static const char* effective_hostname(const char* hostname, tBoolean want_copy)
/* calculates the "effective hostname" of <hostname> (RFC2965, 1.) */
{ if (my_strchr(hostname, '.') != NULL) /* the most likely case */
  { if (want_copy) return(my_strdup(hostname));
    else return(hostname);
  }
  else
  { char* spfbuf;
    my_spf(NULL, 0, &spfbuf, "%s.local", hostname);
    return(my_spf_use(spfbuf));
  }
}

/* call this if you didn't want a copy from effective_hostname() */
#define effective_hostname_cleanup(orig, eff) \
  do { if (eff != orig) memory_deallocate(eff); } while (0)

static tBoolean is_hostname_numerical(const char* hostname)
/* returns whether a hostname "looks" like a numerical IP address; we can't
   simply check for a "num.num.num.num" pattern, e.g. because IPv6 addresses
   look different; and we also can't use things like inet_addr() or inet_pton()
   because that's not portable; however, the current algorithm is wrong too, so
   FIXME! */
{ const size_t len = strlen(hostname);
  char a, z; /* first and last character of <hostname> */
  return(cond2boolean( (len > 0) && ( (a = *hostname) >= '0' ) && (a <= '9')
    && ( (z = hostname[len - 1]) >= '0' ) && (z <= '9') ));
}

#define is_hostname_hdn(hostname) (!is_hostname_numerical(hostname))

static tBoolean cookie_domainmatch(const char* A, const char* B)
/* domain-match relation (RFC2965, 1.); case-insensitivity is given because all
   domain strings were converted to lowercase */
{ if (!strcmp(A, B)) return(truE);
  if ( (*B == '.') && is_suffix(A, B) && is_hostname_hdn(B + 1) &&
       is_hostname_hdn(A) )
    return(truE);
  return(falsE);
}

static one_caller tBoolean cookie_pathmatch(const char* P1, const char* P2)
/* path-match relation (RFC2965, 1.) - or was that "patch-math"? :-) */
{ const size_t len2 = strlen(P2);
  return(cond2boolean( (len2 <= strlen(P1)) && (!strncmp(P1, P2, len2)) ));
}

static tBoolean cookie_expired(tCookie* cookie)
{ tCookieFlags flags = cookie->flags;
  if ( (!(flags & coofExpired)) && (flags & coofUseExpiry) &&
       (cookie->expiry <= my_time()) )
  { cookie->flags |= coofExpired; flags = cookie->flags; }
  return(cond2boolean(flags & coofExpired));
}

static void cookie_remove(tCachedHostInformation* hostinfo,
  const tCookie* cookie)
/* detaches the <cookie> from the <hostinfo> cookie list and deallocates it */
{ tCookie *c = hostinfo->cookies, *next;
  if (c == NULL) { /* "should not happen" */ }
  else if (c == cookie) hostinfo->cookies = cookie->next;
  else
  { while ( (next = c->next) != NULL )
    { if (next == cookie) { c->next = cookie->next; break; }
      c = next;
    }
  }
  cookie_deallocate(cookie);
  if (hostinfo->cookiecount > 0) /* "should" be true */
    hostinfo->cookiecount--;
}

static one_caller void cookie_remove_expired(tCachedHostInformation* hostinfo)
/* removes all expired cookies from the hostinfo's cookie list */
{ tCookie* cookie = hostinfo->cookies;
  while (cookie != NULL)
  { tCookie* next = cookie->next;
    if (cookie_expired(cookie)) cookie_remove(hostinfo, cookie);
    cookie = next;
  }
}

static one_caller /*@null@*/ tCookie*
  cookie_lookup(const tCachedHostInformation* hostinfo, const tCookie* cookie)
/* returns a cookie from the hostinfo->cookies list which is "the same" as
   <cookie>; RFC2965, 3.3.3: "If a user agent receives a Set-Cookie2 response
   header whose NAME is the same as that of a cookie it has previously stored,
   the new cookie supersedes the old when: the old and new Domain attribute
   values compare equal, using a case-insensitive string-compare; and, the old
   and new Path attribute values string-compare equal (case-sensitive)." Case-
   insensitivity for domain strings is given because they were converted to
   lowercase. */
{ tCookie* c = hostinfo->cookies;
  if (c != NULL)
  { const char *name = cookie->name, *domain = null2empty(cookie->domain),
      *path = null2empty(cookie->path);
    while (c != NULL)
    { if ( (!strcmp(c->name, name)) &&
           (!strcmp(null2empty(c->domain), domain)) &&
           (!strcmp(null2empty(c->path), path)) )
        break; /* found */
      c = c->next;
    }
  }
  return(c);
}

static tBoolean cookie_config_allows(const tResource* resource)
/* returns whether the configuration allows to store/send/... any cookies for
   the <resource> */
{ tBoolean retval = falsE; /* default */
  const tConfigCookie* cc;
  const tCachedHostInformation* hostinfo;
  const char* hostname;
  switch (resource->protocol)
  { case rpHttp: cc = config.http_cookies; break;
#if OPTION_TLS
    case rpHttps: cc = config.https_cookies; break;
#endif
    default: cc = NULL; break;
  }
  if (cc == NULL) goto out; /* nothing allowed */
  if ( (hostinfo = resource2textual_host(resource)) == NULL ) goto out;
  if ( ( (hostname = hostinfo->hostname) == NULL ) || (*hostname == '\0') )
    goto out;
  while (cc != NULL)
  { const char* pattern = cc->hosts_pattern;
    if ( (pattern != NULL) && (my_pattern_matcher(pattern, hostname)) )
    { if (cc->flags & ccfAllowed) retval = truE;
      break;
    }
    cc = cc->next;
  }
  out:
  return(retval);
}


/* Expiry values (date-time parsing) */

static const struct
{ const char* name; /* (sorted in alphabetical order) */
  unsigned char monthnumber /*1..12*/, namelen,
    monthlength; /* (sorted in real-world order, just to confuse readers:-) */
} month_table[12] =
{ { "april",     4, 5,  31 },
  { "august",    8, 6,  28 },
  { "december", 12, 8,  31 },
  { "february",  2, 8,  30 },
  { "january",   1, 7,  31 },
  { "july",      7, 4,  30 },
  { "june",      6, 4,  31 },
  { "march",     3, 5,  31 },
  { "may",       5, 3,  30 },
  { "november", 11, 8,  31 },
  { "october",  10, 7,  30 },
  { "september", 9, 9,  31 }
};

#define nomod(y, d) ( (y % d) == 0 )
#define is_leapyear(y) ( nomod(y, 4) && (nomod(y, 400) || (!nomod(y, 100))) )

static one_caller unsigned char month_maxday(unsigned char month, /* 1..12 */
  unsigned short year)
/* maxday is the lexicographical predecessor of mayday... :-) */
{ unsigned char maxday = month_table[month - 1].monthlength;
  if ( (month == 2) && (is_leapyear(year)) ) maxday++; /* February */
  return(maxday);
}

static one_caller unsigned short dmy2yday(unsigned char day,
  unsigned char month /*1..12*/, unsigned short year)
/* transforms a date to the corresponding year-day */
{ static const unsigned short daysum[12] =
  { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
  unsigned short yday = daysum[month - 1];
  if ( (month > 2) && (is_leapyear(year)) ) yday++;
  yday += day;
  yday--; /* year-day values are zero-based ("days since January 1") */
  return(yday);
}

static one_caller tMbsIndex parse_month_long(const char* str)
{ my_binary_search(0, 11, streqcase3(str, month_table[idx].name),
    return(idx))
}

static one_caller tMbsIndex parse_month_short(const char* str)
{ my_binary_search(0, 11, strneqcase3(str, month_table[idx].name, 3),
    return(idx))
}

static const_after_init time_t localtime_offset = 0;

static one_caller tBoolean do_parse_datetime(const char* str,
  const char* pattern, /*@out@*/ time_t* _t)
{ const char *s = str, *p = pattern;
  time_t t, tm_year;
  char ch;
  unsigned short year, yday;
  unsigned char day, month, timepart[3], hour, minute, second;
  year = yday = 0;
  day = month = timepart[0] = timepart[1] = timepart[2]
      = hour = minute = second = 0;
  while ( (ch = *p++) != '\0' )
  { if (ch == 'w') /* weekday */
    { /* (We accept any combination of letters and ignore them.) */
      if (my_isalpha(*s)) { do { s++; } while (my_isalpha(*s)); }
      else goto failed;
    }
    else if (ch == 'W') /* _optional_ weekday, followed by "," and opt. ws */
    { if (my_isalpha(*s))
      { do { s++; } while (my_isalpha(*s));
        if (*s++ != ',') goto failed;
        while (is_cookie_whitespace(*s)) s++;
      }
    }
    else if (ch == 'd') /* day */
    { if (!my_isdigit(*s)) goto failed;
      day = *s++ - '0';
      if (my_isdigit(*s)) { day = 10 * day + (*s++ - '0'); }
    }
    else if (ch == 'm') /* month name */
    { tMbsIndex idx;
      if (!my_isalpha(*s)) goto failed; /* (quick pre-check) */
      idx = parse_month_long(s);
      if (idx >= 0) s += month_table[idx].namelen;
      else
      { idx = parse_month_short(s);
        if (idx < 0) goto failed;
        s += 3;
      }
      month = month_table[idx].monthnumber;
    }
    else if (ch == 'y') /* year */
    { /* Some servers send only a single digit as the year... For "low" year
         values, retawq follows RFC2822, 4.3 (which seems to be the most recent
         definition of this transformation): "If a two digit year is
         encountered whose value is between 00 and 49, the year is interpreted
         by adding 2000, ending up with a value between 2000 and 2049. If a two
         digit year is encountered with a value between 50 and 99, or any three
         digit year is encountered, the year is interpreted by adding 1900." */
      unsigned char i;
      if (!my_isdigit(*s)) goto failed;
      i = 0;
      yearloop:
      year = 10 * year + (*s++ - '0');
      if ( (i < 3) && (my_isdigit(*s)) ) { i++; goto yearloop; }
      if (year <= 49) year += 2000;
      else if (year <= 999) year += 1900;
      /* "else": hope it's usable, don't change it */
    }
    else if (ch == 't') /* time */
    { unsigned short i;
      handle_time:
      i = 0;
      timeloop: /* (Nah, not what you think...:-) */
      if (!my_isdigit(*s)) goto failed;
      timepart[i] = *s++ - '0';
      if (my_isdigit(*s)) timepart[i] = 10 * timepart[i] + (*s++ - '0');
      if ( (i < 2) && (*s == ':') ) { i++; s++; goto timeloop; }
    }
    else if (ch == 'T') /* _optional_ time at _end_ of string */
    { while (is_cookie_whitespace(*s)) s++;
      if (my_isdigit(*s)) goto handle_time;
    }
    else if (ch == ' ') /* whitespace */
    { if (is_cookie_whitespace(*s))
      { do { s++; } while (is_cookie_whitespace(*s)); }
      else goto failed;
    }
    else if (*s++ != ch) goto failed; /* exact character match required */
  }

  /* Parsing worked, now convert the data: */
  if (year < 2005) { t = 0; goto done; } /* in the past - ignore details */
  else if (year > 2020) year = 2020; /* "far enough" in the future for us */
  hour = timepart[0]; minute = timepart[1]; second = timepart[2];
  if (hour > 23) hour = 23;
  if (minute > 59) minute = 59;
  if (second > 59) second = 59;
  if (month <= 0) month = 1;
  else if (month > 12) month = 12;
  if (day <= 0) day = 1;
  else
  { const unsigned char maxday = month_maxday(month, year);
    if (day > maxday) day = maxday;
  }
  yday = dmy2yday(day, month, year);
  tm_year = (time_t) (year - 1900);

  /* The following algorithm is taken from SUSv3 ("Base Definitions -> General
     Concepts -> Seconds Since the Epoch"); we can't use library functions like
     mktime() because they are often buggy or not portable. */
  t = ((time_t) second) + ((time_t) minute)*60 + ((time_t) hour)*3600 +
      ((time_t) yday)*86400 + (tm_year-70)*31536000 + ((tm_year-69)/4)*86400 -
      ((tm_year-1)/100)*86400 + ((tm_year+299)/400)*86400;
  /* ...and the next line finally converts UTC/GMT to local time. */
  t += localtime_offset;

  done:
  *_t = t;
  return(truE);
  failed:
  return(falsE);
}

static tBoolean parse_datetime(const char* str, /*@out@*/ time_t* t)
/* tries to convert an "expires" cookie attribute value string to a time_t and
   returns whether that worked; we're very lenient in order to handle as many
   (buggy) real-world expiry strings as possible... */
{ enum { num = 3 };
  /* Currently, only the formats mentioned in RFC2616, 3.3.1, are handled: */
  static const char* const pattern[num] =
  { "Wd m yT",  /* RFC822/1123/2822: "[ day "," ] dd mm [yy]yy hh:mm:ss zzz" */
    "Wd-m-yT",  /* RFC850 (obsolete): "Weekday, DD-Mon-YY HH:MM:SS TIMEZONE" */
    "w m d t y" /* asctime(), ctime() */
  };
  unsigned char i;
  for (i = 0; i < num; i++)
  { if (do_parse_datetime(str, pattern[i], t)) return(truE);
  }
  return(falsE); /* couldn't convert it - format unknown */
}


/* Storing */

static one_caller tBoolean cookie_accept_any(const tResource* resource)
/* returns whether any cookies may be accepted (i.e. stored in a hostinfo
   cookie list) for the <resource> */
{
#if OPTION_LOCAL_CGI
#define not_local_cgi (resource->protocol != rpLocalCgi) &&
#else
#define not_local_cgi /* nothing */
#endif
  return(cond2boolean(cookie_config_allows(resource) && not_local_cgi /* && */
    (!(resource->flags & (rfIsRedirection | rfIsEmbedded))) ));
  /* (redirection/embedded: RFC2965, 3.3.6) */
#undef not_local_cgi
}

static one_caller tBoolean cookie_storable_c(const tCookie* cookie)
/* returns whether the cookie components have a decent size */
{ const char *name = cookie->name, *value = cookie->value,
    *domain = cookie->domain, *path = cookie->path, *portstr = cookie->portstr,
    *comment = cookie->comment;
  if ( (strlen(name) > COOKIE_MAXLEN) ||
       ( (value != NULL) && (strlen(value) > COOKIE_MAXLEN) ) ||
       ( (domain != NULL) && (strlen(domain) > COOKIE_MAXLEN) ) ||
       ( (path != NULL) && (strlen(path) > COOKIE_MAXLEN) ) ||
       ( (portstr != NULL) && (strlen(portstr) > COOKIE_MAXLEN) ) ||
       ( (comment != NULL) && (strlen(comment) > COOKIE_MAXLEN) ) )
    return(falsE); /* crumblet too big to swallow */
  return(truE);
}

static one_caller tBoolean cookie_storable_h(tCachedHostInformation* hostinfo)
/* returns whether there's enough room left in the hostinfo's cookie list to
   store one more cookie. */
{ if (hostinfo->cookiecount <= COOKIE_MAXNUM_PER_HOST) return(truE);
  else
  { /* Try to remove an expired cookie to get free room: */
    tCookie* c = hostinfo->cookies;
    tBoolean removed_one = falsE;
    while (c != NULL)
    { if (cookie_expired(c))
      { cookie_remove(hostinfo, c); removed_one = truE; break; }
      c = c->next;
    }
    return(removed_one);
  }
}

static void cookie_skip_whitespace(const char** _ptr)
{ const char* ptr = *_ptr;
  while (1)
  { const char ch = *ptr;
    if (!is_cookie_whitespace(ch)) break;
    ptr++;
  }
  *_ptr = ptr;
}

enum
{ canDontCare = 0, canComment = 1, canCommentUri = 2, canDiscard = 3,
  canDomain = 4, canExpires = 5, canMaxAge = 6, canPath = 7, canPort = 8,
  canSecure = 9, canVersion = 10
};
typedef unsigned char tCookieAttributeName;
#define MAX_CAN (10)

static const char* const strCan[MAX_CAN + 1] =
{ strA /*don't care*/, "comment", "commenturl", "discard", strDomain,
  "expires", "max-age", "path", "port", "secure", strVersion
};

static one_caller tMbsIndex cookie_do_lookup_attrname(const char* str)
{ my_binary_search(0, MAX_CAN, streqcase3(str, strCan[idx]), return(idx))
}

static one_caller tCookieAttributeName cookie_lookup_attrname(const char* str)
{ tMbsIndex idx = cookie_do_lookup_attrname(str);
  if (idx < 0) idx = 0; /* canDontCare */
  return((tCookieAttributeName) idx);
}

#if COOKIE_STRICT_ATTRVALUES
static tBoolean cookie_used_setcookie2;
static one_caller tBoolean is_tokenchar(unsigned char ch)
/* returns whether <ch> may appear in an HTTP token (RFC2616, 2.2) */
{ static const unsigned char notok[32] = { 255, 255, 255, 255, 5, 147, 0, 252,
   1, 0, 0, 56, 0, 0, 0, 168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  return(cond2boolean(!my_bit_test(notok, ch)));
}
#endif

static __sallocator /*@notnull@*/ char* __callocator
  cookie_token(const char** _ptr, tBoolean is_value, tBoolean is_expires_value)
/* returns a single cookie token (or quoted string) */
{ const char *ptr = *_ptr, *start, *ptr_wss = NULL;
  char ch, *retval;
  unsigned char* utemp;
  if ( (is_value) && (*ptr == '"') ) /* quoted string */
  { ptr++; start = ptr;
    while (1)
    { ch = *ptr;
      if ( (ch == '\0') || (ch == '"') ) /* reached end of string */
        break;
      ptr++;
    }
    if (ch == '"') ptr_wss = ptr + 1;
  }
  else /* unquoted token */
  { start = ptr;
#if COOKIE_STRICT_ATTRVALUES
    /* This is the strictly correct algorithm for RFC2965... */
    if (cookie_used_setcookie2)
    { while (is_tokenchar(*ptr)) ptr++;
    }
    else
#endif
    /* ...and this is the algorithm that should work with all those header
       lines in real life which contain unquoted attribute values with
       non-token characters, e.g. path=/ */
    { while (1)
      { ch = *ptr;
        if ( (ch == '\0') || (ch == ';') ||
             ( (!is_value) && ((ch == '=') || (is_cookie_whitespace(ch))) ) )
          break;
        if ( (ch == ',') && (!is_expires_value) )
        { /* Handle a horrible syntax definition bug: in RFC2109, "," is used
             as a separator of different cookies in a set-cookie header; but in
             Netscape's proposal there are "expires=..." attributes which can
             have a "," after a weekday name. We try to distinguish these cases
             this way: if is_expires_value, the cookie follows Netscape's
             proposal and not RFC2109 (because the latter doesn't define an
             "expires=..." attribute), so the "," isn't a cookie separator but
             a normal attrvalue character. So we _break_ on a "," only if we're
             _not_ inside an "expires=..." attrvalue. (Note that Netscape's
             proposal doesn't use "," for cookie separation because it doesn't
             define cookie separation at all.) */
          break;
        }
        ptr++;
      }
      if ( (is_value) && (ptr > start) ) /* trim trailing whitespace */
      { ptr--;
        while (ptr > start)
        { ch = *ptr;
          if (!is_cookie_whitespace(ch)) break;
          ptr--;
        }
        ptr++;
      }
    }
  }
  retval = my_strndup(start, ptr - start);
  utemp = (unsigned char*) retval;
  while (1)
  { const unsigned char uch = *utemp;
    if (uch == '\0') break;
    else if (is_control_char(uch)) *utemp = '?';
    utemp++;
  }
  if (ptr_wss != NULL) ptr = ptr_wss; /* setup for whitespace-skipping */
  cookie_skip_whitespace(&ptr);
  *_ptr = ptr;
  return(retval);
}

#define is_attr_set(can) my_bit_test(canbits, can)

static one_caller void cookie_handle_text(tResource* resource,
  const char* text, tBoolean used_setcookie2)
/* decides whether the cookie data (an HTTP header text snippet) may be stored
   in the hostinfo cookie list and stores it if so; this function may only be
   called if a former cookie_accept_any() call agreed. */
{ enum { canbytes = (((MAX_CAN + 1) + 7) / 8) };
  unsigned char canbits[canbytes]; /* bitfield */
  tCachedHostInformation* hostinfo = resource2textual_host(resource);
  tCookie cookie, *lcookie; /* ("l" as in "lookup") */

  if (hostinfo == NULL) return; /* "should not happen" */
#if COOKIE_STRICT_ATTRVALUES
  cookie_used_setcookie2 = used_setcookie2;
#endif

  /* Parse the HTTP header text for a single cookie */

  loop:
  my_memclr_var(cookie); my_memclr_arr(canbits);
  cookie.name = cookie_token(&text, falsE, falsE);
  if ( (cookie.name[0] == '\0') || (*text != '=') )
  { /* fundamental syntax error; ignore all the further text */
    memory_deallocate(cookie.name);
    return;
  }
  text++; cookie_skip_whitespace(&text);
  cookie.value = cookie_token(&text, truE, falsE);

  while (*text == ';')
  { char *attrname, *attrvalue;
    tCookieAttributeName can;

    text++; cookie_skip_whitespace(&text);
    attrname = cookie_token(&text, falsE, falsE);
    can = cookie_lookup_attrname(attrname);
    memory_deallocate(attrname);

    if (*text != '=') attrvalue = NULL;
    else
    { text++; cookie_skip_whitespace(&text);
      attrvalue = cookie_token(&text, truE, cond2boolean(can == canExpires));
    }

    if (is_attr_set(can))
    { /* RFC2965, 3.2.2: "If an attribute appears more than once in a cookie,
         the client SHALL use only the value associated with the first
         appearance of the attribute." */
      goto skip_attribute;
    }
    my_bit_set(canbits, can);

    switch (can)
    { case canComment: case canCommentUri:
        if ( (attrvalue != NULL) && (*attrvalue != '\0') )
        { tBoolean is_uri = cond2boolean(can == canCommentUri);
          const char* comment = cookie.comment;
          char* spfbuf;
          if (comment != NULL) /* already stored text or URI formerly */
          { my_spf(NULL, 0, &spfbuf, (is_uri ? "%s <%s>" : "%s %s"), comment,
              attrvalue);
            memory_deallocate(comment); /* forget old text */
            cookie.comment = my_spf_use(spfbuf); /* store new text */
          }
          else
          { if (is_uri)
            { my_spf(NULL, 0, &spfbuf, "<%s>", attrvalue);
              cookie.comment = my_spf_use(spfbuf);
            }
            else { cookie.comment = attrvalue; attrvalue = NULL; }
          }
        }
        break;
      case canDiscard: cookie.flags |= coofNotToDisk; break;
      case canDomain:
        if ( (attrvalue != NULL) && (*attrvalue != '\0') )
        { if ( (used_setcookie2) && (*attrvalue != '.') )
          { /* RFC2965, 3.2.2: we must prepend a dot */
            char* d = __memory_allocate(strlen(attrvalue) + 1 + 1, mapString);
            *d = '.'; my_strcpy_tolower(d + 1, attrvalue); cookie.domain = d;
          }
          else cookie.domain = my_strdup_tolower(attrvalue);
        }
        break;
      case canExpires:
        if ( (attrvalue != NULL) && (*attrvalue != '\0') &&
             (!is_attr_set(canMaxAge)) && (!used_setcookie2) )
        { /* (We don't let the old-fashioned canExpires override canMaxAge. And
              we don't handle canExpires attrvalues for set-cookie2 headers
              ("should not" appear there anyway, but...) due to the "," anomaly
              - see the is_expires_value comment above.) */
          time_t t;
          if (parse_datetime(attrvalue, &t))
          { cookie.expiry = t; cookie.flags |= coofUseExpiry; }
        }
        break;
      case canMaxAge:
        /* IMPLEMENTME: make this more precise, using Date and Age headers! */
        if ( (attrvalue != NULL) && (my_isdigit(*attrvalue)) )
        { int i;
          my_atoi(attrvalue, &i, NULL, MY_ATOI_INT_MAX);
          cookie.expiry = my_time() + ((time_t) i);
          cookie.flags |= coofUseExpiry;
        }
        break;
      case canPath: cookie.path = attrvalue; attrvalue = NULL; break;
      case canPort:
        cookie.flags |= coofPort;
        if (attrvalue != NULL) cookie.portstr = my_strdup(attrvalue);
        if ( (attrvalue != NULL) && (my_isdigit(*attrvalue)) )
        { char *s = attrvalue, *p; /* start, ptr */
          tPortnumber portnum[COOKIE_MAXNUM_PORTS];
          unsigned short portcount = 0;
          char ch;
          int i;
          next_port:
          p = s;
          while (1)
          { ch = *p;
            if (ch == '\0') break;
            else if (ch == ',') { *p = '\0'; break; }
            p++;
          }
          my_atoi(s, &i, NULL, 99999);
          if ( (i >= 0) && (i <= 65535) ) /* store it (unless duplicate) */
          { const tPortnumber port = (tPortnumber) htons((tPortnumber) i);
            tBoolean found = falsE;
            unsigned short count;
            for (count = 0; count < portcount; count++)
            { if (portnum[count] == port) { found = truE; break; }
            }
            if (!found) portnum[portcount++] = port;
          }
          if ( (ch != '\0') && (portcount < COOKIE_MAXNUM_PORTS) )
          { s = p + 1; goto next_port; }
          if (portcount > 0) /* actually got some port numbers */
          { const size_t size = portcount * sizeof(tPortnumber);
            tPortnumber* pn = (tPortnumber*) __memory_allocate(size, mapOther);
            my_memcpy(pn, portnum, size);
            cookie.portlist = pn; cookie.portlistlen = portcount;
            cookie.flags |= coofPortvalues;
          }
        }
        else if (used_setcookie2) /* RFC2965, 3.3.4, "Port Selection", 2. */
        { tPortnumber* p = cookie.portlist =
            (tPortnumber*) __memory_allocate(sizeof(tPortnumber), mapOther);
          *p = resource->uri_data->portnumber; cookie.portlistlen = 1;
        }
        break;
#if OPTION_TLS
      case canSecure:
        /* RFC2965, 3.2.2: "When it sends a "secure" cookie back to a server,
           the user agent SHOULD use no less than the same level of security as
           was used when it received the cookie from the server." */
        if (is_tlslike(resource->protocol)) cookie.flags |= coofTlsOnly;
        break;
#endif
      case canVersion:
        if (attrvalue != NULL)
        { int i;
          my_atoi(attrvalue, &i, NULL, 99);
          cookie.version = (tCookieVersion) i;
        }
        break;
    }
    skip_attribute:
    __dealloc(attrvalue);
  }

  /* Handle this cookie */

  if (cookie.name[0] == '$') goto ignore_cookie; /* RFC2965, 3.2.2 */
  if ( (used_setcookie2) && (!is_attr_set(canVersion)) )
    goto ignore_cookie; /* RFC2965, 3.3.2 */
  if (cookie.domain != NULL)
  { const char *domain = cookie.domain, *reshn;
    if (used_setcookie2)
    { if ( (!has_embedded_dots(domain)) && (!strcmp(domain, ".local")) )
        goto ignore_cookie; /* RFC2965, 3.3.2, *2 */
    }
    else
    { if ( (!has_embedded_dots(domain)) || (*domain != '.') )
        goto ignore_cookie; /* RFC2109, 4.3.2, *2 */
    }
    if ( (reshn = resource2textual_host(resource)->hostname) != NULL )
    { const char* eff = effective_hostname(reshn, falsE);
      tBoolean is_good = cookie_domainmatch(eff, domain);
      effective_hostname_cleanup(reshn, eff);
      if (!is_good) goto ignore_cookie; /* RFC2965, 3.3.2, *3 */
      if ( (is_hostname_hdn(reshn)) && (is_suffix(reshn, domain)) )
      { ssize_t pos = (ssize_t) (strlen(reshn) - strlen(domain));
        while (--pos >= 0)
        { if (reshn[pos] == '.') goto ignore_cookie; /* RFC2965, 3.3.2, *4 */
        }
      }
    }
  }
  if (cookie.path != NULL)
  { const char* uri_path = null2empty(resource->uri_data->path);
    const size_t plen = strlen(cookie.path), uri_plen = strlen(uri_path);
    if ( (plen > uri_plen) || (strncmp(cookie.path, uri_path, plen)) )
      goto ignore_cookie; /* RFC2965, 3.3.2, *1 */
  }
  if (cookie.portlist != NULL)
  { const tPortnumber resport = resource->uri_data->portnumber,
      *p = cookie.portlist;
    unsigned short i;
    tBoolean found = falsE;
    for (i = 0; i < cookie.portlistlen; i++)
    { if (p[i] == resport) { found = truE; break; } }
    if (!found) goto ignore_cookie; /* RFC2965, 3.3.2, *5 */
  }

  if (cookie.domain == NULL) /* use default (RFC2965, 3.3.1) */
  { const char* hostname = resource2textual_host(resource)->hostname;
    cookie.domain = effective_hostname(null2empty(hostname), truE);
  }
  if (cookie.path == NULL) /* use default (RFC2965, 3.3.1) */
  { const char *path = resource->uri_data->path, *slash;
    if ( (path != NULL) && ( (slash = my_strrchr(path, chDirsep)) != NULL ) )
      cookie.path = my_strndup(path, slash - path + 1);
    else cookie.path = my_strdup(strSlash);
  }
  if (used_setcookie2) cookie.flags |= coofSc2;

  if (!cookie_storable_c(&cookie)) goto ignore_cookie; /* too big */
  lcookie = cookie_lookup(hostinfo, &cookie);
  if (lcookie != NULL) /* a cookie with that name/... already exists */
  { if ( ( (cookie.flags & coofSc2) || (!(lcookie->flags & coofSc2)) ) &&
         (cookie.version >= lcookie->version) )
    { /* (The condition is a consequence of RFC2965, 9.1.) */
      resource->flags |= rfCookieStorer;
      if (cookie_expired(&cookie)) cookie_remove(hostinfo, lcookie); /*forget*/
      else { __cookie_deallocate(lcookie); *lcookie=cookie; goto next_cookie; }
    }
  }
  else /* it's a "new" cookie */
  { if ( (!cookie_expired(&cookie)) && (cookie_storable_h(hostinfo)) )
    { resource->flags |= rfCookieStorer;
      lcookie = cookie_allocate();
      *lcookie = cookie; lcookie->next = hostinfo->cookies;
      hostinfo->cookies = lcookie; hostinfo->cookiecount++;
      goto next_cookie;
    }
  }

  ignore_cookie:
  __cookie_deallocate(&cookie);

  /* Look for further cookies */

  next_cookie:
  if (*text == ',') { text++; cookie_skip_whitespace(&text); goto loop; }
}

#undef is_attr_set


/* Sending */

typedef struct
{ const char* text; /* the text to be sent for a single cookie */
  unsigned short domainlen; /* length of the cookie->domain string */
  unsigned short pathlen; /* length of the cookie->path string */
} tCookieSorterElement;

static int cookie_sorter(const void* _a, const void* _b)
/* sorts by domain length (voluntary) and path length (required by RFC2965,
   3.3.4) */
{ const tCookieSorterElement *a = (const tCookieSorterElement*) _a,
    *b = (const tCookieSorterElement*) _b;
  unsigned short alen = a->pathlen, blen = b->pathlen;
  if (alen != blen) return( ((int) blen) - ((int) alen) );
  return( ((int) b->domainlen) - ((int) a->domainlen) );
}

static one_caller const char* cookie_collect(tResource* resource)
/* collects all cookies which should be sent for the <resource> */
{ const char *retval = strEmpty, *reshn, *reseff, *respath;
  tHashIndex bucket;
  unsigned short count, maxcount; /* number of cookies to be sent */
  /*@relnull@*/ tCookieSorterElement* sorter_base; /* cookie texts */
  tCookieVersion highest_version;

  if (!cookie_config_allows(resource)) goto out0; /* the most likely case */
  if (resource->flags & (rfIsRedirection | rfIsEmbedded)) /* RFC2965, 3.3.6 */
    goto out0;

  reshn = resource2textual_host(resource)->hostname;
  if (reshn == NULL) goto out0; /* "should not happen" */
  reseff = effective_hostname(reshn, falsE);

  retval = my_strdup("Cookie2: $Version=\"1\"\r\n"); /* RFC2965, 3.3.5, 9.1 */
  sorter_base = NULL; count = maxcount = 0; highest_version = 0;
  respath = null2empty(resource->uri_data->path);

  /* Traverse the cookie lists of all hosts and collect applicable cookies */

  for (bucket = 0; bucket < HASHTABLESIZE_CHI; bucket++)
  { tCachedHostInformation* hostinfo = chi_head[bucket];
    while (hostinfo != NULL)
    { tCookie* cookie = hostinfo->cookies;
      tBoolean found_expired = falsE;
      while (cookie != NULL)
      { static const char strAttrPort[] = "; $Port";
        const char *domain, *path, *quote, *spfdomain, *spfpath, *spfport;
        char *spfbuf, *temp;
        unsigned char portlistlen;
        tCookieFlags flags;
        tCookieVersion version;

        /* Check whether this cookie should/may be sent */

        if (cookie_expired(cookie)) { found_expired = truE; goto cont; }
        flags = cookie->flags;
        if ( (flags & coofTlsOnly)
#if OPTION_TLS
             && (!is_tlslike(resource->protocol))
#endif
           )
          goto cont;
        domain = cookie->domain;
        if (!cookie_domainmatch(reseff, null2empty(domain))) goto cont;
        path = cookie->path;
        if (!cookie_pathmatch(respath, null2empty(path))) goto cont;

        portlistlen = cookie->portlistlen;
        if (portlistlen > 0)
        { tPortnumber resport = resource->uri_data->portnumber,
            *p = cookie->portlist;
          unsigned short i;
          tBoolean allowed = falsE;
          for (i = 0; i < portlistlen; i++)
          { if (p[i] == resport) { allowed = truE; break; }
          }
          if (!allowed) goto cont;
        }

        /* Okay, actually wanna send this cookie */

        if (count >= maxcount) /* need to allocate more space */
        { if (maxcount >= COOKIE_MAXNUM_SEND) goto out; /* too many cookies */
          maxcount += 20;
          sorter_base = memory_reallocate(sorter_base,
            maxcount * sizeof(tCookieSorterElement), mapOther);
        }
        version = cookie->version;
        quote = ( (version > 0) ? strDoubleQuote : strEmpty );
        if ( (domain == NULL) || (version == 0) ) spfdomain = strEmpty;
        else
        { my_spf(NULL, 0, &temp, "; $Domain=\"%s\"", domain);
          spfdomain = my_spf_use(temp);
        }
        if ( (path == NULL) || (version == 0) ) spfpath = strEmpty;
        else
        { my_spf(NULL, 0, &temp, "; $Path=\"%s\"", path);
          spfpath = my_spf_use(temp);
        }

        if (flags & coofPort)
        { const char* portstr = cookie->portstr;
          if (portstr == NULL) spfport = strAttrPort;
          else
          { my_spf(NULL, 0, &temp, "%s=\"%s\"", strAttrPort, portstr);
            spfport = my_spf_use(temp);
          }
        }
        else spfport = strEmpty;

        my_spf(NULL, 0, &spfbuf, "%s=%s%s%s%s%s%s", cookie->name, quote,
          cookie->value, quote, spfpath, spfdomain, spfport);
        my_spf_cleanup(strEmpty, spfdomain); my_spf_cleanup(strEmpty, spfpath);
        if ( (spfport != strEmpty) && (spfport != strAttrPort) )
          memory_deallocate(spfport);
        sorter_base[count].text = my_spf_use(spfbuf);
        sorter_base[count].domainlen = strlen(null2empty(domain));
        sorter_base[count].pathlen = strlen(null2empty(path));
        count++;
        if (highest_version < version) highest_version = version;
        cont:
        cookie = cookie->next;
      } /* cookies */
      if (found_expired) cookie_remove_expired(hostinfo);
      hostinfo = hostinfo->next;
    } /* hostinfos */
  } /* buckets */
  out:
  if (count > 0) /* actually send a "Cookie:" header */
  { char *spfbuf, *cookies, *dest;
    unsigned short cnt;
    size_t len = 0;
    resource->flags |= rfCookieSender;
    qsort(sorter_base, count, sizeof(tCookieSorterElement), cookie_sorter);
    for (cnt = 0; cnt < count; cnt++) len += strlen(sorter_base[cnt].text);
    len += 2 * (count - 1) + 1; /* for the "; " separators and trailing '\0' */
    cookies = dest = __memory_allocate(len, mapString);
    for (cnt = 0; cnt < count; cnt++)
    { const char *text = sorter_base[cnt].text, *src = text;
      char ch;
      if (cnt > 0) { *dest++ = ';'; *dest++ = ' '; }
      while ( (ch = *src++) != '\0' ) *dest++ = ch;
      memory_deallocate(text);
    }
    memory_deallocate(sorter_base);
    *dest = '\0';
    my_spf(NULL, 0, &spfbuf, "%sCookie: %s%s\r\n", retval,
      ( (highest_version > 0) ? "$Version=\"1\"; " : strEmpty ), /* CHECKME! */
      cookies);
    memory_deallocate(retval); memory_deallocate(cookies);
    retval = my_spf_use(spfbuf);
  }
  effective_hostname_cleanup(reshn, reseff);
  out0:
  return(retval);
}

#define cookie_collect_cleanup(ptr) \
  do { if (ptr != strEmpty) memory_deallocate(ptr); } while (0)


/* Inspection */

static one_caller char* cookie_reviewlist(const tCachedHostInformation*
  hostinfo)
/* constructs a "review list" (an HTML page snippet) for "about:hostinfo"
   resources */
{ char* retval;
  tCookie* cookie = hostinfo->cookies;
  if (cookie == NULL) return(NULL);

  retval = my_strdup(_("\n<br>cookies:"));
  while (cookie != NULL)
  { const char *portstr, *expiry, *cn = cookie->name, *cv = cookie->value,
      *cd = null2empty(cookie->domain), *cp = null2empty(cookie->path),
      *cc = null2empty(cookie->comment), *hn = htmlify(cn), *hv = htmlify(cv),
      *hd = htmlify(cd), *hp = htmlify(cp), *hc = htmlify(cc);
    char *spfbuf;
    tCookieFlags flags = cookie->flags;

    if (cookie->portlistlen <= 0) portstr = strEmpty;
    else
    { char* p = strbuf;
      tPortnumber* portlist = cookie->portlist;
      tBoolean is_first = truE;
      unsigned short i;
      p += sprint_safe(p, _(", ports="));
      for (i = 0; i < cookie->portlistlen; i++)
      { if (is_first) is_first = falsE;
        else p += sprint_safe(p, ",");
        p += sprint_safe(p, strPercd, ntohs(portlist[i]));
      }
      portstr = strbuf;
    }

    if (flags & coofUseExpiry)
    { sprint_safe(strbuf2, _("expiry=%ld, "), cookie->expiry);
      expiry = strbuf2;
    }
    else expiry = strEmpty;

    my_spf(NULL, 0, &spfbuf,
      _("%s\n<br>%s=%s, domain=%s, path=%s, comment=%s, %sflags=%d%s%s%s"),
      retval, hn, hv, hd, hp, hc, expiry, cookie->flags, portstr,
      ( (flags & coofExpired) ? _(" (expired)") : strEmpty ),
      ( (flags & coofTlsOnly) ? _(" (secure)") : strEmpty ));
    memory_deallocate(retval); retval = my_spf_use(spfbuf);
    htmlify_cleanup(cn, hn); htmlify_cleanup(cv, hv); htmlify_cleanup(cd, hd);
    htmlify_cleanup(cp, hp); htmlify_cleanup(cc, hc);
    cookie = cookie->next;
  }
  return(retval);
}


/* Initialization */

static one_caller void __init cookie_initialize(void)
{ /* Calculate the time offset between UTC/GMT and the local time. This code is
     a good candidate for the Ugliest Time Calculation / Gross Mental Teardown
     award... The point is to avoid buggy or non-portable library functions
     (and "struct tm" as a whole). */
  time_t now = my_time(), test; /* <now> is the UTC time */
  const char* str = ctime(&now); /* convert UTC number to local-time string */
  if ( (str != NULL) && (parse_datetime(str, &test)) )
  { /* "should" always work; this converts local-time string to local-time
       number; now we just have to compare UTC number and local-time number: */
    localtime_offset = test - now;
  }
}
