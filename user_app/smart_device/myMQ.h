#ifndef _MYMQ_H
#define _MYMQ_H
#include <sys/msg.h> 			//IPC_NOWAIT头文件
#define MQ_INFO_BUFF 1024		//MQ消息中INFO字段长度
typedef struct _msgQueenDataType{
	int offset;
	int devType;
	char info[MQ_INFO_BUFF];
}msgQueenDataType;
typedef struct _message_struct{
	long msgType;
	msgQueenDataType msgData;
}msg_struct;
typedef struct _msgQShortDataType{
	int statusCode;
}msgQShortDataType;
typedef struct _msg_short_struct{
	long msgType;
	msgQShortDataType msgData;
}msg_short_struct;
typedef enum _ipc_wait_enum{
	ipc_need_wait = 0,
	ipc_no_wait = IPC_NOWAIT
}IPC_WAIT_ENUM;
unsigned int GetDispatchMQKey(long msg_type);
int create_mq(unsigned int mqKey,int *mqid_out);
int Enqueue_MQ(unsigned int mqKey,msg_struct msgs,int msgssize, IPC_WAIT_ENUM enqueuetype);
int Enqueue_MQ_Short(unsigned int mqKey,msg_short_struct msgs,int msgsize,IPC_WAIT_ENUM enqueue_type);
int Dequeue_MQ(unsigned int mqKey,long msgType,msg_struct *msgbuff, int buffsize,IPC_WAIT_ENUM dequeuetype);
int Dequeue_MQ_Short(unsigned int mqKey,long msgType,msg_short_struct *msgbuff, int buffsize,IPC_WAIT_ENUM dequeue_type);
int Delete_MQ(unsigned int mqKey);
int DeleteAllMQ(int max_msg_id);
#endif
