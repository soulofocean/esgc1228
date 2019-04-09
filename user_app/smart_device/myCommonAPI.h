#ifndef _MY_COMMON_API_H
#define _MY_COMMON_API_H

#define ARG_ARR_COUNT 16			//定义Socket接收到的最大参数个数
#define ARG_LEN 128					//每个参数最大的长度

int my_itoa(int intValue,char *outStr,int str_len);
int replace_string(char *result, char *source, const char* oldStr, char *destStr);
//将source_arg字符串按照空格进行拆分放入指针数组result中
//最多拆分成arg_count个参数
//每个参数长度最大为ARG_LEN
//实际拆分出来的参数长度保存在used_count中
int split_arg_by_space(char *source_arg,char (*result)[ARG_LEN],int arg_count,int *used_count);

#endif
