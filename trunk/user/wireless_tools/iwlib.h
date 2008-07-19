/*
 *	Wireless Tools
 *
 *		Jean II - HPLB 97->99 - HPL 99->04
 *
 * Common header for the Wireless Extension library...
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2004 Jean Tourrilhes <jt@hpl.hp.com>
 */

#ifndef IWLIB_H
#define IWLIB_H

/*#include "CHANGELOG.h"*/

/***************************** INCLUDES *****************************/

/* Standard headers */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>		/* gethostbyname, getnetbyname */
#include <net/ethernet.h>	/* struct ether_addr */
#include <sys/time.h>		/* struct timeval */
#include <unistd.h>

/* This is our header selection. Try to hide the mess and the misery :-(
 * Don't look, you would go blind ;-) */

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#endif

/* Kernel headers 2.4.X + Glibc 2.2 - Mandrake 8.0, Debian 2.3, RH 7.1
 * Kernel headers 2.2.X + Glibc 2.2 - Slackware 8.0 */
#if defined(__GLIBC__) \
    && __GLIBC__ == 2 \
    && __GLIBC_MINOR__ >= 2 \
    && LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
//#define GLIBC22_HEADERS
#define GENERIC_HEADERS

/* Kernel headers 2.4.X + Glibc 2.1 - Debian 2.2 upgraded, RH 7.0
 * Kernel headers 2.2.X + Glibc 2.1 - Debian 2.2, RH 6.1 */
#elif defined(__GLIBC__) \
      && __GLIBC__ == 2 \
      && __GLIBC_MINOR__ == 1 \
      && LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
//#define GLIBC_HEADERS
#define GENERIC_HEADERS

/* Kernel headers 2.2.X + Glibc 2.0 - Debian 2.1 */
#elif defined(__GLIBC__) \
      && __GLIBC__ == 2 \
      && __GLIBC_MINOR__ == 0 \
      && LINUX_VERSION_CODE >= KERNEL_VERSION(2,0,0) \
      && LINUX_VERSION_CODE < KERNEL_VERSION(2,1,0)
#define GLIBC_HEADERS
#define KLUDGE_HEADERS

/* Note : is it really worth supporting kernel 2.0.X, knowing that
 * we require WE v9, which is only available in 2.2.X and higher ?
 * I guess one could use 2.0.x with an upgraded wireless.h... */

/* Kernel headers 2.0.X + Glibc 2.0 - Debian 2.0, RH 5 */
#elif defined(__GLIBC__) \
      && __GLIBC__ == 2 \
      && __GLIBC_MINOR__ == 0 \
      && LINUX_VERSION_CODE < KERNEL_VERSION(2,1,0) \
      && LINUX_VERSION_CODE >= KERNEL_VERSION(2,0,0)
#define GLIBC_HEADERS

/* Kernel headers 2.0.X + libc5 - old systems */
#elif defined(_LINUX_C_LIB_VERSION_MAJOR) \
      && _LINUX_C_LIB_VERSION_MAJOR == 5 \
      && LINUX_VERSION_CODE >= KERNEL_VERSION(2,0,0) \
      && LINUX_VERSION_CODE < KERNEL_VERSION(2,1,0)
#define LIBC5_HEADERS

/* Unsupported combination */
#else
#error "Your kernel/libc combination is not supported"
#endif

#ifdef GENERIC_HEADERS 
/* Proposed by Dr. Michael Rietz <rietz@mail.amps.de>, 27.3.2 */
/* If this works for all, it might be more stable on the long term - Jean II */
#include <net/if_arp.h>		/* For ARPHRD_ETHER */
#include <sys/socket.h>		/* For AF_INET & struct sockaddr */
#include <netinet/in.h>         /* For struct sockaddr_in */
#include <netinet/if_ether.h>
#endif /* GENERIC_HEADERS */    

#ifdef GLIBC22_HEADERS 
/* Added by Ross G. Miller <Ross_Miller@baylor.edu>, 3/28/01 */
#include <linux/if_arp.h> 	/* For ARPHRD_ETHER */
#include <linux/socket.h>	/* For AF_INET & struct sockaddr */
#include <sys/socket.h>
#endif /* GLIBC22_HEADERS */    

#ifdef KLUDGE_HEADERS
#include <socketbits.h>
#endif	/* KLUDGE_HEADERS */

#ifdef GLIBC_HEADERS
#include <linux/if_arp.h>	/* For ARPHRD_ETHER */
#include <linux/socket.h>	/* For AF_INET & struct sockaddr */
#include <linux/in.h>		/* For struct sockaddr_in */
#endif	/* KLUDGE_HEADERS || GLIBC_HEADERS */

