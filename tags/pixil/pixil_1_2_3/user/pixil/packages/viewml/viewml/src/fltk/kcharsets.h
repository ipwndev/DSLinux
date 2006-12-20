#ifndef __KCHARSET_H
#define __KCHARSET_H

#include "qfont.h"
#include "qlist.h"

class KCharset
{
 public:
  KCharset() { }
  KCharset(const char * str) { }
  KCharset(const QString & str) { }
  const char * name() const { return ""; }
  QFont::CharSet qtCharset() const { return QFont::ISO_8859_1; }
  bool isDisplayable() { return true; }
  bool isDisplayable(KCharset charset, const char * face) { return true; }
  KCharset & operator=(const KCharset & set) { return *this; }
  QFont & setQFont(QFont & fnt) { return fnt; }
  operator const char *() const { return name(); }
  bool ok() const { return true; }
  bool isAvailable() { return true; }
};

class KCharsetConversionResult
{
 protected:
  KCharset m_Set;
  QString m_Text;
 public:
  char * copy() const 
    { 
      char * c = new char[m_Text.length()+1];
      strcpy(c,m_Text);
      return c;
    }

  void setResult(const char * r) { m_Text = r; }
  KCharset charset() const { return m_Set; }
  operator const QString & () const { return m_Text; }
  operator const char * () const { return m_Text; }
};


class KCharsetConverter
{
 protected:
  QList<KCharsetConversionResult> m_Result;
 public:
  enum Flags{
    INPUT_AMP_SEQUENCES=1,
    OUTPUT_AMP_SEQUENCES=2,
    AMP_SEQUENCES=INPUT_AMP_SEQUENCES|OUTPUT_AMP_SEQUENCES,
    UNKNOWN_TO_ASCII=4,
    UNKNOWN_TO_QUESTION_MARKS=0
  };
   
  KCharsetConverter() { }
  KCharsetConverter(KCharset inputCharset
		    ,int flags=UNKNOWN_TO_QUESTION_MARKS) { }

  bool ok() { return true; }
  const QList<KCharsetConversionResult> & multipleConvert(const char *str)
    {
      return m_Result;
    }
  const char * outputCharset() { return "us-ascii"; }
   
};

class KCharsets
{
 protected:
  KCharset m_KCharset;
  KCharsetConversionResult m_Result;
 public:
  KCharsets() {}
  KCharset defaultCh() const { return m_KCharset; }
  KCharset defaultCharset() const { return m_KCharset; }

  const KCharsetConversionResult & convertTag(const char * tag);
  const KCharsetConversionResult & convertTag(const char * tag,int & len);
};

#endif
