/* retawq/renderer.c - plaintext/HTML renderer
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

/* This code is roughly part of the user interface; it's taken out for the only
   reason that source files are smaller this way. Look how it's #include'd
   from main.c; maybe dirty, but simple and functional...
*/

/* Basic operation of this renderer: the caller sets up some data (in a
   tRendererData structure); the renderer scans/parses the given content and
   builds text lines from that; each single line is fed to the caller's "line
   callback" function which "consumes" the lines and tells the renderer
   whether further lines should be generated. This is, so far, the most
   correct/clean/fast rendering concept I was able to render. :-) */

#define renderer_interface static /* (currently) */

#define HTML_FORMS_SLOPPY 1
/* Grumble... Some web page authors write rubbish like
     <tr><form action="/search.php3"><td> or
     <table><form action="...."><tr>
   which is explicitly forbidden by htmlspec, e.g.
     <!ELEMENT TR       - O (TH|TD)+        -- table row -->
   ...and then users will blame retawq because they can't submit that form.
*/

my_enum1 enum
{ raUnknown = 0, raLayout = 1, raCalcWidth = 2
} my_enum2(unsigned char) tRendererAction;

my_enum1 enum
{ rdfNone = 0, rdfCallerDone = 0x01, rdfRendererDone = 0x02,
  rdfOutOfContent = 0x04, rdfVirtual = 0x08, rdfHtml = 0x10,
  rdfCalledParserHtmlStart = 0x20, rdfAe = 0x40,
#if TGC_IS_GRAPHICS
  rdfGraphical = 0x80, /* create graphical widgets for active elements */
#endif
#if TGC_IS_PIXELING
  rdfPixeling = 0x100, /* measure width/height in pixels, not in characters */
#endif
#if CONFIG_HTML & HTML_FRAMES
  rdfFrames = 0x200, /* build "real" frames */
#endif
  rdfFinal = 0x400, rdfAttributes = 0x800,
#if MIGHT_USE_COLORS
  rdfColors = 0x1000,
#endif
  rdfAlignment = 0x2000
} my_enum2(unsigned short) tRendererDataFlags;
#define rdfAnyDone (rdfCallerDone | rdfRendererDone)

typedef unsigned char tRendererText;
typedef attr_t tRendererAttr;

typedef struct tRendererElement
{ struct tRendererElement* next;
  tRendererText* text;
  tRendererAttr* attr;
  size_t textcount;
  tBoolean is_spacer;
} tRendererElement;

struct tRendererData;
typedef void (*tRendererLineCallback)(struct tRendererData*);

typedef struct tRendererData
{ void* rsd; /* "renderer-specific data", e.g. "tRendererHtmlData*" */
  tRendererLineCallback line_callback; /*CS*/
  void* line_callback_data; /*CS*/
  tBrowserDocument* document; /*CS*/
  const tRendererElement* element; /*CR*/
  tRendererText* inttext;
  tRendererAttr* intattr;
  tActiveElementNumber* intaenum;
  size_t inttextlen;
  size_t line_width; /*CS*/
  tCoordinate resulting_line_width; /* (CR for raCalcWidth) */
  tRendererDataFlags flags; /*CS*/
  tRendererAction ra; /*CS*/
} tRendererData;
/* Fields marked "CS" must/may be set by the caller; fields marked "CR" may
   be read by the caller; all other fields are private to the renderer. */

#define is_html(data) ((data)->flags & rdfHtml)

typedef struct
{ size_t count; /* for ordered lists */
  tRendererText symbol; /* for unordered lists */
  unsigned char depth;
  tBoolean is_ordered;
} tHtmlListBase;

typedef signed int tHtmlNestingNumber; /* ("signed" for simplicity only) */

my_enum1 enum
{ hnestfNone = 0, hnestfBold = 0x01, hnestfUnderlined = 0x02,
  hnestfAeStyle = 0x04, hnestfForbidPre = 0x08, hnestfInsidePre = 0x10
} my_enum2(unsigned char) tHtmlNestingFlags;
/* ("hnestf" to disambiguate from "hnf" (tHtmlNodeFlags) clearly) */

typedef struct
{ const char* unknown_tagname; /* for htkInvalid */
  tHtmlNestingNumber li_recalc;
  size_t li_offset;
  tRendererAttr currattr;
  tHtmlNodeFlags align; /* contains only hnfAlignAny flags */
  tHtmlTagKind htk;
  tHtmlNestingFlags hnestf;
  unsigned char listdepth;
} tHtmlNesting;

my_enum1 enum
{ rhdfNone = 0, rhdfAtParStart = 0x01, rhdfIsFirstThTd = 0x02
} my_enum2(unsigned char) tRendererHtmlDataFlags;

