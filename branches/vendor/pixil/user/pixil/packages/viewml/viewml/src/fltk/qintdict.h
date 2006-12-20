#ifndef __QINTDICT_H
#define __QINTDICT_H

#include "fltk-qbase.h"
#include "qcollection.h"
#include <map>

template <class T>
class QIntDict  : public QCollection
{
 protected:
  map<unsigned int,T*> m_Map;
 public:
  QDict(int size=17, bool cs=true, bool ck = true) :
    QCollection { }
  ~QDict() { clear(); }
  void setAutoDelete(bool enable) { };
  int count() const { return m_Map.size(); }

  bool isEmpty() const { return m_Map.empty(); }

  T* operator[](unsigned int key) { return m_Map[key]; }

  virtual void clear() 
    { 
      map<string,T*>::iterator pos;
      if(m_bAutoDelete) {
	for(pos=m_Map.begin(); pos != m_Map.end(); ++pos)
	  delete pos->second;
      }
      m_Map.clear(); 
    }

  T* find(unsigned int key)
    {
      map<unsigned int,T*>::iterator pos;

      pos = m_Map.find(key);
      if(pos != m_Map.end())
	return pos->second;
      else
	return 0;
    }

  bool remove(unsigned int key)
    {
      map<unsigned int,T*>::iterator pos;

      pos = m_Map.find(key);
      if(pos != m_Map.end()) {
	if(m_bAutoDelete)
	  delete pos->second;
	m_Map.erase(pos);
	return TRUE;
      }

      return FALSE;
    }

  void insert(unsigned int key, const T * item)
    {
      m_Map[key] = (T*)item;
    }

  map<unsigned int,T*>::iterator begin() { return m_Map.begin(); }

};
template <class T>
class QIntDictIterator 
{
 protected:
  map<unsigned int,T*>::iterator m_Pos;
 public:
  QIntDictIterator(QIntDict<T> & t) { m_Pos = t.begin(); }
  T * current() const { m_Pos->second; }
  QIntDictIterator & operator++() { m_Pos++; return *this; }
  unsigned int currentKey() const { return m_Pos->first; }
};

#endif
