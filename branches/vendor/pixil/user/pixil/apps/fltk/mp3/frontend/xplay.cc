/* Sound Player
   Portions Copyright (C) 2002, Century Embedded Technologies
   Original Copyright (C) 1997 by Woo-jae Jung 
*/

#include <pixil_config.h>

#include <pthread.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream.h>

#include "xplay.h"
#include "comm.h"

char *nullstring = "";
char *stopstring = "Stop";
char *nonestring = "None";

/*  common.cc playlist functions
extern char** splay_list;
extern int splay_listsize;
extern bool splay_shuffleflag;
extern bool splay_repeatflag;
*/

pthread_mutex_t startuplock;

extern class_musics musics;
extern class_music music;

/*******************/
/* Frame functions */
/*******************/
int
getslidernumber(int framenumber, int maxframe)
{
    register unsigned int a;

    a = MAXFRAMESLIDER * (unsigned int) framenumber;
    a /= (unsigned int) maxframe;

    return (int) a;
}

static int
getframenumber(int slidernum, int maxframe)
{
    register unsigned int a;

    a = (unsigned int) maxframe *(unsigned int) slidernum;
    a /= (unsigned int) MAXFRAMESLIDER;

    return (int) a;
}

static int
adjustframe(int slider, int maxframe)
{
    int f, s;

    if (maxframe == 0)
	return 0;
    f = getframenumber(slider, maxframe);
    for (;;) {
	s = getslidernumber(f, maxframe);
	if (s >= slider)
	    break;
	f++;
    }

    return f;
}



/*************************************/
/* Funcitions playing MPEG/Wave file */
/*************************************/
static void
seterrorcode(int errcode)
{
    musics.errorcode = errcode;
    musics.errorflag = true;
    music_done();
}

