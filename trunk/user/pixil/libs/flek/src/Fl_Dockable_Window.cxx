#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Pack.H>

#include <Flek/Fl_Dockable_Window.H>
#include "pixmaps/dock_grip_tile.xpm"
#include <stdio.h>

long
    Fl_Dockable_Window::gripper_width =
    10;
Fl_Dockable_Window *
    Fl_Dockable_Window::current =
    0;

void
Fl_Gripper::draw()
{
    if (_grippertype & FL_DRAGABLE) {
	// Draw the Gnome style gripper:
#if FL_MAJOR_VERSION == 1
	Fl_Color col = value()? selection_color() : color();
	draw_box(box(), col);
#else
	draw_box();
#endif
	// draw_label();

#if FL_MAJOR_VERSION == 1
	fl_push_clip(x() + 1, y() + 1, w() - 3, h() - 3);
#else
	fl_clip(x() + 1, y() + 1, w() - 3, h() - 3);
#endif

	for (int i = 0; i < w(); i += 6)
	    for (int j = 0; j < h(); j += 6)
		fl_draw_pixmap(grip_tile_xpm, i + 2, j + 2);

	fl_pop_clip();
    } else if (_grippertype & FL_SHOVABLE)
	Fl_Button::draw();
}

int
Fl_Gripper::handle(int event)
{
    int rval = Fl_Button::handle(event);
    int x_drag, y_drag;
    Fl_Dockable_Window *group = (Fl_Dockable_Window *) parent();
    Fl_Dockable_Window::current = group;

    switch (event) {
    case FL_PUSH:
	// Set the Fl_Dockable_Window that may eventually get docked.
	if (group->window() && (group->window()->modal()))
	    return 1;

	if (_grippertype & FL_DRAGABLE) {
	    Fl_Dockable_Window::current = (Fl_Dockable_Window *) parent();
	    x_down = Fl::event_x_root();
	    y_down = Fl::event_y_root();
	    x_first = Fl_Dockable_Window::current->x_root();
	    y_first = Fl_Dockable_Window::current->y_root();
	}
	return 1;

    case FL_DRAG:
    case FL_RELEASE:{
	    int x_up = Fl::event_x();
	    int y_up = Fl::event_y();

	    Fl_Pack *parent = (Fl_Pack *) group->parent();

	    if ((_grippertype & FL_SHOVABLE) && (event & FL_RELEASE)
		&& (group->parent()) && (x_up <= w() + 2)
		&& (y_up <= h() + 2)) {

#if 0
		int content_location = 0;
		// Apparently there is a bug somewhere in this algorithm.  bdl
		for (int i = 0; i < parent->children(); i++) {
		    if (parent->child(i)->type() != FL_WINDOW) {
			content_location = i;
		    }
		}

		int group_location = parent->find(group);
		int new_location =
		    content_location + (content_location - group_location) +
		    1;
#else
		// Simpler algorithm.  bdl
		// Shove it down until it hits the bottom, then shove it
		// up to the top.
		int new_location = parent->find(group) + 1;
		if (new_location >= parent->children())
		    new_location = 0;
#endif

		parent->insert(*((Fl_Widget *) group), new_location);
		parent->redraw();
		return 1;
	    }

	    if (!(_grippertype & FL_DRAGABLE))
		return 1;

	    if (group->window() && group->window()->modal())
		return 1;

	    x_drag = Fl::event_x_root();
	    y_drag = Fl::event_y_root();

	    int delta = ((x_drag + y_drag) - (x_down + y_down));

	    if (delta < 0)
		delta *= -1;

	    if (group->parent()) {
		if (delta > (FL_DOCK_DELTA * 2)) {
		    x_first = Fl_Dockable_Window::current->x_root();
		    y_first = Fl_Dockable_Window::current->y_root();

		    group->undock((x_first + (x_drag - x_down)),
				  (y_first + (y_drag - y_down)));

		    // Once undocked, give the window focus and continue dragging.
		    take_focus();
		    Fl::pushed(this);
		}
	    } else {
		// See if anyone want to dock with me..
		for (Fl_Window * o = Fl::first_window(); o;
		     o = Fl::next_window(o)) {
		    int ex = o->x_root();
		    int ey = o->y_root();
		    int ew = o->w();
		    int eh = o->h();

		    int cx = Fl::event_x_root();
		    int cy = Fl::event_y_root();

		    if (o->visible() && (cx > ex) && (cy > ey)
			&& (cx < (ew + ex)) && (cy < (eh + ey))) {
			// Send the host window a message that we want to dock with it.
			// printf ("Fl_Dockable_Window:: Attempting to dock.\n");
			if (Fl::handle(FL_DOCK, o)) {
			    // printf ("Fl_Dockable_Window:: Docking.\n");
			    // redraw();
			    if (event != FL_RELEASE) {
				// I want to send JUST the drag event.  Not push!
				// Right now it just stops dragging after a successful dock.
				// take_focus();
				// Fl::pushed(this);
			    }
			    break;
			}
		    }
		}
	    }

	    // Isn't it docked?
	    if (!group->docked())
		// Move the window around following the pointer.
		group->position((x_first + (x_drag - x_down)),
				(y_first + (y_drag - y_down)));
	}

	return 1;
    }

    return rval;
}

