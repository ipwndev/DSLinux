#include <kapp.h>
#include "html.h"
#include <Fl.H>
#ifdef _NANOX
#include <n_x.h>
#else
#include <x.H>
#endif
#include <fl_draw.H>
#include <Fl_Window.H>
#include <Fl_Button.H>
#include <Fl_Pixmap.H>
#include <Fl_Input.H>
#ifdef USING_FLEK_LIB
#include <Fl_Animator.H>
#include <Flve_Combo.H>
#endif
#include <Fl_Pack.H>
//#include <Fl_Scroll.H>

#include "fltk/nxscroll.h"

#include <Fl_Image.H>
#include "http.h"
#include "vmlapp.h"
#include "history.h"
#include "fltk/qconst.h"
#include "fltk/qlist.h"
#include "pixmaps.h"
#include "bookmark.h"
#include "bookmark.xpm"
#include "viewml.xpm"

static Fl_Pixmap	pixmap_bm(bookmark_xpm);

// CRH
#include <unistd.h>
#include <sys/stat.h>

// Define which box type you wish for the buttons at the top
// #define VIEWML_BUTTON_STYLE FL_UP_BOX
#define VIEWML_BUTTON_STYLE FL_FLAT_BOX
#define VIEWML_MAX_HISTORY		10
#define	VIEWML_MAX_BOOKMARKS	10


// Internal forward/back history list
QList<QString> g_History;
int g_HistoryIndex=-1;

// Initial x/y postion on screen
int g_AppX = 0;
int g_AppY = 0;

char g_StartURL[256];

// Initial app width/height
// CRH int g_AppWidth = 600;
// CRH int g_AppHeight = 400;
int g_AppWidth = 640;
int g_AppHeight = 480;

#ifdef USING_FLEK_LIB
Flve_Combo	*g_URL;
Fl_Animator	*g_Logo = 0;
#else
Fl_Input * g_URL;
Fl_Button * g_Logo;
#endif

static char				*g_BMFile = NULL;							// Class default, unless specified

extern Fl_Button *Forward;
extern Fl_Button *Backward;
extern Fl_Button *Stop;
extern Fl_Button *Reload;

static char oldstr[256] = "";
static int hindex = 0;

int g_Vert = 0;
#ifdef USING_FLEK_LIB
Flve_Combo	*g_Input;
#else
Fl_Input * g_Input;
#endif

Fl_Window * g_HTMLWindow;
VMLAppWindow * g_AppWindow = 0;

void LoadPage(const char * URL);
VMLAppWindow * make_window();

KApplication g_KApp;

#define HTML_WIDGET KHTMLView
#define xHTML_WIDGET KHTMLWidget

void LoadDoc(const char * URL, int w, int h);
#ifdef USING_FLEK_LIB
void UpdateHistoryBox(const char *URL);
#endif

HTML_WIDGET * g_HTML;
HTTPConnection * g_Conn;

BookMark		*g_BookMark;										// Global bookmark entity

#ifdef _NANOX

// the below is needed to fix some X compilation problems with Nano-X
unsigned long KeyPress;
bool XCheckTypedEvent( Display *, unsigned long , 
		       XEvent * )
{
	return false;
}
#endif


