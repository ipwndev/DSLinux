/* -*-C++-*- 

   "$Id$"
   
   Copyright 1999-2000 by the Flek development team.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
   
   Please report all bugs and problems to "flek-devel@sourceforge.net".

*/

#ifndef _FL_TREE_H_
#define _FL_TREE_H_

#include <FL/fl_draw.H>
#include <FL/Fl_Scroll.H>

#include <Flek/Fl_Toggle_Node_Base.H>

#define FL_DAMAGE_TREE 0x40

enum sort_order {
  NORMAL_SORT = 0,
  REVERSE_SORT = 1
};

/** @package libflek_ui
 * This is the base class for Fl_Toggle_Tree. It could conceivably be 
 * used to create other tree-like widgets. 
 *
 * This class has been designed to maintain a doubly linked tree list, 
 * and defers data storage and management to subclasses of 
 * Fl_Toggle_Tree_Base and Fl_Toggle_Node_Base. This class also maintains 
 * wether it's branches are visible or not (i.e. "opened" or "closed"). 
 */
class Fl_Toggle_Tree_Base : public Fl_Widget 
{
public:

  /**
   * The constructor makes an empty Fl_Toggle_Tree_Base. 
   */
  Fl_Toggle_Tree_Base (int x, int y, int w, int h);

  /**
   * Returns the first (top) Fl_Toggle_Node_Base in the widget tree.
   */
  Fl_Toggle_Node_Base* first(void) {
    return first_;
  }

  Fl_Toggle_Node_Base* find(int fy, int& depth, int& ry);

  /**
   * Sets the current pointer to t.
   */
  void traverse_start(Fl_Toggle_Node_Base * a);

  /**
   * Sets the traversal pointer to first() and then
   * returns first().
   */
  Fl_Toggle_Node_Base * traverse_start();

  /**
   * If the current node has an up pointer, traverse_up returns 
   * this pointer and sets it as the current node. 
   */
  void traverse_up (void);

  /**
   * This method traverses forward through the tree. Traversal through 
   * the node tree is done by a depth first traversal that updates the 
   * current node pointer. If traverse_forward returns 0, then the 
   * current node pointer has reached the end of the tree. Otherwise, 
   * traverse_forward returns the next node in the tree. 
   *
   * The visible flag should be set to 1 if you want to restrict 
   * traversal to the visible tree (the Fl_Toggle_Node_Bases that are 
   * not closed). The depth variable is updated with the new node depth,
   * if the old node depth is passed to traverse_forward. 
   */
  Fl_Toggle_Node_Base * traverse_forward(int visible, int &depth);

  /**
   * Equivalent to traverse_forward (0, temp). 
   */
  Fl_Toggle_Node_Base * traverse_forward();

  /**
   * Traverse back one.
   */
  Fl_Toggle_Node_Base * traverse_backward();

  /**
   * Insert n as the next item after current node. 
   */
  void add_next (Fl_Toggle_Node_Base* node);

  /**
   * Insert n as the next sub item after the current node. 
   */
  void add_sub (Fl_Toggle_Node_Base* node);

  /**
   * Remove n (and all of it's sub nodes) from the tree. If 
   * successful returns 1, else returns 0. 
   */
  int remove (Fl_Toggle_Node_Base * a);

  /** 
   * Clear the tree. 
   */
  int clear();

  /**
   * Virtual method that draws the tree. 
   */
  virtual void draw(void);

  /**
   * Update the height of the tree. 
   */
  void update_height(void);

  /**
   * Set node n as "open". 
   */
  int open(Fl_Toggle_Node_Base* node);

  /**
   * Set node n as "closed".
   */
  int close(Fl_Toggle_Node_Base* node);

  /**
   * Return the top most node in the tree.
   */
  Fl_Toggle_Node_Base* top() {
    return top_;
  }
protected:

  Fl_Toggle_Node_Base* first_;
  Fl_Toggle_Node_Base* top_;
  Fl_Toggle_Node_Base* t_current_;
  Fl_Toggle_Node_Base* current_;

  int top_depth_;
  int top_yoffset_;

  Fl_Toggle_Node_Base* damaged_;

  virtual int height(Fl_Toggle_Node_Base* node);
  int height_(Fl_Toggle_Node_Base* node) {
    return height(node) | 1;
    // height must be |1 , so Fl_ToggleTree can do color-swapping
    // with &1 on y coordinate
  }
  int total_height(Fl_Toggle_Node_Base* node);
  void update_top(void);
  virtual void draw_node(int depth, int cy, Fl_Toggle_Node_Base* node);
  virtual int handle(int) { return 1; }

  Fl_Toggle_Node_Base* sort_( Fl_Toggle_Node_Base* start,
                  int (*compar)(Fl_Toggle_Node_Base *, Fl_Toggle_Node_Base *),
                  int down, sort_order order = NORMAL_SORT);

public:
  static int s_compare_(void* a, void *b);
  static int s_compare_reverse_(void* a, void *b);

  Fl_Toggle_Node_Base* sort(Fl_Toggle_Node_Base* start, int (*compar)(Fl_Toggle_Node_Base *, Fl_Toggle_Node_Base *),
                sort_order order = NORMAL_SORT);
  Fl_Toggle_Node_Base* sort_tree(Fl_Toggle_Node_Base* start, int (*compar)(Fl_Toggle_Node_Base *, Fl_Toggle_Node_Base *),
                     sort_order order = NORMAL_SORT);

  void sort(int (*compar)(Fl_Toggle_Node_Base *, Fl_Toggle_Node_Base *),
            sort_order order = NORMAL_SORT);
  void sort_tree(int (*compar)(Fl_Toggle_Node_Base *, Fl_Toggle_Node_Base *),
                 sort_order order = NORMAL_SORT);
};

#endif
