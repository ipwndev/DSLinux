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

#include "video12/viddec_mods.hh"
//#include "video12/modules/idct_std.hh"
//#include "video12/modules/mcomp_std.hh"
//#include "video12/modules/mcomp_scalar.hh"
//#include "video12/modules/mcomp_scalar_unaligned.hh"
#include "video12/modules/mcomp_sgl_simple.hh"

#if ENABLE_MMX
//#include "video12/modules/mcomp_mmx.hh"
#include "video12/modules/mcomp_sgl_mmx.hh"
#include "libvideogfx/arch/x86/CPUcapabilities.hh"
#endif


#if 0
MotionCompensation* MotionCompensation::Create()
{
#if 0 //ENABLE_MMX
  if (cpucapabilities.HasMMX())
    return new MotionCompensation_MMX;
#endif

#if 0 //UNALIGNED_MEMORYACCESS
  return new MotionCompensation_Scalar_Unaligned;
#endif

  return new MotionCompensation_Scalar;

  return new MotionCompensation_Standard;
}
#endif

#include <iostream.h>

MotionCompensation_SglMB* MotionCompensation_SglMB::Create()
{
#if ENABLE_MMX
  if (cpucapabilities.HasMMX())
    return new MotionCompensation_SglMB_MMX;
#endif

  return new MotionCompensation_SglMB_Simple;
}

