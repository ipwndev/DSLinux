/* retawq/wk.c - window kinds
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

/* This code is part of the user interface; it's taken out for the only reason
   that source files are smaller this way. Look how it's #include'd from
   main.c; maybe dirty, but simple and functional...
*/

/* Helper functions for text/graphics modes */

#if CONFIG_TG == TG_GTK

#define connect_object(object, signal, handler, data) \
  gtk_signal_connect(object, signal, GTK_SIGNAL_FUNC(handler), data)
#define connect_widget(widget, signal, handler, data) \
  connect_object(GTK_OBJECT(widget), signal, handler, data)
#define connect_window(window, signal, handler, data) \
  connect_object(GTK_OBJECT(window), signal, handler, data)
#define show_widget(x) gtk_widget_show(GTK_WIDGET(x))
#define pack_box(container, element) \
  gtk_box_pack_start(GTK_BOX(container), element, FALSE, FALSE, 0)

typedef struct
{ tWindow* window;
  tProgramCommandCode pcc;
} tGtkWindowCommand;

static my_inline __sallocator tGtkWindowCommand* __callocator
  create_window_command(const tWindow* window, tProgramCommandCode pcc)
{ tGtkWindowCommand* retval =
    (tGtkWindowCommand*) __memory_allocate(sizeof(tGtkWindowCommand), mapGtk);
  retval->window = __unconstify(tWindow*, window); retval->pcc = pcc;
  return(retval);
}

static void graphics_handle_command_code(GtkWidget* widget, gpointer _data)
{ const tGtkWindowCommand* data = (tGtkWindowCommand*) _data;
  tWindow* window = data->window;
  const GtkWindow* ww = window->ww;
  tProgramCommandCode pcc = data->pcc;
  current_window = window; handle_command_code(pcc); current_window = NULL;
}

static gint graphics_propagate_event(tGraphicsWidget* w __cunused,
  tGraphicsEvent* event __cunused, gpointer data __cunused)
{ /* just enforce the propagation of signals to parent widgets: */
  return(FALSE);
}

static gint window_handle_delete(tGraphicsWidget* w __cunused,
  tGraphicsEvent* event __cunused, gpointer data __cunused)
{ /* don't handle it here, let GTK call window_handle_destroy()... */
  return(FALSE);
}

static gint window_handle_destroy(tGraphicsWidget* w __cunused, gpointer data)
{ window_hide((tWindow*) data, 2);
  return(TRUE);
}

static gint window_handle_configure(tGraphicsWidget* widget __cunused,
  tGraphicsEvent* event __cunused, gpointer data)
{ tWindow* w = (tWindow*) data;
  window_redraw(w);
  return(TRUE);
}

static tGraphicsWidget* create_menu(tGraphicsWidget* bar, char* _title)
{ tGraphicsWidget *menu = gtk_menu_new(),
    *title = gtk_menu_item_new_with_label(_title);
  show_widget(title);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(title), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(bar), title);
  return(menu);
}

static void __create_item(const tWindow* window, GtkAccelGroup* accel,
  tGraphicsWidget* menu, const char* text, tProgramCommandCode pcc, tKey key)
{ tGraphicsWidget* item = gtk_menu_item_new_with_label(text);
  gtk_menu_append(GTK_MENU(menu), item);
  connect_widget(item, strGtkActivate, graphics_handle_command_code,
    (gpointer) create_window_command(window, pcc));
  if (key != '\0')
  { GdkModifierType t;
    if (my_isupper(key)) { t = GDK_CONTROL_MASK; key = my_tolower(key); }
    else t = GDK_MOD1_MASK;
    gtk_widget_add_accelerator(item, strGtkActivate, accel, key, t,
      GTK_ACCEL_VISIBLE);
  }
  show_widget(item);
}

#define create_item(m, t, pcc, k) __create_item(retval, accel, m, t, pcc, k)

static void create_button(const tWindow* window, tGraphicsWidget* container,
  char* text, tProgramCommandCode pcc)
{ tGraphicsWidget* button = gtk_button_new_with_label(text);
  connect_widget(button, strGtkClicked, graphics_handle_command_code,
    (gpointer) create_window_command(window, pcc));
  pack_box(container, button);
  show_widget(button);
}

static gint graphics_cm(tGraphicsWidget* w __cunused, tGraphicsEvent* _event,
  gpointer data)
/* shows a contextual menu in graphics mode */
{
  if (_event->type == GDK_BUTTON_PRESS)
  { GdkEventButton* event = (GdkEventButton*) _event;
    guint b = event->button;
    if (b == 3)
    { tGraphicsWidget *menu = gtk_menu_new(),
        *item = gtk_menu_item_new_with_label("TEST");
      tWindow* window = (tWindow*) data;
      show_widget(menu); show_widget(item);
      gtk_menu_append(GTK_MENU(menu), item);
      gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, b, event->time);
      return(TRUE);
    }
  }
  return(FALSE);
}


static const char* const retawq_logo_mini[] = {
"64 64 2 1",
" 	c None",
".	c #FF0000",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                        .....                   ",
"                         ....        ...........                ",
"            ..................     ..............               ",
"            ..................    ................              ",
"            ..................   ..................             ",
"            ..................   ..................             ",
"                 .............  ...... ............             ",
"                  ............ .....   ............             ",
"                  ............ ....    ............             ",
"                  .................    ............             ",
"                   ...............     ............             ",
"                   ...............     ............             ",
"                   ..............       ..........              ",
"                   ..............        ........               ",
"                   ..............          ....                 ",
"                   .............                                ",
"                   .............                                ",
"                   .............                                ",
"                   .............                                ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                   ............                                 ",
"                  .............                                 ",
"                  ..............                                ",
"             .........................                          ",
"            ..........................                          ",
"            ..........................                          ",
"            ..........................                          ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                "};

#elif TGC_IS_CURSES

static void cu_display_title(const tWindow* window, const char* title,
  short titlerow)
{ short count = 0, maxcount = COLS - 1;
  char ch, buf[1024];
  if (maxcount > 1000) maxcount = 1000; /* avoid buffer overflows... */
  /* setup */
  buf[count++] = window->spec->ui_char; buf[count++] = ' '; buf[count++] = '-';
  buf[count++] = ' ';
  while ( ((ch = *title++) != '\0') && (count < maxcount) ) buf[count++] = ch;
  while (count <= maxcount) buf[count++] = ' ';
  buf[count] = '\0';
  /* output */
  (void) move(titlerow, 0); (void) attron(A_REVERSE); (void) addstr(buf);
  (void) attroff(A_REVERSE);
}

