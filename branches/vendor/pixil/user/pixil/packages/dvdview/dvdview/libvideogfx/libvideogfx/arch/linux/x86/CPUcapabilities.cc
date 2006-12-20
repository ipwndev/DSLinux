/********************************************************************************
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

#include "config.h"

#include <fstream.h>
#include <string.h>

#include "libvideogfx/arch/x86/CPUcapabilities.hh"


class CPU_Capabilities_LinuxProc : public CPU_Capabilities
{
public:
  CPU_Capabilities_LinuxProc()
    {
      mmx=kni=AMD3dnow,mtrr=cmov=fpu=false;

      char buf[500+1];
      ifstream str("/proc/cpuinfo");
      if (str)
        {
          while (!str.eof())
            {
              str.getline(buf,500);
              if (strncmp(buf,"flags",5)==0)
                {
                  if (strstr(buf,"mmx"))   mmx =true;
                  if (strstr(buf,"kni"))   kni =true;
                  if (strstr(buf,"mtrr"))  mtrr=true;
                  if (strstr(buf,"cmov"))  cmov=true;
                  if (strstr(buf,"3dnow")) AMD3dnow=true;
                  if (strstr(buf,"fpu"))   fpu =true;
                }
            }
        }
    }

  bool HasMMX() { return mmx; }
  bool HasKNI() { return kni; }
  bool Has3dNow() { return AMD3dnow; }
  bool HasMTRR() { return mtrr; }
  bool HasCMOV() { return cmov; }
  bool HasFPU()  { return fpu; }

private:
  bool mmx,kni,AMD3dnow,mtrr,cmov,fpu;
};

static CPU_Capabilities_LinuxProc cpucap;
CPU_Capabilities& cpucapabilities = cpucap;