static void
xplaympeg(char *filename, Soundinputstream * loader, Rawplayer * player)
{

    //  cout << "xplaympeg()\n";

    Mpegtoraw *server;

    bool threadflag = 0;
    int panelupdate = 0;

    int pcmperframe, frequency;
    int updatedelay;

// Server
    if ((server = new Mpegtoraw(loader, player)) == NULL) {
	seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
	return;
    }

    server->initialize(filename);
    //  server->setforcetomono(splay_forcetomonoflag);
    //  server->setdownfrequency(splay_downfrequency);

    if (threadflag) {
	server->makethreadedplayer(50);

    }
    //  Setsongname   (server->getname());
    //  Setsongmusican(server->getartist());

    music.quit = music.pause = music.setframeflag = false;
    musics.feedback->Hz = 440;
    musics.feedback->SR = 44100;

    SetDsp(FEEDBACK);

    if (!server->run(-1))	// Initialize MPEG player
    {

	musics.Setcurrentmpegstatus(0, 0, 0, 0, "Srv Lives");
	if (threadflag)
	    server->freethreadedplayer();
	seterrorcode(server->geterrorcode());
	//    Paneldone();
	delete server;
	return;
    }

    pcmperframe = server->getpcmperframe();
    frequency = server->getfrequency();
    updatedelay = frequency / pcmperframe / 6;

    if (getquotaflag()) {
	//int q=server->getfrequency()*(server->isstereo()+1);
	/*
	   if(!splay_forcetomonoflag)q<<=1;
	   q>>=1;
	 */

	player->setquota(1024);
    }
    char title[1024];
    strcpy(title, "Error locating title");
    {
	ID3 data;
	parseID3(loader, &data);
	if (0 == strcmp("", data.songname)) {
	    strcpy(title, filename);
	} else {
	    sprintf(title, "%s - %s", data.artist, data.songname);
	}
    }

    musics.Setcurrentmpegstatus(server->getversion(), server->getlayer(),
				server->getfrequency(), server->getbitrate(),
				title);

    /*
       server->getcrccheck(),   server->isstereo(),
       server->getforcetomono(),server->getdownfrequency());
       Unlockframeslider();
     */

    for (;;) {


	SetDsp(FEEDBACK);

	if (GetDsp()) {
	    SetDspData((void *) (server->GetRawData()));
	}

	if (music.setframeflag) {
	    int a;

	    music.setframeflag = false;
	    panelupdate = updatedelay;
	    a = adjustframe(music.setframenumber, server->gettotalframe());
	    server->setframe(a);
	    /*
	       Unlockframeslider();
	       Setframestatus(server->getcurrentframe(),
	       0,
	       pcmperframe,frequency);
	     */
	    musics.pcmperframe = server->getpcmperframe();
	    musics.currentframe = server->getcurrentframe();
	    musics.maxframe = server->gettotalframe();
	    if (musics.volumeflag != 0) {
		//        Setvolume(musics.newvolume);
		musics.volumeflag = 0;
	    }
	}

	if (music.pause)	// When pause set,
	{
	    if (threadflag) {
		server->pausethreadedplayer();
		server->setframe(server->getcurrentframe());
	    }
	    server->clearbuffer();
	    while (music.pause)
		if (threadflag && (server->getframesaved() < (49))) {
		    server->run(1);
		    panelupdate = updatedelay;
		} else if (music.setframeflag) {
		    music.setframeflag = false;
		    server->setframe(adjustframe(music.setframenumber,
						 server->gettotalframe()));
		    /*
		       Setframestatus(server->getcurrentframe(),
		       0,
		       pcmperframe,frequency);
		       Unlockframeslider();
		     */
		    musics.pcmperframe = server->getpcmperframe();
		    musics.currentframe = server->getcurrentframe();
		    musics.maxframe = server->gettotalframe();
		    if (musics.volumeflag != 0) {
			//            Setvolume(musics.newvolume);
			musics.volumeflag = 0;
		    }
		} else
		    usleep(100);

	    if (music.setframeflag) {
		panelupdate = -1;
		music.setframeflag = false;
		server->setframe(adjustframe(music.setframenumber,
					     server->gettotalframe()));
	    }

	    if (threadflag)
		server->unpausethreadedplayer();
	}

	if (panelupdate < 0) {
	    /*
	       Setframestatus(server->getcurrentframe(),
	       server->gettotalframe(),
	       pcmperframe,frequency);
	     */

	    musics.pcmperframe = server->getpcmperframe();

	    musics.currentframe = server->getcurrentframe();

	    musics.maxframe = server->gettotalframe();

	    panelupdate = updatedelay;

	    if (musics.volumeflag != 0) {
		//       Setvolume(musics.newvolume);
		musics.volumeflag = 0;
	    }

	} else
	    panelupdate--;

	if (music.quit) {
	    if (threadflag) {
		server->stopthreadedplayer();
		while (server->existthread())
		    usleep(10);
	    }
	    player->abort();
	    player->resetsoundtype();
	    break;
	}

	if (!server->run(5)) {
	    if (!threadflag) {
		music_done();
		break;
	    }

	    if (server->getframesaved() == 0) {
		music_done();
		break;
	    }

	    usleep(100);

	}

    }

    if (threadflag)
	server->freethreadedplayer();

#ifdef CONFIG_DEBUG
    cout << "\tdone\n";
#endif

    //  Paneldone();
    delete server;
    return;
}

