/* This is the worst code you have ever seen written on purpose.... this code is just a big hack to test
out the functionality of the library */

#ifdef SONY_PS2
#include <eetypes.h>
#include <eeregs.h>
#include "timer.h"
#endif

#include <mycrypt.h>

int     errnum;


int
null_setup (const unsigned char *key, int keylen, int num_rounds,
        symmetric_key * skey)
{
  return CRYPT_OK;
}

void
null_ecb_encrypt (const unsigned char *pt, unsigned char *ct,
          symmetric_key * key)
{
  memcpy (ct, pt, 8);
}

void
null_ecb_decrypt (const unsigned char *ct, unsigned char *pt,
          symmetric_key * key)
{
  memcpy (pt, ct, 8);
}

int
null_test (void)
{
  return CRYPT_OK;
}

int
null_keysize (int *desired_keysize)
{
  return CRYPT_OK;
}

const struct _cipher_descriptor null_desc = {
  "memcpy()",
  255,
  8, 8, 8, 1,
  &null_setup,
  &null_ecb_encrypt,
  &null_ecb_decrypt,
  &null_test,
  &null_keysize
};


prng_state prng;

void
store_tests (void)
{
  unsigned char buf[8];
  unsigned long L;
  ulong64 LL;

  printf ("LOAD32/STORE32 tests\n");
  L = 0x12345678UL;
  STORE32L (L, &buf[0]);
  L = 0;
  LOAD32L (L, &buf[0]);
  if (L != 0x12345678UL) {
    printf ("LOAD/STORE32 Little don't work\n");
    exit (-1);
  }
  LL = CONST64 (0x01020304050607);
  STORE64L (LL, &buf[0]);
  LL = 0;
  LOAD64L (LL, &buf[0])
    if (LL != CONST64 (0x01020304050607)) {
    printf ("LOAD/STORE64 Little don't work\n");
    exit (-1);
  }

  L = 0x12345678UL;
  STORE32H (L, &buf[0]);
  L = 0;
  LOAD32H (L, &buf[0]);
  if (L != 0x12345678UL) {
    printf ("LOAD/STORE32 High don't work, %08lx\n", L);
    exit (-1);
  }
  LL = CONST64 (0x01020304050607);
  STORE64H (LL, &buf[0]);
  LL = 0;
  LOAD64H (LL, &buf[0])
    if (LL != CONST64 (0x01020304050607)) {
    printf ("LOAD/STORE64 High don't work\n");
    exit (-1);
  }
}

void
cipher_tests (void)
{
  int     x;

  printf ("Ciphers compiled in\n");
  for (x = 0; cipher_descriptor[x].name != NULL; x++) {
    printf
      (" %12s (%2d) Key Size: %4d to %4d, Block Size: %3d, Default # of rounds: %2d\n",
       cipher_descriptor[x].name, cipher_descriptor[x].ID,
       cipher_descriptor[x].min_key_length * 8,
       cipher_descriptor[x].max_key_length * 8,
       cipher_descriptor[x].block_length * 8,
       cipher_descriptor[x].default_rounds);
  }

}

void
ecb_tests (void)
{
  int     x;

  printf ("ECB tests\n");
  for (x = 0; cipher_descriptor[x].name != NULL; x++) {
    printf (" %12s: ", cipher_descriptor[x].name);
    if ((errnum = cipher_descriptor[x].test ()) != CRYPT_OK) {
      printf (" **failed** Reason: %s\n", error_to_string (errnum));
      exit (-1);
    } else {
      printf ("passed\n");
    }
  }
}

