/*
 *	Wireless Tools
 *
 *		Jean II - HPLB 97->99 - HPL 99->00
 *
 * Main code for "iwconfig". This is the generic tool for most
 * manipulations...
 * You need to link this code against "iwlib.c" and "-lm".
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2002 Jean Tourrilhes <jt@hpl.hp.com>
 */

#include "iwlib.h"		/* Header */

/************************** DOCUMENTATION **************************/

/*
 * IOCTL RANGES :
 * ------------
 *	The initial implementation of iwpriv was using the SIOCDEVPRIVATE
 * ioctl range (up to 16 ioctls - driver specific). However, this was
 * causing some compatibility problems with other usages of those
 * ioctls, and those ioctls are supposed to be removed.
 *	Therefore, I created a new ioctl range, at SIOCIWFIRSTPRIV. Those
 * ioctls are specific to Wireless Extensions, so you don't have to
 * worry about collisions with other usages. On the other hand, in the
 * new range, the SET convention is enforced (see below).
 *	The differences are :		SIOCDEVPRIVATE	SIOCIWFIRSTPRIV
 *		o availability :	<= 2.5.X	WE > 11 (>= 2.4.13)
 *		o collisions		yes		no
 *		o SET convention	optional	enforced
 *		o number		16		32
 *
 * NEW DRIVER API :
 * --------------
 *	Wireless Extension 13 introduce a new driver API. Wireless
 * Extensions requests can be handled via a iw_handler table instead
 * of through the regular ioctl handler.
 *	The new driver API can be handled only with the new ioctl range
 * and enforce the GET convention (see below).
 *	The differences are :		old API		new API
 *		o handler		do_ioctl()	struct iw_handler_def
 *		o SIOCIWFIRSTPRIV	WE > 11		yes
 *		o SIOCDEVPRIVATE	yes		no
 *		o GET convention	optional	enforced
 *	Note that the new API before Wireless Extension 15 contains bugs
 * with regards to handling sub-ioctls and addr/float data types.
 *
 * SET/GET CONVENTION :
 * ------------------
 *	The regular Wireless Extensions use a SET/GET convention, where
 * the low order bit identify a SET (0) or a GET (1) request.
 *	The new ioctl range enforce the SET convention : SET request will
 * be available to root only and can't return any arguments. If you don't
 * like that, just use every other two ioctl.
 *	The new driver API enforce the GET convention : GET request won't
 * be able to accept any arguments (except if its fits within (union
 * iwreq_data)). If you don't like that, just use the old API (aka the
 * ioctl handler).
 *	In any case, it's a good idea to not have ioctl with both SET
 * and GET arguments. If the GET arguments doesn't fit within
 * (union iwreq_data) and SET do, or vice versa, the current code in iwpriv
 * won't work. One exception is if both SET and GET arguments fit within
 * (union iwreq_data), this case should be handled safely in a GET
 * request.
 *
 * SUB-IOCTLS :
 * ----------
 *	Wireless Extension 15 introduce sub-ioctls. For some applications,
 * 32 ioctl is not enough, and this simple mechanism allow to increase
 * the number of ioctls by adding a sub-ioctl index to some of the ioctl
 * (so basically a two level addressing).
 *	One might argue that at the point, some other mechanisms might be
 * better, like using a real filesystem abstraction (/proc, driverfs, ...),
 * but sub-ioctls are simple enough to not have much drawbacks (which means
 * that it's a quick and dirty hack ;-).
 *
 *	There is two slightly different variation of the sub-ioctl scheme :
 *	If the payload fit within (union iwreq_data), the first int (4 bytes)
 * is reserved as the sub-ioctl number and the regular payload shifted by
 * 4 bytes.
 *	If the ioctl use (struct iw_point), the sub-ioctl number is in the
 * flags member of the structure.
 *	Then, in your handler you would just extract the sub-ioctl number
 * and do the appropriate processing.
 *
 *	Sub-ioctls are declared normally in the private definition table,
 * with cmd (first arg) beeing the sub-ioctl number. Then, you need to
 * declare the real ioctl which will process the sub-ioctls with the
 * SAME ARGUMENTS and a NULL NAME.
 *	It could look like, for example :
 * --------------------------------------------
	// --- Raw access to sub-ioctl handlers ---
	{ 0x8BE0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_paramN" },
	{ 0x8BE1, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_paramN" },
	// --- sub-ioctls handlers ---
	{ 0x8BE0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "" },
	{ 0x8BE1, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "" },
	// --- sub-ioctls definitions ---
	{ 1, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_param1" },
	{ 1, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_param1" },
	{ 2, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_param2" },
	{ 2, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_param2" },
 * --------------------------------------------
 *	And iwpriv should do the rest for you ;-)
 *
 *	Note that version of iwpriv up to v24 (included) expect at most
 * 16 ioctls definitions and will likely crash when given more.
 *	There is no fix that I can see, apart from recommending an upgrade
 * of Wireless Tools. Wireless Extensions 15 will check this condition, so
 * another workaround is restricting those extra definitions to WE-15.
 *
 *	Another problem is that new API before Wireless Extension 15
 * will get it wrong when passing fixed arguments of 12-15 bytes. It will
 * try to get them inline instead of by pointer. You can fool the new API
 * to do the right thing using fake ioctl definitions (but remember that
 * you will get more likely to hit the limit of 16 ioctl definitions).
 *	For safety, use the ioctl handler before v15.
 *
 * NEW DATA TYPES (ADDR/FLOAT) :
 * ---------------------------
 *	Wireless Tools 25 introduce two new data types, addr and float,
 * corresponding to struct sockaddr and struct iwfreq.
 *	Those types are properly handled with Wireless Extensions 15.
 * However, the new API before v15 won't handle them properly.
 *
 *	The first problem is that the new API won't know their size, so
 * won't copy them. This can be workaround with a fake ioctl definition.
 *	The second problem is that a fixed single addr won't be inlined
 * in struct iwreq and will be passed as a pointer. This is due to an
 * off-by-one error, where all fixed data of 16 bytes is considered too
 * big to fit in struct iwreq.
 *
 *	For those reasons, I would recommend to use the ioctl handler
 * before v15 when manipulating those data.
 *
 * TOKEN INDEX :
 * -----------
 *	Token index is very similar to sub-ioctl. It allow the user
 * to specify an integer index in front of a bunch of other arguments
 * (addresses, strings, ...).
 *	Token index works only with data passed as pointer, and is
 * otherwise ignored. If your data would fit within struct iwreq, you
 * need to declare the command *without* IW_PRIV_SIZE_FIXED to force
 * this to happen (and check arg number yourself).
 * --------------------------------------------
	// --- Commands that would fit in struct iwreq ---
	{ 0x8BE0, IW_PRIV_TYPE_ADDR | 1, 0, "set_param_with_token" },
	// --- No problem here ---
	{ 0x8BE1, IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 2, 0, "again" },
 * --------------------------------------------
 *	The token index feature is pretty transparent, the token index
 * will just be in the flags member of (struct iw_point). Default value
 * (if the user doesn't specify it) will be 0. Token index itself will
 * work with any version of Wireless Extensions.
 *	Token index is not compatible with sub-ioctl (both use the same
 * field of struct iw_point). However, token index can be use to offer
 * raw access to the sub-ioctl handlers (if it uses struct iw_point) :
 * --------------------------------------------
	// --- sub-ioctls handler ---
	{ 0x8BE0, IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 1, 0, "" },
	// --- sub-ioctls definitions ---
	{ 0, IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 1, 0, "setaddr" },
	{ 1, IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 1, 0, "deladdr" },
	// --- raw access with token index (+ iwreq workaround) ---
	{ 0x8BE0, IW_PRIV_TYPE_ADDR | 1, 0, "rawaddr" },
 * --------------------------------------------
 *
 * Jean II
 */

/**************************** CONSTANTS ****************************/

static const char *	argtype[] = {
  "     ", "byte ", "char ", "", "int  ", "float", "addr " };

#define IW_MAX_PRIV_DEF	128

/* Backward compatibility */
#ifndef IW_PRIV_TYPE_ADDR
#define IW_PRIV_TYPE_ADDR	0x6000
#endif	/* IW_PRIV_TYPE_ADDR */

/************************* MISC SUBROUTINES **************************/

/*------------------------------------------------------------------*/
/*
 * Print usage string
 */
static void
iw_usage(void)
{
  fprintf(stderr, "Usage: iwpriv interface [private-command [private-arguments]]\n");
  fprintf(stderr, "              interface [roam {on|off}]\n");
  fprintf(stderr, "              interface [port {ad-hoc|managed|N}]\n");
}

/************************* SETTING ROUTINES **************************/

/*------------------------------------------------------------------*/
/*
 * Execute a private command on the interface
 */
static int
set_private_cmd(int		skfd,		/* Socket */
		char *		args[],		/* Command line args */
		int		count,		/* Args count */
		char *		ifname,		/* Dev name */
		char *		cmdname,	/* Command name */
		iwprivargs *	priv,		/* Private ioctl description */
		int		priv_num)	/* Number of descriptions */
{
  struct iwreq	wrq;
  u_char	buffer[4096];	/* Only that big in v25 and later */
  int		i = 0;		/* Start with first command arg */
  int		k;		/* Index in private description table */
  int		temp;
  int		subcmd = 0;	/* sub-ioctl index */
  int		offset = 0;	/* Space for sub-ioctl index */

  /* Check if we have a token index.
   * Do it know so that sub-ioctl takes precendence, and so that we
   * don't have to bother with it later on... */
  if((count > 1) && (sscanf(args[0], "[%i]", &temp) == 1))
    {
      subcmd = temp;
      args++;
      count--;
    }

  /* Search the correct ioctl */
  k = -1;
  while((++k < priv_num) && strcmp(priv[k].name, cmdname));

  /* If not found... */
  if(k == priv_num)
    {
      fprintf(stderr, "Invalid command : %s\n", cmdname);
      return(-1);
    }
	  
  /* Watch out for sub-ioctls ! */
  if(priv[k].cmd < SIOCDEVPRIVATE)
    {
      int	j = -1;

      /* Find the matching *real* ioctl */
      while((++j < priv_num) && ((priv[j].name[0] != '\0') ||
				 (priv[j].set_args != priv[k].set_args) ||
				 (priv[j].get_args != priv[k].get_args)));

      /* If not found... */
      if(j == priv_num)
	{
	  fprintf(stderr, "Invalid private ioctl definition for : %s\n",
		  cmdname);
	  return(-1);
	}

      /* Save sub-ioctl number */
      subcmd = priv[k].cmd;
      /* Reserve one int (simplify alignement issues) */
      offset = sizeof(__u32);
      /* Use real ioctl definition from now on */
      k = j;

      printf("<mapping sub-ioctl %s to cmd 0x%X-%d>\n", cmdname,
	     priv[k].cmd, subcmd);
    }

  /* If we have to set some data */
  if((priv[k].set_args & IW_PRIV_TYPE_MASK) &&
     (priv[k].set_args & IW_PRIV_SIZE_MASK))
    {
      switch(priv[k].set_args & IW_PRIV_TYPE_MASK)
	{
	case IW_PRIV_TYPE_BYTE:
	  /* Number of args to fetch */
	  wrq.u.data.length = count;
	  if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
	    wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

	  /* Fetch args */
	  for(; i < wrq.u.data.length; i++) {
	    sscanf(args[i], "%i", &temp);
	    buffer[i] = (char) temp;
	  }
	  break;

	case IW_PRIV_TYPE_INT:
	  /* Number of args to fetch */
	  wrq.u.data.length = count;
	  if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
	    wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

	  /* Fetch args */
	  for(; i < wrq.u.data.length; i++) {
	    sscanf(args[i], "%i", &temp);
	    ((__s32 *) buffer)[i] = (__s32) temp;
	  }
	  break;

	case IW_PRIV_TYPE_CHAR:
	  if(i < count)
	    {
	      /* Size of the string to fetch */
	      wrq.u.data.length = strlen(args[i]) + 1;
	      if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
		wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

	      /* Fetch string */
	      memcpy(buffer, args[i], wrq.u.data.length);
	      buffer[sizeof(buffer) - 1] = '\0';
	      i++;
	    }
	  else
	    {
	      wrq.u.data.length = 1;
	      buffer[0] = '\0';
	    }
	  break;

	case IW_PRIV_TYPE_FLOAT:
	  /* Number of args to fetch */
	  wrq.u.data.length = count;
	  if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
	    wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

	  /* Fetch args */
	  for(; i < wrq.u.data.length; i++) {
	    double		freq;
	    if(sscanf(args[i], "%lg", &(freq)) != 1)
	      {
		printf("Invalid float [%s]...\n", args[i]);
		return(-1);
	      }    
	    if(index(args[i], 'G')) freq *= GIGA;
	    if(index(args[i], 'M')) freq *= MEGA;
	    if(index(args[i], 'k')) freq *= KILO;
	    sscanf(args[i], "%i", &temp);
	    iw_float2freq(freq, ((struct iw_freq *) buffer) + i);
	  }
	  break;

	case IW_PRIV_TYPE_ADDR:
	  /* Number of args to fetch */
	  wrq.u.data.length = count;
	  if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
	    wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

	  /* Fetch args */
	  for(; i < wrq.u.data.length; i++) {
	    if(iw_in_addr(skfd, ifname, args[i],
			  ((struct sockaddr *) buffer) + i) < 0)
	      {
		printf("Invalid address [%s]...\n", args[i]);
		return(-1);
	      }
	  }
	  break;

	default:
	  fprintf(stderr, "Not yet implemented...\n");
	  return(-1);
	}
	  
      if((priv[k].set_args & IW_PRIV_SIZE_FIXED) &&
	 (wrq.u.data.length != (priv[k].set_args & IW_PRIV_SIZE_MASK)))
	{
	  printf("The command %s need exactly %d argument...\n",
		 cmdname, priv[k].set_args & IW_PRIV_SIZE_MASK);
	  return(-1);
	}
    }	/* if args to set */
  else
    {
      wrq.u.data.length = 0L;
    }

  strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

  /* Those two tests are important. They define how the driver
   * will have to handle the data */
  if((priv[k].set_args & IW_PRIV_SIZE_FIXED) &&
      ((iw_get_priv_size(priv[k].set_args) + offset) <= IFNAMSIZ))
    {
      /* First case : all SET args fit within wrq */
      if(offset)
	wrq.u.mode = subcmd;
      memcpy(wrq.u.name + offset, buffer, IFNAMSIZ - offset);
    }
  else
    {
      if((priv[k].set_args == 0) &&
	 (priv[k].get_args & IW_PRIV_SIZE_FIXED) &&
	 (iw_get_priv_size(priv[k].get_args) <= IFNAMSIZ))
	{
	  /* Second case : no SET args, GET args fit within wrq */
	  if(offset)
	    wrq.u.mode = subcmd;
	}
      else
	{
	  /* Thirst case : args won't fit in wrq, or variable number of args */
	  wrq.u.data.pointer = (caddr_t) buffer;
	  wrq.u.data.flags = subcmd;
	}
    }

  /* Perform the private ioctl */
  if(ioctl(skfd, priv[k].cmd, &wrq) < 0)
    {
      fprintf(stderr, "Interface doesn't accept private ioctl...\n");
      fprintf(stderr, "%s (%X): %s\n", cmdname, priv[k].cmd, strerror(errno));
      return(-1);
    }

  /* If we have to get some data */
  if((priv[k].get_args & IW_PRIV_TYPE_MASK) &&
     (priv[k].get_args & IW_PRIV_SIZE_MASK))
    {
      int	j;
      int	n = 0;		/* number of args */

      printf("%-8.8s  %s:", ifname, cmdname);

      /* Check where is the returned data */
      if((priv[k].get_args & IW_PRIV_SIZE_FIXED) &&
	 (iw_get_priv_size(priv[k].get_args) <= IFNAMSIZ))
	{
	  memcpy(buffer, wrq.u.name, IFNAMSIZ);
	  n = priv[k].get_args & IW_PRIV_SIZE_MASK;
	}
      else
	n = wrq.u.data.length;

      switch(priv[k].get_args & IW_PRIV_TYPE_MASK)
	{
	case IW_PRIV_TYPE_BYTE:
	  /* Display args */
	  for(j = 0; j < n; j++)
	    printf("%d  ", buffer[j]);
	  printf("\n");
	  break;

	case IW_PRIV_TYPE_INT:
	  /* Display args */
	  for(j = 0; j < n; j++)
	    printf("%d  ", ((__s32 *) buffer)[j]);
	  printf("\n");
	  break;

	case IW_PRIV_TYPE_CHAR:
	  /* Display args */
	  buffer[wrq.u.data.length - 1] = '\0';
	  printf("%s\n", buffer);
	  break;

	case IW_PRIV_TYPE_FLOAT:
	  {
	    double		freq;
	    /* Display args */
	    for(j = 0; j < n; j++)
	      {
		freq = iw_freq2float(((struct iw_freq *) buffer) + j);
		if(freq >= GIGA)
		  printf("%gG  ", freq / GIGA);
		else
		  if(freq >= MEGA)
		  printf("%gM  ", freq / MEGA);
		else
		  printf("%gk  ", freq / KILO);
	      }
	    printf("\n");
	  }
	  break;

	case IW_PRIV_TYPE_ADDR:
	  {
	    char		scratch[128];
	    struct sockaddr *	hwa;
	    /* Display args */
	    for(j = 0; j < n; j++)
	      {
		hwa = ((struct sockaddr *) buffer) + j;
		if(j)
		  printf("           %.*s", 
			 (int) strlen(cmdname), "                ");
		printf("%s\n", iw_pr_ether(scratch, hwa->sa_data));
	      }
	  }
	  break;

	default:
	  fprintf(stderr, "Not yet implemented...\n");
	  return(-1);
	}
    }	/* if args to set */

  return(0);
}

