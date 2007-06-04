/*
  BeOS Socket abstruction library "libsocket.a"
 
 Copyright (C) 1998,99 Kazuho Okui. ALL RIGHT RESERVED
  
 This file is part of libsock.a

 libsocket.a is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 $Id$
 $RCSfile$:
*/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <kernel/OS.h>
#include <errno.h>
#include <termios.h>
#include <sys/uio.h>
#include "libsocket.h"

#undef read
#undef write
#undef close

#undef socket
#undef bind
#undef getsockname
#undef getpeername

#undef connect
#undef accept
#undef listen
#undef closesocket
#undef socketpair

#undef recv
#undef send
#undef recvfrom
#undef recvmsg
#undef sendto
#undef sendmsg

#undef select

#define BE_MAX_DESC 256

static int BE_SOCK_HANDLE [BE_MAX_DESC]; 
static int BE_SOCK_REVERSE_HANDLE [BE_MAX_DESC];

static char READ_AVAIL [BE_MAX_DESC];
static char READ_AVAIL_FLAG [BE_MAX_DESC];

static int s_catched_signum = 0;

typedef struct 
{
  int nfds;
  struct fd_set rfds;
  struct fd_set wfds;
  struct fd_set efds;
  struct timeval *timeout;
} select_info;

typedef struct
{
  int numfds;
  int r_errno;
} reply_info;


int sock_initialized = 0;

#define SEND_FILE_DESCRIPTOR 'fils'
#define RECV_FILE_DESCRIPTOR 'filr'

/****************************************************************************
 *	libsocket signal handler
 ****************************************************************************/
void
__libsocket_sighandler (int signum)
{
  s_catched_signum = signum;
}
/****************************************************************************
 *	libsocket thread functions.
 ****************************************************************************/

static int32
file_select (void *data)
{
  int nread, fd, code;
  thread_id sender;
  reply_info reply;
  
  code = receive_data (&sender, (void *)&fd, sizeof(int));
  if (code != SEND_FILE_DESCRIPTOR) {
    exit_thread (B_ERROR);
    return B_ERROR;
  }
  
 again_file_read:
  nread = read (fd, &READ_AVAIL[fd], 1);
  if (nread <= 0) {
    switch (errno) {
    case EAGAIN:
    case EIO:
      snooze (10 * 1000);
      goto again_file_read;
      break;

    default:
      /*perror ("FILE ERROR");*/
      
      reply.numfds = -1;
      reply.r_errno = errno;
      send_data (sender, RECV_FILE_DESCRIPTOR,
		 (void *)&reply, sizeof (reply_info));
      exit_thread (B_ERROR);
      return B_ERROR;
    }
  }

  if (nread == 1) {
    /* set avail flag. */
    READ_AVAIL_FLAG[fd] = 1;
    reply.numfds = fd;
    reply.r_errno = 0;
    send_data (sender, RECV_FILE_DESCRIPTOR,
	       (void *)&reply, sizeof(reply_info));
    exit_thread (B_OK);
    return B_OK;
  } else {
    exit_thread (B_ERROR);
    return B_ERROR;
  }
  
}
/****************************************************************************
 *	libsocket hook functions.
 ****************************************************************************/

/***********************
 * socket_initialize
 **********************/
void
__init_socket_handle()
{
  int i;

  for (i = 0; i < BE_MAX_DESC; i++){
    BE_SOCK_HANDLE[i] = -1;
    BE_SOCK_REVERSE_HANDLE[i] = -1;
    READ_AVAIL_FLAG[i] = 0;
    READ_AVAIL[i] = 0;
  }
  sock_initialized = 1;
}


/***********************
 * read 
 **********************/
ssize_t
__libsocket_read (int fd, void *buf, size_t nbyte)
{
  char *ptr;
  int sts;
  
  struct termio tio;
  
  if ( BE_SOCK_HANDLE[fd] >= 0 && sock_initialized){
    sts = recv (BE_SOCK_HANDLE[fd], buf, nbyte, 0);
    return sts;
  } else {
    if (READ_AVAIL_FLAG[fd]) {
      ptr = buf;
      *ptr++ = READ_AVAIL[fd];
      READ_AVAIL_FLAG[fd] = 0;
      nbyte--;
      if (nbyte == 0)
	return 1;
      tcgetattr (fd, &tio);
      if  (((tio.c_lflag &= ICANON) == 0) && (tio.c_cc[VMIN] == 1))
	return 1;
      return read (fd, ptr, nbyte) + 1;
    } 
    return read (fd, buf,nbyte);
  }
}