#ifdef LIBC5_HEADERS
#include <sys/socket.h>		/* For AF_INET & struct sockaddr & socket() */
#include <linux/if_arp.h>	/* For ARPHRD_ETHER */
#include <linux/in.h>		/* For struct sockaddr_in */
#endif	/* LIBC5_HEADERS */

/* Those 3 headers were previously included in wireless.h */
#include <linux/types.h>		/* for "caddr_t" et al		*/
#include <linux/socket.h>		/* for "struct sockaddr" et al	*/
#include <linux/if.h>			/* for IFNAMSIZ and co... */

/* Private copy of Wireless extensions */
#include <wireless.h>

/* Make gcc understant that when we say inline, we mean it.
 * I really hate when the compiler is trying to be more clever than me,
 * because in this case gcc is not able to figure out functions with a
 * single call site, so not only I have to tag those functions inline
 * by hand, but then it refuse to inline them properly.
 * Total saving for iwevent : 150B = 0.7%.
 * Fortunately, in gcc 3.4, they now automatically inline static functions
 * with a single call site. Hurrah !
 * Jean II */
#if __GNUC__ == 3
#if __GNUC_MINOR__ >= 1 && __GNUC_MINOR__ < 4
#define inline		inline		__attribute__((always_inline))
#endif
#endif /* __GNUC__ */

