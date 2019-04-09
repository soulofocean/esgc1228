#include<stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>   
#include <errno.h> 
#include<string.h>
#include "myMQ.h"
#include "egsc_util.h"
unsigned int GetDispatchMQKey(long msg_type)
{
	return msg_type & 0xFFFFFFFF;
}
int create_mq(unsigned int mqKey,int *mqid_out)
{
	int ret = 0;
	int msqid = msgget(mqKey,IPC_EXCL);  /*检查消息队列是否存在*/    
	if(msqid < 0){      
		msqid = msgget(mqKey,IPC_CREAT|0666);/*创建消息队列*/
		if(msqid <0){
			egsc_log_error("failed to create mq key=[%u] | errno=%d [%s]\n",mqKey,errno,strerror(errno));     
			ret = -1;
		}
	}
	*mqid_out = msqid;
	egsc_log_debug("create_mq mqkey[%u] msqid[%d] ret=%d\n",mqKey,msqid,ret);
	return ret;
}
int Enqueue_MQ(unsigned int mqKey,msg_struct msgs,int msgsize,IPC_WAIT_ENUM enqueue_type)
{
	int ret;
	int msqid;
	ret = create_mq(mqKey,&msqid);
	if(ret < 0)
		return ret;
	ret = msgsnd(msqid,&msgs,msgsize,enqueue_type);
	if(ret < 0){
		egsc_log_error("msgsnd() write msg failed,ret=%d,errno=%d[%s]\n",ret,errno,strerror(errno));
	}
	egsc_log_info("[%d]enqueue MQ[key=%u]complete! info=[%s]\n",getpid(),mqKey,msgs.msgData.info);
	return ret;
}
int Enqueue_MQ_Short(unsigned int mqKey,msg_short_struct msgs,int msgsize,IPC_WAIT_ENUM enqueue_type)
{
	int ret;
	int msqid;
	ret = create_mq(mqKey,&msqid);
	if(ret < 0)
		return ret;
	ret = msgsnd(msqid,&msgs,msgsize,enqueue_type);
	if(ret < 0){
		egsc_log_error("msgsnd() write msg failed,ret=%d,errno=%d[%s]\n",ret,errno,strerror(errno));
	}
	egsc_log_info("[%d]enqueue MQ[key=%u]complete! status code=[%d]\n",getpid(),mqKey,msgs.msgData.statusCode);
	return ret;
}

int Dequeue_MQ(unsigned int mqKey,long msgType,msg_struct *msgbuff, int buffsize,IPC_WAIT_ENUM dequeue_type)
{
	int msgid;
	int ret = 0;
	msgid=msgget(mqKey,IPC_EXCL);
	if(msgid<0){
		egsc_log_debug("get msgid fail! mqkey = %d errno=%d[%s]\n",mqKey,errno,strerror(errno));
		return -1;
	}
	ret = msgrcv(msgid,msgbuff,buffsize,msgType,dequeue_type);
	if(ret < 0){
		egsc_log_error("msgrcv() get msg failed,errno=%d[%s]\n",errno,strerror(errno));
	}
	else{
		egsc_log_info("[dequeue key=%u ret=%d]:devtype=[%d] offset=[%d] info=[%s]\n",mqKey,ret,msgbuff->msgData.devType,msgbuff->msgData.offset,msgbuff->msgData.info);
	}
	return ret;
}
int Dequeue_MQ_Short(unsigned int mqKey,long msgType,msg_short_struct *msgbuff, int buffsize,IPC_WAIT_ENUM dequeue_type)
{
	int msgid;
	int ret = 0;
	msgid=msgget(mqKey,IPC_EXCL);
	if(msgid<0){
		egsc_log_debug("get msgid fail! mqkey = %d errno=%d[%s]\n",mqKey,errno,strerror(errno));
		return -1;
	}
	ret = msgrcv(msgid,msgbuff,buffsize,msgType,dequeue_type);
	if(ret < 0){
		egsc_log_error("msgrcv() get msg failed,errno=%d[%s]\n",errno,strerror(errno));
	}
	else{
		egsc_log_info("[dequeue key=%u ret=%d]:statusCode=[%d]\n",mqKey,ret,msgbuff->msgData.statusCode);
	}
	return ret;
}

int Delete_MQ(unsigned int mqKey)
{
	int msgid;
	int ret = 0;
	msgid=msgget(mqKey,IPC_EXCL);
	if(msgid<0){
		egsc_log_error("get msgid by key [%u]fail!errno=%d[%s]\n",mqKey,errno,strerror(errno));
		return -1;
	}
	ret = msgctl(msgid,IPC_RMID,0);
	if(ret < 0){
		egsc_log_error("msgctl IPC_RMID failed,errno=%d[%s]\n",errno,strerror(errno));
	}
	else{
		egsc_log_debug("Delete MQ key=[%u],msqid=[%d]Complete! ret = %d\n",mqKey,msgid,ret);
	}
	return ret;
}
int DeleteAllMQ(int max_msg_id)
{
	int d_index = max_msg_id;
	unsigned int d_count = 0;
	for(;d_index>0;--d_index)
	{
		if(msgctl(d_index,IPC_RMID,0)==EGSC_RET_SUCCESS)
			++d_count;
	}
	printf("delete count:%d\n",d_count);
	return EGSC_RET_SUCCESS;
}


