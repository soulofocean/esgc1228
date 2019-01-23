#ifndef _SOCKET_SERVER_H
#define _SOCKET_SERVER_H
#define MAXBUF 1024  
#define SOCKET_SERVER_PORT 8888
#define SOCKET_SERVER_LISNUM 2
int socketServerStart(unsigned int myport, unsigned int lisnum, char * serveraddr);
#endif