
#ifndef _WIDGETS_H_
#define _WIDGETS_H_

#define BUTTON_PUSH  0x00
#define BUTTON_RADIO 0x01
#define BUTTON_TOGGLE 0x02

#define TEXT_BUTTON  0x1000
#define IMAGE_BUTTON 0x2000

#define BUTTON_DOWN_EVENT 0x00
#define BUTTON_UP_EVENT   0x01

struct group_struct;

typedef struct button_struct
{

    /* Values that can be set by the user */

    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;

    unsigned short style;
    char text[15];

    GR_COLOR bgcolor;
    GR_COLOR fgcolor;

    GR_IMAGE_ID upImage;
    GR_IMAGE_ID downImage;

    /* Internal values */
    GR_WINDOW_ID wid;
    unsigned state;

    void (*downProc) (void *);
    void (*upProc) (void *);

    void *downData;
    void *upData;

    struct group_struct *group;
    struct button_struct *next;
}
button_t;

typedef struct group_struct
{
    button_t *active;
}
group_t;

void buttonShow(button_t * button);
void buttonHide(button_t * button);
void buttonSetState(button_t * button, int state);

void buttonCallback(button_t * button,
		    int mode, void (*proc) (void *), void *data);

button_t *buttonCreate(GR_WINDOW_ID parent, int type,
		       int x, int y, int w, int h, char *text,
		       GR_COLOR bgcolor, GR_COLOR fgcolor);

button_t *imageButtonCreate(GR_WINDOW_ID parent, int type,
			    int x, int y, int w, int h,
			    char *upimage, char *downimage);

group_t *groupCreate(void);
button_t *groupActive(group_t * group);

void groupAddButton(group_t * group, button_t * button);
void widgetHandler(GR_EVENT * event);

#endif
