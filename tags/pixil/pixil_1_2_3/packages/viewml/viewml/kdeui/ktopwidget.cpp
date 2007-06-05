/* This file is part of the KDE libraries
    Copyright  (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
               (C) 1997, 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
               (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)

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

 // $Id$
 // $Log$
 // Revision 1.1  2006/10/03 11:26:33  dslinux_amadeus
 // adding pristine copy of pixil to HEAD so I can branch from it
 //
 // Revision 1.1  2003/09/08 19:42:11  jasonk
 // Addition of packages directory and associated files.
 //
 // Revision 1.1.1.1  2003/08/07 21:18:33  jasonk
 // Initial import of PIXIL into new cvs repository.
 //
 // Revision 1.1.1.1  2003/06/23 22:04:24  jasonk
 //
 //
 // Revision 1.1.1.1  2000/07/07 16:11:00  jasonk
 // Initial import of ViewML
 //
 // Revision 1.46  1999/01/18 10:57:13  kulow
 // .moc files are back in kdelibs. Built fine here using automake 1.3
 //
 // Revision 1.45  1999/01/15 09:31:32  kulow
 // it's official - kdelibs builds with srcdir != builddir. For this I
 // automocifized it, the generated rules are easier to maintain than
 // selfwritten rules. I have to fight with some bugs of this tool, but
 // generally it's better than keeping them updated by hand.
 //
 // Revision 1.44  1998/08/02 09:31:22  radej
 // sven: added closeEvent to behave like old KTW
 //
 // Revision 1.43  1998/08/01 21:59:29  radej
 // sven: inherits KTMainWindow now. KTW is dead.
 //
 // Revision 1.42  1998/06/18 19:17:02  radej
 // sven: That was kfind bug! And I fixed it there and forgot about it.
 //       No changes but I don't know how to stop this thing.
 //
 // Revision 1.41  1998/06/12 19:39:17  ettrich
 // Matthias: Something I noticed with KLyX
 //
 //
 //     /** Deletes all KTMainWindows. This is a good thing to call before
 //       * an applications wants to exit via kapp->quit(). Rationale: The
 //       * destructors of main windows may want to delete other widgets
 //       * as well. Now, if an application calls kapp->quit() then Qt
 //       * will destroy all widgets in a somewhat random order which may
 //       * result in double-free'ed memory (=segfault). Since not every
 //       * program checks for QApplication::closingDown() before deleting
 //       * a widget, calling KTMainWindow::deleteAll() before is a good
 //       * and proper solution.
 //      */
 //   static void deleteAll();
 //
 //     /** Deletes all KTopLevelWidgets. This is a good thing to call before
 //       * an applications wants to exit via kapp->quit(). Rationale: The
 //       * destructors of main windows may want to delete other widgets
 //       * as well. Now, if an application calls kapp->quit() then Qt
 //       * will destroy all widgets in a somewhat random order which may
 //       * result in double-free'ed memory (=segfault). Since not every
 //       * program checks for QApplication::closingDown() before deleting
 //       * a widget, calling KTopLevelWidgets::deleteAll() before is a good
 //       * and proper solution.
 //       */
 //       static void deleteAll();
 //
 // Revision 1.40  1998/05/10 17:42:00  radej
 // Fixed: two vertical toolbars when window is tall enough to
 // have one under another. All toolbars had with of the first one.
 // Fixedsize mode has even bigger bug. Ugh.
 //
 // Revision 1.39  1998/04/16 18:46:49  radej
 // Removed som debug messages before beta4
 //
 // Revision 1.38  1998/04/16 16:06:48  ettrich
 // Matthias: kfm session management showstopper hopefully fixed
 //
 // Revision 1.37  1998/04/05 18:18:44  radej
 // Reverted to old interface (before Matthias' changes)
 //



#include <ktopwidget.h>
#include <ktopwidget.h>

// a static pointer (too bad we cannot have static objects in libraries)
//QList<KTopLevelWidget>* KTopLevelWidget::memberList = 0L;

KTopLevelWidget::KTopLevelWidget( const char *name )
  : KTMainWindow(name)
{}

KTopLevelWidget::~KTopLevelWidget()
{}

void KTopLevelWidget::closeEvent (QCloseEvent *ev)
{
  ev->accept();
}
#include "ktopwidget.moc"

