/* regexp.c
 * (c) 2005 Konstantin S. Kravtsov <k6@pisem.net>
 * Written on the base of code of pcre_subst designed by Bert Driehuis <driehuis@playbeing.org>
 * with some changes.
 * Used here PCRE library is a library of functions to support regular expressions whose syntax
 * and semantics are as close as possible to those of the Perl 5 language.
 * PCRE used in case that HAVE_PCRE is defined (-DHAVE_PCRE), libc regex (POSIX regex functions)
 * used otherwise.
 */

#include "cfg.h"

#ifdef JS

#include "links.h"
#include "struct.h"

#include <ctype.h>

#if defined(HAVE_PCRE) || defined(HAVE_REGEX)

#ifdef HAVE_PCRE
#include <pcre.h>
#else /* HAVE_PCRE */
#include <regex.h>
#endif /* HAVE_PCRE */

#define MAXCAPTURE	300	/*supposed to be a multiple of 3*/

#define REGEX_DEBUG
#undef REGEX_DEBUG
#ifdef REGEX_DEBUG
static void
dumpstr(const char *str, int len, int start, int end)
{
	int i;
	for (i = 0; i < strlen(str); i++) {
		if (i >= start && i < end)
			putchar(str[i]);
		else
			putchar('-');
	}
	putchar('\n');
}

static void
dumpmatch(const char *str, int len, const char *rep, int nmat, const int *ovec)
{
	int i;
	printf("%s	Input\n", str);
	printf("nmat=%d", nmat);
	for (i = 0; i < nmat * 2; i++)
		printf(" %d", ovec[i]);
	printf("\n");
	for (i = 0; i < nmat * 2; i += 2)
		dumpstr(str, len, ovec[i], ovec[i+1]);
	printf("\n");
}

#endif /* REGEX_DEBUG */

static int
findreplen(const char *rep, int nmat, const int *replen)
{
	int len = 0;
	int val;
	char *cp = (char *)rep;
	while(*cp) {
		if (*cp == '$' && isdigit(cp[1])) {
			val = strtoul(&cp[1], &cp, 10);
			if (val && val <= nmat)
				len += replen[val -1];
#ifdef REGEX_DEBUG
			else
				fprintf(stderr, "repl %d out of range\n", val);
#endif /* REGEX_DEBUG */
		} else {
			cp++;
			len++;
		}
	}
	return len;
}

static void
doreplace(char *out, const char *rep, int nmat, int *replen, const char **repstr)
{
	int val;
	char *cp = (char *)rep;
	while(*cp) {
		if (*cp == '$' && isdigit(cp[1])) {
			val = strtoul(&cp[1], &cp, 10);
			if (val && val <= nmat) {
				strncpy(out, repstr[val - 1], replen[val - 1]);
				out += replen[val -1];
			}
		} else {
			*out++ = *cp++;
		}
	}
}

char *cp, *ep;

static char *
edit(const char *str, int len, const char *rep, int nmat, const int *ovec, char *res)
{
	int i, slen, rlen;
	const int *mvec = ovec;
	int replen[MAXCAPTURE];
	const char *repstr[MAXCAPTURE];
	nmat--;
	ovec += 2;
	for (i = 0; i < nmat; i++) 
	{
		replen[i] = ovec[i * 2 + 1] - ovec[i * 2];
		repstr[i] = &str[ovec[i * 2]];
#ifdef REGEX_DEBUG
		printf(">>>%d %d %.*s\n", i, replen[i], replen[i], repstr[i]);
#endif /* REGEX_DEBUG */
	}
	slen = len;
	len -= mvec[1] - mvec[0];
	len += rlen = findreplen(rep, nmat, replen);
#ifdef REGEX_DEBUG
	printf("resulting length %d (srclen=%d)\n", len, slen);
#endif /* REGEX_DEBUG */
	if (mvec[0] > 0) 
	{
		strncpy(cp, str, (mvec[0] > ep - cp - 1)? ep - cp - 1 :mvec[0]);
		cp += mvec[0];
	}
	if(ep - cp < rlen)
		return NULL;
	doreplace(cp, rep, nmat, replen, repstr);
	cp += rlen;
	return res;
}

