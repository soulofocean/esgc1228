#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdio.h>
#include "myProtocol.h"
#include "myMQ.h"
#include "egsc_util.h"
#include <time.h>
#include "myCommonAPI.h"
#define INT_STR_LEN 11
const char devStatusFlag[] = "===DEV_STATUS===";
const char sysTimeFlag[] = "===SYSTEM_TIME===";
const char devTypeFlag[] = "===DEV_TYPE===";
const char subDevTypeFlag[] = "===SUB_DEV_TYPE===";
const char devIdFlag[] = "===DEV_ID===";
const char subDevIDFlag[] = "===SUB_DEV_ID===";
const char recordTypeFlag[] = "===REC_TYPE===";
const char credenceTypeFlag[] = "===CRE_TYPE===";
const char credenceNoFlag[] = "===CRE_NO===";
const char entryTypeFlag[] = "===ENTRY_TYPE===";
const char devMacFlag[] = "===DEV_MAC===";
const char gateOpenModeFlag[] = "===GATE_OPEN_MODE===";
const char imgPathFlag[] = "===IMG_PATH===";
const char passTypeFlag[] = "===PASS_TYPE===";
const char eventTypeFlag[] = "===EVENT_TYPE===";
const char errCodeFlag[] = "===ERR_CODE===";
//电梯的FLAG
const char workModeFlag[] = "===WORK_MODE===";
const char stateFlag[] = "===STATE===";
const char floorFlag[] = "===FLOOR===";
const char dicrectionFlag[] = "===DICRECTION===";

const char userTypeFlag[] = "===USER_TYPE===";
const char userIDFlag[] = "===USER_ID===";
const char destFloorFlag[] = "===DEST_FLOOR===";
const char lightModeFlag[] = "===LIGHT_MODE===";

const char carIDFlag[] = "===Car_ID===";//i
const char phyFloorFlag[] = "===PHY_FlOOR===";//i
const char dispFloorFlag[] = "===DISP_FlOOR===";
const char carStatusFLag[] = "===CAR_STATUS===";
const char doorStatusFlag[] = "===DOOR_STATUS===";
const char errStatusFlag[] = "===ERR_STATUS===";//i
const char errMsgFlag[] = "===ERR_MSG===";
const char fireCtlStatusFlag[] = "===FIRE_CTL_STATUS===";//i
unsigned int global_fork_us = 1000;
DEV_MSG_ACK_ENUM global_ack_type = NO_ACK;
long global_msg_type = SOCKET_SEND_MSG_TYPE;
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
int Update_Dev_Fork_List(unsigned int arr[], int arrIndex, EGSC_DEV_TYPE devType, int devCount)
{
	if(arrIndex > DEV_FORK_LIST_MAX_SIZE - 1){
		egsc_log_error("arrIndex out of range.\n");
		return EGSC_RET_ERROR;
	}
	arr[arrIndex] = GetMQMsgType(devType,devCount);
	return EGSC_RET_SUCCESS;
}
int replace_err_code(char *result,char *source, int err_code)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(err_code, tmp, sizeof(tmp));
	return replace_string(result, source, errCodeFlag, tmp);
}

