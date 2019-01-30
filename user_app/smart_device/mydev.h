#ifndef _MYDEV_H
#define _MYDEV_H

#include <egsc_sdk.h>
#include <egsc_util.h>
#include <egsc_platform.h>
#include "mydev_list.h"

#define egsc_log_user(fmt, args...)  egsc_log_print(EGSC_LOG_INFO,"[EGSC] "fmt, ##args)


typedef enum _MYDEV_CREDENCE_TYPE
{
    CREDENCE_TYPE_BUTTON                = 0,        //按钮（协议文档当前未定义，临时添加）
    CREDENCE_TYPE_EM                    = 1,        //ID卡
    CREDENCE_TYPE_MF1_A                 = 2,        //IC卡，TypeA 标准
    CREDENCE_TYPE_MF1_B                 = 3,        //IC卡，TypeB 标准
    CREDENCE_TYPE_CPU                   = 4,        //CPU 卡
    CREDENCE_TYPE_CAR_PLATE             = 5,        //车牌
    CREDENCE_TYPE_FINGER_PRINT          = 6,        //指纹
    CREDENCE_TYPE_FACE                  = 7,        //人脸
    CREDENCE_TYPE_TICKET                = 8,        //纸票/二维码
    CREDENCE_TYPE_BLUETOOTH             = 9,        //蓝牙
    CREDENCE_TYPE_SECRET                = 10,       //固定密码
    CREDENCE_TYPE_DYNAMICPASSWORD       = 11,       //动态密码
    CREDENCE_TYPE_NFC                   = 12,       //NFC
}MYDEV_CREDENCE_TYPE;

typedef enum _MYDEV_USER_TYPE
{
    USER_TYPE_MONTH_A   = 1,    //固定用户A (物业管理人员)
    USER_TYPE_MONTH_B   = 2,    //固定用户B（业主）
    USER_TYPE_MONTH_C   = 3,    //固定用户C（其他工作人员）
    USER_TYPE_MONTH_D   = 4,    //固定用户D
    USER_TYPE_TEMP_A    = 5,    //临时用户A
    USER_TYPE_TEMP_B    = 6,    //临时用户B
}MYDEV_USER_TYPE;

typedef enum _MYDEV_RECORD_TYPE
{
    RECORD_TYPE_BURSH_CARD_OPEN_PARK_RECORD             = 10001,    //刷卡开闸事件上报
    RECORD_TYPE_CARNO_OPEN_PARK_RECORD                  = 10002,    //车牌开闸事件上报
    RECORD_TYPE_REMOTE_OPEN_PARK_RECORD                 = 10003,    //车场远程开闸事件上报
    RECORD_TYPE_MANUAL_OPEN_PARK_RECORD                 = 10004,    //手动开闸事件上报
    RECORD_TYPE_FACE_OPEN_DOOR_RECORD                   = 30000,    //人脸开门上报
    RECORD_TYPE_REMOTE_OPEN_DOOR_RECORD                 = 30001,    //远程开门上报
    RECORD_TYPE_BURSH_CARD_OPEN_DOOR_RECORD             = 30002,    //刷卡开门上报
    RECORD_TYPE_QR_CODE_OPEN_DOOR_RECORD                = 30003,    //二维码开门上报
    RECORD_TYPE_FINGER_OPEN_DOOR_RECORD                 = 30004,    //指纹开门上报
    RECORD_TYPE_PASSWORD_OPEN_DOOR_RECORD               = 30005,    //密码开门上报
    RECORD_TYPE_BUTTON_OPEN_DOOR_RECORD                 = 30006,    //按钮开门上报
    RECORD_TYPE_BLACKLIST_OPEN_DOOR_RECORD              = 30007,    //黑名单开门上报
    RECORD_TYPE_FACE_OPEN_DOOR_FAIL_RECORD              = 30008,    //人脸验证失败上报
    RECORD_TYPE_REMOTE_OPEN_DOOR_FAIL_RECORD            = 30009,    //远程验证失败上报
    RECORD_TYPE_CPU_CARD_OPEN_DOOR_FAIL_RECORD          = 30010,    //CPU卡验证失败上报
    RECORD_TYPE_QR_CODE_OPEN_DOOR_FAIL_RECORD           = 30011,    //二维码验证失败上报
    RECORD_TYPE_FINGER_OPEN_DOOR_FAIL_RECORD            = 30012,    //指纹验证失败上报
    RECORD_TYPE_PASSWORD_OPEN_DOOR_FAIL_RECORD          = 30013,    //密码验证失败上报
    RECORD_TYPE_DYNAMICPASSWORD_OPEN_DOOR_RECORD        = 30015,    //动态密码开门上报
    RECORD_TYPE_DYNAMICPASSWORD_OPEN_DOOR_FAIL_RECORD   = 30016,    //动态密码开门失败上报
    RECORD_TYPE_NFC_OPEN_DOOR_RECORD                    = 30017,    //NFC开门上报
    RECORD_TYPE_NFC_OPEN_DOOR_FAIL_RECORD               = 30018,    //NFC开门失败上报
    RECORD_TYPE_BHT_OPEN_DOOR_RECORD                    = 30021,    //蓝牙开门成功上报
    RECORD_TYPE_BHT_OPEN_DOOR_FAIL_RECORD               = 30022,    //蓝牙开门失败上报
}MYDEV_RECORD_TYPE;

