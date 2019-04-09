#include<stdio.h>
#include<unistd.h> 
#include <errno.h> 
#include<string.h>

#include "mydev.h"
#include "myProtocol.h"

#include "myMQ.h"
#include <signal.h>
#include <sys/wait.h>

#include<mySocket.h>

static int socket_rcv_tid = -1; //socket接收线程ID
static int socket_send_tid = -1;//socket发送线程ID
static int socket_send_short_tid = -1;//socket发送线程ID


int main_do_V2(unsigned int *dev_arr,int arr_size)
{
	egsc_log_info("[main:%d] arr_size=%d\n",getpid(),arr_size);
	main_process_loop(dev_arr,arr_size);
	return EGSC_RET_SUCCESS;
}
int fork_do_V2(msgQueenDataType *myarg){
	egsc_log_info("[Child:%d] devType=%d offset=%d\n",getpid(),myarg->devType,myarg->offset);
	int ret;
	ret = my_dev_single_init(myarg->devType,myarg->offset);
	return ret;
}
void socket_rcv_fn(unsigned long arg)
{
	egsc_log_debug("enter.\n");
	char buff[SOCKET_RCV_BUFF] = {0};
	int socket_id = -1;
	socketServerInit(SOCKET_SERVER_PORT,SOCKET_SERVER_LISNUM, buff, &socket_id);
	socketServerLoopRsv(socket_id);
}
void socket_send_fn(unsigned long arg)
{
	egsc_log_debug("enter.\n");
	socketServerLoopSend();
}
void socket_send_short_fn(unsigned long arg)
{
	egsc_log_debug("enter.\n");
	socketServerLoopSendShort();
}


int main(int argc, char * argv [ ])
{
	egsc_log_level = EGSC_LOG_DEBUG;
	signal(SIGCHLD, SIG_IGN);//和主进程wait(NULL)选着搭配用防止僵尸进程,去掉后进程会提示为僵尸进程
	signal(SIGPIPE, SIG_IGN);//Socket的异常断开处理忽略
	EGSC_RET_CODE ret = EGSC_RET_ERROR;
	int arg = 0;
	//初始化设备配置文件
	ret = mydev_init_V2();
	if(ret<0)
		return ret;
	ret = egsc_platform_task_create("socket_rcv_task", &socket_rcv_tid, socket_rcv_fn, arg, 1024, 0);
	ret = egsc_platform_task_create("socket_send_task", &socket_send_tid, socket_send_fn, arg, 1024, 0);
	ret = egsc_platform_task_create("socket_send_short_task", &socket_send_short_tid, socket_send_short_fn, arg, 1024, 0);
	ret = process_loop_msg();
	return ret;
	#if 0
	msgQueenDataType myarg;
	int main_pid = getpid();
	//无Socket的保留代码，目前被return被截断
	unsigned int dev_arr[DEV_FORK_LIST_MAX_SIZE]={0};
	ret = Update_Dev_Fork_List(dev_arr, 0, EGSC_TYPE_DOOR_CTRL, 2);
	//ret = Update_Dev_Fork_List(dev_arr, 1, EGSC_TYPE_PATROL_DEV, 4);
	egsc_log_debug("ret=%d\n",ret);
	if(ret<0)
		return ret;
	//按照dev_arr中信息为每个设备fork一个进程，每个设备信息保存在myarg中
	ret = ForkMulDev(dev_arr,&myarg);
	if(ret < 0)
		return ret;
	if(getpid()==main_pid){
		ret = main_do_V2(dev_arr,sizeof(dev_arr)/sizeof(dev_arr[0]));
		wait(NULL);//和signal(SIGCHLD, SIG_IGN)搭配用
		//main_do_V2(dev_arr,sizeof(dev_arr)/sizeof(dev_arr[0]));
	}
	else{
		ret = fork_do_V2(&myarg);
	}
	return ret;
	#endif
}