/***********************
 * write
 **********************/
ssize_t
__libsocket_write (int fd, const void *buf, size_t nbyte)
{

  if (BE_SOCK_HANDLE[fd] >= 0 && sock_initialized) {
    return send (BE_SOCK_HANDLE[fd], buf, nbyte, 0);
  } else {
      return write (fd, buf,nbyte);
  }
}

int
__libsocket_close (int fd)
{
  int result;

  if ( BE_SOCK_HANDLE[fd] >= 0 && sock_initialized){
    result = closesocket( BE_SOCK_HANDLE[fd] );
    close(fd);
    BE_SOCK_REVERSE_HANDLE[ BE_SOCK_HANDLE[fd] ] = -1;
    BE_SOCK_HANDLE[fd] = -1;
    return result;
  }else{
    return close(fd);
  }

}

/***********************
 * socket
 **********************/
int
__libsocket_socket (int family, int type, int proto)
{

  int s,fd;

  s = socket(family, type, proto);

  if ( s < 0){
    return -1;
  }else{
    fd = open ("/dev/null",O_RDWR);
    BE_SOCK_HANDLE[fd] = s;
    BE_SOCK_REVERSE_HANDLE[s] = fd;
    return fd;
  }
}

/***********************
 * bind
 **********************/
int
__libsocket_bind (int s, const struct sockaddr *addr, int size)
{
  if (BE_SOCK_HANDLE[s] >= 0 && sock_initialized) {
    return bind (BE_SOCK_HANDLE[s], addr, size);
  } else {
    errno = EIO;
    return -1;
  }

}

/***********************
 * getsockname
 **********************/
int
__libsocket_getsockname (int s, struct sockaddr *addr, int *size)
{
  if (BE_SOCK_HANDLE[s] >= 0 && sock_initialized) {
    return getsockname (BE_SOCK_HANDLE[s], addr, size);
  } else {
    errno = EIO;
    return -1;
  }

}

/***********************
 * getpeername
 **********************/
int
__libsocket_getpeername (int s, struct sockaddr *addr, int *size)
{
  if (BE_SOCK_HANDLE[s] >= 0 && sock_initialized) {
    return getpeername (BE_SOCK_HANDLE[s], addr, size);
  } else {
    errno = EIO;
    return -1;
  }

}

/***********************
 * recv
 **********************/
ssize_t
__libsocket_recv (int fd, void *buf, size_t nbyte, int flag)
{
  if (BE_SOCK_HANDLE[fd] >= 0 && sock_initialized) {
    return recv (BE_SOCK_HANDLE[fd], buf, nbyte, flag);
  } else {
    errno = EIO;
    return -1;
  }
  
}

/***********************
 * send
 **********************/
ssize_t
__libsocket_send (int fd, const void *buf, size_t nbyte, int flag)
{

  if (BE_SOCK_HANDLE[fd] >= 0 && sock_initialized) {
    return send (BE_SOCK_HANDLE[fd], buf, nbyte, flag);
  } else {
    errno = EIO;
    return -1;
  }
}

/***********************
 * recvfrom
 **********************/
ssize_t
__libsocket_recvfrom (int fd, void *buf, size_t nbyte, int flag,
		    struct sockaddr *from, int *fromlen)
{
  if (BE_SOCK_HANDLE[fd] >= 0 && sock_initialized) {
    return recvfrom (BE_SOCK_HANDLE[fd], buf, nbyte, flag, from, fromlen);
  } else {
    errno = EIO;
    return -1;
  }
  
}

/***********************
 * recvmsg
 **********************/
ssize_t
__libsocket_recvmsg (int fd, struct msghdr *msg, int flag)
{
  if (BE_SOCK_HANDLE[fd] >= 0 && sock_initialized) {
    int i, nbyte=0, rbyte;
    char buf[1500];

    for(i = 0; i<msg->msg_iovlen; i++)
       nbyte += msg->msg_iov[i].iov_len;

    rbyte = recvfrom (BE_SOCK_HANDLE[fd], buf, nbyte, flag, msg->msg_name, &msg->msg_namelen);

    if (rbyte > 0)
       {
        nbyte = msg->msg_accrightslen;
        memcpy(msg->msg_accrights,buf,nbyte);
        for(i = 0; i<msg->msg_iovlen; i++)
           {
            if (nbyte + msg->msg_iov[i].iov_len > rbyte)
               {
                memcpy(msg->msg_iov[i].iov_base,buf+nbyte,rbyte - nbyte);
                return rbyte;
               }
            memcpy(msg->msg_iov[i].iov_base,buf+nbyte,msg->msg_iov[i].iov_len);
            nbyte += msg->msg_iov[i].iov_len;
           }
       }

    return rbyte;
  } else {
    errno = EIO;
    return -1;
  }
  
}

