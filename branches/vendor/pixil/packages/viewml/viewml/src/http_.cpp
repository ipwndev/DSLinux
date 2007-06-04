#include "http.h"
#include <Fl.H>
#include "html.h"

////////////////////////////////////////////////////////////

static void _CheckData(void * pt)
{
  HTTPConnection * pThis = (HTTPConnection*)pt;
  HTTPConnection::HTTPRequest * p = pThis->GetFinished();

  while(p) {
    if(!p->toss()) {
      if(p->newPage()) {

		  // For some reason, not all of the items on the mapPendingFiles queue
		  // are cleared when a new page is requested.  On a new page, all items are
		  // cleared
		  p->HTML()->cancelAllRequests();
		  p->HTML()->getKHTMLWidget()->ClrTitle();
	p->HTML()->begin(p->getURL());
	
	p->HTML()->write((const char*)p->getBuffer());
	
	p->HTML()->end();
	p->HTML()->parse();
	
	p->HTML()->redraw();
	
      } else {

	p->HTML()->data(p->getURL(), (const char *)p->getBuffer(), 
			p->getSize(), true);
      }
    }
    delete p;
    p = pThis->GetFinished();
  }
  
  if(!pThis->m_bDocDone) {
    Fl::add_timeout(.5, _CheckData, pt);
  }
}

void _HaveData(HTTPConnection::HTTPRequest * p)
{
  p->HTML()->data(p->getURL(), 0,0, true);
}

void HTTPConnection::slotClick(KHTMLView * v, const char * d1, int b, const char * p)
{

  if(p && p[0]) {
    KHTMLView * v1 = v->findView((const char *)p);
    if(v1)
      v = v1;
  }

  loadPage(v,d1);

  if(m_pDocCallback)
    m_pDocCallback(d1,0,0);

  m_bDocDone = false;
  StartThread();
}

void HTTPConnection::BeginDataTimer()
{
  Fl::add_timeout(.5, _CheckData, (void*)this);
}

void HTTPConnection::do_size()
{
  //  printf("docSize: x=%d y=%d\n",m_pHTML->docWidth(),
  //	 m_pHTML->docHeight());

  //  if(m_pDocCallback)
  //    m_pDocCallback(0,m_pHTML->docWidth(), m_pHTML->docHeight());
}

////////////////////////////////////////////////////////////