typedef struct
{ tHtmlNesting* nesting;
  tHtmlNestingNumber numnest, maxnest, p_level, table_level, ae_level,
    form_level;
  tActiveElementNumber _ae, __ae;
  tHtmlFormNumber _hfn, __hfn;
  tLinenumber currline;
  tRendererHtmlDataFlags flags;
} tRendererHtmlData;


/* Helper functions */

static one_caller void renderer_deallocate_rsd(tRendererData* data)
{ const void* _rsd = data->rsd;
  if (_rsd == NULL) return; /* nothing to do */
  if (is_html(data))
  { const tRendererHtmlData* rsd = (const tRendererHtmlData*) _rsd;
    __dealloc(rsd->nesting);
  }
  memory_deallocate(_rsd); data->rsd = NULL;
}

static __my_inline void renderer_deliver_line(tRendererData* data)
/* delivers one line of text to the callback */
{ (data->line_callback)(data);
}

static const tAttribute* find_attribute(const tHtmlNode* node,
  tAttributeName name)
{ const tAttribute* retval = NULL;
  if (node->kind != htkText)
  { const tAttribute* a = (const tAttribute*) (node->data);
    while (a != NULL)
    { if (a->name == name) { retval = a; break; }
      a = a->next;
    }
  }
  return(retval);
}


/* Plaintext renderer */

static tBoolean plaintext_deliver_line(tRendererData* data,
  tRendererText* text, size_t* _len)
{ size_t len = *_len, line_width = data->line_width, using;
  if (len < line_width) using = len; /* use the whole text */
  else /* try to break the line at a space character */
  { size_t si = line_width - 1; /* "split-index" */
    while ( (si > 0) && (text[si] != ' ') ) si--;
    if (si > 0) using = si;
    else using = line_width - 1; /* a whole line of unbreakable text */
  }
  switch (data->ra)
  { case raLayout:
      if (data->flags & rdfVirtual) data->element = NULL;
      else
      { static tBoolean did_init = falsE;
        static tRendererElement element;
        if (!did_init) { my_memclr_var(element); did_init = truE; }
        element.text = text; element.textcount = using;
        data->element = &element;
      }
      break;
    case raCalcWidth: data->resulting_line_width = using; break;
  }
  renderer_deliver_line(data);
  while ( (using < len) && (text[using] == ' ') ) using++;
  if (using >= len) len = 0; /* all was used */
  else
  { size_t count;
    for (count = 0; count < len - using; count++)
      text[count] = text[count + using];
    len -= using;
  }
  *_len = len;
  return(cond2boolean(!(data->flags & rdfCallerDone)));
}

#define plaintext_append_char(ch) \
  do \
  { inttext[inttextlen++] = (tRendererText) ch; \
    if (inttextlen > line_width) \
    { if (!plaintext_deliver_line(data, inttext, &inttextlen)) goto out; } \
  } while (0)

static one_caller void renderer_plaintext(tRendererData* data)
{ const tContentblock* content = data->document->cantent->content;
  const char* contentdata;
  const size_t line_width = data->line_width;
  size_t contentsize, pos = 0, inttextlen = 0,
    maxinttextlen = line_width + 8 + 4;
  tRendererText* inttext = __memory_allocate(maxinttextlen *
    sizeof(tRendererText), mapRendering);
  unsigned char c;
  char ch;

  content_recalc: contentdata = content->data; contentsize = content->used;
  loop:
  if (pos >= contentsize)
  { content = content->next; pos = 0;
    if (content != NULL) goto content_recalc;
    else
    { data->flags |= rdfOutOfContent;
      while ( (inttextlen > 0) &&
              (plaintext_deliver_line(data, inttext, &inttextlen)) )
      { /* deliver any remaining text */ }
      data->flags |= rdfRendererDone; goto out;
    }
  }
  ch = contentdata[pos++]; c = (unsigned char) ch;
  if (is_bad_uchar(c))
  { if (ch == '\n')
    { if (!plaintext_deliver_line(data, inttext, &inttextlen)) goto out; }
    else if (ch == '\t')
    { unsigned char count = 8;
      while (count-- > 0) plaintext_append_char(' ');
    }
    else if (ch != '\r') { ch = '?'; goto do_append; }
  }
  else { do_append: plaintext_append_char(ch); }
  goto loop;

  out:
  memory_deallocate(inttext);
}

#undef plaintext_append_char


/* HTML renderer */

#define html_currnest(rsd) ((rsd)->numnest - 1)
#define html_nestdata(what) (rsd->nesting[html_currnest(rsd)].what)
#define html_hnestf_or(value) \
  do { html_nestdata(hnestf) |= (value); html_recalc_currattr(data); } while(0)

#define html_inside_pre_tag(rsd) (html_nestdata(hnestf) & hnestfInsidePre)
#define html_do_ae(data) ((data)->flags & rdfAe)
#define html_do_attr(data) ((data)->flags & rdfAttributes)
#if MIGHT_USE_COLORS
#define html_do_colors(data) ((data)->flags & rdfColors)
#endif
#define set_form_data /* (currently nothing to do) */