/*
static char *stripfilename(char *str)
{
  static char songname[30+1];

  char *ss;
  int p=0,s=0;

  for(;str[p];p++)
    if(str[p]=='/')
    {
      p++;
      s=p;
    }

  ss=str+s;
  for(p=0;p<30 && ss[p];p++)songname[p]=ss[p];
  songname[p]=0;

  return songname;
}


#define FRAMESIZE (4096)

static void xplaywave(char *filename,Soundinputstream *loader,Rawplayer *player)
{
  Wavetoraw *server;
  int frequency;
  
// Server
  if((server=new Wavetoraw(loader,player))==NULL)
  {
    seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
    return;
  }
  server->initialize();
  //  server->setforcetomono(splay_forcetomonoflag);

  //  Setsongname(stripfilename(filename));

  music.pause=music.quit=music.setframeflag=false;
  //  Unlockframeslider();

  if(!server->run())      // Initialize Wave player
  {
    //    Setcurrentwavestatus(0,0,0);
    seterrorcode(server->geterrorcode());
    //    Paneldone();
    delete server;
    return;
  }

  {
    int q=server->getfrequency()*(server->isstereo()+1);
    
    //if(server->getsamplesize()==16)
    //{
    //  if(!splay_forcetomonoflag)q<<=1;
    //}
    //q>>=1;
    

    player->setquota(q);
  }

  frequency=server->getfrequency();
 
 // Setcurrentwavestatus(server->getsamplesize(),
//		       frequency,
//		       server->isstereo());
  
  for(;;)
  {
    if(music.pause)
    {
      player->abort();player->resetsoundtype();
      while(music.pause)
      {
	if(music.setframeflag)
	{
	  music.setframeflag=false;
	  
	  //Setframestatus(server->getcurrentpoint()/FRAMESIZE,
	//		 0,
	//		 FRAMESIZE,frequency);
	  
	  server->setcurrentpoint(FRAMESIZE*
				  adjustframe(music.setframenumber,
					      server->gettotallength()/FRAMESIZE));
	  //	  Unlockframeslider();
	}
	usleep(100);
      }
    }

    if(music.setframeflag)
    {
      player->abort();player->resetsoundtype();
      music.setframeflag=false;
      server->setcurrentpoint(FRAMESIZE*
			      adjustframe(music.setframenumber,
					  server->gettotallength()/FRAMESIZE));
      
      //Unlockframeslider();
      //Setframestatus(server->getcurrentpoint()/FRAMESIZE,
	//		     0,
	//	     FRAMESIZE,frequency);
      
    }
    else
    {
      
      //Setframestatus(server->getcurrentpoint()/FRAMESIZE,
	//	     server->gettotallength()/FRAMESIZE,
	//	     FRAMESIZE,frequency);
      
    }

    if(music.quit)
    {
      player->getprocessed();
      player->abort();player->resetsoundtype();
      break;
    }

    if(!server->run())
    {
      music_done();
      break;
    }
  }

  //  Paneldone();
  delete server;
  return;
}
*/

static void
xplayfile(char *filename)
{

    char *device = Rawplayer::defaultdevice;
    Rawplayer *player;
    Soundinputstream *loader;

// Loader
    {
	int err;

	if (strstr(filename, "http://")) {

	    loader = new Soundinputstreamfromhttp();
	    if ((loader->open(filename)) == false) {
		seterrorcode(err);
		return;
	    }

	} else {

	    if ((loader = Soundinputstream::hopen(filename, &err)) == NULL) {
		seterrorcode(err);
		return;
	    }

	}



    }

// Player
    if (device == NULL)
	device = Rawplayer::defaultdevice;
    if (device[0] != '/')
	device = Rawplayer::defaultdevice;
    player = new Rawplayer;

    if (player == NULL) {
	seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
	return;
    }
    if (!player->initialize(device)) {
	seterrorcode(player->geterrorcode());
	return;
    }
// Select which
//  if(strstr(filename,".mp") || strstr(filename,".MP"))
    xplaympeg(filename, loader, player);
//  else
//    xplaywave(filename,loader,player);

    //  Clearcurrentstatus();


// Clean up
    delete loader;
    delete player;
}