/*------------------------------------------------------------------*/
/*
 * Execute a private command on the interface
 */
static inline int
set_private(int		skfd,		/* Socket */
	    char *	args[],		/* Command line args */
	    int		count,		/* Args count */
	    char *	ifname)		/* Dev name */
{
  iwprivargs	priv[IW_MAX_PRIV_DEF];
  int		number;		/* Max of private ioctl */

  /* Read the private ioctls */
  number = iw_get_priv_info(skfd, ifname, priv, IW_MAX_PRIV_DEF);

  /* Is there any ? */
  if(number <= 0)
    {
      /* Could skip this message ? */
      fprintf(stderr, "%-8.8s  no private ioctls.\n\n",
	      ifname);
      return(-1);
    }

  return(set_private_cmd(skfd, args + 1, count - 1, ifname, args[0],
			 priv, number));
}

/************************ CATALOG FUNCTIONS ************************/

/*------------------------------------------------------------------*/
/*
 * Print on the screen in a neat fashion all the info we have collected
 * on a device.
 */
static int
print_priv_info(int		skfd,
		char *		ifname,
		char *		args[],
		int		count)
{
  int		k;
  iwprivargs	priv[IW_MAX_PRIV_DEF];
  int		n;

  /* Avoid "Unused parameter" warning */
  args = args; count = count;

  /* Read the private ioctls */
  n = iw_get_priv_info(skfd, ifname, priv, IW_MAX_PRIV_DEF);

  /* Is there any ? */
  if(n <= 0)
    {
      /* Could skip this message ? */
      fprintf(stderr, "%-8.8s  no private ioctls.\n\n",
	      ifname);
    }
  else
    {
      printf("%-8.8s  Available private ioctl :\n", ifname);
      /* Print them all */
      for(k = 0; k < n; k++)
	if(priv[k].name[0] != '\0')
	  printf("          %-16.16s (%.4X) : set %3d %s & get %3d %s\n",
		 priv[k].name, priv[k].cmd,
		 priv[k].set_args & IW_PRIV_SIZE_MASK,
		 argtype[(priv[k].set_args & IW_PRIV_TYPE_MASK) >> 12],
		 priv[k].get_args & IW_PRIV_SIZE_MASK,
		 argtype[(priv[k].get_args & IW_PRIV_TYPE_MASK) >> 12]);
      printf("\n");
    }
  return(0);
}

