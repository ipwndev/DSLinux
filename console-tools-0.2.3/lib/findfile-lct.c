#include <config.h>

#include <lct/local.h>
#include <lct/font.h>
#include <lct/console.h>

static const char *mapdirpath[] = { 
  "", 
  DATADIR "/" TRANSDIR "/", 
#ifdef LOCALDATADIR
  LOCALDATADIR "/" TRANSDIR "/",
#endif
  0 };
static const char *mapsuffixes[] = {
  "", ".acm",
  0 };
static const char *sfmsuffixes[] = {
  "", ".sfm", ".uni",
  0 };
static const char *sfmfallbacksuffixes[] = {
  "", ".fallback",
  0 };


static const char *fontdirpath[] = { "", DATADIR "/" FONTDIR "/", 
#ifdef LOCALDATADIR
  LOCALDATADIR "/" FONTDIR "/", 
#endif
  0 };
static const char *fontsuffixes[] = { 
  "", ".psf", ".cp", ".fnt", ".psfu", 
  0 };

static const char *keymapdirpath[] =
{
  "", DATADIR "/" KEYMAPDIR "/**/", 
#ifdef LOCALDATADIR
  LOCALDATADIR "/" KEYMAPDIR "/**/",
#endif
  KERNDIR "/", 
  0 };
static const char *keymapsuffixes[] = { 
  "", ".kmap", ".map",
  0 };

static const char *videomodedirpath[] = { "", DATADIR "/" VIDEOMODEDIR "/", 
#ifdef LOCALDATADIR
  LOCALDATADIR "/" VIDEOMODEDIR "/", 
#endif
  0};
static const char *videomodesuffixes[] = { "", 0 };


static const struct magic __ff_magics__[] =
{
    { FFF_PSF,	"\066\004",	"\377\377", 2 },
    { FFF_XPSF,	"\066\005",	"\377\377", 2 },
    { FFF_CP,	"\000\000\000\000\000\000\001\000EGA     ",
      "\000\000\000\000\000\000\377\377\377\377\377\377\377\377\377\377", 16 },
    { FFF_CP,	"\000\000\000\000\000\000\001\000VIDEO   ",
      "\000\000\000\000\000\000\377\377\377\377\377\377\377\377\377\377", 16 },
  
  /* default when no magic: */
    { FFF_RAW, NULL, NULL, 0 },
  
  /* terminator */
    { FF_END, NULL, NULL, 0 }
};

/* font-file magics */
const struct magicset ff_magics =
{
  32,				       /* max magic length: more than enough */
  __ff_magics__
};


/* template for wrappers around findfont */
#define FINDFONT_WRAPPER(NAME, PATH, SUFFIXES, MAGICS) \
FINDFONT_WRAPPER_HEADER(NAME) \
{ \
  return findfile(fnam, PATH, SUFFIXES, fullname, maxfullength, minus_meaning, MAGICS, magic_return); \
}

/* wrappers */

FINDFONT_WRAPPER(font,		fontdirpath,	fontsuffixes,		&ff_magics)
FINDFONT_WRAPPER(acm,		mapdirpath,	mapsuffixes,		NULL)
FINDFONT_WRAPPER(sfm,		mapdirpath,	sfmsuffixes,		NULL)
FINDFONT_WRAPPER(sfmfallback,	mapdirpath,	sfmfallbacksuffixes,	NULL)
FINDFONT_WRAPPER(keymap,	keymapdirpath,	keymapsuffixes,		NULL)
FINDFONT_WRAPPER(videomode,   videomodedirpath, videomodesuffixes,	NULL)
