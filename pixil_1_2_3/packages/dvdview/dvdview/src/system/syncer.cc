
#include "system/syncer.hh"

#include <iostream.h>
#include <time.h>


void Syncer_Realtime::Reset()
{
  d_locked=false;
}


void Syncer_Realtime::WaitUntil(PTS pts)
{
  // cout << "syncer received PTS: " << pts << endl;

  /* --------------- This code is a quick hack and probably has a lot of overflow problems.
  */

  struct timeval tv;
  gettimeofday(&tv,NULL);

  if (d_locked)
    {
      long long usec2wait = (pts-d_lastpts)*1000*100/90/d_speed;
      long long usec_passed = (tv.tv_sec -d_lasttime.tv_sec )*1000000 +
	                      (tv.tv_usec-d_lasttime.tv_usec);
      if (usec_passed > usec2wait)
	return;

      usec2wait -= usec_passed;
      struct timespec ts;
      ts.tv_sec  = usec2wait/1000000;
      ts.tv_nsec = (usec2wait%1000000)*1000;
      nanosleep(&ts,NULL);
      return;
    }  

  d_lastpts = pts;
  d_lasttime = tv;
  d_locked = true;
}

