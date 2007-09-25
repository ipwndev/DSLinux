#ifndef __QDICT_H
#define __QDICT_H

#include "fltk-qbase.h"
#include "qcollection.h"
#include <map>

template <class T> 
class QDict : public QCollection
{
 protected:
  map<string,T*> m_Map;
 public:
  QDict(int size=17, bool cs=true, bool ck = true) :
    QCollection() { }
  virtual ~QDict() { clear(); }

  int count() const { return m_Map.size(); }

  bool isEmpty() const { return m_Map.empty(); }

  T* operator[](const QString & key) { return m_Map[key]; }

  virtual void clear() 
    { 
      typename std::map<string,T*>::iterator pos;
      if(m_bAutoDelete) {
	for(pos=m_Map.begin(); pos != m_Map.end(); ++pos)
	  delete pos->second;
      }
      m_Map.clear(); 
    }

  T* find(const QString & key)
    {
      typename std::map<string,T*>::iterator pos;

      pos = m_Map.find(key);
      if(pos != m_Map.end())
	return pos->second;
      else
	return 0;
    }

  bool remove(const QString & key)
    {
      typename std::map<string,T*>::iterator pos;

      pos = m_Map.find(key);
      if(pos != m_Map.end()) {
	if(m_bAutoDelete)
	  delete pos->second;
	m_Map.erase(pos);
	return TRUE;
      }

      return FALSE;
    }

  void insert(const QString & key, const T * item)
    {
      m_Map[key] = (T*)item;
    }

  typename std::map<string,T*>::iterator end() { return m_Map.end(); }
  typename std::map<string,T*>::iterator begin() { return m_Map.begin(); }
};

template <class T>
class QDictIterator 
{
 protected:
  typename std::map<string,T*>::iterator m_Pos;
  QDict<T> & m_Dict;
 public:
  QDictIterator(QDict<T> & t) : m_Dict(t) { m_Pos = t.begin();  }
  T * current() const 
    { 
      if(m_Pos != m_Dict.end()) 
	return m_Pos->second; 
      else
	return NULL;
    }
  QDictIterator & operator++() { m_Pos++; return *this; }
  QString currentKey() const { return QString(m_Pos->first.data()); }
};

#endif

