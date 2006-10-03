/* This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
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

#ifndef _KFILEPREVIEW_H
#define _KFILEPREVIEW_H

#include "kpreview.h"
#include "kfileinfocontents.h"
#include "knewpanner.h"
#include "kdir.h"

class KFilePreview: protected KNewPanner, public KFileInfoContents
{
    Q_OBJECT
	
public:

    KFilePreview( KDir *inDir, bool s, QDir::SortSpec sorting,
		QWidget * parent=0, const char * name=0 );
    ~KFilePreview();
    
    virtual QWidget *widget() { return this; }
    virtual void setAutoUpdate(bool);
    virtual void setCurrentItem(const char *filename, const KFileInfo *i);
    virtual void repaint(bool f = true);

    virtual QString findCompletion( const char *base, bool activateFound );

    virtual bool acceptsFiles() { return true; }
    virtual bool acceptsDirs() { return true; }

    void registerPreviewModule( const char * format, PreviewHandler readPreview,
                                PreviewType inType);

protected slots:
    
    void dirActivated(KFileInfo *);
    void fileActivated(KFileInfo *);
    void fileHighlighted(KFileInfo *);

protected:
    
    virtual KFileInfoContents *getDirList() { return 0; }
    virtual KFileInfoContents *getFileList() { return 0; }

    virtual void highlightItem(unsigned int item);
    virtual void clearView();
    virtual bool insertItem(const KFileInfo *i, int index);
    
private:
    KFileInfoContents *fileList;
    KPreview *myPreview;

};

#endif
