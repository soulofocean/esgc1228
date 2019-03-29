#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdio.h>
#include "myProtocol.h"
#include "myMQ.h"
#include "egsc_util.h"
#include <time.h>
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
int Update_Dev_Fork_List(unsigned int arr[], int arrIndex, EGSC_DEV_TYPE devType, int devCount)
{
	if(arrIndex > DEV_FORK_LIST_MAX_SIZE - 1){
		egsc_log_error("arrIndex out of range.\n");
		return EGSC_RET_ERROR;
	}
	arr[arrIndex] = GetMQMsgType(devType,devCount);
	return EGSC_RET_SUCCESS;
}
int my_itoa(int intValue,char *outStr,int str_len)
{
	return snprintf(outStr,str_len,"%d",intValue);
}
/*将字符串source中的oldStr子串替换为destStr字串,存放在result中*/
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
int split_arg_by_space(char *source_arg,char (*result)[ARG_LEN],int arg_count,int *used_count)
{
	int ret = 0;
	//计算理论上空格加上参数的总长度,arg_count意为最大拆分的长度，目前按照dev_ctl 0 0 record arg最长5个
	int max_len = ARG_ARR_COUNT*ARG_LEN+arg_count-1;
	*used_count = 0;
	if(strlen(source_arg)>max_len){
		egsc_log_error("source_arg too long[len=%d]\n",strlen(source_arg));
		return -1;
	}
	int arg_index = 0;
	char tmp[max_len+1];
	memset(tmp,0,max_len+1);
	for(arg_index=0;arg_index<arg_count;++arg_index){
		memset(result[arg_index],0,sizeof(result[arg_index]));
	}
	strcpy(tmp,source_arg);
	for(arg_index=0;arg_index<arg_count-1;++arg_index){
		sscanf(tmp,"%s %[^\n]",result[arg_index],result[arg_index+1]);
		if(strcmp(result[arg_index],"\0")==0){
			break;
		}
		(*used_count)++;
		strcpy(tmp,result[arg_index+1]);
	}
	if(arg_index == arg_count-1 && strcmp(result[arg_index],"\0")!=0 && *used_count<ARG_ARR_COUNT)
	{
		(*used_count)++;
	}
	//display result
	egsc_log_debug("usedcount=[%d]\n",*used_count);
	for(arg_index=0;arg_index<ARG_ARR_COUNT;++arg_index){
		egsc_log_info("result[%d]=[%s]\tcompare:%d\n",arg_index,result[arg_index],strcmp(result[arg_index],"\0"));
	}
	return ret;
}
