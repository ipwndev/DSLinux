/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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

#ifndef __HTMLDATA_H__
#define __HTMLDATA_H__

#include <qcolor.h>
#include <qcursor.h>
#include <qstring.h>
#include <kcharsets.h>

class HTMLSettings
{
public:
    HTMLSettings();
    HTMLSettings( const HTMLSettings & );

    const HTMLSettings &operator=( const HTMLSettings & );

    void setFontSizes(const int *newFontSizes);
    void getFontSizes(int *newFontSizes);
    void resetFontSizes(void);

    int     fontSizes[7];
    int     fontBaseSize;
    QColor  fontBaseColor;
    QString fontBaseFace;

    QString fixedFontFace;

    QColor  linkColor;
    QColor  vLinkColor;

#ifdef EXEC_EXTENSIONS
    QColor  eLinkColor;
#endif

    QColor  bgColor;

    KCharset charset; 

    bool    underlineLinks;
    bool    forceDefault;
};

#endif