/*------------------------------------------------------------------*/
/*
 * Print on the screen in a neat fashion all the info we have collected
 * on a device.
 */
static int
print_priv_all(int		skfd,
	       char *		ifname,
	       char *		args[],
	       int		count)
{
  int		k;
  iwprivargs	priv[IW_MAX_PRIV_DEF];
  int		n;

  /* Avoid "Unused parameter" warning */
  args = args; count = count;

  /* Read the private ioctls */
  n = iw_get_priv_info(skfd, ifname, priv, IW_MAX_PRIV_DEF);

  /* Is there any ? */
  if(n <= 0)
    {
      /* Could skip this message ? */
      fprintf(stderr, "%-8.8s  no private ioctls.\n\n",
	      ifname);
    }
  else
    {
      printf("%-8.8s  Available read-only private ioctl :\n", ifname);
      /* Print them all */
      for(k = 0; k < n; k++)
	/* We call all ioctl that don't have a null name, don't require
	 * args and return some (avoid triggering "reset" commands) */
	if((priv[k].name[0] != '\0') && (priv[k].set_args == 0) &&
	   (priv[k].get_args != 0))
	  set_private_cmd(skfd, NULL, 0, ifname, priv[k].name,
			  priv, n);
      printf("\n");
    }
#if 0
  // Debug
  printf("struct ifreq = %d ; struct iwreq = %d ; IFNAMSIZ = %d\n",
	 sizeof(struct ifreq), sizeof(struct iwreq), IFNAMSIZ);
  printf("struct iw_freq = %d ; struct sockaddr = %d\n",
	 sizeof(struct iw_freq), sizeof(struct sockaddr));
#endif
  return(0);
}

