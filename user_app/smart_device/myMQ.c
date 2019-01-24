#include<stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>   
#include <errno.h> 
#include<string.h>
#include "myMQ.h"

int Enqueue_MQ(int mqKey,msg_struct msgs)
{
	int ret;
	int msqid;
	msqid = msgget(mqKey,IPC_EXCL);  /*检查消息队列是否存在*/    
	if(msqid < 0){      
		msqid = msgget(mqKey,IPC_CREAT|0666);/*创建消息队列*/
		if(msqid <0){
			printf("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));     
			return -1;
		}
	}
	ret = msgsnd(msqid,&msgs,sizeof(msg_struct),IPC_NOWAIT);
	if(ret < 0){
		printf("msgsnd() write msg failed,ret=%d,errno=%d[%s]\n",ret,errno,strerror(errno));
	}
	printf("send complete!\n");
	return ret;
}

int Dequeue_MQ(int mqKey,long msgType, msg_struct *msgs)
{
	int msgid;
	int ret;
	msgid=msgget(mqKey,IPC_EXCL);
	if(msgid<0){
		printf("get msgid fail!errno=%d[%s]",errno,strerror(errno));
		return -1;
	}
	ret = msgrcv(msgid,msgs,sizeof(msg_struct),msgType,0);
	if(ret < 0){
		printf("msgrcv() get msg failed,errno=%d[%s]\n",errno,strerror(errno));
	}
	printf("devtype=[%d] offset=[%d] pid=[%d]\n",msgs->msgData.devType,msgs->msgData.offset,getpid());
	return ret;
}
