#include<stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>   
#include <errno.h> 
#include<string.h>

#include "egsc_util.h"
#include "socket_server.h"

#include "myMQ.h"
//#include<stdlib.h>


#define DEV_NUM 5

/*typedef struct _msgQueenDataType{
	int offset;
	int devType;
}msgQueenDataType;

typedef struct _message_struct{
	long msgType;
	msgQueenDataType msgData;
}msg_struct;*/

int main_send_mq()
{
	msg_struct msgs;
	int ret;
	int msqid;
	int msg_type;
	printf("ftok=%d\n",SOCKET_RSV_MQ_KEY);
	msqid=msgget(SOCKET_RSV_MQ_KEY,IPC_EXCL);  /*检查消息队列是否存在*/    
	if(msqid < 0){      
		msqid = msgget(SOCKET_RSV_MQ_KEY,IPC_CREAT|0666);/*创建消息队列*/
		if(msqid <0){
			printf("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));     
			return -1;
		}
	}
	while (1){
			for(msg_type=1;msg_type<=DEV_NUM;++msg_type){
				msgs.msgType = msg_type;
				//strcpy(msgs.a,"Hello");
				//msgs.a = myarg;
				msgs.msgData.devType = msg_type * 100;
				msgs.msgData.offset = msg_type-1;
				printf("msqid=%d\n",msqid);
				ret = msgsnd(msqid,&msgs,sizeof(msg_struct),IPC_NOWAIT);
				if(ret < 0){
					printf("msgsnd() write msg failed,ret=%d,errno=%d[%s]\n",ret,errno,strerror(errno));
					return -1;
				}
				printf("send complete!\n");
			}
			sleep(10);
	}
	//msgctl(MSGKEY,IPC_RMID,0);删除消息队列
}
int fork_rcv_mq(msgQueenDataType *myarg)
{
	int msgid,ret;
	msg_struct msgs;
	while(1){
		msgid=msgget(SOCKET_RSV_MQ_KEY,IPC_EXCL);
		if(msgid<0){
			printf("get msgid fail!errno=%d[%s]",errno,strerror(errno));
			sleep(1);
			continue;
		}
		ret = msgrcv(msgid,&msgs,sizeof(msg_struct),myarg->offset,0);
		if(ret < 0){
			printf("msgrcv() get msg failed,errno=%d[%s]\n",errno,strerror(errno));
			sleep(1);
			continue;
		}
		printf("devtype=[%d] offset=[%d] pid=[%d]\n",msgs.msgData.devType,msgs.msgData.offset,getpid());
	}
}
int main_do(msgQueenDataType *myarg)
{
	printf("I'm Main Thread\n");
	return main_send_mq();
}
int fork_do(msgQueenDataType *myarg){
	printf("devType:%d,offset:%d,pid:%d\n",myarg->devType,myarg->offset,getpid());
	return fork_rcv_mq(myarg);
}
int main_fork(int argc, char * argv [ ])
{
	int main_pid = getpid();
	printf("Test Fork![%d->%d]\n",getppid(),getpid());
	int count=0;
	msgQueenDataType myarg;
	pid_t fpid;
	for(count=0;count<DEV_NUM;++count){
		fpid = fork();
		if(fpid<0)
			return -1;
		else if(fpid==0){
			//printf("count:%d [%d->%d->%d]\n",count++,getppid(),getpid(),fpid);
			myarg.offset = count+1;
			break;
		}
	}
	printf("count:%d [%d->%d->%d]\n",count++,getppid(),getpid(),fpid);
	if(getpid()==main_pid){
		myarg.devType = 888;
		myarg.offset = 0;
		main_do(&myarg);
	}
	else{
		fork_do(&myarg);
	}
	printf("[%d] finished!\n",getpid());
    return 0;  
}

void test()
{
	printf("test\n");
}

int main_new(int argc, char * argv [ ])
{
	egsc_log_debug("main enter\n");
    int ret=0;
	int mqKey = 8888;
	msg_struct msgs,msgs_out={0};
	//msg_struct *p=(msg_struct*)malloc(sizeof(msg_struct));
	msgs.msgData.devType = 9;
	msgs.msgData.offset = 3;
	msgs.msgType = (msgs.msgData.devType << 16) + msgs.msgData.offset;
	strncpy(msgs.msgData.info,"HelloMQ",MQ_INFO_BUFF);
	printf("msgType:%ld %ld %ld\n",msgs.msgType,msgs.msgType >> 16,msgs.msgType & 0xFFFF);
	ret = Enqueue_MQ(mqKey, msgs, sizeof(msg_struct)-sizeof(long),ipc_no_wait);
	msgs.msgData.offset = 5;
	ret = Enqueue_MQ(mqKey, msgs, sizeof(msg_struct)-sizeof(long),ipc_no_wait);
	while(1){
		ret = Dequeue_MQ(mqKey, msgs.msgType, &msgs_out,sizeof(msg_struct),ipc_no_wait);
		if(ret<0)
			break;
		printf("main:devtype=[%d] offset=[%d]\ninfo=%s\n",msgs_out.msgData.devType,msgs_out.msgData.offset,msgs_out.msgData.info);
	}
	//msgs_out = Dequeue_MQ(mqKey, msgs.msgType);
	//printf("main:devtype=[%d] offset=[%d]\n",msgs_out.msgData.devType,msgs_out.msgData.offset);
	//ret = Delete_MQ(mqKey);//删除消息队列
	//mydev_init_V2();
	//char tmp[20]={0};
	//if(argv[1])
	//	strncpy(tmp,argv[1],sizeof(tmp));
	//socketServerStart(SOCKET_SERVER_PORT, SOCKET_SERVER_LISNUM, tmp);
	return ret;
	//return main_fork(argc,argv);
}

