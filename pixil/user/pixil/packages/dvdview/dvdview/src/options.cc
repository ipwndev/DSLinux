
#include "options.hh"

Options options;

Options::Options()
{
  // Quality
#if 0
  options.UseOnlyOneMVOfBiPred = false;
  options.NoHalfPelInB         = false;

  // Warnings

  options.WarnOnFalseMPEG1Fields = false;


  // Output options

  options.Disable_X11Shm     = false;
#endif
  options.Postproc_ShowMBs   = false;
#if 0
  options.Postproc_ShowQ     = false;
  options.Postproc_ShowMV    = false;
  options.Postproc_ShowMVCol = false;
  options.Postproc_Mark      = false;

  options.Postproc_ShowMV_Hold = false;
  options.Postproc_ShowMV_P    = false;
  options.Postproc_ShowMV_B    = false;
  options.Postproc_ShowMVCol_ShowPhase = false;

  options.FrameWait=0;
#endif
}

