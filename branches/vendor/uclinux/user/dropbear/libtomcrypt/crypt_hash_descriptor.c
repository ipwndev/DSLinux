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

struct _hash_descriptor hash_descriptor[TAB_SIZE] = {
{ NULL, 0, 0, 0, { 0x00 }, 0, NULL, NULL, NULL, NULL },
{ NULL, 0, 0, 0, { 0x00 }, 0, NULL, NULL, NULL, NULL },
{ NULL, 0, 0, 0, { 0x00 }, 0, NULL, NULL, NULL, NULL },
{ NULL, 0, 0, 0, { 0x00 }, 0, NULL, NULL, NULL, NULL } };
