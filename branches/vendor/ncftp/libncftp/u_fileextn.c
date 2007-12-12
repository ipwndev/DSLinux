/* u_fileextn.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
FilenameExtensionIndicatesASCII(const char *const pathName, const char *const extnList)
{
	const char *extn;
	char *cp;
	int c;
	char extnPattern[16];

	if ((pathName == NULL) || (pathName[0] == '\0'))
		return (0);

	extn = pathName + strlen(pathName) - 1;
	forever {
		if (extn <= pathName)
			return (0);	/* End of pathname, no extension. */
		c = (int) *--extn;
		if (IsLocalPathDelim(c))
			return (0);	/* End of filename, no extension. */
		if (c == '.') {
			extn += 1;
			break;
		}
	}
	if (strlen(extn) > (sizeof(extnPattern) - 2 - 1 - 1)) {
		return (0);
	}
#ifdef HAVE_SNPRINTF
	snprintf(extnPattern, sizeof(extnPattern),
#else
	sprintf(extnPattern,
#endif
		"|.%s|",
		extn
	);

	cp = extnPattern;
	forever {
		c = *cp;
		if (c == '\0')
			break;
		if (isupper(c)) {
			c = tolower(c);
			*cp++ = (char) c;
		} else {
			cp++;
		}
	}

	/* Extension list is specially formatted, like this:
	 *
	 * 	|ext1|ext2|ext3|...|extN|
	 *
	 * I.e, each filename extension is delimited with 
	 * a pipe, and we always begin and end the string
	 * with a pipe.
	 */
	if (strstr(extnList, extnPattern) != NULL) {
		return (1);
	}
	return (0);
}	/* FilenameExtensionIndicatesASCII */