typedef enum _MYDEV_EVENT_TYPE
{
    //门禁事件类型
    EVENT_TYPE_TAMPER_ALARM                 = 30300,    //防拆报警
    EVENT_TYPE_OPEN_DOOR_TIMEOUT_ALARM      = 30301,    //开门超时报警
    EVENT_TYPE_OPEN_DOOR_ABNORMAL_ALARM     = 30302,    //异常开门报警
    EVENT_TYPE_DANGLE_AFTER_ALARM           = 30303,    //尾随报警
    EVENT_TYPE_DEVICE_FAILURE_ALARM         = 30304,    //设备故障报警
    EVENT_TYPE_BOUN_ZONE_ALARM              = 30305,    //防区报警
    EVENT_TYPE_BOUN_ZONE_FAULT              = 30306,    //防区故障
    //通道闸事件类型
    EVENT_TYPE_REVERSE_ENTRY_ALARM          = 30308,    //反向闯入报警
    EVENT_TYPE_STAY_ALARM                   = 30309,    //逗留报警
    EVENT_TYPE_MISTAKE_ENTRY_ALARM          = 30310,    //误闯报警
    EVENT_TYPE_OVERTURN_ALARM               = 30311,    //翻越报警
    //周界事件类型
    EVENT_TYPE_PERIMETER_ZONE_ALARM         = 91001,    //周界防区报警
    EVENT_TYPE_PERIMETER_ZONE_GUARD         = 91101,    //周界防区布防
    EVENT_TYPE_PERIMETER_ZONE_UNGUARD       = 91102,    //周界防区撤防
    EVENT_TYPE_PERIMETER_SUBCHAN_CANCEL     = 91100,    //周界子系统消警
    EVENT_TYPE_PERIMETER_ZONE_RECOVER       = 91103,    //周界防区恢复
    //指路牌事件类型
    EVENT_TYPE_VISITOR_FACE_IDENTIFY        = 94001,    //指路牌人脸识别事件
    //设备管理
    EVENT_TYPE_DEVICE_UPGRADE_FAULT         = 90013,    //上报固件升级异常事件
}MYDEV_EVENT_TYPE;

typedef enum _MYDEV_PARKING_BARRIER_GATE_DEVICE_STATUS
{
    PARKING_BARRIER_GATE_OPEN		= 1,	// 防拆报警
    PARKING_BARRIER_GATE_CLOSE		= 2,	// 开门超时报警
    PARKING_BARRIER_GATE_RUNING	    = 3,	// 异常开门报警
}MYDEV_PARKING_BARRIER_GATE_DEVICE_STATUS;

typedef enum _MYDEV_PARKING_FLOOR_SENSOR_DEVICE_STATUS
{
    PARKING_FLOOR_SENSOR_HAVE_CAR	= 0,	// 有车
    PARKING_FLOOR_SENSOR_NO_CAR		= 1,	// 无车
}MYDEV_PARKING_FLOOR_SENSOR_DEVICE_STATUS;

typedef enum _MYDEV_PARKING_LOCK_CONTROLLER_DEVICE_STATUS
{
	PARKING_LOCK_CONTROLLER_OFF_LINE	= 0,	// 控制器离线
	PARKING_LOCK_CONTROLLER_ON_LINE		= 1,	// 控制器在线
}MYDEV_PARKING_LOCK_CONTROLLER_DEVICE_STATUS;

typedef enum _MYDEV_PARKING_LOCK_DEVICE_STATUS
{
    PARKING_LOCK_RISE_LOCK		= 0,	// 升锁状态
    PARKING_LOCK_DROP_LOCK		= 1,	// 降锁状态（仅不带车辆探测功能的车位锁使用）
    PARKING_LOCK_ABNORMAL		= 2,	// 异常状态
    PARKING_LOCK_DROP_OCCUPIED	= 3,	// 降锁状态，且车位已停车（带车辆探测功能的车位锁使用）
    PARKING_LOCK_DROP_VACANT	= 4,	// 降锁状态，且车位无车（带车辆探测功能的车位锁使用）
    PARKING_LOCK_ON_LINE		= 5,	// 车位锁在线
    PARKING_LOCK_OFF_LINE		= 6,	// 车位锁离线
}MYDEV_PARKING_LOCK_DEVICE_STATUS;

