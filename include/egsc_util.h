#ifndef _EGSC_UTIL_H_
#define _EGSC_UTIL_H_

#include <stdarg.h>
#include "egsc_def.h"

#ifndef NULL
#define NULL ((void *)0)
#endif


// 调试信息输出辅助接口 ----------------------------------------------------

enum EGSC_LOG_LEVEL //调试信息打印级别
{
    EGSC_LOG_ERROR  = 0,
    EGSC_LOG_INFO   = 1,
    EGSC_LOG_DEBUG  = 2,
    EGSC_LOG_TRACE  = 3,
};
extern int egsc_log_level;
int egsc_log_print(int level, const char * format, ...);
#define egsc_log_debug(fmt, args...)  egsc_log_print(EGSC_LOG_DEBUG,"[EGSC %s:%d] "fmt,__func__, __LINE__, ##args)
#define egsc_log_info(fmt, args...)   egsc_log_print(EGSC_LOG_INFO, "[EGSC %s:%d] "fmt,__func__, __LINE__, ##args)
#define egsc_log_error(fmt, args...)  egsc_log_print(EGSC_LOG_ERROR,"[EGSC %s:%d] "fmt,__func__, __LINE__, ##args)

// -------------------------------------------------------------------------



// 字符串辅助接口 ----------------------------------------------------

// 类似C snprintf函数
int egsc_snprintf(char *buf, unsigned int size, const char *fmt, ...);

//类似C vsnprintf函数
int egsc_vsnprintf(char *buf, unsigned int size, const char *fmt, va_list args);

//类似C vsnprintf函数
int egsc_sprintf(char *buf, const char *fmt, ...);

// -------------------------------------------------------------------------



#endif

