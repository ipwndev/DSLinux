#ifndef __QBUFFER_H
#define __QBUFFER_H

#include "qiodevice.h"

class QByteArray
{
 protected:
  const char * m_Buffer;
  unsigned int m_nSize;
 public:
  QByteArray () { m_Buffer = 0; m_nSize =0 ;}
  ~QByteArray() { delete [] m_Buffer; }
  QByteArray(const QByteArray & a)
    {
      m_Buffer =0; 
      m_nSize=0;
      if(a.buffer() && a.length())
	assign(a.buffer(),a.length());
    }
  QByteArray & assign(const char * a, unsigned int n) 
    { 
      if(m_Buffer)
	delete [] m_Buffer;
      m_Buffer = new char [n];
      memcpy((char*)m_Buffer,a,n);
      m_nSize = n;
      return *this; 
    }
  unsigned int length() const { return m_nSize; }
  const char * buffer() const { return m_Buffer; }
};

class QBuffer : public QIODevice
{
 protected:
  QByteArray m_Buffer;
 public:
  QBuffer() { }
  QBuffer(const QBuffer & b) : m_Buffer(b.m_Buffer) { }
  virtual ~QBuffer() { }
  virtual int readBlock(char * data, unsigned int  maxlen)
    {
      unsigned int len = (m_Buffer.length() > maxlen ? maxlen : m_Buffer.length());
      memcpy(data,m_Buffer.buffer(),len );
      
      return len;
    }
  virtual int writeBlock( const char * data, unsigned int len)
    {
      m_Buffer.assign(data,len);
      return 0;
    }
  QByteArray buffer() const { return m_Buffer; }
};



#endif
