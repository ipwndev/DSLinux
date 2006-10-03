#include "qfontinfo.h"

QFontInfo::QFontInfo(QFont & qf)
{
  m_family = qf.family();
  m_italic = qf.italic();
  m_weight = qf.weight();
  m_charSet = qf.charSet();
}

QString QFontInfo::family()
{
  return(m_family.data());
}

bool QFontInfo::italic()
{
  return(m_italic);
}

static char *fixed_pitch_list[] =
{
  "courier",
  NULL
};

bool QFontInfo::fixedPitch()
{
  int count;

  count = 0;
  while(fixed_pitch_list[count])
  {
    if(strcmp(m_family.data(),fixed_pitch_list[count++]) == 0)
      return(true);
  }
  return(false);
}

bool QFontInfo::bold()
{
  if(m_weight > 50)
    return(true);
  else
    return(false);
}

QFont::CharSet QFontInfo::charSet()
{
  return(m_charSet);
}
