#ifndef __HTTP_H
#define __HTTP_H

#include "qobject.h"
#include "qstring.h"
#include <queue>
#include <pthread.h>

struct _HTRequest;
struct _HTList;
struct _HTChunk;

typedef _HTRequest HTRequest;
typedef _HTList HTList;
typedef _HTChunk HTChunk;

#define HTML_WIDGET KHTMLView
#define xHTML_WIDGET KHTMLWidget

class HTML_WIDGET;

class HTTPConnection : public QObject
{
  Q_OBJECT;
protected:
  HTRequest * request;
  HTList * converters;
  HTList * encodings;

  bool m_bTossCurrent;

  pthread_mutex_t m_ListLock;
public:

  bool m_bDocDone;

  class HTTPRequest
    {
    public:
      enum Method { GET,POST };
    protected:
      QString m_strURL;
      QString m_strData;
      unsigned char * m_Buffer;
      int m_nSize;
      bool m_bNewPage;
      bool m_bToss;
      Method m_Method;
      HTML_WIDGET * m_View;
      
    public:

      void * _request;

      HTTPRequest(HTML_WIDGET * p, const QString & url, Method m = GET, 
		  bool bNewPage=false, const char * data=0);
      ~HTTPRequest();

      Method getMethod() { return m_Method; }

      HTML_WIDGET * HTML() { return m_View; }
      const char * getData() const { return m_strData; }
      const char * getURL() const { return m_strURL; }
      unsigned char * getBuffer() { return m_Buffer; }
      bool newPage() { return m_bNewPage; }
      int getSize() { return m_nSize; }
      void addBuffer(unsigned char * buf, int len);
      int toss() { return m_bToss; }
      void toss(bool bToss) { m_bToss = bToss; }
    };

  HTTPRequest * m_pCurrentRequest;

  void setCurrent(HTTPRequest * p);
  void tossCurrent(HTML_WIDGET * p);

  HTTPConnection();
  ~HTTPConnection();
  int Connect(QObject * p);

  void loadPage(HTML_WIDGET * p, const char * URL,
		HTTPRequest::Method method=HTTPRequest::GET, const char * data=0);
  void setHTML(HTML_WIDGET * p);
  HTML_WIDGET * HTML() const { return m_pHTML; }

 protected:
  queue<HTTPRequest *> m_Reqs;
  queue<HTTPRequest *> m_Finished;
  pthread_t m_Thread;
  bool m_bThreadRunning;
  HTML_WIDGET * m_pHTML;

  void StartThread();
  void StopThread();
  void BeginDataTimer();
  static void * runthread(void* p);

  void (*m_pDocCallback)(const char *,int,int);

 public:

  void do_size();

  void EmptyQueues(HTML_WIDGET * p);

  void AddReq(HTTPRequest * r);
  void AddReq(HTML_WIDGET * p, const char * url, 
	      HTTPRequest::Method method = HTTPRequest::GET, 
	      bool bNewPage=false, const char * data=0);

  HTTPRequest * GetReq();
  int ReqSize();

  void AddFinished(HTTPRequest * p);
  HTTPRequest * GetFinished();
  int FinishedSize();

  int getURL(HTTPRequest * r);
  int postURL(HTTPRequest * r);

  void setDocCallback(void(*p)(const char *,int,int)) { m_pDocCallback = p; }
  
 protected slots:
  void slotDocumentRequest(KHTMLView * v, const char * url);
  void slotDocumentStarted(KHTMLView * v);
  void slotOpenURL(KHTMLView * v,const char * URL);
  void slotDocDone();
  void slotClick(KHTMLView * p, const char * d1, int b, const char * p);
  void slotForm(KHTMLView *,const char *,const char *, const char *);
    
};

#endif
