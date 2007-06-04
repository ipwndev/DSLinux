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

#include "ktabbar.h"

struct KTabBarProtected
{
    QTabBar *qtab;
    QWidget *tabw;
    KTabButton *right;
    KTabButton *left;
    bool rightscroll;
    bool leftscroll;
    int tpos;
    int t_width;
    int tw_width;
    int tw_height;
    int tw_start;
};

KTabBar::KTabBar(QWidget *parent, const char *name)
 : QWidget(parent, name)
{
  initMetaObject();
  init();
}

KTabBar::~KTabBar()
{
  //debug("KTabBar - destructor");

  //debug("KTabBar - destructor done");
}

void KTabBar::init()
{
  ptab = new KTabBarProtected;
  ptab->tpos = 0;
  ptab->leftscroll = ptab->rightscroll = false;
  ptab->tabw = new QWidget(this);
  ptab->qtab = new QTabBar(ptab->tabw);

  ptab->left = new KTabButton(LeftArrow, this);
  connect( ptab->left, SIGNAL(clicked()), SLOT( leftClicked()) );

  ptab->right = new KTabButton(RightArrow, this);
  connect( ptab->right, SIGNAL(clicked()), SLOT( rightClicked()) );

  connect( ptab->qtab, SIGNAL(selected(int)), SLOT( emitSelected(int)) );

  //debug("init - done");
}

QTabBar *KTabBar::getQTab()
{
  return ptab->qtab;
}

QSize KTabBar::sizeHint()
{
  return ptab->qtab->sizeHint();
}

int KTabBar::addTab( QTab *tab )
{
  return ptab->qtab->addTab( tab );
}

void KTabBar::setTabEnabled( int tab, bool enable )
{
  ptab->qtab->setTabEnabled( tab, enable );
}

bool KTabBar::isTabEnabled( int tab )
{
  return ptab->qtab->isTabEnabled( tab );
}

int KTabBar::currentTab()
{
  return ptab->qtab->currentTab();
}

QTab *KTabBar::tab( int tab )
{
  return ptab->qtab->tab( tab );
}

int KTabBar::keyboardFocusTab()
{
  return ptab->qtab->keyboardFocusTab();
}

void KTabBar::setCurrentTab(int tab)
{
  ptab->qtab->setCurrentTab(tab);
}

void KTabBar::setCurrentTab(QTab *tab)
{
  ptab->qtab->setCurrentTab(tab);
}

void KTabBar::leftClicked()
{
  if(ptab->tpos == (0-(2*JUMP)))
    ptab->tpos += (2 * JUMP);
  else
    ptab->tpos += JUMP;
  QResizeEvent e(size(), size());
  resizeEvent(&e);
  emit scrolled( RightArrow );
}

void KTabBar::rightClicked()
{
  if(ptab->tpos == 0)
    ptab->tpos -= (2 * JUMP);
  else
    ptab->tpos -= JUMP;
  QResizeEvent e(size(), size());
  resizeEvent(&e);
  emit scrolled( LeftArrow );
}

void KTabBar::emitSelected(int tab)
{
  emit selected(tab);
}

void KTabBar::setSizes()
{
  ptab->leftscroll = ptab->rightscroll = false;
  ptab->tw_width = width();
  ptab->t_width = ptab->qtab->sizeHint().width();
  if(ptab->tw_width > ptab->t_width)
    ptab->tw_width = ptab->t_width;
  ptab->tw_height = ptab->qtab->sizeHint().height();
  ptab->tw_start = 0;
  int r = 0;

  //debug("width: %d - t_width: %d", width(), ptab->t_width);
  //debug("tpos %d", ptab->tpos);

  if(width() >= ptab->t_width)
    ptab->tpos = 0;
  else
  {
    if(ptab->tpos < 0)
    {
      //debug("leftscroll");
      ptab->leftscroll = true;
      ptab->tw_start = ptab->tw_height;
      ptab->tw_width -= ptab->tw_start;
      r = ptab->tw_height;
    }

    if(width() < (ptab->t_width + ptab->tpos + ptab->tw_start))
    {
      //debug("rightscroll");
      ptab->rightscroll = true;
      ptab->tw_width -= ptab->tw_height;
    }
  }
  if(ptab->tw_width > (ptab->t_width+ptab->tpos))
    ptab->tw_width = ptab->t_width + ptab->tpos;
  //debug("tw_width %d", tw_width);
  //debug("\n");
}

void KTabBar::resizeEvent(QResizeEvent *)
{
  //debug("KTabBar, resizing");

  setSizes();

  if(ptab->leftscroll)
  {
    ptab->left->setGeometry( 0, 0, ptab->tw_height, ptab->tw_height);
    ptab->left->show();
  }
  else
    ptab->left->hide();

  if(ptab->rightscroll)
  {
    ptab->right->setGeometry( width()-ptab->tw_height, 0,
                              ptab->tw_height, ptab->tw_height);
    ptab->right->show();
  }
  else
    ptab->right->hide();

  ptab->tabw->setGeometry( ptab->tw_start, 0,
                           ptab->tw_width, ptab->tw_height);
  ptab->qtab->setGeometry( ptab->tpos, 0,
                           ptab->qtab->sizeHint().width(), ptab->tw_height);
  //debug("KTabBar\ntabw: %dx%d\nqtab: %dx%d", tw_width, tw_height,
  //                          qtab->sizeHint().width(), tw_height);

  QPaintEvent pe(geometry());
  paintEvent( &pe );

  //debug("KTabBar, resize - done");
}

void KTabBar::paintEvent(QPaintEvent *)
{
  //debug("KTabBar - painting");

  int end = width()-1;
  int start = ptab->tw_width + ptab->tpos;
  //debug("width: %d - start: %d", width(), start);

  if(ptab->leftscroll)  // if we need a left button we start the tabs at tw_height
    start = ptab->tw_height; // ( tw_height == left->width() )
  //start = 0;
  if(ptab->rightscroll)
    end -= ptab->tw_height;
  //debug("width: %d", width());
  //debug("tw_width: %d", tw_width);
  //debug("tw_start: %d", tw_start);
  //debug("2*tw_height + tw_width: %d", (2*tw_height)+tw_width);
  //debug("start: %d - end: %d", start, end);

  QPainter p;
  // start painting widget
  p.begin(this);
  QPen pen( white, 1 );
  p.setPen( pen );

  // draw bottom line
  if(start < end)
    p.drawLine( start, ptab->tw_height-1, end, ptab->tw_height-1);

  p.end();

  //qtab->repaint();

  //debug("KTabBar - painting done");
}
#include "ktabbar.moc"
