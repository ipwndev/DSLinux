/*
 * crypt.h: header for crypt.c 
 *
 * Written By Michael Sandrof
 *
 * Copyright(c) 1990 
 *
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 *
 * @(#)$Id: encrypt.h,v 1.1.1.1 2003/04/11 01:09:07 dan Exp $
 */

#ifndef __CRYPT_H_
#define __CRYPT_H_

	char	*crypt_msg (char *, char *);
	char	*decrypt_msg (char *, char *);
	void	encrypt_cmd (char *, char *, char *, char *);
	char	*is_crypted (char *);
	void	BX_my_decrypt (char *, int, char *);
	void	BX_my_encrypt (char *, int, char *);

#define CRYPT_HEADER ""
#define CRYPT_HEADER_LEN 5

#endif /* __crypt_h_ */
