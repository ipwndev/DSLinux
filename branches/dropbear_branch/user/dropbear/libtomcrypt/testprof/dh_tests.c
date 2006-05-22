#include <tomcrypt_test.h>

#ifdef MDH

int dh_tests (void)
{
  unsigned char buf[3][4096];
  unsigned long x, y, z;
  int           stat, stat2;
  dh_key        usera, userb;

  DO(dh_test());

  /* make up two keys */
  DO(dh_make_key (&yarrow_prng, find_prng ("yarrow"), 512, &usera));
  DO(dh_make_key (&yarrow_prng, find_prng ("yarrow"), 512, &userb));

  /* make the shared secret */
  x = 4096;
  DO(dh_shared_secret (&usera, &userb, buf[0], &x));

  y = 4096;
  DO(dh_shared_secret (&userb, &usera, buf[1], &y));
  if (y != x) {
    fprintf(stderr, "DH Shared keys are not same size.\n");
    return 1;
  }
  if (memcmp (buf[0], buf[1], x)) {
    fprintf(stderr, "DH Shared keys not same contents.\n");
    return 1;
  }

  /* now export userb */
  y = 4096;
  DO(dh_export (buf[1], &y, PK_PUBLIC, &userb));
	  dh_free (&userb);

  /* import and make the shared secret again */
  DO(dh_import (buf[1], y, &userb));
  z = 4096;
  DO(dh_shared_secret (&usera, &userb, buf[2], &z));

  if (z != x) {
    fprintf(stderr, "failed.  Size don't match?\n");
    return 1;
  }
  if (memcmp (buf[0], buf[2], x)) {
    fprintf(stderr, "Failed.  Content didn't match.\n");
    return 1;
  }
  dh_free (&usera);
  dh_free (&userb);

/* test encrypt_key */
  dh_make_key (&yarrow_prng, find_prng ("yarrow"), 512, &usera);
  for (x = 0; x < 16; x++) {
    buf[0][x] = x;
  }
  y = sizeof (buf[1]);
  DO(dh_encrypt_key (buf[0], 16, buf[1], &y, &yarrow_prng, find_prng ("yarrow"), find_hash ("md5"), &usera));
  zeromem (buf[0], sizeof (buf[0]));
  x = sizeof (buf[0]);
  DO(dh_decrypt_key (buf[1], y, buf[0], &x, &usera));
  if (x != 16) {
    fprintf(stderr, "Failed (length)\n");
    return 1;
  }
  for (x = 0; x < 16; x++)
    if (buf[0][x] != x) {
      fprintf(stderr, "Failed (contents)\n");
      return 1;
    }

/* test sign_hash */
  for (x = 0; x < 16; x++) {
     buf[0][x] = x;
  }
  x = sizeof (buf[1]);
  DO(dh_sign_hash (buf[0], 16, buf[1], &x, &yarrow_prng		, find_prng ("yarrow"), &usera));
  DO(dh_verify_hash (buf[1], x, buf[0], 16, &stat, &usera));
  buf[0][0] ^= 1;
  DO(dh_verify_hash (buf[1], x, buf[0], 16, &stat2, &usera));
  if (!(stat == 1 && stat2 == 0)) { 
     fprintf(stderr, "dh_sign/verify_hash %d %d", stat, stat2);
     return 1;
  }
  dh_free (&usera);
  return 0;
}

#else

int dh_tests(void)
{
   fprintf(stderr, "NOP");
   return 0;
}

#endif

/* $Source$ */
/* $Revision$ */
/* $Date$ */