char * 
regexp_replace(char * from, char *to, char *text)
{
	int nmat, offset = 0, textlen;
	int ovec[MAXCAPTURE];
	char *res, *ret, *pom;
	const char *overfl = NULL;	/* warning, go away */
	int global, i;
#ifdef HAVE_PCRE
	const char *er_ptr;
	int erroffset;
#else
	regmatch_t pmat[MAXCAPTURE/3];
	regex_t ppat_data;
	regex_t *ppat;
#endif

	if( from == NULL || to == NULL || text == NULL)
	{
		if(text == NULL) return NULL;
		ret = (unsigned char *)js_mem_alloc(strlen(text)+1);
		strcpy(ret,text);
		return ret;
	}
	while(*from == ' ' || *from == '\t') from++;
#ifdef HAVE_PCRE
	pom = pcre_malloc(strlen(from)+1);
#else /* HAVE_PCRE */
	pom = mem_alloc(strlen(from)+1);
#endif /* HAVE_PCRE */
	if(*from != '/' || !from[1])
	{
		strcpy(pom, from);
		global = 0;
	}
	else
	{
		for( i = strlen(from)-1; i > 1 && (from[i] == ' ' || from[i] == '\t'); i--);
		if( from[i] == '/')
		{
			strncpy(pom, from+1, i-1);
			pom[i-1] = '\0';
			global = 0;
		}else if( i > 1 && from[i] == 'g' && from[i-1] == '/')
		{
			strncpy(pom, from+1, i-2);
			pom[i-2] = '\0';
			global = 1;
		}else
		{
			strncpy(pom, from, i+1);
			pom[i+1] = '\0';
			global = 0;
		}
	}
#ifdef REGEX_DEBUG
	printf("Search pattern is '%s', global = %d\n",pom,global);
#endif /* REGEX_DEBUG */
	
#ifdef HAVE_PCRE
	pcre *ppat = pcre_compile(pom, 0/*PCRE_ANCHORED*/, &er_ptr, &erroffset, NULL);
	pcre_free(pom);
#else /* HAVE_PCRE */
	ppat = &ppat_data;
	if (regcomp(ppat, pom, REG_EXTENDED)) ppat = NULL;
	mem_free(pom);
#endif /* HAVE_PCRE */
	if (ppat == NULL)
	{
		if(text == NULL) return NULL;
		ret = (unsigned char *)js_mem_alloc(strlen(text)+1);
		strcpy(ret,text);
		return ret;
	}
	textlen = strlen(text);
#ifdef HAVE_PCRE
	res = pcre_malloc(MAXCAPTURE+textlen);
#else /* HAVE_PCRE */
	res = mem_alloc(MAXCAPTURE+textlen);
#endif /* HAVE_PCRE */
	cp = res;
	ep = res+MAXCAPTURE+textlen;
	if(global)
	{
		do {
#ifdef HAVE_PCRE
			nmat = pcre_exec(ppat, NULL, text, textlen, offset, 0, ovec, sizeof(ovec)/sizeof(int));
#else /* HAVE_PCRE */
			if (regexec(ppat, text+offset, MAXCAPTURE/3, pmat, 0))
				nmat = 0;
			else
				for( nmat = 0; nmat < MAXCAPTURE/3; nmat++ )
					if((ovec[nmat<<1] = pmat[nmat].rm_so) == -1 ||
						(ovec[(nmat<<1)+1] = pmat[nmat].rm_eo) == -1) break;
#endif /* HAVE_PCRE */
#ifdef HAVE_PCRE
			for(i = 0; i < nmat*2; i++)
				ovec[i]-=offset;
#endif /* HAVE_PCRE */
#ifdef REGEX_DEBUG
			dumpmatch(text+offset, textlen-offset, to, nmat, ovec);
#endif /* REGEX_DEBUG */
			if(nmat > 0)
			{
				overfl = edit(text+offset, textlen - offset, to, nmat, ovec, res);
				offset += ovec[1];
			}
		} while (nmat >0 && overfl);
	}
	else
	{
#ifdef HAVE_PCRE
		nmat = pcre_exec(ppat, NULL, text, textlen, 0, 0, ovec, sizeof(ovec)/sizeof(int));
#else /* HAVE_PCRE */
		 if (regexec(ppat, text, MAXCAPTURE/3, pmat, 0))
			 nmat = 0;
		 else
			 for( nmat = 0; nmat < MAXCAPTURE/3; nmat++ )
				 if((ovec[nmat<<1] = pmat[nmat].rm_so) == -1 ||
					(ovec[(nmat<<1)+1] = pmat[nmat].rm_eo) == -1) break;
#endif /* HAVE_PCRE */

#ifdef REGEX_DEBUG
		dumpmatch(text+offset, textlen-offset, to, nmat, ovec);
#endif /* REGEX_DEBUG */
		if(nmat > 0)
		{
			overfl = edit(text+offset, textlen - offset, to, nmat, ovec, res);
			offset += ovec[1];
		}
	}
	
	if ( textlen >= offset && cp + textlen - offset < ep)
	{
		strncpy(cp, text+offset, textlen - offset);
		*(cp +textlen - offset) = '\0';
	}
	else
		*(ep-1) = '\0';
	ret = (unsigned char *)js_mem_alloc(strlen(res)+1);
	strcpy(ret,res);
#ifdef HAVE_PCRE
	pcre_free(res);
	pcre_free(ppat);
#else /* HAVE_PCRE */
	mem_free(res);
	regfree(ppat);
#endif /* HAVE_PCRE */
	return ret;
}

#else

char * 
regexp_replace(char * from, char *to, char *text)
{
	return stracpy1("Regular expressions not supported");
}

#endif

#endif
