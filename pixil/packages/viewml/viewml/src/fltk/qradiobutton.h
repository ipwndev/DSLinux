#ifndef __QRADIOBUTTON_H
#define __QRADIOBUTTON_H

/* Lots o' changes CRH
#include "qwidget.h"
#include <Fl_Check_Button.H>

class QRadioButton : public QWidget
{
 public:
  QRadioButton(QWidget * parent, const char * name=0) : QWidget(parent,name)
    { 
      QWidget::setMinimumSize(30,30);
      Fl_Button * b = new Fl_Check_Button(0,0,30,30,"");
      Fl_Group::add(b);
      setWidget(b);
    }
  ~QRadioButton() { }
  bool isChecked() const { return false; }
  virtual void setChecked(bool check) { }
};
*/

#include "qwidget.h"
#include <Fl_Button.H>

static void cb_radio(Fl_Widget *, QWidget *me)
		{
		Fl_Group *g = ((Fl_Group *)me->parent());
		int i;
		for(i = 0; i < g->children(); i++)
			{
			QWidget *w = (QWidget *)g->child(i);
			if(w->getWidget()->type() == FL_RADIO_BUTTON && w != me)
				((Fl_Button *)w->getWidget())->clear();
			}
		}

class QRadioButton : public QWidget
{
protected:
	Fl_Button *b;
public:
	QRadioButton(QWidget * parent, const char * name=0) : QWidget(parent,name)
		{ 
		b = new Fl_Button(0, 0, 0, 0, 0);
		b->selection_color(FL_RED);
		b->box(FL_ROUND_DOWN_BOX);
		b->down_box(FL_ROUND_DOWN_BOX);
		b->type(FL_RADIO_BUTTON);
		setWidget(b);
		b->callback((Fl_Callback *)cb_radio, this);
		}
	~QRadioButton() { }
	bool isChecked() const { return 1 == b->value();}
	void setChecked(bool check) {if(check) {cb_radio(0, this); b->set();}}

};

#endif
