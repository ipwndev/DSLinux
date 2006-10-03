#ifndef QFONT_H
#define QFONT_H

#include "qrect.h"
#include "qpoint.h"
#include "qsize.h"
#include "fltk-qbase.h"
#include "fl_draw.H"

class QFont
{
 protected:
  int m_nPixelSize;
 public:

  QFont();
  QFont ( const QString & family, int pointSize = 16, 
	  int weight = Normal, bool italic = FALSE );

  enum CharSet {ISO_8859_1, Latin1 = ISO_8859_1, AnyCharSet, ISO_8859_2, 
		Latin2 = ISO_8859_2, ISO_8859_3, Latin3 = ISO_8859_3, 
		ISO_8859_4, Latin4 = ISO_8859_4, ISO_8859_5, ISO_8859_6, 
		ISO_8859_7, ISO_8859_8, ISO_8859_9, Latin5 = ISO_8859_9, 
		ISO_8859_10, Latin6 = ISO_8859_10, ISO_8859_11, ISO_8859_12,
		ISO_8859_13, Latin7 = ISO_8859_13, ISO_8859_14, 
		Latin8 = ISO_8859_14, ISO_8859_15, Latin9 = ISO_8859_15, 
		KOI8R, Set_Ja, Set_1 = Set_Ja, Set_Ko, Set_Th_TH, Set_Zh, 
		Set_Zh_TW, Set_N = Set_Zh_TW, Unicode, Set_GBK, Set_Big5 };

 protected:
  CharSet m_CharSet;
 public:

  enum Weight { Light = 25, Normal = 50, DemiBold = 63, Bold = 75, 
		Black = 87 };

  QString m_sFamily;

  int m_nWeight;
  bool m_bItalic;
  bool m_bUnderline;
  bool m_bStrikeOut;

  void setFamily(const QString & f) { m_sFamily = f; }
  void setWeight(int & w) { m_nWeight = w; }
  void setItalic(bool & i) { m_bItalic = i; }
  void setUnderline(bool & u) { m_bUnderline = u; }
  void setStrikeOut(bool & s) { m_bStrikeOut = s; }

  void SelectFont() const;
// CRH for qpushbutton
  const int getFont() const;

  CharSet charSet() const { return m_CharSet; }
  void setCharSet(CharSet set) { m_CharSet = set; }

  const int  weight() const
    {	return m_nWeight; }
  const bool italic() const
    {	return m_bItalic; }
  const bool underline() const
    {	return m_bUnderline; }
  const bool strikeOut() const
    {	return m_bStrikeOut; }
  const char * family() const
    {   return m_sFamily; }
// CRH
  const int  size() const
    {	return m_nPixelSize; }
};


#endif
