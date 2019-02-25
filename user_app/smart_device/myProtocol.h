#ifndef _MYPROTOCOL_H
#define _MYPROTOCOL_H
#include "myMQ.h"
#include "egsc_def.h"
#define DEV_FORK_LIST_MAX_SIZE 16	//最多支持16种设备
#define DEV_MAX_COUNT 1000			//每种设备最大数目
#define ARG_ARR_COUNT 10			//定义Socket接收到的最大参数个数
#define ARG_LEN 100					//每个参数最大的长度

typedef enum _Rsv_Msg_Process_Result{
	No_Need_Rsp = 0,
	DSP_MSG = 1,
	SEND_MSG = 2
}RsvMsgProcResultEnum;
unsigned int CombineInt(unsigned int devType, unsigned int devIndex);
int Update_Dev_Fork_List(unsigned         int arr[], int arrIndex, EGSC_DEV_TYPE devType, int devCount);
int my_itoa(int intValue,char *outStr,int str_len);
int replace_string(char *result, char *source, const char* oldStr, char *destStr);
int replace_dev_id(char *result,char *source, char *dev_id);
int replace_sub_dev_id(char *result,char *source, char *sub_dev_id);
int replace_dev_type(char *result,char *source, int dev_type);
int replace_sub_dev_type(char *result,char *source, int sub_dev_type);
int ForkMulDev(unsigned int dev_arr[],msgQueenDataType *myarg);
//将source_arg字符串按照空格进行拆分放入指针数组result中
//最多拆分成arg_count个参数
//每个参数长度最大为ARG_LEN
//实际拆分出来的参数长度保存在used_count中
int split_arg_by_space(char *source_arg,char (*result)[ARG_LEN],int arg_count,int *used_count);
#endif
