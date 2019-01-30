#ifndef _MYPROTOCOL_H
#define _MYPROTOCOL_H
#include "myMQ.h"
#include "egsc_def.h"
#define DEV_FORK_LIST_MAX_SIZE 16	//最多支持16种设备
#define DEV_INDEX_OFFSET 16			//预留16bit放设备序号，DEVTYPE<<16
#define DEV_OFFSET_OP 0xFFFF		//和上面的16bits对应
typedef enum _Rsv_Msg_Process_Result{
	No_Need_Rsp = 0,
	DSP_MSG = 1,
	SEND_MSG = 2
}RsvMsgProcResultEnum;

int processMsgFromRsvMQ(msg_struct *msgbuff);
RsvMsgProcResultEnum handleMsg(msg_struct *msgbuff, int *count);
unsigned int CombineInt(unsigned int devType, unsigned int devIndex);
int Update_Dev_Fork_List(unsigned         int arr[], int arrIndex, EGSC_DEV_TYPE devType, int devCount);
unsigned int GetMQMsgType(int dev_type,int dev_offset);
unsigned int GetDevType(unsigned int msg_type);
unsigned int GetDevCount(unsigned int msg_type);
#endif
