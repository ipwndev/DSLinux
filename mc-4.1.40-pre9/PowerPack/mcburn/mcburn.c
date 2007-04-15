// mcburn.c
// Function definitions for cdrecord support in Midnight Commander
// Copyright 2001 Bart Friederichs

/* 
 * TODO for future versions
 * - Beautify the burn dialog so that it looks like all dialogs in mc.
 * - Build in multi-burner/cdrom support
 * - Check for enough drive space for image in $HOME_DIR
 */

#include <config.h>
#include <locale.h>

#include "tty.h"
// inclusions

#include <stdio.h>
#include <stdlib.h>		/* getenv (), rand */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>		/* open, O_RDWR */
#include <errno.h>

#ifdef HAVE_MMAP
#   include <sys/mman.h>
#endif

//#include "mad.h"
#include "util.h"
#include "panel.h"
#include "cmd.h"		/* Our definitions */

#ifdef HAVE_DLGSWITCH
#include "dlglist.h"
#endif

#include "view.h"		/* view() */
#include "dialog.h"		/* query_dialog, message */
#include "file.h"		/* the file operations */
#include "find.h"		/* do_find */
#include "hotlist.h"
#include "tree.h"
#include "subshell.h"		/* use_subshell */
#include "cons.saver.h"
#include "global.h"
#include "dlg.h"		/* required by wtools.h */
#include "widget.h"		/* required by wtools.h */
#include "wtools.h"		/* listbox */
#include "command.h"		/* for input_w */
#include "win.h"		/* do_exit_ca_mode */
#include "layout.h"		/* get_current/other_type */
#include "ext.h"		/* regex_command */
#include "view.h"		/* view */
#include "key.h"		/* get_key_code */
#include "help.h"		/* interactive_display */
#include "fs.h"
#include "boxes.h"		/* cd_dialog */
#include "color.h"
#include "user.h"
#include "setup.h"
#include "x.h"
#include "profile.h"
#include "dir.h"

#define MIDNIGHT

#include "../vfs/vfs.h"

#define WANT_WIDGETS

#include "main.h"		/* global variables, global functions */

#ifndef MAP_FILE
#   define MAP_FILE 0
#endif

////////////////////////////////////

#include "mcburn.h"

#define TOGGLE_VARIABLE 0
#define INPUT_VARIABLE 1

/* Burner options box coords */
#define BX	4
#define BY	2

/* Filesystem option box coords */
#define FY	2

/* widget types */
#define CHECKBOX	1
#define INPUT		2

/* option category */
#define BURNER		1
#define FS			2

// global settings
int interimage = 1;
int dummyrun = 0;
int joliet = 1;
int rockridge = 1;
int multi = 1;
int speed = 2;
int scsi_bus = -1, scsi_id = -1, scsi_lun = -1;
char *cdwriter = NULL;

static int burner_option_width = 0, fs_option_width = 0;
static int FX = 0;
static char *burn_options_title, *burner_title, *fs_title;
static char *burn_title;
static Dlg_head *burn_conf_dlg;
static Dlg_head *burn_dlg;
static char *burndir;
static char *cdrecord_path;
static char *mkisofs_path;
static int burner_options, fs_options;			/* amount of burner and fs options */

/* one struct with all burner settings */
static struct {
	char *text;
	int *variable;
	int type;				/* CHECKBOX, INPUT */
	int category;			/* BURNER, FS */
	WCheck *w_check;
	WInput *w_input;
	char *tk;
	char *description;
	