#ifdef CBC
void
cbc_tests (void)
{
  symmetric_CBC cbc;
  int     x, y;
  unsigned char blk[32], ct[32], key[32], IV[32];
  const unsigned char test[] =
    { 0XFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  printf ("CBC tests\n");
  /* ---- CBC ENCODING ---- */
  /* make up a block and IV */
  for (x = 0; x < 32; x++)
    blk[x] = IV[x] = x;

  /* now lets start a cbc session */
  if ((errnum =
       cbc_start (find_cipher ("blowfish"), IV, key, 16, 0,
          &cbc)) != CRYPT_OK) {
    printf ("CBC Setup: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* now lets encode 32 bytes */
  for (x = 0; x < 4; x++) {
    if ((errnum = cbc_encrypt (blk + 8 * x, ct + 8 * x, &cbc)) != CRYPT_OK) {
      printf ("CBC encrypt: %s\n", error_to_string (errnum));
      exit (-1);
    }
  }

  zeromem (blk, sizeof (blk));

  /* ---- CBC DECODING ---- */
  /* make up a IV */
  for (x = 0; x < 32; x++)
    IV[x] = x;

  /* now lets start a cbc session */
  if ((errnum =
       cbc_start (find_cipher ("blowfish"), IV, key, 16, 0,
          &cbc)) != CRYPT_OK) {
    printf ("CBC Setup: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* now lets decode 32 bytes */
  for (x = 0; x < 4; x++) {
    if ((errnum = cbc_decrypt (ct + 8 * x, blk + 8 * x, &cbc)) != CRYPT_OK) {
      printf ("CBC decrypt: %s\n", error_to_string (errnum));
      exit (-1);
    }
  }


  /* print output */
  for (x = y = 0; x < 32; x++)
    if (blk[x] != x)
      y = 1;
  printf ("  %s\n", y ? "failed" : "passed");

  /* lets actually check the bytes */
  memset (IV, 0, 8);
  IV[0] = 0xFF;         /* IV  = FF 00 00 00 00 00 00 00 */
  memset (blk, 0, 32);
  blk[8] = 0xFF;        /* BLK = 00 00 00 00 00 00 00 00 FF 00 00 00 00 00 00 00 */
  cbc_start (find_cipher ("memcpy()"), IV, key, 8, 0, &cbc);
  cbc_encrypt (blk, ct, &cbc);  /* expect: FF 00 00 00 00 00 00 00 */
  cbc_encrypt (blk + 8, ct + 8, &cbc);  /* expect: 00 00 00 00 00 00 00 00 */
  if (memcmp (ct, test, 16)) {
    printf ("CBC failed logical testing.\n");
    for (x = 0; x < 16; x++)
      printf ("%02x ", ct[x]);
    printf ("\n");
    exit (-1);
  } else {
    printf ("CBC passed logical testing.\n");
  }
}
#else
void
cbc_tests (void)
{
  printf ("CBC not compiled in\n");
}
#endif

#ifdef OFB
void
ofb_tests (void)
{
  symmetric_OFB ofb;
  int     x, y;
  unsigned char blk[32], ct[32], key[32], IV[32];

  printf ("OFB tests\n");
  /* ---- ofb ENCODING ---- */
  /* make up a block and IV */
  for (x = 0; x < 32; x++)
    blk[x] = IV[x] = x;

  /* now lets start a ofb session */
  if ((errnum =
       ofb_start (find_cipher ("cast5"), IV, key, 16, 0, &ofb)) != CRYPT_OK) {
    printf ("OFB Setup: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* now lets encode 32 bytes */
  for (x = 0; x < 4; x++) {
    if ((errnum = ofb_encrypt (blk + 8 * x, ct + 8 * x, 8, &ofb)) != CRYPT_OK) {
      printf ("OFB encrypt: %s\n", error_to_string (errnum));
      exit (-1);
    }
  }

  zeromem (blk, sizeof (blk));

  /* ---- ofb DECODING ---- */
  /* make up a IV */
  for (x = 0; x < 32; x++)
    IV[x] = x;

  /* now lets start a ofb session */
  if ((errnum =
       ofb_start (find_cipher ("cast5"), IV, key, 16, 0, &ofb)) != CRYPT_OK) {
    printf ("OFB setup: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* now lets decode 32 bytes */
  for (x = 0; x < 4; x++) {
    if ((errnum = ofb_decrypt (ct + 8 * x, blk + 8 * x, 8, &ofb)) != CRYPT_OK) {
      printf ("OFB decrypt: %s\n", error_to_string (errnum));
      exit (-1);
    }
  }

  /* print output */
  for (x = y = 0; x < 32; x++)
    if (blk[x] != x)
      y = 1;
  printf ("  %s\n", y ? "failed" : "passed");
  if (y)
    exit (-1);
}
#else
void
ofb_tests (void)
{
  printf ("OFB not compiled in\n");
}
#endif

#ifdef CFB
void
cfb_tests (void)
{
  symmetric_CFB cfb;
  int     x, y;
  unsigned char blk[32], ct[32], key[32], IV[32];

  printf ("CFB tests\n");
  /* ---- cfb ENCODING ---- */
  /* make up a block and IV */
  for (x = 0; x < 32; x++)
    blk[x] = IV[x] = x;

  /* now lets start a cfb session */
  if ((errnum =
       cfb_start (find_cipher ("blowfish"), IV, key, 16, 0,
          &cfb)) != CRYPT_OK) {
    printf ("CFB setup: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* now lets encode 32 bytes */
  for (x = 0; x < 4; x++) {
    if ((errnum = cfb_encrypt (blk + 8 * x, ct + 8 * x, 8, &cfb)) != CRYPT_OK) {
      printf ("CFB encrypt: %s\n", error_to_string (errnum));
      exit (-1);
    }
  }

  zeromem (blk, sizeof (blk));

  /* ---- cfb DECODING ---- */
  /* make up ahash_descriptor[prng->yarrow.hash].hashsize IV */
  for (x = 0; x < 32; x++)
    IV[x] = x;

  /* now lets start a cfb session */
  if ((errnum =
       cfb_start (find_cipher ("blowfish"), IV, key, 16, 0,
          &cfb)) != CRYPT_OK) {
    printf ("CFB Setup: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* now lets decode 32 bytes */
  for (x = 0; x < 4; x++) {
    if ((errnum = cfb_decrypt (ct + 8 * x, blk + 8 * x, 8, &cfb)) != CRYPT_OK) {
      printf ("CFB decrypt: %s\n", error_to_string (errnum));
      exit (-1);
    }
  }

  /* print output */
  for (x = y = 0; x < 32; x++)
    if (blk[x] != x)
      y = 1;
  printf ("  %s\n", y ? "failed" : "passed");
  if (y)
    exit (-1);
}
#else
void
cfb_tests (void)
{
  printf ("CFB not compiled in\n");
}
#endif

#ifdef CTR
void
ctr_tests (void)
{
  symmetric_CTR ctr;
  int     x, y;
  unsigned char blk[32], ct[32], key[32], count[32];
  const unsigned char test[] =
    { 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0 };

  printf ("CTR tests\n");
  /* ---- CTR ENCODING ---- */
  /* make up a block and IV */
  for (x = 0; x < 32; x++)
    blk[x] = count[x] = x;

  /* now lets start a ctr session */
  if ((errnum =
       ctr_start (find_cipher ("xtea"), count, key, 16, 0,
          &ctr)) != CRYPT_OK) {
    printf ("CTR Setup: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* now lets encode 32 bytes */
  for (x = 0; x < 4; x++) {
    if ((errnum = ctr_encrypt (blk + 8 * x, ct + 8 * x, 8, &ctr)) != CRYPT_OK) {
      printf ("CTR encrypt: %s\n", error_to_string (errnum));
      exit (-1);
    }
  }

  zeromem (blk, sizeof (blk));

  /* ---- CTR DECODING ---- */
  /* make up a IV */
  for (x = 0; x < 32; x++)
    count[x] = x;

  /* now lets start a cbc session */
  if ((errnum =
       ctr_start (find_cipher ("xtea"), count, key, 16, 0,
          &ctr)) != CRYPT_OK) {
    printf ("CTR Setup: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* now lets decode 32 bytes */
  for (x = 0; x < 4; x++) {
    if ((errnum = ctr_decrypt (ct + 8 * x, blk + 8 * x, 8, &ctr)) != CRYPT_OK) {
      printf ("CTR decrypt: %s\n", error_to_string (errnum));
      exit (-1);
    }
  }

  /* print output */
  for (x = y = 0; x < 32; x++)
    if (blk[x] != x)
      y = 1;
  printf ("  %s\n", y ? "failed" : "passed");
  if (y)
    exit (-1);

  /* lets actually check the bytes */
  memset (count, 0, 8);
  count[0] = 0xFF;      /* IV  = FF 00 00 00 00 00 00 00 */
  memset (blk, 0, 32);
  blk[9] = 2;           /* BLK = 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 00 */
  ctr_start (find_cipher ("memcpy()"), count, key, 8, 0, &ctr);
  ctr_encrypt (blk, ct, 8, &ctr);   /* expect: FF 00 00 00 00 00 00 00 */
  ctr_encrypt (blk + 8, ct + 8, 8, &ctr);   /* expect: 00 03 00 00 00 00 00 00 */
  if (memcmp (ct, test, 16)) {
    printf ("CTR failed logical testing.\n");
    for (x = 0; x < 16; x++)
      printf ("%02x ", ct[x]);
    printf ("\n");
  } else {
    printf ("CTR passed logical testing.\n");
  }

}
#else
void
ctr_tests (void)
{
  printf ("CTR not compiled in\n");
}
#endif

void
hash_tests (void)
{
  int     x;
  printf ("Hash tests\n");
  for (x = 0; hash_descriptor[x].name != NULL; x++) {
    printf (" %10s (%2d) ", hash_descriptor[x].name, hash_descriptor[x].ID);
    if ((errnum = hash_descriptor[x].test ()) != CRYPT_OK) {
      printf ("**failed** Reason: %s\n", error_to_string (errnum));
      exit(-1);
    } else {
      printf ("passed\n");
    }
  }
}

#ifdef MRSA
void
pad_test (void)
{
  unsigned char in[100], out[100];
  unsigned long x, y;

  /* make a dummy message */
  for (x = 0; x < 16; x++)
    in[x] = (unsigned char) x;

  /* pad the message so that random filler is placed before and after it */
  y = 100;
  if ((errnum =
       rsa_pad (in, 16, out, &y, find_prng ("yarrow"), &prng)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* depad the message to get the original content */
  memset (in, 0, sizeof (in));
  x = 100;
  if ((errnum = rsa_depad (out, y, in, &x)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* check outcome */
  printf ("rsa_pad: ");
  if (x != 16) {
    printf ("Failed.  Wrong size.\n");
    exit (-1);
  }
  for (x = 0; x < 16; x++)
    if (in[x] != x) {
      printf ("Failed.  Expected %02lx and got %02x.\n", x, in[x]);
      exit (-1);
    }
  printf ("passed.\n");
}
void
rsa_test (void)
{
  unsigned char in[520], out[520];
  unsigned long x, y, z, limit;
  int     stat;
  rsa_key key;
  clock_t t;

  /* ---- SINGLE ENCRYPT ---- */
  /* encrypt a short 8 byte string */
  if ((errnum =
       rsa_make_key (&prng, find_prng ("yarrow"), 1024 / 8, 65537,
             &key)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  for (x = 0; x < 8; x++)
    in[x] = (unsigned char) (x + 1);
  y = sizeof (in);
  if ((errnum = rsa_exptmod (in, 8, out, &y, PK_PUBLIC, &key)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* decrypt it */
  zeromem (in, sizeof (in));
  x = sizeof (out);
  if ((errnum = rsa_exptmod (out, y, in, &x, PK_PRIVATE, &key)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* compare */
  printf ("RSA    : ");
  for (x = 0; x < 8; x++)
    if (in[x] != (x + 1)) {
      printf ("Failed.  x==%02lx, in[%ld]==%02x\n", x, x, in[x]);
      exit (-1);
    }
  printf ("passed.\n");

  /* test the rsa_encrypt_key functions */
  for (x = 0; x < 16; x++)
    in[x] = x;
  y = sizeof (out);
  if ((errnum =
       rsa_encrypt_key (in, 16, out, &y, &prng, find_prng ("yarrow"),
            &key)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  zeromem (in, sizeof (in));
  x = sizeof (in);
  if ((errnum = rsa_decrypt_key (out, y, in, &x, &key)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("RSA en/de crypt key routines: ");
  if (x != 16) {
    printf ("Failed (length)\n");
    exit (-1);
  }
  for (x = 0; x < 16; x++)
    if (in[x] != x) {
      printf ("Failed (contents)\n");
      exit (-1);
    }
  printf ("Passed\n");

  /* test sign_hash functions */
  for (x = 0; x < 16; x++)
    in[x] = x;
  x = sizeof (in);
  if ((errnum = rsa_sign_hash (in, 16, out, &x, &key)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("RSA signed hash: %lu bytes\n", x);
  if ((errnum = rsa_verify_hash (out, x, in, &stat, &key)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("Verify hash: %s, ", stat ? "passed" : "failed");
  in[0] ^= 1;
  if ((errnum = rsa_verify_hash (out, x, in, &stat, &key)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("%s\n", (!stat) ? "passed" : "failed");
  if (stat)
    exit (-1);
  rsa_free (&key);

  /* make a RSA key */
#ifdef SONY_PS2_NOPE
  limit = 1024;
#else
  limit = 2048;
#endif

  {
    int     tt;

    for (z = 1024; z <= limit; z += 512) {
      t = XCLOCK ();
      for (tt = 0; tt < 3; tt++) {
         if ((errnum = rsa_make_key (&prng, find_prng ("yarrow"), z / 8, 65537, &key)) != CRYPT_OK) {
            printf ("Error: %s\n", error_to_string (errnum));
            exit (-1);
         }

         /* check modulus size */
         if (mp_unsigned_bin_size(&key.N) != (int)(z/8)) { 
            printf("\nRSA key supposed to be %lu bits but was %d bits\n", z, mp_count_bits(&key.N));
            exit(EXIT_FAILURE);
         }

         if (tt < 2) {
            rsa_free (&key);
         }
      }
      t = XCLOCK () - t;
      printf ("Took %.0f ms to make a %ld-bit RSA key.\n", 1000.0 * (((double) t / 3.0) / (double) XCLOCKS_PER_SEC), z);

      /* time encryption */
      t = XCLOCK ();

      for (tt = 0; tt < 20; tt++) {
         y = sizeof (in);
         if ((errnum = rsa_exptmod (in, 8, out, &y, PK_PUBLIC, &key)) != CRYPT_OK) {
            printf ("Error: %s\n", error_to_string (errnum));
            exit (-1);
         }
      }
      t = XCLOCK () - t;
      printf ("Took %.0f ms to encrypt with a %ld-bit RSA key.\n",
              1000.0 * (((double) t / 20.0) / (double) XCLOCKS_PER_SEC), z);

      /* time decryption */
      t = XCLOCK ();
      for (tt = 0; tt < 20; tt++) {
          x = sizeof (out);
          if ((errnum = rsa_exptmod (out, y, in, &x, PK_PRIVATE, &key)) != CRYPT_OK) {
             printf ("Error: %s\n", error_to_string (errnum));
             exit (-1);
          }
      }
      t = XCLOCK () - t;
      printf ("Took %.0f ms to decrypt with a %ld-bit RSA key.\n",
      1000.0 * (((double) t / 20.0) / (double) XCLOCKS_PER_SEC), z);
      rsa_free (&key);
    }
  }
}
#else
void
pad_test (void)
{
  printf ("MRSA not compiled in\n");
}

void
rsa_test (void)
{
  printf ("MRSA not compiled in\n");
}
#endif

#ifdef BASE64
void
base64_test (void)
{
  unsigned char buf[2][100];
  unsigned long x, y;

  printf ("Base64 tests\n");
  zeromem (buf, sizeof (buf));
  for (x = 0; x < 16; x++)
    buf[0][x] = (unsigned char) x;

  x = 100;
  if (base64_encode (buf[0], 16, buf[1], &x) != CRYPT_OK) {
    printf ("  error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("  encoded 16 bytes to %ld bytes...[%s]\n", x, buf[1]);
  memset (buf[0], 0, 100);
  y = 100;
  if (base64_decode (buf[1], x, buf[0], &y) != CRYPT_OK) {
    printf ("  error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("  decoded %ld bytes to %ld bytes\n", x, y);
  for (x = 0; x < 16; x++)
    if (buf[0][x] != x) {
      printf (" **failed**\n");
      exit (-1);
    }
  printf ("  passed\n");
}
#else
void
base64_test (void)
{
  printf ("Base64 not compiled in\n");
}
#endif

void
time_hash (void)
{
  clock_t t1;
  int     x, y;
  unsigned long z;
  unsigned char input[4096], out[MAXBLOCKSIZE];
  printf ("Hash Time Trials (4KB blocks):\n");
  for (x = 0; hash_descriptor[x].name != NULL; x++) {
    t1 = XCLOCK ();
    z = sizeof (out);
    y = 0;
    while (XCLOCK () - t1 < (5 * XCLOCKS_PER_SEC)) {
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      hash_memory (x, input, 4096, out, &z);
      y += 32;
    }
    t1 = XCLOCK () - t1;
    printf ("%-20s: Hash at %5.2f Mbit/sec\n", hash_descriptor[x].name,
        ((8.0 * 4096.0) *
         ((double) y / ((double) t1 / (double) XCLOCKS_PER_SEC))) /
        1000000.0);
  }
}

void
time_ecb (void)
{
  clock_t t1, t2;
  long    x, y1, y2;
  unsigned char pt[32], key[32];
  symmetric_key skey;
  void    (*func) (const unsigned char *, unsigned char *, symmetric_key *);

  printf ("ECB Time Trials for the Symmetric Ciphers:\n");
  for (x = 0; cipher_descriptor[x].name != NULL; x++) {
    cipher_descriptor[x].setup (key, cipher_descriptor[x].min_key_length, 0,
                &skey);

#define DO1   func(pt,pt,&skey);
#define DO2   DO1 DO1
#define DO4   DO2 DO2
#define DO8   DO4 DO4
#define DO16  DO8 DO8
#define DO32  DO16 DO16
#define DO64  DO32 DO32
#define DO128 DO64 DO64
#define DO256 DO128 DO128

    func = cipher_descriptor[x].ecb_encrypt;
    y1 = 0;
    t1 = XCLOCK ();
    while (XCLOCK () - t1 < 3 * XCLOCKS_PER_SEC) {
      DO256;
      y1 += 256;
    }
    t1 = XCLOCK () - t1;

    func = cipher_descriptor[x].ecb_decrypt;
    y2 = 0;
    t2 = XCLOCK ();
    while (XCLOCK () - t2 < 3 * XCLOCKS_PER_SEC) {
      DO256;
      y2 += 256;
    }
    t2 = XCLOCK () - t2;
    printf
      ("%-20s: Encrypt at %5.2f Mbit/sec and Decrypt at %5.2f Mbit/sec\n",
       cipher_descriptor[x].name,
       ((8.0 * (double) cipher_descriptor[x].block_length) *
    ((double) y1 / ((double) t1 / (double) XCLOCKS_PER_SEC))) / 1000000.0,
       ((8.0 * (double) cipher_descriptor[x].block_length) *
    ((double) y2 / ((double) t2 / (double) XCLOCKS_PER_SEC))) /
       1000000.0);

#undef DO256
#undef DO128
#undef DO64
#undef DO32
#undef DO16
#undef DO8
#undef DO4
#undef DO2
#undef DO1
  }
}

#ifdef MDH
void
dh_tests (void)
{
  unsigned char buf[3][4096];
  unsigned long x, y, z;
  int     low, high, stat, stat2;
  dh_key  usera, userb;
  clock_t t1;

  printf("Testing builting DH parameters...."); fflush(stdout);
  if ((errnum = dh_test()) != CRYPT_OK) {
     printf("DH Error: %s\n", error_to_string(errnum));
     exit(-1);
  }
  printf("Passed.\n");

  dh_sizes (&low, &high);
  printf ("DH Keys from %d to %d supported.\n", low * 8, high * 8);

  /* make up two keys */
  if ((errnum =
       dh_make_key (&prng, find_prng ("yarrow"), 96, &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  if ((errnum =
       dh_make_key (&prng, find_prng ("yarrow"), 96, &userb)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* make the shared secret */
  x = 4096;
  if ((errnum = dh_shared_secret (&usera, &userb, buf[0], &x)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  y = 4096;
  if ((errnum = dh_shared_secret (&userb, &usera, buf[1], &y)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  if (y != x) {
    printf ("DH Shared keys are not same size.\n");
    exit (-1);
  }
  if (memcmp (buf[0], buf[1], x)) {
    printf ("DH Shared keys not same contents.\n");
    exit (-1);
  }

  /* now export userb */
  y = 4096;
  if ((errnum = dh_export (buf[1], &y, PK_PUBLIC, &userb)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  dh_free (&userb);

  /* import and make the shared secret again */
  if ((errnum = dh_import (buf[1], y, &userb)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  z = 4096;
  if ((errnum = dh_shared_secret (&usera, &userb, buf[2], &z)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  printf ("DH routines: ");
  if (z != x) {
    printf ("failed.  Size don't match?\n");
    exit (-1);
  }
  if (memcmp (buf[0], buf[2], x)) {
    printf ("Failed.  Content didn't match.\n");
    exit (-1);
  }
  printf ("Passed\n");
  dh_free (&usera);
  dh_free (&userb);

/* time stuff */
  {
    static int sizes[] = { 96, 128, 160, 192, 224, 256, 320, 384, 512 };
    int     ii, tt;

    for (ii = 0; ii < (int) (sizeof (sizes) / sizeof (sizes[0])); ii++) {
      t1 = XCLOCK ();
      for (tt = 0; tt < 25; tt++) {
    dh_make_key (&prng, find_prng ("yarrow"), sizes[ii], &usera);
    dh_free (&usera);
      }
      t1 = XCLOCK () - t1;
      printf ("Make dh-%d key took %f msec\n", sizes[ii] * 8,
          1000.0 * (((double) t1 / 25.0) / (double) XCLOCKS_PER_SEC));
    }
  }

/* test encrypt_key */
  dh_make_key (&prng, find_prng ("yarrow"), 128, &usera);
  for (x = 0; x < 16; x++)
    buf[0][x] = x;
  y = sizeof (buf[1]);
  if ((errnum =
       dh_encrypt_key (buf[0], 16, buf[1], &y, &prng, find_prng ("yarrow"),
               find_hash ("md5"), &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  zeromem (buf[0], sizeof (buf[0]));
  x = sizeof (buf[0]);
  if ((errnum = dh_decrypt_key (buf[1], y, buf[0], &x, &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("DH en/de crypt key routines: ");
  if (x != 16) {
    printf ("Failed (length)\n");
    exit (-1);
  }
  for (x = 0; x < 16; x++)
    if (buf[0][x] != x) {
      printf ("Failed (contents)\n");
      exit (-1);
    }
  printf ("Passed (size %lu)\n", y);

/* test sign_hash */
  for (x = 0; x < 16; x++)
    buf[0][x] = x;
  x = sizeof (buf[1]);
  if ((errnum =
       dh_sign_hash (buf[0], 16, buf[1], &x, &prng, find_prng ("yarrow"),
             &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  if ((errnum = dh_verify_hash (buf[1], x, buf[0], 16, &stat, &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  buf[0][0] ^= 1;
  if ((errnum = dh_verify_hash (buf[1], x, buf[0], 16, &stat2, &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("dh_sign/verify_hash: %s (%d,%d), %lu\n",
      ((stat == 1)
       && (stat2 == 0)) ? "passed" : "failed", stat, stat2, x);
  dh_free (&usera);
}
#else
void
dh_tests (void)
{
  printf ("MDH not compiled in\n");
}
#endif

int     callback_x = 0;
void
callback (void)
{
  printf ("%c\x08", "-\\|/"[++callback_x & 3]);
#ifndef SONY_PS2
  fflush (stdout);
#endif
}

void
rng_tests (void)
{
  unsigned char buf[16];
  clock_t t1;
  int     x, y;

  printf ("RNG tests\n");
  t1 = XCLOCK ();
  x = rng_get_bytes (buf, sizeof (buf), &callback);
  t1 = XCLOCK () - t1;
  printf ("  %f bytes per second...",
      (double) x / ((double) t1 / (double) XCLOCKS_PER_SEC));
  printf ("read %d bytes.\n  ", x);
  for (y = 0; y < x; y++)
    printf ("%02x ", buf[y]);
  printf ("\n");

#ifdef YARROW
  if ((errnum =
       rng_make_prng (128, find_prng ("yarrow"), &prng,
              &callback)) != CRYPT_OK) {
    printf (" starting yarrow error: %s\n", error_to_string (errnum));
    exit (-1);
  }
#endif
}

#ifdef MECC
void
ecc_tests (void)
{
  unsigned char buf[4][4096];
  unsigned long x, y, z;
  int     stat, stat2, low, high;
  ecc_key usera, userb;
  clock_t t1;

  if ((errnum = ecc_test ()) != CRYPT_OK) {
    printf ("ecc Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  ecc_sizes (&low, &high);
  printf ("ecc Keys from %d to %d supported.\n", low * 8, high * 8);

  /* make up two keys */
  if ((errnum =
       ecc_make_key (&prng, find_prng ("yarrow"), 24, &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  if ((errnum =
       ecc_make_key (&prng, find_prng ("yarrow"), 24, &userb)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  /* make the shared secret */
  x = 4096;
  if ((errnum = ecc_shared_secret (&usera, &userb, buf[0], &x)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  y = 4096;
  if ((errnum = ecc_shared_secret (&userb, &usera, buf[1], &y)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  if (y != x) {
    printf ("ecc Shared keys are not same size.\n");
    exit (-1);
  }

  if (memcmp (buf[0], buf[1], x)) {
    printf ("ecc Shared keys not same contents.\n");
    exit (-1);
  }

  /* now export userb */
  y = 4096;
  if ((errnum = ecc_export (buf[1], &y, PK_PUBLIC, &userb)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  ecc_free (&userb);
  printf ("ECC-192 export took %ld bytes\n", y);

  /* import and make the shared secret again */
  if ((errnum = ecc_import (buf[1], y, &userb)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  z = 4096;
  if ((errnum = ecc_shared_secret (&usera, &userb, buf[2], &z)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }

  printf ("ecc routines: ");
  if (z != x) {
    printf ("failed.  Size don't match?\n");
    exit (-1);
  }
  if (memcmp (buf[0], buf[2], x)) {
    printf ("Failed.  Content didn't match.\n");
    exit (-1);
  }
  printf ("Passed\n");
  ecc_free (&usera);
  ecc_free (&userb);

/* time stuff */
  {
    static int sizes[] = { 20, 24, 28, 32, 48, 65 };
    int     ii, tt;

    for (ii = 0; ii < (int) (sizeof (sizes) / sizeof (sizes[0])); ii++) {
      t1 = XCLOCK ();
      for (tt = 0; tt < 10; tt++) {
    if ((errnum =
         ecc_make_key (&prng, find_prng ("yarrow"), sizes[ii],
               &usera)) != CRYPT_OK) {
      printf ("Error: %s\n", error_to_string (errnum));
      exit (-1);
    }
    ecc_free (&usera);
      }
      t1 = XCLOCK () - t1;
      printf ("Make ECC-%d key took %f msec\n", sizes[ii] * 8,
          1000.0 * (((double) t1 / 10.0) / (double) XCLOCKS_PER_SEC));
    }
  }

/* test encrypt_key */
  ecc_make_key (&prng, find_prng ("yarrow"), 20, &usera);
  for (x = 0; x < 32; x++)
    buf[0][x] = x;
  y = sizeof (buf[1]);
  if ((errnum =
       ecc_encrypt_key (buf[0], 32, buf[1], &y, &prng, find_prng ("yarrow"),
            find_hash ("sha256"), &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  zeromem (buf[0], sizeof (buf[0]));
  x = sizeof (buf[0]);
  if ((errnum = ecc_decrypt_key (buf[1], y, buf[0], &x, &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("ECC en/de crypt key routines: ");
  if (x != 32) {
    printf ("Failed (length)\n");
    exit (-1);
  }
  for (x = 0; x < 32; x++)
    if (buf[0][x] != x) {
      printf ("Failed (contents)\n");
      exit (-1);
    }
  printf ("Passed (size: %lu)\n", y);
/* test sign_hash */
  for (x = 0; x < 16; x++)
    buf[0][x] = x;
  x = sizeof (buf[1]);
  if ((errnum =
       ecc_sign_hash (buf[0], 16, buf[1], &x, &prng, find_prng ("yarrow"),
              &usera)) != CRYPT_OK) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf("Signature size: %lu\n", x);
  if (ecc_verify_hash (buf[1], x, buf[0], 16, &stat, &usera)) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  buf[0][0] ^= 1;
  if (ecc_verify_hash (buf[1], x, buf[0], 16, &stat2, &usera)) {
    printf ("Error: %s\n", error_to_string (errnum));
    exit (-1);
  }
  printf ("ecc_sign/verify_hash: %s (%d,%d)\n",
      ((stat == 1) && (stat2 == 0)) ? "passed" : "failed", stat, stat2);
  ecc_free (&usera);
}
#else
void
ecc_tests (void)
{
  printf ("MECC not compiled in\n");
}
#endif

#ifdef MPI
void
test_prime (void)
{
  char buf[1024];
  mp_int  a;
  int     x;

  /* make a 1024 bit prime */
  mp_init (&a);
  rand_prime (&a, 128*8, &prng, find_prng ("yarrow"));

  /* dump it */
  mp_todecimal (&a, buf);
  printf ("1024-bit prime:\n");
  for (x = 0; x < (int) strlen (buf);) {
    printf ("%c", buf[x]);
    if (!(++x % 60))
      printf ("\\ \n");
  }
  printf ("\n\n");

  mp_clear (&a);
}
#else
void
test_prime (void)
{
  printf ("MPI not compiled in\n");
}
#endif

void
register_all_algs (void)
{
#ifdef RIJNDAEL
  register_cipher (&aes_desc);
#endif
#ifdef BLOWFISH
  register_cipher (&blowfish_desc);
#endif
#ifdef XTEA
  register_cipher (&xtea_desc);
#endif
#ifdef RC5
  register_cipher (&rc5_desc);
#endif
#ifdef RC6
  register_cipher (&rc6_desc);
#endif
#ifdef SAFERP
  register_cipher (&saferp_desc);
#endif
#ifdef TWOFISH
  register_cipher (&twofish_desc);
#endif
#ifdef SAFER
  register_cipher (&safer_k64_desc);
  register_cipher (&safer_sk64_desc);
  register_cipher (&safer_k128_desc);
  register_cipher (&safer_sk128_desc);
#endif
#ifdef RC2
  register_cipher (&rc2_desc);
#endif
#ifdef DES
  register_cipher (&des_desc);
  register_cipher (&des3_desc);
#endif
#ifdef CAST5
  register_cipher (&cast5_desc);
#endif
#ifdef NOEKEON
  register_cipher (&noekeon_desc);
#endif
#ifdef SKIPJACK
  register_cipher (&skipjack_desc);
#endif
  register_cipher (&null_desc);

#ifdef TIGER
  register_hash (&tiger_desc);
#endif
#ifdef MD2
  register_hash (&md2_desc);
#endif
#ifdef MD4
  register_hash (&md4_desc);
#endif
#ifdef MD5
  register_hash (&md5_desc);
#endif
#ifdef SHA1
  register_hash (&sha1_desc);
#endif
#ifdef SHA256
  register_hash (&sha256_desc);
#endif
#ifdef SHA224
  register_hash (&sha224_desc);
#endif
#ifdef SHA384
  register_hash (&sha384_desc);
#endif
#ifdef SHA512
  register_hash (&sha512_desc);
#endif
#ifdef RIPEMD128
  register_hash (&rmd128_desc);
#endif
#ifdef RIPEMD160
  register_hash (&rmd160_desc);
#endif
#ifdef WHIRLPOOL
  register_hash (&whirlpool_desc);
#endif

#ifdef YARROW
  register_prng (&yarrow_desc);
#endif
#ifdef SPRNG
  register_prng (&sprng_desc);
#endif
}

void
test_errs (void)
{
#define ERR(x)  printf("%25s => %s\n", #x, error_to_string(x));

  ERR (CRYPT_OK);
  ERR (CRYPT_ERROR);

  ERR (CRYPT_INVALID_KEYSIZE);
  ERR (CRYPT_INVALID_ROUNDS);
  ERR (CRYPT_FAIL_TESTVECTOR);

  ERR (CRYPT_BUFFER_OVERFLOW);
  ERR (CRYPT_INVALID_PACKET);

  ERR (CRYPT_INVALID_PRNGSIZE);
  ERR (CRYPT_ERROR_READPRNG);

  ERR (CRYPT_INVALID_CIPHER);
  ERR (CRYPT_INVALID_HASH);
  ERR (CRYPT_INVALID_PRNG);

  ERR (CRYPT_MEM);

  ERR (CRYPT_PK_TYPE_MISMATCH);
  ERR (CRYPT_PK_NOT_PRIVATE);

  ERR (CRYPT_INVALID_ARG);
  ERR (CRYPT_FILE_NOTFOUND);

  ERR (CRYPT_PK_INVALID_TYPE);
  ERR (CRYPT_PK_INVALID_SYSTEM);
  ERR (CRYPT_PK_DUP);
  ERR (CRYPT_PK_NOT_FOUND);
  ERR (CRYPT_PK_INVALID_SIZE);

  ERR (CRYPT_INVALID_PRIME_SIZE);
}


void dsa_tests(void)
{
   unsigned char msg[16], out[1024], out2[1024];
   unsigned long x, y;
   int err, stat1, stat2;
   dsa_key key, key2;

   /* make a random key */
   if ((err = dsa_make_key(&prng, find_prng("yarrow"), 20, 128, &key)) != CRYPT_OK) {
      printf("Error making DSA key: %s\n", error_to_string(err));
      exit(-1);
   }
   printf("DSA Key Made\n");

   /* verify it */
   if ((err = dsa_verify_key(&key, &stat1)) != CRYPT_OK) {
      printf("Error verifying DSA key: %s\n", error_to_string(err));
      exit(-1);
   }
   printf("DSA key verification: %s\n", stat1 == 1 ? "passed" : "failed");
   if (stat1 == 0) exit(-1);     

   /* sign the message */
   x = sizeof(out);
   if ((err = dsa_sign_hash(msg, sizeof(msg), out, &x, &prng, find_prng("yarrow"), &key)) != CRYPT_OK) {
      printf("Error signing with DSA key: %s\n", error_to_string(err));
      exit(-1);
   }
   printf("DSA 160/1024 signature is %lu bytes long\n", x);

   /* verify it once */
   if ((err = dsa_verify_hash(out, x, msg, sizeof(msg), &stat1, &key)) != CRYPT_OK) {
      printf("Error verifying with DSA key 1: %s\n", error_to_string(err));
      exit(-1);
   }

   /* Modify and verify again */
   msg[0] ^= 1;
   if ((err = dsa_verify_hash(out, x, msg, sizeof(msg), &stat2, &key)) != CRYPT_OK) {
      printf("Error verifying with DSA key 2: %s\n", error_to_string(err));
      exit(-1);
   }
   msg[0] ^= 1;
   printf("DSA Verification: %d, %d, %s\n", stat1, stat2, (stat1 == 1 && stat2 == 0) ? "passed" : "failed");
   if (!(stat1 == 1 && stat2 == 0)) exit(-1);

   /* test exporting it */
   x = sizeof(out2);
   if ((err = dsa_export(out2, &x, PK_PRIVATE, &key)) != CRYPT_OK) {
      printf("Error export PK_PRIVATE DSA key: %s\n", error_to_string(err));
      exit(-1);
   }
   printf("Exported PK_PRIVATE DSA key in %lu bytes\n", x);
   if ((err = dsa_import(out2, x, &key2)) != CRYPT_OK) {
      printf("Error importing PK_PRIVATE DSA key: %s\n", error_to_string(err));
      exit(-1);
   }
   /* verify a signature with it */
   if ((err = dsa_verify_hash(out, x, msg, sizeof(msg), &stat1, &key2)) != CRYPT_OK) {
      printf("Error verifying with DSA key 3: %s\n", error_to_string(err));
      exit(-1);
   }
   printf("PRIVATE Import Test: %s\n", stat1 == 1 ? "passed" : "failed");
   if (stat1 == 0) exit(-1);
   dsa_free(&key2);

   /* export as public now */
   x = sizeof(out2);
   if ((err = dsa_export(out2, &x, PK_PUBLIC, &key)) != CRYPT_OK) {
      printf("Error export PK_PUBLIC DSA key: %s\n", error_to_string(err));
      exit(-1);
   }
   printf("Exported PK_PUBLIC DSA key in %lu bytes\n", x);
   if ((err = dsa_import(out2, x, &key2)) != CRYPT_OK) {
      printf("Error importing PK_PUBLIC DSA key: %s\n", error_to_string(err));
      exit(-1);
   }
   /* verify a signature with it */
   if ((err = dsa_verify_hash(out, x, msg, sizeof(msg), &stat1, &key2)) != CRYPT_OK) {
      printf("Error verifying with DSA key 4: %s\n", error_to_string(err));
      exit(-1);
   }
   printf("PUBLIC Import Test: %s\n", stat1 == 1 ? "passed" : "failed");
   if (stat1 == 0) exit(-1);

   dsa_free(&key2);
   dsa_free(&key);
}

#ifdef PKCS_1
void pkcs1_test(void)
{
   unsigned char buf[3][128];
   int err, res1, res2, res3, prng_idx, hash_idx;
   unsigned long x, y, l1, l2, l3, i1, i2;

   /* get hash/prng  */
   hash_idx = find_hash("sha1");
   prng_idx = find_prng("yarrow");

   /* do many tests */
   for (x = 0; x < 10000; x++) {
      zeromem(buf, sizeof(buf));

      /* make a dummy message (of random length) */
      l3 = (rand() & 31) + 8;
      for (y = 0; y < l3; y++) buf[0][y] = rand() & 255;

      /* encode it */
      l1 = sizeof(buf[1]);
      if ((err = pkcs_1_oaep_encode(buf[0], l3, NULL, 0, 1024, hash_idx, prng_idx, &prng, buf[1], &l1)) != CRYPT_OK) {
         printf("OAEP encode: %s\n", error_to_string(err));
         exit(-1);
      }

      /* decode it */
      l2 = sizeof(buf[2]);
      if ((err = pkcs_1_oaep_decode(buf[1], l1, NULL, 0, 1024, hash_idx, buf[2], &l2, &res1)) != CRYPT_OK) {
         printf("OAEP decode: %s\n", error_to_string(err));
         exit(-1);
      }

      if (res1 != 1 || l2 != l3 || memcmp(buf[2], buf[0], l3) != 0) {
         printf("res == %d, Outsize == %lu, should have been %lu, msg contents follow.\n", res1, l2, l3);
         printf("ORIGINAL:\n");
         for (x = 0; x < l3; x++) {
             printf("%02x ", buf[0][x]);
         }
         printf("\nRESULT:\n");
         for (x = 0; x < l2; x++) {
             printf("%02x ", buf[2][x]);
         }
         printf("\n\n");
         exit(-1);
      }

      /* test PSS */
      l1 = sizeof(buf[1]);
      if ((err = pkcs_1_pss_encode(buf[0], l3, l3>>2, hash_idx, prng_idx, &prng, 1024, buf[1], &l1)) != CRYPT_OK) {
         printf("PSS encode: %s\n", error_to_string(err));
         exit(-1); 
      }
      
      if ((err = pkcs_1_pss_decode(buf[0], l3, buf[1], l1, l3>>2, hash_idx, 1024, &res1)) != CRYPT_OK) {
         printf("PSS decode1: %s\n", error_to_string(err));
         exit(-1); 
      }
      
      buf[0][i1 = abs(rand()) % l3] ^= 1;
      if ((err = pkcs_1_pss_decode(buf[0], l3, buf[1], l1, l3>>2, hash_idx, 1024, &res2)) != CRYPT_OK) {
         printf("PSS decode2: %s\n", error_to_string(err));
         exit(-1); 
      }

      buf[0][i1] ^= 1;
      buf[1][i2 = abs(rand()) % l1] ^= 1;
      if ((err = pkcs_1_pss_decode(buf[0], l3, buf[1], l1, l3>>2, hash_idx, 1024, &res3)) != CRYPT_OK) {
         printf("PSS decode3: %s\n", error_to_string(err));
         exit(-1); 
      }

      if (!(res1 == 1 && res2 == 0 && res3 == 0)) {
         printf("PSS failed: %d, %d, %d, %lu\n", res1, res2, res3, l3);
         exit(-1);
      }
   }
   printf("PKCS #1: Passed\n");
}

#endif /* PKCS_1 */

int
main (void)
{
#ifdef SONY_PS2
  TIMER_Init ();
#endif
  srand(time(NULL));

  register_all_algs ();
   
  if ((errnum = yarrow_start (&prng)) != CRYPT_OK) {
    printf ("yarrow_start: %s\n", error_to_string (errnum));
  }
  if ((errnum = yarrow_add_entropy ((unsigned char *)"hello", 5, &prng)) != CRYPT_OK) {
    printf ("yarrow_add_entropy: %s\n", error_to_string (errnum));
  }
  if ((errnum = yarrow_ready (&prng)) != CRYPT_OK) {
    printf ("yarrow_ready: %s\n", error_to_string (errnum));
  }

  printf (crypt_build_settings);
  test_errs ();

#ifdef HMAC
  printf ("HMAC: %s\n", hmac_test () == CRYPT_OK ? "passed" : "failed");
  if (hmac_test() != CRYPT_OK) exit(EXIT_FAILURE);
#endif

#ifdef OMAC
  printf ("OMAC: %s\n", omac_test () == CRYPT_OK ? "passed" : "failed");
  if (omac_test() != CRYPT_OK) exit(EXIT_FAILURE);
#endif

#ifdef PMAC
  printf ("PMAC: %s\n", pmac_test () == CRYPT_OK ? "passed" : "failed");
  if (pmac_test() != CRYPT_OK) exit(EXIT_FAILURE);
#endif

#ifdef EAX_MODE
  printf ("EAX : %s\n", eax_test () == CRYPT_OK ? "passed" : "failed");
  if (eax_test() != CRYPT_OK) exit(EXIT_FAILURE);
#endif

#ifdef OCB_MODE
  printf ("OCB : %s\n", ocb_test () == CRYPT_OK ? "passed" : "failed");
  if (ocb_test() != CRYPT_OK) exit(EXIT_FAILURE);
#endif

  store_tests ();
  cipher_tests ();
  hash_tests ();

#ifdef PKCS_1
  pkcs1_test();
#endif

  ecb_tests ();
  cbc_tests ();
  ctr_tests ();
  ofb_tests ();
  cfb_tests ();

  rng_tests ();
  test_prime();

  dsa_tests();
  rsa_test ();
  pad_test ();
  ecc_tests ();
  dh_tests ();

  base64_test ();

  time_ecb ();
  time_hash ();

#ifdef SONY_PS2
  TIMER_Shutdown ();
#endif

  return 0;
}