/********************** PRIVATE IOCTLS MANIPS ***********************/
/*
 * Convenient access to some private ioctls of some devices
 */

/*------------------------------------------------------------------*/
/*
 * Set roaming mode on and off
 * Found in wavelan_cs driver
 */
static int
set_roaming(int		skfd,		/* Socket */
	    char *	args[],		/* Command line args */
	    int		count,		/* Args count */
	    char *	ifname)		/* Dev name */
{
  u_char	buffer[1024];
  struct iwreq		wrq;
  int		i = 0;		/* Start with first arg */
  int		k;
  iwprivargs	priv[IW_MAX_PRIV_DEF];
  int		number;
  char		RoamState;		/* buffer to hold new roam state */
  char		ChangeRoamState=0;	/* whether or not we are going to
					   change roam states */

  /* Read the private ioctls */
  number = iw_get_priv_info(skfd, ifname, priv, IW_MAX_PRIV_DEF);

  /* Is there any ? */
  if(number <= 0)
    {
      /* Could skip this message ? */
      fprintf(stderr, "%-8.8s  no private ioctls.\n\n",
	      ifname);
      return(-1);
    }

  if(count != 1)
    {
      iw_usage();
      return(-1);
    }

  if(!strcasecmp(args[i], "on"))
    {
      printf("%-8.8s  enable roaming\n", ifname);
      if(!number)
	{
	  fprintf(stderr, "This device doesn't support roaming\n");
	  return(-1);
	}
      ChangeRoamState=1;
      RoamState=1;
    }
  else
    if(!strcasecmp(args[i], "off"))
      {
	i++;
	printf("%-8.8s  disable roaming\n",  ifname);
	if(!number)
	  {
	    fprintf(stderr, "This device doesn't support roaming\n");
	    return(-1);
	  }
	ChangeRoamState=1;
	RoamState=0;
      }
    else
      {
	iw_usage();
	return(-1);
      }

  if(ChangeRoamState)
    {
      k = -1;
      while((++k < number) && strcmp(priv[k].name, "setroam"));
      if(k == number)
	{
	  fprintf(stderr, "This device doesn't support roaming\n");
	  return(-1);
	}
      strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

      buffer[0]=RoamState;

      memcpy(wrq.u.name, &buffer, IFNAMSIZ);

      if(ioctl(skfd, priv[k].cmd, &wrq) < 0)
	{
	  fprintf(stderr, "Roaming support is broken.\n");
	  return(-1);
	}
    }

  return(0);
}