static my_inline size_t html_line_width(tRendererData* data)
{ const tRendererHtmlData* rsd = (const tRendererHtmlData*) data->rsd;
  return(data->line_width - html_nestdata(li_offset));
}

static void html_recalc_currattr(tRendererData* data)
{ if (html_do_attr(data))
  { tRendererHtmlData* rsd = (tRendererHtmlData*) data->rsd;
    const tHtmlNestingFlags hnestf = html_nestdata(hnestf);
    tRendererAttr attr = 0;
    if (hnestf & hnestfBold) attr |= A_BOLD;
    if (hnestf & hnestfUnderlined) attr |= A_UNDERLINE;
    if (hnestf & hnestfAeStyle)
    { attr |= A_UNDERLINE;
#if (TGC_IS_CURSES) && (MIGHT_USE_COLORS)
      if (html_do_colors(data)) attr |= my_color_attr(cpnBlue);
#endif
    }
    html_nestdata(currattr) = attr;
  }
}

static my_inline void html_form_off(tRendererHtmlData* rsd)
{ rsd->form_level = 0; rsd->_hfn = INVALID_HTML_FORM_NUMBER; }

static my_inline void __html_finish_ae(tRendererHtmlData* rsd)
{ rsd->ae_level = 0; rsd->_ae = INVALID_AE; }

static void html_finish_ae(tRendererData* data)
{ tRendererHtmlData* rsd = (tRendererHtmlData*) data->rsd;
  __html_finish_ae(rsd);
  html_nestdata(hnestf) &= ~hnestfAeStyle;
  html_recalc_currattr(data);
}

static void __html_deliver_line(tRendererData* data, tHtmlNodeFlags align)
{ tRendererHtmlData* rsd = (tRendererHtmlData*) data->rsd;
  size_t inttextlen = data->inttextlen, using, alignoff,
    line_width = html_line_width(data);
  tRendererText* inttext = data->inttext;
  tBoolean do_attr = cond2boolean(html_do_attr(data)), is_line_empty,
    do_ae = cond2boolean(html_do_ae(data));
  tRendererElement element, spacer;
  tHtmlNestingNumber li_recalc;

  if (inttextlen < line_width) using = inttextlen; /* use whole text */
  else /* try to break the line at a space character */
  { size_t si = line_width - 1; /* "split-index" */
    while ( (si > 0) && (inttext[si] != ' ') ) si--;
    if (si > 0) using = si;
    else using = line_width - 1; /* a whole line of unbreakable text */
  }
  while ( (using > 0) && (inttext[using - 1] == ' ') ) using--;
    /* remove trailing whitespace; e.g. necessary for empty-line decision and
       also nice when dumping into a file */
  is_line_empty = cond2boolean(!(using > 0));

  alignoff = html_nestdata(li_offset);
  if (align)
  { if (align & hnfAlignCenter) alignoff = (line_width - 1 - using) / 2;
    else if (align & hnfAlignRight) alignoff = line_width - 1 - using;
  }

  if ( (do_ae) && (using > 0) )
  { tActiveElementNumber* intaenum = data->intaenum;
    size_t start, count, end;
    tActiveElementNumber _ae;
    start = count = end = 0;
    aeloop:
    _ae = intaenum[count];
    while (count < using)
    { if (_ae != intaenum[count]) break;
      end = count++;
    }
    if (_ae != INVALID_AE)
    {
#if TGC_IS_CURSES
      const tBrowserDocument* document = data->document;
      tActiveElement* ae = &(document->active_element[_ae]);
      tActiveElementCoordinates *ex = ae->aec, *aec =
        memory_allocate(sizeof(tActiveElementCoordinates), mapRendering);
      aec->y = rsd->currline;
      aec->x1 = start + alignoff; aec->x2 = end + alignoff;
      if (ex == NULL) ae->aec = aec;
      else { while (ex->next != NULL) { ex = ex->next; } ex->next = aec; }
#endif
    }
    count = end + 1;
    if (count < using) { start = count; goto aeloop; }
  }
  switch (data->ra)
  { case raLayout:
      if ( (using > 0) && (!(data->flags & rdfVirtual)) )
      { if (alignoff > 0)
        { my_memclr_var(spacer); spacer.textcount = alignoff;
          spacer.is_spacer = truE; spacer.next = &element;
          data->element = &spacer;
        }
        else data->element = &element;
        my_memclr_var(element);
        element.text = inttext; element.textcount = using;
        if (do_attr) element.attr = data->intattr;
      }
      else data->element = NULL;
      break;
    case raCalcWidth: data->resulting_line_width = using; break;
  }
  if (!(data->flags & rdfCallerDone)) renderer_deliver_line(data);

  rsd->currline++;
  if (is_line_empty) rsd->flags |= rhdfAtParStart;
  else rsd->flags &= ~rhdfAtParStart;

  li_recalc = html_nestdata(li_recalc);
  if (li_recalc > 0)
  { size_t depth = (size_t) rsd->nesting[li_recalc].listdepth,
      offset = depth * 2, full_line_width = data->line_width;
    tHtmlNestingNumber hnn = li_recalc;
    if (offset + 10 > full_line_width) offset = full_line_width - 10;
    while (hnn <= html_currnest(rsd))
    { rsd->nesting[hnn].li_recalc = 0; rsd->nesting[hnn].li_offset = offset;
      hnn++;
    }
  }

  while ( (using < inttextlen) && (inttext[using] == ' ') ) using++;
  if (using >= inttextlen) inttextlen = 0; /* all was used */
  else
  { size_t count;
    for (count = 0; count < inttextlen - using; count++) /* IMPROVEME! */
    { inttext[count] = inttext[count + using];
      if (do_attr) data->intattr[count] = data->intattr[count + using];
      if (do_ae) data->intaenum[count] = data->intaenum[count + using];
    }
    inttextlen -= using;
  }
  data->inttextlen = inttextlen;
}

