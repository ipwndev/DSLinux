/*********************************************************************
  idct_arai.hh

  purpose:
    Perform IDCT using AAN-algorithm.

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   30/Dec/98 - Dirk Farin
     - Extracted the main IDCT code from the encoder.
 *********************************************************************/

#ifndef IDCT_ARAI_HH
#define IDCT_ARAI_HH

#include "types.hh"

void IDCT_Int (const short* in[8], short* out[8]);
void IDCT_Int2(      short* in[8], short* out[8]);
void IDCT_Int2b(     short* in, Pixel* out[8],bool add);

#endif
