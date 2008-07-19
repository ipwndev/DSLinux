#ifndef __QLISTBOX_H
#define __QLISTBOX_H

#include "qstring.h"
#include "qwidget.h"
#include "Fl_Browser.H"

class QListBox : public QWidget
{
 protected:
// CRH new
  Fl_Browser *browser;
  bool m_bMultiSelection;
  int m_nCount;
 public:

  QListBox(QWidget * parent=0, const char * name=0, WFlags f=0) : QWidget(parent,name)
    { 
// CRH new
      browser = new Fl_Browser(0, 0, 0, 0, 0);
      browser->type(FL_HOLD_BROWSER);
// CRH new
      setWidget(browser);
      m_nCount = 0;
    }

  unsigned int count() const { return m_nCount; }
  virtual void setCurrentItem(int index ) {  } 
  void setMultiSelection(bool multi) 
    { 
      m_bMultiSelection = multi; 
      if (multi == true) browser->type(FL_MULTI_BROWSER);
      else browser->type(FL_HOLD_BROWSER);
    }

  bool isMultiSelection() const { return m_bMultiSelection; }
  void insertItem(const QString & text, int index=-1) 
    { 
// CRH     Fl_Browser::add(text,(void*)index);
	browser->add(text.data(), 0);
	browser->textfont(m_Font.getFont());
	browser->textsize(m_Font.size());
// end CRH
      m_nCount++;
    }
// CRH  virtual void setSelected(int item, bool b) { }
  virtual void setSelected(int item, bool b) { browser->select(item + 1, 1);}
// CRH long maxItemWidth () const { return 0; }
  long maxItemWidth () const
	{
		float len = 0;
		float max = 0;
		for(int i = 1; i <= m_nCount; i++)
			if(max < (len = fl_width(browser->text(i))))
				max = len;
		return (long)max;
	}
// CRH void changeItem(const QString & text, int index) { }
  void changeItem(const QString & text, int index)
		{browser->text(index + 1, text.data());}
  bool isSelected(int item) const { if (browser->selected(item + 1)) return true; else return false; }
};


#endif
