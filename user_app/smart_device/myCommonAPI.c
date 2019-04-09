#include "myCommonAPI.h"
#include <string.h>
#include <stdio.h>
#include "egsc_util.h"
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

