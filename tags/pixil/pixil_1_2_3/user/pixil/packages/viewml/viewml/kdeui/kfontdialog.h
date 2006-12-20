/*
    $Id$

    Requires the Qt widget libraries, available at no cost at 
    http://www.troll.no
       
    Copyright (C) 1997 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu

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
  
    $Log$
    Revision 1.1  2006/10/03 11:26:31  dslinux_amadeus
    adding pristine copy of pixil to HEAD so I can branch from it

    Revision 1.1  2003/09/08 19:42:09  jasonk
    Addition of packages directory and associated files.

    Revision 1.1.1.1  2003/08/07 21:18:32  jasonk
    Initial import of PIXIL into new cvs repository.

    Revision 1.1.1.1  2003/06/23 22:04:23  jasonk


    Revision 1.1.1.1  2000/07/07 16:11:00  jasonk
    Initial import of ViewML

    Revision 1.14  1998/11/30 19:31:33  lavikka
    Now kfontdialog uses QLayout instead of hardcoded widget coordinates.
    Command buttons are aligned correctly as well. Looks good and behaves well.

    Revision 1.13  1998/09/01 20:21:54  kulow
    I renamed all old qt header files to the new versions. I think, this looks
    nicer (and gives the change in configure a sense :)

    Revision 1.12  1998/08/31 12:43:25  esken
    GPL -> LGPL

    Revision 1.11  1998/06/01 09:13:35  kalle
    Added static getFontAndText()

    Revision 1.10  1998/06/01 08:42:42  kalle
    KFontDialog:
    - you can now enter your own example string
    - new static method getXLFD() that converts a QFont() to a X Logical Font Description

    KIntegerLine:
    - new signal valueChanged( int )

    Revision 1.9  1998/01/21 15:07:00  jacek
    Added real KCharsets support

    Revision 1.7  1997/11/09 22:56:12  wuebben
    Bernd: colorscheme related changes

    Revision 1.6  1997/11/09 03:45:57  wuebben
    *** empty log message ***

    Revision 1.5  1997/10/21 20:45:01  kulow
    removed all NULLs and replaced it with 0L or "".
    There are some left in mediatool, but this is not C++

    Revision 1.4  1997/10/16 11:15:22  torben
    Kalle: Copyright headers
    kdoctoolbar removed

    Revision 1.3  1997/10/05 18:25:55  ssk
    Added short descriptions for documentation index.

    Revision 1.2  1997/05/02 16:46:39  kalle
    Kalle: You may now override how KApplication reacts to external changes
    KButton uses the widget default palette
    new kfontdialog version 0,5
    new kpanner by Paul Kendall
    new: KIconLoader

    Revision 1.6  1997/04/29 03:11:18  wuebben
    Added more comments

    Revision 1.5  1997/04/29 02:44:24  wuebben

    added support for ~/.kde/config/kdefonts
    and X server fontlist lookup

    Revision 1.4  1997/04/27 01:50:49  wuebben

    Revision 1.3  1997/04/20 14:59:45  wuebben
    fixed a minor bug which caused the last font in the font list to not
    be displayed

    Revision 1.1  1997/04/20 00:18:15  wuebben
    Initial revision

    Revision 1.2  1997/03/02 22:40:59  wuebben

    Revision 1.1  1997/01/04 17:36:44  wuebben
    Initial revision


*/


#ifndef _K_FONT_DIALOG_H_
#define _K_FONT_DIALOG_H_

#include <qmessagebox.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qframe.h> 
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollbar.h>
#include <qtooltip.h>
#include <kbuttonbox.h>

#include <qstring.h>
#include <qfont.h>

/**
* Dialog for interactive font selection.
* @author Bernd Wuebben (wuebben@kde.org)
* @version $Id$
*/
class KFontDialog : public QDialog {

    Q_OBJECT

      // Usage of the KFontDialog Class:
      //
      // Case 1) The pointer fontlist is null ( Recommended Usage !!)
      // 
      // In this case KFontDialog will first search
      // for ~/.kde/config/kdefonts. If kdefonts if available
      // it will insert the fonts listed there into the family combo.
      // 
      // Note: ~/.kde/config/kdefonts is managed by kfontmanager. 
      // ~/.kde/config/kdefonts is a newline separated list of font names.
      // Such as: time\nhelvetica\nfixed\n etc.You should however not 
      // manipulate that list -- that is the job of kfontmanager.
      // 
      // If ~/.kde/config/kdefonts doesn't exist, KFontDialog will query
      // the X server and insert all availabe fonts.
      //
      // Case 2) The pointer fontlist is non null. In this cae KFontDialog 
      // will insert the strings of that QStrList into the family combo.
      // 
      // Note: Due to a bug in Qt 1.2 you must 
      // supply at this point at least two fonts in the QStrList that
      // fontlist points to. The bug has been reported and will hopefully
      // be fixed in Qt.1.3. 


public:
    KFontDialog( QWidget *parent = 0L, const char *name = 0L,
			bool modal = FALSE, const QStrList* fontlist = 0L );

    void setFont( const QFont &font );
    QFont font()	{  return selFont; }

    /*
     * This is probably the function you are looking for.
     * Just call this to pop up a dialog to get the selected font.
     * returns result().
     */

    static int getFont( QFont &theFont );

  /*
   * When you are not only interested in the font selected, but also
   * in the example string typed in, you can call this method.
   */
  static int getFontAndText( QFont &theFont, QString &theString );

  /*
   * This function converts a QFont into the corresponding X Logical Font 
   * Description.
   */
  static QString getXLFD( const QFont &theFont );

signals:
	/*
	 * connect to this to monitor the font as it as selected if you are
	 * not running modal.
	 */
	void fontSelected( const QFont &font );

private slots:

      void 	family_chosen_slot(const char* );
      void      size_chosen_slot(const char* );
      void      weight_chosen_slot(const char*);
      void      style_chosen_slot(const char*);
      void      display_example(const QFont &font);
      void      charset_chosen_slot(const char *);
      void      setColors();
private:

    bool loadKDEInstalledFonts();
    void fill_family_combo();
    void setCombos();


    QVBoxLayout  *layout;
    QGroupBox	 *box1;
    QGroupBox	 *box2;
    KButtonBox   *bbox;
    QGridLayout  *box1layout;
    QGridLayout  *box2layout;

    // pointer to an optinally supplied list of fonts to 
    // inserted into the fontdialog font-family combo-box
    QStrList     *fontlist; 

    QLabel	 *family_label;
    QLabel	 *size_label;
    QLabel       *weight_label;
    QLabel       *style_label;
    QLabel	 *charset_label;

    QLabel	 *actual_family_label;
    QLabel	 *actual_size_label;
    QLabel       *actual_weight_label;
    QLabel       *actual_style_label;
    QLabel	 *actual_charset_label;


    QLabel	 *actual_family_label_data;
    QLabel	 *actual_size_label_data;
    QLabel       *actual_weight_label_data;
    QLabel       *actual_style_label_data;
    QLabel	 *actual_charset_label_data;
    QComboBox    *family_combo;
    QComboBox    *size_combo;
    QComboBox    *weight_combo;
    QComboBox    *style_combo;
    QComboBox	 *charset_combo;    

    QLineEdit       *example_edit;
    QFont         selFont;

};


#endif
