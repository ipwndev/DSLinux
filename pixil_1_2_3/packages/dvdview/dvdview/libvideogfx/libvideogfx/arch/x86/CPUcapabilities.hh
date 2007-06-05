/********************************************************************************
  libvideogfx/arch/x86/CPUcapabilities.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
    09/Nov/1999 - Dirk Farin - First revision
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

#ifndef LIBVIDEOGFX_ARCH_X86_CPUCAPABILITIES_HH
#define LIBVIDEOGFX_ARCH_X86_CPUCAPABILITIES_HH

class CPU_Capabilities
{
public:
  virtual ~CPU_Capabilities() { }

  virtual bool HasMMX()   = 0;
  virtual bool HasKNI()   = 0;
  virtual bool Has3dNow() = 0;
  virtual bool HasMTRR()  = 0;
  virtual bool HasCMOV()  = 0;
  virtual bool HasFPU()   = 0;
};

extern CPU_Capabilities& cpucapabilities;

#endif
