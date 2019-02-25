#ifndef _MY_SOCKET_H
#define _MY_SOCKET_H
#include "myMQ.h"
#define SOCKET_RCV_BUFF MQ_INFO_BUFF - 1  
#define SOCKET_SERVER_PORT 8888
#define SOCKET_SERVER_LISNUM 2
#define SOCKET_SEND_SLEEP_SEC 1

int socketServerInit(unsigned int myport, unsigned int lisnum, char * serveraddr,int *socketID);
int socketServerLoopRsv(int sockfd);
int socketServerLoopSend();
int socketServerLoopSendShort();
#endif