int main(int argc, char ** argv)
{
  // CRH  int index,loop;
  int c;
  extern int  optind;
  extern char *optarg;
  // end CRH
  
  VMLAppWindow * mw;
  
  // recover any command-line parameters
  g_StartURL[0] = 0x00;
  
  // CRH (with a minor addition by jsk for -u backward-compatibility)
  while ((c = getopt(argc, argv, "b:h:u:w:x:y:?")) != EOF)
    {
      switch (c)
	{
	case 'b':
		if (optarg)
			g_BMFile = optarg;
		break;
	case 'h':
	  g_AppHeight = atoi(optarg);
	  break;
	case 'u':
	  strcpy(g_StartURL, optarg);
	  break;
	case 'w':
	  g_AppWidth = atoi(optarg);
	  break;
	case 'x':
	  g_AppX = atoi(optarg);
	  break;
	case 'y':
	  g_AppY = atoi(optarg);
	  break;
	default:
	  printf("%s [opts] [URL or filename]\n", argv[0]);
	  printf("\t -u URL (deprecated)\n");
	  printf("\t -h window height\n");
	  printf("\t -w window width\n");
	  printf("\t -x window X co-ord\n");
	  printf("\t -y window Y co-ord\n");
	  exit(1);
	}
    }
  if (optind < argc)
    strcpy(g_StartURL, argv[optind]);
  // end CRH
  
  // fake out the widget - "remove" all parms
  argc = 1;
  
  g_Conn = new HTTPConnection();
  g_Conn->setDocCallback(LoadDoc);

  if (g_BMFile == NULL)
  	g_BookMark = new BookMark(VIEWML_MAX_BOOKMARKS);
  else
	  g_BookMark = new BookMark(VIEWML_MAX_BOOKMARKS, g_BMFile);

  mw = make_window();

  g_Conn->setHTML(g_HTML);

  mw->show(argc,argv);

  g_HTML->setGeometry(SCROLLPAD,TITLEHEIGHT ,WIDTH,HEIGHT);
  g_HTML->Fl_Window::begin();

  g_History.setAutoDelete(1);

// CRH cerr << "Main window is " << g_AppWindow << "(" << fl_xid(g_AppWindow) << ")\n";
// CRH cerr << "HTML Widget is " << g_HTML << "(" << fl_xid(g_HTML) << ")\n";

#ifdef _NANOX

  GrReparentWindow(fl_xid(g_HTML),fl_xid(g_AppWindow),SCROLLPAD,TITLEHEIGHT);

  GR_WINDOW_INFO info;
  GrGetWindowInfo(fl_xid(g_HTML),&info);

#endif

  if(g_StartURL[0] != 0x00)
    LoadPage(g_StartURL);
  
  while(Fl::wait()) {}

  return 0;
}

Fl_Button *Forward=(Fl_Button *)0;

//davet
#define MAX_PROTO_STRLEN 7 // must be set to length of longest protocol
static char *protocols[] = 
{
  "http://",
  "file://",
#ifdef OPTIONAL_URI_PROTOCOLS
  "ftp://",
  "afs://",
  "news:",
  "nntp:",
  "mid:",
  "cid:",
  "mailto:",
  "wais://",
  "prospero://",
  "telnet://"
  "gopher://",
#endif
  NULL
};

void SetInput(const char * URL)
{
	g_Input->value(URL);
//  g_Input->input->value(URL);
//  g_Input->input->set_changed();
//  g_Input->input->mark(256);
}

void _LoadPage(const char * URL)
{
// CRH
struct stat status;
  char buf[1024];
  int pindex;

  // get URL protocol
  strncpy(buf,URL,MAX_PROTO_STRLEN);
  buf[MAX_PROTO_STRLEN] = 0x00;

  // check for protocol in list
  pindex = 0;
  while(protocols[pindex] != NULL)
  {
    if(strncmp(buf,protocols[pindex],strlen(protocols[pindex])) == 0)
      break;
    ++pindex;
  }

  // check for missing protocol
  if(protocols[pindex] == NULL)
// CRH
    if(!stat(URL, &status)
       && (S_ISREG(status.st_mode) || S_ISDIR(status.st_mode)))
      if(URL[0] == '/')
	sprintf(buf,"file://%s",URL);
      else
	sprintf(buf,"file://%s/%s", getenv("PWD"), URL);
    else
// end
      sprintf(buf,"http://%s",URL);
  else
    strcpy(buf,URL);

#ifdef USING_FLEK_LIB
  UpdateHistoryBox(buf);
#endif
  SetInput(buf);

  g_Conn->loadPage(g_HTML,buf);
}

void AddHistory(const char * URL)
{
  if(g_HistoryIndex != -1 && g_HistoryIndex != (int)g_History.count()-1) {
    while(g_HistoryIndex != (int)g_History.count()-1)
      g_History.removeLast();
  }

  g_History.append(new QString(URL));
  g_HistoryIndex = g_History.count() - 1;
}

void LoadPage(const char * URL)
{
  AddHistory(URL);
  _LoadPage(URL);

}

