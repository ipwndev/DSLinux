

#define MWINCLUDECOLORS
#include <nano-X.h>
#include "widgets.h"


int
main(int argc, char **argv)
{

    button_t *foo, *bar, *red, *blue;
    group_t *group;

    GR_WINDOW_ID wid;

    GrOpen();

    wid = GrNewWindow(GR_ROOT_WINDOW_ID, 0, 0, 200, 200, 0, GRAY, GRAY);
    GrMapWindow(wid);

    foo = buttonCreate(wid, BUTTON_RADIO, 0, 0, 30, 30, "Foo", BLACK, WHITE);
    bar = buttonCreate(wid, BUTTON_RADIO, 31, 0, 30, 30, "Bar", BLACK, WHITE);

    red = buttonCreate(wid, BUTTON_PUSH, 0, 31, 30, 30, "Red", RED, BLUE);
    blue = buttonCreate(wid, BUTTON_PUSH, 31, 30, 35, 35, "Blue", BLUE, RED);

    group = groupCreate();
    groupAddButton(group, foo);
    groupAddButton(group, bar);

    while (1) {
	GR_EVENT event;
	GrGetNextEvent(&event);
	widgetHandler(&event);
    }
}
