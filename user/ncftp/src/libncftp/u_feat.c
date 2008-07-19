/* u_feat.c
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
StrToBoolOrInt(const char *s)
{
	int c;
	int result;

	for (;;) {
		c = *s++;
		if (c == '\0')
			return 0;
		if (!isspace(c))
			break;
	}

	if (isupper(c))
		c = tolower(c);
	result = 0;
	switch (c) {
		case 'f':			       /* false */
		case 'n':			       /* no */
			break;
		case 'o':			       /* test for "off" and "on" */
			c = (int) s[1];
			if (isupper(c))
				c = tolower(c);
			if (c == 'f')
				break;
			result = 1;
			break;
		case 't':			       /* true */
		case 'y':			       /* yes */
			result = 1;
			break;
		default:			       /* 1, 0, -1, other number? */
			result = atoi(s - 1);
	}
	return result;
}						       /* StrToBoolOrInt */




static const char *gConnInfoOptStrings[] = {
	"PASV",
	"SIZE",
	"MDTM",
	"MDTM_set",
	"REST",
	"NLST_a",
	"NLST_d",
	"FEAT",
	"MLSD",
	"MLST",
	"CLNT",
	"HELP_SITE",
	"SITE_UTIME",
	"STATfileParamWorks",
	"NLSTfileParamWorks",
	"require20",
	"allowProxyForPORT",
	"doNotGetStartCWD",
	NULL
};

typedef enum ConnInfoOptions {
	kOptPASV,
	kOptSIZE,
	kOptMDTM,
	kOptMDTM_set,
	kOptREST,
	kOptNLST_a,
	kOptNLST_d,
	kOptFEAT,
	kOptMLSD,
	kOptMLST,
	kOptCLNT,
	kOptHELP_SITE,
	kOptSITE_UTIME,
	kOptSTATfileParamWorks,
	kOptNLSTfileParamWorks,
	kOptRequire20,
	kOptAllowProxyForPORT,
	kOptDoNotGetStartCWD,
	kOptNumConnInfoOptions
} ConnInfoOptions;





void
FTPManualOverrideFeatures(const FTPCIPtr cip)
{
	char tokbuf[256];
	char *parse;
	char *context;
	char *opt;
	int intval;
	char *charval;
	const char **optlist;
	ConnInfoOptions optnum;
	
	/* Example:
	 *    your_ftp_prog -o "hasPASV=1,!HELP_SITE,require20=0" ...
	 * Have your program set cip->manualOverrideFeatures to that option string.
	 */
	 
	if ((cip->manualOverrideFeatures == NULL) || (cip->manualOverrideFeatures[0] == '\0'))
		return;
		
	STRNCPY(tokbuf, cip->manualOverrideFeatures);
	
	for (   parse = tokbuf, context = NULL;
		((opt = strtokc(parse, ",;\n\t\r", &context)) != NULL);
		parse = NULL)
	{
		intval = 1;
		charval = strchr(opt, '=');
		if (charval != NULL) {
			*charval++ = '\0';
			intval = StrToBoolOrInt(charval);
		} else if (*opt == '!') {
			opt++;
			intval = 0;
		}
		if (ISTRNEQ(opt, "has", 3))
			opt += 3;
		if (ISTRNEQ(opt, "use", 3))
			opt += 3;
		if (ISTRNEQ(opt, "have", 4))
			opt += 4;
		if (ISTRNEQ(opt, "no", 2)) {
			opt += 2;
			intval = 0;
		}
		for (optlist = gConnInfoOptStrings, optnum = kOptPASV; *optlist != NULL; optlist++, optnum++) {
			if (ISTREQ(opt, *optlist)) {
				switch (optnum) {
					case kOptPASV:
						cip->hasPASV = intval;
						break;
					case kOptSIZE:
						cip->hasSIZE = intval;
						break;
					case kOptMDTM:
						cip->hasMDTM = intval;
						break;
					case kOptMDTM_set:
						cip->hasMDTM_set = intval;
						break;
					case kOptREST:
						cip->hasREST = intval;
						break;
					case kOptNLST_a:
						cip->hasNLST_a = intval;
						break;
					case kOptNLST_d:
						cip->hasNLST_d = intval;
						break;
					case kOptFEAT:
						cip->hasFEAT = intval;
						break;
					case kOptMLSD:
						cip->hasMLSD = intval;
						break;
					case kOptMLST:
						cip->hasMLST = intval;
						break;
					case kOptCLNT:
						cip->hasCLNT = intval;
						break;
					case kOptHELP_SITE:
						cip->hasHELP_SITE = intval;
						break;
					case kOptSITE_UTIME:
						cip->hasSITE_UTIME = intval;
						break;
					case kOptSTATfileParamWorks:
						cip->STATfileParamWorks = intval;
						break;
					case kOptNLSTfileParamWorks:
						cip->NLSTfileParamWorks = intval;
						break;
					case kOptRequire20:
						cip->require20 = intval;
						break;
					case kOptAllowProxyForPORT:
						cip->allowProxyForPORT = intval;
						break;
					case kOptDoNotGetStartCWD:
						cip->doNotGetStartingWorkingDirectory = intval;
						break;
					case kOptNumConnInfoOptions:
						break;
				}
				break;	
			}
		}
	}
}