#endif /* CONFIG_TG */


/*** Window kind: browser window */

typedef struct tWindowView /* a "view" of a resource within a browser window */
{ tBrowserDocument bd; /* "bd": "browser document" or "base document" */
  struct tWindow* window; /* the window to which the view belongs */
  struct tWindowView *prev, *next; /* the list of views within that window */
} tWindowView; /* CHECKME: rename! */

typedef struct
{ tWindowView *first_view, *current_view;
#if TGC_IS_GRAPHICS
  tGraphicsWindow* ww; /* "window widget" */
  tGraphicsWidget *contents, *location, *info, *message;
  tGraphicsLowlevelWindow* drawable; /* for the actual contents */
  tGraphicsContext* gc; /* ditto */
#endif
} tWkBrowserData; /* for window->wksd */


/** Browser window stuff */

static void wk_browser_redraw_message(tBrowserDocument* document)
{ const tWindowView* view = (const tWindowView*) (document->container);
  const tWindow* window = view->window;
  const tResourceRequest* request = document->request;
  const tResource* resource = request->resource;
  tBoolean is_error;
  const char *message = calculate_reqresmsg(request, resource, 0, &is_error);
  const size_t msgsize = strlen(message) + 1;
  char* temp = __memory_allocate(msgsize + 1, mapString); /* ("+1": flags) */
#if TGC_IS_CURSES
  if (window == current_window_x)
#endif
  { show_message(message, is_error); }
  *temp = boolean2bool(is_error); /* (flags - just is_error for now) */
  my_memcpy(temp + 1, message, msgsize);
  __dealloc(document->last_info); document->last_info = temp;
}

static void wk_browser_redraw_title(/*@notnull@*/ const tBrowserDocument*
  document, short titlerow)
{ const tWindowView* view = (const tWindowView*) (document->container);
  const tWindow* window = view->window;
  tResource* resource = document->request->resource;
  char* temp;
#if TGC_IS_GRAPHICS
  tGraphicsWindow* ww = window->ww;
  char* window_title = unconstify(strProgramVersion);
  char* s = strbuf;
  if (document->html_title != NULL)
    window_title = unconstify(document->html_title);
  if (document->title != NULL)
    gtk_label_set_text(GTK_LABEL(window->location), document->title);
#if OPTION_COOKIES
  if ( (resource != NULL) && (resource->flags & rfCookieAnything) )
    s += sprint_safe(s, strPercs, _(strCookies));
#endif
  if (document->flags & wvfHandledRedirection)
    s += sprint_safe(s, strPercsDash, _(strRedirection));
  else if (document->bddm == bddmSource)
    s += sprint_safe(s, strPercsDash, _(strSource));
  *s = '\0';
  temp = strbuf;
  gtk_window_set_title(ww, window_title);
  gtk_label_set_text(GTK_LABEL(window->info), temp);

#else /* #if TGC_IS_GRAPHICS */

  const tCantent* cantent = document->cantent;
  const char *major = ( (cantent != NULL) ? (cantent->major_html_title) :
    NULL ), *html_title = ( (major != NULL) ? major :
    (document->minor_html_title) ), *title = document->title;
#define dashify(cond, str) \
  ( (cond) ? str : strEmpty ), ( (cond) ? strSpacedDash : strEmpty )
  my_spf(strbuf, STRBUF_SIZE, &temp,
#if OPTION_COOKIES
/*A*/ "%s"
#endif
/*BCDE*/ "%s%s%s%s%s%s%s"
    ,
#if OPTION_COOKIES
/*A*/ ( ( (resource != NULL) && (resource->flags & rfCookieAnything) )
/*A*/   ? _(strCookies) : strEmpty ), /* RFC2965, 6.1, *2 */
#endif
/*B*/ dashify((document->flags & wvfHandledRedirection), _(strRedirection)),
/*C*/ dashify((document->bddm == bddmSource), _(strSource)),
/*D*/ dashify((html_title != NULL), html_title),
/*E*/ null2empty(title)
  );
#undef dashify
  cu_display_title(window, temp, titlerow); my_spf_cleanup(strbuf, temp);

#endif /* #if TGC_IS_GRAPHICS */
}

#if DO_WK_INFO

typedef struct tIbuf
{ struct tIbuf* next;
  const char* str;
} tIbuf;

typedef struct
{ tIbuf *head, *tail;
} tIbuffer; /* "ibuffer" like "inode" :-) */

my_enum1 enum
{ iiInvalid = -1, iiImage = 0, iiForm = 1, iiFrame = 2, iiLink = 3
} my_enum2(signed char) tIbufferIndex;
#define iiMax (iiLink)
static const char* const strIheader[iiMax + 1] =
{ N_("Images: "), N_("Forms: "), N_("Frames: "), N_("Links: ") };

