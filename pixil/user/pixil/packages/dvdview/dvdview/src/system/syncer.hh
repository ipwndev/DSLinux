/********************************************************************************
  system/syncer.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
    03/Sep/2000 - Dirk Farin
     - adjustable playback-speed
    08/Apr/2000 - Dirk Farin
     - first revision
 ********************************************************************************
    Copyright (C) 1999  Dirk Farin

    This program is distributed under GNU Public License (GPL) as
    outlined in the COPYING file that comes with the source distribution.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************************************/

#ifndef DVDVIEW_SYSTEM_SYNCER_HH
#define DVDVIEW_SYSTEM_SYNCER_HH

#include "system/system1.hh"
#include <sys/time.h>
#include <unistd.h>


class Syncer
{
public:
  virtual ~Syncer() { }

  virtual void Reset() { }
  virtual void WaitUntil(PTS) = 0;
};


class Syncer_AsFastAsPossible : public Syncer
{
public:
  void WaitUntil(PTS) { /* Return immediately without waiting. */ }
};


class Syncer_Realtime : public Syncer
{
public:
  Syncer_Realtime() : d_speed(100) { Reset(); }

  void Reset();
  void WaitUntil(PTS);
  void SetSpeed(int percent) // 100% - real-time / >100% - faster / <100% - slower
  { assert(percent>0); d_speed=percent; } // 100% - real-time / >100% - faster / <100% - slower

private:
  bool d_locked;
  PTS  d_lastpts;
  struct timeval d_lasttime;
  int  d_speed;
};

#endif
