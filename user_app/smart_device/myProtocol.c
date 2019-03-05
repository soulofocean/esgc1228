#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdio.h>
#include "myProtocol.h"
#include "myMQ.h"
#include "egsc_util.h"
#include <time.h>

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
	char tmp[11] = {0};
	my_itoa(dev_type, tmp, sizeof(tmp));
	return replace_string(result, source, devTypeFlag, tmp);
}
int replace_sub_dev_type(char *result,char *source, int sub_dev_type)
{
	char tmp[11] = {0};
	my_itoa(sub_dev_type, tmp, sizeof(tmp));
	return replace_string(result, source, subDevTypeFlag, tmp);
}
int replace_record_type(char *result,char *source, int record_type)
{
	char tmp[11] = {0};
	my_itoa(record_type, tmp, sizeof(tmp));
	return replace_string(result, source, recordTypeFlag, tmp);
}
int replace_credence_type(char *result,char *source, int credence_type)
{
	char tmp[11] = {0};
	my_itoa(credence_type, tmp, sizeof(tmp));
	return replace_string(result, source, credenceTypeFlag, tmp);
}
int replace_credence_no(char *result,char *source, char *credence_no)
{
	return replace_string(result, source, credenceNoFlag, credence_no);
}
int replace_entry_type(char *result,char *source, int entry_type)
{
	char tmp[11] = {0};
	my_itoa(entry_type, tmp, sizeof(tmp));
	return replace_string(result, source, entryTypeFlag, tmp);
}
int replace_dev_mac(char *result,char *source, char *dev_mac)
{
	return replace_string(result, source, devMacFlag, dev_mac);
}
int replace_gate_open_mode(char *result,char *source, int gate_open_mode)
{
	char tmp[11] = {0};
	my_itoa(gate_open_mode, tmp, sizeof(tmp));
	return replace_string(result, source, gateOpenModeFlag, tmp);
}

int replace_img_path(char *result,char *source, char *img_path)
{
	return replace_string(result, source, imgPathFlag, img_path);
}
int replace_pass_type(char *result,char *source, int pass_type)
{
	char tmp[11] = {0};
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
	if(arg_index==arg_count-1)
	{
		(*used_count)++;
	}
	//display result
	for(arg_index=0;arg_index<ARG_ARR_COUNT;++arg_index){
		egsc_log_info("result[%d]=[%s]\tcompare:%d\n",arg_index,result[arg_index],strcmp(result[arg_index],"\0"));
	}
	return ret;
}
