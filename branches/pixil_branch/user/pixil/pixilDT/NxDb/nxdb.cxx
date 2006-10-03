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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "nxdb.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <stdlib.h>
#endif

#ifdef WIN32
#include <iostream>
#include <direct.h>
#include "../getopt/getopt.h"
#include "VCMemoryLeak.h"
#endif

#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#endif

#define OPTIONS "p:k:d:m:"
char *p_arg = 0;

NxDb::NxDb(int argc, char *argv[])
{
    int c;
    path = ".";
    while ((c = getopt(argc, argv, OPTIONS)) != -1) {
	switch (c) {
	case 'p':
	    path = optarg;

	    /*
	       p_arg = optarg;
	       struct stat dir;
	       if ( stat(p_arg, &dir) == 0) {
	       path = p_arg;
	       }
	     */
	    break;
	default:
	    break;
	}			// switch
    }				// while
    dbNum = 0;
    dbinit(0);
}

int
NxDb::Open(string _dbName, fildes * _dbDesc,
	   field * _dbField, int var, int path_flag)
{

    Register(_dbName, _dbDesc, _dbField, var);

    // Open Database
    string path_dbName;
    if (path_flag)
	path_dbName = _dbName;
    else {
	int len = path.length() - 1;
	if (path[len] != '/' && _dbName[0] != '/')
	    path += '/';
	path_dbName = path + _dbName;
    }
    struct stat dir;
    if (0 != stat(path.c_str(), &dir)) {

#ifndef WIN32
	mkdir(path.c_str(), S_IRWXU);
	
#else /* 
 */
	_mkdir(path.c_str());
	
#endif
    }
    if (!dbopen(_dbDesc, (char *) path_dbName.c_str())) {
	fprintf(stderr, "(1) Unable to open:[%s.%s]\n", path_dbName.c_str(),
		_dbDesc->filext);
	return 0;
    } else {
	fprintf(stderr, "Open DB: [%s] fp: %d\n", path_dbName.c_str(),
		_dbDesc->fildesc);
    }
    return 1;
}

int
NxDb::Close(string _dbName)
{

    int dbNumber = db[_dbName];
    fildes *_dbDesc = dbDesc[dbNumber];
    
dbclose(_dbDesc);
    
#ifdef WIN32
	return (0);
    
#endif
}

void
NxDb::Register(string _dbName, fildes * _dbDesc, field * _dbField, int var)
{
    TDatabase::iterator p = db.find(_dbName);

    if (p == db.end()) {
	db.insert(TValue(_dbName, dbNum));
	dbNum++;
    }

    TDatabase::iterator result = db.find(_dbName);

    if (result == db.end()) {
	exit(-1);
    } else {
	int dbNumber = db[_dbName];
	dbDesc[dbNumber] = _dbDesc;
	dbField[dbNumber] = _dbField;
    }
}

int
NxDb::Create(string _dbName, fildes * _dbDesc, field * _dbField, int var,
	     int path_flag)
{
    string path_dbName;
    if (path_flag)
	path_dbName = _dbName;
    else
	path_dbName = path + _dbName;
    if (!dbcreat(_dbDesc, (char *) path_dbName.c_str())) {
	cerr << "NxDb::Could not create: " << path_dbName << _dbDesc->
	    filext << endl;
	exit(-1);
    }
    Register(_dbName, _dbDesc, _dbField, var);

    //  use(0, NULL, NULL);
    Close(_dbName);
    return 1;
}

int
NxDb::SetFlags(const string _dbName, const int &recno, const int &flags)
{

#ifdef DEBUG
    //assert(db.find(_dbName) != db.end());

#endif /*  */
    char buf[MAXRECSIZ];
    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    int ok = getrec(recno, fp, &buf[0]);
    if (ok)
	fp->flags = flags;

    else
	return -1;
    ok = putrec(recno, fp, &buf[0]);
    dbsave(fp);
    if (ok)
	return 0;

    else
	return -2;
}

int
NxDb::GetFlags(const string _dbName, const int &recno, int &flags)
{

#ifdef DEBUG
    //assert(db.find(_dbName) != db.end());

#endif /*  */
    char buf[MAXRECSIZ];
    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    int ok = getrec(recno, fp, &buf[0]);

    //if (ok)
    flags = fp->flags;

    //cout << "GetFlags(): fp->flags = " << fp->flags << endl;
    //else
    //      return -1;
    return ok;
}

