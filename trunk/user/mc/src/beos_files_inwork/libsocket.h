/*
  BeOS Socket abstruction library "libsocket.a"
 
 Copyright (C) 1998,99 Kazuho Okui. ALL RIGHT RESERVED
  
 This file is part of libsocket.a

 libsocket.a is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 $Id$
 $RCSfile$: include header
*/

#ifndef __LIBSOCKET_H__
#define __LIBSOCKET_H__

#include <sys/types.h>
#include <socket.h>

/* structs */
struct msghdr {
	caddr_t msg_name;
	int	msg_namelen;
	struct	iovec *msg_iov;
	int	msg_iovlen;
	caddr_t	msg_accrights;
	int	msg_accrightslen;
};

/* hook functions */
#define read		__libsocket_read
#define write		__libsocket_write
#define close		__libsocket_close

#define socket		__libsocket_socket
#define bind		__libsocket_bind
#define getsockname	__libsocket_getsockname
#define getpeername	__libsocket_getpeername

#define connect		__libsocket_connect
#define accept		__libsocket_accept
#define listen		__libsocket_listen
#define closesocket	__libsocket_closesocket
#define socketpair	__libsocket_socketpair

#define recv		__libsocket_recv
#define send		__libsocket_send
#define recvfrom	__libsocket_recvfrom
#define recvmsg		__libsocket_recvmsg
#define sendto		__libsocket_sendto
#define sendmsg		__libsocket_sendmsg

#define select		__libsocket_select

/* ANSI prototype */

ssize_t	__libsocket_read (int, void *, size_t);
ssize_t	__libsocket_write (int, const void *, size_t);
int	__libsocket_close (int);

int	__libsocket_socket (int, int, int);
int	__libsocket_bind (int, const struct sockaddr *, int);
int	__libsocket_getsockname (int, struct sockaddr *, int *);
int	__libsocket_getpeername (int, struct sockaddr *, int *);


ssize_t	__libsocket_recv (int, void *, size_t, int);
ssize_t	__libsocket_send (int, const void *, size_t, int);
ssize_t	__libsocket_recvfrom (int, void *, size_t, int,
			      struct sockaddr *, int *);
ssize_t	__libsocket_recvmsg (int, struct msghdr *, int);
ssize_t	__libsocket_sendto (int, const void *, size_t, int,
			    const struct sockaddr *, int);

ssize_t	__libsocket_sendmsg (int, const struct msghdr *, int);

int	__libsocket_connect (int, const struct sockaddr *, int);
int	__libsocket_accept (int, struct sockaddr *, int *);
int	__libsocket_listen (int, int);
int	__libsocket_closesocket (int);
int	__libsocket_socketpair (int, int, int, int sv[2]);

int	__libsocket_select (int,
			  struct fd_set *,
			  struct fd_set *,
			  struct fd_set *,
			  struct timeval *);

void	__init_socket_handle (void);

#endif /* __LIBSOCKET_H__ */
