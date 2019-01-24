#ifndef _MYMQ_H
#define _MYMQ_H
typedef struct _msgQueenDataType{
	int offset;
	int devType;
}msgQueenDataType;

typedef struct _message_struct{
	long msgType;
	msgQueenDataType msgData;
}msg_struct;
int Enqueue_MQ(int mqKey,msg_struct msgs);
int Dequeue_MQ(int mqKey,long msgType, msg_struct *msgs);
#endif
