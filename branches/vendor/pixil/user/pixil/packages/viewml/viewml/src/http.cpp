#include "http.h"

//#ifndef HTPROXY_H
//#define HTPROXY_H
//#endif


#define ERRNO_DONE
#include "WWWLib.h"
#include "WWWHTTP.h"
#include "WWWInit.h"
#include "HTProxy.h" 
#include "vmlapp.h"

#ifdef USING_FLEK_LIB
#include <Fl_Animator.H>
extern Fl_Animator *g_Logo;
#endif


int printer (const char * fmt, va_list pArgs)
{
  return(0);
    return (vfprintf(stdout, fmt, pArgs));
}

int tracer (const char * fmt, va_list pArgs)
{
  return(0);
    return (vfprintf(stderr, fmt, pArgs));
}

void HTTPConnection::setHTML(HTML_WIDGET * p)
{
  m_pHTML = p;
}

HTTPConnection::HTTPRequest::~HTTPRequest()
{
  delete [] m_Buffer;
}

HTTPConnection::HTTPRequest::HTTPRequest(HTML_WIDGET * p, 
					 const QString & url, 
					 Method m, bool bNewPage, const char * data) 
{
  _request = 0;
  m_bToss = false;
  m_strURL = url; 
  m_bNewPage = bNewPage; 
  m_nSize = 0; 
  m_Buffer = 0; 
  m_Method = m;
  m_View = p;

  if(data)
    m_strData = data;

}


void HTTPConnection::HTTPRequest::addBuffer(unsigned char * buf, int len)
{
  if(!m_Buffer)
    m_Buffer = (unsigned char *)malloc(len + 1);
  else
  {
    unsigned char *p = (unsigned char *)realloc(m_Buffer,m_nSize + len + 1);
    m_Buffer = p;
  }
  memcpy(m_Buffer + m_nSize,buf,len);
  *(m_Buffer + m_nSize + len) = 0;
  m_nSize += len;

#ifdef NEVER
  if(!m_Buffer) {
    m_Buffer = new unsigned char[len];
  } else {
    unsigned char * p  = new unsigned char[m_nSize + len];
    memcpy(p,m_Buffer,m_nSize);
    delete [] m_Buffer;
    m_Buffer = p;
  }
  memcpy(m_Buffer + m_nSize,buf,len);
  m_nSize += len;
#endif
}

HTTPConnection::HTTPConnection() 
{ 
  pthread_mutex_init(&m_ListLock, 0);

  m_bThreadRunning = false;
  m_pDocCallback = 0;

  HTProfile_newPreemptiveClient("ViewML", "0.14");
  
  HTPrint_setCallback(printer);
  HTTrace_setCallback(tracer);

#if 0  
  HTSetTraceMessageMask("*");
#endif

  char * pProxy = getenv("VIEWML_PROXY");

  if(pProxy && pProxy[0])
    HTProxy_add("http",pProxy);
  
  /* Register the default set of MIME header parsers */
  HTMIMEInit();
  
}

HTTPConnection::~HTTPConnection()
{
  HTFormat_deleteAll();
  HTLibTerminate();  
  pthread_mutex_destroy(&m_ListLock);
}

int HTTPConnection::Connect(QObject * p)
{
  connect(p,SIGNAL(documentStarted(KHTMLView *)),
	  SLOT(slotDocumentStarted(KHTMLView *)));
  connect(p,SIGNAL(documentRequest(KHTMLView *, const char*)),
	  SLOT(slotDocumentRequest(KHTMLView *, const char*)));
  connect(p,SIGNAL(imageRequest(KHTMLView *, const char*)),
	  SLOT(slotOpenURL(KHTMLView *, const char*)));
  connect(p,SIGNAL(URLSelected(KHTMLView*,const char*,int,const char*)), 
	  SLOT(slotClick(KHTMLView*,const char*,int,const char*)));
  // Changed documentDone() to be documentDone(KHTMLView *) --- jmw
  connect(p,SIGNAL(documentDone(KHTMLView *)), SLOT(slotDocDone()));
  connect(p,SIGNAL(documentDone()), SLOT(slotDocDone()));
  connect(p,SIGNAL(formSubmitted(KHTMLView *,const char *, const char *, const char*)),
	  SLOT(slotForm(KHTMLView *,const char *, const char *, const char*)));

  return 0;
}

void HTTPConnection::slotDocumentStarted(KHTMLView * v)
{
  //Connect((QObject*)v);
}