#ifdef __cplusplus
extern "C" {
#endif

/****************************** DEBUG ******************************/


/************************ CONSTANTS & MACROS ************************/

/* Various versions information */
/* Recommended Wireless Extension version */
#define WE_VERSION	17
/* Version of Wireless Tools */
#define WT_VERSION	27

/* Paths */
#define PROC_NET_WIRELESS	"/proc/net/wireless"
#define PROC_NET_DEV		"/proc/net/dev"

/* Some usefull constants */
#define KILO	1e3
#define MEGA	1e6
#define GIGA	1e9
/* For doing log10/exp10 without libm */
#define LOG10_MAGIC	1.25892541179

/* Backward compatibility for Wireless Extension 9 */
#ifndef IW_POWER_MODIFIER
#define IW_POWER_MODIFIER	0x000F	/* Modify a parameter */
#define IW_POWER_MIN		0x0001	/* Value is a minimum  */
#define IW_POWER_MAX		0x0002	/* Value is a maximum */
#define IW_POWER_RELATIVE	0x0004	/* Value is not in seconds/ms/us */
#endif /* IW_POWER_MODIFIER */

#ifndef IW_ENCODE_NOKEY
#define IW_ENCODE_NOKEY         0x0800  /* Key is write only, so not here */
#define IW_ENCODE_MODE		0xF000	/* Modes defined below */
#endif /* IW_ENCODE_NOKEY */
#ifndef IW_ENCODE_TEMP
#define IW_ENCODE_TEMP		0x0400  /* Temporary key */
#endif /* IW_ENCODE_TEMP */

/* More backward compatibility */
#ifndef SIOCSIWCOMMIT
#define SIOCSIWCOMMIT	SIOCSIWNAME
#endif /* SIOCSIWCOMMIT */

/* Still more backward compatibility */
#ifndef IW_FREQ_FIXED
#define IW_FREQ_FIXED	0x01
#endif /* IW_FREQ_FIXED */

/****************************** TYPES ******************************/

/* Shortcuts */
typedef struct iw_statistics	iwstats;
typedef struct iw_range		iwrange;
typedef struct iw_param		iwparam;
typedef struct iw_freq		iwfreq;
typedef struct iw_quality	iwqual;
typedef struct iw_priv_args	iwprivargs;
typedef struct sockaddr		sockaddr;

/* Structure for storing all wireless information for each device
 * This is a cut down version of the one above, containing only
 * the things *truly* needed to configure a card.
 * Don't add other junk, I'll remove it... */
typedef struct wireless_config
{
  char		name[IFNAMSIZ + 1];	/* Wireless/protocol name */
  int		has_nwid;
  iwparam	nwid;			/* Network ID */
  int		has_freq;
  double	freq;			/* Frequency/channel */
  int		freq_flags;
  int		has_key;
  unsigned char	key[IW_ENCODING_TOKEN_MAX];	/* Encoding key used */
  int		key_size;		/* Number of bytes */
  int		key_flags;		/* Various flags */
  int		has_essid;
  int		essid_on;
  char		essid[IW_ESSID_MAX_SIZE + 1];	/* ESSID (extended network) */
  int		has_mode;
  int		mode;			/* Operation mode */
} wireless_config;

/* Structure for storing all wireless information for each device
 * This is pretty exhaustive... */
typedef struct wireless_info
{
  struct wireless_config	b;	/* Basic information */

  int		has_sens;
  iwparam	sens;			/* sensitivity */
  int		has_nickname;
  char		nickname[IW_ESSID_MAX_SIZE + 1]; /* NickName */
  int		has_ap_addr;
  sockaddr	ap_addr;		/* Access point address */
  int		has_bitrate;
  iwparam	bitrate;		/* Bit rate in bps */
  int		has_rts;
  iwparam	rts;			/* RTS threshold in bytes */
  int		has_frag;
  iwparam	frag;			/* Fragmentation threshold in bytes */
  int		has_power;
  iwparam	power;			/* Power management parameters */
  int		has_txpower;
  iwparam	txpower;		/* Transmit Power in dBm */
  int		has_retry;
  iwparam	retry;			/* Retry limit or lifetime */

  /* Stats */
  iwstats	stats;
  int		has_stats;
  iwrange	range;
  int		has_range;
} wireless_info;

/* Structure for storing an entry of a wireless scan.
 * This is only a subset of all possible information, the flexible
 * structure of scan results make it impossible to capture all
 * information in such a static structure. */
typedef struct wireless_scan
{
  /* Linked list */
  struct wireless_scan *	next;

  /* Cell identifiaction */
  int		has_ap_addr;
  sockaddr	ap_addr;		/* Access point address */

  /* Other information */
  struct wireless_config	b;	/* Basic information */
  iwstats	stats;			/* Signal strength */
  int		has_stats;
} wireless_scan;

/*
 * Context used for non-blocking scan.
 */
typedef struct wireless_scan_head
{
  wireless_scan *	result;		/* Result of the scan */
  int			retry;		/* Retry level */
} wireless_scan_head;

/* Structure used for parsing event streams, such as Wireless Events
 * and scan results */
typedef struct stream_descr
{
  char *	end;		/* End of the stream */
  char *	current;	/* Current event in stream of events */
  char *	value;		/* Current value in event */
} stream_descr;

/* Prototype for handling display of each single interface on the
 * system - see iw_enum_devices() */
typedef int (*iw_enum_handler)(int	skfd,
			       char *	ifname,
			       char *	args[],
			       int	count);

/**************************** PROTOTYPES ****************************/
/*
 * All the functions in iwcommon.c
 */

/* ---------------------- SOCKET SUBROUTINES -----------------------*/
int
	iw_sockets_open(void);
void
	iw_enum_devices(int		skfd,
			iw_enum_handler fn,
			char *		args[],
			int		count);
/* --------------------- WIRELESS SUBROUTINES ----------------------*/
int
	iw_get_kernel_we_version(void);
int
	iw_print_version_info(const char *	toolname);
int
	iw_get_range_info(int		skfd,
			  const char *	ifname,
			  iwrange *	range);
int
	iw_get_priv_info(int		skfd,
			 const char *	ifname,
			 iwprivargs **	ppriv);
int
	iw_get_basic_config(int			skfd,
			    const char *	ifname,
			    wireless_config *	info);
int
	iw_set_basic_config(int			skfd,
			    const char *	ifname,
			    wireless_config *	info);
/* --------------------- PROTOCOL SUBROUTINES --------------------- */
int
	iw_protocol_compare(const char *	protocol1,
			    const char *	protocol2);
/* -------------------- FREQUENCY SUBROUTINES --------------------- */
void
	iw_float2freq(double	in,
		      iwfreq *	out);
double
	iw_freq2float(const iwfreq *	in);
void
	iw_print_freq_value(char *	buffer,
			    int		buflen,
			    double	freq);
void
	iw_print_freq(char *	buffer,
		      int	buflen,
		      double	freq,
		      int	channel,
		      int	freq_flags);
int
	iw_freq_to_channel(double			freq,
			   const struct iw_range *	range);
int
	iw_channel_to_freq(int				channel,
			   double *			pfreq,
			   const struct iw_range *	range);
void
	iw_print_bitrate(char *	buffer,
			 int	buflen,
			 int	bitrate);
/* ---------------------- POWER SUBROUTINES ----------------------- */
int
	iw_dbm2mwatt(int	in);
int
	iw_mwatt2dbm(int	in);
void
	iw_print_txpower(char *			buffer,
			 int			buflen,
			 struct iw_param *	txpower);
/* -------------------- STATISTICS SUBROUTINES -------------------- */
int
	iw_get_stats(int		skfd,
		     const char *	ifname,
		     iwstats *		stats,
		     const iwrange *	range,
		     int		has_range);
void
	iw_print_stats(char *		buffer,
		       int		buflen,
		       const iwqual *	qual,
		       const iwrange *	range,
		       int		has_range);
/* --------------------- ENCODING SUBROUTINES --------------------- */
void
	iw_print_key(char *			buffer,
		     int			buflen,
		     const unsigned char *	key,
		     int			key_size,
		     int			key_flags);
int
	iw_in_key(const char *		input,
		  unsigned char *	key);
int
	iw_in_key_full(int		skfd,
		       const char *	ifname,
		       const char *	input,
		       unsigned char *	key,
		       __u16 *		flags);
/* ----------------- POWER MANAGEMENT SUBROUTINES ----------------- */
void
	iw_print_pm_value(char *	buffer,
			  int		buflen,
			  int		value,
			  int		flags);
void
	iw_print_pm_mode(char *		buffer,
			 int		buflen,
			 int		flags);
/* --------------- RETRY LIMIT/LIFETIME SUBROUTINES --------------- */
void
	iw_print_retry_value(char *	buffer,
			     int	buflen,
			     int	value,
			     int	flags);
/* ----------------------- TIME SUBROUTINES ----------------------- */
void
	iw_print_timeval(char *			buffer,
			 int			buflen,
			 const struct timeval *	time);
/* --------------------- ADDRESS SUBROUTINES ---------------------- */
int
	iw_check_mac_addr_type(int	skfd,
			       char *	ifname);
int
	iw_check_if_addr_type(int	skfd,
			      char *	ifname);
#if 0
int
	iw_check_addr_type(int		skfd,
			   char *	ifname);
#endif
int
	iw_get_mac_addr(int			skfd,
			const char *		name,
			struct ether_addr *	eth,
			unsigned short *	ptype);
void
	iw_ether_ntop(const struct ether_addr* eth, char* buf);
char*
	iw_ether_ntoa(const struct ether_addr* eth);
int
	iw_ether_aton(const char* bufp, struct ether_addr* eth);
int
	iw_in_inet(char *bufp, struct sockaddr *sap);
int
	iw_in_addr(int			skfd,
		   char *		ifname,
		   char *		bufp,
		   struct sockaddr *	sap);
/* ----------------------- MISC SUBROUTINES ------------------------ */
int
	iw_get_priv_size(int		args);

/* ---------------------- EVENT SUBROUTINES ---------------------- */
void
	iw_init_event_stream(struct stream_descr *	stream,
			     char *			data,
			     int			len);
int
	iw_extract_event_stream(struct stream_descr *	stream,
				struct iw_event *	iwe,
				int			we_version);
/* --------------------- SCANNING SUBROUTINES --------------------- */
int
	iw_process_scan(int			skfd,
			char *			ifname,
			int			we_version,
			wireless_scan_head *	context);
int
	iw_scan(int			skfd,
		char *			ifname,
		int			we_version,
		wireless_scan_head *	context);

/**************************** VARIABLES ****************************/

/* Modes as human readable strings */
extern const char * const	iw_operation_mode[];
#define IW_NUM_OPER_MODE	7

/************************* INLINE FUNTIONS *************************/
/*
 * Functions that are so simple that it's more efficient inlining them
 */

/*
 * Note : I've defined wrapper for the ioctl request so that
 * it will be easier to migrate to other kernel API if needed
 */

/*------------------------------------------------------------------*/
/*
 * Wrapper to push some Wireless Parameter in the driver
 */
static inline int
iw_set_ext(int			skfd,		/* Socket to the kernel */
	   const char *		ifname,		/* Device name */
	   int			request,	/* WE ID */
	   struct iwreq *	pwrq)		/* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}

/*------------------------------------------------------------------*/
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int
iw_get_ext(int			skfd,		/* Socket to the kernel */
	   const char *		ifname,		/* Device name */
	   int			request,	/* WE ID */
	   struct iwreq *	pwrq)		/* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}

/*------------------------------------------------------------------*/
/*
 * Close the socket used for ioctl.
 */
static inline void
iw_sockets_close(int	skfd)
{
  close(skfd);
}

/*------------------------------------------------------------------*/
/* Backwards compatability
 * Actually, those form are much easier to use when dealing with
 * struct sockaddr... */
static inline char*
iw_pr_ether(char* bufp, const unsigned char* addr)
{
  iw_ether_ntop((const struct ether_addr *) addr, bufp);
  return bufp;
}
/* Backwards compatability */
static inline int
iw_in_ether(const char *bufp, struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  return iw_ether_aton(bufp, (struct ether_addr *) sap->sa_data) ? 0 : -1;
}

/*------------------------------------------------------------------*/
/*
 * Create an Ethernet broadcast address
 */
static inline void
iw_broad_ether(struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  memset((char *) sap->sa_data, 0xFF, ETH_ALEN);
}

/*------------------------------------------------------------------*/
/*
 * Create an Ethernet NULL address
 */
static inline void
iw_null_ether(struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  memset((char *) sap->sa_data, 0x00, ETH_ALEN);
}

#ifdef __cplusplus
}
#endif

#endif	/* IWLIB_H */
