#ifndef __QARRAY_H
#define __QARRAY_H

#include <sys/types.h>

template <class T>
class QArray 
{
 protected:
//  T m_Temp;
  unsigned char * m_pBuffer;
  int m_nSize ;

 public:
  QArray() { m_pBuffer =0; m_nSize =0;  }
  QArray(int sz) { m_pBuffer =0; m_nSize =0; resize(sz); }

  unsigned int size() { return (unsigned int)m_nSize; }

  bool fill(const T & v, int newsize=-1)
  {
    unsigned int index = 0;
    if(newsize > 0)
      resize(newsize);

    while(size() > index)
      (*this)[index++] = v;
    return true;
  }

  QArray<T> copy() const { return *this; }

  void resize(int size)
    {
      unsigned char * b;
      if(m_nSize < size) {
	b = new unsigned char[size * sizeof(T)];
	memcpy(b, m_pBuffer, m_nSize * sizeof(T));
	delete [] m_pBuffer;
	m_pBuffer = b;
	m_nSize = size;
      } 
    }

  T  & operator[](int index)
    { 
      if(index >= (int)size())
	resize(index + 10);
      return (T&)*(T*)(m_pBuffer + (sizeof(T) * index));
    }
};

#endif
