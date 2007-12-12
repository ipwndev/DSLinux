#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Based on Dynscpy, but not the same! */

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
 *
 * You can also do stuff like this:
 *
 *     p = NULL;
 *     Dynsrecpy(&p, "cruel", 0);
 *     Dynsrecpy(&p, "hello, ", p, " world!", 0);
 *     -* p now contains "hello, cruel world!" *-
 */


/*VARARGS*/
char *
Dynsrecpy(char **dst, ...)
{
	va_list ap;
	const char *src;
	char *newdst, *dcp;
	size_t catLen, srcLen;
	int recursive;
	if (dst == (char **) 0)
		return NULL;

	recursive = 0;
	catLen = 0;
	va_start(ap, dst);
	src = va_arg(ap, char *);
	while (src != NULL) {
		if (src == *dst) {
			recursive = 1;
		}
		catLen += strlen(src);
		src = va_arg(ap, char *);
	}
	va_end(ap);

	if (recursive == 0) {
		if (*dst == NULL) {
			newdst = malloc(catLen + 2);
		} else if ((catLen + 2) > (strlen(*dst) + 1)) {
			newdst = realloc(*dst, catLen + 2);
		} else {
			/* Don't need to make it bigger */
			newdst = *dst;
		}
		if (newdst == NULL)
			return NULL;

		dcp = newdst;
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
	} else {
		newdst = malloc(catLen + 2);
		if (newdst == NULL)
			return NULL;

		dcp = newdst;
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

		if (*dst != NULL)
			free(*dst);
	}
	*dst = newdst;
	return (newdst);
}	/* Dynsrecpy */
