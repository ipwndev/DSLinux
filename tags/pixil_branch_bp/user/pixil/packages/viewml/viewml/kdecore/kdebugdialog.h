/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
#ifndef _KDEBUGDIALOG
#define _KDEBUGDIALOG

#include <qdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

class KDebugDialog : public QDialog
{
  Q_OBJECT

private:
  QGroupBox* pInfoGroup;
  QLabel* pInfoLabel1;
  QComboBox* pInfoCombo;
  QLabel* pInfoLabel2;
  QLineEdit* pInfoFile;
  QLabel* pInfoLabel3;
  QLineEdit* pInfoShow;
  QGroupBox* pWarnGroup;
  QLabel* pWarnLabel1;
  QComboBox* pWarnCombo;
  QLabel* pWarnLabel2;
  QLineEdit* pWarnFile;
  QLabel* pWarnLabel3;
  QLineEdit* pWarnShow;
  QGroupBox* pErrorGroup;
  QLabel* pErrorLabel1;
  QComboBox* pErrorCombo;
  QLabel* pErrorLabel2;
  QLineEdit* pErrorFile;
  QLabel* pErrorLabel3;
  QLineEdit* pErrorShow;
  QGroupBox* pFatalGroup;
  QLabel* pFatalLabel1;
  QComboBox* pFatalCombo;
  QLabel* pFatalLabel2;
  QLineEdit* pFatalFile;
  QLabel* pFatalLabel3;
  QLineEdit* pFatalShow;

  QCheckBox* pAbortFatal;
  QPushButton* pOKButton;
  QPushButton* pCancelButton;
  QPushButton* pHelpButton;

public:
  KDebugDialog();
  ~KDebugDialog();

  void setInfoOutput( int n )
	{ pInfoCombo->setCurrentItem( n ); }
  int infoOutput() const
	{ return pInfoCombo->currentItem(); }
  void setInfoFile( const char* pFileName )
	{ pInfoFile->setText( pFileName ); }
  const char* infoFile() const
	{ return pInfoFile->text(); }
  void setInfoShow( const char* pShowString )
	{ pInfoShow->setText( pShowString ); }
  const char* infoShow() const
	{ return pInfoShow->text(); }
  void setWarnOutput( int n )
	{ pWarnCombo->setCurrentItem( n ); }
  int warnOutput() const
	{ return pWarnCombo->currentItem(); }
  void setWarnFile( const char* pFileName )
	{ pWarnFile->setText( pFileName ); }
  const char* warnFile() const
	{ return pWarnFile->text(); }
  void setWarnShow( const char* pShowString )
	{ pWarnShow->setText( pShowString ); }
  const char* warnShow() const
	{ return pWarnShow->text(); }
  void setErrorOutput( int n )
	{ pErrorCombo->setCurrentItem( n ); }
  int errorOutput() const
	{ return pErrorCombo->currentItem(); }
  void setErrorFile( const char* pFileName )
	{ pErrorFile->setText( pFileName ); }
  const char* errorFile() const
	{ return pErrorFile->text(); }
  void setErrorShow( const char* pShowString )
	{ pErrorShow->setText( pShowString ); }
  const char* errorShow() const
	{ return pErrorShow->text(); }
  void setFatalOutput( int n )
	{ pFatalCombo->setCurrentItem( n ); }
  int fatalOutput() const
	{ return pFatalCombo->currentItem(); }
  void setFatalFile( const char* pFileName )
	{ pFatalFile->setText( pFileName ); }
  const char* fatalFile() const
	{ return pFatalFile->text(); }
  void setFatalShow( const char* pShowString )
	{ pFatalShow->setText( pShowString ); }
  const char* fatalShow() const
	{ return pFatalShow->text(); }
  void setAbortFatal( bool bAbort )
	{ pAbortFatal->setChecked( bAbort ); }
  bool abortFatal() const
	{ return pAbortFatal->isChecked(); }

public slots:
  void showHelp();

private:
  // Disallow assignment and copy-construction
  KDebugDialog( const KDebugDialog& );
  KDebugDialog& operator= ( const KDebugDialog& );
};

#endif
