/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.org
 */
#include "tomcrypt.h"
#include <stdarg.h>


/**
  @file der_encode_sequence_multi.c
  ASN.1 DER, encode a SEQUENCE, Tom St Denis
*/

#ifdef LTC_DER

int der_encode_sequence_multi(unsigned char *out, unsigned long *outlen, ...)
{
   int           err, type;
   unsigned long size, x;
   void          *data;
   va_list       args;
   ltc_asn1_list *list;

   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* get size of output that will be required */
   va_start(args, outlen);
   x = 0;
   for (;;) {
       type = va_arg(args, int);
       size = va_arg(args, unsigned long);
       data = va_arg(args, void*);

       if (type == LTC_ASN1_EOL) { 
          break;
       }

       switch (type) {
           case LTC_ASN1_INTEGER:
           case LTC_ASN1_SHORT_INTEGER:
           case LTC_ASN1_BIT_STRING:
           case LTC_ASN1_OCTET_STRING:
           case LTC_ASN1_NULL:
           case LTC_ASN1_OBJECT_IDENTIFIER:
           case LTC_ASN1_IA5_STRING:
           case LTC_ASN1_PRINTABLE_STRING:
           case LTC_ASN1_UTCTIME:
           case LTC_ASN1_SEQUENCE:
                ++x; 
                break;
          
           default:
               va_end(args);
               return CRYPT_INVALID_ARG;
       }
   }
   va_end(args);

   /* allocate structure for x elements */
   if (x == 0) {
      return CRYPT_NOP;
   }

   list = XCALLOC(sizeof(*list), x);
   if (list == NULL) {
      return CRYPT_MEM;
   }

   /* fill in the structure */
   va_start(args, outlen);
   x = 0;
   for (;;) {
       type = va_arg(args, int);
       size = va_arg(args, unsigned long);
       data = va_arg(args, void*);

       if (type == LTC_ASN1_EOL) { 
          break;
       }

       switch (type) {
           case LTC_ASN1_INTEGER:
           case LTC_ASN1_SHORT_INTEGER:
           case LTC_ASN1_BIT_STRING:
           case LTC_ASN1_OCTET_STRING:
           case LTC_ASN1_NULL:
           case LTC_ASN1_OBJECT_IDENTIFIER:
           case LTC_ASN1_IA5_STRING:
           case LTC_ASN1_PRINTABLE_STRING:
           case LTC_ASN1_UTCTIME:
           case LTC_ASN1_SEQUENCE:
                list[x].type   = type;
                list[x].size   = size;
                list[x++].data = data;
                break;
         
           default:
               va_end(args);
               err = CRYPT_INVALID_ARG;
               goto LBL_ERR;
       }
   }
   va_end(args);

   err = der_encode_sequence(list, x, out, outlen);   
LBL_ERR:
   XFREE(list);
   return err;
}

#endif


/* $Source$ */
/* $Revision$ */
/* $Date$ */