static one_caller void wk_browser_build_document_info(/*@notnull@*/
  tBrowserDocument* document)
{ static const char strMasterPre[] = "<ul>\n", strMasterPost[] = "</ul>\n",
      strIpre[] = "<li>", strIpost[] = "</li>\n";
  tWindow* iw; /* "info window" */
  tIbuffer ibuf[iiMax + 1];
  tIbufferIndex idx;
  const tHtmlNode* node;
  const tResourceRequest* request = document->request;
  const tUriData* req_ud = request->uri_data;
  const tResource* resource = request->resource;
  tCantent* cantent = document->cantent;
  const char *uri = document->title, *html_title;
  tActiveElementNumber _ae, __ae;
  tHtmlFormNumber _hfn, __hfn;
  char* spfbuf;

  my_spf(NULL, 0, &spfbuf, _("Information about \"%s\""), ( (uri != NULL) ? uri
    : _(strUnknown) ));
  iw = visible_window_x[1 - current_window_index_x] =
    wk_info_create(my_spf_use(spfbuf), bddmHtml, NULL);

  if (uri != NULL)
  { const char* html_uri = htmlify(uri);
    my_spf(strbuf, STRBUF_SIZE, &spfbuf, "<p>%s<a href=\"%s\">%s</a></p>\n",
      _(strUriColonSpace), html_uri, html_uri);
    htmlify_cleanup(uri, html_uri); wk_info_collect(iw, spfbuf);
    my_spf_cleanup(strbuf, spfbuf);
  }

  html_title = ( (cantent != NULL) ? cantent->major_html_title : NULL );
  if (html_title == NULL) html_title = document->minor_html_title;
  if ( (html_title != NULL) && (*html_title != '\0') )
  { const char* htitle = htmlify(html_title);
    my_spf(strbuf, STRBUF_SIZE, &spfbuf, "<p>%s\"%s\"</p>\n", _("Title: "),
      htitle);
    htmlify_cleanup(html_title, htitle);
    wk_info_collect(iw, spfbuf); my_spf_cleanup(strbuf, spfbuf);
  }

  if (resource != NULL)
  { const size_t size = resource->bytecount;
    my_spf(strbuf, STRBUF_SIZE, &spfbuf, "<p>%s%d %s</p>\n", _("Size: "),
      localized_size(size), bytebytes(size));
    wk_info_collect(iw, spfbuf); my_spf_cleanup(strbuf, spfbuf);
  }

  if ( (cantent == NULL) ||
       ( (document->bddm != bddmHtml) && (cantent->kind != rckHtml) ) )
    goto not_html;

  /* Currently we must run the whole document once through the renderer to get
     the handling of "inside <pre> tag" correctly in the permanently stored
     tree of HTML nodes. Rubbish... IMPROVEME! */
#if 0
  { const tLinenumber l = document->origin_y;
    const tActiveElementNumber a = document->aecur;
    document_display(document, wrtToEnd);
    if (document->origin_y != l) /* ("likely") */
    { document->origin_y = l; document->aecur = a;
      document_display(document, wrtRedraw);
    }
  }
#else
  document_display(document, wrtToEnd);
#endif

  parser_html_start(cantent); my_memclr_arr(ibuf);
  _ae = __ae = INVALID_AE; _hfn = __hfn = INVALID_HTML_FORM_NUMBER;
  while ( (node = parser_html_next(falsE)) != NULL )
  { const tHtmlTagKind htk = node->kind;
    const tHtmlNodeFlags hnf = node->flags;
    const tBoolean has_aebase = cond2boolean(hnf & hnfHasAeBase);
    const tActiveElementBase* aeb;
    const tAttribute* attr;
    const char *str, *hstr;
    if (has_aebase) { __ae++; _ae = __ae; }
    else _ae = INVALID_AE;
    if (hnf & hnfGoodForm) { __hfn++; _hfn = __hfn; }
    else _hfn = INVALID_HTML_FORM_NUMBER;
    if (htk == htkText) goto do_next;
    idx = iiInvalid;
    /* CHECKME: avoid duplicates in the lists of links and images? */
    switch (htk)
    { case htkA:
        if (_ae == INVALID_AE) goto do_next;
        aeb = &(cantent->aebase[_ae]);
        if ( ( (str = aeb->data) != NULL ) && (*str != '\0') )
        { tUriData* uri_data;
          idx = iiLink;
          use_linkstr:
          uri_data = uri_parse(str, req_ud, NULL, NULL, 2);
          /* CHECKME: use the "proper" referrer! */
          str = uri_data->uri; hstr = htmlify(str);
          my_spf(NULL, 0, &spfbuf, "%s<a href=\"%s\">%s</a>%s", strIpre, hstr,
            hstr, strIpost);
          htmlify_cleanup(str, hstr); uri_put(uri_data);
        }
        break;
      case htkImg:
        attr = find_attribute(node, anSrc);
        if ( (attr != NULL) && ( (str = attr->value) != NULL ) && (*str!='\0'))
        { idx = iiImage; goto use_linkstr; }
        break;
      case htkForm:
        if (_hfn != INVALID_HTML_FORM_NUMBER)
        { const tHtmlForm* f = &(cantent->form[_hfn]);
          const char* au = f->action_uri;
          if ( (au != NULL) && (*au != '\0') )
          { const tHtmlFormFlags hff = f->flags;
            my_spf(NULL, 0, &spfbuf, "%s\"%s\"%s%s%s", strIpre, au,
              ( (hff & hffMethodPost) ? " - POST" : strEmpty ),
              ( (hff & hffEncodingMultipart) ? " - multipart" : strEmpty ),
              strIpost);
            idx = iiForm;
          }
        }
        break;
      case htkFrame:
        if (_ae == INVALID_AE) goto do_next;
        aeb = &(cantent->aebase[_ae]);
        if ( ( (str = aeb->data) != NULL ) && (*str != '\0') )
        { idx = iiFrame; goto use_linkstr; }
        break;
    }
    if (idx >= 0)
    { tIbuf* ib = __memory_allocate(sizeof(tIbuf), mapOther);
      ib->next = NULL; ib->str = my_spf_use(spfbuf);
      if (ibuf[idx].tail != NULL) ibuf[idx].tail->next = ib;
      ibuf[idx].tail = ib;
      if (ibuf[idx].head == NULL) ibuf[idx].head = ib;
    }
    do_next:
    if (!(hnf & hnfStoredInTree)) deallocate_html_node(node);
  }
  parser_html_finish();

  for (idx = 0; idx <= iiMax; idx++)
  { tIbuf* ib = ibuf[idx].head;
    if (ib == NULL) goto do_next_idx; /* no entries exist for this one */
    sprint_safe(strbuf, "<p>%s</p>\n%s", _(strIheader[idx]), strMasterPre);
    wk_info_collect(iw, strbuf);
    while (ib != NULL)
    { tIbuf* next = ib->next;
      const char* str = ib->str;
      wk_info_collect(iw, str);
      memory_deallocate(str); memory_deallocate(ib); ib = next;
    }
    wk_info_collect(iw, strMasterPost);
    do_next_idx: {}
  }

  not_html: {}
  wk_info_finalize(iw);
}

#endif /* #if DO_WK_INFO */


/** Browser window document operations interface */

static tBoolean wk_browser_dop_find_coords(const tBrowserDocument* document,
  /*@out@*/ short* _x1, /*@out@*/ short* _y1, /*@out@*/ short* _x2,
  /*@out@*/ short* _y2)
{ const tWindowView* view = (const tWindowView*) (document->container);
  const tWindow* window = view->window;
  short minrow, maxrow;
  const tBoolean retval = window_contents_minmaxrow(window, &minrow, &maxrow);
  if (retval) { *_x1 = 0; *_y1 = minrow; *_x2 = COLS - 1; *_y2 = maxrow; }
  return(retval);
}