/***********************
 * sendto
 **********************/
ssize_t
__libsocket_sendto (int fd, const void *buf, size_t nbyte, int flag,
		  const struct sockaddr *to, int tolen)
{

  if (BE_SOCK_HANDLE[fd] >= 0 && sock_initialized) {
    return sendto (BE_SOCK_HANDLE[fd], buf, nbyte, flag, to, tolen);
  } else {
    errno = EIO;
    return -1;
  }
}

/***********************
 * sendmsg
 **********************/
ssize_t
__libsocket_sendmsg (int fd, const struct msghdr *msg, int flag)
{
  if (BE_SOCK_HANDLE[fd] >= 0 && sock_initialized) {
    // Acording to Stevens, copying is the only way to do this right.
    int i,nbyte;
    char buf[1500]; // 1500 is an MTU guess.. where do we get this number?

    nbyte = msg->msg_accrightslen;
    memcpy(buf,msg->msg_accrights,nbyte);
    for(i = 0; i<msg->msg_iovlen; i++)
       {
        if (nbyte + msg->msg_iov[i].iov_len > 1500)
           {
            errno = EMSGSIZE;
            return -1;
           }
        memcpy(buf+nbyte,msg->msg_iov[i].iov_base,msg->msg_iov[i].iov_len);
        nbyte += msg->msg_iov[i].iov_len;
       }
    return sendto (BE_SOCK_HANDLE[fd], buf, nbyte, flag, msg->msg_name, msg->msg_namelen);
  } else {
    errno = EIO;
    return -1;
  }

}

/***********************
 * connect
 **********************/
int
__libsocket_connect (int s, const struct sockaddr *addr, int size)
{
  if (BE_SOCK_HANDLE[s] >= 0 && sock_initialized) {
    return connect( BE_SOCK_HANDLE[s], addr, size);
  } else {
    errno = EIO;
    return -1;
  }

}

/***********************
 * accept
 **********************/
int
__libsocket_accept (int s, struct sockaddr *addr, int *size)
{
  if (BE_SOCK_HANDLE[s] >= 0 && sock_initialized) {
    return accept (BE_SOCK_HANDLE[s], addr, size);
  } else {
    errno = EIO;
    return -1;
  }

}

/***********************
 * listen
 **********************/
int
__libsocket_listen (int s, int backlog)
{
  if (BE_SOCK_HANDLE[s] >= 0 && sock_initialized) {
    return listen (BE_SOCK_HANDLE[s], backlog);
  } else {
    errno = EIO;
    return -1;
  }

}

/***********************
 * closesocket
 **********************/
int
__libsocket_closesocket (int fd)
{
  int result;
  
  if (BE_SOCK_HANDLE[fd] >= 0 && sock_initialized) {
    result = closesocket( BE_SOCK_HANDLE[fd] );
    close(fd);
    BE_SOCK_REVERSE_HANDLE[ BE_SOCK_HANDLE[fd] ] = -1;
    BE_SOCK_HANDLE[fd] = -1;
    return result;
  } else {
    errno = EIO;
    return -1;
  }

}

/***********************
 * closesocket
 **********************/