	/* only applicable for the input widget; shoot me for the overhead */
	int i_length;
} options[] = {
    {N_("make &Intermediate image"),  &interimage, CHECKBOX, BURNER, NULL, NULL, "interimage", "Make intermediate image", 0 },
    {N_("&Dummy run"), &dummyrun, CHECKBOX, BURNER, NULL, NULL, "dummyrun", "Turn the laser off", 0 },
    {N_("&Multisession CD"), &multi, CHECKBOX, BURNER, NULL, NULL, "multi", "Create a multi-session CD", 0 },	
	{N_("Speed"), &speed, INPUT, BURNER, NULL, NULL, "speed", "Speed", 3 },
	{N_("&Joliet extensions"), &joliet, CHECKBOX, FS, NULL, NULL, "joliet", "Use Joliet extensions", 0},
    {N_("&RockRidge extensions"), &rockridge, CHECKBOX, FS, NULL, NULL, "rockridge", "Use RockRidge extensions", 0},
	{0,0,0,0}
};

/* return a string that is a concatenation of s1 and s2 */
char *concatstrings(const char *s1, const char *s2) {
	char *temp = NULL;
	int length = 0;
	
	length = strlen(s1) + strlen(s2) + 1;
	temp = malloc(length*sizeof(char));
	
	strcpy(temp, s1);
	strcat(temp, s2);
	
	return(temp);	
}

/* check for program and return a pointer to a string containing the full path */
char *check_for(char *program) {
	char *command;
	FILE *output;

	char buffer[1024];
	char *fullpath;

	command = malloc( (strlen(program) + 11) * sizeof(char));
	strcpy(command, "which ");
	strcat(command, program);
	strcat(command, " 2>&1");

	if (!(output = popen(command, "r")) )
	{
			message (0, "Error ", "An error occurred checking for program (popen failed)");
			return NULL;
	}

	while (!feof(output))
			fgets(buffer, 1024, output);

	/* remove newline from buffer */
	buffer[strlen(buffer)-1] = '\0';

	/* not starting with '/' means it is not found */
	if (buffer[0] != '/')
		return NULL;

	fullpath = malloc( (strlen(buffer)+1)	* sizeof(char) );
	strncpy(fullpath, buffer, strlen(buffer)+1);

  pclose(output);
	return fullpath;
}

/* this lets the user choose a dir and burn that dir, with the
   current options */
