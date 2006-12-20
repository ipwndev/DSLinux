/*********************************************************************
  postproc_mvcol.hh
    Video frame post processor (Motionvectors).

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   20/Sep/2000 - Dirk Farin
     - Adaptation to new output architecture
   06/May/1999 - Dirk Farin
     - first implementation
 *********************************************************************/

#ifndef DVDVIEW_VPOSTPROC_PP_MVCOL_HH
#define DVDVIEW_VPOSTPROC_PP_MVCOL_HH

#include "libvideogfx/containers/array2.hh"
#include "vpostproc/postproc.hh"
#include "vpostproc/pp_mvcol.hh"

struct BlockColorInfo
{
  double hue,sat;
};


class VideoPostprocessor_MVCol : public VideoPostprocessor_Accumulate
{
public:
  VideoPostprocessor_MVCol()
    : d_showp(true), d_showb(false),
      d_hold(false), d_fwd(true), d_bkw(true) { }
  ~VideoPostprocessor_MVCol() { }

  void SetHoldMode(bool hold=false) { d_hold=hold; }
  void SelectMVs(bool showforw,bool showback) { d_fwd=showforw; d_bkw=showback; }
  void SelectFrametypes(bool p=true,bool b=false) { d_showp=p, d_showb=b; }

  bool NeedsPictureData(uint3 pictype) const;
  bool NeedsMBData(uint3 pictype) const;

  void BeginPicture(const DecodedImageData*);
  void ShowMBRows(DecodedImageData*);
  void FinishedPicture();

private:
  bool d_showp,d_showb;
  bool d_hold;
  bool d_fwd,d_bkw;

  bool d_fields;

  void RedrawHold(Pixel*const* yy,Pixel*const* cb,Pixel*const* cr,int cw,int ch);

  Array2<BlockColorInfo> d_blks;
};

#endif