void LoadDoc(const char * URL, int w, int h) 
{
  if(URL) {
    AddHistory(URL);
    SetInput(URL);
  }
  return;
}

void AddBookmark_cb(Fl_Widget *w, void *data)
{
	char					*cpTitle,								// Title
							*cpURL;									// URL

	cpTitle = (char *)g_HTML->getKHTMLWidget()->getTitle()->c_str();
	cpURL = (char *)g_HTML->getCurURL()->c_str();
	g_BookMark->AddBookmark(cpTitle, cpURL);
	return;
} // end of AddBookmark_cb

void DelBookmark_cb(Fl_Widget *w, void *data)
{
	int					idx = (int) data;							// Index of selected menu item

	g_BookMark->DelBookmark(idx);
	return;
} // end of DelBookmark_cb

void SelBookmark_cb(Fl_Widget *w, void *data)
{
	int					idx = (int)data;							// Index of selected menu item

	LoadPage(g_BookMark->GetURL(idx));
	return;
} // end of SelBookmark_cb

void Reload_Callback(Fl_Widget * w, void * data)
{
  _LoadPage(g_Input->value());
}

void Forward_Callback(Fl_Widget * w, void * data)
{
  if(g_HistoryIndex == -1 || g_HistoryIndex+1 > (int)g_History.count()-1)
    return;

  _LoadPage(*g_History.at(++g_HistoryIndex));
}

void Backward_Callback(Fl_Widget * w, void * data)
{
  if(g_HistoryIndex == -1 || g_HistoryIndex == 0)
    return;

  _LoadPage(*g_History.at(--g_HistoryIndex));
}

#ifdef USING_FLEK_LIB
void Combo_Callback(Fl_Widget *w, void *data)
{
	const char					*newURL;
	Flve_Combo					*o = (Flve_Combo *)w;

	// Get the text from the input widget
	newURL = o->input->value();
	LoadPage(newURL);
	
	return;
} // end of Combo_Callback()
#endif

void Input_Callback(Fl_Widget * w, void * data)
{
  Fl_Input * i = (Fl_Input*)w;
  QString str  =  i->value();
  History hobj;

  // if old and new strings match, must have been a CR
  if(strcmp((char *)str,oldstr) == 0)
  {
    oldstr[0] = 0x00;
    hobj.AddToHistoryList(str);
    hindex = 1;

    LoadPage(str);
  }

  // if there's a space in str, set a history entry
  else if(strstr((char *)str," "))
  {
    i->value(hobj.GetHistoryEntry(hindex));
    i->set_changed();
    i->mark(256);
    strcpy(oldstr,hobj.GetHistoryEntry(hindex));
    if(hobj.GetHistoryEntry(++hindex) == NULL)
      hindex = 0;
  }

  // must be an updated string - save it
  else
  {
    strcpy(oldstr,(char *)str);
  }
}


#ifdef USING_FLEK_LIB
void
UpdateHistoryBox(const char *str)
{
	int					rc;											// Return code
	if (!(g_Input->item.count()))
	{
		g_Input->item.insert(0, str);
	} // end of if
	else if ( (rc = g_Input->item.find(str)) != -1)
	{
		// This is already in the list, save it, remove it from the list,
		// and re-insert it at the top
		g_Input->item.remove(rc);
		g_Input->item.insert(0, str);
	} // end of else-if
	else
	{
		// This is a new one, remove the VIEWML_MAX_HISTORY'th item
		// and insert this one at the top
		if (g_Input->item.count() == VIEWML_MAX_HISTORY)
		{
			g_Input->item.remove(VIEWML_MAX_HISTORY ? VIEWML_MAX_HISTORY - 1 : 0);
		} // end of if 
		g_Input->item.insert(0, str);
	} // end of else

	return;
} // end of UpdateHistoryBox()
#endif