static void wk_browser_dop_display_meta1(tBrowserDocument* document)
{ wk_browser_redraw_message(document);
}

static void wk_browser_dop_display_meta2(tBrowserDocument* document)
{ short minrow, maxrow;
  if (document_minmaxrow(document, &minrow, &maxrow))
    wk_browser_redraw_title(document, maxrow + 1);
}

static const tBrowserDocumentOps wk_browser_document_ops =
{ wk_browser_dop_find_coords, wk_browser_dop_display_meta1,
  wk_browser_dop_display_meta2
};


/** Browser window view handling */

static void deallocate_windowviewlist(const tWindowView* view)
/* deallocates the given list of views and drops associated request and
   resource data */
{ while (view != NULL)
  { const tWindowView* next = view->next;
    document_tear(&(view->bd)); memory_deallocate(view); view = next;
  }
}

static void cut_windowviewlist(tWindow* window)
/* "cuts" the current view and all its successors out of the <window> and
   deallocates them */
{ tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  tWindowView* view = wksd->current_view;
  if (view != NULL)
  { if (view->prev != NULL) view->prev->next = NULL;
    wksd->current_view = view->prev;
    if (wksd->first_view == view) wksd->first_view = NULL;
    deallocate_windowviewlist(view);
  }
}

static /* __sallocator -- not an "only" reference... */ tWindowView*
  __callocator wk_browser_append_view(tWindow* window)
/* prepares a new view in the given browser window and removes any old "tail"
   of views in that window */
{ tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  tWindowView *current_view = wksd->current_view,
    *new_view = memory_allocate(sizeof(tWindowView), mapWindowView);
  new_view->window = window;
  if (current_view != NULL) /* wasn't a new (empty) window */
  { const tWindowView* tail = current_view->next;
    if (tail != NULL) deallocate_windowviewlist(tail); /* remove old tail */
    current_view->next = new_view; new_view->prev = current_view;
  }
  wksd->current_view = new_view;
  if (wksd->first_view == NULL) wksd->first_view = new_view;
  return(new_view);
}

static void wk_browser_request_callback(void* _document,
  tDhmNotificationFlags flags)
{ tBrowserDocument* document = (tBrowserDocument*) _document;
  tResourceRequest* request = document->request;
  tResource* resource = request->resource;
  tWindowView* view = (tWindowView*) (document->container);
  tWindow* window = view->window;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "wk_browser_request_callback(): document=%p, request=%p, resource=%p, dhmnf=%d\n", document, request, resource, flags);
  debugmsg(debugstrbuf);
#endif
  if (resource != NULL) request_copy_error(request, resource);
  if (flags & (dhmnfDataChange | dhmnfMetadataChange))
  { tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
#if MIGHT_USE_SCROLL_BARS
    document->sbvi = 0;
#endif
    if ( (window_is_visible(window)) && (wksd->current_view == view) )
    { if (request->flags & rrfResourceChanged) /* REMOVEME?! */
        request->flags &= ~rrfResourceChanged;
      else if (document->flags & wvfScreenFull)
        document->flags |= wvfDontDrawContents;
      document_display(document, wrtRedraw);
    }
  }
  else if (flags & dhmnfAttachery) /* a resource was attached to the request */
  { dhm_notification_setup(resource, wk_browser_request_callback,
      document, dhmnfDataChange | dhmnfMetadataChange, dhmnSet);
    cantent_attach(document->cantent, resource->cantent);
  }
  test_redirection(window, document);
}

static void wk_browser_prr(tWindow* window, const char* uri, tPrrFlags prrf,
  const tBrowserDocument* referrer)
/* prepares a resource request in a browser window */
{ tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  tWindowView *current_view = wksd->current_view, *new_view;
  tPrrData prr_data;
  tResourceRequest* request;
  tBrowserDocument* document;
  const char* anchor;
  unsigned char redirections = 0;

  prr_setup(&prr_data, uri, prrf | prrfWantUriAnchor);
  prr_data.referrer = referrer; prepare_resource_request(&prr_data);
  request = prr_data.result; prr_data.result = NULL;
  anchor = prr_data.uri_anchor; prr_data.uri_anchor = NULL;

  if ( (prrf & prrfIsRedirection) && (referrer != NULL) )
  { const unsigned char count = redirections = referrer->redirections + 1;
    if (count > config.redirections)
    { redirection_error: request_set_error(request, reRedirection);
      goto redirection_checked;
    }
    if ( (prrf & prrfIsHttpRedirection) && (referrer != NULL) )
    { const tResource* res = referrer->request->resource;
      if ( (res != NULL) && (res->flags & rfPost) )
        goto redirection_error; /* RFC2616, 9.3 */
    }
  }
  redirection_checked: {}

  new_view = wk_browser_append_view(window);
  document = &(new_view->bd);
  document_init(document, &wk_browser_document_ops, new_view, request);
  document->anchor = anchor; document->redirections = redirections;
  document->title = my_strdup(request->uri_data->uri);
  if (prrf & prrfPost) document->flags |= wvfPost;
  if (prrf & prrfSource)
  { document->bddm = bddmSource;
    if (current_view != NULL)
    { const char* html_title = current_view->bd.minor_html_title;
      if (html_title != NULL)
        document->minor_html_title = my_strdup(html_title);
    }
  }
  else if (prrf & prrfHtml) document->bddm = bddmHtml;

  if (prrf & prrfRedrawAll) window_redraw_all();
  else if (prrf & prrfRedrawOne) window_redraw(window);
  if (request->state != rrsError)
  { tResourceRequestAction action;
    if (prrf & prrfEnforcedReload) action = rraEnforcedReload;
    else if (prrf & prrfReload) action = rraReload;
    else
    { action = rraLoad;
#if CONFIG_MENUS & MENUS_UHIST
      if (is_environed)
      { my_strdedup(uri_history[uri_history_index], request->uri_data->uri);
        if (uri_history_index < URI_HISTORY_LEN - 1) uri_history_index++;
        else uri_history_index = 0;
        /* IMPLEMENTME: if the URI already is in the list, just "move it" to
           the top of the list! */
      }
#endif
    }
    dhm_notification_setup(request, wk_browser_request_callback, document,
      dhmnfDataChange | dhmnfMetadataChange | dhmnfAttachery, dhmnSet);
    request_queue(request, action);
  }
  prr_setdown(&prr_data);
}

