/*
 * Check SSL ciphers & certificates
 *
 * This plugin connects to a SSL server, checks its certificate and the
 * available ciphers
 *
 * This plugin was written by Michel Arboi <arboi@alussinan.org>
 */
 
#include "includes.h"

#ifndef ssl_get_cipher_by_char
#define ssl_get_cipher_by_char(ssl,ptr) \
	((ssl)->method->get_cipher_by_char(ptr))
#endif

#define EN_NAME "SSL ciphers"
#define FR_NAME "Chiffres SSL"

/*
#define EN_DESC "\
This plugin connects to a SSL server, and\n\
checks its certificate and the available ciphers.\n\
Weak (export version) ciphers are reported, as well as \n\
self signed certificates or certificates signed by unknown\n\
CAs."
*/
#define EN_DESC "\
This plugin connects to a SSL server, and\n\
checks its certificate and the available (shared) SSLv2 ciphers.\n\
Weak (export version) ciphers are reported."

#define COPYRIGHT "(C) 2002 Michel Arboi"
#define SUMMARY "checks the server certificate and available SSLv2 ciphers"


PlugExport int plugin_init(desc)
     struct arglist * desc;
{
#ifndef HAVE_SSL
  return -1;
#else
  plug_set_id(desc, 10863);
  plug_set_version(desc, "$Revision$");
 
  plug_set_name(desc, FR_NAME, "francais");
  plug_set_name(desc, EN_NAME, NULL);
  
  plug_set_description(desc, EN_DESC, NULL);
  plug_set_summary(desc, SUMMARY, NULL);
  plug_set_copyright(desc, COPYRIGHT, NULL);
  plug_set_category(desc, ACT_GATHER_INFO);
  plug_set_family(desc, "General", NULL);
  plug_set_dep(desc, "find_service.nes");
  return 0;
#endif
}


PlugExport 
int plugin_run(env)
     struct arglist * env;
{
#ifndef HAVE_SSL
  return -1;
#else
  char		*p, *q, *trp_name, *trp0_name;
  int		port, trp, trp0, cnx = -1, bits;
  char		buf[2048], *pbuf, rep[512], *prep;
  SSL_CIPHER		*c = NULL;
  SSL			*ssl = NULL;
  int			weak = 0, medium = 0, strong = 0, null =0, nCiphers = 0;
  int			cert_printed = 0;
  char			*msg;
  X509			*cert;
  BIO			*b;
  BUF_MEM		*bptr;
  int			rejected[3];
    
  
  p  = plug_get_key(env, "Transport/SSL");
  if (p == NULL)
    return 0;
  port = atoi(p);

  trp0 = plug_get_port_transport(env, port);
  trp0_name = (char*)get_encaps_name(trp0);