int replace_dev_status(char *result,char *source, int dev_status)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(dev_status, tmp, sizeof(tmp));
	return replace_string(result, source, devStatusFlag, tmp);
}
int replace_event_type(char *result,char *source, int event_type)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(event_type, tmp, sizeof(tmp));
	return replace_string(result, source, eventTypeFlag, tmp);
}
int replace_disp_floor(char* result, char* source, char* disp_floor)
{
	return replace_string(result, source, dispFloorFlag, disp_floor);
}
int replace_car_status(char* result, char* source, char* car_status)
{
	return replace_string(result, source, carStatusFLag, car_status);
}
int replace_door_status(char* result, char* source, char* door_status)
{
	return replace_string(result, source, doorStatusFlag, door_status);
}
int replace_err_msg(char* result, char* source, char* err_msg)
{
	return replace_string(result, source, errMsgFlag, err_msg);
}
int replace_car_id(char *result,char *source, int car_id)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(car_id, tmp, sizeof(tmp));
	return replace_string(result, source, carIDFlag, tmp);
}
int replace_phy_floor(char *result,char *source, int phy_floor)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(phy_floor, tmp, sizeof(tmp));
	return replace_string(result, source, phyFloorFlag, tmp);
}
int replace_err_status(char *result,char *source, int err_status)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(err_status, tmp, sizeof(tmp));
	return replace_string(result, source, errStatusFlag, tmp);
}
int replace_fire_ctl_status(char *result,char *source, int fire_ctl_status)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(fire_ctl_status, tmp, sizeof(tmp));
	return replace_string(result, source, fireCtlStatusFlag, tmp);
}
int replace_user_id(char* result, char* source, char* user_id)
{
	return replace_string(result, source, userIDFlag, user_id);
}
int replace_light_mode(char* result, char* source, char* light_mode)
{
	return replace_string(result, source, lightModeFlag, light_mode);
}
int replace_user_type(char *result,char *source, int user_type)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(user_type, tmp, sizeof(tmp));
	return replace_string(result, source, userTypeFlag, tmp);
}
int replace_dest_floor(char *result,char *source, char* dest_floor)
{
	return replace_string(result, source, destFloorFlag, dest_floor);
}
int replace_work_mode(char* result, char* source, char* work_mode)
{
	return replace_string(result, source, workModeFlag, work_mode);
}
int replace_status(char *result,char *source, int status)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(status, tmp, sizeof(tmp));
	return replace_string(result, source, stateFlag, tmp);
}
int replace_floor(char *result,char *source, int floor)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(floor, tmp, sizeof(tmp));
	return replace_string(result, source, floorFlag, tmp);
}
int replace_dicrection(char *result,char *source, int dicrection)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(dicrection, tmp, sizeof(tmp));
	return replace_string(result, source, dicrectionFlag, tmp);
}
int replace_system_time(char *result,char *source)
{
	char now[100] = {0};
	time_t nowtim;
	struct tm *tm_now ;
	time(&nowtim) ;
	tm_now = localtime(&nowtim) ;		
	sprintf(now, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
		tm_now->tm_year+1900, tm_now->tm_mon+1,tm_now->tm_mday, tm_now->tm_hour, 
		tm_now->tm_min, tm_now->tm_sec);
	return replace_string(result, source, sysTimeFlag, now);
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
	char tmp[INT_STR_LEN] = {0};
	my_itoa(dev_type, tmp, sizeof(tmp));
	return replace_string(result, source, devTypeFlag, tmp);
}
int replace_sub_dev_type(char *result,char *source, int sub_dev_type)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(sub_dev_type, tmp, sizeof(tmp));
	return replace_string(result, source, subDevTypeFlag, tmp);
}
int replace_record_type(char *result,char *source, int record_type)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(record_type, tmp, sizeof(tmp));
	return replace_string(result, source, recordTypeFlag, tmp);
}
int replace_credence_type(char *result,char *source, int credence_type)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(credence_type, tmp, sizeof(tmp));
	return replace_string(result, source, credenceTypeFlag, tmp);
}
int replace_credence_no(char *result,char *source, char *credence_no)
{
	return replace_string(result, source, credenceNoFlag, credence_no);
}
int replace_entry_type(char *result,char *source, int entry_type)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(entry_type, tmp, sizeof(tmp));
	return replace_string(result, source, entryTypeFlag, tmp);
}
int replace_dev_mac(char *result,char *source, char *dev_mac)
{
	return replace_string(result, source, devMacFlag, dev_mac);
}
int replace_gate_open_mode(char *result,char *source, int gate_open_mode)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(gate_open_mode, tmp, sizeof(tmp));
	return replace_string(result, source, gateOpenModeFlag, tmp);
}

int replace_img_path(char *result,char *source, char *img_path)
{
	return replace_string(result, source, imgPathFlag, img_path);
}
int replace_pass_type(char *result,char *source, int pass_type)
{
	char tmp[INT_STR_LEN] = {0};
	my_itoa(pass_type, tmp, sizeof(tmp));
	return replace_string(result, source, passTypeFlag, tmp);
}


