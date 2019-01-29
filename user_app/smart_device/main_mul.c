#include<stdio.h>
#include<unistd.h> 
#include <errno.h> 
#include<string.h>
#include <errno.h> 

#include "mydev.h"
#include "myProtocol.h"

#include "myMQ.h"
int main_do_V2(msgQueenDataType *myarg)
{
	egsc_log_debug("[main:%d] devType=%d offset=%d\n",getpid(),myarg->devType,myarg->offset);
	msg_struct msgbuff;
	msgbuff.msgType = GetMQMsgType(EGSC_TYPE_DOOR_CTRL,0);
	msgbuff.msgData.devType = EGSC_TYPE_DOOR_CTRL;
	msgbuff.msgData.offset = 0;
	strcpy(msgbuff.msgData.info,"stop");
	PutDispatchNMQ(msgbuff,2);
	while(1)
    {
        egsc_platform_sleep(1000);
    }
	return EGSC_RET_SUCCESS;
}
int fork_do_V2(msgQueenDataType *myarg){
	egsc_log_debug("[Child:%d] devType=%d offset=%d\n",getpid(),myarg->devType,myarg->offset);
	int ret;
	ret = my_dev_single_init(myarg->devType,myarg->offset);
	return ret;
}

int main(int argc, char * argv [ ])
{
	//egsc_log_level = EGSC_LOG_DEBUG;
	int index = 0;
	EGSC_DEV_TYPE dev_type;
	int dev_count = 0;
	EGSC_RET_CODE ret = EGSC_RET_ERROR;

	msgQueenDataType myarg;
	int main_pid = getpid();
	pid_t fpid;
	
	mydev_init_V2();
	
	unsigned int dev_arr[DEV_FORK_LIST_MAX_SIZE]={0};
	ret = Update_Dev_Fork_List(dev_arr, 0, EGSC_TYPE_DOOR_CTRL, 2);
	//ret = Update_Dev_Fork_List(dev_arr, 1, EGSC_TYPE_PATROL_DEV, 4);
	egsc_log_debug("ret=%d\n",ret);
	if(ret<0)
		return ret;
	int child_break = 0;
	for(index=0;index<sizeof(dev_arr);++index){
		if(!dev_arr[index]){
			egsc_log_debug("Quit Fork!\n");
			break;
		}
		dev_type = dev_arr[index]>>DEV_INDEX_OFFSET;
		dev_count = dev_arr[index] & DEV_OFFSET_OP;
		egsc_log_debug("dev_type=%d dev_count=%d\n",dev_type,dev_count);
		myarg.devType = dev_type;
		int dev_index = 0;//不在外面定义有的编译器会有warning
		for(;dev_index<dev_count;++dev_index){
			myarg.offset = dev_index;
			fpid = fork();
			if(fpid<0){
				egsc_log_error("fork error,errno=%d[%s]\n",errno,strerror(errno));
				return -1;
			}
			else if(fpid == 0)
			{
				//I'm child
				child_break = 1;
				break;
			}
		}
		if(child_break)
			break;
	}
	if(getpid()==main_pid){
		return main_do_V2(&myarg);
	}
	else{
		return fork_do_V2(&myarg);
	}
	return ret;
}