#if TGC_IS_GRAPHICS
static one_caller void wk_browser_build_graphics(tWindow* window)
{ tGraphicsWindow* w = (GtkWindow*) gtk_window_new(GTK_WINDOW_TOPLEVEL);
  tGraphicsWidget *base=gtk_vbox_new(FALSE,0), *fluff=gtk_vbox_new(FALSE,0),
    *buttons = gtk_hbox_new(FALSE, 0), *texts = gtk_vbox_new(FALSE, 0),
    *contents = gtk_drawing_area_new(), *url0 = gtk_hbox_new(FALSE, 0),
    *url1 = gtk_label_new(_("Location: ")), *url2 = gtk_label_new(strEmpty),
    *info0 = gtk_hbox_new(FALSE, 0), *info1 = gtk_label_new(strEmpty),
    *info2 = gtk_label_new(strEmpty), *mbar = gtk_menu_bar_new(),
    *mFile = create_menu(mbar, _(strFileUc)), *mView = create_menu(mbar,
    _("View")), *mBookmark = create_menu(mbar, _("Bookmark")),
    *mWindow = create_menu(mbar, _("Window"));
  tGraphicsLowlevelWindow *some_parent, *drawable;
  GdkWindowAttr* drawattrs = memory_allocate(sizeof(GdkWindowAttr), mapGtk);
  GtkStyle* style;
  tGraphicsContext* gc;
  static tBoolean is_first_call = truE;
  static char *strHome, *strStop;
  static GdkPixmap* logo_mini_pixmap;
  static GdkBitmap* logo_mini_mask;
  GtkAccelGroup* accel = gtk_accel_group_new();

  if (is_first_call)
  { tGraphicsLowlevelWindow* template_window;
    GdkWindowAttr* attr = memory_allocate(sizeof(GdkWindowAttr), mapGtk);
    is_first_call = falsE;
    strHome = i18n_strdup(_("Home")); strStop = i18n_strdup(_("Stop"));
    attr->window_type = GDK_WINDOW_CHILD;
    template_window = gdk_window_new(NULL, attr, 0);
    logo_mini_pixmap = gdk_pixmap_create_from_xpm_d(template_window,
      &logo_mini_mask, NULL, (gchar**) retawq_logo_mini);
  }

  gtk_window_set_policy(w, TRUE /* CHECKME: documentation contradicts it! */,
    TRUE, FALSE);
  gtk_window_set_title(GTK_WINDOW(w), strProgramVersion);
  gtk_window_add_accel_group(GTK_WINDOW(w), accel);

  gtk_drawing_area_size(GTK_DRAWING_AREA(contents), config.width,
    config.height);
  gtk_widget_set_events(contents, GDK_EXPOSURE_MASK);

  /* basic window structure */
  gtk_container_add(GTK_CONTAINER(w), base);
  pack_box(base, mbar); pack_box(base, fluff); pack_box(base, contents);
  pack_box(fluff, buttons); pack_box(fluff, texts);
  pack_box(texts, url0); pack_box(texts, info0);
  pack_box(url0, url1); pack_box(url0, url2);
  pack_box(info0, info1); pack_box(info0, info2);

  /* menus */
  create_item(mFile, _("New Window"), pccWindowNew, 'n');
  create_item(mFile, _(strOpenInNewWindow), pccWindowNewFromDocument, 'N');
  create_item(mFile, _("Open Local File..."), pccLocalFileDirOpen, 'O');
  create_item(mFile, _("Open URL..."), pccGoUri, 'o');
  create_item(mFile, _("Open Relative URL..."), pccGoUriPreset, '\0');
  create_item(mFile, _(strSaveAs), pccDocumentSave, 's');
  create_item(mFile, _(strUcClose), pccWindowClose, 'c');
  create_item(mFile, _(strUcQuit), pccQuit, 'q');
  create_item(mView, _(strBack), pccViewBack, '\0');
  create_item(mView, _(strForward), pccViewForward, '\0');
  create_item(mView, _(strReload), pccDocumentReload, 'R');
  if (config.home_uri != NULL) create_item(mView, strHome,pccGoHome, 'h');
  if (config.search_engine != NULL)
    create_item(mView, _(strSearch), pccGoSearch, 'e');
  create_item(mView, strStop, pccStop, '.');
  create_item(mView, _("Document Info"), pccUnknown, 'I');
  create_item(mView, _(strSourceCode), pccDocumentEnforceSource, '\\');
  create_item(mBookmark, _(strAddBookmark), pccUnknown, '\0');
  create_item(mWindow, "(Titles of all windows)", pccUnknown, '\0');

  /* buttons */
  create_button(retval, buttons, _(strBack), pccViewBack);
  create_button(retval, buttons, _(strForward), pccViewForward);
  create_button(retval, buttons, _(strReload), pccDocumentReload);
  if (config.home_uri != NULL)
    create_button(retval, buttons, strHome, pccGoHome);
  if (config.search_engine != NULL)
    create_button(retval, buttons, _(strSearch), pccGoSearch);
  create_button(retval, buttons, strStop, pccStop);

  /* show all */
  show_widget(url0); show_widget(url1); show_widget(url2);
  show_widget(info0); show_widget(info1); show_widget(info2);
  show_widget(mbar); show_widget(buttons); show_widget(texts);
  show_widget(fluff); show_widget(contents); show_widget(base);
  show_widget(w);

  /* GDK window and gc for contents */
  drawattrs->event_mask = gtk_widget_get_events(contents) | GDK_EXPOSURE_MASK
    | GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK;
  drawattrs->x = contents->allocation.x;
  drawattrs->y = contents->allocation.y;
  drawattrs->width = contents->allocation.width;
  drawattrs->height = contents->allocation.height;
  drawattrs->wclass = GDK_INPUT_OUTPUT;
  drawattrs->visual = gtk_widget_get_visual(contents);
  drawattrs->colormap = gtk_widget_get_colormap(contents);
  drawattrs->window_type = GDK_WINDOW_CHILD;
  some_parent = gtk_widget_get_parent_window(contents);
  drawable = gdk_window_new(some_parent, drawattrs,
    GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP);
  gdk_window_set_icon(some_parent, NULL, logo_mini_pixmap, logo_mini_mask);
  style = gtk_widget_get_style(contents);
  gc = gdk_gc_new(drawable);
  gdk_window_set_background(drawable, &(style->white));
  gdk_gc_set_foreground(gc, &(style->black));
  gdk_gc_set_background(gc, &(style->white));
  gdk_window_show(drawable);

  /* store */
  retval->ww = w;
  retval->contents = contents; retval->location = url2;
  retval->info = info1; retval->message = info2;
  retval->drawable = drawable; retval->gc = gc;

  /* connect signal handlers */
  connect_window(w, strGtkDestroy, window_handle_destroy, retval);
  connect_window(w, strGtkDelete, window_handle_delete, retval);
  connect_widget(contents, strGtkConfigure, window_handle_configure, retval);
  connect_widget(contents, strGtkKey, graphics_propagate_event, NULL);
  connect_widget(contents, strGtkButton, graphics_cm, retval);
}
#endif


