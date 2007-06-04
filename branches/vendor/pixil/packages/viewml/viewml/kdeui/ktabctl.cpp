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
 * Revision 1.1  2003/09/08 19:42:10  jasonk
 * Addition of packages directory and associated files.
 *
 * Revision 1.1.1.1  2003/08/07 21:18:33  jasonk
 * Initial import of PIXIL into new cvs repository.
 *
 * Revision 1.1.1.1  2003/06/23 22:04:24  jasonk
 *
 *
 * Revision 1.1.1.1  2000/07/07 16:11:00  jasonk
 * Initial import of ViewML
 *
 * Revision 1.16  1999/01/18 10:57:07  kulow
 * .moc files are back in kdelibs. Built fine here using automake 1.3
 *
 * Revision 1.15  1999/01/15 09:31:26  kulow
 * it's official - kdelibs builds with srcdir != builddir. For this I
 * automocifized it, the generated rules are easier to maintain than
 * selfwritten rules. I have to fight with some bugs of this tool, but
 * generally it's better than keeping them updated by hand.
 *
 * Revision 1.14  1998/09/01 20:22:19  kulow
 * I renamed all old qt header files to the new versions. I think, this looks
 * nicer (and gives the change in configure a sense :)
 *
 * Revision 1.13  1998/07/13 08:39:32  hoss
 * Fixed small bug in showTab. pages[i]->raise() does only work with more than
 * one widget!
 *
 * Revision 1.12  1998/06/16 21:23:36  hoss
 * Added support for setFont and setShape
 *
 * Revision 1.11  1997/11/23 22:23:55  leconte
 * Two patches have been applied for the header line painting bug:
 * I removed mine.
 *
 * Revision 1.10  1997/11/18 21:41:39  kalle
 * kiconloaderdialog uses the default fonts (patch by Paul Kendall)
 * ktabctl paints the header lines correctly (patch by Paul Kendall)
 *
 * Revision 1.9  1997/11/15 03:10:49  esken
 * Applied another patch by Bertrand Leconte, which corrects the
 * Tab-Changes-Focus bug.
 *
 * Revision 1.8  1997/11/13 14:11:45  esken
 * Applied the changes of Bertrand Leconte. Now KTabCtl paints its top line.
 * Additional fix by me: Color "white" of Top/Left line changed to
 * "colorGroup().light()" (This bug was observed by Bernd Deimel, thanks).
 *
 * Revision 1.7  1997/10/21 20:45:06  kulow
 * removed all NULLs and replaced it with 0L or "".
 * There are some left in mediatool, but this is not C++
 *
 * Revision 1.6  1997/10/16 11:15:54  torben
 * Kalle: Copyright headers
 * kdoctoolbar removed
 *
 * Revision 1.5  1997/10/09 11:46:29  kalle
 * Assorted patches by Fritz Elfert, Rainer Bawidamann, Bernhard Kuhn and Lars Kneschke
 *
 * Revision 1.4  1997/05/30 20:04:41  kalle
 * Kalle:
 * 30.05.97:	signal handler for reaping zombie help processes reinstalls itself
 * 		patch to KIconLoader by Christian Esken
 * 		slightly better look for KTabCtl
 * 		kdecore Makefile does not expect current dir to be in path
 * 		Better Alpha support
 *
 * Revision 1.3  1997/05/09 15:10:13  kulow
 * Coolo: patched ltconfig for FreeBSD
 * removed some stupid warnings
 *
 * Revision 1.2  1997/04/15 20:35:14  kalle
 * Included patch to ktabctl.cpp from kfind distribution
 *
 * Revision 1.1.1.1  1997/04/13 14:42:43  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:09  cvsuser
 * Sources imported
 *
 * Revision 1.1  1997/03/15 22:40:57  kalle
 * Initial revision
 *
 * Revision 1.2.2.1  1997/01/07 14:41:57  alex
 * release 0.1
 *
 * Revision 1.2  1997/01/07 14:39:15  alex
 * some doc added, tested - ok.
 *
 * Revision 1.1.1.1  1997/01/07 13:44:53  alex
 * imported
 *
 *
 * KTabCtl provides a universal tab control. It is in no ways limited to dialogs and
 * can be used for whatever you want. It has no buttons or any other stuff.
 *
 * However, this is based on the original QTabDialog.
 */

#include "qtabbar.h"
#include "qpushbutton.h"
#include "qpainter.h"
#include "qpixmap.h"

#include "ktabctl.h"
#include "ktabctl.h"

KTabCtl::KTabCtl(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    tabs = new QTabBar(this, "_tabbar");
    connect(tabs, SIGNAL(selected(int)), this, SLOT(showTab(int)));
    tabs->move(2, 1);
    
    blBorder = TRUE;

    //    setFont(QFont("helvetica")); //BL: Why force a font ?
}

KTabCtl::~KTabCtl()
{
}

void KTabCtl::resizeEvent(QResizeEvent *)
{
    int i;
    QRect r = getChildRect();
    
    if (tabs) {
        for (i=0; i<(int)pages.size(); i++) {
            pages[i]->setGeometry(r);
        }
        if( ( tabs->shape() == QTabBar::RoundedBelow ) ||
            ( tabs->shape() == QTabBar::TriangularBelow ) ) {
            tabs->move( 0, height()-tabs->height()-4 );
        }
    }
}

void KTabCtl::setFont(const QFont & font)
{
    QFont f(font);
    f.setWeight(QFont::Light);
    QWidget::setFont(f);

    setSizes();
}

void KTabCtl::setTabFont(const QFont & font)
{
    QFont f(font);
//    f.setWeight(QFont::Light);
    tabs->setFont(f);

    setSizes();
}

