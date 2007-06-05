#ifndef _FL_NODE_H_
#define _FL_NODE_H_

class Fl_Toggle_Node_Base {

  friend class Fl_Toggle_Tree_Base;

public:

  Fl_Toggle_Node_Base () {
    prev_ = 0;
    next_ = 0;
    sub_ = 0;
    vsub_ = 0;
    up_ = 0;
    opened_ = 0;
  }

  inline Fl_Toggle_Node_Base* previous() { return prev_; }
  inline Fl_Toggle_Node_Base* next() { return next_; }
  inline Fl_Toggle_Node_Base* child() { return sub_; }
  inline Fl_Toggle_Node_Base* visible_child() { return vsub_; }
  inline Fl_Toggle_Node_Base* parent() { return up_; }

protected:

  Fl_Toggle_Node_Base* prev_;
  Fl_Toggle_Node_Base* next_;
  Fl_Toggle_Node_Base* sub_;
  Fl_Toggle_Node_Base* vsub_;
  Fl_Toggle_Node_Base* up_;

  int opened_;

};

#endif