static my_inline void html_deliver_line(tRendererData* data)
{ const tRendererHtmlData* rsd = (const tRendererHtmlData*) data->rsd;
  __html_deliver_line(data, html_nestdata(align));
}

static void html_append_charattr(tRendererData* data, tRendererText ch,
  tRendererAttr attr)
{ tRendererText* inttext = data->inttext;
  size_t inttextlen = data->inttextlen;
  if ((ch == ' ') && ( (inttextlen <= 0) || (inttext[inttextlen - 1] == ' ') ))
  { /* at beginning of line or space character already present; htmlspec 9.1:
       "user agents should collapse input white space sequences when producing
       output inter-word space." */
    const tRendererHtmlData* rsd = (const tRendererHtmlData*) data->rsd;
    if (!html_inside_pre_tag(rsd)) return;
  }
  inttext[inttextlen] = ch;
  if (html_do_attr(data)) data->intattr[inttextlen] = attr;
  if (html_do_ae(data))
    data->intaenum[inttextlen] = ((tRendererHtmlData*)(data->rsd))->_ae;
  inttextlen++; data->inttextlen = inttextlen;
  if (inttextlen > html_line_width(data)) html_deliver_line(data);
}

static my_inline void html_append_char(tRendererData* data, tRendererText ch)
{ const tRendererHtmlData* rsd = (const tRendererHtmlData*) data->rsd;
  html_append_charattr(data, ch, html_nestdata(currattr));
}

static my_inline void html_append_str(tRendererData* data, const char* str)
{ tRendererText ch;
  while ( (ch = *str++) != '\0' ) html_append_char(data, ch);
}

static one_caller void html_append_hline_char(tRendererData* data)
{
#if (!TGC_IS_CURSES)
  html_append_charattr(data, '-', 0);
#else
  if ( (!html_do_attr(data)) || (__MY_HLINE == '-') ) /* hrm... */
    html_append_charattr(data, '-', 0);
  else html_append_charattr(data, __MY_HLINE, __MY_HLINE);
#endif
}

#define for_each_hnn(hnn) for (hnn = html_currnest(rsd); hnn > 0; hnn--)

static void html_nest(tRendererData* data, tHtmlTagKind htk, const char* utn,
  tHtmlNodeFlags hnf)
{ tRendererHtmlData* rsd = (tRendererHtmlData*) data->rsd;
  tHtmlNestingNumber num = rsd->numnest, maxnum = rsd->maxnest;
  tHtmlNesting *nesting = rsd->nesting, *n;
  if (num >= maxnum)
  { maxnum += 20; rsd->maxnest = maxnum;
    rsd->nesting = nesting = (tHtmlNesting*)
      memory_reallocate(nesting, maxnum * sizeof(tHtmlNesting), mapRendering);
  }
  n = &(nesting[num]);
  if (num > 0)
  { tHtmlNodeFlags align = hnf & hnfAlignAny;
    *n = nesting[num - 1];
    if (align) n->align = align;
  }
  else my_memclr_var(nesting[num]);
  n->htk = htk; n->unknown_tagname = utn;
  rsd->numnest++;
  if (htk_forbids_pre(htk))
  { html_nestdata(hnestf) &= ~hnestfInsidePre;
    html_hnestf_or(hnestfForbidPre);
  }
}

