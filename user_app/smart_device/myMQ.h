#ifndef _MYMQ_H
#define _MYMQ_H
#include <sys/msg.h> 			//IPC_NOWAIT头文件
#define MQ_INFO_BUFF 1024		//MQ消息中INFO字段长度
#define MQ_SEND_BUFF sizeof(msg_struct)-sizeof(long) //ENQueue_MQ消息的大小
#define MQ_RSV_BUFF sizeof(msg_struct)				 //DeQueue_MQ消息的大小 必须比ENQueue_MQ的大否则报错
#define SOCKET_RSV_MQ_KEY 8888	//接收到的SOCKET消息缓存的MQKEY ftok(".",8) 
#define SOCKET_SEND_MQ_KEY 9999 //将要通过SOCKET发送出去的MSG的缓存MQKEY ftok(".",9) 
#define DISPATCH_MQ_KEY 7777	//消息分发给各子设备的MQKEY ftok(".",7) 
#define SOCKET_RSV_MSG_TYPE 1	//Socket接收到的数据放入MQ默认类型
#define SOCKET_SEND_MSG_TYPE 2	//Socket发送出去的数据放入MQ默认类型
typedef struct _msgQueenDataType{
	int offset;
	int devType;
	char info[MQ_INFO_BUFF];
}msgQueenDataType;

typedef struct _message_struct{
	long msgType;
	msgQueenDataType msgData;
}msg_struct;
typedef enum _ipc_wait_enum{
	ipc_need_wait = 0,
	ipc_no_wait = IPC_NOWAIT
}IPC_WAIT_ENUM;
int Enqueue_MQ(unsigned int mqKey,msg_struct msgs,int msgssize, IPC_WAIT_ENUM enqueuetype);
int Dequeue_MQ(unsigned int mqKey,long msgType,msg_struct *msgbuff, int buffsize,IPC_WAIT_ENUM dequeuetype);
int Delete_MQ(unsigned int mqKey);
unsigned int GetDispatchMQKey(long msg_type);
int PutRsvMQ(msg_struct msgs);
int PutSendMQ(msg_struct msgs);
int PutDispatchMQ(msg_struct msgs);
int PutDispatchNMQ(msg_struct msgs,int put_count);
int GetRsvMQ(msg_struct *msgbuff);
int GetSendMQ(msg_struct *msgbuff);
int GetDispatchMQ(long msgType,msg_struct *msgbuff);
int DelDispatchMQ(long msgType);
#endif