void do_burn (WPanel *panel)
{
		char buffer[1024];

		// make sure cdrecord and mkisofs is available, and get their full path
		if (!(mkisofs_path = check_for("mkisofs")))
		{
     		message (0, " Error ", "Couldn't find mkisofs");
				return;
		}

		if (!(cdrecord_path = check_for("cdrecord")))
		{
				message (0, " Error ", "Couldn't find cdrecord");
				return;
		}

		if (!strcmp(cpanel->dir.list[cpanel->selected].fname, ".."))
		{
			message(0, " Error ", "You can't burn the parent-directory");
			return;
		}

    burndir = concat_dir_and_file(cpanel->cwd, cpanel->dir.list[cpanel->selected].fname);

    if (!S_ISDIR(cpanel->dir.list[cpanel->selected].buf.st_mode))
    {
			message(0, " Error ", "You can't burn a single file to CD");
			return;
    }
    else
    {
			if (!scan_for_recorder(cdrecord_path)) {
				sprintf(buffer, "No CD-Writer found");
				message(0, " Error ", buffer);
				return;
			} 

			init_burn();					/* initialize the burn dialog */
			run_dlg(burn_dlg);				/* run the dialog */
			destroy_dlg(burn_dlg);			/* and throw it away after usage */
			
			if (burn_dlg->ret_value == B_ENTER) {
				/* here, the actual burning takes place 
				 * construct a (series of) command(s) to execute 
				 */
				
				sprintf(buffer, "echo \"Burning %s to CD ...\"", burndir);
				shell_execute(buffer,EXECUTE_INTERNAL);
				
				/** continue here
				 ** 1. make a mkisofs command
				 ** 2. make a cdrecord command
				 ** 3. pipe them if necessary (!interimage)
				 ** 4. run consecutively if necessary (interimage)
				 ** NOTE: dummyrun is NOT before writing, its just a dummy run (nice for testing purposes)
				 **/
				
				/* STEP 1: create an image if the user wants to. Put the image in $HOMEDIR
				 * it's the user's responsibility to make sure the is enough room (TODO 3)
				 * this is where the fs-options come in, using -r for RockRidge and -J for Joliet extensions
				 */
				if (interimage) {
					sprintf(buffer, "echo \"Building image...\"");
					shell_execute(buffer, EXECUTE_INTERNAL);
					strcpy(buffer, mkisofs_path);
					if (rockridge) strcat(buffer, " -r");
					if (joliet) strcat(buffer, " -J");
					strcat(buffer, " -o ");
					strcat(buffer, home_dir);
					strcat(buffer, "/mcburn.iso ");
					strcat(buffer, burndir);
					shell_execute(buffer, EXECUTE_INTERNAL);
				}
				
				/* STEP 2: create a cdrecord command, this is where speed, dummy, multi and the scsi_* vars come in
				 * also, check for an image or pipe it right into cdrecord
				 *
				 * STEP 2b: cdrecord without a pipe (assume the $HOME/mcburn.iso exists (TODO 4))
				 */
				if (interimage) {
					sprintf(buffer, "echo \"Burning CD...\"");
					shell_execute(buffer, EXECUTE_INTERNAL);
					strcpy (buffer, cdrecord_path);
					if (dummyrun) strcat(buffer, " -dummy");
					if (multi) strcat(buffer, " -multi");
					sprintf(buffer, "%s -v speed=%d", buffer, speed);
					sprintf(buffer, "%s dev=%d,%d,%d", buffer, scsi_bus, scsi_id, scsi_lun);
					sprintf(buffer, "%s -data %s/mcburn.iso", buffer, home_dir);
					
				} else {		/* no image present, pipe mkisofs into cdrecord */
					/* first get the size of the image to build */
					FILE *pipe;
					int imagesize;
					strcpy(buffer, "mkisofs -R -q -print-size private_collection/  2>&1 | sed -e \"s/.* = //\""); 
					pipe = popen(buffer, "r");
					fgets(buffer, 1024, pipe);
					imagesize = atoi(buffer);
					pclose(pipe);
					
					sprintf(buffer, "[ \"0%d\" -ne 0 ] && %s %s %s %s | %s %s %s speed=%d dev=%d,%d,%d -data -", 
						imagesize, mkisofs_path, rockridge?" -r":"", joliet?" -J":"", burndir, cdrecord_path, dummyrun?" -dummy":"", multi?" -multi":"", 
						speed, scsi_bus, scsi_id, scsi_lun);
				}
				
				/* execute the burn command */
				shell_execute(buffer, EXECUTE_INTERNAL);
				
			}
	}
}

/* scan for CD recorder 
   This functions executes 'cdrecord -scanbus' and checks the output for 'CD-ROM'. It gets the first occurrence.
   In the future, it should give all available CD-ROMs so that the user can choose from them
   Also, the bus, id and lun received from the line can be wrong.

   return 1 if a recorder is found, 0 otherwise
*/
int scan_for_recorder (char *cdrecord_command)
{
	char *command;
	char buffer[1024];
	FILE *output;

	command = calloc( (strlen(cdrecord_command)+15), sizeof(char) );
	strncpy(command, cdrecord_command, strlen(cdrecord_command));
	strcat(command, " -scanbus 2>&1");

	if (!(output = popen(command, "r")) )
	{
			message (0, "Error ", "An error occurred scanning for writers (popen failed)");
			return 0;
	}

	while (!feof(output)) {
			int i = -1;

			fgets(buffer, 1024, output);

			/* remove newline from buffer */
			buffer[strlen(buffer)-1] = '\0';

			for (i=0; i < strlen(buffer); i++)
				if (buffer[i] == '\'') break;

			/* parse all lines from 'cdrecord -scanbus'
         and select the first CD-ROM */
      	if (buffer[0] == '\t' && strstr(buffer, "CD-ROM")) {
				/* this is a scsi cdrom player in this line */
				scsi_bus = buffer[1] - 48;
				scsi_id = buffer[3] - 48;
				scsi_lun = buffer[5] - 48;

				/* free the memory first, before allocating new */
				if (cdwriter) free(cdwriter);
				cdwriter = calloc(26, sizeof(char));
				strncpy(cdwriter, &buffer[i+1], 8);
				strcat(cdwriter, " ");
				strncat(cdwriter, &buffer[i+12], 16);

				break;										/* remove this to select _last_ occurence, i should make a menu where the user can choose */
		}
	}

	free(command);
	pclose(output);
	
	/* if the scsi_* vars are still -1, no recorder was found */
	if (scsi_lun == -1) return 0;
	else return 1;
}