static void html_linebreaks(tRendererData* data, tHtmlTagKind htk)
{ if (htk_is_par(htk))
  { const tRendererHtmlData* rsd = (const tRendererHtmlData*) data->rsd;
    if (data->inttextlen > 0) html_deliver_line(data);
    if (!(rsd->flags & rhdfAtParStart)) html_deliver_line(data);
  }
  else if (htk_is_block(htk))
  { if (data->inttextlen > 0) html_deliver_line(data);
  }
}

static void __html_denest(tRendererData* data, tHtmlNestingNumber hnn)
{ tRendererHtmlData* rsd = (tRendererHtmlData*) data->rsd;
  while (html_currnest(rsd) > hnn)
  { const tHtmlTagKind htk = html_nestdata(htk);
    switch (htk)
    { case htkQ: html_append_char(data, '"'); break;
      case htkSub: case htkSup: html_append_char(data, ')'); break;
      case htkStrike: case htkS: case htkDel:
        html_append_str(data, "]]"); break;
    }
    html_linebreaks(data, htk);
    rsd->numnest--;
  }
  if (rsd->ae_level > html_currnest(rsd)) __html_finish_ae(rsd);
  if (rsd->p_level > html_currnest(rsd)) rsd->p_level = 0;
  if (rsd->table_level > html_currnest(rsd)) rsd->table_level = 0;
#if !HTML_FORMS_SLOPPY
  if (rsd->form_level > html_currnest(rsd)) html_form_off(rsd);
#endif
}

#define html_denest(hnn) __html_denest(data, hnn) /* abbr. */

#define __try_denest(condition, offset) \
  do \
  { tHtmlNestingNumber hnn; \
    for_each_hnn(hnn) \
    { const tHtmlTagKind htk = rsd->nesting[hnn].htk; \
      if (condition) { html_denest(hnn - offset); return; } \
    } \
  } while (0)

#define try_denest(condition) __try_denest((condition), 0)
#define try_other_denest(condition) __try_denest((condition), 1)

static one_caller void html_opening_denest(tRendererData* data,
  const tHtmlTagKind this_htk)
/* Certain opening tags cause auto-closing of certain former tags (mostly
   related to htfAllowEndtag tags). */
{ tRendererHtmlData* rsd = (tRendererHtmlData*) data->rsd;
  tHtmlNestingNumber p_level = rsd->p_level;
  if ( (p_level > 0) && (htk_is_block(this_htk)) )
  { /* htmlspec 9.3.1 says: "The P element [...] cannot contain block-level
       elements [...]." */
    html_denest(p_level - 1); rsd->p_level = 0;
  }
  switch (this_htk)
  { case htkHtml: /* close "everything" */
      denest_everything: html_denest(0); break;
    case htkHead: case htkBody: /* close everything but a htkHtml */
      try_denest(htk == htkHtml);
      /* didn't find a former htkHtml, thus: */
      goto denest_everything; /*@notreached@*/ break;
    case htkLi:
      try_denest((htk == htkUl) || (htk == htkOl));
      try_other_denest(htk == htkLi); break;
    case htkTd: case htkTh:
      try_denest((htk == htkTr) || (htk == htkTable));
      try_other_denest((htk == htkTd) || (htk == htkTh)); break;
    case htkTr:
      try_denest(htk == htkTable); try_other_denest(htk == htkTr); break;
    case htkOption:
      try_denest((htk == htkSelect) || (htk == htkForm));
      try_other_denest(htk == htkOption); break;
    case htkDd: case htkDt:
      try_denest(htk == htkDl);
      try_other_denest((htk == htkDd) || (htk == htkDt)); break;
  }
}

static one_caller void html_nesting_opener(tRendererData* data,
  const tHtmlNode* node)
{ tRendererHtmlData* rsd = (tRendererHtmlData*) data->rsd;
  const tHtmlTagKind htk = node->kind;
  if (!htk_forbids_endtag(htk))
  { const char* utn = NULL;
    if (htk == htkInvalid)
    { const tAttribute* a = find_attribute(node, anInternalTagname);
      if (a != NULL) utn = a->value;
    }
    html_opening_denest(data, htk);
    html_linebreaks(data, htk);
    html_nest(data, htk, utn, node->flags);
    if (htk == htkP) rsd->p_level = html_currnest(rsd);
  }
  else html_linebreaks(data, htk);
  if ( (html_do_ae(data)) && (node->flags & hnfHasAeBase) )
  { tBrowserDocument* document = data->document;
    tActiveElement* aes = document->active_element;
    const tCantent* cantent = document->cantent;
    const tActiveElementBase* aebase = cantent->aebase;
    tActiveElementNumber aenum = document->aenum, aemax = document->aemax, _ae;
    tActiveElementKind aek;
    rsd->__ae++; _ae = rsd->__ae; aek = aebase[_ae].kind;
    if (aek == aekFormHidden) html_finish_ae(data);
    else
    { rsd->_ae = _ae; rsd->ae_level = html_currnest(rsd);
      html_hnestf_or(hnestfAeStyle);
    }
    if (_ae >= aenum)
    { tHtmlFormNumber hfn;
      if (aenum >= aemax)
      { aemax += aenum_incvalue(aemax); document->aemax = aemax;
        aes = document->active_element = memory_reallocate(aes, aemax *
          sizeof(tActiveElement), mapRendering);
      }
      aenum++; document->aenum = aenum;
      init_ae(&(aebase[_ae]), &(aes[_ae]), truE, document);
      if ( (is_form_aek(aek)) && ((hfn=rsd->_hfn) != INVALID_HTML_FORM_NUMBER))
      { tHtmlForm* f = &(cantent->form[hfn]);
        tActiveElementNumber last = f->last_ae;
        if (f->first_ae == INVALID_AE) f->first_ae = _ae;
        if ( (last == INVALID_AE) || (last < _ae) ) f->last_ae = _ae;
      }
    }
  }
}

