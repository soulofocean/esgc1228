#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdio.h>
#include "myProtocol.h"
#include "myMQ.h"
#include "egsc_util.h"

const char devTypeFlag[] = "===DEV_TYPE===";
const char subDevTypeFlag[] = "===SUB_DEV_TYPE===";
const char devIdFlag[] = "===DEV_ID===";
const char subDevIDFlag[] = "===SUB_DEV_ID===";
int Update_Dev_Fork_List(unsigned         int arr[], int arrIndex, EGSC_DEV_TYPE devType, int devCount)
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
	int max_len = arg_count*ARG_LEN+arg_count-1;
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
		egsc_log_info("count=%d\n",*used_count);
	}
	//display result
	for(arg_index=0;arg_index<ARG_ARR_COUNT;++arg_index){
		egsc_log_info("result[%d]=[%s]\tcompare:%d\n",arg_index,result[arg_index],strcmp(result[arg_index],"\0"));
	}
	return ret;
}
