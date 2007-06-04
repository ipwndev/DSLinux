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

#ifndef _FL_TOGGLE_TREE_H_
#define _FL_TOGGLE_TREE_H_

#include <Flek/Fl_Toggle_Tree_Base.H>
#include <Flek/Fl_Toggle_Node.H>
#include <FL/Fl_Pixmap.H>


enum Fl_ToggleState {
  FL_TOGGLE_NONE = 0,
  FL_TOGGLE_SELECT = 1,
  FL_TOGGLE_RESELECT = 2,
  FL_TOGGLE_SELECT_MASK = 3,
  FL_TOGGLE_OPENED = 4,
  FL_TOGGLE_CLOSED = 8,
  FL_TOGGLE_HIT = 16
};

class Fl_Input;

/** @package libflek_ui
 * Fl_Toggle_Tree is a tree widget that allows branch visibility to 
 * be toggled on and off, similar to a collapsible list widget that is 
 * found in most widget tool kits. 
 * <p><img src="Fl_Toggle_Tree.png">
 */
class Fl_Toggle_Tree : public Fl_Toggle_Tree_Base {

public:
  /**
   * The constructor makes an empty Fl_Toggle_Tree. 
   */
  Fl_Toggle_Tree(int x, int y, int w, int h);
  virtual ~Fl_Toggle_Tree();

  virtual int handle(int event);

  /**
   * Returns the current node (as set with the traversal functions).
   */
  Fl_Toggle_Node* current(void) {
    return (Fl_Toggle_Node *)t_current_;
  }

  /**
   * Returns the state of the Fl_Toggle_Tree widget. Enumerated states 
   * include: FL_TOGGLE_NONE = 0, FL_TOGGLE_SELECT = 1,
   * FL_TOGGLE_RESELECT = 2, FL_TOGGLE_SELECT_MASK = 3, 
   * FL_TOGGLE_OPENED = 4, FL_TOGGLE_CLOSED = 8 and
   * FL_TOGGLE_HIT = 16.
   */
  Fl_ToggleState state(void) {
    return state_;
  }

  /**
   * Set the selection text color for lines in the browser.
   */
  void selection_label_color(Fl_Color c) {
    selection_label_color_ = c;
  }

  /**
   * Get the selection text color for lines in the browser.
   */
  Fl_Color selection_label_color(void) {
    return selection_label_color_;
  }

  /**
   * Sets the alternating background color for lines in the browser.
   */
  void alternate_color(Fl_Color c) {
    alternate_color_ = c;
  }

  /**
   * Gets the alternating background color for lines in the browser.
   */
  Fl_Color alternate_color(void) {
    return alternate_color_;
  }

  /**
   * Sets the trim color that seperates lines of text in the browser. 
   */
  void trim_color(Fl_Color c) {
    trim_color_ = c;
  }

  /**
   * Gets the trim color that seperates lines of text in the browser. 
   */
  Fl_Color trim_color(void) {
    return trim_color_;
  }

  /**
   * Sets if toggled items are indented or not. 
   */
  void indent_toggles(int b) {
    indent_toggles_ = b;
  };

  /**
   * Gets if toggled items are indented or not. 
   */
  int indent_toggles(void) {
    return indent_toggles_;
  };

  /**
   * Sets if lines are drawn between toggle nodes.
   */
  void draw_lines(int b) {
    draw_lines_ = b;
  };

  /**
   * Gets if lines are drawn between toggle nodes.
   */
  int draw_lines(void) {
    return draw_lines_;
  };

  /**
   * Open node n. This reveals any sub items belonging to n. 
   */
  void open(Fl_Toggle_Node* node);

  /**
   * Close node n. This hides any sub items belonging to n. 
   */
  void close(Fl_Toggle_Node* node);

  /**
   * Set the horizontal label offset to x pixels. 
   */
  void label_offset(int l) {
    label_offset_ = l;
  }

  /**
   * Set the horizontal pixmap offset to x pixels. 
   */
  void pixmap_offset(int l) {
    pixmap_offset_ = l;
  }

  /**
   * Set the pixmap displayed when a node is "open".
   */
  void opened_pixmap(Fl_Pixmap *);

  /**
   * Set the pixmap displayed when a node is "closed".
   */
  void closed_pixmap(Fl_Pixmap *a);

  /**
   * Get the pixmap displayed when a node is "open".
   */
  Fl_Pixmap * opened_pixmap() {
    return opened_pixmap_;
  }

  /**
   * Get the pixmap displayed when a node is "closed".
   */
  Fl_Pixmap * closed_pixmap() {
    return closed_pixmap_;
  }

