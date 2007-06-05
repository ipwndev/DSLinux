/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdir.h>
#include <qfileinfo.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <dirent.h>
#include "kdir.h"
#include <time.h>
#include <qtimer.h>
#include <kfm.h>
#include "kfileinfo.h"

/*
** KDir - URL aware directory interface
**
*/

uint KDir::maxReturns = 100;

KDir::KDir()
    : QObject(0, "KDir")
{
    initLists();
    myNameFilter= "*";
    myFilterSpec= QDir::DefaultFilter;
    mySortSpec = QDir::DefaultSort;
    setPath(QDir::currentDirPath());
}

KDir::KDir(const char *url, const char *nameFilter,
	   QDir::SortSpec sortSpec,
	   QDir::FilterSpec filterSpec)
    : QObject(0, "KDir")
{
    initLists();
    myNameFilter = (nameFilter == 0) ? "*" : nameFilter;
    mySortSpec= sortSpec;
    myFilterSpec= filterSpec;
    myFilteredDirtyFlag= true;
    setPath(url);
}

KDir::KDir(const KDir &d)
    : QObject(0, "KDir")
{
    initLists();
    myNameFilter= d.nameFilter();
    setPath(d.url());
    mySortSpec= d.sorting();
    myFilterSpec= d.filter();
    myFilteredDirtyFlag= true;
}

KDir::KDir(const QDir &d)
    : QObject(0, "KDir")
{
    initLists();
    myNameFilter= d.nameFilter();
    setPath(d.path());
    mySortSpec= d.sorting();
    myFilterSpec= d.filter();
    myFilteredDirtyFlag= true;
}

void KDir::initLists() 
{
    myOpendir = 0;
    readFiles = 0;
    setMatchAllDirs(true);
    myTmpEntries.setAutoDelete(false);

    myDirtyFlag= true;
    myFilteredDirtyFlag= true;
    myKfm= 0;
    myEntries.setAutoDelete(true);
    myFilteredEntries.setAutoDelete(true);
    myFilteredNames.setAutoDelete(true);
}

KDir::~KDir()
{
    delete myKfm;
}


KDir &KDir::operator= (const KDir &d)
{
    setPath(d.path());
    return *this;
}

KDir &KDir::operator= (const QDir &d)
{
    setPath(d.path());
    return *this;
}

KDir &KDir::operator= (const char *url)
{
    setPath(url);
    return *this;
}

void KDir::cdUp()
{
    KURL u = myLocation.url().data();
    u.cd("..");
    setPath(u.url());
}

void KDir::setPath(const char *url)
{
    QString ts = url;
    if (ts.right(1) != "/")
	ts += "/";
    KURL tmp = ts.data();
    isBlocking = true;

    if (tmp.isLocalFile()) { // we can check, if the file is there
	struct stat buf;
	QString ts = tmp.path();
	int ret = stat(ts.data(), &buf);
	readable = (ret == 0);
	if (readable) { // further checks
	    DIR *test;
	    test = opendir(ts); // we do it just to test here
	    readable = (test != 0); 
	    if (test)
		closedir(test);
	}
    } else {
	readable = true; // what else can we say?
	isBlocking = false;
    }

    if (!tmp.isMalformed())
	myLocation= tmp.url();
    else
	warning("Malformed url %s\n", url);

    root = (strcmp(myLocation.path(),"/") == 0);
    
    if (!readable)
	return;  // nothing more we can do here

    if (myOpendir) {
	closedir(myOpendir);
	myOpendir = 0;
    }
    myDirtyFlag= true;
    myFilteredDirtyFlag= true;
}

void KDir::setURL(const char *url)
{
    setPath(url);
}

void KDir::setNameFilter(const char *nameFilter)
{
    myFilteredDirtyFlag |= (myNameFilter != nameFilter);
    myNameFilter= nameFilter;
}

void KDir::setFilter(int f)
{
    myFilteredDirtyFlag |= (myFilterSpec != f);
    myFilterSpec= static_cast<QDir::FilterSpec>(f);
}

void KDir::setSorting(int s)
{
    myFilteredDirtyFlag |= (mySortSpec != s);
    mySortSpec= static_cast<QDir::SortSpec>(s);
}

uint KDir::count()
{
    if (myFilteredDirtyFlag)
	updateFiltered();
    
    return myFilteredNames.count();
}

void KDir::updateFiltered()
{
    myFilteredNames.clear();
    myFilteredEntries.clear();

    if (myDirtyFlag) {
	myTmpEntries.clear();
	myEntries.clear();
	if (isBlocking)
	    getEntries();
	else
	    startLoading();
    } else {
	for (KFileInfo *i= myEntries.first(); i; i=myEntries.next()) {

	    if (filterEntry(i)) {
		KFileInfo *fi= new KFileInfo(*i);
		CHECK_PTR(fi);
		
		myFilteredEntries.append(fi);
		myFilteredNames.append(fi->fileName());
	    }
	}
    }
}

