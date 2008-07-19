//-----------------------------------------------------------------------------
//
// kbanner - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#include <stdlib.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qscrbar.h>
#include <qcombo.h>
#include <qchkbox.h>
#include <qgrpbox.h>
#include <qmsgbox.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <kapp.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>

#include <qlayout.h>
#include <kbuttonbox.h>

#include "kflash.h"

#include "kflash.moc"

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static KFlashSaver *saver = NULL;

//-----------------------------------------------------------------------------
// standard screen saver interface functions
//
void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new KFlashSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	KFlashSetup dlg;

	return dlg.exec();
}

const char *getScreenSaverName()
{
	return glocale->translate("Flash Movies");
}

void exposeScreenSaver( int x, int y, int width, int height )
{
        if ( saver )
        {
                saver->expose( x, y, width, height );
        }
} 

//-----------------------------------------------------------------------------

KFlashSetup::KFlashSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	QString		 str;
	QLabel		*label;
	QPushButton	*button;

	readSettings();

	setCaption( klocale->translate("Setup kflash") );

	QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);
	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);	

	QHBoxLayout *tl2 = new QHBoxLayout;
	tl->addLayout(tl2);

	label = new QLabel( glocale->translate("Movie File:"), this );
	label->setMinimumSize( label->sizeHint());
	tl2->addWidget(label);

	fileWidget = new QLineEdit(this);
	fileWidget->setText(flashFile);
	fileWidget->setMinimumSize( fileWidget->sizeHint());
	fileWidget->setMinimumWidth( 250);
	connect( fileWidget, SIGNAL( textChanged(const char *) ), SLOT(slotSetFile(const char *) ) );
	tl2->addWidget(fileWidget);

	button = new QPushButton ( glocale->translate("Browse"), this);
	button->setMinimumSize( button->sizeHint());
	connect( button, SIGNAL( clicked() ), SLOT(slotFile() ) );
	tl2->addWidget(button);

	fullscreen = new QCheckBox ( glocale->translate("Fullscreen"), this);
	fullscreen->setMinimumSize( fullscreen->sizeHint());
	fullscreen->setChecked(fullScreen);
	tl->addWidget(fullscreen);

	sound = new QCheckBox ( glocale->translate("Enable sound"), this);
	sound->setMinimumSize( sound->sizeHint());
	sound->setChecked(enableSound);
	tl->addWidget(sound);

	KButtonBox *bbox = new KButtonBox(this);	
	button = bbox->addButton( glocale->translate("About"));
	connect( button, SIGNAL( clicked() ), SLOT(slotAbout() ) );

	button = bbox->addButton( glocale->translate("OK"));	
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton(glocale->translate("Cancel"));
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );

	bbox->addStretch(1);
	bbox->layout();

	tl->addWidget(bbox);

	tl->freeze();
}

void
KFlashSetup::slotOkPressed()
{
	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	config->writeEntry( "Movie", flashFile );

	if (sound->isChecked()) {
		config->writeEntry( "Sound", "yes" );
	} else {
		config->writeEntry( "Sound", "no" );
	}

	if (fullscreen->isChecked()) {
		config->writeEntry( "FullScreen", "yes" );
	} else {
		config->writeEntry( "FullScreen", "no" );
	}

	config->sync();

	accept();
}

void
KFlashSetup::slotSetFile(const char *text)
{
	flashFile = text;
}

void
KFlashSetup::slotFile()
{
	flashFile = QFileDialog::getOpenFileName();
	fileWidget->setText(flashFile);
}

void
KFlashSetup::slotAbout()
{
	QMessageBox::message(glocale->translate("About Flash"), 
			     glocale->translate("Flash Screen Saver. Version 1.0\n\nwritten by Olivier Debon\nodebon@club-internet.fr"), 
			     glocale->translate("OK"));
}