/** Browser window operations interface */

static void wop_browser_create(tWindow* window)
{ window->wksd = memory_allocate(sizeof(tWkBrowserData), mapWksd);
#if TGC_IS_GRAPHICS
#endif
}

static void wop_browser_remove(tWindow* window)
{ const tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  const tWindowView* view = wksd->first_view;
  if (view != NULL) deallocate_windowviewlist(view);
#if TGC_IS_GRAPHICS
#endif
  memory_deallocate(wksd);
}

static void wop_browser_redraw(const tWindow* window)
{ const tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  tWindowView* view = wksd->current_view;
  tBrowserDocument* document = ( (view != NULL) ? (&(view->bd)) : NULL );
  short i, minrow, maxrow;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "wop_browser_redraw(): window=%p, view=%p, document=%p\n", window, view, document);
  debugmsg(debugstrbuf);
#endif
  if (document != NULL) document_display(document, wrtRedraw);
  else if (window_contents_minmaxrow(window, &minrow, &maxrow))
  { for (i = minrow; i <= maxrow; i++) { (void) move(i,0); (void) clrtoeol(); }
    cu_display_title(window, _(strBracedNewWindow), maxrow + 1);
    if (window == current_window_x) show_message(strEmpty, falsE);
  }
  must_reset_cursor();
}

static tBoolean wop_browser_is_precious(const tWindow* window)
{ const tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  return(cond2boolean(wksd->first_view != NULL));
}

static tBoolean wop_browser_handle_pcc(tWindow* window,tProgramCommandCode pcc)
{ tBoolean retval = truE;
  tWkBrowserData* wksd;
  tWindowView* view;
  switch (pcc)
  {case pccViewBack:
    wksd = (tWkBrowserData*) (window->wksd); view = wksd->current_view;
    if ( (view != NULL) && (view->prev != NULL) )
    { wksd->current_view = view->prev; window_redraw(window); }
    break;
   case pccViewForward:
    wksd = (tWkBrowserData*) (window->wksd); view = wksd->current_view;
    if ( (view != NULL) && (view->next != NULL) )
    { wksd->current_view = view->next; window_redraw(window); }
    break;
   case pccDocumentReload: case pccDocumentReloadEnforced:
    { tBrowserDocument* document;
      tBoolean is_post;
      wksd = (tWkBrowserData*) (window->wksd); view = wksd->current_view;
      if (view == NULL) goto out;
      document = &(view->bd); is_post = cond2boolean(document->flags &wvfPost);
      lid.prrf = ( (pcc == pccDocumentReloadEnforced) ? prrfEnforcedReload :
        prrfReload );
      if (is_post) lid.prrf |= prrfPost;
      if ( (!is_post) || (config.flags & cfDontConfirmRepost) )
        wk_browser_reload(window);
      else ask_yesno(_("Really repost form data?"), cgRepost, NULL);
    }
    break;
   case pccDocumentEnforceSource:
    { const tBrowserDocument* document = window_currdoc(window);
      if ( (document != NULL) && (document->bddm != bddmSource) )
      { wk_browser_prr(require_browser_window(), document->title,
          prrfRedrawOne | prrfSource, NULL);
      }
    }
    break;
   case pccDocumentInfo:
#if DO_WK_INFO
    wksd = (tWkBrowserData*) (window->wksd); view = wksd->current_view;
    if (view != NULL) wk_browser_build_document_info(&(view->bd));
#else
    fwdact(_("Finformation window"));
#endif
    break;
   default: retval = falsE; break; /* not handled here, try generic handler */
  }
  out:
  return(retval);
}

static tBrowserDocument* wop_browser_find_currdoc(const tWindow* window)
{ const tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  tWindowView* view = wksd->current_view;
  return( (view != NULL) ? (&(view->bd)) : NULL );
}

static const char* wop_browser_get_info(tWindow* window,
  /*@out@*/ tBoolean* _is_error)
{ const tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  const tWindowView* view = wksd->current_view;
  const char* retval = ( (view != NULL) ? (view->bd.last_info) : NULL );
  *_is_error = ( (retval != NULL) ? cond2boolean(*retval++ & 1) : falsE );
  return(retval);
}

static const char* wop_browser_get_menu_entry(tWindow* window)
{ const tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  const tWindowView* view = wksd->current_view;
  const tBrowserDocument* document;
  const tCantent* cantent;
  const char* text;
  if (view == NULL) { text = _(strBracedNewWindow); goto out; }
  document = &(view->bd); cantent = document->cantent;
  if ( (cantent != NULL) && ( (text = cantent->major_html_title) != NULL ) )
    goto out;
  if ( (text = document->minor_html_title) != NULL ) goto out;
  if ( (text = document->title) != NULL ) goto out;
  text = unconstify_or_(strUnknown); /* "should not happen" */
  out:
  return(text);
}

