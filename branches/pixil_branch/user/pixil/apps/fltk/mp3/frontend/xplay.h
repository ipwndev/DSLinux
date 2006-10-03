/* Sound Player (X-interface)

   Portions Copyright (C) 2002 Century Embedded Technologies
   Copyright (C) 1997 by Woo-jae Jung */

#include <mpegsound.h>

#define MAXFRAMESLIDER 2000

// Strings used frequently
extern char *stopstring, *nonestring, *nullstring;

//
// Functions.cc
//
extern bool exitwhendone;

//
// Xplay.cc
//

// frame managemant
void Setframe(int frame);
int getslidernumber(int frame, int maxframe);

// misc
bool getquotaflag(void);
void setquotaflag(bool flag);

// start up function
void *_startup(void *);
