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

#define SELECT_TYPE_ARG1 int

/* Define to the type of args 2, 3 and 4 for select(). */
#define SELECT_TYPE_ARG234 (fd_set *)

/* Define to the type of arg5 for select(). */
#define SELECT_TYPE_ARG5 (struct timeval *)

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the mktime function.  */
#define HAVE_MKTIME 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* #define HAVE_STRCASECMP 1 */

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the <unistd.h> header file.  */
/* #define HAVE_UNISTD_H 1 */

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

#define HAVE_LONG_LONG 1
#define SCANF_LONG_LONG "%I64d"
#define PRINTF_LONG_LONG "%I64d"
#define PRINTF_LONG_LONG_I64D 1
#define SCANF_LONG_LONG_I64D 1
