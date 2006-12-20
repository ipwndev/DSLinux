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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <xml/xml.h>
#include <nano-X.h>

#include "nanowm.h"
#include "themes.h"

/* This is a list of widgets that are accepted by each component  */
/* this gives us the change to be a little more flexable with the */
/* script */

char *titlebarList[] = { "caption", "closebutton", "titlebar", "" };
char *borderList[] = { "border" };

char **widgetList[] = {
    titlebarList,
    borderList,
    borderList,
    borderList,
    borderList
};

/* XML callback functions */

static void *data_image(xml_token * tag, void *data, char *text, int size);
static void *data_fgcolor(xml_token * tag, void *data, char *text, int size);
static void *data_bgcolor(xml_token * tag, void *data, char *text, int size);
static void *data_hotspot(xml_token * tag, void *data, char *text, int size);
static void *data_width(xml_token * tag, void *data, char *text, int size);
static void *data_height(xml_token * tag, void *data, char *text, int size);
static void *data_padding(xml_token * tag, void *data, char *text, int size);

static void *active_state_init(xml_token * tag, void *in);
static void *inactive_state_init(xml_token * tag, void *in);
static void *widget_init(xml_token * tag, void *in);
static void *titlebar_init(xml_token * tag, void *in);
static void *border_init(xml_token * tag, void *in);

#include "themetags.h"

static void *
data_image(xml_token * tag, void *data, char *text, int size)
{
    widget_state_t *state = (widget_state_t *) data;

    state->image = (char *) calloc(size + 1, 1);
    strncpy(state->image, text, size);

    return ((void *) state);
}

static void *
data_fgcolor(xml_token * tag, void *data, char *text, int size)
{
    unsigned long val;

    widget_state_t *state = (widget_state_t *) data;

    xml_parseColor(text, &val, size);

    state->fgcolor = GR_RGB((val >> 16) & 0xFF,
			    (val >> 8) & 0xFF, (val & 0xFF));

    return ((void *) state);
}

static void *
data_bgcolor(xml_token * tag, void *data, char *text, int size)
{

    unsigned long val;
    widget_state_t *state = (widget_state_t *) data;

    xml_parseColor(text, &val, size);

    state->bgcolor = GR_RGB((val >> 16) & 0xFF,
			    (val >> 8) & 0xFF, (val & 0xFF));


    return ((void *) state);
}

static void *
active_state_init(xml_token * tag, void *in)
{
    widget_t *widget = (widget_t *) in;
    widget->states[STATE_ACTIVE].flags = 1;

    return (&widget->states[STATE_ACTIVE]);
}

static void *
inactive_state_init(xml_token * tag, void *in)
{
    widget_t *widget = (widget_t *) in;
    widget->states[STATE_INACTIVE].flags = 1;

    return (&widget->states[STATE_INACTIVE]);
}

static void *
data_hotspot(xml_token * tag, void *data, char *text, int size)
{
    widget_t *widget = (widget_t *) data;
    char *p = text;
    char *tmp;

    /* Figure out where the text ends (hint, its either a < or a \0) */

    while (*p != '<' && *p != 0)
	p++;

    tmp = alloca((int) (p - text) + 2);
    bzero(tmp, (int) (p - text) + 2);

    strncpy(tmp, text, (int) (p - text));

    sscanf(tmp, "%i %i %i %i",
	   &widget->hotspot.x, &widget->hotspot.y,
	   &widget->hotspot.w, &widget->hotspot.h);

    return (data);
}

static void *
data_width(xml_token * tag, void *data, char *text, int size)
{
    widget_t *widget = (widget_t *) data;
    char *p = text;
    char *tmp;

    if (!widget)
	return (0);

    /* Figure out where the text ends (hint, its either a < or a \0) */

    while (*p != '<' && *p != 0)
	p++;

    tmp = alloca((int) (p - text) + 2);
    bzero(tmp, (int) (p - text) + 2);

    strncpy(tmp, text, (int) (p - text));

    /* Parse the string, it should be two integers */
    sscanf(tmp, "%i %i", &widget->width.min, &widget->width.max);

    return (data);
}

static void *
data_height(xml_token * tag, void *data, char *text, int size)
{
    widget_t *widget = (widget_t *) data;

    char *p = text;
    char *tmp;

    if (!widget)
	return (0);

    /* Figure out where the text ends (hint, its either a < or a \0) */

    while (*p != '<' && *p != 0)
	p++;

    tmp = alloca((int) (p - text) + 2);
    bzero(tmp, (int) (p - text) + 2);

    strncpy(tmp, text, (int) (p - text));

    /* Parse the string, it should be two integers */
    sscanf(tmp, "%i %i", &widget->height.min, &widget->height.max);

    return (data);
}