void
NxDb::Insert(string _dbName, char *record)
{

    cerr << "Insert\n";

    int dbNumber = db[_dbName];
    fildes *fp = dbDesc[dbNumber];
    int recno = APPEND;
    char ret_buf[MAXRECSIZ];


    for (int i = 1; i <= fp->nrecs; i++) {
	getrec(i, fp, ret_buf);
	if (recerased(fp)) {
	    break;
	}
    }
    fp->flags = NEW;
    putrec(recno, fp, record);

    //cout << "Insert(): recno = " << fp->currec << endl;
    dbsave(fp);
}

void
NxDb::Insert(string _dbName, char *record, int &rec)
{

#ifdef DEBUG
    //assert(db.find(_dbName) != db.end());

#endif /*  */
    int dbNumber = db[_dbName];
    fildes *fp = dbDesc[dbNumber];
    int recno = APPEND;
    char ret_buf[MAXRECSIZ];

    // look for an empty spot
    for (int i = 1; i <= fp->nrecs; i++) {
	getrec(i, fp, ret_buf);
	if (recerased(fp)) {
	    break;
	}
    }
    fp->flags = NEW;
    putrec(recno, fp, record);

    //cout << "fp->currec = " << fp->currec << endl;
    rec = fp->currec;
    dbsave(fp);
}

void
NxDb::Edit(string _dbName, int recno, char *record)
{

    cerr << "Edit." << endl;

    int dbNumber = db[_dbName];
    fildes *fp = dbDesc[dbNumber];

    // If record has NEW bit set, keep the new bit
    // or else set the bit to changed
    if (!(fp->flags & NEW))
	fp->flags = CHANGED;

    //cout << "Edit() recno = " << recno << endl;
    putrec(recno, fp, record);
    dbsave(fp);
}

void
NxDb::DeleteRec(string _dbName, int recno)
{

    int dbNumber = db[_dbName];
    fildes *fp = dbDesc[dbNumber];
    char ret_buf[MAXRECSIZ];
    getrec(recno, fp, ret_buf);

    // If a NEW occurs and then is immediately
    // delete, it would send the deleted record
    // across during a sync, which the other side
    // would not know about. Therefore, ERASE instead
    // of DELETED.
    //////////////////////////////////////////////////
    if (fp->flags & NxDb::NEW)
	fp->flags = NxDb::ERASED;

    else
	fp->flags = NxDb::DELETED;

    //cout << "DeleteRec(): fp->flags = " << fp->flags << endl;
    putrec(recno, fp, ret_buf);

    //fp->flags = 0;
    dbsave(fp);
}

void
NxDb::EraseRec(string _dbName, int recno)
{

#ifdef DEBUG
    //assert(db.find(_dbName) != db.end());

#endif /*  */
    int dbNumber = db[_dbName];
    fildes *fp = dbDesc[dbNumber];
    char ret_buf[MAXRECSIZ];
    getrec(recno, fp, ret_buf);
    fp->flags = NxDb::ERASED;
    putrec(recno, fp, ret_buf);
    fp->flags = 0;
    dbsave(fp);
}

int
NxDb::NumRecs(string _dbName)
{

    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    return fp->nrecs;
}

int
NxDb::Select(string _dbName, int ret[], int ret_size, bool bDeleteFlag,
	     int flags)
{

    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    char ret_buf[MAXRECSIZ];

    int idx = 0;
    for (int i = 1; i <= fp->nrecs; i++) {
	getrec(i, fp, ret_buf);
	if (flags != -1) {

	    //cout << "fp->flags = " << fp->flags << endl;
	    //cout << "flags = " << flags << endl;
	    if (!(fp->flags & flags))
		continue;
	} else {
	    if (fp->flags & NxDb::ERASED ||	// Skip erase records
		(fp->flags & NxDb::DELETED && !bDeleteFlag)) {	// Skip logically deleted records if bDeleteFlag not set
		continue;
	    }
	}
	ret[idx] = i;
	idx++;
    }
    return idx;
}

