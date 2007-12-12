/* usio.h */

#ifndef _usio_h_
#define _usio_h_ 1

#ifdef __cplusplus
extern "C"
{
#endif	/* __cplusplus */

#define kUNewFailed (-9)
#define kUBindFailed (-10)
#define kUListenFailed (-11)

/* For compatibility with Sio 6.1.5 and earlier */
#define UAcceptS UAccept

/* UAccept.c */
int UAccept(int, struct sockaddr_un *const, int *, int);

/* UBind.c */
int UBind(int, const char *const, const int, const int);
int UListen(int, int);

/* UConnect.c */
int UConnect(int, const struct sockaddr_un *const, int, int);

/* UConnectByName.c */
int UConnectByName(int, const char *const, const int);

/* UNew.c */
int MakeSockAddrUn(struct sockaddr_un *, const char *const);
int UNewStreamClient(void);
int UNewDatagramClient(void);
int UNewStreamServer(const char *const, const int, const int, int);
int UNewDatagramServer(const char *const, const int, const int);

/* URecvfrom.c */
int URecvfrom(int, char *const, size_t, int, struct sockaddr_un *const, int *, int);

/* USendto.c */
int USendto(int, const char *const, size_t, int, const struct sockaddr_un *const, int, int);

/* USendtoByName.c */
int USendtoByName(int, const char *const, size_t, int, const char *const, int);

#ifdef __cplusplus
}
#endif

#endif	/* _usio_h_ */
