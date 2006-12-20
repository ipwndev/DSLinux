/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//-----------------------------------------------------------------------------
// KDE color selection dialog.

// layout managment added Oct 1997 by Mario Weilguni 
// <mweilguni@sime.com>

#ifndef __KCOLORDLG_H__
#define __KCOLORDLG_H__

#include <qdialog.h>
#include <qtableview.h>
#include <qframe.h>
#include <qrangecontrol.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include "kselect.h"

//-----------------------------------------------------------------------------

class KHSSelector : public KXYSelector
{
  Q_OBJECT
public:
  KHSSelector( QWidget *parent );

private:
  void drawPalette();

protected:
  virtual void resizeEvent( QResizeEvent * );
  virtual void drawContents( QPainter *painter );

protected:
  QPixmap pixmap;
};

//-----------------------------------------------------------------------------

class KValueSelector : public KSelector
{
  Q_OBJECT
public:
  KValueSelector( QWidget *parent );

  void setHue( int h )	{	hue = h; }
  void setSaturation( int s )	{	sat = s; }

  void drawPalette();

protected:
  virtual void resizeEvent( QResizeEvent * );
  virtual void drawContents( QPainter *painter );

protected:
  int hue;
  int sat;
  QPixmap pixmap;
};

//-----------------------------------------------------------------------------

class KColorCells : public QTableView
{
  Q_OBJECT
public:
  KColorCells( QWidget *parent, int rows, int cols );
  ~KColorCells();

  void setColor( int colNum, const QColor &col );
  QColor color( int indx )
  {	return colors[indx]; }
  int numCells()
  {	return numRows() * numCols(); }
	
  int getSelected()
  {	return selected; }

  signals:
  void colorSelected( int col );

protected:
  virtual void paintCell( QPainter *painter, int row, int col );
  virtual void resizeEvent( QResizeEvent * );
  virtual void mouseReleaseEvent( QMouseEvent * );

  QColor *colors;
  int	selected;
};

//-----------------------------------------------------------------------------

class KColorPatch : public QFrame
{
  Q_OBJECT
public:
  KColorPatch( QWidget *parent );
  virtual ~KColorPatch();

  void setColor( const QColor &col );

protected:
  virtual void drawContents( QPainter *painter );

private:
  QColor color;
  uint pixel;
  int colContext;
};

//-----------------------------------------------------------------------------

/// KDE Color Selection dialog
/** KDE Color Selection dialog
Features:

Colour selection from a standard system palette.
Colour selection from a Palette of H vs S and V selectors (similar to windoze).
Direct input of HSV or RGB values.
Saving of custom colors

simplest use:
QColor myColor;
int result = KColorDialog::getColor( myColor );

 */
class KColorDialog : public QDialog
{
  Q_OBJECT
public:
  /// Constructor
  /** Construct a KColorDialog */
  KColorDialog( QWidget *parent = 0L, const char *name = 0L,
				bool modal = FALSE );

  /// Preselect a color
  /** Preselect a color */
  void setColor( const QColor &col );

  /// Retrieve the currently selected color.
  /** Retrieve the currently selected color. */
  QColor color()	{	return selColor; }
  
  /**
	This is probably the function you are looking for.
	Just call this to pop up dialog get the selected color.
	returns result().
	*/
  static int getColor( QColor &theColor );

 public slots:
 void slotOkPressed();

  signals:
 /// Notify when a color is selected.
 /**
   connect to this to monitor the color as it as selected if you are
   not running modal.
   */
 void colorSelected( const QColor &col );

 private slots:
 void slotRGBChanged();
  void slotHSVChanged();
  void slotHSChanged( int, int );
  void slotVChanged( int );
  void slotSysColorSelected( int );
  void slotCustColorSelected( int );
  void slotAddToCustom();
  void getHelp();

private:
  void readSettings();
  void writeSettings();
  void setRgbEdit();
  void setHsvEdit();

private:
  KColorCells *sysColorCells;
  KColorCells *custColorCells;
  QLineEdit *hedit;
  QLineEdit *sedit;
  QLineEdit *vedit;
  QLineEdit *redit;
  QLineEdit *gedit;
  QLineEdit *bedit;
  KColorPatch *patch;
  KHSSelector *palette;
  KValueSelector *valuePal;
  QColor selColor;
};

//----------------------------------------------------------------------------

class KColorCombo : public QComboBox
{
	Q_OBJECT
public:
	KColorCombo( QWidget *parent, const char *name = 0L );

	void setColor( const QColor &col );

public slots:
	void slotActivated( int index );
	void slotHighlighted( int index );

signals:
	void activated( const QColor &col );
	void highlighted( const QColor &col );

protected:
	virtual void resizeEvent( QResizeEvent *re );

private:
	void addColors();
	QColor customColor;
	QColor color;
};

#endif		// __KCOLORDLG_H__

