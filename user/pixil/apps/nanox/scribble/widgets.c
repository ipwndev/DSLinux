
#include <stdlib.h>
#include <string.h>
#include <nano-X.h>
#include "widgets.h"

static button_t *widgetList = 0;

static void
buttonDraw(button_t * button)
{

    int w, h, b;

    int xpos, ypos;

    GR_GC_ID gc = GrNewGC();
    GR_FONT_ID fid = GrCreateFont(GR_FONT_GUI_VAR, 0, 0);
    GrSetGCFont(gc, fid);

//  props.props = GR_WM_PROPS_NOBACKGROUND;
//  GrSetWMProperties(button->wid, &props);

    if (button->style & IMAGE_BUTTON) {
	if (button->state) {
	    if (button->downImage)
		GrDrawImageToFit(button->wid, gc, 0, 0, button->w, button->h,
				 button->downImage);
	} else {
	    if (button->upImage)
		GrDrawImageToFit(button->wid, gc, 0, 0, button->w, button->h,
				 button->upImage);
	}

	GrSetGCForeground(gc, GR_RGB(0xa8, 0xa8, 0xa8));
	GrLine(button->wid, gc, 0, 0, button->w, 0);

	GrDestroyGC(gc);
	GrDestroyFont(fid);
	return;
    }
#if 0
    if (button->state)
	GrSetGCForeground(gc, button->fgcolor);
    else
	GrSetGCForeground(gc, button->bgcolor);
#else
    GrSetGCForeground(gc, button->bgcolor);
#endif

    GrFillRect(button->wid, gc, 2, 2, button->w - 4, button->h - 4);

    /* Draw Border & Corners */

    GrSetGCForeground(gc, GR_RGB(0xa8, 0xa8, 0xa8));


    if (!button->y)
	GrSetGCForeground(gc, GR_RGB(0, 0, 0));

    GrLine(button->wid, gc, 0, 0, button->w, 0);

#if 0
    if (button->state) {
	GrSetGCForeground(gc, button->bgcolor);
	GrSetGCBackground(gc, button->fgcolor);
    } else {
	GrSetGCForeground(gc, button->fgcolor);
	GrSetGCBackground(gc, button->bgcolor);
    }
#else
    GrSetGCForeground(gc, button->fgcolor);
    GrSetGCBackground(gc, button->bgcolor);
#endif

    GrGetGCTextSize(gc, button->text, -1, GR_TFTOP, &w, &h, &b);

    xpos = (button->w - w) / 2;
    ypos = (button->h - h) / 2;

    GrText(button->wid, gc, xpos, ypos, button->text, -1, GR_TFTOP);


    GrDestroyGC(gc);
    GrDestroyFont(fid);
}

static void
buttonClickdown(button_t * button)
{

    if (button->style & BUTTON_RADIO) {
	if (button->group) {
	    group_t *group = button->group;

	    if (group->active) {
		group->active->state = 0;
		buttonDraw(group->active);
	    }

	    group->active = button;
	}
    } else if (button->style & BUTTON_TOGGLE) {
	if (button->state)
	    button->state = 0;
	else
	    button->state = 1;

    } else {
	button->state = 1;
    }

    if (button->downProc)
	button->downProc(button->downData);
    buttonDraw(button);
}

static void
buttonClickup(button_t * button)
{

    if (button->style & BUTTON_RADIO || button->style & BUTTON_TOGGLE)
	return;
    if (button->state == 0)
	return;

    button->state = 0;


    if (button->upProc)
	button->upProc(button->upData);
    buttonDraw(button);
}

static void
addWidget(button_t * button)
{

    /* Add the widget to the list of existing widgets */
    if (!widgetList)
	widgetList = button;
    else {
	button_t *p = widgetList;
	while (p->next)
	    p = p->next;
	p->next = button;
    }
}


/* External API functions */

void
buttonShow(button_t * button)
{
    GrMapWindow(button->wid);
}

