#ifndef __KAPP_H
#define __KAPP_H

#include <kcharsets.h>
#include "klocale.h"
#include "qcursor.h"
#include "fltk-qdefs.h"

class KApplication;
extern KApplication g_KApp;

#ifndef klocale
#define klocale KApplication::getKApplication()->getLocale()
#endif

class KApplication
{
 protected:
  KCharsets m_Sets;
  KLocale m_Locale;
  QString m_Data;
 public:
  KApplication() { m_Data = "./"; }
  KCharsets * getCharsets() { return &m_Sets; }
  static KApplication * getKApplication() { return &g_KApp; }
  static const QString & kde_datadir() { return g_KApp.m_Data; }
  static const QString & kde_configdir() { return g_KApp.m_Data; }
  QColor selectColor;
  QColor selectTextColor;
  KLocale * getLocale() { return &m_Locale; }
  static GUIStyle style() { return WindowsStyle; }
};


#endif