void
Fl_Gripper::grippertype(unsigned char t)
{
    _grippertype = t;
}

unsigned char
Fl_Gripper::grippertype()
{
    return _grippertype;
}


Fl_Dockable_Window::Fl_Dockable_Window(int x, int y, int w, int h,
				       const char *l)
:  Fl_Window(x, y, w, h, l)
{
    create_dockable_window();
}

Fl_Dockable_Window::Fl_Dockable_Window(int w, int h, const char *l)
    :
Fl_Window(w, h, l)
{
    create_dockable_window();
}

void
Fl_Dockable_Window::create_dockable_window()
{
    clear_border();
    uw_ = w();
    uh_ = h();
    gripper = new Fl_Gripper(0, 0, gripper_width, h());
    _docked = 0;
    contents = new Fl_Window(gripper_width, 0, w(), h());
    resizable(contents);
    grippertype(FL_DRAGABLE);
    begin();
}

void
Fl_Dockable_Window::show()
{
    Fl_Window::show();
    gripper->show();
}

int
Fl_Dockable_Window::handle(int event)
{
    int rval;
    switch (event) {
    case FL_SHOW:
	rval = Fl_Window::handle(event);
	contents->show();
	return rval;
    case FL_HIDE:
	contents->hide();
	break;
    }
    return Fl_Window::handle(event);
}

void
Fl_Dockable_Window::undock(int x, int y)
{
    _docked = 0;
    // printf ("undocking..\n");
    if (parent()) {
	Fl_Group *group = (Fl_Group *) parent();
	hide();

	group->remove((Fl_Widget *) this);

	// printf ("Let dock handle the event..\n");
	// Give the dock an opportunity to handle the event.
	Fl::handle(FL_UNDOCK, group->window());

	// group->redraw();

	if ((x > -1) && (y > -1))
	    position(x, y);

	// printf("positioned x, y = %d,%d", x, y);

	size(uw_, uh_);

	// printf ("show me!!\n");
	show();
	// layout();
    }
}

void
Fl_Dockable_Window::grippertype(unsigned char t)
{
    // Set the type of the gripper.
    gripper->grippertype(t);

    if ((t & FL_DRAGABLE) || (t & FL_SHOVABLE)) {
	contents->resize(gripper_width, 0, w(), h());
	gripper->show();
    } else {
	contents->resize(0, 0, w(), h());
	gripper->hide();
    }
}

unsigned char
Fl_Dockable_Window::grippertype()
{
    return gripper->grippertype();
}
