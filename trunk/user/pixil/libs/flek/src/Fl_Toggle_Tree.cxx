#include <stdio.h>
#include <FL/Fl_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl.H>

#ifdef PIXIL
#include <nxapp.h>
#endif

#include <Flek/Fl_Toggle_Node.H>
#include <Flek/Fl_Toggle_Tree.H>
#include "pixmaps/tt_open_icon.xpm"
#include "pixmaps/tt_closed_icon.xpm"
#include "pixmaps/tt_file_small.xpm"
#include "pixmaps/tt_folder_small.xpm"

void
Fl_Toggle_Tree::select_range(Fl_Toggle_Node * start,
			     Fl_Toggle_Node * end, int add)
{

    Fl_Toggle_Node *tnode = (Fl_Toggle_Node *) first_;
    Fl_Toggle_Node *selecting = 0;
    selection_current_ = 0;
    selection_count_ = 0;

    traverse_start(tnode);

    while (tnode) {
	if (!selecting) {
	    if (tnode == start)
		selecting = end;
	    else if (tnode == end)
		selecting = start;
	}
	// add == 0 - pick one only (unpick rest)
	// add == 1 - add picked (never unpick)
	// add > 1  - toggle picked

	int tmp = tnode->selected_;
	if (selecting && (add > 1))
	    tnode->selected_ = !tnode->selected_;
	else if (selecting)
	    tnode->selected_ = 1;
	else if (add == 0)
	    tnode->selected_ = 0;

	tnode->changed_ = tmp != tnode->selected_;

	if (tnode == selecting)
	    selecting = 0;

	tnode = traverse_forward();
    }
    current_ = 0;
    if (selection_count() == 1)
	current_ = selection();
}

Fl_Pixmap *
    Fl_Toggle_Tree::s_closed_pixmap_ =
    0;
Fl_Pixmap *
    Fl_Toggle_Tree::s_opened_pixmap_ =
    0;

static const int
    no_columns[1] = {
0 };

Fl_Toggle_Tree::Fl_Toggle_Tree(int x, int y, int w, int h):
Fl_Toggle_Tree_Base(x, y, w, h)
{
    pixmap_offset_ = 16;
    label_offset_ = 40;
    current_ = 0;
    state_ = FL_TOGGLE_NONE;
    closed_pixmap_ = 0;
    opened_pixmap_ = 0;
    if (s_closed_pixmap_ == 0) {
	s_closed_pixmap_ = new Fl_Pixmap(tt_closed_icon_xpm);
    }
    if (s_opened_pixmap_ == 0) {
	s_opened_pixmap_ = new Fl_Pixmap(tt_open_icon_xpm);
    }
    closed_pixmap(s_closed_pixmap_);
    opened_pixmap(s_opened_pixmap_);
    column_widths_ = no_columns;
    column_char_ = '\t';

    selection_i_ = 0;
    selection_count_ = 0;
    selection_current_ = 0;

#if FL_MAJOR_VERSION == 1
    textfont_ = FL_HELVETICA;
#endif
    textsize_ = 12;

#ifdef PIXIL
    textcolor_ = NxApp::GlobalColor(APP_FG);
#else
    textcolor_ = FL_BLACK;
#endif

    edit_input_ = new Fl_Input(x, y, 0, 0);
    edit_input_->box(FL_FLAT_BOX);
#ifdef PIXIL
    edit_input_->color(NxApp::GlobalColor(APP_BG));
#else
    edit_input_->color(FL_WHITE);
#endif

#if FL_MAJOR_VERSION == 1
    edit_input_->textcolor(FL_BLACK);
    edit_input_->textfont(textfont_);
    edit_input_->textsize(textsize_);
#endif
    edit_callback((Fl_Callback *) edit_default_callback, this);
    edit_input_->
	when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);

    edit_input_->hide();
    edit_on_reselect_ = 1;

#ifdef PIXIL
    color(NxApp::GlobalColor(HILIGHT_LITE));
    selection_color(NxApp::GlobalColor(HILIGHT));
    selection_label_color(NxApp::GlobalColor(HILIGHT_TEXT));
    alternate_color(NxApp::GlobalColor(HILIGHT_DARK));
#else
    color(FL_WHITE);
    selection_color(FL_BLACK);
    selection_label_color(FL_YELLOW);
    alternate_color(FL_LIGHT2);
#endif

    trim_color(FL_LIGHT1);
    draw_lines_ = 0;

#ifdef PIXIL
    NxApp::DefaultFont(this);
#endif
}

Fl_Toggle_Tree::~Fl_Toggle_Tree()
{
    delete edit_input_;
}

int cnt = 0;

