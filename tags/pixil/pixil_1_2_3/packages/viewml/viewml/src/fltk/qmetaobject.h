#ifndef __QMETAOBJECT
#define __QMETAOBJECT

#include "qstring.h"

class QObject;

typedef void (QObject::*QMember)();

struct QMetaData
{
  char * name;
  QMember ptr;
  QString _name;

  void stripWhiteSpace()
  {
    QString tmp = name;
    _name = tmp.stripWhiteSpace();
  }
    
};

class QMetaObject
{
 protected:
  QMetaData * m_SlotData;
  QMetaData * m_SignalData;;
  int m_nSlots;
  int m_nSignals;

  QString m_strClassName;

 public:
  QMetaObject( const char *class_name, const char *superclass_name,
	       QMetaData *slot_data,  int n_slots,
	       QMetaData *signal_data, int n_signals )
    {
      m_SlotData = slot_data;
      m_SignalData = signal_data;
      m_nSlots = n_slots;
      m_nSignals = n_signals;
      m_strClassName = class_name;

      for(int i=0; i<m_nSignals; i++) {
	m_SignalData[i].stripWhiteSpace();
      }

      for(int i=0; i<m_nSlots; i++) {
	m_SlotData[i].stripWhiteSpace();
      }
    }

  QMember findsignal(const char * signal)
    {
      QString _signal(signal+1);
      _signal = _signal.stripWhiteSpace();

      for(int i=0; i<m_nSignals; i++) {
	if(!strcmp(m_SignalData[i]._name, _signal))
	  return m_SignalData[i].ptr;
      }

      return 0;
    }

  QMember findslot(const char * slot)
    {
      QString _slot(slot+1);
      _slot = _slot.stripWhiteSpace();

      for(int i=0; i<m_nSlots; i++) {
	if(!strcmp(m_SlotData[i]._name, _slot))
	  return m_SlotData[i].ptr;
      }

      return 0;
    }

};

#endif
