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

#include "htmldata.h"
#include <kapp.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//const int defaultFontSizes[7] = { 8, 10, 12, 14, 18, 24, 32 };
const int defaultFontSizes[7] = { 8, 10, 12, 14, 16, 18, 24 };


HTMLSettings::HTMLSettings()
{
    memcpy( fontSizes, defaultFontSizes, sizeof(fontSizes) );
    fontBaseSize  = 3;
    fontBaseColor = black;
    fontBaseFace  = "times";

    fixedFontFace = "courier";

    linkColor     = blue;
    vLinkColor    = darkMagenta;

#ifdef EXEC_EXTENSIONS
    eLinkColor    = cyan;
#endif

    bgColor       = lightGray;

    charset	  = KApplication::getKApplication()->getCharsets()->defaultCh();

    underlineLinks = true;
    forceDefault = false;
}

HTMLSettings::HTMLSettings( const HTMLSettings &s )
{
    memcpy( fontSizes, s.fontSizes, sizeof(fontSizes) );
    fontBaseSize  = s.fontBaseSize;
    fontBaseColor = s.fontBaseColor;
    fontBaseFace  = s.fontBaseFace;

    fixedFontFace = s.fixedFontFace;

    linkColor     = s.linkColor;
    vLinkColor    = s.vLinkColor;

#ifdef EXEC_EXTENSIONS    
    eLinkColor    = s.eLinkColor;
#endif

    bgColor       = s.bgColor;
    
    charset       = s.charset;

    underlineLinks = s.underlineLinks;
    forceDefault   = s.forceDefault;
}

const HTMLSettings &HTMLSettings::operator=( const HTMLSettings &s )
{
    memcpy( fontSizes, s.fontSizes, sizeof(fontSizes) );
    fontBaseSize  = s.fontBaseSize;
    fontBaseColor = s.fontBaseColor;
    fontBaseFace  = s.fontBaseFace;

    fixedFontFace = s.fixedFontFace;

    linkColor     = s.linkColor;
    vLinkColor    = s.vLinkColor;

#ifdef EXEC_EXTENSIONS
    eLinkColor    = s.eLinkColor;
#endif

    bgColor       = s.bgColor;
    
    charset       = s.charset;

    underlineLinks = s.underlineLinks;
    forceDefault   = s.forceDefault;

    return *this;
}

void HTMLSettings::setFontSizes(const int *newFontSizes)
{
    memcpy( fontSizes, newFontSizes, sizeof(fontSizes) );
}

void HTMLSettings::getFontSizes(int *oldFontSizes)
{
    memcpy( oldFontSizes, fontSizes, sizeof(fontSizes) );
}

void HTMLSettings::resetFontSizes(void)
{
    memcpy( fontSizes, defaultFontSizes, sizeof(fontSizes) );
}