void HTTPConnection::slotDocumentRequest(KHTMLView * v, const char * url)
{
  m_bDocDone = false;

#ifdef USING_FLEK_LIB
  if (g_Logo)
	  g_Logo->start_animation();
#endif

  AddReq(v,url,HTTPRequest::GET,true);
  StartThread();
}

void HTTPConnection::slotForm(KHTMLView * v,const char * url, 
			      const char * method, const char * data)
{
  QString tmp;
  tmp = url;

  if(!strcmp(method,"GET")) {
    tmp += '?';
    tmp += data;
    loadPage(v,tmp,HTTPRequest::GET,data);
  }
 else {
    loadPage(v,tmp,HTTPRequest::POST,data);
  }

  if(m_pDocCallback)
    m_pDocCallback(tmp,0,0);

  m_bDocDone = false;
  StartThread();
}



void HTTPConnection::slotDocDone()
{
  m_bDocDone = true;

#ifdef USING_FLEK_LIB
  if (g_Logo)
  {
	  g_Logo->stop_animation();
	  g_Logo->frame(0);
	  g_Logo->redraw();
  } // end of if
#endif

  do_size();

}

void HTTPConnection::slotOpenURL(KHTMLView * v, const char * URL)
{
  m_bDocDone = false;

  AddReq(v,URL);
  StartThread();
}

void HTTPConnection::StartThread()
{
  if(m_bThreadRunning)
    return;
  
  m_bThreadRunning = true;
  pthread_create(&m_Thread,NULL,runthread,this);

  BeginDataTimer();
}

void HTTPConnection::loadPage(HTML_WIDGET * p, const char * URL,
			      HTTPRequest::Method method, const char * data)
{
  m_bDocDone = false;

  EmptyQueues(p);

#ifdef USING_FLEK_LIB
  if (g_Logo)
	  g_Logo->start_animation();
#endif

  AddReq(p,URL,method,true,data);
  StartThread();
}

void HTTPConnection::setCurrent(HTTPRequest * p)
{
  pthread_mutex_lock(&m_ListLock);
  m_pCurrentRequest = p;
  pthread_mutex_unlock(&m_ListLock);
}  

void HTTPConnection::tossCurrent(HTML_WIDGET * p)
{
  pthread_mutex_lock(&m_ListLock);
  if(m_pCurrentRequest) {
    if(m_pCurrentRequest->HTML() == p)
      m_pCurrentRequest->toss(true);
    //    if(m_pCurrentRequest->_request)
    //      HTHost_killPipe(HTNet_host(HTRequest_net(m_pCurrentRequest->_request)));
  }
  pthread_mutex_unlock(&m_ListLock);
}  

void * HTTPConnection::runthread(void * p)
{
  HTTPConnection * pThis = (HTTPConnection*)p;
  HTTPRequest * r = 0;

  r = pThis->GetReq();
  pThis->setCurrent(r);
  while(r) {
    pThis->getURL(r);
    pThis->AddFinished(r);
    r = pThis->GetReq();
    pThis->setCurrent(r);
  }

  pThis->setCurrent(0);

  pthread_detach(pThis->m_Thread);
  pThis->m_bThreadRunning = false;

  return 0;
}

void HTTPConnection::StopThread()
{

}

int HTTPConnection::postURL(HTTPRequest * r)
{
  char * c = (char*)alloca(strlen(r->getData())+1);
  char * c_ptr = c;
  HTChunk * chunk = NULL;
  HTAnchor * anchor = NULL;  
  HTAssocList * formfields = NULL;

  strcpy(c,r->getData());

  while(*c && (c_ptr = strchr(c,'&'))) {

    *c_ptr = 0;
    /* Create a list to hold the form arguments */
    if (!formfields) formfields = HTAssocList_new();

    /* Parse the content and add it to the association list */
    HTParseFormInput(formfields, c);
    
    c = c_ptr+1;
  }

  /* Add the last field */
  if (c) { 
    if (!formfields) formfields = HTAssocList_new();
    HTParseFormInput(formfields, c);
  }

  /* Create a request */
  request = HTRequest_new();
  
  /* Set the default output to "asis" */
  HTRequest_setOutputFormat(request, WWW_SOURCE);
  HTRequest_setPreemptive(request, YES);
  
  /* Get an anchor object for the URI */
  anchor = HTAnchor_findAddress(r->getURL());

  r->_request = request;

  /* Post the data and get the result in a chunk */
  chunk = HTPostFormAnchorToChunk(formfields, anchor, request);

  /* If chunk != NULL then we have the data */
  if (chunk) {
    int sz = HTChunk_size(chunk);
    r->addBuffer((unsigned char*)HTChunk_data(chunk),sz);
    HTChunk_delete(chunk);
  }

  /* Clean up the form fields */
  HTAssocList_delete(formfields);
  HTRequest_delete(request);

  return 0;
}

