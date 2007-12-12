/* pref.h
 *
 * Copyright (c) 1992-2004 by Mike Gleason.
 * All rights reserved.
 * 
 */

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define kFirewallPrefFileName			"firewall.txt"
#	define kGlobalFirewallPrefFileName		"..\\..\\firewall.txt"
#	define kGlobalFixedFirewallPrefFileName		"..\\..\\firewall_fixed.txt"
#	define kGlobalPrefFileName			"..\\..\\prefs_v3.txt"
#	define kGlobalFixedPrefFileName			"..\\..\\prefs_v3_fixed.txt"
#	define kPrefFileName				"prefs_v3.txt"
#	define kPrefFileNameV2				"prefs"
#	define kFirstFileName				"init_v3.txt"
#else
#	define kFirewallPrefFileName			"firewall"
#	define kGlobalFirewallPrefFileName		SYSCONFDIR "/ncftp.firewall"
#	define kGlobalFixedFirewallPrefFileName		SYSCONFDIR "/ncftp.firewall.fixed"
#	define kGlobalPrefFileName			SYSCONFDIR "/ncftp.prefs_v3"
#	define kGlobalFixedPrefFileName			SYSCONFDIR "/ncftp.prefs_v3.fixed"
#	define kPrefFileName				"prefs_v3"
#	define kPrefFileNameV2				"prefs"
#	define kFirstFileName				"init_v3"
#endif

#define kOpenSelectedBookmarkFileName		"bm2open"

typedef void (*PrefProc)(const char *const, FILE *const fp);
typedef struct PrefOpt {
	const char *varname;
	PrefProc proc;
	int visible;
} PrefOpt;

#define kPrefOptObselete (-1)
#define kPrefOptInvisible 0
#define kPrefOptVisible 1

#define PREFOBSELETE (PrefProc) 0, kPrefOptObselete,

/* pref.c */
void SetAnonPass(const char *const, FILE *const);
void SetAutoAscii(const char *const val, FILE *const fp);
void SetAutoResume(const char *const, FILE *const);
void SetAutoSaveChangesToExistingBookmarks(const char *const val, FILE *const fp);
void SetConfirmClose(const char *const, FILE *const);
void SetConnTimeout(const char *const, FILE *const);
void SetCtrlTimeout(const char *const, FILE *const);
void SetLogSize(const char *const val, FILE *const fp);
void SetNoAds(const char *const val, FILE *const fp);
void SetOneTimeMessages(const char *const val, FILE *const);
void SetPager(const char *const, FILE *const);
void SetPassive(const char *const, FILE *const);
#ifdef ncftp
void SetProgressMeter(const char *const, FILE *const);
#else
void SetProgressMeter(const char *const UNUSED(val), FILE *const UNUSED(fp));
#endif
void SetRedialDelay(const char *const val, FILE *const fp);
void SetSavePasswords(const char *const, FILE *const);
void SetSOBufsize(const char *const val, FILE *const fp);
void SetXferTimeout(const char *const, FILE *const);
void SetXtTitle(const char *const, FILE *const);
void Set(const char *const, const char *const);
void ProcessPrefsFile(FILE *const fp);
void LoadPrefs(void);
void InitPrefs(void);
void PostInitPrefs(void);
void SavePrefs(void);
void WriteDefaultFirewallPrefs(FILE *);
void ProcessFirewallPrefFile(FILE *);
void LoadFirewallPrefs(int);
void CheckForNewV3User(void);
int HasSeenOneTimeMessage(const char *const msg);
void SetSeenOneTimeMessage(const char *const msg);
int OneTimeMessage(const char *const msg);