/* the burn dialog box message handler 
 * the burn dialog could use some better layout
 */
static int burn_callback (struct Dlg_head *h, int Id, int Msg)
{
	int i, line=4;
	char buffer[1025];
	
    switch (Msg) {
    case DLG_DRAW:
#ifndef HAVE_X
			attrset (COLOR_NORMAL);
			dlg_erase (h);
			draw_box (h, 0, 0, h->lines, h->cols);

			dlg_move (h, 1, 1);
			addstr(_("directory:"));
	
			attrset (COLOR_YELLOW);
			dlg_move (h, 1, 11);
			addstr(burndir);
			attrset (COLOR_NORMAL);
			
			dlg_move (h, 3, 1);
			addstr(_("settings:"));

			// run through all options
			for (i = 0; options[i].tk; i++)
			{
				switch (options[i].type) {
					case CHECKBOX:
						if (*options[i].variable == 1)
						{
							dlg_move(h, line, 1);
							addstr(_("- "));
							dlg_move(h, line++, 4);
							addstr(options[i].description);
						}
						break;
					case INPUT:
						dlg_move(h, line, 1);
						addstr(options[i].description);
						dlg_move(h, line++, strlen(options[i].description) + 2);
						sprintf(buffer, "%d", *options[i].variable);
						addstr(buffer);
						break;
					default:
						break;
				}
			}
			
#endif
			break;

    case DLG_END:
			break;
    }

    return 0;
}

/* this initializes the burn dialog box
   there will be no way back after OK'ing this one */
void init_burn ()
{
	int i;

	int dialog_height = 0, dialog_width = 27;


    /* button titles */
    char* burn_button = _("&Burn");
    char* cancel_button = _("&Cancel");


//    message (NULL,"!!!!","!!!!");

	/* we need the height and width of the dialog
		 that depends on the settings */
	for (i = 0; options[i].tk; i++)
		switch (options[i].type) {
			case CHECKBOX:
    			if (*options[i].variable == 1) dialog_height++;		/* just print the setting when it is checked */
				break;
			case INPUT:
				dialog_height++;				/* these are always printed */
				break;
			default:
				break;
		}

	dialog_height += 7;

//    message (NULL, strdup(burndir), "!!!!");

	if (strlen(burndir) + 12 > 27) 
	    dialog_width = strlen(burndir) + 12;

//	    dialog_width = 30;

    burn_dlg = create_dlg(0,0,dialog_height,dialog_width, dialog_colors, burn_callback,
			"CD Burn", "burn",
			DLG_CENTER | DLG_GRID);

    x_set_dialog_title(burn_dlg, _("Burn directory to CD"));

    /* add the Burn and Cancel buttons */
    add_widget (burn_dlg, button_new(dialog_height-2,1,B_ENTER, DEFPUSH_BUTTON,
		    burn_button, 0, 0, "button-burn"));

    add_widget (burn_dlg, button_new(dialog_height-2,12,B_CANCEL, NORMAL_BUTTON,
		    cancel_button, 0, 0, "button-cancel"));

}

/****************************************************************************************************************************************************
  
															  MC-Burn options functions

****************************************************************************************************************************************************/

