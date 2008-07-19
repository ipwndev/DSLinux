#ifndef __QLIST_H
#define __QLIST_H

#include <vector>
#include "qcollection.h"
using namespace std;
template <class T>
class QList : public vector<T*>, public QCollection
{
 private:
  int m_nCurrent;
 public:
  QList() : QCollection() { m_nCurrent = 0;}
  virtual ~QList() { clear(); }
  void append(const T * t) { push_back((T*)t); }
  unsigned int count() const { return (unsigned int)size(); }
  bool isEmpty() const { return empty(); }

  void clear() {
    while(removeLast());
  }

  T * current() { 
    if(size() && m_nCurrent != -1)
      return (*this)[m_nCurrent];
    else
      return NULL;
  }

  T * prev() { 
    if(m_nCurrent>0) {
      return (*this)[--m_nCurrent];
    } else {
      m_nCurrent=-1;
      return NULL;
    }
  }

  T * next() { 
    if(m_nCurrent<(size()-1)) {
      return (*this)[++m_nCurrent];
    } else {
      m_nCurrent=-1;
      return NULL;
    }
  }
  
  int at() const {
    if(size() && m_nCurrent != -1)
      return m_nCurrent;
    else
      return -1;
  }

  T * at(const int & i) { 
    if(i<(size())) {
      m_nCurrent = i;
      return (*this)[i];
    } else {
      return NULL;
    }
  }

  bool removeLast() {
    if(size()) {
      if(m_bAutoDelete)
	delete getLast();
      pop_back();
      return true;
    } else {
      return false;
    }
  }

  T* take(unsigned int index) {
    if(!size())
      return NULL;

    T* tmp = (*this)[0];
    
    erase(begin());

    if(!size())
      m_nCurrent = -1;

    if(m_nCurrent > size())
      m_nCurrent = size();
    
    return tmp;
  }


  T* getFirst() const {
    if(size())
      return (*this)[0];
    else
      return NULL;
  }

  T* getLast() const {
    if(size())
      return (*this)[size()-1];
    else
      return NULL;
  }

  T* first() {
    if(size()) {
      m_nCurrent = 0;;
      return (*this)[0];
    } else {
      return NULL;
    }
  }

  T* last() { 
    if(size()) {
      m_nCurrent = size()-1;
      return (*this)[size()-1];
    } else {
      return NULL;
    }
  }

  bool remove(const T* item)
    {
      return removeRef(item);
    }

  int size() const { return (int)vector<T*>::size(); }

  int findRef(const T* t) 
    { 
      int i;
      for(i=0; i<size(); i++) {
	if((*this)[i] == t) {
	  m_nCurrent = i;	  
	  return i;
	}
      }  
      return -1;
    }

  bool removeRef(const T* item)
    {
      typename vector<T*>::iterator pos = find(begin(),end(),item);
      if(pos != end()) {
	if(m_bAutoDelete)
	  delete *pos;
	erase(pos);
	return true;
      }
      return false;
    }
};

#endif