  /**
   * Gets the current column width array. This array is zero-terminated 
   * and specifies the widths in pixels of each column. The
   * text is split at each column_char() and each part is formatted into 
   * it's own column. After the last column any remaining text is formatted
   * into the space between the last column and the right edge of the 
   * Fl_Toggle_Tree wdiget, even if the text contains instances of
   * column_char() . The default value is a one-element array of just a 
   * zero, which makes there are no columns. 
   */
  const int* column_widths() const {return column_widths_; }

  /**
   * Sets the current array to w. Make sure the last entry is zero. 
   */
  void column_widths(const int* l) {
    column_widths_ = l;
  }

  /**
   * Gets the default text font for the lines in the browser. 
   */
  Fl_Font textfont() const {return (Fl_Font)textfont_; }

  /**
   * Sets the default text font for the lines in the browser. 
   */
  void textfont(Fl_Font s) {
    textfont_ = s;
  }

  /**
   * Gets the default text size for the lines in the browser. 
   */
  unsigned textsize() const {return textsize_; }

  /**
   * Sets the default text size for the lines in the browser. 
   */
  void textsize(unsigned s) {
    textsize_ = s;
  }

  /**
   * Gets the default text color for the lines in the browser. 
   */
  Fl_Color textcolor() const {return (Fl_Color)textcolor_; }

  /**
   * Sets the default text color for the lines in the browser. 
   */
  void textcolor(Fl_Color n) {
    textcolor_ = n;
  }

  /**
   * Gets the current column separator character. By default this is '\t' (tab).
   */
  char column_char() const {return column_char_; }

  /**
   * Sets the column separator to c. This will only have an effect if 
   * you also set column_widths().
   */
  void column_char(char c) {
    column_char_ = c;
  }

  /**
   * Traverses forward and returns the next selected node. 
   */
  Fl_Toggle_Node* selection(void);

  /**
   * Returns the i-th selected node. 
   */
  Fl_Toggle_Node* selection(int i);

  /**
   * Returns the currently selected index.
   */
  int selection_count(void);

  /**
   * Return the selected node. If multiple nodes are selected, then 
   * selected() returns 0, and selection() should be checked. Note
   * that the current() node is not necessarily equal to the selected() 
   * node. Fl_Toggle_Tree attempts to maintain the selected item
   * even while the tree is being modified. Of course, if the node 
   * is unselected or deleted by some operation, selected() will return 0. 
   */
  Fl_Toggle_Node* selected(void) {
    return (Fl_Toggle_Node *)current_;
  }

  /**
   * Sets nodes between start and end as selected. add.
   */
  void select_range(Fl_Toggle_Node* start, Fl_Toggle_Node* end, int add = 0);

  /**
   * Unselects all selected items.
   */
  void unselect(void) {
    select_range(0, 0, 0);
  }

  /* Remove an item from the tree but also select the previous line
     if possible. */

  /**
   * Removes n (and all of it's sub nodes) from the tree. If successful 
   * remove returns 1, otherwise it returns 0. 
   */
  int remove (Fl_Toggle_Node * a) {
    Fl_Toggle_Node * sel = 0;
    if (a->selected_) {
      if (a->up_)
	sel = (Fl_Toggle_Node *)a->up_;
      else if (a->prev_)
	sel = (Fl_Toggle_Node *)a->prev_;
    }
    Fl_Toggle_Tree_Base::remove((Fl_Toggle_Node_Base *) a);

    if (!sel) sel = (Fl_Toggle_Node *)first_;
    if (sel) sel->selected_ = 1;
    current_ = sel;
    redraw();
    return 1;
  }

  /**
   * Performs a find() with d as it's argument and remove()s the
   * returned node if it exists.
   */
  int remove (void * a);

  /**
   * Performs a find() with the string c as it's argument and remove()s 
   * the returned node if it exists. 
   */
  int remove (char * a);

  /**
   * Sets the current pointer to t. 
   */
  void traverse_start(Fl_Toggle_Node_Base * a) {
    Fl_Toggle_Tree_Base::traverse_start(a);
  }

  /**
   * Sets the traversal pointer to 
   * first() and then returns first(). The second form is provided for 
   * convenience. 
   */
  Fl_Toggle_Node * traverse_start() {
    return (Fl_Toggle_Node *) Fl_Toggle_Tree_Base::traverse_start();
  }

  /**
   * This method traverses forward through the tree. Traversal through 
   * the node tree is done by a depth first traversal that updates the 
   * current node pointer. If traverse_forward returns 0, then the 
   * current node pointer has reached the end of the tree. Otherwise, 
   * traverse_forward returns the next node in the tree. 
   * 
   * The visible flag should be set to 1 if you want to restrict traversal 
   * to the visible tree (the Fl_Toggle_Node_Bases that are not closed). 
   * The depth variable is updated with the new node depth, if the old 
   * node depth is passed to traverse_forward. 
   */
  Fl_Toggle_Node * traverse_forward(int visible, int &depth) {
    return (Fl_Toggle_Node *) Fl_Toggle_Tree_Base::traverse_forward(visible, depth);
  }

