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

/* Future releases will make use of this */
#include "mycrypt.h"

static const char *err_2_str[] =
{
   "CRYPT_OK",
   "CRYPT_ERROR",
   "Non-fatal 'no-operation' requested.",

   "Invalid keysize for block cipher.",
   "Invalid number of rounds for block cipher.",
   "Algorithm failed test vectors.",

   "Buffer overflow.",
   "Invalid input packet.",

   "Invalid number of bits for a PRNG.",
   "Error reading the PRNG.",

   "Invalid cipher specified.",
   "Invalid hash specified.",
   "Invalid PRNG specified.",

   "Out of memory.",

   "Invalid PK key or key type specified for function.",
   "A private PK key is required.",

   "Invalid argument provided.",
   "File Not Found",

   "Invalid PK type.",
   "Invalid PK system.",
   "Duplicate PK key found on keyring.",
   "Key not found in keyring.",
   "Invalid sized parameter.",

   "Invalid size for prime.",

};

#ifdef MPI
static const struct {
    int mpi_code, ltc_code;
} mpi_to_ltc_codes[] = {
   { MP_OKAY ,  CRYPT_OK},
   { MP_MEM  ,  CRYPT_MEM},
   { MP_VAL  ,  CRYPT_INVALID_ARG},
};
#endif

const char *error_to_string(int err)
{
   if (err < 0 || err >= (int)(sizeof(err_2_str)/sizeof(err_2_str[0]))) {
      return "Invalid error code.";
   } else {
      return err_2_str[err];
   }   
}

#ifdef MPI
/* convert a MPI error to a LTC error (Possibly the most powerful function ever!  Oh wait... no) */
int mpi_to_ltc_error(int err)
{
   int x;

   for (x = 0; x < (int)(sizeof(mpi_to_ltc_codes)/sizeof(mpi_to_ltc_codes[0])); x++) {
       if (err == mpi_to_ltc_codes[x].mpi_code) { 
          return mpi_to_ltc_codes[x].ltc_code;
       }
   }
   return CRYPT_ERROR;
}
#endif

