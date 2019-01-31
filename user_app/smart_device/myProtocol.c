#include<string.h>
#include<stdio.h>
#include "myProtocol.h"
#include "myMQ.h"
#include "egsc_util.h"

const char devTypeFlag[] = "===DEV_TYPE===";
const char subDevTypeFlag[] = "===SUB_DEV_TYPE===";
const char devIdFlag[] = "===DEV_ID===";
const char subDevIDFlag[] = "===SUB_DEV_ID===";

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
int my_itoa(int intValue,char *outStr,int str_len)
{
	return snprintf(outStr,str_len,"%d",intValue);
}
/*将字符串source中的s1子串替换为s2字串*/
int replace_string(char *result, char *source, const char* oldStr, char *destStr)
{
    char *q=NULL;
    char *p=NULL;	
	if(NULL == result || NULL == source || NULL == oldStr || NULL == destStr)
		return -1;	   
    p=source;
    while((q=strstr(p, oldStr))!=NULL)
    {
        strncpy(result, p, q-p);
        result[q-p]= '\0';//very important, must attention!
        strcat(result, destStr);
        strcat(result, q+strlen(oldStr));
        strcpy(p,result);
    }
    strcpy(result, p);   
	return 0;
}
int replace_dev_id(char *result,char *source, char *dev_id)
{
	return replace_string(result, source, devIdFlag, dev_id);
}
int replace_sub_dev_id(char *result,char *source, char *sub_dev_id)
{
	return replace_string(result, source, subDevIDFlag, sub_dev_id);
}
int replace_dev_type(char *result,char *source, int dev_type)
{
	char tmp[20] = {0};
	my_itoa(dev_type, tmp, 20);
	return replace_string(result, source, devTypeFlag, tmp);
}
int replace_sub_dev_type(char *result,char *source, int sub_dev_type)
{
	char tmp[20] = {0};
	my_itoa(sub_dev_type, tmp, 20);
	return replace_string(result, source, subDevTypeFlag, tmp);
}

