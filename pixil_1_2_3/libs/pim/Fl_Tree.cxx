/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#include "FL/Fl_Node.H"
#include "FL/Fl_Tree.H"
#include <stdio.h>		// printf
#include <stdlib.h>		// qsort

Fl_Tree::Fl_Tree(int x, int y, int w, int h):
Fl_Widget(x, y, w, h)
{
    first_ = 0;
    top_ = 0;
    top_depth_ = 0;
    damaged_ = 0;
}

void
Fl_Tree::draw_node(int depth, int cy, Fl_Node *)
{
    fl_color(FL_BLACK);
    fl_rectf(x(), cy, depth * 16, 16);
    fl_color(FL_WHITE);
    fl_rectf(x() + depth * 16, cy, w() - depth * 16, 16);
}

Fl_Node *
Fl_Tree::find(int fy, int &depth, int &ry)
{
    int cy = parent()->y() + top_yoffset_;
    int ey = parent()->y() + parent()->h();

    if (fy < cy)
	return 0;

    depth = top_depth_;
    Fl_Node *node = top_;
    traverse_start(top_);

    while (cy < ey && node) {
	ry = cy;
	cy += height(node);
	if (cy > fy)
	    return node;
	node = traverse_forward(1, depth);
    }
    return 0;
}

void
Fl_Tree::update_height(void)
{
    resize(x(), y(), w(), total_height(first_));
}

void
Fl_Tree::draw(void)
{

    update_top();

    int cy = parent()->y() + top_yoffset_;
    int ey = parent()->y() + parent()->h();
    int depth = top_depth_;
    Fl_Node *node = top_;
    int drawing = 0;
    if (damage() == FL_DAMAGE_ALL)
	drawing = 1;
    if (damage() == FL_DAMAGE_CHILD && damaged_ == 0)
	drawing = 1;
    while (cy < ey && node) {
	if (damaged_ == node) {
	    if (damage() == FL_DAMAGE_CHILD) {
		draw_node(depth, cy, node);
		return;
	    }
	    drawing = 1;
	}
	if (drawing)
	    draw_node(depth, cy, node);
	cy += height(node);
	if (node->vsub_) {
	    //printf("has sub\n");
	    node = node->vsub_;
	    depth++;
	} else if (node->next_) {
	    //printf("has no sub\n");
	    node = node->next_;
	} else {
	    while (node && !node->next_) {
		node = node->up_;
		depth--;
	    }
	    if (node)
		node = node->next_;
	}
    }
    fl_color(parent()->color());
    fl_rectf(x(), cy, w(), ey - cy);
}

int (*s_node_compare_) (Fl_Node *, Fl_Node *) = 0;

int
Fl_Tree::s_compare_(void *a, void *b)
{
    Fl_Node *nodeA = *(Fl_Node **) a;
    Fl_Node *nodeB = *(Fl_Node **) b;
    return s_node_compare_(nodeA, nodeB);
}

int
Fl_Tree::s_compare_reverse_(void *a, void *b)
{
    Fl_Node *nodeA = *(Fl_Node **) a;
    Fl_Node *nodeB = *(Fl_Node **) b;
    return -s_node_compare_(nodeA, nodeB);
}

Fl_Node *
Fl_Tree::sort_(Fl_Node * start, int (*compar) (Fl_Node *, Fl_Node *),
	       int down, tree_sort_order order)
{
    int i;
    Fl_Node *node;

    i = 0;
    node = start;

    while (node) {
	node = node->next_;
	i++;
    }
    Fl_Node **array = new Fl_Node *[i];

    i = 0;
    node = start;
    while (node) {
	array[i] = node;
	node = node->next_;
	i++;
    }
    s_node_compare_ = compar;

    if (order == TREE_REVERSE_SORT) {
	qsort(array, i, sizeof(Fl_Node *),
	      (int (*)(const void *, const void *)) s_compare_reverse_);
    } else {
	qsort(array, i, sizeof(Fl_Node *),
	      (int (*)(const void *, const void *)) s_compare_);
    }

    start = array[0];
    int j = 1;
    node = start;
    node->prev_ = 0;		//james
    while (j < i) {
	node->next_ = array[j];
	node->next_->prev_ = node;
	node = node->next_;
	j++;
    }
    node->next_ = 0;

    if (down) {
	node = start;
	while (node) {
	    if (node->sub_)
		node->sub_ = sort_tree(node->sub_, compar, order);
	    if (node->vsub_)
		node->vsub_ = node->sub_;
	    node = node->next_;
	}
    }

    delete[]array;

    return start;
}