static one_caller void html_nesting_closer(tRendererData* data,
  const tHtmlNode* node)
{ tRendererHtmlData* rsd = (tRendererHtmlData*) (data->rsd);
  const tHtmlTagKind this_htk = node->kind;
  if (this_htk != htkInvalid)
  {
#if HTML_FORMS_SLOPPY
    if (this_htk == htkForm) html_form_off(rsd); /* found explicit </form> */
#endif
    try_other_denest(htk == this_htk);
  }
  else
  { const tAttribute* attr = find_attribute(node, anInternalTagname);
    const char *utn, *utn2;
    if ( (attr != NULL) && ( (utn = attr->value) != NULL ) )
    { const tHtmlNesting* nesting = rsd->nesting;
      try_other_denest( (htk == htkInvalid) &&
        ( (utn2 = nesting[hnn].unknown_tagname) != NULL ) &&
        (!strcmp(utn, utn2)) );
    }
  }
}

static one_caller void renderer_html(tRendererData* data)
{ tRendererHtmlData* rsd = (tRendererHtmlData*) data->rsd;
  const tBoolean do_ae = cond2boolean(html_do_ae(data));
  const size_t maxinttextlen = data->line_width + 5;
  const tHtmlNode* node;

  data->inttext = __memory_allocate(maxinttextlen * sizeof(tRendererText),
    mapRendering);
  if (html_do_attr(data))
  { data->intattr = __memory_allocate(maxinttextlen * sizeof(tRendererAttr),
      mapRendering);
  }
  if (do_ae)
  { data->intaenum = __memory_allocate(maxinttextlen *
      sizeof(tActiveElementNumber), mapRendering);
  }

  while ( (!(data->flags & rdfCallerDone)) &&
          ( (node = parser_html_next(cond2boolean(html_inside_pre_tag(rsd))))
            != NULL ) )
  { static const char listmarkers[] = "*+#o";
    const tHtmlTagKind htk = node->kind;
    const tHtmlNodeFlags hnf = node->flags;
    const tBoolean is_endtag = cond2boolean(hnf & hnfIsEndtag);
    const char* s;
    char listbuf[3];
    tBoolean finish_ae = falsE;

    if (htk == htkText) /* the most likely/special case first */
    { char ch;
      s = (const char*) (node->data);
      if (s == NULL) goto next_tag;
      put_text:
      while ( (ch = *s++) != '\0' )
      { const unsigned char c = (unsigned char) ch;
        if (is_bad_uchar(c))
        { if (ch == '\n')
          { if (!html_inside_pre_tag(rsd)) ch = ' ';
            else { html_deliver_line(data); continue; }
          }
          else if (ch == '\t')
          { if (!html_inside_pre_tag(rsd)) ch = ' ';
            else { html_append_str(data, "        "); continue; }
          }
          else if (ch == '\r') continue;
          else ch = '?';
        }
        html_append_char(data, ch);
      }
      if (finish_ae) html_finish_ae(data);
      goto next_tag;
    }

    if (is_endtag) { html_nesting_closer(data, node); goto next_tag; }
    html_nesting_opener(data, node);

    switch (htk)
    { case htkBr: html_deliver_line(data); break;
      case htkH1: case htkH2: case htkH3: case htkH4: case htkH5: case htkH6:
      case htkB: case htkStrong: case htkBig:
        html_hnestf_or(hnestfBold); break;
      case htkBlockquote: case htkU: case htkI: case htkAddress: case htkEm:
      case htkDfn: case htkCite:
        html_hnestf_or(hnestfUnderlined); break;

      /* lists */
      case htkLi:
        listbuf[0] = listmarkers[(html_nestdata(listdepth)) & 3];
        listbuf[1] = ' '; listbuf[2] = '\0';
        if (html_nestdata(listdepth) < 255)
        { html_nestdata(listdepth)++;
          html_nestdata(li_recalc) = html_currnest(rsd);
        }
        s = listbuf; goto put_text; /*@notreached@*/ break;
      case htkDd: s = "-> "; goto put_text; /*@notreached@*/ break;

      /* some active-element stuff */
      case htkForm:
        if (hnf & hnfGoodForm)
        { rsd->__hfn++; rsd->_hfn = rsd->__hfn;
          rsd->form_level = html_currnest(rsd);
        }
        else html_form_off(rsd);
        break;
      case htkInput: case htkTextarea:
        if ( /* (do_ae) && */ (rsd->_ae != INVALID_AE) )
        { const tBrowserDocument* document = data->document;
          const tCantent* cantent = document->cantent;
          const tActiveElementBase* aeb = &(cantent->aebase[rsd->_ae]);
          const tActiveElement* ae = &(document->active_element[rsd->_ae]);
          const tActiveElementFlags aeflags = ae->flags;
          const tActiveElementKind aek = aeb->kind;
          tHtmlInputLength l, len, deslen;
          const char* temp;
          set_form_data
          switch (aek)
          {case aekFormCheckbox:
            s = ( (aeflags & aefCheckedSelected) ? "[X]" : "[_]" );
            good_ae_text: finish_ae = truE; goto put_text;
            /*@notreached@*/ break;
           case aekFormRadio:
            s = ( (aeflags & aefCheckedSelected) ? "(*)" : "(_)" );
            goto good_ae_text; /*@notreached@*/ break;
           case aekFormText: case aekFormPassword: case aekFormFile:
            deslen = ((aek == aekFormFile) ? 20 : aeb->size);
            if (deslen + 10 > html_line_width(data))
              deslen = html_line_width(data) - 10; /* make layout possible */
            temp = ae->current_text;
            if (temp == NULL) { len = l = 0; }
            else
            { len = strlen(temp); l = 0;
              if (len > deslen) len = deslen; /* obey web page author :-) */
              if (aek == aekFormPassword) { while (l < len) strbuf3[l++]='*'; }
              else
              { while (l < len)
                { char ch = temp[l];
                  if ( (ch <= 32) || (ch == 127) ) ch = '_';
                  strbuf3[l++] = ch;
                }
              }
            }
            while (l < deslen) strbuf3[l++] = '_';
            strbuf3[l] = '\0'; s = strbuf3;
            goto good_ae_text; /*@notreached@*/ break;
           case aekFormSubmit: case aekFormReset:
           case aekFormButton: case aekFormImage:
            s = aeb->render; goto good_ae_text; /*@notreached@*/ break;
          }
        }
        break;
      case htkButton:
        if ( /* (do_ae) && */ (rsd->_ae != INVALID_AE) ) { set_form_data }
        break;
      case htkArea:
        if ( /* (do_ae) && */ (rsd->_ae != INVALID_AE) )
        { const tAttribute* a = find_attribute(node, anAlt);
          set_form_data
          if ( (a != NULL) && ( (s = a->value) != NULL ) && (*s != '\0') )
          { /* fine */ }
          else s = _("[an image-map hotspot]");
          goto good_ae_text;
        }
        break;
      case htkSelect:
        if ( /* (do_ae) && */ (rsd->_ae != INVALID_AE) )
        { const char* bitfield =
            data->document->active_element[rsd->_ae].current_text;
          const tHtmlOption *o, *o0;
          tHtmlOptionNumber num;
          set_form_data
          if (bitfield == NULL) goto empty_selection;
          o = o0 = (const tHtmlOption*)
            (data->document->cantent->aebase[rsd->_ae].render);
          num = 0;
          while (o != NULL)
          { if (my_bit_test(bitfield, num)) /* found a selected option */
            { s = o->render;
              if (s != NULL) goto good_ae_text;
            }
            o = o->next; num++;
          }
          if (o0 != NULL) { s = o0->render; if (s != NULL) goto good_ae_text; }
          empty_selection: s = _("[empty selection list]"); goto good_ae_text;
        }
        break;

      /* tables */
      case htkTable:
        rsd->table_level = html_currnest(rsd); rsd->flags |= rhdfIsFirstThTd;
        break;
      case htkTr: rsd->flags |= rhdfIsFirstThTd; break;
/*G*/ case htkTh: html_hnestf_or(hnestfBold); /*@fallthrough@*/
/*L*/ case htkTd:
/*U*/   if (rsd->flags & rhdfIsFirstThTd) rsd->flags &= ~rhdfIsFirstThTd;
/*E*/   else { s = " | "; goto put_text; }
        break;

      /* other stuff */
      case htkImg:
        { const tAttribute* a = find_attribute(node, anAlt);
          if ( (a != NULL) && ( (s = a->value) != NULL ) && (*s != '\0') )
            goto put_text;
        }
        break;
      case htkHr:
        { const tAttribute* w = find_attribute(node, anWidth);
          const char* wv;
          size_t line_width = html_line_width(data), dashcount = line_width;
          if ( (w != NULL) && ( (wv = w->value) != NULL ) && (my_isdigit(*wv)))
          { int x;
            my_atoi(wv, &x, &wv, 100);
            if ( (*wv == '%') && (*(wv + 1) == '\0') && (x >= 0) && (x <= 100))
            { const size_t x2 = (size_t) x;
              dashcount = ( (line_width * x2) / 100 ); /* CHECKME: rounding? */
            }
          }
          if (data->inttextlen > 0) html_deliver_line(data);
          if (dashcount > 1)
          { while (dashcount-- > 1) html_append_hline_char(data);
            __html_deliver_line(data, hnf & hnfAlignAny);
          }
        }
        break;
      case htkPre:
        if (!(html_nestdata(hnestf) & hnestfForbidPre))
          html_hnestf_or(hnestfInsidePre);
        break;
      case htkSub: s = "_("; goto put_text; /*@notreached@*/ break;
      case htkSup: s = "^("; goto put_text; /*@notreached@*/ break;
      case htkStrike: case htkS: case htkDel:
        s = "[["; goto put_text; /*@notreached@*/ break;
      case htkQ:
        html_hnestf_or(hnestfUnderlined); s = strDoubleQuote;
        goto put_text; /*@notreached@*/ break;
      case htkFrame: case htkIframe:
        if ( /* (do_ae) && */ (rsd->_ae != INVALID_AE) )
        { s = data->document->cantent->aebase[rsd->_ae].render;
          goto good_ae_text;
        }
        break;
      case htkObject:
        if (find_attribute(node, anDeclare) == NULL)
        { const tAttribute* t = find_attribute(node, anType);
          const char* tv;
          if ( (t != NULL) && ( (tv = t->value) != NULL ) && (*tv != '\0') &&
               (strlen(tv) <= 1024) )
          { sprint_safe(strbuf3, _("[an embedded object of type \"%s\"]"), tv);
            s = strbuf3;
          }
          else s = _("[an embedded object]");
          goto put_text;
        }
        break;
    }

    next_tag: {}
    if (!(hnf & hnfStoredInTree)) deallocate_html_node(node);
  }
  data->flags |= rdfOutOfContent;
  html_denest(0);
  while ( (data->inttextlen > 0) && (!(data->flags & rdfCallerDone)) )
    html_deliver_line(data); /* deliver any remaining text */
  data->flags |= rdfRendererDone;
  memory_deallocate(data->inttext); __dealloc(data->intattr);
  __dealloc(data->intaenum);
}