void KDir::timerEvent() {
    myTmpEntries.clear();
    if (myOpendir)
	getEntries();
    readFiles = 0;
    if (myTmpEntries.count())
	emit newFilesArrived(&myTmpEntries);
}

void KDir::getEntries() {

    struct dirent *dp;
    
    if (!myOpendir) {
	QString ts = myLocation.path();
	myOpendir = opendir(ts);
	if (!myOpendir)
	    return;
	dp = readdir(myOpendir); // take out the "."
    }

    if (myOpendir) {
	
	KFileInfo *i;
	QString path = myLocation.path();
	path += "/";

	do { 
	    dp = readdir(myOpendir);
	    if (!dp)
		break;
	    i = new KFileInfo(path, dp->d_name);
	    CHECK_PTR(i);
	    if (!i->fileName() || !i->fileName()[0]) {
		delete i;
		continue;
	    }
	    myEntries.append(i);
	    
	    /* if we just increase readFiles, if we found a file, it may
	     * be, that we die on the wrong filter */
	    readFiles++;

	    if (filterEntry(i)) {
		KFileInfo *fi= new KFileInfo(*i);
		CHECK_PTR(fi);
		
		myFilteredEntries.append(fi);
		myFilteredNames.append(fi->fileName());
		myTmpEntries.append(fi);
	    }
	} while (dp && readFiles < maxReturns);

	if (dp) { // there are still files left
	    QTimer::singleShot(40, this, SLOT(timerEvent()));
	} else {
	    closedir(myOpendir);
	    myOpendir = 0;
	    emit finished();
	}
    }
   
    myDirtyFlag= false;
    myFilteredDirtyFlag= false;
}

const KFileInfoList *KDir::entryInfoList(int filterSpec,
					 int sortSpec)
{
    setSorting(sortSpec);
    setFilter(filterSpec);
    
    if (myFilteredDirtyFlag)
	updateFiltered();
    
    if (isBlocking && !myOpendir)
	emit finished(); // the using class must know, that there are no more

    return &myFilteredEntries;
}

bool KDir::match(const char *filter, const char *name)
{
    // Split on white space
    QString s = filter;
    char *g = strtok(s.data(), " ");
    
    bool matched = false;
    while (g) {
	if (QDir::match(g, name)) {
	    matched = true;
	    break;
	}
	g = strtok(0, " ");
    }
    
    return matched;
}

bool KDir::filterEntry(KFileInfo *i)
{
    if (!strcmp(i->fileName(), ".."))
	return !root; 

    if (!(myFilterSpec & QDir::Hidden) && i->fileName()[0] == '.')
	return false;

    if (myNameFilter.isEmpty())
	return true;

    if ( !(myFilterSpec & QDir::Dirs) && i->isDir())
	return false;

    if (matchAllDirs() && i->isDir()) 
	return true;

    if (match(myNameFilter, i->fileName()))
	return true;

    return false;
}

bool KDir::startLoading()
{
    // If kfm is not busy
    if (myKfm == 0) {
	// Create a connection to kfm
	myKfm = new KFM;
	
	CHECK_PTR(myKfm);
	
	// Check connection was made
	if ( !myKfm->isOK() ) {
	    delete myKfm;
	    myKfm = 0;
	    readable = false;
	    emit finished(); // what else can I say? ;)
	} else {	// If all is well
	
	    connect(myKfm, SIGNAL(finished()), SLOT(slotKfmFinished()));
	    connect(myKfm, SIGNAL(error(int, const char *)),
		    SLOT(slotKfmError(int, const char *)));
	    connect(myKfm, SIGNAL(dirEntry(KDirEntry&)),
		    SLOT(slotDirEntry(KDirEntry&)));
	    myKfm->list(myLocation.url());
	    return true;
	}
    }
    
    return false;
}

void KDir::slotDirEntry(KDirEntry& entry) // SLOT
{

    KFileInfo *i= new KFileInfo(entry);
    CHECK_PTR(i);
    
    myEntries.append(i);
    
    if (filterEntry(i)) {
	
	KFileInfo *fi= new KFileInfo(entry);
	CHECK_PTR(fi);
	
	myFilteredEntries.append(fi);
	myFilteredNames.append(fi->fileName());
	
	emit dirEntry(fi);
    }
}

void KDir::setBlocking(bool _block) 
{
    isBlocking = _block;
}

void KDir::slotKfmFinished() // SLOT
{
    delete myKfm;
    myKfm= 0;
    
    emit finished();
}

void KDir::slotKfmError(int kerror, const char *text) // SLOT
{
    delete myKfm;
    myKfm= 0;
    
    emit error(kerror, text);
}

void KDir::setMaxReturns(uint max) {
    maxReturns = max;
}

#include "kdir.moc"