typedef enum _MYDEV_INFORMATION_SCREEN_DEVICE_STATUS
{
    INFORMATION_SCREEN_ON_LINE		= 0,	//在线
    INFORMATION_SCREEN_OFF_LINE		= 1,	//离线
}MYDEV_INFORMATION_SCREEN_DEVICE_STATUS;

typedef enum _MYDEV_ELECTRIC_FENCE_DEVICE_STATUS
{
    ELECTRIC_FENCE_ON_LINE		= 0,	// 子设备脉冲主机在线
    ELECTRIC_FENCE_OFF_LINE		= 1,	// 子设备脉冲主机离线
}MYDEV_ELECTRIC_FENCE_DEVICE_STATUS;

typedef enum _MYDEV_GATE_OPEN_MODE
{
    GATE_OPEN_MODE_AUTO			= 1,			//自动开闸
    GATE_OPEN_MODE_CONFIRM		= 2,			//确认开闸
    GATE_OPEN_MODE_MANUAL		= 3,			//手动开闸
    GATE_OPEN_MODE_OFFLINE		= 4,			//脱机开闸
}MYDEV_GATE_OPEN_MODE;

typedef enum _MYDEV_DEVICE_ENTRY_TYPE
{
    DEVICE_ENTRY_TYPE_BIG_ENTER = 1,            //大车场入口
    DEVICE_ENTRY_TYPE_BIG_EXIT = 2,             //大车场出口
    DEVICE_ENTRY_TYPE_SMALL_ENTER = 3,          //小车场入口
    DEVICE_ENTRY_TYPE_SMALL_EXIT = 4,           //小车场出口
    DEVICE_ENTRY_TYPE_CENTER_CHARGE = 5,        //中央收费
    DEVICE_ENTRY_TYPE_CENTER_CHARGE_EXIT = 6,   // 中央收费出口
}MYDEV_DEVICE_ENTRY_TYPE;

typedef enum _MYDEV_FAC_DEVICE_STATE
{
    FAC_DEVICE_STATE_RUN = 1,                   //正常运行
    FAC_DEVICE_STATE_FAULT = 2,                 //故障状态
}MYDEV_FAC_DEVICE_STATE;

typedef enum _MYDEV_FAC_DIRECTION
{
    FAC_DIRECTION_STOP = 0,                     //停
    FAC_DIRECTION_UP = 1,                       //上
    FAC_DIRECTION_DOWN = 2,                     //下
}MYDEV_FAC_DIRECTION;

typedef enum _MYDEV_FAC_BA_ERROR_STATUS
{
    FAC_BA_ERROR_STATUS_OK = 0,                     //无故障
    FAC_BA_ERROR_STATUS_FAULT = 1,                  //有故障
}MYDEV_FAC_BA_ERROR_STATUS;

typedef enum _MYDEV_PAK_INTERCOM_CONTROL_COMMAND_TYPE
{
    PAK_INTERCOM_CONTROL_COMMAND_TYPE_REQ = 1,              //请求对讲
    PAK_INTERCOM_CONTROL_COMMAND_TYPE_ACK = 2,              //应答对讲
    PAK_INTERCOM_CONTROL_COMMAND_TYPE_END = 3,              //结束/拒绝对讲
}MYDEV_PAK_INTERCOM_CONTROL_COMMAND_TYPE;

typedef enum _MYDEV_FAC_BA_FIRE_CTRL_STATUS
{
    FAC_BA_FIRE_CTRL_STATUS_NORMAL = 0,             //非消防状态
    FAC_BA_FIRE_CTRL_STATUS_ONFIRE = 1,             //消防状态
}MYDEV_FAC_BA_FIRE_CTRL_STATUS;

typedef struct _user_certificate_info
{
    struct list_head node;
    int dev_type;
    char device_id[128];            //id完整字符串
    char start_time[128];
    char end_time[128];
    int user_type;
    int credence_type;
    char credence_no[128];
    char user_name[128];
    char user_id[128];
    char op_time[128];
    int auth_floor_num;
    int *auth_floor;
    char place_no[32];
    char place_lock_no[32];
}user_certificate_info;

typedef struct _user_dev_info
{
    int dev_handle;                 //设备handle
    int status;                     //设备运行状态
    int magic_num;                  //设备唯一性编码
    int updated;                    //设备是否已升级
    int update_delay;               //设备升级延迟时间
    struct list_head node;          //设备链表节点
    egsc_dev_info dev_info;         //设备信息

    void *status_cb_func;           //设备状态回调接口
    void *srv_req_cb_tbl;           //服务器请求命令入口
    void *dev_req_if_tbl;           //设备请求命令入口
}user_dev_info;

int mydev_init();
int mydev_init_V2();
int mydev_uninit();
int mydev_create();
int mydev_delete();
int mydev_start();
int mydev_stop();
int my_dev_single_init(EGSC_DEV_TYPE dev_type, int dev_offset);
void main_process_loop(unsigned int *dev_arr, int arr_size);
void Child_process_loop(user_dev_info *user_dev,int dev_offset);
#endif

