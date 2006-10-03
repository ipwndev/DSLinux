#include "qcolor.h"
#include "string.h"
#include <stdlib.h>

typedef struct
{
  char *name;
  unsigned char red;
  unsigned char green;
  unsigned char blue;
}ColorEntry;

static ColorEntry color_list[] =
{
  { "black",0,0,0},
  { "silver",192,192,192},
  { "gray",128,128,128},
// CRH
  { "lightgray",211,211,211},
  { "white",255,255,255},
  { "maroon",128,0,0},
  { "red",255,0,0},
  { "purple",128,0,128},
  { "fuchsia",255,0,255},
  { "green",0,128,0},
// CRH
  { "lightgreen",144,238,144},
  { "lime",0,255,0},
  { "olive",128,128,0},
  { "yellow",255,255,0},
// CRH
  { "lightyellow",255,255,224},
  { "navy",0,0,128},
  { "blue",0,0,255},
// CRH
  { "lightblue",173,216,230},
  { "teal",0,128,128},
  { "aqua",0,255,255},
  { NULL,0,0,0 }
};

static int _fromhex(const char * h)
{
  return strtol(h,0,16);
}


void QColor::setNamedColor(const QString & color)
{
  char * c = (char*)color.data();

  m_nRed = m_nGreen = m_nBlue = 0;

  if(c[0] == '#')
 {
    int n = strlen(c+1) / 3;
    char buf[5];
    strncpy(buf,c+1,n);
    buf[n] = 0;
    m_nRed = _fromhex(buf);

    strncpy(buf,c+1+n,n);
    buf[n] = 0;
    m_nGreen = _fromhex(buf);

    strncpy(buf,c+1+n+n,n);
    buf[n] = 0;
    m_nBlue = _fromhex(buf);

    m_bSet = true;
  }
  else
  {
    color.lower();
    int count = 0;
    while(color_list[count].name)
    {
      if(strcmp(color_list[count].name,(char *)color) == 0)
      {
	m_nRed = color_list[count].red;
	m_nGreen = color_list[count].green;
	m_nBlue = color_list[count].blue;
	m_bSet = true;
	break;
      }
      ++count;
    }
  }
}
