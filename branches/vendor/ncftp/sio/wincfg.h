/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* #define alarm_time_t unsigned int */ /* leave undefined */
#define gethost_addrptr_t const char *
#define gethostname_size_t int
#define listen_backlog_t int
#define read_return_t int
#define read_size_t unsigned int
#define sockaddr_size_t int
#define sockopt_size_t int
#define SETSOCKOPT_ARG4 (char *)
#define GETSOCKOPT_ARG4 (char *)
#define write_return_t int
#define write_size_t unsigned int
#define tv_sec_t long
#define tv_usec_t long
#define recv_return_t int
#define recv_size_t unsigned int
#define send_return_t int
#define send_size_t unsigned int

/* #define CAN_USE_SYS_SELECT_H 1 */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* #undef HAVE_MSGHDR_ACCRIGHTS */

/* #define HAVE_MSGHDR_CONTROL 1 */

/* Define if you have sigsetjmp and siglongjmp. */
/* #define HAVE_SIGSETJMP 1 */

/* #undef HAVE_SOCKADDR_UN_SUN_LEN */

/* #define HAVE_STRUCT_CMSGDHR 1 */
  
/* Define to the type of arg1 for select(). */
#define SELECT_TYPE_ARG1 int

/* Define to the type of args 2, 3 and 4 for select(). */
#define SELECT_TYPE_ARG234 (fd_set FAR *)

/* Define to the type of arg5 for select(). */
#define SELECT_TYPE_ARG5 (const struct timeval FAR *)

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you have the sigaction function.  */
/* #define HAVE_SIGACTION 1 */

/* Define if you have the <sys/select.h> header file.  */
/* #define HAVE_SYS_SELECT_H 1 */

/* Define if you have the <sys/time.h> header file.  */
/* #undef HAVE_SYS_TIME_H */

/* Define if you have the <sys/un.h> header file.  */
/* #define HAVE_SYS_UN_H 1 */

/* Define if you have the <unistd.h> header file.  */
/* #define HAVE_UNISTD_H 1 */

/* Define if you have the 44bsd library (-l44bsd).  */
/* #undef HAVE_LIB44BSD */

/* Define if you have the gen library (-lgen).  */
/* #undef HAVE_LIBGEN */

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the resolv library (-lresolv).  */
/* #undef HAVE_LIBRESOLV */

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */
