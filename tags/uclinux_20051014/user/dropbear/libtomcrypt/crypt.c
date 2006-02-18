/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@iahu.ca, http://libtomcrypt.org
 */
#include "mycrypt.h"

/* Dropbear doesn't need these 

const char *crypt_build_settings =
   "LibTomCrypt " SCRYPT "\n\n"
   "Endianess: "
#if defined(ENDIAN_NEUTRAL)
   "neutral\n"
#elif defined(ENDIAN_LITTLE)
   "little"
   #if defined(ENDIAN_32BITWORD)
   " (32-bit words)\n"
   #else
   " (64-bit words)\n"
   #endif
#elif defined(ENDIAN_BIG)
   "big"
   #if defined(ENDIAN_32BITWORD)
   " (32-bit words)\n"
   #else
   " (64-bit words)\n"
   #endif
#endif
   "Clean stack: "
#if defined(CLEAN_STACK)
   "enabled\n"
#else
   "disabled\n"
#endif
   "Ciphers built-in:\n"
#if defined(BLOWFISH)
   "   Blowfish\n"
#endif
#if defined(RC2)
   "   RC2\n"
#endif
#if defined(RC5)
   "   RC5\n"
#endif
#if defined(RC6)
   "   RC6\n"
#endif
#if defined(SAFERP)
   "   Safer+\n"
#endif
#if defined(SAFER)
   "   Safer\n"
#endif
#if defined(RIJNDAEL)
   "   Rijndael\n"
#endif
#if defined(XTEA)
   "   XTEA\n"
#endif
#if defined(TWOFISH)
   "   Twofish "
   #if defined(TWOFISH_SMALL) && defined(TWOFISH_TABLES)
       "(small, tables)\n"
   #elif defined(TWOFISH_SMALL)
       "(small)\n"
   #elif defined(TWOFISH_TABLES)
       "(tables)\n"
   #else
       "\n"
   #endif
#endif
#if defined(DES)
   "   DES\n"
#endif
#if defined(CAST5)
   "   CAST5\n"
#endif
#if defined(NOEKEON)
   "   Noekeon\n"
#endif
#if defined(SKIPJACK)
   "   Skipjack\n"
#endif

    "\nHashes built-in:\n"
#if defined(SHA512)
   "   SHA-512\n"
#endif
#if defined(SHA384)
   "   SHA-384\n"
#endif
#if defined(SHA256)
   "   SHA-256\n"
#endif
#if defined(SHA224)
   "   SHA-224\n"
#endif
#if defined(TIGER)
   "   TIGER\n"
#endif
#if defined(SHA1)
   "   SHA1\n"
#endif
#if defined(MD5)
   "   MD5\n"
#endif
#if defined(MD4)
   "   MD4\n"
#endif
#if defined(MD2)
   "   MD2\n"
#endif
#if defined(RIPEMD128)
   "   RIPEMD128\n"
#endif
#if defined(RIPEMD160)
   "   RIPEMD160\n"
#endif
#if defined(WHIRLPOOL)
   "   WHIRLPOOL\n"
#endif

    "\nBlock Chaining Modes:\n"
#if defined(CFB)
    "   CFB\n"
#endif
#if defined(OFB)
    "   OFB\n"
#endif
#if defined(ECB)
    "   ECB\n"
#endif
#if defined(CBC)
    "   CBC\n"
#endif
#if defined(CTR)
    "   CTR\n"
#endif

    "\nPRNG:\n"
#if defined(YARROW)
    "   Yarrow\n"
#endif
#if defined(SPRNG)
    "   SPRNG\n"
#endif
#if defined(RC4)
    "   RC4\n"
#endif

    "\nPK Algs:\n"
#if defined(MRSA)
    "   RSA"
#if defined(RSA_TIMING)
    " + RSA_TIMING "
#endif
    "\n"
#endif
#if defined(MDH)
    "   DH\n"
#endif
#if defined(MECC)
    "   ECC\n"
#endif
#if defined(MDSA)
    "   DSA\n"
#endif

    "\nCompiler:\n"
#if defined(WIN32)
    "   WIN32 platform detected.\n"
#endif
#if defined(__CYGWIN__)
    "   CYGWIN Detected.\n"
#endif
#if defined(__DJGPP__)
    "   DJGPP Detected.\n"
#endif
#if defined(_MSC_VER)
    "   MSVC compiler detected.\n"
#endif
#if defined(__GNUC__)
    "   GCC compiler detected.\n"
#endif
#if defined(INTEL_CC)
    "   Intel C Compiler detected.\n"
#endif

    "\nVarious others: "
#if defined(BASE64)
    " BASE64 "
#endif
#if defined(MPI)
    " MPI "
#endif
#if defined(HMAC)
    " HMAC "
#endif
#if defined(OMAC)
    " OMAC "
#endif
#if defined(PMAC)
    " PMAC "
#endif
#if defined(EAX_MODE)
    " EAX_MODE "
#endif
#if defined(OCB_MODE)
    " OCB_MODE "
#endif
#if defined(TRY_UNRANDOM_FIRST)
    " TRY_UNRANDOM_FIRST "
#endif
#if defined(LTC_TEST)
    " LTC_TEST "
#endif
#if defined(PKCS_1)
    " PKCS#1 "
#endif
#if defined(PKCS_5)
    " PKCS#5 "
#endif
#if defined(SMALL_CODE)
    " SMALL_CODE "
#endif
#if defined(NO_FILE)
    " NO_FILE "
#endif
#if defined(LTC_TEST)
    " LTC_TEST "
#endif
    "\n"
    "\n\n\n"
    ;
	*/