void
buttonHide(button_t * button)
{
    button->state = 0;
    if (button->group)
	if (button->group->active == button)
	    button->group->active = 0;

    GrUnmapWindow(button->wid);
}

void
buttonSetState(button_t * button, int state)
{
    button->state = state;

    if (button->group) {
	if (state) {
	    if (button->group->active) {
		button->group->active->state = 0;
		buttonDraw(button->group->active);
	    }

	    button->group->active = button;
	} else if (button->group->active == button)
	    button->group->active = 0;
    }

    buttonDraw(button);
}

void
buttonCallback(button_t * button, int mode, void (proc) (void *), void *data)
{


    if (mode == BUTTON_DOWN_EVENT) {
	button->downProc = proc;
	button->downData = data;
    } else {
	button->upProc = proc;
	button->upData = data;
    }
}

static button_t *
makeButton(GR_WINDOW_ID parent, int type,
	   int x, int y, int w, int h, GR_COLOR bgcolor, GR_COLOR fgcolor)
{

    button_t *local = (button_t *) calloc(sizeof(button_t), 1);
    if (!local)
	return (0);

    local->x = x;
    local->y = y;
    local->w = w;
    local->h = h;

    local->bgcolor = bgcolor;
    local->fgcolor = fgcolor;

    local->style = type;
    local->state = 0;


    local->wid = GrNewWindow(parent, x, y, w, h, 0, bgcolor, fgcolor);

    GrSelectEvents(local->wid, GR_EVENT_MASK_EXPOSURE |
		   GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP);

    addWidget(local);
    return (local);
}

button_t *
buttonCreate(GR_WINDOW_ID parent, int type,
	     int x, int y, int w, int h, char *text,
	     GR_COLOR bgcolor, GR_COLOR fgcolor)
{

    button_t *local = makeButton(parent, type, x, y, w, h, bgcolor, fgcolor);
    if (!local)
	return (0);

    local->style |= TEXT_BUTTON;
    strcpy(local->text, text);
    buttonShow(local);
    return (local);
}

button_t *
imageButtonCreate(GR_WINDOW_ID parent, int type,
		  int x, int y, int w, int h, char *downimage, char *upimage)
{

    button_t *local = makeButton(parent, type, x, y, w, h, GR_RGB(0, 0, 0),
				 GR_RGB(0, 0, 0));
    if (!local)
	return (0);

    local->style |= IMAGE_BUTTON;

    local->upImage = GrLoadImageFromFile(upimage, 0);
    local->downImage = GrLoadImageFromFile(downimage, 0);

    buttonShow(local);
    return (local);
}


static button_t *
findWidget(GR_WINDOW_ID id)
{

    button_t *ptr = widgetList;
    while (ptr) {
	if (ptr->wid == id)
	    return (ptr);
	ptr = ptr->next;
    }

    return (0);
}

group_t *
groupCreate(void)
{
    return ((group_t *) calloc(sizeof(group_t), 1));
}

void
groupAddButton(group_t * group, button_t * button)
{
    if (!button || !group)
	return;
    button->group = group;

    if (button->state) {
	if (group->active) {
	    group->active->state = 0;
	    buttonDraw(group->active);
	}

	group->active = button;
    }
}

button_t *
groupActive(group_t * group)
{
    return (group->active);
}

void
widgetHandler(GR_EVENT * event)
{

    button_t *button;

    switch (event->type) {

    case GR_EVENT_TYPE_EXPOSURE:
	button = findWidget(event->exposure.wid);

	if (button)
	    buttonDraw(button);

	break;

    case GR_EVENT_TYPE_BUTTON_DOWN:
	button = findWidget(event->button.wid);

	if (button)
	    buttonClickdown(button);

	break;

    case GR_EVENT_TYPE_BUTTON_UP:
	button = findWidget(event->button.wid);

	if (button)
	    buttonClickup(button);

	break;
    }

}
