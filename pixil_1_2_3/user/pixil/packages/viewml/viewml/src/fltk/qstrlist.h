#ifndef QSTRLIST_H
#define QSTRLIST_H

#include "qstring.h"
#include "qarray.h"
#include "qcollection.h"

#include <vector>

class QStrList : public vector<char *>, public QCollection
{
 protected:
  int m_nCurrent;
  bool m_bDeepCopies;
 public:
  QStrList(bool deepCopies=true) { m_nCurrent=0; m_bDeepCopies = deepCopies; }
  virtual ~QStrList() { }
  void append(const char * t) { push_back((char *)t); }
  int count() const { return size(); }
  bool isEmpty() const { return empty(); }

  char * next() { 
    if(m_nCurrent<(size()-1))
      return (*this)[++m_nCurrent];
    else
      return NULL;
  }

  int find(const char * c) 
    { 
      int i;
      for(i=0; i<size(); i++) {
	if(!strcmp((*this)[i],c)) {
	  m_nCurrent = i;	  
	  return i;
	}
      }  
      return -1;
    }

  char *  at(int & i) { 
    if(i<(size()-1)) {
      m_nCurrent = i;
      return (*this)[i];
    } else {
      return NULL;
    }
  }

  int size() const { return (int)vector<char*>::size(); }

  char * getLast() const {
    if(size())
      return (*this)[size()-1];
    else
      return NULL;
  }

  char * getFirst() const {
    if(size())
      return (*this)[0];
    else
      return NULL;
  }

  char * first() {
    if(size()) {
      m_nCurrent = 0;;
      return (*this)[0];
    } else {
      return NULL;
    }
  }

};

#endif
