#include<stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>   
#include <errno.h> 
#include<string.h>
#include "myMQ.h"
#include "egsc_util.h"

int Enqueue_MQ(int mqKey,msg_struct msgs,int msgsize,IPC_WAIT_ENUM enqueue_type)
{
	int ret;
	int msqid;
	msqid = msgget(mqKey,IPC_EXCL);  /*检查消息队列是否存在*/    
	if(msqid < 0){      
		msqid = msgget(mqKey,IPC_CREAT|0666);/*创建消息队列*/
		if(msqid <0){
			egsc_log_error("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));     
			return -1;
		}
	}
	ret = msgsnd(msqid,&msgs,msgsize,enqueue_type);
	if(ret < 0){
		egsc_log_error("msgsnd() write msg failed,ret=%d,errno=%d[%s]\n",ret,errno,strerror(errno));
	}
	egsc_log_debug("[%d]enqueue complete!\n",getpid());
	return ret;
}

int Dequeue_MQ(int mqKey,long msgType,msg_struct *msgbuff, int buffsize,IPC_WAIT_ENUM dequeue_type)
{
	int msgid;
	int ret = 0;
	msgid=msgget(mqKey,IPC_EXCL);
	if(msgid<0){
		egsc_log_error("get msgid fail!errno=%d[%s]",errno,strerror(errno));
		return -1;
	}
	ret = msgrcv(msgid,msgbuff,buffsize,msgType,dequeue_type);
	if(ret < 0){
		egsc_log_error("msgrcv() get msg failed,errno=%d[%s]\n",errno,strerror(errno));
	}
	else{
		egsc_log_debug("Function:devtype=[%d] offset=[%d]\n",msgbuff->msgData.devType,msgbuff->msgData.offset);
	}
	return ret;
}
int Delete_MQ(int mqKey)
{
	int msgid;
	int ret = 0;
	msgid=msgget(mqKey,IPC_EXCL);
	if(msgid<0){
		egsc_log_error("get msgid fail!errno=%d[%s]",errno,strerror(errno));
		return -1;
	}
	ret = msgctl(msgid,IPC_RMID,0);
	if(ret < 0){
		egsc_log_error("msgctl IPC_RMID failed,errno=%d[%s]\n",errno,strerror(errno));
	}
	else{
		egsc_log_debug("Delete Complete!\n");
	}
	return ret;
}
int PutRsvMQ(msg_struct msgs)
{
	return Enqueue_MQ(SOCKET_RSV_MQ_KEY, msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutSendMQ(msg_struct msgs)
{
	return Enqueue_MQ(SOCKET_SEND_MQ_KEY, msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutDispatchMQ(msg_struct msgs)
{
	return Enqueue_MQ(DISPATCH_MQ_KEY, msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutDispatchNMQ(msg_struct msgs,int put_count)
{
	int index = 0;
	int ret = 0;
	for(;index<put_count;++put_count){
		ret = Enqueue_MQ(DISPATCH_MQ_KEY, msgs, MQ_SEND_BUFF, ipc_no_wait);
		if(ret < 0)
			return ret;
		if(index<put_count){
			msgs.msgType++;
			msgs.msgData.offset++;
		}
	}
	return ret;
}

int GetRsvMQ(msg_struct *msgbuff)
{
	return Dequeue_MQ(SOCKET_RSV_MQ_KEY, 0, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}
int GetSendMQ(msg_struct *msgbuff)
{
	return Dequeue_MQ(SOCKET_SEND_MQ_KEY, 0, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}
int GetDispatchMQ(long msgType,msg_struct *msgbuff)
{
	return Dequeue_MQ(DISPATCH_MQ_KEY, msgType, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}


