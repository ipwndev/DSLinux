/* DStrInternal.h */

#ifndef _DStrInternal_h_
#define _DStrInternal_h_ 1

#ifdef __cplusplus
extern "C"
{
#endif	/* __cplusplus */

typedef struct DStr {
	char *s;
	size_t len;
	size_t allocSize;
} DStr, *DStrPtr;


#ifdef __cplusplus
}
#endif	/* __cplusplus */

/* These checks should catch most cases where an
 * uninitialized or trashed DStr structure has
 * been passed in.  As a consequence, it limits
 * our strings to a maximum length of 16777215.
 *
 * The assumptions are that malloc never
 * returns an unaligned pointer, and that our
 * allocation sizes are always multiples of 16
 * bytes.
 */
#define IS_DSTR_CORRUPT(dst) \
	((((long) dst->s & 1) != 0) || ((dst->allocSize & 0xFF00000F) != 0) || ((dst->len & 0xFF000000) != 0))

#endif