/* Generic renderer */

static one_caller void renderer_start(tRendererData* data)
{ tBrowserDocument* document = data->document;
  tCantent* cantent = document->cantent;
#if TGC_IS_CURSES
  tActiveElementNumber count, num;
  tActiveElement* aes;
#endif
  if ( (cantent == NULL) || ( (cantent->content) == NULL ) )
  { data->flags |= rdfRendererDone | rdfOutOfContent; return; }
#if TGC_IS_CURSES
  if (!(data->flags & rdfAttributes))
  { /* In curses mode, we can't deliver colors etc. when attributes are off. */
#if MIGHT_USE_COLORS
    data->flags &= ~rdfColors;
#endif
  }
#endif
#if MIGHT_USE_COLORS
  if (!use_colors) data->flags &= ~rdfColors;
#endif
#if TGC_IS_CURSES
  if ( ( (num = document->aenum) > 0 ) &&
       ( (aes = document->active_element) != NULL ) )
  { for (count = 0; count < num; count++) deallocate_aec(&(aes[count].aec)); }
#endif
  if (is_html(data))
  { tRendererHtmlData* rsd;
    parser_html_start(cantent); data->flags |= rdfCalledParserHtmlStart;
    data->rsd = rsd = (tRendererHtmlData*)
      memory_allocate(sizeof(tRendererHtmlData), mapRendering);
    rsd->flags = rhdfAtParStart;
    rsd->_ae = INVALID_AE; rsd->__ae = -1;
    rsd->_hfn = INVALID_HTML_FORM_NUMBER; rsd->__hfn = -1;
    html_nest(data, htkInvalid, NULL, hnfNone); /* (for simplicity) */
  }
}

static one_caller void renderer_finish(tRendererData* data)
{ if (data->flags & rdfCalledParserHtmlStart) parser_html_finish();
  renderer_deallocate_rsd(data);
}

renderer_interface void renderer_run(tRendererData* data)
/* sole entry point for "external" callers */
{ renderer_start(data);
  if (!(data->flags & rdfAnyDone))
  { if (is_html(data)) renderer_html(data);
    else renderer_plaintext(data);
  }
  renderer_finish(data);
}