int  HTTPConnection::getURL(HTTPRequest * r)
{
  const char * URL = r->getURL();

  if(r->getMethod() == HTTPRequest::POST)
    return postURL(r);

  /* Moved these below the above line to avoid creating a */
  /* new request that never gets used */
  
  request = HTRequest_new();
  HTChunk * chunk;

  HTRequest_setOutputFormat(request, WWW_SOURCE);
  HTRequest_setPreemptive(request, YES);

  if (URL) {
    char * cwd = HTGetCurrentDirectoryURL();
    char * absolute_url = HTParse(URL, cwd, PARSE_ALL);

    r->_request = request;

    chunk = HTLoadToChunk(absolute_url, request);

    HT_FREE(absolute_url);
    HT_FREE(cwd);

    /* If chunk != NULL then we have the data */
    if (chunk) {
      int sz = HTChunk_size(chunk);
      r->addBuffer((unsigned char*)HTChunk_data(chunk),sz);
      HTChunk_delete(chunk);
    }
 } 
  
  HTRequest_delete(request);
  
  return 0;
}

void HTTPConnection::AddReq(HTTPRequest * r)
{
  pthread_mutex_lock(&m_ListLock);
  m_Reqs.push(r);
  pthread_mutex_unlock(&m_ListLock);  
}


void HTTPConnection::AddReq(HTML_WIDGET * p, const char * url, 
			    HTTPRequest::Method method, bool bNewPage,
			    const char * data)
{

  HTTPRequest * r = new HTTPRequest(p,url,method,bNewPage,data);
  pthread_mutex_lock(&m_ListLock);

  m_Reqs.push(r);
  pthread_mutex_unlock(&m_ListLock);
}

void _HaveData(HTTPConnection::HTTPRequest * p);


void HTTPConnection::EmptyQueues(HTML_WIDGET * p)
{
  HTTPRequest * r,* pAddFirst=0, *pFinishFirst=0;

  tossCurrent(p);

  while((r = GetReq()) && r != pAddFirst) 
    { 
      if(r->HTML() != p) {
	AddReq(r); 
	if(!pAddFirst)
	  pAddFirst = r;
      } else {
	_HaveData(r);
	delete r;
      }
    }
  
  while((r = GetFinished()) && r != pFinishFirst) 
    { 
      if(r->HTML() != p) { 
	AddFinished(r);       
	if(!pFinishFirst)
	  pFinishFirst = r;
      } else {
	_HaveData(r);
	delete r;
      }
    }
}

HTTPConnection::HTTPRequest * HTTPConnection::GetReq()
{
  HTTPRequest * r = 0;
  pthread_mutex_lock(&m_ListLock);
  if(m_Reqs.size()) {
    r = m_Reqs.front();

    m_Reqs.pop();

  }

  if(m_Reqs.size()) {
    StopThread();
  }

  pthread_mutex_unlock(&m_ListLock);
  return r;
}

int HTTPConnection::ReqSize()
{
 pthread_mutex_lock(&m_ListLock);
 int r = m_Reqs.size();
 pthread_mutex_unlock(&m_ListLock);
 return r;

}

void HTTPConnection::AddFinished(HTTPRequest * r)
{
  pthread_mutex_lock(&m_ListLock);
  m_Finished.push(r);
  pthread_mutex_unlock(&m_ListLock);
}

HTTPConnection::HTTPRequest * HTTPConnection::GetFinished()
{
  HTTPRequest * r = 0;
  pthread_mutex_lock(&m_ListLock);
  if(m_Finished.size()) {
    r = m_Finished.front();
    m_Finished.pop();
  }
  pthread_mutex_unlock(&m_ListLock);
  return r;
}

int HTTPConnection::FinishedSize()
{
 pthread_mutex_lock(&m_ListLock);
 int r = m_Finished.size();
 pthread_mutex_unlock(&m_ListLock);
 return r;

}

#include "http.moc"


