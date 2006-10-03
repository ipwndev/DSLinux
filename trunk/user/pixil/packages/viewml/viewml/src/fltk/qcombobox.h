#ifndef __QCOMBOBOX_H
#define __QCOMBOBOX_H

#include "qstring.h"
#include "qwidget.h"
#include "Fl_Choice.H"

#define QCOMBO_HEIGHT 20
#define QCOMBO_PADDING 50

class QComboBox : public QWidget
{
 protected:
  Fl_Choice * m_pCombo;
  int m_nCount;
 public:
  QComboBox(bool rw, QWidget * parent=0, const char * name=0) : QWidget(parent,name)
    { 
      m_nCount = 0;
      QWidget::setMinimumSize(40,20);
      m_pCombo = new Fl_Choice(0,0,40,20);
      setWidget(m_pCombo);
    }
  int count() const { return m_nCount; }
  void insertItem(const QString & text, int index=-1) 
    {       
      m_nCount++;

      if(!text.length()) {
	m_pCombo->add(" ");
	_change_size(text);
      } else {
	m_pCombo->add(text);
	_change_size(text);
      }
    }
  
  void _change_size(const QString & text) 
    {
/* CRH
     if(sizeHint().height() < fl_width((const char *) text) + QCOMBO_PADDING, 
	 QCOMBO_HEIGHT) {
      
	QWidget::setMinimumSize(fl_width((const char *) text) + QCOMBO_PADDING, 
				QCOMBO_HEIGHT);
*/
	m_pCombo->textfont(m_Font.getFont());
	m_pCombo->textsize(m_Font.size());
	QFontMetrics fm(m_Font);
	if(sizeHint().width() < fm.width(text) + QCOMBO_PADDING)
		QWidget::setMinimumSize(fm.width(text) + QCOMBO_PADDING, fm.height() + 6);
	QWidget::resize(sizeHint());
    }

// CRH  void setCurrentItem(int index) { return; m_pCombo->value(index);}
  void setCurrentItem(int index) {m_pCombo->value(index);}

  int getCurrentItem() { return m_pCombo->value(); }
  void changeItem(const QString & text, int index) 
    { 
      m_pCombo->replace(index,text); 
      _change_size(text);
    }
};

#endif

