#include <config.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <lct/font.h>
#include <lct/console.h>

#define MAXFONTSIZE 512

static void height_adjust(cfontdesc* cfd)
{
  unsigned i;

  if (cfd->charwidth != 8)
    return;

  for (cfd->charheight = 32; cfd->charheight > 0; cfd->charheight--)
    for (i = 0; i < cfd->charcount; i++)
      if (cfd->chardata[32*i+cfd->charheight-1])
	return;
}

/*
 * get kernel font, trying to use, in this order:
 * - KDFONTOP (linux 2.1.106 and later)
 * - GIO_FONTX
 * - GIO_FONT
 */
cfontdesc* get_kernel_font(int fd)
{
  int retval;
  int saved_errno = 0;
  cfontdesc* cfd_ret;

  /* FIXME: should be handled to some extent by autoconf */
#if !defined(KDFONTOP) && !defined(GIO_FONTX) && !defined(GIO_FONT)
# error Cannot get kernel font
#endif
  
  /* allocate the return struct */
  cfd_ret = (cfontdesc*)calloc(1, sizeof(cfontdesc));
  
  /*
   * try KDFONTOP
   */
  
#ifdef KDFONTOP
  {
    struct console_font_op cfo;

# ifndef NDEBUG
    fprintf (stderr, "Trying KDFONTOP.\n");
# endif
    
    cfo.data = (char *)malloc (32*MAXFONTSIZE*4);
    cfo.charcount = MAXFONTSIZE;
    cfo.width = 32; cfo.height = 32;
    cfo.flags = 0; cfo.op = KD_FONT_OP_GET;

    retval = ioctl (fd, KDFONTOP, &cfo);
    if ((retval == -1) && (errno != EINVAL) && (errno != ENOSYS))
      {
	saved_errno = errno;
	free(cfo.data);
	errno = saved_errno;
	return NULL;
      }
  
    if (retval == 0)
      {
	cfd_ret->charcount = cfo.charcount;
	cfd_ret->charwidth = cfo.width;
	cfd_ret->charheight = cfo.height;
	cfd_ret->chardata = cfo.data;
	return cfd_ret;
      }
  }
#endif /* KDFONTOP */

  /*
   * try GIO_FONTX
   */

#ifdef GIO_FONTX
  {
    struct consolefontdesc cfd;

# ifndef NDEBUG
    fprintf (stderr, "Trying GIO_FONTX.\n");
# endif
    
    cfd.charheight = 0;
    cfd.chardata = (char*)malloc (32*MAXFONTSIZE);
    cfd.charcount = MAXFONTSIZE;

    retval = ioctl (fd, GIO_FONTX, &cfd);
    if ((retval == -1) && (errno != EINVAL))
      {
	saved_errno = errno;
	free (cfd.chardata);
	errno = saved_errno;
	return NULL;
      }

    if (retval == 0)
      {
	cfd_ret->charcount = cfd.charcount;
	cfd_ret->charwidth = 8;
	cfd_ret->charheight = cfd.charheight;
	cfd_ret->chardata = cfd.chardata;
	return cfd_ret;
      }
  }
#endif /* GIO_FONTX */
  
  /*
   * try GIO_FONT
   */

#ifdef GIO_FONT
  cfd_ret->chardata = (char*)malloc (32*256);
  cfd_ret->charcount = 256;
  cfd_ret->charwidth = 8;

# ifndef NDEBUG
    fprintf (stderr, "Trying GIO_FONT.\n");
# endif
    
  retval = ioctl (fd, GIO_FONT, cfd_ret->chardata);
  if (retval == -1)
    {
      saved_errno = errno;
      free (cfd_ret->chardata);
      errno = saved_errno;
      return NULL;
    }

  /* As we had to rely on GIO_FONT, find the most efficient char-height
   * from the data we got.
   * FIXME: should this be set to console cell-height ?
   */
  height_adjust(cfd_ret);

  return cfd_ret;
#endif /* GIO_FONT */
}


/*
 * set kernel font, trying to use, in this order:
 * - KDFONTOP (linux 2.1.106 and later)
 * - PIO_FONTX
 * - PIO_FONT
 */
int set_kernel_font(int fd, cfontdesc *cfd)
{
  /* FIXME: should be handled to some extent by autoconf */
#if !defined(KDFONTOP) && !defined(PIO_FONTX) && !defined(PIO_FONT)
# error Cannot set kernel font
#endif
  
  /* allow height autodetection for width 8 */
  /* Who uses this, and what for ?? */
  if (!cfd->charheight)
    {
      if (cfd->charwidth != 8)
	{
	  errno = EINVAL;
	  return -1;
	}
      height_adjust(cfd);
    }

#ifdef KDFONTOP
  {
    struct console_font_op cfo;

# ifndef NDEBUG
    fprintf (stderr, "Trying KDFONTOP.\n");
# endif
    
    cfo.op = KD_FONT_OP_SET;
    cfo.data = cfd->chardata;
    cfo.charcount = cfd->charcount;
    cfo.height = cfd->charheight;
    cfo.width = cfd->charwidth;

    if (0 == ioctl (fd, KDFONTOP, &cfo))
      return 0;
    else if (errno != EINVAL)
      return -1;
  }
#endif /* KDFONTOP */

  /* only KDFONTOP can set a width different from 8 */
  if (cfd->charwidth != 8)
    {
      errno = EINVAL;
      return -1;
    }

  /*#if defined(PIO_FONTX) && !defined(sparc)*/
#ifdef PIO_FONTX
  {
    struct consolefontdesc cf;
    
# ifndef NDEBUG
    fprintf (stderr, "Trying PIO_FONTX.\n");
# endif
    
    cf.chardata = cfd->chardata;
    cf.charcount = cfd->charcount;
    cf.charheight = cfd->charheight;
    
    if (0 == ioctl (fd, PIO_FONTX, &cf))
      return 0;
    else if (errno != EINVAL)
      return -1;
  }
#endif /* PIO_FONTX */

#ifdef PIO_FONT
# ifndef NDEBUG
    fprintf (stderr, "Trying PIO_FONT.\n");
# endif
    
  if (ioctl (fd, PIO_FONT, cfd->chardata))
    return -1;
#endif /* PIO_FONT */
  
  return 0;
}

void restore_rom_font(int fd)
{
  /* On most kernels this won't work since it is not supported
     when BROKEN_GRAPHICS_PROGRAMS is defined, and that is defined
     by default.  Moreover, this is not defined for vgacon. */
  if (ioctl(fd, PIO_FONTRESET, 0))
    {
      perror("PIO_FONTRESET");
    }
}

