/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

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
/*
 * $Id$
 *
 * $Log$
 * Revision 1.1  2006/10/03 11:26:32  dslinux_amadeus
 * adding pristine copy of pixil to HEAD so I can branch from it
 *
 * Revision 1.1  2003/09/08 19:42:09  jasonk
 * Addition of packages directory and associated files.
 *
 * Revision 1.1.1.1  2003/08/07 21:18:32  jasonk
 * Initial import of PIXIL into new cvs repository.
 *
 * Revision 1.1.1.1  2003/06/23 22:04:24  jasonk
 *
 *
 * Revision 1.1.1.1  2000/07/07 16:11:00  jasonk
 * Initial import of ViewML
 *
 * Revision 1.9  1999/01/18 10:56:54  kulow
 * .moc files are back in kdelibs. Built fine here using automake 1.3
 *
 * Revision 1.8  1999/01/15 09:31:13  kulow
 * it's official - kdelibs builds with srcdir != builddir. For this I
 * automocifized it, the generated rules are easier to maintain than
 * selfwritten rules. I have to fight with some bugs of this tool, but
 * generally it's better than keeping them updated by hand.
 *
 * Revision 1.7  1997/10/16 11:15:27  torben
 * Kalle: Copyright headers
 * kdoctoolbar removed
 *
 * Revision 1.6  1997/05/17 20:38:23  kalle
 * Kalle:
 * - Bugfix for KPanner (from Paul Kendall)
 * - Better colors with kdisplay schemes (from Bernd Wuebben)
 * - new behavior in KApplication::invokeHTMLHelp(): if the first
 *   argument (the filename) is empty, the filename is defaulted to
 * 	$KDEDIR/doc/HTML/<appname>/<appname>.html
 * - KApplication::getCaption for Matthias added (breaks binary compatibility!)
 *
 * Revision 1.5  1997/05/09 15:10:12  kulow
 * Coolo: patched ltconfig for FreeBSD
 * removed some stupid warnings
 *
 * Revision 1.4  1997/05/02 19:33:49  kulow
 * Coolo: corrected some CVS related problems with Kalle's last commit
 * updated khtmlw to 0.5.4
 *
 * Revision 1.3  1997/05/02 16:46:40  kalle
 * Kalle: You may now override how KApplication reacts to external changes
 * KButton uses the widget default palette
 * new kfontdialog version 0,5
 * new kpanner by Paul Kendall
 * new: KIconLoader
 *
 * Revision 1.2  1997/04/30 22:28:37  kulow
 * Coolo: updated KWidget classes (out of kghostview)
 * patched kbutton (from Taj)
 * BTW: Do you like the new look of the toolbar? (I'm not)
 *
 * Revision 1.1.1.1  1997/04/13 14:42:43  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:08  cvsuser
 * Sources imported
 *
 * Revision 1.1  1997/03/09 16:41:56  kalle
 * Initial revision
 *
 */

#include <qpainter.h>
#include <qcursor.h>
#include <qframe.h>
#include <kpanner.h>
#include "kpanner.h"

KPanner::KPanner(QWidget *parent, const char *name, unsigned flags, int d)
    : QWidget(parent, name)
{
    u_flags = flags;

    /*
     * create the child widgets and the divider widget
     */
    cw0 = new QWidget(this, "_pchild0");
    cw1 = new QWidget(this, "_pchild1");
    divider = new QFrame(this, "_pdivider", 0, TRUE);
    divider->setFrameStyle(QFrame::Panel | QFrame::Raised);
    divider->setLineWidth(1);

  
    /*
     * set the cursor shape
     */
    if((u_flags & P_ORIENTATION) == O_HORIZONTAL)
        divider->setCursor(QCursor(sizeVerCursor));
    else // O_VERTICAL
        divider->setCursor(QCursor(sizeHorCursor));

    u_limit = 0;
    l_limit = 0;

    pos = d;
    checkRange(pos);
    divider->installEventFilter(this);
}

KPanner::~KPanner() {
    delete cw0;
    delete cw1;
    delete divider;
}

/*
 * resize the panner.
 * this calculates the new geometry information for the client widgets
 * and the separator widget
 */

void KPanner::resizeEvent(QResizeEvent*)
{
    int abs_coord = pos;

    if((u_flags & P_UNITS) == U_PERCENT)
        abs_coord = (int)(pos / 100.0 * getMaxValue());
    
    if((u_flags & P_ORIENTATION) == O_HORIZONTAL) {
        cw0->setGeometry(0, 0, width(), abs_coord);
        cw1->setGeometry(0, abs_coord+4, width(), height()-abs_coord-4);
    }
    else { // O_VERTICAL
        cw0->setGeometry(0, 0, abs_coord, height());
        cw1->setGeometry(abs_coord+4, 0, width()-abs_coord-4, height());
    }
        
    setDividerGeometry(pos);
}

