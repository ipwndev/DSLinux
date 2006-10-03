#ifndef __QSTACK_H
#define __QSTACK_H

#include <stack>
#include <vector>
#include "qcollection.h"

template <class T>
class QStack : public stack<T*, vector<T*> >, public QCollection
{
 public:
  QStack() : QCollection() { }
  virtual ~QStack() { clear(); }
  virtual void clear() 
    { 
      while(remove());
    }
  bool remove() 
    { 
      if(!size())
	return false;
      T * t = top();  
      if(t) {
	if(m_bAutoDelete)
	  delete t;
	pop(); 
	return true; 
      }
      return false;
    }

  T* top()
    {
      if(size())
	return stack<T*,vector<T*> >::top();
      else
	return 0;
    }

  bool isEmpty() const { return empty(); }
  void push(const T* t) { stack<T*, vector<T*> >::push((T*)t); }
  int count() const { return size(); }
};

#endif