static void *
data_padding(xml_token * tag, void *data, char *text, int size)
{
    theme_t *theme = (theme_t *) data;
    char *p = text;
    char *tmp;

    /* Figure out where the text ends (hint, its either a < or a \0) */
    if (!theme)
	return (0);

    while (*p != '<' && *p != 0)
	p++;

    tmp = alloca((int) (p - text) + 2);
    bzero(tmp, (int) (p - text) + 2);

    strncpy(tmp, text, (int) (p - text));

    sscanf(tmp, "%i %i %i %i",
	   &theme->client.left, &theme->client.right,
	   &theme->client.top, &theme->client.bottom);

    return (data);
}

static void *
widget_init(xml_token * tag, void *in)
{

    component_t *comp = (component_t *) in;
    char **widget_names = widgetList[comp->type];
    xml_prop *prop;

    char *lname = 0;
    int index = 0;

    for (prop = tag->props; prop; prop = prop->next) {
	if (strcmp(prop->keyword, "name") == 0) {
	    lname = prop->value;
	    break;
	}
    }

    if (!lname)
	return (0);

    while (strlen(widget_names[index])) {
	if (strcmp(widget_names[index], lname) == 0)
	    break;
	index++;
    }

    if (!strlen(widget_names[index]))
	return (0);

    /* Actually make the widget */
    comp->widgets[index] = (widget_t *) calloc(sizeof(widget_t), 1);

    /* Init a few of the items */
    comp->widgets[index]->width.min = 0;
    comp->widgets[index]->width.max = -1;

    comp->widgets[index]->height.min = 0;
    comp->widgets[index]->height.max = -1;

    return ((void *) comp->widgets[index]);
}

static void *
titlebar_init(xml_token * tag, void *in)
{

    theme_t *data = (theme_t *) in;
    component_t *comp = &data->components[COMPONENT_TITLEBAR];

    /* Set up the correct number of widgets */

    comp->widgetCount = TITLEBAR_WIDGET_COUNT;
    comp->widgets =
	(widget_t **) calloc(comp->widgetCount * sizeof(widget_t *), 1);
    comp->type = COMPONENT_TITLEBAR;

    return ((void *) comp);
}

static void *
border_init(xml_token * tag, void *in)
{

    component_t *comp;
    theme_t *data = (theme_t *) in;
    int index;

    if (strcmp(tag->tag, "border_left") == 0)
	index = COMPONENT_LEFT;
    else if (strcmp(tag->tag, "border_right") == 0)
	index = COMPONENT_RIGHT;
    else if (strcmp(tag->tag, "border_bottom") == 0)
	index = COMPONENT_BOTTOM;
    else if (strcmp(tag->tag, "border_top") == 0)
	index = COMPONENT_TOP;
    else {
	error("Unknown tag <%s> while parsing the theme file\n", tag->tag);

	return (0);
    }


    comp = &data->components[index];

    /* Set up the correct number of widgets */

    comp->widgetCount = BORDER_WIDGET_COUNT;
    comp->widgets =
	(widget_t **) calloc(comp->widgetCount * sizeof(widget_t *), 1);
    comp->type = index;

    return ((void *) comp);
}

void
freeTheme(theme_t * theme)
{

    int i, w, s;

    /* Go through and check each component */

    for (i = 0; i < THEME_COMPONENT_COUNT; i++) {
	component_t *comp = &theme->components[i];

	if (comp->widgets) {
	    for (w = 0; w < comp->widgetCount; w++)
		if (comp->widgets[w]) {
		    widget_t *widget = comp->widgets[w];

		    for (s = 0; s < 2; s++) {
			if (widget->states[s].image)
			    free(widget->states[s].image);
			if (widget->states[s].imageid)
			    GrDestroyWindow(widget->states[s].imageid);
		    }

		    free(widget);
		    comp->widgets[w] = 0;
		}

	    free(comp->widgets);
	}
    }

    if (theme->font)
	GrDestroyFont(theme->font);
    free(theme);
}

/* Main theme function */

theme_t *
createTheme(const char *directory)
{

    xml_parser engine;
    theme_t *theme;
    int ret;

    char *xmlfile;

    if (!directory)
	return (0);

    xmlfile = alloca(strlen(directory) + strlen("/theme.xml") + 1);
    sprintf(xmlfile, "%s/theme.xml", directory);

    /* Check that we can get to the theme XML file */

    if (access(xmlfile, R_OK)) {
	error("Unable to open the theme definition file '%s'.\n", xmlfile);
	return (0);
    }

    /* Create the theme structure */
    theme = (theme_t *) calloc(sizeof(theme_t), 1);
    if (!theme)
	return (0);

    engine.tags = themeToplevel;

    /* Now, start the parser */
    ret = xml_parseFile(&engine, xmlfile, (void *) theme);

    if (ret) {
	error("Error %d while loading theme '%s'.\n", directory);
	freeTheme(theme);
	return (0);
    }

    theme->font = GrCreateFont(GR_FONT_GUI_VAR, 12, 0);
    return (theme);
}