void KPanner::setDividerGeometry(int pos)
{
    int abs_coord = pos;
    
    if((u_flags & P_UNITS) == U_PERCENT)
        abs_coord = (int)(pos / 100.0 * getMaxValue());
    
    if((u_flags & P_ORIENTATION) == O_HORIZONTAL)
        divider->setGeometry(0, abs_coord, width(), 4);
    else // O_VERTICAL
        divider->setGeometry(abs_coord, 0, 4, height());
}

/*
 * dragging the mouse on the separator initiates the drag action. It does
 * not check for any mouse button (yet).
 */

bool KPanner::eventFilter(QObject *obj, QEvent *ev)
{
    /*
     * initiate divider drag action. record current position
     */
    
    if(ev->type() == Event_MouseButtonPress) {
        if((u_flags & P_ORIENTATION) == O_HORIZONTAL)
            old_coord = divider->y();
        else // O_VERTICAL
            old_coord = divider->x();
    }
    
    if(ev->type() == Event_MouseMove) {

        if(obj == (QObject *)divider) {
            QMouseEvent *mev = (QMouseEvent *)ev;

            /*
             * get the new coordinate, depending on our orientation.
             */
            if((u_flags & P_ORIENTATION) == O_HORIZONTAL)
                delta = mev->pos().y();
            else // O_VERTICAL
                delta = mev->pos().x();
            old_coord += delta;

            /*
             * check for limits
             */
            checkRange( old_coord );
            
            if((u_flags & P_ORIENTATION) == O_HORIZONTAL)
                divider->move(0, old_coord);
            else // O_VERTICAL
                divider->move(old_coord, 0);

            return TRUE;
        }
    }

    if(ev->type() == Event_MouseButtonRelease) {
        int max_value = getMaxValue();
    
        if((u_flags & P_UNITS) == U_PERCENT)
            pos = (int)((old_coord * 100.0) / max_value);
        else
            pos = old_coord;
        setDividerGeometry(pos);
        resizeEvent(0);
        emit(positionChanged());
        return TRUE;
    }
    
    return FALSE;
}

QWidget *KPanner::child0()
{
    return cw0;
}

QWidget *KPanner::child1()
{
    return cw1;
}

void KPanner::setSeparator(int size)
{
    if((u_flags & P_UNITS) == U_PERCENT)
        pos = size;
    else
        pos = (int)(size / 100.0 * getMaxValue());
    
    checkRange(pos);
    resizeEvent(0);
    emit(positionChanged());
}

void KPanner::setAbsSeparator(int size)
{
    if((u_flags & P_UNITS) == U_PERCENT)
        pos = (int)(size * 100.0 / getMaxValue());
    else
        pos = size;
    
    checkRange(pos);
    resizeEvent(0);
    emit(positionChanged());
}

void KPanner::setLimits(int l, int u)
{
    l_limit = (l < 0 ? 0 : l);
    if((u_flags & P_UNITS) == U_PERCENT) {
        if(u < 0 && u >= -100)
            u_limit = -100;
        else if(u >= 100)
            u_limit = 100;
        else
            u_limit = u;
    }
    else {
        int m = getMaxValue();
        
        if(u < 0 && u >= -m)
            u_limit = -m;
        else if(u >= m)
            u_limit = m;
        else
            u_limit = u;
    }
    
    if(l_limit >= u_limit) {
        l_limit = 0;
        u_limit = 0;
    }

    if(checkRange(pos)) {
        resizeEvent(0);
    }
}

int KPanner::getMaxValue()
{
    if(u_flags & O_HORIZONTAL) {
        return height() - 4;
    }
    else { // O_VERTICAL
        return width() - 4;
    }
   return 0;
}

bool KPanner::checkRange(int & value)
{
    int max_value = getMaxValue();
    
    if((u_flags & P_UNITS) == U_PERCENT) {
        float percent;

        percent = value * 100.0 / max_value;
        if( percent < l_limit )
            percent = l_limit;
        else if( u_limit > 0 && percent > u_limit )
            percent = u_limit;
        else if( u_limit <= 0 && percent > 100+u_limit )
            percent = 100+u_limit;
        else
            return FALSE;
        value = (int)(percent / 100.0 * max_value);
    }
    else {
        if( value < l_limit )
            value = l_limit;
        else if( u_limit > 0 && value > u_limit )
            value = u_limit;
        else if( u_limit <= 0 && value > max_value+u_limit )
            value = max_value+u_limit;
        else
            return FALSE;
    }
    return TRUE;
}

int KPanner::getSeparator() 
{
    if((u_flags & P_UNITS) == U_PERCENT)
        return pos;
    else
        return (int)(pos / 100.0 * getMaxValue());
}

int KPanner::getAbsSeparator() 
{
    if((u_flags & P_UNITS) == U_PERCENT)
        return (int)(pos * 100.0 / getMaxValue());
    else
        return pos;
}
#include "kpanner.moc"
