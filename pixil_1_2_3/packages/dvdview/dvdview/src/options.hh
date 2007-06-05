
#ifndef OPTIONS_HH
#define OPTIONS_HH

#include "types.hh"


struct Options
{
  Options();


  // Quality Flags

#if 0
  bool UseOnlyOneMVOfBiPred;    // This reduces the accuracy of the bidirectional prediction !
  bool NoHalfPelInB;            //  """
#endif

  // Warnings

#if 0
  bool WarnOnFalseMPEG1Fields;
#endif

  // Output options

#if 0
  bool Disable_X11Shm;
#endif
  bool Postproc_ShowMBs;
#if 0
  bool Postproc_ShowQ;
  bool Postproc_ShowMV;
  bool Postproc_ShowMVCol;
  bool Postproc_Mark;

  bool Postproc_ShowMV_Hold;
  bool Postproc_ShowMV_P;
  bool Postproc_ShowMV_B;
  bool Postproc_ShowMVCol_ShowPhase;

  int  FrameWait;  // Time to wait after displaying a frame (in 1/100s)
#endif
};

extern Options options;

#endif