/*------------------------------------------------------------------*/
/*
 * Get and set the port type
 * Found in wavelan2_cs and wvlan_cs drivers
 */
static int
port_type(int		skfd,		/* Socket */
	  char *	args[],		/* Command line args */
	  int		count,		/* Args count */
	  char *	ifname)		/* Dev name */
{
  struct iwreq	wrq;
  int		i = 0;		/* Start with first arg */
  int		k;
  iwprivargs	priv[IW_MAX_PRIV_DEF];
  int		number;
  char		ptype = 0;
  char *	modes[] = { "invalid", "managed (BSS)", "reserved", "ad-hoc" };

  /* Read the private ioctls */
  number = iw_get_priv_info(skfd, ifname, priv, IW_MAX_PRIV_DEF);

  /* Is there any ? */
  if(number <= 0)
    {
      /* Could skip this message ? */
      fprintf(stderr, "%-8.8s  no private ioctls.\n\n", ifname);
      return(-1);
    }

  /* Arguments ? */
  if(count == 0)
    {
      /* So, we just want to see the current value... */
      k = -1;
      while((++k < number) && strcmp(priv[k].name, "gport_type") &&
	     strcmp(priv[k].name, "get_port"));
      if(k == number)
	{
	  fprintf(stderr, "This device doesn't support getting port type\n");
	  return(-1);
	}
      strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

      /* Get it */
      if(ioctl(skfd, priv[k].cmd, &wrq) < 0)
	{
	  fprintf(stderr, "Port type support is broken.\n");
	  exit(0);
	}
      ptype = *wrq.u.name;

      /* Display it */
      printf("%-8.8s  Current port mode is %s <port type is %d>.\n\n",
	     ifname, modes[(int) ptype], ptype);

      return(0);
    }

  if(count != 1)
    {
      iw_usage();
      return(-1);
    }

  /* Read it */
  /* As a string... */
  k = 0;
  while((k < 4) && strncasecmp(args[i], modes[k], 2))
    k++;
  if(k < 4)
    ptype = k;
  else
    /* ...or as an integer */
    if(sscanf(args[i], "%i", (int *) &ptype) != 1)
      {
	iw_usage();
	return(-1);
      }
  
  k = -1;
  while((++k < number) && strcmp(priv[k].name, "sport_type") &&
	strcmp(priv[k].name, "set_port"));
  if(k == number)
    {
      fprintf(stderr, "This device doesn't support setting port type\n");
      return(-1);
    }
  strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

  *(wrq.u.name) = ptype;

  if(ioctl(skfd, priv[k].cmd, &wrq) < 0)
    {
      fprintf(stderr, "Invalid port type (or setting not allowed)\n");
      return(-1);
    }

  return(0);
}

