#include <config.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/stat.h>
#include <lct/local.h>
#include <lct/font.h>

/* defined later */
static void position_codepage(int requested_height, FILE *fpi);

/*@null@*/
simple_font* read_simple_font (FILE* fontfile, FILE* sfmfile, int font_format)
{
  struct stat stbuf;
  off_t filesize;
  int has_internal_sfm = 0;
  simple_font* the_font;

  /*
   * check arguments
   */
  
  /* fontfile is required */
  if (fontfile == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  /*
   * init
   */
  
  the_font = (simple_font*)calloc (1, sizeof(simple_font));
  if (!the_font) return NULL;

  /*
   * get filesize
   */
  
  /* FIXME: should not use _fileno ! */
  if (fstat(fontfile->_fileno, &stbuf) == -1)
    goto rsf_return_error;
  
  if (S_ISREG(stbuf.st_mode))
    filesize = stbuf.st_size;
  else
    {
      /* reading from pipe */
      errno = 0;
      filesize = -1;		       /* cannot know, so say we don't know */
    }

  
  /*
   * Position `fontfile' on fontdata for loading;
   * set relevant parameters accoring to font_format
   */
  switch (font_format)
    {
    case FFF_PSF:
      {
	struct psf_header psfhdr;

	if (fread(&psfhdr, sizeof(struct psf_header), 1, fontfile) != 1)
	  {
	    if (feof(fontfile))
	      errno = EBFONT; /* header should be complete */
	    goto rsf_return_error;
	  }

	/* this was already identified, or we shouldn't be there */
	assert (PSF_MAGIC_OK(psfhdr));

	if (! PSF_MODE_VALID(psfhdr.mode))
	  {
	    int saved_errno = errno;
	    fprintf(stderr, _("Unsupported psf file mode: %u.\n"), psfhdr.mode);
	    errno = saved_errno;
	    goto rsf_return_error;
	  }
	
	the_font->font.charcount = ((PSF_MODE_HAS512(psfhdr.mode)) ? 512 : 256);
	has_internal_sfm = (PSF_MODE_HASSFM(psfhdr.mode));
	the_font->font.charheight = psfhdr.charheight;
	the_font->font.charwidth = 8;

#if 0	/* this useless test is only possible on uncompressed files */
	{
	  int head = sizeof(struct psf_header)
	    + the_font->font.charcount*the_font->font.charheight;

	  if ((filesize > 0) && (head > filesize || (!has_internal_sfm && head != filesize)))
	    {
	      fprintf(stderr, _("Input file: bad length.\n"));
	      exit(1);
	    }
	}
#endif
	
	break;
      }

    /* CP is not a simple_font format, though we could emulate that */
    case FFF_CP:
      fprintf (stderr, _("CP is not a simple_font format.\n"));
      errno = EBFONT;
      goto rsf_return_error;
      
    case FFF_RAW:
#if 1	/* RAW filesize check */
      /* raw font */
      if ((filesize > 0) && (filesize & 0377)) 
	{
	  errno = EBFONT;
	  goto rsf_return_error;
	}
#endif
      if (filesize > 0)
	{
	  the_font->font.charheight = filesize/256;
	  the_font->font.charwidth = 8;
	  the_font->font.charcount = 256;
	}
      else
	{
	  fprintf (stderr, _("Cannot (yet) load a non-seekable RAW file\n"));
	  errno = EINVAL;
	  goto rsf_return_error;
	}
      break;
      
    default:
      fprintf (stderr, _("I can't handle this font-file format yet.\n"));
      errno = EINVAL;
      goto rsf_return_error;
    }  


  /*
   * read font bitmaps
   */

  /* allocate ( unit_max = 32 ) * (fontsize_max = 512)
   * FIXME: could be scaled down to (32 * fontsize) ? could that break PIO_FONT 
   * maybe 32 * min (fontsize, 256) ? */
  the_font->font.chardata = (char*)malloc (16384);

  fontdata_read_binary (fontfile, &(the_font->font) );

  /*
   * sanity check for file-size on old font-formats
   */
  if ((font_format == FFF_CP) || (font_format == FFF_RAW))
    {
      long old_pos = ftell (fontfile);
      
      xfseek (fontfile, 0, SEEK_END);
      
      if (((font_format == FFF_RAW) && (old_pos != ftell (fontfile))) ||
	  ((font_format == FFF_CP) && (ftell (fontfile) != 9780)))
	{
	  fprintf (stderr, _("Bad font input font-file length. Aborting.\n"));
	  exit (1);
	}
    }

  /* read provided SFM if there is one */
  if ((font_format == FFF_PSF) && has_internal_sfm)
    {
      if (-1 == sfm_read_binary (fontfile, &(the_font->sfm), the_font->font.charcount))
	goto rsf_return_error;

#ifndef NDEBUG
      /* if (verbose)*/
      fprintf(stderr, _("Reading SFM from font file.\n"));
#endif
    }

  return the_font; /* success */

rsf_return_error:
  {
    int saved_errno = errno;

    if (the_font->font.chardata)
      free (the_font->font.chardata);
    free (the_font);

    errno = saved_errno;
  }
  return NULL;
}


/*@unused@*/
static void position_codepage(int requested_height, FILE *fpi) 
{
  int offset;

  /* code page: first 40 bytes, then 8x16 font,
   * then 6 bytes, then 8x14 font,
   * then 6 bytes, then 8x8 font
   */

  /* if we're there we already read sizeof(psf_header) bytes. */
  
  if (!requested_height) 
    {
      fprintf(stderr,
	      _("This file contains 3 fonts: 8x8, 8x14 and 8x16. Please indicate\n"
		"using a --char-height option of 8, 14, or 16 which one you want to load.\n"));
      exit(1);
    }
  switch (requested_height) 
    {
    case 8:
      offset = 7732; break;
    case 14:
      offset = 4142; break;
    case 16:
      offset = 40; break;
    default:
      fprintf(stderr, 
	      _("You asked for font height %d, but only 8, 14, 16 are possible here.\n"),
	      requested_height);
      exit(1);
    }
  
  /* use SEEK_CUR to allow for pipes */
  if (xfseek(fpi, offset, SEEK_SET))
    {
      perror(_("seek error on input file"));
      exit(1);
    }
}