/* this shows the burn options dialog */
void burn_config ()
{
    int result, i;

    init_burn_config();

    run_dlg(burn_conf_dlg);

    /* they pushed the OK or Save button, set the variables right */
    result = burn_conf_dlg->ret_value;
	
	if (result == B_ENTER || result == B_EXIT) 
		for (i = 0; options[i].tk; i++)
			switch (options[i].type) {
				case CHECKBOX:
	    			if (options[i].w_check->state & C_CHANGE) 
		    			*options[i].variable = !(*options[i].variable);
					break;
				case INPUT:
					*options[i].variable = atoi(options[i].w_input->buffer);
					break;
			}

	/* If they pressed the save button, save the values to ~/.mc/mcburn.conf */
    if (result == B_EXIT){
		save_mcburn_settings();
    }

    destroy_dlg(burn_conf_dlg);
}

/* the options dialog box message handler */
static int burn_options_callback (struct Dlg_head *h, int Id, int Msg)
{
    switch (Msg) {
    case DLG_DRAW:
		attrset (COLOR_NORMAL);
		dlg_erase (h);

		/* all around the dialog box box */
		draw_box (h, 1, 2, h->lines - 2, h->cols - 4);

		/* option boxes */
		draw_box (h, BY, BX, burner_options + 2, burner_option_width);
		draw_box (h, FY, FX, fs_options + 2, fs_option_width);

		/* titles */
		dlg_move (h, 1, (h->cols - strlen(burn_options_title))/2);
		addstr (burn_options_title);
		dlg_move (h, BY, BX+2);
		addstr (burner_title);
		dlg_move (h, FY, FX+2);
		addstr (fs_title);

		break;

    case DLG_END:
//	r_but = Id;
		break;
    }
    return 0;
}



/* this initializes the burn options dialog box */
void init_burn_config ()
{
    int i = 0;
    static int dialog_height = 0, dialog_width = 0;
    static int b1, b2, b3;
	
    /* button title */
    char* ok_button = _("&Ok");
    char* cancel_button = _("&Cancel");
    char* save_button = _("&Save");
	register int l1;

	burner_options = 0;
	fs_options = 0;	
	
	/* count the amount of burner and fs options */
	for (i=0; options[i].tk; i++) 
		switch (options[i].category) {
			case BURNER: 
				burner_options++;
				break;
			case FS:
				fs_options++;
				break;
		}
	
	/* similar code is in options.c */
	burn_options_title = _(" CD-Burn options ");
	burner_title = _(" Burner options ");
	fs_title = _(" Filesystem options ");

	/* get the widths for the burner options and the fs options */
	burner_option_width = strlen(burner_title) + 1;
	fs_option_width = strlen(fs_title) + 1;
	for (i = 0; options[i].tk; i++)
	{
		/* make sure the whole inputfield width is accounted for */
		if (options[i].type == INPUT) l1 = options[i].i_length;
		else l1 = 0;
		
		/* calculate longest width of text */
		if (options[i].category == BURNER) {
			options[i].text = _(options[i].text);
			l1 += strlen(options[i].text) + 7;
			if (l1 > burner_option_width)
				burner_option_width = l1;
		}
		
		if (options[i].category == FS) {
			options[i].text = _(options[i].text);
			l1 += strlen(options[i].text) + 7;
			if (l1 > fs_option_width)
				fs_option_width = l1;
		}
			
	}
	
	l1 = 11 + strlen (ok_button)
	 	+ strlen (save_button)
		+ strlen (cancel_button);

	i = (burner_option_width + fs_option_width - l1) / 4;
	b1 = 5 + i;
	b2 = b1 + strlen(ok_button) + i + 6;
	b3 = b2 + strlen(save_button) + i + 4;

    dialog_width = burner_option_width + fs_option_width + 7;
    FX = FY + burner_option_width + 2;

    /* figure out the height for the burn options dialog */
    if (burner_options > fs_options)
		dialog_height = burner_options + 7;
    else
		dialog_height = fs_options + 7;

    burn_conf_dlg = create_dlg(0,0,dialog_height,dialog_width, dialog_colors, burn_options_callback,
			"CD Burn options", "burnconf", DLG_CENTER | DLG_GRID);
    x_set_dialog_title(burn_conf_dlg, _("Burner options"));

    /* add the OK, Cancel and Save buttons */
    add_widget (burn_conf_dlg, button_new(dialog_height-3,b1,B_ENTER, DEFPUSH_BUTTON,
		    ok_button, 0, 0, "button-ok"));

    add_widget (burn_conf_dlg, button_new(dialog_height-3,b3,B_CANCEL, NORMAL_BUTTON,
		    cancel_button, 0, 0, "button-cancel"));

    add_widget (burn_conf_dlg, button_new(dialog_height-3,b2,B_EXIT, NORMAL_BUTTON,
		    save_button, 0, 0, "button-save"));


    /* add the burner options */
	for (i = 0; options[i].tk; i++) {
		int x, y;
		char *defvalue;
		
		/* first find out where the widget should go (burner option or fs option) */
		switch (options[i].category) {
			case BURNER:
				x = BX + 1;
				y = BY + 1 + i;
				break;
			case FS:
				x = FX + 1;
				y = FY - burner_options + 1 + i;
				break;
			default:
				break;
		}

		/* then create a new widget, depending on the type 
		 * afterwards, add it to the dialog box 				*/
		switch (options[i].type) {
			case CHECKBOX:
				options[i].w_check = check_new(y, x, options[i].variable, options[i].text, options[i].tk);					/* here, the values are taken from the variables and put in the checkboxes */
				add_widget(burn_conf_dlg, options[i].w_check);
				break;
			case INPUT:
				defvalue = malloc((options[i].i_length + 1) * sizeof(char));	
				snprintf(defvalue, options[i].i_length, "%d", *options[i].variable);
			
				add_widget(burn_conf_dlg, label_new(y, x, options[i].text, options[i].tk));							/* a label */				
				options[i].w_input = input_new(y, x + strlen(options[i].text) + 1, 0, options[i].i_length, defvalue, options[i].tk);	/* and the input widget behind it */
				add_widget(burn_conf_dlg, options[i].w_input);
				break;
			default:
				break;
		}
	}

}

