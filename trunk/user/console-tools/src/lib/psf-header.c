#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <lct/local.h>
#include <lct/font.h>

int psf_header_read (FILE* in, struct psf_header* psfhdr_ptr)
{
  if ( fread(psfhdr_ptr, sizeof(struct psf_header), 1, in) < 1 )
    {
      if (feof(in))
	  errno = EBFONT;
      return -1;
    }

  if (! PSF_MAGIC_OK(*psfhdr_ptr) )
    {
      errno = EBFONT;
      return -1;
    }

  if (! PSF_MODE_VALID(psfhdr_ptr->mode) )
    {
      fprintf(stderr, _("Unknown PSF mode number (%d).\n"), psfhdr_ptr->mode);
      errno = EBFONT;
      return -1;
    }
  
  return 0;
}

int psf_header_write (FILE* out, struct psf_header* psfhdr_ptr)
{
  int r;
  
  r = fwrite(psfhdr_ptr, sizeof(struct psf_header), 1, out);
  if (r < 1)
      return -1;
  
  return 0;
}

int write_as_psf_header (FILE* out, 
			 int charheight,
			 int charcount,
			 int with_sfm)
{
  struct psf_header psfhdr;

  /* sanity check - allows to drop `defaults:' from switch constructs */
  if (((charcount != 256) && (charcount != 512)) ||
      ((with_sfm != 0) && (with_sfm != 1)))
    {
      errno = EINVAL;
      return -1;
    }
  
  psfhdr.magic1 = PSF_MAGIC1;
  psfhdr.magic2 = PSF_MAGIC2;

  psfhdr.charheight = charheight;

  /* choose psfhdr.mode according to args */
  switch (with_sfm)
    {
    case 0:
      switch (charcount)
	{
	case 256:
	  psfhdr.mode = PSF_MODE256NOSFM;
	  break;
	case 512:
	  psfhdr.mode = PSF_MODE512NOSFM;
	  break;
	}
      break;
    case 1:
      switch (charcount)
	{
	case 256:
	  psfhdr.mode = PSF_MODE256SFM;
	  break;
	case 512:
	  psfhdr.mode = PSF_MODE512SFM;
	  break;
	}
      break;
    }
  
  return psf_header_write (out, &psfhdr);
}