Fl_Node *
Fl_Tree::sort(Fl_Node * start, int (*compar) (Fl_Node *, Fl_Node *),
	      tree_sort_order order)
{
    if (first_)
	return sort_(start, compar, 0, order);
    return 0;
}

Fl_Node *
Fl_Tree::sort_tree(Fl_Node * start, int (*compar) (Fl_Node *, Fl_Node *),
		   tree_sort_order order)
{
    if (first_)
	return sort_(start, compar, 1, order);
    return 0;
}

void
Fl_Tree::sort(int (*compar) (Fl_Node *, Fl_Node *), tree_sort_order order)
{
    if (first_)
	first_ = top_ = sort(first_, compar, order);
}

void
Fl_Tree::sort_tree(int (*compar) (Fl_Node *, Fl_Node *),
		   tree_sort_order order)
{
    if (first_)
	first_ = top_ = sort_tree(first_, compar, order);
}

void
Fl_Tree::update_top(void)
{
    Fl_Node *node = first_;
    int py = parent()->y();
    int ly = y();
    int h = 0;
    int depth = 0;

    while (node && ly + (h = height(node)) <= py) {
	ly += h;
	if (node->vsub_) {
	    node = node->vsub_;
	    depth++;
	} else if (node->next_) {
	    node = node->next_;
	} else {
	    while (node && !node->next_) {
		node = node->up_;
		depth--;
	    }
	    if (node)
		node = node->next_;
	}
    }

    top_ = node;
    top_depth_ = depth;
    top_yoffset_ = ly - py;
}

int
Fl_Tree::total_height(Fl_Node * node)
{
    int ret = 0;
    int depth = 1;
    while (node) {
	ret += height(node);
	if (node->vsub_) {
	    node = node->vsub_;
	    depth++;
	} else if (node->next_) {
	    node = node->next_;
	} else {
	    while (node && !node->next_) {
		node = node->up_;
		depth--;
		if (depth <= 0)
		    node = 0;
	    }
	    if (node)
		node = node->next_;
	}
    }
    return ret;
}

int
Fl_Tree::height(Fl_Node *)
{
    return 17;
}

Fl_Node *
Fl_Tree::traverse_start()
{
    t_current_ = first_;
    return t_current_;
}

void
Fl_Tree::traverse_start(Fl_Node * a)
{
    t_current_ = a;
}

void
Fl_Tree::traverse_up(void)
{
    if (t_current_ && t_current_->up_)
	t_current_ = t_current_->up_;
}


Fl_Node *
Fl_Tree::traverse_forward(int visible, int &depth)
{

    if (visible) {
	if (t_current_ && t_current_->vsub_ != 0) {
	    t_current_ = t_current_->vsub_;
	    depth++;
	    return t_current_;
	}
    } else {
	if (t_current_ && t_current_->sub_ != 0) {
	    t_current_ = t_current_->sub_;
	    depth++;
	    return t_current_;
	}
    }

    if (t_current_ && t_current_->next_ != 0) {
	t_current_ = t_current_->next_;
	return t_current_;
    }

    while (t_current_ && !t_current_->next_) {
	t_current_ = t_current_->up_;
	depth--;
    }
    if (t_current_)
	t_current_ = t_current_->next_;

    return t_current_;
}

