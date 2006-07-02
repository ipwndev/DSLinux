/*
 * history.h: header for history.c 
 *
 * Copyright 1990 Michael Sandrof
 * Copyright 1997 EPIC Software Labs
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 *
 * @(#)$Id: history.h,v 1.1.1.1 2003/04/11 01:09:07 dan Exp $
 */

#ifndef __history_h__
#define __history_h__

	BUILT_IN_COMMAND(history);
	void	set_history_size	(Window *, char *, int);
	void	add_to_history 		(char *);
	char	*get_from_history (int);
	void	get_history 		(int);
	char	*do_history 		(char *, char *);
	void	shove_to_history 	(char, char *);

/* used by get_history */
#define NEXT 0
#define PREV 1

#endif /* _HISTORY_H_ */
