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
