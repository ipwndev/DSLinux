/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)

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

#ifndef __KTABBAR_H__
#define __KTABBAR_H__

#include <kapp.h>
#include <kwizard.h>
#include <qtabbar.h>
#include <kdbtn.h>

struct KTabBarProtected;

#define JUMP 30

/**
* KTabBar is very similar to QTabBar. The only difference is that if the tab bar
* needs more space it provides left and right buttons used to scroll the tab bar.
* @short KTabBar
* @author Thomas Tanghus <tanghus@earthling.net>
* @version 0.1
*/
class KTabBar : public QWidget
{
    Q_OBJECT

public:
    KTabBar( QWidget * parent = 0, const char * name = 0 );
   ~KTabBar();

    QTabBar *getQTab();
    QSize sizeHint();
    int addTab( QTab *tab );
    void setTabEnabled( int tab, bool enable );
    bool isTabEnabled( int tab );
    int currentTab();
    QTab *tab( int tab );
    int keyboardFocusTab();

public slots:
    void setCurrentTab( int tab );
    void setCurrentTab( QTab *tab );

signals:
    void selected( int );
    void scrolled( ArrowType );

protected slots:
    void leftClicked();
    void rightClicked();
    void emitSelected(int);

protected:
    void init();
    void setSizes();
    void resizeEvent ( QResizeEvent * );
    void paintEvent ( QPaintEvent * );

    KTabBarProtected *ptab;
};


#endif // __KTABBAR_H__



