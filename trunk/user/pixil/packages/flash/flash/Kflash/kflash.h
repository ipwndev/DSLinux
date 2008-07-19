//-----------------------------------------------------------------------------
//
// kflash - Flash screen saver for KDE
//
// Copyright (c)  Olivier Debon 2000
//

#ifndef __KFLASH_H__
#define __KFLASH_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlined.h>
#include <qpushbt.h>
#include "saver.h"
#include "flash.h"


class KFlashSaver : public kScreenSaver
{
	Q_OBJECT
public:
	KFlashSaver( Drawable drawable );
	virtual ~KFlashSaver();

private:
	void readSettings();
	void initialise();
	void blank();
	long FlashGraphicInitX11( Window w );
	void FlashCopyX11(int all);
        XShmSegmentInfo	 segInfo;	// Shared memory information

protected slots:
	void slotTimeout();

protected:
	QTimer		timer;
	long		delay;
	FlashDisplay	flashDisplay;
	FlashInfo	flashInfo;
	FlashHandle 	flashHandle;
	Window		target;
	Pixmap		canvas;
	long		xOffset;
	long		yOffset;
	//Settings
	QString		flashFile;
	int		fullScreen;
	int		enableSound;
};


class KFlashSetup : public QDialog
{
	Q_OBJECT
public:
	KFlashSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotAbout();
	void slotFile();
	void slotOkPressed();
	void slotSetFile(const char *);

private:
	QLineEdit	*fileWidget;
	QCheckBox	*sound;
	QCheckBox	*fullscreen;
	//Settings
	QString		flashFile;
	int		fullScreen;
	int		enableSound;
};

#endif