int __libsocket_socketpair( int d,int type,int protocol, int sv[2])
{
 int yes = 1, no = 0;
 struct sockaddr_in sa;

 if (d != AF_INET || type != SOCK_STREAM)
    {
     errno = EAFNOSUPPORT;
     return -1;
    }

 sv[0] = socket(d,type,protocol);
 sv[1] = socket(d,type,protocol);

 //  beos doesn't have this option if (setsockopt(SO_USELOOPBACK))
 if (setsockopt(sv[0],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) return -1; 
 if (setsockopt(sv[0],SOL_SOCKET,SO_REUSEPORT,&yes,sizeof(yes)) < 0) return -1;
 if (setsockopt(sv[1],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) return -1;
 if (setsockopt(sv[1],SOL_SOCKET,SO_REUSEPORT,&yes,sizeof(yes)) < 0) return -1;
 if (setsockopt(sv[1],SOL_SOCKET,SO_NONBLOCK,&yes,sizeof(yes)) < 0) return -1;

 sa.sin_family = AF_INET;
 sa.sin_addr.s_addr = htonl(INADDR_ANY);
 sa.sin_port = htons(9595);

 if (bind(sv[1],&sa,sizeof(struct sockaddr_in) < 0)) return -1;
 if (listen(sv[1],1) < 0) return -1;
 if ((sv[1] = accept(sv[1],0,0)) < 0) return -1;

 sa.sin_family = AF_INET;
 sa.sin_addr.s_addr = inet_addr("127.0.0.1");
 sa.sin_port = htons(9595);

 if (connect(sv[0],&sa,sizeof(struct sockaddr_in)) < 0) return -1;
 if (setsockopt(sv[1],SOL_SOCKET,SO_NONBLOCK,&no,sizeof(no)) < 0) return -1;

 return 0;
}

/***********************
 * select
 **********************/
int
__libsocket_select (int nfds,
		  struct fd_set *rfds,
		  struct fd_set *wfds,
		  struct fd_set *efds,
		  struct timeval *timeout)
{

  struct fd_set sock_rfds;	/* socket rfds */
  struct fd_set u_sock_rfds;	/* backup socket rfds */
  struct fd_set file_rfds;	/* file/process rfds */

  int sock_nfds = 0, u_sock_nfds, sock_numfds = 0;
  int file_nfds = 0;
  int fd, code;
  reply_info reply;
  
  long sts;
  int file_avail = 0;
  int select_errno = 0;

  bigtime_t start_time, timeout_time;

  thread_id file_read_thread[BE_MAX_DESC];
  thread_id sender;

  struct timeval tout;
  char buf[32];
  struct sigaction libsock_act;
  struct sigaction old_int_act, old_winch_act, old_chld_act;
  sigset_t select_sigmask, old_sigmask;
  
  /* If rfds is 0, this function are usleep */
  if (rfds == NULL) {
    if (timeout == NULL ) {
      errno = EINVAL;
      return -1;
    } else {
      snooze (timeout->tv_sec * 1000 * 1000 + timeout->tv_usec);
      return 0;
    }
  }

  /* Set timeout */
  start_time = real_time_clock_usecs ();
  timeout_time = timeout->tv_sec * 1000 * 1000 + timeout->tv_usec;

  /* Set signal handler mask */
  s_catched_signum = 0;
  sigemptyset (&libsock_act.sa_mask);
  sigaddset (&libsock_act.sa_mask, SIGINT);
  sigaddset (&libsock_act.sa_mask, SIGWINCH);
  sigaddset (&libsock_act.sa_mask, SIGCHLD);
  libsock_act.sa_handler = __libsocket_sighandler;
  libsock_act.sa_flags = 0;

  /* trap and backup signal */
  sigaction (SIGINT, &libsock_act, &old_int_act);
  sigaction (SIGWINCH, &libsock_act, &old_winch_act);
  sigaction (SIGCHLD, &libsock_act, &old_chld_act);

  /* Set select() signal mask */
  sigemptyset (&select_sigmask);
  sigaddset (&select_sigmask, SIGINT);
  sigaddset (&select_sigmask, SIGWINCH);
  sigaddset (&select_sigmask, SIGCHLD);
  
  /* Clear file/socket rfds */
  FD_ZERO (&file_rfds); 
  FD_ZERO (&sock_rfds); 
 
  /* Only rfds are checked. */
  if (wfds)
    FD_ZERO (wfds);
  if(efds)
    FD_ZERO (efds);

  /*
   * separate socket descripor and file/process descriptor.
   */
  for (fd = 0; fd < nfds; fd++){
    if (FD_ISSET (fd, rfds) ){
      if (BE_SOCK_HANDLE[fd] >= 0 ){
	/* socket descriptor */
	FD_SET (BE_SOCK_HANDLE[fd], &sock_rfds);
	sock_nfds++;
      }
      else {
	/* file/proces descriptor */
	FD_SET (fd, &file_rfds);
	file_nfds = fd + 1;
      }
    }
  }

  FD_ZERO (rfds);

  /*
   * Spawn file/process select thread.
   */
  if (file_nfds) {
    for (fd = 0; fd < file_nfds; fd++) {
      READ_AVAIL_FLAG[fd] = 0;
      if (FD_ISSET (fd, &file_rfds)) {

	/* Spawn thread to the number of check fd */
	sprintf (buf, "file select [%d]", fd);
	file_read_thread[fd] =
	  spawn_thread (file_select, buf, B_NORMAL_PRIORITY, 0);

	send_data (file_read_thread[fd], SEND_FILE_DESCRIPTOR,
		   (void *)&fd, sizeof(int));
	resume_thread (file_read_thread[fd]);
      }
    }
  }

  /* Select loop */
  do {
    if (s_catched_signum) goto select_error;
    
    if (sock_nfds) {
      tout.tv_sec = 0;
      tout.tv_usec = 50 * 1000;
      u_sock_nfds = sock_nfds;
      u_sock_rfds = sock_rfds;
      /* Masking select() call, Because BeOS select() call is catched signal,
	 still never reading/writing socket data */
      sigprocmask (SIG_SETMASK, &select_sigmask, &old_sigmask);
      sock_numfds = select (u_sock_nfds, &u_sock_rfds, 0, 0, &tout);
      sigprocmask (SIG_SETMASK, &old_sigmask, NULL);

      if (sock_numfds < 0) {
	select_errno = errno;
	goto select_error;
      } else if (sock_numfds > 0) {
	break;
      }
    } else {
      snooze (50 * 1000);
    }
    
  check_data:
    /* Check file/process descripter select routine */
    if (has_data (find_thread (NULL))) {
      code = receive_data (&sender, (void *)&reply, sizeof (reply_info));
      switch (code) {

      case B_INTERRUPTED:
	select_errno = EINTR;
	goto select_error;
	break;

      case RECV_FILE_DESCRIPTOR:
	if (reply.numfds < 0) {
	  select_errno = reply.r_errno;
	  goto select_error;
	} else {
	  file_avail++;
	  snooze (100);
	  goto check_data;
	}
	break;

      default:
	continue;
      }
    }
  } while (file_avail == 0 && real_time_clock_usecs () < start_time + timeout_time);

  if (s_catched_signum) goto select_error;

  /* Mask this routine */
  sigprocmask (SIG_SETMASK, &select_sigmask, &old_sigmask);
  
  if (file_avail) {
    for (fd = 0; fd < file_nfds; fd++) {
      if (READ_AVAIL_FLAG[fd]) {
	/* set file/process descriptor */
	FD_SET (fd, rfds);
	wait_for_thread (file_read_thread[fd], &sts);
      } else {
	if (FD_ISSET (fd, &file_rfds))
	    kill_thread (file_read_thread[fd]);
      }
    }
  } else {
    for (fd = 0; fd < file_nfds; fd++) {
      if (FD_ISSET (fd, &file_rfds))
	kill_thread (file_read_thread[fd]);
    }
  }

  if (sock_numfds) {
    for (fd = 0; fd < sock_nfds; fd++) {
      if (FD_ISSET (fd, &u_sock_rfds)) {
	FD_SET(BE_SOCK_REVERSE_HANDLE[fd], rfds);
      }
    }
  }

  sigaction (SIGINT, &old_int_act, NULL);
  sigaction (SIGWINCH, &old_winch_act, NULL);
  sigaction (SIGCHLD, &old_chld_act, NULL);
  sigprocmask (SIG_SETMASK, &old_sigmask, NULL);
  
  return file_avail + sock_numfds;

 select_error:

  for (fd = 0; fd < file_nfds; fd++) {
    if (FD_ISSET (fd, &file_rfds))
	kill_thread (file_read_thread[fd]);
  }

  if (select_errno) {
    errno = select_errno;
  }

  sigaction (SIGINT, &old_int_act, NULL);
  sigaction (SIGWINCH, &old_winch_act, NULL);
  sigaction (SIGCHLD, &old_chld_act, NULL);

  switch (s_catched_signum) {
  case SIGINT:
    kill (0, SIGINT);
    errno = EINTR;
    break;

  case SIGWINCH:
    kill (0, SIGWINCH);
    errno = EINTR;
    break;

  case SIGCHLD:
    kill (0, SIGCHLD);
    errno = EINTR;
    break;

  default:
    break;
  }

  return -1;
}