void
Fl_Toggle_Tree::draw_node(int depth, int cy, Fl_Toggle_Node_Base * node)
{
    Fl_Toggle_Node *tnode = (Fl_Toggle_Node *) node;

    if (damage() == FL_DAMAGE_CHILD && !tnode->changed_ && damaged_ == 0) {
	return;
    }

    tnode->changed_ = 0;
    if (tnode->selected_) {
	fl_color(selection_color());
	fl_rectf(x(), cy + 1, w(), height_(tnode) - 1);
    } else {
	fl_color((cy - y()) & 1 ? color() : alternate_color());
	fl_rectf(x(), cy + 1, w(), height_(tnode) - 1);
    }
    fl_color(trim_color());
    fl_line(x(), cy, x() + w(), cy);
    fl_color(FL_BLACK);

    if (draw_lines_) {
	int i;
	Fl_Toggle_Node *n;
	fl_xyline(x() + depth * 16 + 8, cy + 8, x() + (depth + 1) * 16,
		  cy + 8);
	if (tnode->next_)
	    fl_xyline(x() + depth * 16 + 8, cy, x() + depth * 16 + 8,
		      cy + 16);
	else
	    fl_xyline(x() + depth * 16 + 8, cy, x() + depth * 16 + 8, cy + 8);
	for (i = depth - 1, n = (Fl_Toggle_Node *) tnode->up_; n; i--,
	     n = (Fl_Toggle_Node *) n->up_)
	    if (n->next_)
		fl_xyline(x() + i * 16 + 8, cy, x() + i * 16 + 8, cy + 16);
    }

    if (tnode->can_open_) {
	if (tnode->opened_)
	    opened_pixmap_->draw(x() + depth * 16, cy);
	else
	    closed_pixmap_->draw(x() + depth * 16, cy);
    }
#if FL_MAJOR_VERSION == 1
    if (tnode->selected_)
	textcolor(selection_label_color());
    else
	textcolor(labelcolor());
#endif

    if (tnode->label_) {
	int D = depth * 16 + label_offset_;
	draw_label(tnode->label_, D, x(), cy, w(), 16);
    }
    if (tnode->pixmap_) {
	tnode->pixmap_->draw(x() + depth * 16 + pixmap_offset_, cy + 1);
    }
}