#if CONFIG_MENUS & MENUS_CONTEXT
static void wop_browser_setup_cm(tWindow* window, short* _x, short* _y,
  tActiveElementNumber _ae)
{ const tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  const tWindowView* view = wksd->current_view;
  if (view != NULL)
  { const tBrowserDocument* document = &(view->bd);
    const tResourceRequest* request = document->request;
    const char* uri = ( (request != NULL) ? request->uri_data->uri : NULL );
    short x, y;

    /* ae-specific entries */
    if (_ae != INVALID_AE)
    { const tCantent* cantent = document->cantent;
      const tActiveElementBase* aeb = cantent->aebase;
      tActiveElementKind aek = ( (aeb != NULL) ? aeb[_ae].kind : aekUnknown );
      const tActiveElementCoordinates* aec = find_visible_aec(document, _ae);
      if ( (aec != NULL) && (document_line2row(document, aec->y, &y)) )
      { x = aec->x1; y++; *_x = x; *_y = y; }
      cm_add(strShowElementInfo, cm_handle_command_code, pccElementInfo);
      if ( (aeb != NULL) && (aeb[_ae].flags & aefDisabled) )
        cm_add(strEnableThisElement, cm_handle_command_code, pccElementEnable);
      if (aek == aekLink)
      { cm_add(strOpenLinkInNewWindow, cm_handle_command_code,
          pccWindowNewFromElement);
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
        cm_add(strSaveLinkAs, cm_handle_command_code, pccDownloadFromElement);
#endif
      }
      else if (is_form_aek(aek))
      { tHtmlFormNumber hfn = calc_hfn(cantent, _ae);
        if (hfn != INVALID_HTML_FORM_NUMBER)
        { cm_add(strSubmitThisForm, cm_handle_command_code, pccFormSubmit);
          /* IMPLEMENTME: "Show Form Info" */
        }
      }
      cm_add_separator();
    }

    /* general entries */
    if (view->prev != NULL)
      cm_add(strBack, cm_handle_command_code, pccViewBack);
    if (view->next != NULL)
      cm_add(strForward, cm_handle_command_code, pccViewForward);
    if ( (uri != NULL) && (*uri != '\0') )
    { cm_add(strReload, cm_handle_command_code, pccDocumentReload);
      cm_add(strEnforcedReload, cm_handle_command_code,
        pccDocumentReloadEnforced);
      cm_add(strSaveAs, cm_handle_command_code, pccDocumentSave);
      cm_add(strOpenInNewWindow, cm_handle_command_code,
        pccWindowNewFromDocument);
#if 0
      cm_add(strAddBookmark, cm_handle_command_code, pccBookmarkAdd);
#endif
    }
    cm_add(strSourceCode, cm_handle_command_code, pccDocumentEnforceSource);
    cm_add_separator();
  }
}
#endif /* #if CONFIG_MENUS & MENUS_CONTEXT */

#if CONFIG_SESSIONS
static void wop_browser_save_session(tWindow* window, int fd)
{ tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  tWindowView *view = wksd->first_view, *currview = wksd->current_view;
  tBoolean is_fuiw = truE; /* first URI in window? */
  while (view != NULL)
  { const tBrowserDocument* document = &(view->bd);
    const tResourceRequest* request;
    const tWindowViewFlags wvf = document->flags;
    tBrowserDocumentDisplayMode bddm;
    const char *uri, *html_title, *flagsptr, *linenumptr;
    char* spfbuf;
    tLinenumber linenum;
    tBoolean is_currview;

    if (wvf & wvfPost) goto nextview; /* can't handle this correctly */
    is_currview = cond2boolean(view == currview); request = document->request;
    if (request != NULL)
    { const tResource* resource = request->resource;
      if ( (resource != NULL) && ( (uri = resource->uri_data->uri) != NULL ) &&
           (*uri != '\0') )
      { goto write_uri; }
      if ( ( (uri = request->uri_data->uri) != NULL ) && (*uri != '\0') )
        goto write_uri;
    }
    if ( ( (uri = document->title) != NULL ) && (*uri != '\0') )
      goto write_uri;
    goto nextview; /* no usable URI found */

    write_uri:
    if (is_fuiw)
    { char* temp = winbuf + 2;
#if TGC_IS_CURSES
      unsigned char num;
      for (num = 0; num < 2; num++)
      { if (window == visible_window_x[num])
        { psm(temp, 'v'); temp = bdp(temp); *temp++ = '0' + num; break; }
      }
#endif
      *temp++ = '\n'; *temp = '\0'; my_write_str(fd, winbuf); is_fuiw = falsE;
    }

    bddm = document->bddm;
    if ( (bddm != bddmAutodetect) || (is_currview) )
    { char* temp = bdp(flagsbuf);
      if (bddm == bddmSource) *temp++ = 's';
      else if (bddm == bddmHtml) *temp++ = 'h';
      if (is_currview) *temp++ = 'c';
      *temp = '\0';
      flagsptr = flagsbuf;
    }
    else flagsptr = strEmpty;

    linenum = document->origin_y;
    if (linenum <= 0) linenumptr = strEmpty;
    else
    { sprint_safe(bdp(linenumbuf), strPercd, linenum); linenumptr = linenumbuf;
    }

    html_title = ( (bddm == bddmSource) ? document->minor_html_title : NULL );
    if ( (html_title != NULL) && (*html_title == '\0') ) html_title = NULL;

    my_spf(strbuf, STRBUF_SIZE, &spfbuf, "U:%s%s%s%s%s%s\n", flagsptr,
      linenumptr, urimarker, uri, ( (html_title != NULL) ? htmltitlemarker :
      strEmpty ), null2empty(html_title));
    my_write_str(fd, spfbuf); my_spf_cleanup(strbuf, spfbuf);

    nextview: view = view->next;
  }
}
#endif /* #if CONFIG_SESSIONS */

#if CONFIG_DO_TEXTMODEMOUSE
static void wop_browser_handle_mouse(tWindow* window __cunused, tCoordinate x
  __cunused, tCoordinate y __cunused, int flags __cunused)
{ /* IMPLEMENTME! */
}
#endif

static const tWindowSpec wk_browser_spec =
{ wop_browser_create, wop_browser_remove, wop_browser_redraw,
  wop_browser_is_precious, NULL, wop_browser_handle_pcc,
  wop_browser_find_currdoc, wop_browser_get_info, wop_browser_get_menu_entry,
  WSPEC_CM(wop_browser_setup_cm) WSPEC_SESSION(wop_browser_save_session)
  WSPEC_MOUSE(wop_browser_handle_mouse) 'B', 'B', wkBrowser, wsfNone
};


/** Browser window stuff */

