#include "config.h"
#if !defined(HAVE_SETUID) && !defined(HAVE_SETEUID) && !defined(HAVE_SETREUID)
#error Your OS does not support changing user IDs.
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdlib.h>
#include <ctype.h>
#define CONFIG "/etc/autologin.conf"
#define DEFAULT "/bin/sh"

#ifdef HAVE_PAM
#include <security/pam_appl.h>
#include <sys/wait.h>
static int PAM_conv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
	/* We use PAM to authenticate for pam_console only, we don't need
	 * its messages */
	*resp=NULL;
	return PAM_SUCCESS;
}
static struct pam_conv PAM_conversation = {
	&PAM_conv,
	NULL
};
#endif

char runthis[1024];

int main(int argc, char **argv)
{
	struct stat st;
	FILE *f;
	char *cfg;
	struct passwd *pw;
	int user_found=0;
	uid_t uid;
	gid_t gid;
	char *dir, *shell;
	char *user=NULL;
	char *cmd=NULL;
#ifdef HAVE_PAM
	pam_handle_t *pamh;
	pid_t child;
	int status;
#endif

	runthis[0]=0; runthis[1023]=0;
	if(getuid()) {
		puts("ERROR: This program needs to change user IDs; therefore, it must be run as root.");
		return 1;
	}
	if(stat(CONFIG, &st)) {
		perror("ERROR: Couldn't stat "CONFIG":");
		return 1;
	}
	if(st.st_mode & S_IWGRP) {
		puts("ERROR: "CONFIG" must not be group-writable!");
		return 1;
	}
	if(st.st_mode & S_IWOTH) {
		puts("ERROR: "CONFIG" must not be world-writable!");
		return 1;
	}
	if(st.st_uid || st.st_gid) {
		puts("ERROR: "CONFIG" must be owned by user root, group root!");
		return 1;
	}	
	f=fopen(CONFIG, "r");
	if(!f) {
		perror("ERROR: Couldn't open "CONFIG":");
		return 1;
	}
	cfg=(char *) malloc(st.st_size+1);
	while(!feof(f) && !ferror(f)) {
		if(fgets(cfg, st.st_size+1, f)) {
			/* Ignore comments... */
			if(cfg[0]=='#')
				continue;
			
			/* Remove whitespaces... */
			while(isspace(cfg[0]))
				strcpy(cfg, cfg+1);
			while(isspace(cfg[strlen(cfg)-1]))
				cfg[strlen(cfg)-1]=0;
			while(strstr(cfg, " ="))
				strcpy(strstr(cfg, " ="), strstr(cfg, " =")+1);
			while(strstr(cfg, "\t="))
				strcpy(strstr(cfg, "\t="), strstr(cfg, "\t=")+1);
			while(strstr(cfg, "= "))
				strcpy(strstr(cfg, "= ")+1, strstr(cfg, "= ")+2);
			while(strstr(cfg, "=\t"))
				strcpy(strstr(cfg, "=\t")+1, strstr(cfg,"=\t")+2);
			if(!strncasecmp(cfg, "AUTOLOGIN=", 10)) {
				if(strlen(cfg)<11 || (strncasecmp(cfg+10, "Y", 1) && strncasecmp(cfg+10, "1", 1))) {
					/*Fall back to /bin/login */
					execlp("login", "login", NULL);
					printf("ERROR: Couldn't exec /bin/login: $s\n", strerror(errno));
					return 0; /* Why did we get called??? */
				}
			} else if(!strncasecmp(cfg, "USER=", 5))
				user=strdup(cfg+5);
			else if(!strncasecmp(cfg, "EXEC=", 5))
				cmd=strdup(cfg+5);
		}
	}
	fclose(f);
	free(cfg);
	if(user==NULL) {
		puts("ERROR: Required variable USER= not found in "CONFIG".");
		if(cmd)
			free(cmd);
		return 1;
	}
	if(cmd==NULL) /* Try a reasonable default... */
		cmd=strdup(DEFAULT);

	snprintf(runthis, 1023, "%s", cmd);
	free(cmd);
	
	pw = getpwnam(user);
	if(pw) {
		user_found=1;
		uid=pw->pw_uid;
		gid=pw->pw_gid;
		dir=strdup(pw->pw_dir);
		shell=strdup(pw->pw_shell);
	} else {
		printf("ERROR: No such user %s!\n", user);
		return 1;
	}

#ifdef PARANOID
	if(pw->pw_uid==0 || pw->pw_gid==0) {
		free(dir); free(shell);
		puts("Autologin as root is not permitted!");
		return 1;
	}
#endif

	/* Take console ownership and satisfy PAM */
#ifdef HAVE_PAM
	pam_start("autologin", user, &PAM_conversation, &pamh);
	pam_acct_mgmt(pamh, PAM_SILENT);
	pam_set_item(pamh, PAM_USER, user);
	pam_set_item(pamh, PAM_TTY, "tty0");
	pam_set_item(pamh, PAM_RHOST, NULL);
	pam_set_item(pamh, PAM_RUSER, user);
	pam_open_session(pamh, PAM_SILENT);
	child=fork();
	if(child==0) {
#else
	chown("/dev/console", uid, gid);
	chown("/dev/tty", uid, gid);
#endif

#ifdef HAVE_SETREGID
	setregid(gid, gid);
#else /* !HAVE_SETREGID */
#ifdef HAVE_SETEGID
	setegid(gid);
#endif
#ifdef HAVE_SETGID
	setgid(gid);
#endif
#endif

#ifdef HAVE_SETREUID
	setreuid(uid, uid);
#else /* !HAVE_SETREUID */
#ifdef HAVE_SETEUID
	seteuid(uid);
#endif /* HAVE_SETEUID */
#ifdef HAVE_SETUID
	setuid(uid);
#endif /* HAVE_SETUID */
#endif /* !HAVE_SETREUID */

	setenv("HOME", dir, 1);
	setenv("SHELL", shell, 1);
	setenv("USER", user, 1);
	setenv("LOGNAME", user, 1);

	chdir(dir);
	free(user);
	user=NULL;
	
	/* this is neccessary for normal shell execute */
	execlp(runthis, "-sh", NULL);

	free(dir); free(shell);
	printf("ERROR: Couldn't exec %s: %s\n", runthis, strerror(errno));
	return 2;
#ifdef HAVE_PAM
	} else {
		waitpid(child, &status, 0);
		pam_close_session(pamh, 0);
		pam_end(pamh, PAM_SUCCESS);
		if(WIFEXITED(status))
			return 0;
		else
			return WEXITSTATUS(status);
	}
#endif
}