  /**
   * Same as traverse_forward(0, temp). 
   */
  Fl_Toggle_Node * traverse_forward() {
    return (Fl_Toggle_Node *) Fl_Toggle_Tree_Base::traverse_forward();
  }

  Fl_Toggle_Node * traverse_backward() {
    return (Fl_Toggle_Node *) Fl_Toggle_Tree_Base::traverse_backward();
  }

  /**
   * Finds the first node in the tree that matches the string c. 
   * If no node matches c, then find returns 0.
   */
  Fl_Toggle_Node * find (void * a);

  /**
   * Finds the first node in the tree whose data pointer is equal to 
   * the pointer d. If no node matches d, then find returns 0. 
   */
  Fl_Toggle_Node * find (char * a);

  /**
   * Inserts n as the next sub item after the current node. If the tree 
   * is empty, then n becomes the first node of the toggle tree.
   */
  Fl_Toggle_Node * add_sub(char* label = 0, int can_open = 1,
                          Fl_Pixmap* pixmap = 0, void * d = 0) {
    Fl_Toggle_Node * node;
    Fl_Toggle_Tree_Base::add_sub(node = new Fl_Toggle_Node(label, can_open, pixmap, d));
    return node;
  }

  /**
   * Inserts n as the next item after the current node. If the tree 
   * is empty, then n becomes the first node of the toggle tree. 
   */
  Fl_Toggle_Node * add_next(char* label = 0, int can_open = 1,
                           Fl_Pixmap* pixmap = 0, void * d = 0) {
    Fl_Toggle_Node * node;
    Fl_Toggle_Tree_Base::add_next(node = new Fl_Toggle_Node(label, can_open, pixmap, d));
    return node;
  }

  /**
   * Equivalent to add_sub(new Fl_Toggle_Node(label, can_open, pixmap, d)). 
   */
  void add_sub(Fl_Toggle_Node * a) {
    Fl_Toggle_Tree_Base::add_sub(a);
  }

  /**
   * Equivalent to add_next(new Fl_Toggle_Node(label, can_open, pixmap, d)). 
   */
  void add_next(Fl_Toggle_Node * a) {
    Fl_Toggle_Tree_Base::add_next(a);
  }

  /**
   * Sort the tree by label.
   */
  static int sort_by_label(Fl_Toggle_Node_Base* a, Fl_Toggle_Node_Base* b);

  /**
   * If on, this causes editing to occur if an item is reselected.
   */
  void edit_on_reselect(int b) {
    edit_on_reselect_ = b;
  }

  /**
   * Set the callback to invoke when editing.
   */
  void edit_callback(Fl_Callback* c, void* p);

  /**
   * Set the callback to invoke when editing.
   */
  void edit_callback(Fl_Callback* c);

  /**
   * Set the callback to invoke when editing.
   */
  void edit_callback(Fl_Callback0*c);

  /**
   * Set the callback to invoke when editing.
   */
  void edit_callback(Fl_Callback1*c, long p = 0);

  /**
   * The default edit callback.
   */
  static void edit_default_callback(Fl_Input* input, void* ptr);

  /**
   * Stop editing an entry.
   */
  void end_edit(void);

protected:
  virtual void edit(Fl_Toggle_Node* t, int cx, int cy);

  void draw_label(char* str, int indent, int x, int y, int w, int h);
  virtual void draw_node(int depth, int cy, Fl_Toggle_Node_Base* node);
  int label_offset_;
  int pixmap_offset_;
  int indent_toggles_;
  int edit_on_reselect_;
  int draw_lines_;
  Fl_Color alternate_color_;
  Fl_Color selection_label_color_;
  Fl_Color trim_color_;
  const int* column_widths_;
  char column_char_;

  Fl_Font textfont_;
  unsigned textsize_;
  Fl_Color textcolor_;

  Fl_ToggleState state_;
  Fl_Pixmap* opened_pixmap_;
  Fl_Pixmap* closed_pixmap_;
  static Fl_Pixmap* s_opened_pixmap_;
  static Fl_Pixmap* s_closed_pixmap_;

  int selection_i_;
  int selection_count_;
  Fl_Toggle_Node* selection_current_;

  Fl_Input *edit_input_;
};

/**
 * A file (peice of paper) xpm image.
 */
extern char * tt_file_small_xpm[];

/**
 * A file folder xpm image.
 */
extern char * tt_folder_small_xpm[];

/**
 * An open toggle xpm image.
 */
extern char * tt_open_icon_xpm[];

/**
 * A closed toggle xpm image.
 */
extern char * tt_closed_icon_xpm[];

#endif