void KTabCtl::show()
{
    unsigned int i;
    
    if(isVisible())
	return;

    setSizes();

    for(i = 0; i < pages.size(); i++)
	pages[i]->hide();

    QResizeEvent r(size(), size());
    resizeEvent(&r);

    QWidget::show();
}

bool KTabCtl::isTabEnabled(const char *name)
{
    unsigned int i;
    
    for(i = 0; i < pages.size(); i++)
	if (!qstrcmp( pages[i]->name(), name))
	    return tabs->isTabEnabled(i);   /* return the enabled status */
    return FALSE;     /* tab does not exist */
}

void KTabCtl::setTabEnabled(const char * name, bool state)
{
    unsigned i;

    if((name == 0L) || (strlen(name) == 0))
        return;
    
    for(i = 0; i < pages.size(); i++)
	if(!qstrcmp(pages[i]->name(), name))
	    tabs->setTabEnabled(i, state);
}

void KTabCtl::setSizes()
{
    unsigned i;
    
    QSize min(tabs->sizeHint());    /* the minimum required size for the tabbar */
    tabs->resize(min);         /* make sure that the tabbar does not require more space than actually needed. */
    
    
    QSize max(QCOORD_MAX,QCOORD_MAX);
    //int th = min.height();          /* the height of the tabbar itself (without pages and stuff) */

    for (i = 0; i < pages.size(); i++) {

        /*
         * check the actual minimum and maximum sizes
         */
        
	if (pages[i]->maximumSize().height() < max.height())
	    max.setHeight(pages[i]->maximumSize().height());
	if (pages[i]->maximumSize().width() < max.width())
	    max.setWidth( pages[i]->maximumSize().width());
	if ( pages[i]->minimumSize().height() > min.height())
	    min.setHeight( pages[i]->minimumSize().height());
	if ( pages[i]->minimumSize().width() > min.width())
	    min.setWidth( pages[i]->minimumSize().width());
    }

    // BL: min and max are sizes of children, not tabcontrol
    // min.setHeight(min.height() + th);

    if (max.width() < min.width())
	max.setWidth(min.width());
    if (max.height() < min.height())
	max.setHeight(min.height());

    /*
     * now, apply the calculated size values to all of the pages
     */
    
    for( i=0; i<(uint)pages.size(); i++ ) {
	pages[i]->setMinimumSize(min);
	pages[i]->setMaximumSize(max);
    }


    // BL: set minimum size of tabcontrol
    setMinimumSize(min.width()+4, min.height()+tabs->height()+4);

    /*
     * generate a resizeEvent, if we're visible
     */
    
    if(isVisible()) {
	QResizeEvent r(size(), size());
	resizeEvent(&r);
    }
}

void KTabCtl::setBorder( bool state )
{
    blBorder = state;
}

void KTabCtl::setShape( QTabBar::Shape shape )
{
    tabs->setShape( shape );  
}

void KTabCtl::paintEvent(QPaintEvent *)
{
    if (!tabs)
	return;

    if( !blBorder )
        return;
      
    QPainter p;
    p.begin(this);

    int y0 = getChildRect().top() - 1;
    int y1 = getChildRect().bottom() + 2;
    int x1 = getChildRect().right() + 2;
    int x0 = getChildRect().left() - 1;

    p.setPen( colorGroup().light() );
    p.drawLine(x0, y0, x1 - 1, y0);      /* top line */
    p.drawLine(x0, y0 + 1, x0, y1);      /* left line */
    p.setPen(black);
    p.drawLine(x1, y1, x0, y1);          /* bottom line */
    p.drawLine(x1, y1 - 1, x1, y0);
    p.setPen(colorGroup().dark());
    p.drawLine(x0 + 1, y1 - 1, x1 - 1, y1 - 1);  /* bottom */
    p.drawLine(x1 - 1, y1 - 2, x1 - 1, y0 + 1);
    p.end();
}

/*
 * return the client rect. This is the maximum size for any child
 * widget (page).
 */

QRect KTabCtl::getChildRect() const
{
    if( ( tabs->shape() == QTabBar::RoundedBelow ) || 
        ( tabs->shape() == QTabBar::TriangularBelow ) ) {
    	return QRect(2, 1, width() - 4,
		     height() - tabs->height() - 4);      
    } else {
      	return QRect(2, tabs->height() + 1, width() - 4,
		     height() - tabs->height() - 4);
    }
}

/*
 * show a single page, depending on the selected tab
 * emit tabSelected(new_pagenumber) BEFORE the page is shown
 */

void KTabCtl::showTab(int i)
{
    unsigned int j;
    for (j = 0; j < pages.size(); j++) {
      if (j != (unsigned)i) {
        pages[j]->hide();
      }
    }

    if((unsigned)i < pages.size()) {
        emit(tabSelected(i));
	if( pages.size() >= 2 ) { 
	  pages[i]->raise();
	}
        pages[i]->setGeometry(getChildRect());
        pages[i]->show();
    }
}

/*
 * add a tab to the control. This tab will manage the given Widget w.
 * in most cases, w will be a QWidget and will only act as parent for the
 * actual widgets on this page
 * NOTE: w is not required to be of class QWidget, but expect strange results with
 * other types of widgets
 */

void KTabCtl::addTab(QWidget *w, const char *name)
{
    QTab *t = new QTab();
    t->label = name;
    t->enabled = TRUE;
    int id = tabs->addTab(t);   /* add the tab itself to the tabbar */
    if (id == (int)pages.size()) {
	pages.resize(id + 1);
        pages[id] = w;          /* remember the widget to manage by this tab */
    }
    // BL: compute sizes
    setSizes();
}
#include "ktabctl.moc"
