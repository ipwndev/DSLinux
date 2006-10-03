#ifndef __QCHECKBOX_H
#define __QCHECKBOX_H

/* Lots o' changes CRH
#include "qwidget.h"

class QCheckBox : public QWidget
{
 public:
  QCheckBox(QWidget * parent, const char * name=0) : QWidget(parent,name)
    { 
    }
  bool isChecked() const { return true;}
  void setChecked(bool check) { }

};
*/

#include "qwidget.h"
#include <Fl_Button.H>

class QCheckBox : public QWidget
{
protected:
	Fl_Button *b;
public:
	QCheckBox(QWidget * parent, const char * name=0) : QWidget(parent,name)
		{ 
		b = new Fl_Button(0, 0, 0, 0, 0);
		b->selection_color(FL_YELLOW);
		b->box(FL_DOWN_BOX);
		b->down_box(FL_DOWN_BOX);
		b->type(FL_TOGGLE_BUTTON);
		setWidget(b);
		}
	~QCheckBox() { }
	bool isChecked() const { return 1 == b->value();}
	void setChecked(bool check) {if(check) b->value(1);}
};

#endif
