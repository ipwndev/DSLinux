/* test CFB/OFB/CBC modes */
#include <tomcrypt_test.h>

int modes_test(void)
{
   unsigned char pt[64], ct[64], tmp[64], key[16], iv[16], iv2[16];
   int cipher_idx;
   symmetric_CBC cbc;
   symmetric_CFB cfb;
   symmetric_OFB ofb;
   symmetric_CTR ctr;
   unsigned long l;
   
   /* make a random pt, key and iv */
   yarrow_read(pt,  64, &yarrow_prng);
   yarrow_read(key, 16, &yarrow_prng);
   yarrow_read(iv,  16, &yarrow_prng);
   
   /* get idx of AES handy */
   cipher_idx = find_cipher("aes");
   if (cipher_idx == -1) {
      fprintf(stderr, "test requires AES");
      return 1;
   }
   
#ifdef CBC
   /* test CBC mode */
   /* encode the block */
   DO(cbc_start(cipher_idx, iv, key, 16, 0, &cbc));
   l = sizeof(iv2);
   DO(cbc_getiv(iv2, &l, &cbc));
   if (l != 16 || memcmp(iv2, iv, 16)) {
      fprintf(stderr, "cbc_getiv failed");
      return 1;
   }
   DO(cbc_encrypt(pt, ct, 64, &cbc));
   
   /* decode the block */
   DO(cbc_setiv(iv2, l, &cbc));
   zeromem(tmp, sizeof(tmp));
   DO(cbc_decrypt(ct, tmp, 64, &cbc));
   if (memcmp(tmp, pt, 64) != 0) {
      fprintf(stderr, "CBC failed");
      return 1;
   }
#endif

#ifdef CFB   
   /* test CFB mode */
   /* encode the block */
   DO(cfb_start(cipher_idx, iv, key, 16, 0, &cfb));
   l = sizeof(iv2);
   DO(cfb_getiv(iv2, &l, &cfb));
   /* note we don't memcmp iv2/iv since cfb_start processes the IV for the first block */
   if (l != 16) {
      fprintf(stderr, "cfb_getiv failed");
      return 1;
   }
   DO(cfb_encrypt(pt, ct, 64, &cfb));
   
   /* decode the block */
   DO(cfb_setiv(iv, l, &cfb));
   zeromem(tmp, sizeof(tmp));
   DO(cfb_decrypt(ct, tmp, 64, &cfb));
   if (memcmp(tmp, pt, 64) != 0) {
      fprintf(stderr, "CFB failed");
      return 1;
   }
#endif
   
#ifdef OFB
   /* test OFB mode */
   /* encode the block */
   DO(ofb_start(cipher_idx, iv, key, 16, 0, &ofb));
   l = sizeof(iv2);
   DO(ofb_getiv(iv2, &l, &ofb));
   if (l != 16 || memcmp(iv2, iv, 16)) {
      fprintf(stderr, "ofb_getiv failed");
      return 1;
   }
   DO(ofb_encrypt(pt, ct, 64, &ofb));
   
   /* decode the block */
   DO(ofb_setiv(iv2, l, &ofb));
   zeromem(tmp, sizeof(tmp));
   DO(ofb_decrypt(ct, tmp, 64, &ofb));
   if (memcmp(tmp, pt, 64) != 0) {
      fprintf(stderr, "OFB failed");
      return 1;
   }
#endif

#ifdef CTR   
   /* test CTR mode */
   /* encode the block */
   DO(ctr_start(cipher_idx, iv, key, 16, 0, CTR_COUNTER_LITTLE_ENDIAN, &ctr));
   l = sizeof(iv2);
   DO(ctr_getiv(iv2, &l, &ctr));
   if (l != 16 || memcmp(iv2, iv, 16)) {
      fprintf(stderr, "ctr_getiv failed");
      return 1;
   }
   DO(ctr_encrypt(pt, ct, 57, &ctr));
   
   /* decode the block */
   DO(ctr_setiv(iv2, l, &ctr));
   zeromem(tmp, sizeof(tmp));
   DO(ctr_decrypt(ct, tmp, 57, &ctr));
   if (memcmp(tmp, pt, 57) != 0) {
      fprintf(stderr, "CTR failed");
      return 1;
   }
#endif
         
   return 0;
}

/* $Source$ */
/* $Revision$ */
/* $Date$ */
