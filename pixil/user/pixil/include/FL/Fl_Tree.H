#ifndef Fl_Tree_H
#define Fl_Tree_H

#include "Fl_Node.H"
#include <FL/fl_draw.H>
#include "nxscroll.h"

#define FL_DAMAGE_TREE 0x40

enum tree_sort_order {
  TREE_NORMAL_SORT = 0,
  TREE_REVERSE_SORT = 1
};

class Fl_Tree : public Fl_Widget {

public:

  Fl_Tree(int x, int y, int w, int h);

  Fl_Node* first(void) {
    return first_;
  }

  int height_(Fl_Node* node) {
    return height(node) | 1;
    // height must be |1 , so Fl_ToggleTree can do color-swapping
    // with &1 on y coordinate
  }

  Fl_Node* find(int fy, int& depth, int& ry);

  void traverse_start(Fl_Node * a);
  Fl_Node * traverse_start();
  void traverse_up (void);
  Fl_Node * traverse_forward(int visible, int &depth);
  Fl_Node * traverse_forward();
  Fl_Node * traverse_backward();

  void add_next (Fl_Node* node);
  void add_last (Fl_Node *node);
  void add_sub (Fl_Node* node);
  int remove (Fl_Node * a);
  int clear();

  virtual void draw(void);
  void update_height(void);

  int open(Fl_Node* node);
  int close(Fl_Node* node);

  Fl_Node* top() {
    return top_;
  }
protected:

  Fl_Node* first_;
  Fl_Node* top_;
  Fl_Node* t_current_;
  Fl_Node* current_;

  int top_depth_;
  int top_yoffset_;

  Fl_Node* damaged_;

  virtual int height(Fl_Node* node);
  int total_height(Fl_Node* node);
  void update_top(void);
  virtual void draw_node(int depth, int cy, Fl_Node* node);
  virtual int handle(int) { return 1; }

  Fl_Node* sort_( Fl_Node* start,
                  int (*compar)(Fl_Node *, Fl_Node *),
                  int down, tree_sort_order order = TREE_NORMAL_SORT);

public:
  static int s_compare_(void* a, void *b);
  static int s_compare_reverse_(void* a, void *b);

  Fl_Node* sort(Fl_Node* start, int (*compar)(Fl_Node *, Fl_Node *),
                tree_sort_order order = TREE_NORMAL_SORT);
  Fl_Node* sort_tree(Fl_Node* start, int (*compar)(Fl_Node *, Fl_Node *),
                     tree_sort_order order = TREE_NORMAL_SORT);

  void sort(int (*compar)(Fl_Node *, Fl_Node *),
            tree_sort_order order = TREE_NORMAL_SORT);
  void sort_tree(int (*compar)(Fl_Node *, Fl_Node *),
                 tree_sort_order order = TREE_NORMAL_SORT);
};

#endif