/******************************* MAIN ********************************/

/*------------------------------------------------------------------*/
/*
 * The main !
 */
int
main(int	argc,
     char **	argv)
{
  int skfd;		/* generic raw socket desc.	*/
  int goterr = 0;

  /* Create a channel to the NET kernel. */
  if((skfd = iw_sockets_open()) < 0)
    {
      perror("socket");
      return(-1);
    }

  /* No argument : show the list of all device + info */
  if(argc == 1)
    iw_enum_devices(skfd, &print_priv_info, NULL, 0);
  else
    /* Special cases take one... */
    /* All */
    if((!strncmp(argv[1], "-a", 2)) || (!strcmp(argv[1], "--all")))
      iw_enum_devices(skfd, &print_priv_all, NULL, 0);
    else
      /* Help */
      if((!strncmp(argv[1], "-h", 2)) || (!strcmp(argv[1], "--help")))
	iw_usage();
      else
	/* Version */
	if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))
	  goterr = iw_print_version_info("iwpriv");
	else
	  /* The device name must be the first argument */
	  /* Name only : show for that device only */
	  if(argc == 2)
	    print_priv_info(skfd, argv[1], NULL, 0);
	  else
	    /* Special cases take two... */
	    /* All */
	    if((!strncmp(argv[2], "-a", 2)) ||
	       (!strcmp(argv[2], "--all")))
	      print_priv_all(skfd, argv[1], NULL, 0);
	    else
	      /* Roaming */
	      if(!strncmp(argv[2], "roam", 4))
		goterr = set_roaming(skfd, argv + 3, argc - 3, argv[1]);
	      else
		/* Port type */
		if(!strncmp(argv[2], "port", 4))
		  goterr = port_type(skfd, argv + 3, argc - 3, argv[1]);
		else
		  /*-------------*/
		  /* Otherwise, it's a private ioctl */
		  goterr = set_private(skfd, argv + 2, argc - 2, argv[1]);

  /* Close the socket. */
  close(skfd);

  return(goterr);
}