/****************************************************************************************************************************************************
  
															  MC-Burn configfile functions

****************************************************************************************************************************************************/

/* this loads the settings from ~/.mc/mcburn.conf */
void load_mcburn_settings ()
{
	FILE *mcburn_settings_file;
	char *filename;
	int i, setting;
	char buff[128];

	filename = malloc(strlen(home_dir)+strlen(configfile)+1);
	strcpy (filename, home_dir);
	strcat (filename, configfile);

	printf("Loading CD Burn extension settings...");

	if ((mcburn_settings_file = fopen(filename, "r")) == NULL)
	{
		printf("failed\n");
	}
	else
	{
		while (!feof(mcburn_settings_file))
		{
			fscanf(mcburn_settings_file, "%s %d", buff, &setting);

			for (i = 0; options[i].tk; i++)
			{
			 	if (!strcmp(options[i].tk, buff))
				{
					*options[i].variable = setting;
				}
			}
		}
		fclose(mcburn_settings_file);
		printf("done\n");
	}
}

/* this saves the settings to ~/.mc/mcburn.conf */
void save_mcburn_settings ()
{
	FILE *mcburn_settings_file;
	int i;
	char *filename;

	filename = malloc(strlen(home_dir)+strlen(configfile)+1);
	strcpy (filename, home_dir);
	strcat (filename, configfile);

	if ((mcburn_settings_file = fopen(filename, "w")) == NULL)
	{
		message(0, " Save failed ", filename);
	}
	else
	{
		/* first, print a message not to tamper with the file */
		fprintf(mcburn_settings_file, "# This file is generated by MC-Burn. DO NOT EDIT!\n");
		for (i = 0; options[i].tk; i++)
		{
		 	fprintf(mcburn_settings_file, "%s %d\n", options[i].tk,	*options[i].variable);
		}
		fclose(mcburn_settings_file);
	}
}

