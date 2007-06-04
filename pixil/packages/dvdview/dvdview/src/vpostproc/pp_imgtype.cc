
#include "vpostproc/pp_imgtype.hh"
#include "video12/constants.hh"

#include <iostream.h>


void VideoPostprocessor_ImageType::Render(DecodedImageData* dimg)
{
  Assert(dimg->m_picdata1);

  if (dimg->m_picdata1->m_picture_structure == PICSTRUCT_FramePicture)
    cout << "Frame ";
  else
    cout << "Field ";

  if (dimg->m_picdata1->m_picture_coding_type == PICTYPE_B)
    cout << "B\n";
  else if (dimg->m_picdata1->m_picture_coding_type == PICTYPE_P)
    cout << "P\n";
  else if (dimg->m_picdata1->m_picture_coding_type == PICTYPE_I)
    cout << "I\n";
  else
    cout << "D\n";
}
