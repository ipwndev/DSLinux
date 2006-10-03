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
#include <string.h>

extern "C"
{
#include <par/pardb.h>
}


#include "settings.h"

const char *defaultKeywords[] = {
    "server", "port", "username", "password",
    "smtpserver", "smtpname"
};

const char *defaultValues[] = {
    "localhost", "110", "", "",
    "localhost", ""
};

const char *defaultAccounts[] = { "Work", "Home", "Other" };

void
mailSettings::add_new_account(char *name)
{
    struct setAccount *a;

    if (_accounts == 0) {
	_accounts = new struct setAccount;
	a = _accounts;
    } else {
	a = _accounts;
	while (a->next)
	    a = a->next;
	a->next = new struct setAccount;
	a = a->next;
    }

    bzero(a, sizeof(struct setAccount));

    for (int i = 0; i < _count; i++) {
	strncpy(a->values[i], defaultValues[i], 30);
	a->saved[i] = 1;
    }

    strcpy(a->name, name);
    a->next = 0;

    _acount++;
}

struct setAccount *
mailSettings::get_account(char *name)
{
    struct setAccount *a = _accounts;

    while (a) {
	if (!strcmp(name, a->name))
	    return (a);

	a = a->next;
    }

    return (0);
}

void
mailSettings::set_temp_value(char *account, char *keyword, const char *value)
{
    int i;
    struct setAccount *a = get_account(account);

    if (!a)
	return;

    for (i = 0; i < _count; i++) {
	if (!strncmp(keyword, keywords[i], strlen(keywords[i]))) {
	    bzero(a->tempvalues[i], 31);

	    if (strlen(value))
		strncpy(a->tempvalues[i], value, 30);

	    a->saved[i] = 0;
	}
    }
}

int
mailSettings::confirm_account_changes(char *account)
{
    int i;
    int changed = 0;
    struct setAccount *a = get_account(account);

    if (!a)
	return (0);

    for (i = 0; i < _count; i++) {
	if (a->saved[i])
	    continue;

	changed |= set_value(account, keywords[i], a->tempvalues[i]);
	strcpy(a->tempvalues[i], "");
    }

    return (changed);
}

void
mailSettings::reject_account_changes(char *account)
{
    int i;
    struct setAccount *a = get_account(account);

    if (!a)
	return;

    for (i = 0; i < _count; i++) {
	strcpy(a->tempvalues[i], "");
	a->saved[i] = 1;
    }
}

void
mailSettings::reject_all_changes()
{
    int i;

    for (i = 0; i < _acount; i++)
	reject_account_changes(get_account_name(i));
}

int
mailSettings::confirm_all_changes()
{
    int i;
    int changed = 0;

    for (i = 0; i < _acount; i++)
	changed |= confirm_account_changes(get_account_name(i));

    return (changed);
}

int
mailSettings::set_value(char *account, char *keyword, const char *value)
{
    int i;
    struct setAccount *a = get_account(account);

    if (!a)
	return (0);

    for (i = 0; i < _count; i++) {
	if (!strncmp(keyword, keywords[i], strlen(keywords[i]))) {
	    /* Don't do anything if the setting hasn't changed */
	    a->saved[i] = 1;

	    if (strcmp(a->values[i], value)) {
		bzero(a->values[i], 31);
		strncpy(a->values[i], value, 30);
		return (1);
	    } else
		return (0);	/* No change */
	}
    }

    return (0);
}

char *
mailSettings::get_value(char *account, char *keyword)
{
    int i;

    struct setAccount *a = get_account(account);
    if (!a)
	return (0);

    for (i = 0; i < _count; i++) {
	if (!strncmp(keyword, keywords[i], strlen(keywords[i]))) {
	    if (a->saved[i] == 0)
		return (a->tempvalues[i]);
	    else
		return (a->values[i]);
	}
    }

    return (0);
}