void
KFlashSetup::readSettings()
{
	QString str;

	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	str = config->readEntry( "Movie" );
	flashFile = str;

	str = config->readEntry( "FullScreen" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		fullScreen = TRUE;
	else
		fullScreen = FALSE;

	str = config->readEntry( "Sound" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		enableSound = TRUE;
	else
		enableSound = FALSE;
}

//-----------------------------------------------------------------------------

KFlashSaver::KFlashSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	flashHandle = NULL;
	fullScreen = 0;

	readSettings();
	initialise();

	if (flashHandle) {
		FlashGraphicInitX11(drawable);
	}

	timer.start( delay );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

KFlashSaver::~KFlashSaver()
{
	if (flashHandle) {
		FlashClose(flashHandle);
		XShmDetach(qt_xdisplay(), &segInfo);
		XSync(qt_xdisplay(),False);
		XFreePixmap(qt_xdisplay(), canvas);
		shmdt(segInfo.shmaddr);
	}

	timer.stop();
}

// read settings from config file
void
KFlashSaver::readSettings()
{
	QString str;

	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "Settings" );

	str = config->readEntry( "Movie" );
	flashFile = str;

	str = config->readEntry( "FullScreen" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		fullScreen = TRUE;
	else
		fullScreen = FALSE;

	str = config->readEntry( "Sound" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		enableSound = TRUE;
	else
		enableSound = FALSE;
}

long
KFlashSaver::FlashGraphicInitX11( Window w )
{
    XWindowAttributes wattr;
    XPixmapFormatValues *pf;
    Visual *visual;
    int nItems;
    int n;
    struct shmid_ds buf;
    int			 	 targetWidth;
    int 			 targetHeight;
    long			 bpl;	// Bytes per line
    long			 bpp;	// Bytes per pixel
    long			 pad;	// Scanline pad in byte
    // Platform dependent members
    Cursor		 	 buttonCursor;	// Window cursor (a hand if over a button)
    Display			*dpy;		// X11 Display
    GC		 	 gc;		// X11 Graphic context

    dpy = qt_xdisplay();
    target = d;

    // Get Window dimension
    XGetWindowAttributes(dpy, target, &wattr);

    // Get first visual, don't care about others, really !
    visual = wattr.visual;

#define PRINT 0
#if PRINT
	fprintf(stderr,"Id: %x\n", target);
	fprintf(stderr,"VisualId: %x\n", visual->visualid);
	fprintf(stderr,"BitmapPad  = %d\n", BitmapPad(dpy));
	fprintf(stderr,"BitmapUnit = %d\n", BitmapUnit(dpy));
	fprintf(stderr,"Depth      = %d\n", DefaultDepth(dpy,DefaultScreen(dpy)));
	fprintf(stderr,"RedMask    = %x\n", visual->red_mask);
	fprintf(stderr,"GreenMask  = %x\n", visual->green_mask);
	fprintf(stderr,"BlueMask   = %x\n", visual->blue_mask);
	fprintf(stderr,"Bits/RGB   = %d\n", visual->bits_per_rgb);
#endif

	bpp = 0;

	// Get screen info

	for(pf=XListPixmapFormats(dpy, &n); n--; pf++) {
		if (pf->depth == DefaultDepth(dpy, DefaultScreen(dpy))) {
			bpp = pf->bits_per_pixel/8;
			pad = pf->scanline_pad/8;
		}
#if PRINT
		fprintf(stderr,"----------------\n");
		fprintf(stderr,"Depth          = %d\n", pf->depth);
		fprintf(stderr,"Bits Per Pixel = %d\n", pf->bits_per_pixel);
		fprintf(stderr,"Scanline Pad   = %d\n", pf->scanline_pad);
#endif
	}

	gc = DefaultGC(dpy, DefaultScreen(dpy));

	// If fullscreen asked or movie is larger than window, fit the target
	if (fullScreen
	 || wattr.width<flashInfo.frameWidth/20
	 || wattr.height<flashInfo.frameHeight/20) {
		targetWidth = wattr.width;
		targetHeight = wattr.height;
		xOffset = 0;
		yOffset = 0;
	} else {
		targetWidth = flashInfo.frameWidth/20;
		targetHeight = flashInfo.frameHeight/20;
		xOffset = (wattr.width-targetWidth)/2;
		yOffset = (wattr.height-targetHeight)/2;
	}

#if PRINT
	fprintf(stderr,"Target Width  = %d\n", wattr.width);
	fprintf(stderr,"Target Height = %d\n", wattr.height);
#endif

	if (bpp) {
		bpl = (targetWidth*bpp + pad-1)/pad*pad;
	} else {
		bpl = (targetWidth/8 + pad-1)/pad*pad;
	}

	// Prepare data for Direct Graphics
	segInfo.readOnly = False;
	segInfo.shmid = shmget (IPC_PRIVATE,targetHeight*bpl,IPC_CREAT|0777);
	if (segInfo.shmid <0) {
		perror("shmget");
		fprintf(stderr,"Size = %d x %d\n", targetWidth, targetHeight);
	}
	segInfo.shmaddr = (char*)shmat (segInfo.shmid, 0, 0);
	if ((long)segInfo.shmaddr == -1) {
		perror("shmat");
	}
	XShmAttach(dpy, &segInfo);
#ifdef linux
	// Warning : this does NOT work properly on Solaris
	// Special Linux shm behaviour is used here
	// When number of attached clients falls down to zero
	// the shm is removed. This is convenient when it crashes.
	if (shmctl(segInfo.shmid, IPC_RMID, &buf) < 0) {
		perror("shmctl");
	}
#endif
	XSync(dpy, False);

	flashDisplay.pixels = (char*)segInfo.shmaddr;
        flashDisplay.width = targetWidth;
        flashDisplay.height = targetHeight;
        flashDisplay.bpl = bpl;
        flashDisplay.depth = DefaultDepth(dpy, DefaultScreen(dpy));
        flashDisplay.bpp = bpp;

	canvas = XShmCreatePixmap(dpy,target,segInfo.shmaddr,&segInfo,targetWidth,targetHeight,DefaultDepth(dpy, DefaultScreen(dpy)));
	XSync(dpy, False);

	XFlush(dpy);

        return FlashGraphicInit(flashHandle, &flashDisplay);
}

static int
readFile(const char *filename, char **buffer, long *size)
{
	FILE *in;
	char *buf;
	long length;

	if (filename == NULL) return -1;
	in = fopen(filename,"r");
	if (in == 0) {
		perror(filename);
		return -1;
	}
	fseek(in,0,SEEK_END);
	length = ftell(in);
	rewind(in);
	buf = (char *)malloc(length);
	fread(buf,length,1,in);
	fclose(in);

	*size = length;
	*buffer = buf;

	return length;
}

void KFlashSaver::initialise()
{
	char *buffer;
	long size;
	int status;

	if (readFile(flashFile, &buffer, &size) < 0) {
		return;
	}

	flashHandle = FlashNew();

	// Load level 0 movie
	do {
		status = FlashParse(flashHandle, 0, buffer, size);
	} while (status & FLASH_PARSE_NEED_DATA);

	free(buffer);

	FlashGetInfo(flashHandle, &flashInfo);

	delay = 1000/flashInfo.frameRate;

	FlashSettings(flashHandle, PLAYER_LOOP);

	if (enableSound) {
		FlashSoundInit(flashHandle, "/dev/dsp");
	}
}

void
KFlashSaver::FlashCopyX11(int all)
{
    XSetFunction(qt_xdisplay(),gc,GXcopy);
    if (all) {
	    XCopyArea(qt_xdisplay(),canvas,target,gc,
		      0,0,
		      flashDisplay.width,flashDisplay.height,
		      xOffset, yOffset
		      );
    } else {
	    XCopyArea(qt_xdisplay(),canvas,target,gc,
		      flashDisplay.clip_x,flashDisplay.clip_y,
		      flashDisplay.clip_width,flashDisplay.clip_height,
		      flashDisplay.clip_x + xOffset, flashDisplay.clip_y + yOffset
		      );
    }
    XFlush(qt_xdisplay());
}

void KFlashSaver::slotTimeout()
{
	int wakeUp;
	struct timeval tm;

	if (flashHandle == NULL) return;

	wakeUp = FlashExec(flashHandle, FLASH_WAKEUP, 0, &tm);
	if (flashDisplay.flash_refresh) {
		FlashCopyX11(0);
	}
}
