#ifndef _MYPROTOCOL_H
#define _MYPROTOCOL_H
#include "myMQ.h"
typedef enum _Rsv_Msg_Process_Result{
	No_Need_Rsp = 0,
	DSP_MSG = 1,
	SEND_MSG = 2
}RsvMsgProcResultEnum;

int processMsgFromRsvMQ(msg_struct *msgbuff);
RsvMsgProcResultEnum handleMsg(msg_struct *msgbuff, int *count);
#endif
