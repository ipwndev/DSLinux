#include <stdio.h>		// printf
#include <stdlib.h>		// qsort

#include <Flek/Fl_Toggle_Node_Base.H>
#include <Flek/Fl_Toggle_Tree_Base.H>

Fl_Toggle_Tree_Base::Fl_Toggle_Tree_Base(int x, int y, int w, int h):
Fl_Widget(x, y, w, h)
{
    first_ = 0;
    t_current_ = 0;
    top_ = 0;
    top_depth_ = 0;
    damaged_ = 0;
}

void
Fl_Toggle_Tree_Base::draw_node(int depth, int cy, Fl_Toggle_Node_Base *)
{
    fl_color(FL_BLACK);
    fl_rectf(x(), cy, depth * 16, 16);
    fl_color(FL_WHITE);
    fl_rectf(x() + depth * 16, cy, w() - depth * 16, 16);
}

Fl_Toggle_Node_Base *
Fl_Toggle_Tree_Base::find(int fy, int &depth, int &ry)
{
    int cy = parent()->y() + top_yoffset_;
    int ey = parent()->y() + parent()->h();

    if (fy < cy)
	return 0;

    depth = top_depth_;
    Fl_Toggle_Node_Base *node = top_;
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
Fl_Toggle_Tree_Base::update_height(void)
{
    resize(x(), y(), w(), total_height(first_));
}

void
Fl_Toggle_Tree_Base::draw(void)
{
    //  printf("Fl_Toggle_Tree_Base::draw %d %d\n",x(),y());
    update_top();
    int cy = parent()->y() + top_yoffset_;
    int ey = parent()->y() + parent()->h();
    int depth = top_depth_;
    Fl_Toggle_Node_Base *node = top_;
    int drawing = 0;
    //printf("DAMAGE %d %d %d\n",damage(),FL_DAMAGE_ALL,FL_DAMAGE_CHILD);
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
	//printf("%s %d\n",(char*) node->data(),drawing);
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

int (*s_node_compare_) (Fl_Toggle_Node_Base *, Fl_Toggle_Node_Base *) = 0;

int
Fl_Toggle_Tree_Base::s_compare_(void *a, void *b)
{
    Fl_Toggle_Node_Base *nodeA = *(Fl_Toggle_Node_Base **) a;
    Fl_Toggle_Node_Base *nodeB = *(Fl_Toggle_Node_Base **) b;
    return s_node_compare_(nodeA, nodeB);
}

int
Fl_Toggle_Tree_Base::s_compare_reverse_(void *a, void *b)
{
    Fl_Toggle_Node_Base *nodeA = *(Fl_Toggle_Node_Base **) a;
    Fl_Toggle_Node_Base *nodeB = *(Fl_Toggle_Node_Base **) b;
    return -s_node_compare_(nodeA, nodeB);
}

Fl_Toggle_Node_Base *
Fl_Toggle_Tree_Base::sort_(Fl_Toggle_Node_Base * start,
			   int (*compar) (Fl_Toggle_Node_Base *,
					  Fl_Toggle_Node_Base *), int down,
			   sort_order order)
{
    int i;
    Fl_Toggle_Node_Base *node;

    i = 0;
    node = start;

    while (node) {
	node = node->next_;
	i++;
    }
    Fl_Toggle_Node_Base **array = new Fl_Toggle_Node_Base *[i];

    i = 0;
    node = start;
    while (node) {
	array[i] = node;
	node = node->next_;
	i++;
    }
    s_node_compare_ = compar;

    if (order == REVERSE_SORT) {
	qsort(array, i, sizeof(Fl_Toggle_Node_Base *),
	      (int (*)(const void *, const void *)) s_compare_reverse_);
    } else {
	qsort(array, i, sizeof(Fl_Toggle_Node_Base *),
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

Fl_Toggle_Node_Base *
Fl_Toggle_Tree_Base::sort(Fl_Toggle_Node_Base * start,
			  int (*compar) (Fl_Toggle_Node_Base *,
					 Fl_Toggle_Node_Base *),
			  sort_order order)
{
    if (first_)
	return sort_(start, compar, 0, order);
    return 0;
}

Fl_Toggle_Node_Base *
Fl_Toggle_Tree_Base::sort_tree(Fl_Toggle_Node_Base * start,
			       int (*compar) (Fl_Toggle_Node_Base *,
					      Fl_Toggle_Node_Base *),
			       sort_order order)
{
    if (first_)
	return sort_(start, compar, 1, order);
    return 0;
}

void
Fl_Toggle_Tree_Base::
sort(int (*compar) (Fl_Toggle_Node_Base *, Fl_Toggle_Node_Base *),
     sort_order order)
{
    if (first_)
	first_ = top_ = sort(first_, compar, order);
}

void
Fl_Toggle_Tree_Base::
sort_tree(int (*compar) (Fl_Toggle_Node_Base *, Fl_Toggle_Node_Base *),
	  sort_order order)
{
    if (first_)
	first_ = top_ = sort_tree(first_, compar, order);
}

void
Fl_Toggle_Tree_Base::update_top(void)
{
    Fl_Toggle_Node_Base *node = first_;
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
Fl_Toggle_Tree_Base::total_height(Fl_Toggle_Node_Base * node)
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
Fl_Toggle_Tree_Base::height(Fl_Toggle_Node_Base *)
{
    return 17;
}

Fl_Toggle_Node_Base *
Fl_Toggle_Tree_Base::traverse_start()
{
    t_current_ = first_;
    return t_current_;
}

void
Fl_Toggle_Tree_Base::traverse_start(Fl_Toggle_Node_Base * a)
{
    t_current_ = a;
}

void
Fl_Toggle_Tree_Base::traverse_up(void)
{
    if (t_current_ && t_current_->up_)
	t_current_ = t_current_->up_;
}


Fl_Toggle_Node_Base *
Fl_Toggle_Tree_Base::traverse_forward(int visible, int &depth)
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

Fl_Toggle_Node_Base *
Fl_Toggle_Tree_Base::traverse_forward()
{
    int d;
    return traverse_forward(0, d);
}

Fl_Toggle_Node_Base *
Fl_Toggle_Tree_Base::traverse_backward()
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
Fl_Toggle_Tree_Base::add_next(Fl_Toggle_Node_Base * node)
{
    if (!first_) {
	first_ = node;
	t_current_ = node;
    } else {
	if (t_current_ == 0)
	    t_current_ = first_;
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
Fl_Toggle_Tree_Base::add_sub(Fl_Toggle_Node_Base * node)
{

    if (!first_) {
	first_ = node;
	t_current_ = node;
    } else {
	if (t_current_ == 0)
	    t_current_ = first_;
	node->next_ = t_current_->sub_;

	if (t_current_->sub_)
	    t_current_->sub_->prev_ = node;
	node->prev_ = 0;
	t_current_->sub_ = node;

	if (t_current_->opened_)
	    t_current_->vsub_ = node;

	node->up_ = t_current_;
	t_current_ = node;
    }

    update_height();
    parent()->damage(FL_DAMAGE_CHILD);
    redraw();
}

int
Fl_Toggle_Tree_Base::remove(Fl_Toggle_Node_Base * a)
{
    Fl_Toggle_Node_Base *temp = a->sub_;

    while (temp != 0) {		// Remove all children
	remove(temp);
	temp = a->sub_;
    }

    if (a->prev_) {
	a->prev_->next_ = a->next_;
	if (a->next_)
	    a->next_->prev_ = a->prev_;
    } else if (a->up_) {
	Fl_Toggle_Node_Base *o = a->up_;
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
Fl_Toggle_Tree_Base::clear()
{
    while (first_)
	remove(first_);
    return 1;
}

int
Fl_Toggle_Tree_Base::close(Fl_Toggle_Node_Base * node)
{
    int th = total_height(node->vsub_);
    node->opened_ = 0;
    node->vsub_ = 0;
    return th;
}

int
Fl_Toggle_Tree_Base::open(Fl_Toggle_Node_Base * node)
{
    node->vsub_ = node->sub_;
    int th = total_height(node->vsub_);
    node->opened_ = 1;
    return th;
}
