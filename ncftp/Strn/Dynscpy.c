#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Use Dynscpy when you want to manage free'ing the created pointer
 * yourself; Dynsrecpy will try to free an existing pointer if needed
 * before creating the new string, but you have to be sure to be sure
 * initialize the pointer to NULL the first time you use it..
 *
 * Example:
 *
 * {
 *     char *p;                            -* p contains garbage *-
 *     Dynscpy(&p, "foo", "bar", 0);       -* no need to initialize p *-
 *     free(p);                            -* must free p to avoid leak *-
 *     Dynscpy(&p, "hello", "world", 0);   -* p can now be reused *-
 *     free(p);                            -* must free p to avoid leak *-
 *     Dynscpy(&p, "test", "123", 0);      -* p can now be reused *-
 *     free(p);                            -* free p when finished *-
 * }
 *
 * {
 *     char *p;
 *     p = NULL;  -* Must init p to NULL, else free() will get garbage *-
 *     Dynsrecpy(&p, "foo", "bar", 0);     -* on this call to Dynsrecpy *-
 *     Dynsrecpy(&p, "hello", "world", 0); -* p will be freed *-
 *     Dynsrecpy(&p, "test", "123", 0);    -* p will be freed *-
 *     free(p);                            -* free p when finished *-
 * }
 */

/*VARARGS*/
char *
Dynscpy(char **dst, ...)
{
	va_list ap;
	const char *src;
	char *newdst, *dcp;
	size_t curLen, catLen, srcLen;

	if (dst == (char **) 0)
		return NULL;

	catLen = 0;
	va_start(ap, dst);
	src = va_arg(ap, char *);
	while (src != NULL) {
		catLen += strlen(src);
		src = va_arg(ap, char *);
	}
	va_end(ap);

	curLen = 0;

	newdst = malloc(curLen + catLen + 2);
	if (newdst == NULL) {
		*dst = NULL;
		return NULL;
	}

	dcp = newdst + curLen;
	va_start(ap, dst);
	src = va_arg(ap, char *);
	while (src != NULL) {
		srcLen = strlen(src);
		memcpy(dcp, src, srcLen);
		dcp += srcLen;
		src = va_arg(ap, char *);
	}
	va_end(ap);
	*dcp = '\0';

	*dst = newdst;
	return (newdst);
}	/* Dynscpy */
