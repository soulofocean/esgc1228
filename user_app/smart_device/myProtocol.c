#include "myProtocol.h"
#include "myMQ.h"
#include "egsc_util.h"
int processMsgFromRsvMQ(msg_struct *msgbuff)
{
	int ret;
	int process_count;
	RsvMsgProcResultEnum result;
	ret = GetRsvMQ(msgbuff);
	if(ret < 0)
		return ret;
	result = handleMsg(msgbuff,&process_count);
	switch (result)
		{
			case DSP_MSG:
				{
					PutDispatchNMQ(*msgbuff,process_count);
					break;
				}
			case SEND_MSG:
				{
					PutSendMQ(*msgbuff);
					break;
				}
			case No_Need_Rsp:
			default: break;
		}
	return 0;
}
RsvMsgProcResultEnum handleMsg(msg_struct *msgbuff, int *count)
{
	//TODO
	*count = 1;
	return No_Need_Rsp;
}
int Update_Dev_Fork_List(unsigned         int arr[], int arrIndex, EGSC_DEV_TYPE devType, int devCount)
{
	if(arrIndex > DEV_FORK_LIST_MAX_SIZE - 1){
		egsc_log_error("arrIndex out of range.\n");
		return EGSC_RET_ERROR;
	}
	arr[arrIndex] = GetMQMsgType(devType,devCount);
	return EGSC_RET_SUCCESS;
}
unsigned int GetMQMsgType(int dev_type,int dev_offset)
{
	return (dev_type << DEV_INDEX_OFFSET) + dev_offset;
}
unsigned int GetDevType(unsigned int msg_type)
{
	return msg_type>>DEV_INDEX_OFFSET;
}
unsigned int GetDevCount(unsigned int msg_type)
{
	return msg_type & DEV_OFFSET_OP;
}