void
Fl_Toggle_Tree::draw_label(char *str, int indent, int x, int y, int w, int h)
{
    const int *i = column_widths();

    while (w > 6) {		// do each tab-seperated field
	int w1 = w;		// width for this field
	char *e = 0;		// pointer to end of field or null if none
	if (*i) {		// find end of field and temporarily replace with 0
	    for (e = str; *e && *e != column_char(); e++);
	    if (*e) {
		*e = 0;
		w1 = *i++;
	    } else
		e = 0;
	}
	int size = textsize();
	Fl_Font font = textfont();
	Fl_Color lcol = textcolor();
	Fl_Align align = (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	// check for all the @-lines recognized by XForms:
	/*    while (*str == format_char() && *++str && *str != format_char()) {
	   switch (*str++) {
	   case 'l': case 'L': size = 24; break;
	   case 'm': case 'M': size = 18; break;
	   case 's': size = 11; break;
	   case 'b': font = (Fl_Font)(font|FL_BOLD); break;
	   case 'i': font = (Fl_Font)(font|FL_ITALIC); break;
	   case 'f': case 't': font = FL_COURIER; break;
	   case 'c': align = FL_ALIGN_CENTER; break;
	   case 'r': align = FL_ALIGN_RIGHT; break;
	   case 'B': 
	   fl_color((Fl_Color)strtol(str, &str, 10));
	   fl_rectf(x, y, w1, h);
	   break;
	   case 'C':
	   lcol = (Fl_Color)strtol(str, &str, 10);
	   break;
	   case 'F':
	   font = (Fl_Font)strtol(str, &str, 10);
	   break;
	   case 'N':
	   lcol = FL_INACTIVE_COLOR;
	   break;
	   case 'S':
	   size = strtol(str, &str, 10);
	   break;
	   case '-':
	   fl_color(FL_DARK3);
	   fl_line(x+3, y+h/2, x+w1-3, y+h/2);
	   fl_color(FL_LIGHT3);
	   fl_line(x+3, y+h/2+1, x+w1-3, y+h/2+1);
	   break;
	   case 'u':
	   case '_':
	   fl_color(lcol);
	   fl_line(x+3, y+h-1, x+w1-3, y+h-1);
	   break;
	   case '.':
	   goto BREAK;
	   case '@':
	   str--; goto BREAK;
	   }
	   }
	   BREAK:
	 */
	fl_font(font, size);
#if FL_MAJOR_VERSION == 1
#  if FL_MINOR_VERSION == 0
	if (!active_r())
	    lcol = inactive(lcol);
#  else
	if (!active_r())
	    lcol = fl_inactive(lcol);
#  endif
#endif
	//    if (((FL_BLINE*)v)->flags & SELECTED)
	//      lcol = contrast(lcol, selection_color());
	fl_color(lcol);
	fl_draw(str, x + indent, y + 1, w1 - indent, h + 1, align);
	if (!e)
	    break;		// no more fields...
	*e = column_char();	// put the seperator back
	x += w1;
	w -= w1;
	str = e + 1;
	indent = 0;
    }
}

void
Fl_Toggle_Tree::open(Fl_Toggle_Node * node)
{
    if (node->opened_ == 1)
	return;

    int th = Fl_Toggle_Tree_Base::open(node);

    damaged_ = node;
    if (th) {
	damage(FL_DAMAGE_TREE);
    } else {
	damage(FL_DAMAGE_CHILD);
    }
    if (th) {
	resize(x(), y(), w(), h() + th);
	parent()->damage(FL_DAMAGE_CHILD);
    }
}

void
Fl_Toggle_Tree::close(Fl_Toggle_Node * node)
{
    if (node->opened_ == 0)
	return;

    int th = Fl_Toggle_Tree_Base::close(node);

    damaged_ = node;
    if (th) {
	damage(FL_DAMAGE_TREE);
	resize(x(), y(), w(), h() - th);
	parent()->damage(FL_DAMAGE_SCROLL);
    } else {
	damage(FL_DAMAGE_CHILD);
    }
}

void
Fl_Toggle_Tree::edit(Fl_Toggle_Node * t, int cx, int cy)
{
    // printf("edit\n");
    if (!edit_input_->visible() && t) {
	// printf("%d %d %d %d %s\n",cx, cy, w()-(cx-x()), height_(t), t->label());
	edit_input_->resize(cx - 3, cy + 1, w() - (cx - 3 - x()),
			    height_(t) - 1);
	edit_input_->value(t->label());
	edit_input_->show();
	edit_input_->take_focus();
    }
}

void
Fl_Toggle_Tree::edit_default_callback(Fl_Input *, void *ptr)
{
    //  printf("default_edit_cb\n");
    Fl_Toggle_Tree *tree = (Fl_Toggle_Tree *) ptr;
    tree->end_edit();
}

void
Fl_Toggle_Tree::end_edit(void)
{
    //  printf("end_edit\n");
    if (current_) {
	((Fl_Toggle_Node *) current_)->label(strdup(edit_input_->value()));
    }
    edit_input_->hide();

    damaged_ = current_;
    damage(FL_DAMAGE_CHILD);
}

int
Fl_Toggle_Tree::handle(int event)
{
    //  printf("event: %d\n",event);
    static Fl_Toggle_Node *prev = 0;
    switch (event) {
    case FL_ENTER:
	return 1;
    case FL_KEYBOARD:
	return 0;
    case FL_SHORTCUT:
	return 0;
    case FL_RELEASE:{
	    if (edit_input_->visible()) {
		end_edit();
	    }
	    int depth;
	    int cy;
	    Fl_Toggle_Node *tnode =
		(Fl_Toggle_Node *) Fl_Toggle_Tree_Base::find(Fl::event_y(),
							     depth, cy);
	    if (Fl::event_x() < x() + depth * 16 + 16) {
		if (tnode->opened_) {
		    current_ = tnode;
		    state_ = FL_TOGGLE_OPENED;
		    do_callback();
		    close(tnode);
		} else {
		    current_ = tnode;
		    state_ = FL_TOGGLE_CLOSED;
		    do_callback();
		    open(tnode);
		}
	    } else {
		if (Fl::event_state(FL_SHIFT)) {
		    if (prev == 0)
			prev = tnode;
		    select_range(prev, tnode, 1);
		    //current_ = 0;
		    state_ = FL_TOGGLE_SELECT;
		    do_callback();
		} else if (Fl::event_state(FL_CTRL)) {
		    //if (!tnode->selected_)
		    select_range(tnode, tnode, Fl::event_state(FL_CTRL));
		    /*
		       else {
		       selection_current_ = NULL;
		       tnode->selected_ = 0;
		       tnode->changed_ = 1;
		       tnode = 0;
		       }
		       current_ = 0;
		     */
		    state_ = FL_TOGGLE_SELECT;
		    do_callback();
		} else {
		    select_range(tnode, tnode, 0);
		    state_ =
			Fl::event_clicks()? FL_TOGGLE_HIT : FL_TOGGLE_SELECT;
		    if (tnode == current_ && state_ == FL_TOGGLE_SELECT) {
			state_ = FL_TOGGLE_RESELECT;
		    }
		    //current_ = tnode;
		    if (state_ == FL_TOGGLE_RESELECT && edit_on_reselect_) {
			edit(tnode, x() + depth * 16 + label_offset_, cy);
		    }

		    do_callback();
		}
		damaged_ = 0;
		damage(FL_DAMAGE_CHILD);
		prev = tnode;
	    }
	}
	return 1;
    }
    return 1;
}

Fl_Toggle_Node *
Fl_Toggle_Tree::selection(void)
{

    if (current_ == 0)
	current_ = traverse_start();
    else {
	traverse_start(current_);
	current_ = traverse_forward();
    }

    while (current_) {
	if (((Fl_Toggle_Node *) current_)->selected_) {
	    return (Fl_Toggle_Node *) current_;
	}
	current_ = traverse_forward();
    }
    return 0;
}


int
Fl_Toggle_Tree::selection_count(void)
{
    if (selection_current_ == 0)
	selection_count_ = 0;

    if (selection_count_)
	return selection_count_;

    Fl_Toggle_Node *t;
    t = traverse_start();
    while (t) {
	if (t->selected_) {
	    selection_count_++;
	}
	t = traverse_forward();
    }

    return selection_count_;
}


Fl_Toggle_Node *
Fl_Toggle_Tree::selection(int i)
{
    int backwards = 0;

    if (selection_current_ == 0) {
	selection_current_ = traverse_start();
	selection_i_ = 0;
    } else {
	traverse_start(selection_current_);
	if (i > selection_i_) {
	    selection_current_ = traverse_forward();
	    selection_i_++;
	} else if (i < selection_i_) {
	    selection_current_ = traverse_backward();
	    selection_i_--;
	    backwards = 1;
	}
    }

    if (backwards) {
	while (selection_current_) {
	    //printf("traversed bw to:%s\n",selection_current_->label());
	    if (selection_current_->selected_) {
		if (i == selection_i_) {
		    return selection_current_;
		}
		selection_i_--;
	    }
	    selection_current_ = traverse_backward();
	}
    } else {
	while (selection_current_) {
	    if (selection_current_->selected_) {
		if (i == selection_i_) {
		    return selection_current_;
		}
		selection_i_++;
	    }
	    selection_current_ = traverse_forward();
	}
    }
    return 0;
}


void
Fl_Toggle_Tree::opened_pixmap(Fl_Pixmap * a)
{
    if (opened_pixmap_ && opened_pixmap_ != s_opened_pixmap_) {
	delete opened_pixmap_;
    }
    opened_pixmap_ = a;
}


void
Fl_Toggle_Tree::closed_pixmap(Fl_Pixmap * a)
{
    if (closed_pixmap_ && closed_pixmap_ != s_closed_pixmap_) {
	delete closed_pixmap_;
    }
    closed_pixmap_ = a;
}


int
Fl_Toggle_Tree::remove(char *a)
{
    Fl_Toggle_Node *curr;

    curr = find(a);
    if (curr)
	return remove(curr);

    return 0;
}


int
Fl_Toggle_Tree::remove(void *a)
{
    Fl_Toggle_Node *curr;

    curr = find(a);
    if (curr)
	return remove(curr);

    return 0;
}


Fl_Toggle_Node *
Fl_Toggle_Tree::find(char *a)
{
    Fl_Toggle_Node *curr = traverse_start();

    while (curr) {
	if (curr->label()) {
	    if (!strcmp(curr->label(), a)) {
		return curr;
	    }
	}
	curr = traverse_forward();
    }

    return 0;
}


Fl_Toggle_Node *
Fl_Toggle_Tree::find(void *a)
{
    Fl_Toggle_Node *curr = traverse_start();

    while (curr) {
	if (curr->label()) {
	    if (curr->user_data() == a) {
		return curr;
	    }
	}
	curr = traverse_forward();
    }

    return 0;
}


int
Fl_Toggle_Tree::sort_by_label(Fl_Toggle_Node_Base * a,
			      Fl_Toggle_Node_Base * b)
{
    return strcmp(((Fl_Toggle_Node *) a)->label(),
		  ((Fl_Toggle_Node *) b)->label());
}


void
Fl_Toggle_Tree::edit_callback(Fl_Callback * c, void *p)
{
    edit_input_->callback((Fl_Callback *) c, p);
}


void
Fl_Toggle_Tree::edit_callback(Fl_Callback * c)
{
    edit_input_->callback((Fl_Callback *) c);
}


void
Fl_Toggle_Tree::edit_callback(Fl_Callback0 * c)
{
    edit_input_->callback(c);
}


void
Fl_Toggle_Tree::edit_callback(Fl_Callback1 * c, long p)
{
    edit_input_->callback(c, p);
}