/**********/
/* Player */
/**********/
static void
xplay()
{
    bool updateflag = true;	// Control music_stop and music_move;

    //  printf("Welcome to the thread!\n");

    musics.restart = true;
    musics.errorflag = false;
    pthread_mutex_init(&musics.movelock, NULL);

    for (;;) {
	if (musics.restart) {
#ifdef CONFIG_DEBUG
	    printf("THREAD:  In restart\n");
#endif
	    musics.stop = true;
	    musics.restart = false;
	    musics.currentrun = 0;
	    musics.move = 0;
	    updateflag = true;
#ifdef CONFIG_DEBUG
	    printf("THREAD: Out restart\n");
#endif
	}

	if (musics.errorflag) {
#ifdef CONFIG_DEBUG
	    printf("THREAD:  In errorflag\n");
#endif
	    musics.errorflag = false;
#ifdef CONFIG_DEBUG
	    printf("THREAD: Out errorflag\n");
#endif
	}

	if (musics.move != 0) {
#ifdef CONFIG_DEBUG
	    printf("THREAD: In move\n");
#endif
	    pthread_mutex_lock(&musics.movelock);
#ifdef CONFIG_DEBUG
	    printf("THREAD: In move pthread_mutex_lock\n");
#endif
	    musics.currentrun += musics.move;

	    if (musics.currentrun >= 1) {

		musics.restart = true;
		musics.currentrun = 1;
		musics.stop = true;

	    }

	    if (musics.currentrun < 0) {

		musics.currentrun = 0;

	    }

	    musics.move = 0;
#ifdef CONFIG_DEBUG
	    printf("THREAD: In move about to pthread_mutex_unlock\n");
#endif
	    pthread_mutex_unlock(&musics.movelock);
	    updateflag = true;
#ifdef CONFIG_DEBUG
	    printf("THREAD: Out move\n");
#endif
	}

	if (updateflag) {

	    //char *str;
#ifdef CONFIG_DEBUG
	    printf("THREAD: In updateflag=false\n");
#endif
	    updateflag = false;
#ifdef CONFIG_DEBUG
	    printf("THREAD: Out updateflag=false\n");
	    printf("musics.stop value: %d\n", musics.stop);
#endif
	}


	if (musics.stop) {
	    usleep(500);	// Don't play
	} else {

#ifdef CONFIG_DEBUG
	    printf("THREAD: In play\n");
#endif

	    if (musics.currentrun < 1) {
		musics.tracksplayed++;
		char *fname = 0;
#ifdef CONFIG_DEBUG
		printf("THREAD: In play, before GetPlayList()...\n");
#endif
		fname = GetPlayList();
#ifdef CONFIG_DEBUG
		printf("THREAD: In play, Playing file %s\n", fname);
#endif
		xplayfile(fname);
		updateflag = true;

#ifdef CONFIG_DEBUG
		printf("THREAD: Out play\n");
#endif

	    } else {

#ifdef CONFIG_DEBUG
		printf("THREAD: In music.restart=true\n");
#endif
		musics.restart = true;
#ifdef CONFIG_DEBUG
		printf("THREAD: Out music.restart=true\n");
#endif
	    }

	}

    }

}

void
Setframe(int frame)
{
    music.setframenumber = frame;
    music.setframeflag = true;
}

static bool quotaflag = false;
bool
getquotaflag(void)
{
    return quotaflag;
}

void
setquotaflag(bool flag)
{
    quotaflag = flag;
}

/*********************************/
/* Initialize and main functions */
/*********************************/
/*
inline void xinit(int& argc,char *argv[])
{
  pthread_mutex_init(&startuplock,NULL);
  pthread_mutex_lock(&startuplock);
  //  Getxarg(argc,argv);
}
*/
void *
_startup(void *)
{

    pthread_mutex_lock(&startuplock);
    musics.stop = true;
    xplay();

    return NULL;
}

/*
inline int xmain(void)
{
  pthread_t th;

  pthread_create(&th,NULL,_startup,NULL);
  SetupPanel();
  pthread_mutex_unlock(&startuplock);

  return RunPanel();
}
*/

/***********************/
/* Command line player */
/***********************/
/*
inline void error(int n)
{
  fprintf(stderr,"%s: %s\n",splay_progname,splay_Sounderrors[n-1]);
  return;
}
*/
/*
int main(int argc,char *argv[])
{
  int c;

  splay_progname=argv[0];

  xinit(argc,argv);

  while((c=getopt(argc,argv,
		  "V2emrsvd:l:t:"
		  ))>=0)
  {
    switch(c)
    {
      case 'V':printf("x%s %s\n",PACKAGE,VERSION);
               return 0;

      case '2':splay_downfrequency  =   1;break;
      case 'e':splay_exitwhendone   =true;break;
      case 'm':splay_forcetomonoflag=true;break;
      case 'r':splay_repeatflag     =true;break;
      case 's':splay_shuffleflag    =true;break;
      case 'v':splay_verbose++;           break;

      case 'd':splay_devicename=optarg;break;
      case 'l':if(splay_verbose)
		 fprintf(stderr,"List file : %s\n",optarg);
	       readlist(optarg);
	       break;

      default:fprintf(stderr,"Bad argument.\n");
    }
  }

  if(splay_listsize==0)    // Make list by arguments
    arglist(argc,argv,optind);

  return xmain();
}
*/
