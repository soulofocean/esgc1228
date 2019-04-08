#ifndef _MYMQ_H
#define _MYMQ_H
#include <sys/msg.h> 			//IPC_NOWAIT头文件
#define MQ_INFO_BUFF 1024		//MQ消息中INFO字段长度
#define MQ_SEND_BUFF sizeof(msg_struct)-sizeof(long) //ENQueue_MQ消息的大小
#define MQ_RSV_BUFF sizeof(msg_struct)				 //DeQueue_MQ消息的大小 必须比ENQueue_MQ的大否则报错
#define MQ_SEND_BUFF_SHORT sizeof(msg_short_struct) - sizeof(long)
#define MQ_RSV_BUFF_SHORT sizeof(msg_short_struct)
#define SOCKET_RSV_MQ_KEY 8888	//接收到的SOCKET消息缓存的MQKEY ftok(".",8) 
#define SOCKET_SEND_MQ_KEY 9999 //将要通过SOCKET发送出去的MSG的缓存MQKEY ftok(".",9) 
#define SOCKET_SEND_SHORT_MQ_KEY 6666 //将要通过SOCKET发送出去的SHORT_MSG的缓存MQKEY ftok(".",6) 
#define DISPATCH_MQ_KEY 7777	//消息分发给各子设备的MQKEY ftok(".",7) 
#define SOCKET_RSV_MSG_TYPE 1	//Socket接收到的数据放入MQ默认类型
#define SOCKET_SEND_MSG_TYPE 2	//Socket发送出去的数据放入MQ默认类型
#define DEV_INDEX_OFFSET 16			//预留16bit放设备序号，DEVTYPE<<16
#define DEV_OFFSET_OP 0xFFFF		//和上面的16bits对应
typedef enum _dev_msg_ack_enum{
	NO_ACK = 0,
	SHORT_ACK = 1,
	LONG_ACK = 2
}DEV_MSG_ACK_ENUM;
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
extern DEV_MSG_ACK_ENUM global_ack_type;
extern long global_msg_type;
int my_itoa(int intValue,char *outStr,int str_len);
int replace_string(char *result, char *source, const char* oldStr, char *destStr);
unsigned int GetMQMsgType(int dev_type,int dev_offset);
unsigned int GetDevType(unsigned int msg_type);
unsigned int GetDevCount(unsigned int msg_type);
int create_mq(unsigned int mqKey,int *mqid_out);
int Enqueue_MQ(unsigned int mqKey,msg_struct msgs,int msgssize, IPC_WAIT_ENUM enqueuetype);
int Enqueue_MQ_Short(unsigned int mqKey,msg_short_struct msgs,int msgsize,IPC_WAIT_ENUM enqueue_type);
int Dequeue_MQ(unsigned int mqKey,long msgType,msg_struct *msgbuff, int buffsize,IPC_WAIT_ENUM dequeuetype);
int Dequeue_MQ_Short(unsigned int mqKey,long msgType,msg_short_struct *msgbuff, int buffsize,IPC_WAIT_ENUM dequeue_type);
int Delete_MQ(unsigned int mqKey);
unsigned int GetDispatchMQKey(long msg_type);
int PutRsvMQ(msg_struct msgs);
int PutSendMQ(int code,const char* func_name,char * info);
int PutSendShortMQ(int status_code);
int PutDispatchMQ(int dev_type,int dev_index,char* info);
int PutDispatchNMQ(msg_struct msgs,int put_count);
int GetRsvMQ(msg_struct *msgbuff);
int GetSendMQ(msg_struct *msgbuff);
int GetSendShortMQ(msg_short_struct *msgbuff);
int GetDispatchMQ(long msgType,msg_struct *msgbuff);
int DelDispatchMQ(long msgType);
int DeleteAllMQ(int max_msg_id);
void DevMsgAck(int code,const char* func_name,char* msg);
#endif
