#ifndef EAPOL_SM_H
#define EAPOL_SM_H

/* IEEE Std 802.1X-2001, 8.5 */


typedef enum { FALSE = 0, TRUE = 1 } Boolean;
typedef enum { ForceUnauthorized, ForceAuthorized, Auto } PortTypes;
typedef enum { Unauthorized, Authorized } PortState;
typedef enum { EAPRequestIdentity } EAPMsgType;
typedef unsigned int Counter;


/* Authenticator PAE state machine */
struct eapol_auth_pae_sm {
	/* variables */
	Boolean eapLogoff;
	Boolean eapStart;
	PortTypes portMode;
	unsigned int reAuthCount;
	Boolean rxInitialRsp;

	/* constants */
	unsigned int quietPeriod; /* default 60; 0..65535 */
#define AUTH_PAE_DEFAULT_quietPeriod 60
	EAPMsgType initialEAPMsg; /* IEEE 802.1aa/D4 */
#define AUTH_PAE_DEFAULT_initialEAPMsg EAPRequestIdentity
	unsigned int reAuthMax; /* default 2 */
#define AUTH_PAE_DEFAULT_reAuthMax 2
	unsigned int txPeriod; /* default 30; 1..65535 */
#define AUTH_PAE_DEFAULT_txPeriod 30

	/* counters */
	Counter authEntersConnecting;
	Counter authEapLogoffsWhileConnecting;
	Counter authEntersAuthenticating;
	Counter authAuthSuccessesWhileAuthenticating;
	Counter authAuthTimeoutsWhileAuthenticating;
	Counter authAuthFailWhileAuthenticating;
	Counter authAuthReauthsWhileAuthenticating;
	Counter authAuthEapStartsWhileAuthenticating;
	Counter authAuthEapLogoffWhileAuthenticating;
	Counter authAuthReauthsWhileAuthenticated;
	Counter authAuthEapStartsWhileAuthenticated;
	Counter authAuthEapLogoffWhileAuthenticated;

	enum { AUTH_PAE_INITIALIZE, AUTH_PAE_DISCONNECTED, AUTH_PAE_CONNECTING,
	       AUTH_PAE_AUTHENTICATING, AUTH_PAE_AUTHENTICATED,
	       AUTH_PAE_ABORTING, AUTH_PAE_HELD, AUTH_PAE_FORCE_AUTH,
	       AUTH_PAE_FORCE_UNAUTH } state;
};


/* Backend Authentication state machine */
struct eapol_backend_auth_sm {
	/* variables */
	unsigned int reqCount;
	Boolean rxResp;
	Boolean aSuccess;
	Boolean aFail;
	Boolean aReq;
	u8 idFromServer;

	/* constants */
	unsigned int suppTimeout; /* default 30; 1..X */
#define BE_AUTH_DEFAULT_suppTimeout 30
	unsigned int serverTimeout; /* default 30; 1..X */
#define BE_AUTH_DEFAULT_serverTimeout 30
	unsigned int maxReq; /* default 2; 1..10 */
#define BE_AUTH_DEFAULT_maxReq 2

	/* counters */
	Counter backendResponses;
	Counter backendAccessChallenges;
	Counter backendOtherRequestsToSupplicant;
	Counter backendNonNakResponsesFromSupplicant;
	Counter backendAuthSuccesses;
	Counter backendAuthFails;

	enum { BE_AUTH_REQUEST, BE_AUTH_RESPONSE, BE_AUTH_SUCCESS,
	       BE_AUTH_FAIL, BE_AUTH_TIMEOUT, BE_AUTH_IDLE, BE_AUTH_INITIALIZE
	} state;
};


/* Reauthentication Timer state machine */
struct eapol_reauth_timer_sm {
	/* constants */
	unsigned int reAuthPeriod; /* default 3600 s */
#define REAUTH_TIMER_DEFAULT_reAuthPeriod 3600
	Boolean reAuthEnabled;
#define REAUTH_TIMER_DEFAULT_reAuthEnabled TRUE

	enum { REAUTH_TIMER_INITIALIZE, REAUTH_TIMER_REAUTHENTICATE } state;
};


/* Authenticator Key Transmit state machine */
struct eapol_auth_key_tx {
	enum { AUTH_KEY_TX_NO_KEY_TRANSMIT, AUTH_KEY_TX_KEY_TRANSMIT } state;
};


struct eapol_state_machine {
	/* timers */
	int aWhile;
	int quietWhile;
	int reAuthWhen;
	int txWhen;

	/* global variables */
	Boolean authAbort;
	Boolean authFail;
	Boolean authStart;
	Boolean authTimeout;
	Boolean authSuccess;
	u8 currentId;
	Boolean initialize;
	Boolean keyAvailable; /* 802.1aa; was in Auth Key Transmit sm in .1x */
	Boolean keyTxEnabled; /* 802.1aa; was in Auth Key Transmit sm in .1x
			       * stace machines do not change this */
	PortTypes portControl;
	Boolean portEnabled;
	PortState portStatus;
	Boolean portValid; /* 802.1aa */
	Boolean reAuthenticate;
	Boolean keyRun; /* 802.1aa-d6-1 */
	Boolean keyDone; /* 802.1aa-d6-1 */


	/* Port Timers state machine */
	/* 'Boolean tick' implicitly handled as registered timeout */

	struct eapol_auth_pae_sm auth_pae;
	struct eapol_backend_auth_sm be_auth;
	struct eapol_reauth_timer_sm reauth_timer;
	struct eapol_auth_key_tx auth_key_tx;

	u8 addr[ETH_ALEN]; /* Supplicant address */
#define EAPOL_SM_PREAUTH BIT(0)
	int flags; /* EAPOL_SM_* */

	int radius_identifier;
	/* TODO: check when the last messages can be released */
	struct radius_msg *last_recv_radius;
	u8 *last_eap_supp; /* last received EAP Response from Supplicant */
	size_t last_eap_supp_len;
	u8 *last_eap_radius; /* last received EAP Response from Authentication
			      * Server */
	size_t last_eap_radius_len;
	u8 *identity;
	size_t identity_len;
	u8 *radius_class;
	size_t radius_class_len;

	/* Keys for encrypting and signing EAPOL-Key frames */
	u8 *eapol_key_sign;
	size_t eapol_key_sign_len;
	u8 *eapol_key_crypt;
	size_t eapol_key_crypt_len;

	/* Somewhat nasty pointers to global hostapd and STA data to avoid
	 * passing these to every function */
	hostapd *hapd;
	struct sta_info *sta;
};


struct eapol_state_machine *eapol_sm_alloc(hostapd *hapd,
					   struct sta_info *sta);
void eapol_sm_free(struct eapol_state_machine *sm);
void eapol_sm_step(struct eapol_state_machine *sm);
void eapol_sm_initialize(struct eapol_state_machine *sm);
void eapol_sm_dump_state(FILE *f, const char *prefix,
			 struct eapol_state_machine *sm);

#endif /* EAPOL_SM_H */