int
NxDb::Select(string _dbName, char *value, int fieldNo, int ret[],
	     int ret_size, bool bDeleteFlag)
{

    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    field *_dbField = dbField[dbNumber];
    char ret_buf[MAXRECSIZ];
    int idx = 0;
    char type;
    type = _dbField[fieldNo].type;

    for (int i = 1; i <= fp->nrecs; i++) {
	if (idx == ret_size) {
	    return idx;
	}
	memset(ret_buf, 0, MAXRECSIZ);
	getrec(i, fp, ret_buf);

	//cout << "\tSelect(): recno = " << i << endl;
	if (recdeleted(fp) && !bDeleteFlag) {

	    //cout << "recdeleted()\n";
	    continue;
	}
	if (type == 'c') {
	    if (strcmp(value, &ret_buf[_dbField[fieldNo].offset]) == 0) {
		ret[idx] = i;
		idx++;
	    }
	} else if ((type == 'd') || (type == 'i')) {
	    int num = get16(&ret_buf[_dbField[fieldNo].offset]);
	    if (num == atoi(value)) {
		ret[idx] = i;
		idx++;
	    }
	} else if (type == 'l') {
	    long num = get32(&ret_buf[_dbField[fieldNo].offset]);
	    if (num == strtol(value, NULL, 10)) {
		ret[idx] = i;
		idx++;
	    }
	} else {
	    return idx;
	}
    }
    return idx;
}

int
NxDb::Extract(string _dbName, int recno, char *ret_buf)
{
    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    return getrec(recno, fp, ret_buf);
}

int
NxDb::Extract(string _dbName, int recno, int fieldNo, char *ret_buf)
{

    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    field *_dbField = dbField[dbNumber];
    char type;
    char rec_buf[MAXRECSIZ];
    int ival = 0;
    long lval = 0;
    type = _dbField[fieldNo].type;
    if (getrec(recno, fp, rec_buf) == 0)
	return 0;
    switch (type) {
    case 'c':{

	    //cout << "Extract(): c\n";
	    //int size = int(_dbField[fieldNo].size);
	    //size = size - 1;
	    //cerr << "Extract(): size = " << size << endl;
	    //memset(ret_buf, 0, size);
	    strcpy(ret_buf, &rec_buf[_dbField[fieldNo].offset]);

	    //ret_buf[size] = 0;
	    return 1;
	}
    case 'd':
    case 'i':
	ival = get16(&rec_buf[_dbField[fieldNo].offset]);

	//cout << "Extract(): i = " << ival << endl;
	sprintf(ret_buf, "%d", ival);
	return 1;
    case 'l':
	lval = get32(&rec_buf[_dbField[fieldNo].offset]);

	//cout << "Extract(): l\n";
	sprintf(ret_buf, "%ld", lval);
	return 1;
    default:
	break;
    }
    return 0;
}

int
NxDb::ExtractDeleted(string _dbName, int recno, int fieldNo, char *ret_buf)
{

#ifdef DEBUG
    //assert(db.find(_dbName) != db.end());

#endif /*  */
    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    field *_dbField = dbField[dbNumber];
    char type;
    char rec_buf[MAXRECSIZ];
    int ival = 0;
    long lval = 0;
    type = _dbField[fieldNo].type;
    if (getrec(recno, fp, rec_buf) == 1)
	return 0;
    switch (type) {
    case 'c':

	//cout << "Extract(): c\n";
	strcpy(ret_buf, &rec_buf[_dbField[fieldNo].offset]);
	return 1;
    case 'd':
    case 'i':
	ival = get16(&rec_buf[_dbField[fieldNo].offset]);

	//cout << "Extract(): i = " << ival << endl;
	sprintf(ret_buf, "%d", ival);
	return 1;
    case 'l':
	lval = get32(&rec_buf[_dbField[fieldNo].offset]);

	//cout << "Extract(): l\n";
	sprintf(ret_buf, "%ld", lval);
	return 1;
    default:
	break;
    }
    return 0;
}

field *
NxDb::GetField(string _dbName)
{
    int dbNumber = db[_dbName];
    struct field *fp = dbField[dbNumber];

    return fp;
}

fildes *
NxDb::GetFilDes(string _dbName)
{
    int dbNumber = db[_dbName];
    struct fildes *fp = dbDesc[dbNumber];
    return fp;
}

void
NxDb::Purge(string _dbName, int fieldNo, char *value)
{

    int dbNumber = db[_dbName];
    fildes *fp = dbDesc[dbNumber];
    char ret_buf[MAXRECSIZ];
    if (fieldNo != -1) {

	// Purge specific records
	for (int i = 1; i <= fp->nrecs; i++) {
	    Extract(_dbName, i, fieldNo, ret_buf);
	    if (strcmp(ret_buf, value) == 0)
		DeleteRec(_dbName, i);
	}
    } else {

	// Purge all records
	for (int i = 1; i < fp->nrecs; i++)
	    DeleteRec(_dbName, i);
    }
}
void
fatal(int n)
{
    printf("*** Fatal Error #%d\n", n);
}