  for (trp = NESSUS_ENCAPS_SSLv2; trp <=  NESSUS_ENCAPS_TLSv1; trp ++)
    {
      if (cnx >= 0)
	{
	  close_stream_connection(cnx);
	  cnx = -1;
	}

      if ((cnx = open_stream_connection(env, port, trp, -2)) < 0)
	{
	  rejected[trp - NESSUS_ENCAPS_SSLv2]  = 1;
	  continue;
	}
      rejected[trp - NESSUS_ENCAPS_SSLv2]  = 0;

      if ((ssl = (SSL*)stream_get_ssl(cnx)) == NULL)
	continue;

      trp_name = (char*)get_encaps_name(trp);

      if (! cert_printed)
	{
	  cert = SSL_get_peer_certificate(ssl);
	  if(cert != NULL)
	    {
	      b = BIO_new(BIO_s_mem());
	      if(X509_print(b, cert) > 0)
		{
		  BIO_get_mem_ptr(b, &bptr);
		  msg = emalloc(bptr->length + 1 + 80);
		  snprintf(msg, bptr->length + 1 + 80, "Here is the %s server certificate:\n",
			  trp_name);
		  for (p = msg; *p != '\0'; p ++) /*NOP*/ ;
		  strncpy(p, bptr->data, bptr->length);
		  post_note(env, port, msg);
		  efree(&msg);
		}
	      BIO_free(b);
	      cert_printed ++;
	    }
	}

      if (trp != NESSUS_ENCAPS_SSLv2)
	continue;

#define HEREIS	"Here is the list of available SSLv2 ciphers:\n"
      strncpy(buf, HEREIS, sizeof(buf) - 1);
      buf[sizeof(buf) - 1] = '\0';
      pbuf = buf + sizeof(HEREIS) - 1;

      if ((pbuf = SSL_get_shared_ciphers(ssl, pbuf, sizeof(buf))) == NULL)
	continue;

      for (q = pbuf, p = pbuf; ; p ++)
	if (*p == ':' || *p == '\0')
	  {
	    int		eol = (*p == '\0');

	    nCiphers ++;
	    *p = '\0';
	    
	    c = ssl_get_cipher_by_char(ssl, q);
	    bits = 999;
	    if (c != NULL)
	      SSL_CIPHER_get_bits(c, &bits);

	    if (bits == 0)
	      null ++;
	    else
	      /* 
	       * OpenSSL returns the number of secret bits of the algorithm, 
	       * but does not say how many of them are fixed or known.
	       * So we have to check if the algorithm is "export grade"
	       */
	      if (strncmp(q, "EXP", 3) == 0)
		weak ++;
	      else if (bits < 56) /* arbitrary limit 1. You may disagree */
		weak ++;
	      else if (bits < 90) /* arbitrary limit 2. Same remark */
		medium ++;
	      else
		strong ++;

	    q = p + 1;
	    if (eol)
	      break;
	    *p = '\n';
	  }
      post_note(env, port, buf);
      
      if (null >= nCiphers)
	{
	  snprintf(rep, sizeof(rep), "\
The %s server only accepts \"null\" ciphers, which do not protect the\
confidentiality of your data.\n\
\n\
Solution: re-configure or upgrade your server software.",
		  trp_name);
	  post_hole(env, port, rep);
	}
      else
	{
	  if (null > 0)
	    {
	      snprintf(rep, sizeof(rep), "\
The %s server accepts %d \"null\" ciphers which do *not*\n\
protect the confidentiality of your data\n\
You should disable them, as they may be chosen by a badly\n\
configured client software.\
\n\
Solution: re-configure or upgrade your server software.",
		      trp_name, null);
	      post_hole(env, port, rep);
	    }

	  if (strong == 0)
	    {
	      snprintf(rep, sizeof(rep), "\
The %s server does not accept strong \"US grade\" ciphers\n\
with 112 or 128 bit long secret keys\n\
Nessus only counted %d weak \"export class\" and %d medium strength ciphers.\n\
Those ciphers only offer a limited protection against a brute force attack.\n\
\n\
Solution: update your server certificate and/or\n\
upgrade your SSL library or server software.",
		      trp_name, weak, medium);
	      post_hole(env, port, rep);
	    }
	  else if (weak > 0 || medium > 0)
	    {
	      snprintf(rep, sizeof(rep), "\
The %s server offers %d strong ciphers, but also\n\
%d medium strength and %d weak \"export class\" ciphers.\n\
The weak/medium ciphers may be chosen by an export-grade\n\
or badly configured client software. They only offer a \n\
limited protection against a brute force attack\n\
\n\
Solution: disable those ciphers and upgrade your client\n\
software if necessary",
		      trp_name, strong, medium, weak);
	      post_info(env, port, rep);
	    }
	}
    }

  
  prep = rep;
  *prep = '\0';

  if (! rejected[trp0 - NESSUS_ENCAPS_SSLv2])
    {
      for (trp = NESSUS_ENCAPS_SSLv2; trp <= NESSUS_ENCAPS_TLSv1; trp ++)
	if (rejected[trp - NESSUS_ENCAPS_SSLv2])
	  {
	    sprintf(prep, "This %s server does not accept %s connections.\n",
		    trp0_name, get_encaps_name(trp));
	    while (*prep != '\0')
	      prep++;
	  }
    }

  for (trp = NESSUS_ENCAPS_SSLv2; trp <= NESSUS_ENCAPS_TLSv1; trp ++)
    if (trp != trp0 && ! rejected[trp - NESSUS_ENCAPS_SSLv2])
      {
	sprintf(prep, "This %s server also accepts %s connections.\n",
		trp0_name, get_encaps_name(trp));
	    while (*prep != '\0')
	      prep++;
      }
  
  if (*rep != '\0')
    post_note(env, port, rep);

  if (cnx >= 0)
    close_stream_connection(cnx);
  return 0;
#endif
}
