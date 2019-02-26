#ifndef _EGSC_SDK_H_
#define _EGSC_SDK_H_

#include "egsc_def.h"

/*
    设备状态回调函数定义
*/
typedef void (*egsc_dev_status_callback)(int handle, EGSC_DEV_STATUS_CODE status, char *desc_info);

typedef struct _egsc_subdev_id
{
    EGSC_SUBDEV_TYPE subdev_type;       // 子设备类型
    int subdev_num;                     // 子设备序号（1~9999）
    char subdev_mac[16];                // 设备唯一标识（使用MAC地址,112233AABBCC）
}egsc_subdev_id;

/*
    子设备信息结构体
*/
typedef struct _egsc_subdev_info
{
    egsc_subdev_id subdev_id;           // 子设备ID
    char dev_name[256];                 // 子设备名称
    char vendor_name[256];              // 子设备厂商名称
    int detail_type;                    // 子设备型号
    int register_type;                  // 注册类型
    char ip[32];                        // 子设备ip地址
    int port;                           // 端口
    char user_name[64];                 // 用户名
    char user_pw[64];                   // 用户密码
    char version[32];                   // 系统版本号
    int number;                         // 设备编号
    char soft_version[32];              // 软件版本号
}egsc_subdev_info;

/*
    设备信息结构体
*/
typedef struct _egsc_dev_info
{
    char srv_addr[64];                  // 服务器地址，格式示例："192.168.1.1:12000" 或 "egsc.evergrande.com:12000"
    //64bit标准SDK暂时不可用
    //char ip[32];                        // 用户IP配置
    int encrpyt_enable;                 // 服务器通信是否为加密方式
    EGSC_DEV_TYPE dev_type;             // 设备类型
    EGSC_VENDOR_NUMBER vendor_num;      // 厂商编号
    char id[16];                        // 设备唯一标识（使用MAC地址,112233AABBCC）
    char vendor_name[64];               // 厂商名称
    int machine_num;                    // 机号
    char location[64];                  // 位置信息
    char name[64];                      // 设备名称
    char version[64];                   // 系统版本号
    char alg_version[64];               // 算法版本号
    int subdev_count;                   // 子设备数量
    egsc_subdev_info * subdev_info;     // 子设备信息
}egsc_dev_info;

/*
    SDK初始化
*/
EGSC_RET_CODE egsc_sdk_init();

/*
    释放SDK
*/
void egsc_sdk_uninit();

/*
    创建设备
参数:
    info -- 设备信息
    status_cb -- 设备状态回调函数
    handle -- 创建的设备句柄
*/
EGSC_RET_CODE egsc_dev_create(egsc_dev_info *info,        egsc_dev_status_callback status_cb, int *handle);

/*
    设备功能注册
参数:
    handle -- 设备句柄
    srv_req_cb_tbl -- 服务器请求回调函数表指针，不同类型设备                有不同函数表, 由用户层传给SDK
    srv_req_cb_tbl_len -- 服务器请求回调函数表大小（字节数）
    dev_req_intface_buf -- 保存设备请求接口函数表的缓冲区地址，不同类型设备      有不同函数表, 由SDK返回给用户层
    buf_len -- 缓冲区大小（字节数）
    get_len -- 返回的函数表大小（字节数）
*/
EGSC_RET_CODE egsc_dev_func_register(int handle, void *srv_req_cb_tbl, int srv_req_cb_tbl_len,
                                                void * dev_req_if_tbl_buf, int buf_len, int *get_len);

/*
    启动设备
参数:
    handle -- 设备句柄
*/
EGSC_RET_CODE egsc_dev_start(int handle);


/*
    停止设备
参数:
    handle -- 设备句柄
*/
void egsc_dev_stop(int handle);

/*
    删除设备
参数:
    handle -- 设备句柄
*/
void egsc_dev_delete(int handle);

#endif