int
mailSettings::load_settings(void)
{

    char parent[32];
    char child[64];

    int ret, sret, err;

    char *pardb = db_getDefaultDB();
    db_handle *handle = 0;

    if (!pardb) {
	fprintf(stderr, "ERROR:  Couldn't open the PAR database\n");
	return -1;
    }

    handle = db_openDB(pardb, PAR_DB_MODE_RDONLY);
    if (!handle) {
	fprintf(stderr, "ERROR:  Unable to read the database (%d)\n",
		pardb_errno);
	return -1;
    }

    /* Load each account */

    snprintf(parent, sizeof(parent) - 1, "nanomail.accounts");

    bzero(child, sizeof(child));

    ret = db_getFirstChild(handle, parent, child, sizeof(child) - 1);

    while (ret > 0) {
	char node[84];
	char account[64];
	char setting[64];
	char next[128];

	sprintf(node, "%s.%s.name", parent, child);

	bzero(account, sizeof(account));

	err = db_findNode(handle, node, &account, sizeof(account - 1), 0);

	if (err == -1) {
	    printf("Couldn't get the name for %s (%d)\n", node, pardb_errno);
	    goto next_account;
	}

	printf("Loaded account [%s]\n", account);

	if (!get_account(account))
	    add_new_account(account);

	/* Now, get the keywords */
	snprintf(node, sizeof(node) - 1, "%s.%s.settings", parent, child);

	bzero(setting, sizeof(setting));

	printf("Looking for children in %s\n", node);

	sret = db_getFirstChild(handle, node, setting, sizeof(setting) - 1);

	printf("got %d from db_getFirstChild\n", sret);

	while (sret > 0) {
	    char key[128];

	    char buffer[64];

	    sprintf(key, "%s.%s", node, setting);

	    err = db_findNode(handle, key, buffer, sizeof(buffer) - 1, 0);

	    if (err == -1) {
		printf("Couldn't get the keyword %s (%d)\n", key,
		       pardb_errno);
		goto next_keyword;
	    }

	    printf("Loaded keyword [%s] = [%s]\n", setting, buffer);

	    set_value(account, setting, buffer);

	  next_keyword:
	    strcpy(next, setting);
	    bzero(setting, sizeof(setting));

	    printf("Getting next peer of %s\n", next);

	    sret =
		db_getNextChild(handle, node, next, setting,
				sizeof(setting) - 1);
	    printf("Returned %d\n", sret);
	}

      next_account:
	strcpy(next, setting);
	bzero(child, sizeof(child));
	ret = db_getNextChild(handle, parent, next, child, sizeof(child) - 1);
    }

    db_closeDB(handle);
    return 0;
}

int
mailSettings::save_settings(void)
{
    struct setAccount *a;
    int acnt = 0;

    char *pardb = db_getDefaultDB();
    db_handle *handle = 0;

    if (!pardb) {
	fprintf(stderr, "ERROR:  Couldn't open the PAR database\n");
	return -1;
    }

    handle = db_openDB(pardb, PAR_DB_MODE_RW);

    if (!handle) {
	fprintf(stderr, "ERROR:  Unable to read the database (%d)\n",
		pardb_errno);
	return -1;
    }

    /* Save each account */

    for (a = _accounts; a; a = a->next, acnt++) {
	char name[128];

	sprintf(name, "nanomail.accounts.nxmail%d.name", acnt);

	if (db_addNode(handle, name, a->name, strlen(a->name), PAR_TEXT) ==
	    -1) {
	    fprintf(stderr, "ERROR - couldn't write account %s (%d)\n", name,
		    pardb_errno);
	    continue;
	}

	printf("Saved account [%s] to %s\n", a->name, name);

	for (int i = 0; i < _count; i++) {
	    sprintf(name, "nanomail.accounts.nxmail%d.settings.%s", acnt,
		    keywords[i]);

	    if (!a->values[i])
		continue;
	    if (!strlen(a->values[i]))
		continue;

	    printf("Saving [%s] {%d}\n", a->values[i], strlen(a->values[i]));

	    if (db_addNode
		(handle, name, a->values[i], strlen(a->values[i]),
		 PAR_TEXT) == -1) {
		fprintf(stderr, "ERROR - couldn't add keyword %s (%d)\n",
			name, pardb_errno);
		continue;
	    }
	}
    }

    db_closeDB(handle);
    return 0;
}

char *
mailSettings::get_account_name(int num)
{
    struct setAccount *a = _accounts;
    int i;

    for (i = 0; i < num; i++) {
	if (!a)
	    return (0);
	a = a->next;
    }

    if (!a)
	return (0);
    return (a->name);
}

mailSettings::mailSettings(void)
{
    /* Set the defaults */
    _accounts = 0;
    _acount = 0;

    _count = 6;

    for (int i = 0; i < _count; i++)
	strncpy(keywords[i], defaultKeywords[i], 30);

    /* Add the default accounts */

    for (int a = 0; a < 3; a++)
	add_new_account((char *) defaultAccounts[a]);
}