int ForkMulDev(unsigned int dev_arr[],msgQueenDataType *myarg)
{
	int index = 0;
	EGSC_DEV_TYPE dev_type;
	int dev_count = 0;
	pid_t fpid;
	int child_break = 0;
	for(index=0;index<DEV_FORK_LIST_MAX_SIZE;++index){
		if(!dev_arr[index]){
			egsc_log_debug("Quit Fork!\n");
			break;
		}
		dev_type = GetDevType(dev_arr[index]);
		dev_count = GetDevCount(dev_arr[index]);
		egsc_log_debug("dev_type=%d dev_count=%d\n",dev_type,dev_count);
		myarg->devType = dev_type;
		int dev_index = 0;//不在外面定义有的编译器会有warning
		for(dev_index = 0;dev_index<dev_count;++dev_index){
			myarg->offset = dev_index;
			fpid = fork();
			if(fpid<0){
				egsc_log_error("fork error,errno=%d[%s]\n",errno,strerror(errno));
				return -1;
			}
			else if(fpid == 0)
			{
				//I'm child
				global_msg_type = GetMQMsgType(dev_type, dev_index);
				child_break = 1;
				break;
			}
			usleep(global_fork_us);
		}
		if(child_break)
			break;
	}
	return 0;
}
int PutRsvMQ(msg_struct msgs)
{
	return Enqueue_MQ(SOCKET_RSV_MQ_KEY, msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutSendMQ(int code,const char* func_name,char * info)
{
	msg_struct msgs;
	memset(&msgs,0,sizeof(msg_struct));
	msgs.msgType = global_msg_type;
	char tmp[MQ_INFO_BUFF] = {0};
	#if 0 
	//这些操作放在Client端执行替换就可以了，此处不做处理
	char jsonmsg[MQ_INFO_BUFF] = {0};
	if(strstr(info,"{")==NULL)
	{
		snprintf(tmp,MQ_INFO_BUFF-1,"{\"pid\":\%u,\"code\":%d,\"func\":\"\%s\",\"info\":\"%s\"}",getpid(),code,func_name,info);
	}
	else
	{
		snprintf(tmp,MQ_INFO_BUFF-1,"{\"pid\":\%u,\"code\":%d,\"func\":\"\%s\",\"info\":%s}",getpid(),code,func_name,info);
	}
	replace_string(jsonmsg, tmp, "\"{", "{");
	strncpy(tmp,jsonmsg,MQ_INFO_BUFF-1);
	replace_string(jsonmsg, tmp, "}\"", "}");
	strncpy(tmp,jsonmsg,MQ_INFO_BUFF-1);
	#else
	snprintf(tmp,MQ_INFO_BUFF-1,"{\"pid\":\%u,\"code\":%d,\"func\":\"\%s\",\"info\":\"%s\"}",getpid(),code,func_name,info);
	#endif
	strncpy(msgs.msgData.info,tmp,sizeof(msgs.msgData.info)-1);
	return Enqueue_MQ(SOCKET_SEND_MQ_KEY, msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutSendShortMQ(int status_code)
{
	msg_short_struct msgs;
	memset(&msgs,0,sizeof(msg_short_struct));
	msgs.msgType = global_msg_type;
	msgs.msgData.statusCode = status_code;
	return Enqueue_MQ_Short(SOCKET_SEND_SHORT_MQ_KEY, msgs, MQ_SEND_BUFF_SHORT, ipc_no_wait);
}

int PutDispatchMQ(int dev_type,int dev_index,char* info)
{
	egsc_log_debug("Enter PutDispatchMQ dev_type=[%d] dev_index=[%d] info=[%s]\n",dev_type,dev_index,info);
	msg_struct msgs;
	msgs.msgType = GetMQMsgType(dev_type, dev_index);
	msgs.msgData.devType = dev_type;
	msgs.msgData.offset = dev_index;
	strncpy(msgs.msgData.info,info,sizeof(msgs.msgData.info));
	return Enqueue_MQ(GetDispatchMQKey(msgs.msgType), msgs, MQ_SEND_BUFF, ipc_no_wait);
}
int PutDispatchNMQ(msg_struct msgs,int put_count)
{
	int index = 0;
	int ret = 0;
	for(;index<put_count;++index){
		ret = Enqueue_MQ(GetDispatchMQKey(msgs.msgType), msgs, MQ_SEND_BUFF, ipc_no_wait);
		if(ret < 0)
			return ret;
		if(index<put_count){
			msgs.msgType++;
			msgs.msgData.offset++;
		}
	}
	return ret;
}

int GetRsvMQ(msg_struct *msgbuff)
{
	return Dequeue_MQ(SOCKET_RSV_MQ_KEY, 0, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}
int GetSendMQ(msg_struct *msgbuff)
{
	return Dequeue_MQ(SOCKET_SEND_MQ_KEY, 0, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}
int GetSendShortMQ(msg_short_struct *msgbuff)
{
	return Dequeue_MQ_Short(SOCKET_SEND_SHORT_MQ_KEY, 0, msgbuff, MQ_RSV_BUFF_SHORT, ipc_need_wait);
}

int GetDispatchMQ(long msgType,msg_struct *msgbuff)
{
	return Dequeue_MQ(GetDispatchMQKey(msgType), 0, msgbuff, MQ_RSV_BUFF, ipc_need_wait);
}
int DelDispatchMQ(long msgType)
{
	return Delete_MQ(GetDispatchMQKey(msgType));
}
void DevMsgAck(int code,const char* func_name,char* msg)
{
	egsc_log_debug("enter.\n");
	EGSC_RET_CODE ret = EGSC_RET_ERROR;
	//后续useLongMsg可能扩展成枚举，这里先直接判断
	switch (global_ack_type)
	{
		case LONG_ACK:
		{
			ret = PutSendMQ(code,func_name,msg);
			break;
		}
		case SHORT_ACK:
		{
			ret = PutSendShortMQ(code);
			break;
		}
		case NO_ACK:
		{
			egsc_log_debug("PID[%d] is set NO_ACK\n",getpid());
			break;
		}
		default:
		{
			egsc_log_error("Invalid Type:[%d]\n",global_ack_type);
			break;
		}
	}
	egsc_log_debug("pid:[%d] global_ack_type=[%d] ret=[%d]\n",getpid(),global_ack_type,ret);
}
