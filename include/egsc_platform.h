#ifndef _EGSC_PLATFORM_H
#define _EGSC_PLATFORM_H

#define EGSC_TASK_PRIORITY_MIN  0
#define EGSC_TASK_PRIORITY_MAX  32
#define EGSC_TASK_PRIORITY_BASE 16

#define EGSC_TASK_NO_WAIT       0
#define EGSC_TASK_WAIT_FOREVER  ((unsigned long) 0xFFFFFFFF)

// 多任务管理-----------------------------------------------

/*
    创建任务(线程)
参数:
   name -- 任务名
   id -- 保存创建的任务ID
   fn -- 任务入口函数
   arg -- 入口函数的参数
   stack_size -- 任务的栈大小
   priority -- 任务的优先级(0~32)
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_task_create(char* name, int* id,	void (*fn) (unsigned long),
            int arg, int stack_size, int priority);

/*
    删除任务
参数:
   id -- 任务ID
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_task_delete(int id);

/*
    创建互斥锁
参数:
   name -- 互斥锁名字
   arg -- 保留扩展使用
返回值:
   非NULL -- 成功,互斥锁句柄
   NULL -- 失败
*/
void * egsc_platform_mutex_create(char *name, void *arg);


/*
    加锁
参数:
   mutex -- 互斥锁句柄
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_mutex_lock(void *mutex);

/*
    解锁
参数:
   mutex -- 互斥锁句柄
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_mutex_unlock(void *mutex);

/*
    删除锁
参数:
   mutex -- 互斥锁句柄
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_mutex_delete(void *mutex);

/*
    睡眠函数
参数:
   msec -- 睡眠时间，单位ms
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_sleep(int msec);


// 消息队列-----------------------------------------------

/*
    创建消息队列
参数:
   name -- 队列名
   id -- 保存创建的队列ID
   size -- 队列大小
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_queue_create(char* name, int* id, int size);

/*
    发送消息
参数:
   id -- 队列ID
   msg -- 消息内容
   msg_len -- 消息长度
   wait -- 等待标志, 0--不等待，0xFFFFFFFF -- 永远等待
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_queue_send(int id, char* msg, int msg_len, int wait);

/*
    接收消息
参数:
   id -- 队列ID
   msg -- 保存消息内容
   buf_len -- 缓冲区长度
   get_len -- 接收消息长度
   wait -- 等待标志, 0--不等待，0xFFFFFFFF -- 永远等待
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_queue_recv(int id, char* msg, int buf_len, int *get_len, int wait);

/*
    删除消息队列
参数:
    id -- 队列ID
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_queue_delete(int id);


// 定时器-----------------------------------------------

/*
    创建周期性执行定时器
参数:
   id -- 保存定时器ID
   fn -- 定时器处理入口函数
   arg -- 入口函数的参数
   timeout -- 超时值 单位ms
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_timer_create(int* id, void(*fn)(unsigned int,void *), void* arg, int timeout);

/*
    启动定时器
参数:
   id -- 定时器ID
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_timer_start(int id);

/*
    停止定时器
参数:
   id -- 定时器ID
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_timer_stop(int id);

/*
    删除定时器
参数:
   id -- 定时器ID
返回值:
   0 -- 成功
   -1 -- 失败
*/
int egsc_platform_timer_delete(int id);


// 存储读写--------------------------------------------

/*
    读配置
 参数:
    id -- 配置id
    data -- 配置数据存储区
    buf_len -- 存储区大小
    get_len -- 读取大小
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_config_read(int id, void *data, int buf_len, int *get_len);


/*
    写入配置
 参数:
    id -- 配置id
    data -- 配置数据
    len -- 配置数据大小
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_config_write(int id, void *data, int len);


/*
    清除配置数据
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_config_clear(int id);


// 网络管理--------------------------------------------
/*
    设备地址信息结构体
*/
typedef struct _egsc_platform_addr_info
{
    char addr[64];                      // TCP/IP协议的IP地址，其他协议保存相关的地址描述
    char mask[64];                      // TCP/IP协议的子网掩码
    char gateway[64];                   // TCP/IP协议的本地网关地址
    unsigned char mac[6];               // TCP/IP协议的本地MAC地址
    int port;                           // TCP/IP协议的本地端口
}egsc_platform_addr_info;

/*
    获取连接本地地址信息
 参数:
    id -- 连接ID
    addr_info -- 连接本地地址信息
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_local_addr_get(int id, egsc_platform_addr_info *addr_info);

/*
    创建连接
 参数:
    id -- 连接ID
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_conn_create(int *id);

/*
    删除连接
 参数:
    id -- 连接ID
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_conn_delete(int id);


/*
    连接到服务器
 参数:
    id -- 连接ID
    addr -- 服务器地址
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_connect(int id, char * addr);

/*
    断开连接
 参数:
    id -- 连接ID
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_disconnect(int id);

/*
    发送数据
 参数:
    id -- 连接ID
    data -- 数据
    len -- 数据长度
 返回值:
    int -- 发送数据长度
    -1 -- 发送数据失败
*/
int egsc_platform_send(int id, char *data, int len);

/*
    接收数据
 参数:
    id -- 连接ID
    buff -- 数据缓冲区
    len -- 缓冲区长度
 返回值:
    int -- 接收数据长度
    -1 -- 接收数据出错 
*/
int egsc_platform_recv(int id, char *buff, int len);

/*
    设置连接状态回调函数
 参数:
    fn -- 连接状态回调函数
 返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_conn_status_cb_set(void (*fn)(int id, int status));

// 系统通用--------------------------------------------

/*
    获取时间
  tm_started -- 启动以来的毫秒数
  tm_str -- 保存获取的时间值，格式：yyyy-MM--dd HH:mm:ss
  buf_len -- 缓冲区大小
返回值:
*/
void egsc_platform_time_get(int *tm_started, char *tm_str, int buf_len);

/*
    设置时间
  tm_str -- 需要设置的时间值，格式：yyyy-MM--dd HH:mm:ss
返回值:
    0 -- 成功
    -1 -- 失败
*/
int egsc_platform_time_set(char *tm_str);

/*
    linux ntohl类似
*/
unsigned int egsc_platform_ntohl(unsigned int value);

/*
    linux htonl类似
*/
unsigned int egsc_platform_htonl(unsigned int value);

/*
    linux ntohs类似
*/
unsigned short egsc_platform_ntohs(unsigned short value);

/*
    linux htons类似
*/
unsigned short egsc_platform_htons(unsigned short value);

/*
    内存分配，类似于C malloc函数
参数:
   unsigned int  -- 请求分配空间大小
返回值:
   分配的内存指针，失败返回0
*/
void* egsc_platform_malloc(unsigned int size);

/*
    释放内存，类似于C free函数
参数:
   ptr  -- 释放内存的指针
*/
void  egsc_platform_free(void *ptr);

/*
    log print函数
参数:
   str  -- log
*/
void  egsc_platform_print(char *str);


#endif