static void wk_browser_reload(tWindow* window)
{ const tWkBrowserData* wksd = (tWkBrowserData*) (window->wksd);
  const tWindowView* view = wksd->current_view;
  const tBrowserDocument* document;
  const char* uri;
  tBoolean do_post;
  tPrrFlags prrf;
  if (view == NULL) return; /* "should not happen" */
  document = &(view->bd); uri = my_strdup(document->title);
  prrf = lid.prrf | prrfRedrawOne; do_post = cond2boolean(prrf & prrfPost);
  if (do_post)
  { const char* post = document->request->uri_data->post;
    sfbuf = my_strdup(null2empty(post));
    sfbuf_size = sfbuf_maxsize = strlen(sfbuf) + 1;
    prrf |= prrfUseSfbuf;
  }
  cut_windowviewlist(window);
  wk_browser_prr(window, uri, prrf, NULL); memory_deallocate(uri);
}

static tWindow* wk_browser_create(void)
{ return(window_create(&wk_browser_spec)); /* just a frequently used wrapper */
}


/*** Window kind: information window */

#if DO_WK_INFO

typedef struct
{ tBrowserDocument document; /* exactly one document per window */
  const char *window_title, *message;
  tBoolean is_finalized, message_is_error;
} tWkInfoData; /* for window->wksd */


/** Information window document operations interface */

static tBoolean wk_info_dop_find_coords(const tBrowserDocument* document,
  /*@out@*/ short* _x1, /*@out@*/ short* _y1, /*@out@*/ short* _x2,
  /*@out@*/ short* _y2)
{ const tWindow* window = (const tWindow*) (document->container);
  short minrow, maxrow;
  const tBoolean retval = window_contents_minmaxrow(window, &minrow, &maxrow);
  if (retval) { *_x1 = 0; *_y1 = minrow; *_x2 = COLS - 1; *_y2 = maxrow; }
  return(retval);
}

static void wk_info_dop_display_meta1(tBrowserDocument* document)
{
#if TGC_IS_CURSES
  const tWindow* window = (const tWindow*) (document->container);
  if (window == current_window_x)
#endif
  { const tWkInfoData* wksd = (const tWkInfoData*) (window->wksd);
    const char* msg = wksd->message;
    show_message(null2empty(msg), wksd->message_is_error);
  }
}

static void wk_info_dop_display_meta2(tBrowserDocument* document)
{ short minrow, maxrow;
  if (document_minmaxrow(document, &minrow, &maxrow))
  { const tWindow* window = (const tWindow*) (document->container);
    const tWkInfoData* wksd = (const tWkInfoData*) (window->wksd);
    cu_display_title(window, wksd->window_title, maxrow + 1);
  }
}

static const tBrowserDocumentOps wk_info_document_ops =
{ wk_info_dop_find_coords, wk_info_dop_display_meta1, wk_info_dop_display_meta2
};


/** Information window operations interface */

static void wop_info_create(tWindow* window)
{ tWkInfoData* wksd = window->wksd = memory_allocate(sizeof(tWkInfoData),
    mapWksd);
  tBrowserDocument* document = &(wksd->document);
  tCantent* cantent = cantent_create();
  document_init(document, &wk_info_document_ops, window, NULL);
  cantent_attach(document->cantent, cantent);
}

static void wop_info_remove(tWindow* window)
{ tWkInfoData* wksd = (tWkInfoData*) (window->wksd);
  document_tear(&(wksd->document)); __dealloc(wksd->window_title);
  memory_deallocate(wksd);
}

static void wop_info_redraw(const tWindow* window)
{ tWkInfoData* wksd = (tWkInfoData*) (window->wksd);
  tBrowserDocument* document = &(wksd->document);
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "wop_info_redraw(): window=%p\n", window);
  debugmsg(debugstrbuf);
#endif
  document_display(document, wrtRedraw);
}

static tBrowserDocument* wop_info_find_currdoc(const tWindow* window)
{ tWkInfoData* wksd = (tWkInfoData*) (window->wksd);
  tBrowserDocument* document = &(wksd->document);
  return(document);
}

static const char* wop_info_get_info(tWindow* window,
  /*@out@*/ tBoolean* _is_error)
{ const tWkInfoData* wksd = (tWkInfoData*) (window->wksd);
  const char* retval = wksd->message; *_is_error = wksd->message_is_error;
  return(retval);
}

static const char* wop_info_get_menu_entry(tWindow* window)
{ const tWkInfoData* wksd = (const tWkInfoData*) (window->wksd);
  return(wksd->window_title);
}

static const tWindowSpec wk_info_spec =
{ wop_info_create, wop_info_remove, wop_info_redraw, NULL, NULL, NULL,
  wop_info_find_currdoc, wop_info_get_info, wop_info_get_menu_entry,
  WSPEC_CM(NULL) WSPEC_MOUSE(NULL) WSPEC_SESSION(NULL) 'I', 'I', wkInfo,
  wsfNone /* IMPLEMENTME: cm etc.! */
};


/** Information window stuff */

static tWindow* wk_info_create(const char* title,
  tBrowserDocumentDisplayMode bddm, /*@out@*/ tBrowserDocument** _document)
{ tWindow* retval = window_create(&wk_info_spec);
  tWkInfoData* wksd = (tWkInfoData*) (retval->wksd);
  tBrowserDocument* document = &(wksd->document);
  wksd->window_title = title; document->bddm = bddm;
  if (bddm == bddmHtml) cantent_collect_title(document->cantent, title);
  if (_document != NULL) *_document = document;
  return(retval);
}

static void wk_info_collect(tWindow* window, const char* str)
{ tWkInfoData* wksd = (tWkInfoData*) (window->wksd);
  cantent_collect_str(wksd->document.cantent, str);
  if (wksd->is_finalized) window_redraw(window);
}

static void wk_info_finalize(tWindow* window)
/* The ("initial amount" of) information which should be displayed in this
   window has been collected, now it is displayed for the first time. */
{ tWkInfoData* wksd = (tWkInfoData*) (window->wksd);
  wksd->is_finalized = truE; window_redraw_all();
}

static void wk_info_set_message(tWindow* window, const char* msg,
  tBoolean is_error)
{ tWkInfoData* wksd = (tWkInfoData*) (window->wksd);
  wksd->message = msg; wksd->message_is_error = is_error;
}

#endif /* #if DO_WK_INFO */


/*** Window kind: custom connection window */

#if DO_WK_CUSTOM_CONN

#endif /* #if DO_WK_CUSTOM_CONN */


/*** Window kind: built-in text editor */

#if DO_WK_EDITOR
#endif /* #if DO_WK_EDITOR */