VMLAppWindow * make_window() 
{
  VMLAppWindow * w;
  { 
    VMLAppWindow * o = new VMLAppWindow(g_AppX,
					g_AppY,
					APPWIDTH,
					APPHEIGHT);
    g_AppWindow = o;
	o->label("ViewML");

    w = o;
    { 
      Fl_Button* o = Forward = new Fl_Button(42, 3, 25, 25);
      o->box(VIEWML_BUTTON_STYLE);
#ifdef _NANOX
      o->down_box(FL_WHITE_BOX);
#endif
      o->callback(Forward_Callback,0);
      o->when(FL_WHEN_RELEASE);
      pixmap_forward.label(o);
    }
    { 
      Fl_Button* o = Backward = new Fl_Button(22, 3, 25, 25);
      o->box(VIEWML_BUTTON_STYLE);
#ifdef _NANOX
      o->down_box(FL_WHITE_BOX);
#endif
      o->callback(Backward_Callback,0);
      o->when(FL_WHEN_RELEASE);
      pixmap_back.label(o);
    }

    { 
      Fl_Button* o = Reload = new Fl_Button(62, 3, 25, 25);
      o->box(VIEWML_BUTTON_STYLE);
#ifdef _NANOX
      o->down_box(FL_WHITE_BOX);
#endif
      o->callback(Reload_Callback,0);
      o->when(FL_WHEN_RELEASE);
      pixmap_reload.label(o);
    }
	{
		Fl_Menu_Button *o = new Fl_Menu_Button(2, 3, 25, 25);
		o->box(VIEWML_BUTTON_STYLE);
#ifdef _NANOX
		o->down_box(FL_WHITE_BOX);
#endif
//		o->textsize(8);
		pixmap_bm.label(o);

		// Save this into the bookmark class
		g_BookMark->SetMenuWidget(o);
	}
    
    {
#ifdef USING_FLEK_LIB
		g_Logo = new Fl_Animator(viewml_xpm, APPWIDTH - 24, 5, 10, 
									20, 20, 500, "Foobar");
#else
		Fl_Button* o = g_Logo = new Fl_Button(APPWIDTH -24 ,5,20,20);
		o->box(FL_OFLAT_BOX);
		o->down_box(FL_OFLAT_BOX);
		pixmap_vml.label(o);
#endif // USING_FLEK_LIB
    }
    { 
#ifdef USING_FLEK_LIB
		Flve_Combo	*o;
		o = g_URL = new Flve_Combo(92, 5, APPWIDTH - 135, 20, "");
		g_URL->input->callback(Input_Callback, 0);
		g_URL->input->when(FL_WHEN_ENTER_KEY_CHANGED);
		g_URL->callback(Combo_Callback, 0);
		g_URL->when(FL_WHEN_CHANGED);
		g_URL->list_only(false);
		g_URL->display_rows(VIEWML_MAX_HISTORY);
		g_URL->incremental_search(false);
		g_URL->input->parent(g_URL);
#else
      Fl_Input * o = g_URL = new Fl_Input(92, 5, APPWIDTH - 135 , 20, "");
      o->callback(Input_Callback,0);
      o->when(FL_WHEN_ENTER_KEY_CHANGED);
#endif
      g_Input = o;

#ifdef USING_FLEK_LIB
	  // Open the file and add history items
	  {
		  int			ii,
		  				HCnt;
			for (HCnt = ii = 0; ii < 20 && HCnt < VIEWML_MAX_HISTORY; ii++)
			{
				const char *cp;
				History		hobj;
				cp = hobj.GetHistoryEntry(ii);
				if (cp)
				{
					g_Input->item.add(cp);
					++HCnt;
				} // end of if
			}
	  } // end of memory
#endif

    }

    { 

      HTML_WIDGET * v;

      o->begin();
      o->show();

      // This is an ugly ugly hack .. because of broken parenting
      // in FLNX, we need to make sure the HTML Widget is added to
      // it's parent before it is mapped so the window manager
      // doesn't get ahold of it and do something evil. We do this
      // by passing in our top-level widget in the flags field
      // see htmlview.cpp for the other side of the disgusting piece
      // of code.

      v = new HTML_WIDGET(0,0,(int)o);
      v->setGeometry(SCROLLPAD,TITLEHEIGHT,WIDTH,HEIGHT);
      o->add(v);
      
      g_Conn->Connect(v);
      g_HTML = v;
    }
    g_AppWindow = o;
    o->end();
  }


  // recover last entry from history and display the page
  History hobj;

  cerr << "Main window: " << w << endl;
  return w;
}

