/* Strn.h */

#ifndef _Strn_h_
#define _Strn_h_ 1

#ifdef __cplusplus
extern "C"
{
#endif	/* __cplusplus */

/* You could define this from the Makefile. */
#ifndef STRN_ZERO_PAD
#	define STRN_ZERO_PAD 1
#endif

/* You could define this from the Makefile. */
#ifndef STRNP_ZERO_PAD
#	define STRNP_ZERO_PAD 0
#endif

/* Strncat.c */
char *Strncat(char *const, const char *const, const size_t);

/* Strncpy.c */
char *Strncpy(char *const, const char *const, const size_t);

/* Strnpcat.c */
char *Strnpcat(char *const, const char *const, size_t);

/* Strnpcpy.c */
char *Strnpcpy(char *const, const char *const, size_t);

/* Strntok.c */
char *Strtok(char *, const char *);
int Strntok(char *, size_t, char *, const char *);

/* strtokc.c */
char *strtokc(char *, const char *, char **);
size_t strntokc(char *, size_t, char *, const char *, char **);

/* Dynscat.c */
char * Dynscat(char **dst, ...);

/* Dynscpy.c */
char * Dynscpy(char **dst, ...);

/* Dynsrecpy.c */
char * Dynsrecpy(char **dst, ...);

/* StrFree.c */
void StrFree(char **dst);

#ifndef _DStrInternal_h_
typedef struct DStr {
	/* All of these structure fields are read-only; do not modify
	 * them directly.
	 */
	char *s;
	size_t len;
	size_t allocSize;
} DStr, *DStrPtr;
#endif

void DStrInit(DStr *const dst);
void DStrFree(DStr *const dst);
void DStrClear(DStr *const dst);
int DStrNew(DStr *const dst, const size_t srcLen);
const char *DStrCpy(DStr *const dst, const char *const src);
const char *DStrCat(DStr *const dst, const char *const src);
const char *DStrCatList(DStr *const dst, ...);
const char *DStrCpyList(DStr *const dst, ...);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#define STRNCPY(d,s) Strncpy((d), (s), (size_t) sizeof(d))
#define STRNCAT(d,s) Strncat((d), (s), (size_t) sizeof(d))

#endif	/* _Strn_h_ */

/* eof Strn.h */