Fl_Node *
Fl_Tree::traverse_forward()
{
    int d;
    return traverse_forward(0, d);
}

Fl_Node *
Fl_Tree::traverse_backward()
{
    if (t_current_ && t_current_->prev_) {
	t_current_ = t_current_->prev_;
	while (t_current_->sub_) {
	    t_current_ = t_current_->sub_;
	    while (t_current_->next_) {
		t_current_ = t_current_->next_;
		if (t_current_->next_ == 0 && t_current_->sub_) {
		    t_current_ = t_current_->sub_;
		}
	    }
	}
    } else {
	t_current_ = t_current_->up_;
    }
    return t_current_;
}

void
Fl_Tree::add_next(Fl_Node * node)
{
    if (!first_) {
	first_ = node;
	t_current_ = node;
    } else {
	if (t_current_ == 0)
	    t_current_ = first_;
	while (t_current_->next_)
	    t_current_ = t_current_->next_;
	node->next_ = t_current_->next_;
	if (t_current_->next_) {
	    t_current_->next_->prev_ = node;
	}
	t_current_->next_ = node;
	node->prev_ = t_current_;
	node->up_ = t_current_->up_;
	t_current_ = node;
    }

    update_height();
    parent()->damage(FL_DAMAGE_CHILD);
    redraw();
}

void
Fl_Tree::add_sub(Fl_Node * node)
{
    Fl_Node *tmp;

    if (!first_) {
	first_ = node;
	t_current_ = node;
    } else {
	if (t_current_ == 0)
	    t_current_ = first_;
	tmp = t_current_->sub_;
	if (tmp)
	    while (tmp->next_)
		tmp = tmp->next_;
	node->next_ = 0;

	if (tmp)
	    tmp->next_ = node;
	node->prev_ = tmp;
	if (!tmp)
	    t_current_->sub_ = node;

	if (!tmp && (t_current_->opened_))
	    t_current_->vsub_ = node;

	node->up_ = t_current_;
	t_current_ = node;
    }

    update_height();
    parent()->damage(FL_DAMAGE_CHILD);
    redraw();
}

int
Fl_Tree::remove(Fl_Node * a)
{
    Fl_Node *temp = a->sub_;

    while (temp != 0) {		// Remove all children
	remove(temp);
	temp = a->sub_;
    }

    if (a->prev_) {
	a->prev_->next_ = a->next_;
	if (a->next_)
	    a->next_->prev_ = a->prev_;
    } else if (a->up_) {
	Fl_Node *o = a->up_;
	o->sub_ = a->next_;
	if (o->opened_)
	    o->vsub_ = a->next_;
	else
	    o->vsub_ = 0;

	if (a->next_) {
	    a->next_->up_ = o;
	    a->next_->prev_ = a->prev_;
	}
    }

    if (a == first_) {
	if (a->next_) {
	    first_ = a->next_;
	    first_->up_ = 0;
	    first_->prev_ = 0;
	} else {
	    first_ = 0;
	    top_ = 0;
	}
    }

    if (a == current_) {
	if (a->up_)
	    current_ = a->up_;
	else if (a->prev_)
	    current_ = a->prev_;
	else
	    current_ = 0;
    }

    if (a == t_current_) {
	if (a->up_)
	    t_current_ = a->up_;
	else if (a->prev_)
	    t_current_ = a->prev_;
	else
	    t_current_ = 0;
    }

    update_height();
    parent()->damage(FL_DAMAGE_CHILD);
    redraw();

    delete a;
    a = 0;

    return 1;
}

int
Fl_Tree::clear()
{
    return remove(first_);
}

int
Fl_Tree::close(Fl_Node * node)
{
    int th = total_height(node->vsub_);
    node->opened_ = 0;
    node->vsub_ = 0;
    return th;
}

int
Fl_Tree::open(Fl_Node * node)
{
    node->vsub_ = node->sub_;
    int th = total_height(node->vsub_);
    node->opened_ = 1;
    return th;
}
