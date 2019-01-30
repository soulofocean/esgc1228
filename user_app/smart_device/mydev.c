#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "mydev_json.h"
#include <door_ctrl/egsc_door_ctrl.h>
#include <gate_ctrl/egsc_gate_ctrl.h>
#include <elec_lpn/egsc_elec_lpn.h>
#include <parking_lock/egsc_parking_lock.h>
#include <smart_ctrl/egsc_smart_ctrl.h>
#include <elevator/egsc_elevator.h>
#include <parking_ctrl/egsc_parking_ctrl.h>
#include <screen_ctrl/egsc_screen_ctrl.h>
#include "mydev.h"

#include <sys/types.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <sys/wait.h>  
#include <unistd.h>  
#include <arpa/inet.h>  
#include <sys/time.h> 

#include "myMQ.h"
#include "myProtocol.h"

#define EGSC_SUBDEV_NUM     1
#define MYDEV_SOFT_VERSION              "0.1.0"
#define DEVICE_MAGIC_NUM_BASE           (31530)

#define MAXBUF 1024  
#define SOCKET_SERVER_PORT 8888
#define SOCKET_SERVER_LISNUM 2


static int s_mydev_req_id = 11111;
static int s_mydev_dev_magic_num;           //设备唯一MAGIC数字

mydev_json_obj s_device_config_obj = NULL;  //设备用户配置信息

static int s_test_task_id = -1;             //用户输入线程ID
static int s_misc_task_id = -1;             //杂散业务线程ID
static int new_fd;							//Socket套接字ID

egsc_subdev_info s_subdev_info[EGSC_SUBDEV_NUM] = {0};
static struct list_head s_mydev_cert_list_head;
static struct list_head s_mydev_dev_list_head;

static egsc_dev_status_callback s_door_ctrl_status_cb;
static egsc_door_ctrl_cb_tbl s_door_ctrl_srv_req_cb_tbl;
static egsc_door_ctrl_if_tbl s_door_ctrl_req_if_tbl;

static egsc_dev_status_callback s_gate_ctrl_status_cb;
static egsc_gate_ctrl_cb_tbl s_gate_ctrl_srv_req_cb_tbl;
static egsc_gate_ctrl_if_tbl s_gate_ctrl_req_if_tbl;

static egsc_dev_status_callback s_elec_lpn_status_cb;
static egsc_elec_lpn_cb_tbl s_elec_lpn_srv_req_cb_tbl;
static egsc_elec_lpn_if_tbl s_elec_lpn_req_if_tbl;

static egsc_dev_status_callback s_parking_lock_status_cb;
static egsc_parking_lock_cb_tbl s_parking_lock_srv_req_cb_tbl;
static egsc_parking_lock_if_tbl s_parking_lock_req_if_tbl;

static egsc_dev_status_callback s_smart_ctrl_status_cb;
static egsc_smart_ctrl_cb_tbl s_smart_ctrl_srv_req_cb_tbl;
static egsc_smart_ctrl_if_tbl s_smart_ctrl_req_if_tbl;

static egsc_dev_status_callback s_elevator_status_cb;
static egsc_elevator_cb_tbl s_elevator_srv_req_cb_tbl;
static egsc_elevator_if_tbl s_elevator_req_if_tbl;

static egsc_dev_status_callback s_parking_ctrl_status_cb;
static egsc_parking_ctrl_cb_tbl s_parking_ctrl_srv_req_cb_tbl;
static egsc_parking_ctrl_if_tbl s_parking_ctrl_req_if_tbl;

static egsc_dev_status_callback s_screen_ctrl_status_cb;
static egsc_screen_ctrl_cb_tbl s_screen_ctrl_srv_req_cb_tbl;
static egsc_screen_ctrl_if_tbl s_screen_ctrl_req_if_tbl;

#define TEST_HENGDA_TEST_SERVER_ADDR    "10.101.70.247:20001"
#define TEST_HENGDA_SERVER_ADDR         "10.101.70.155:20001"
#define TEST_HUQIAO_SIMU_SERVER_ADDR    "192.168.186.1:5100"
#define TEST_SIMU_DEV_ID                "159357159357"
#define TEST_SIMU_SUBDEV_ID             "159357357159"

#define TEST_DEVICE_CONFIG_FILE_NAME    "./device_config"
#define TEST_CERT_FILE_NAME             "./certificate_file"
#define TEST_PARAM_FILE_NAME            "./parameters_file"

static int user_certificate_search(struct list_head *head, char *subdevice_id, char *credence_no);
static int mydev_create_single(user_dev_info *user_dev);

//设备参数区域
/*设置参数*/
mydev_json_obj s_mydev_params_obj = NULL;

/*设备状态*/
static int s_mydev_status = 0;

/*批量参数*/
static char s_batch_cert_file_id[128];
static char s_batch_cert_op_time[128];

/*开关门参数*/
static int s_gate_optype = 0;           // 闸门开关指令

/*声音参数*/
static int s_level1 = 0;                // 设定时段声音级别
static int s_level2 = 0;                // 非设定时段声音级别
static char s_start_time[128] = "";     // 开始时间 HH:mm
static char s_end_time[128] = "";       // 结束时间 HH:mm

/*车位锁设备升起或降下参数*/
static int s_lock_optype = 0;           // 车位锁设备升起或降下参数

/*电子车位显示屏显示信息参数*/
static char s_showinfo[256];            // 显示屏需要显示的文字

/*设备防区布防信息参数*/
static int s_setup_type;                    // 设置类型
static char s_alarmzone_chan[256];          // 防区通道标记
static int s_subsystem_num;                 // 子系统号

static int user_check_is_num(char c)
{
    if( ('0' <= c) &&
        ('9' >= c))
    {
        return 1;
    }

    return -1;
}

static int user_check_vendor_num(int vendor_num)
{
    int valid = -1;
    switch(vendor_num)
    {
        case EGSC_VENDOR_NUM_HIKVISION:     // 海康
        case EGSC_VENDOR_NUM_DAHUA:         // 大华
        case EGSC_VENDOR_NUM_JIESHUN:       // 捷顺
        case EGSC_VENDOR_NUM_ANJUBAO:       // 安居宝
        case EGSC_VENDOR_NUM_LEELEN:        // 立林
        case EGSC_VENDOR_NUM_IBM:           // IBM
        case EGSC_VENDOR_NUM_HONEYWELL:     // 霍尼韦尔
        case EGSC_VENDOR_NUM_EVERGRANDE:    // 恒大
        case EGSC_VENDOR_NUM_HITACHI:       // 日立电梯
        case EGSC_VENDOR_NUM_QLDT:          // 三菱电梯
        case EGSC_VENDOR_NUM_OTIS:          // 奥的斯电梯
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_device_type(int device_type)
{
    int valid = -1;
    switch(device_type)
    {
        case EGSC_TYPE_CAMERA:                      // IPC枪机
        case EGSC_TYPE_EAGLEEYE_CAM:                // 鹰眼摄像机
        case EGSC_TYPE_BALLHEAD_CAM:                // 球机
        case EGSC_TYPE_FACE_CAP_CAM:                // 人脸抓拍机
        case EGSC_TYPE_PARKING_CTRL:                // 停车场控制器
        case EGSC_TYPE_PARK_LPN_ID:                 // 停车场车牌识别仪
        case EGSC_TYPE_DOOR_CTRL:                   // 门禁控制器
        case EGSC_TYPE_GATE_CTRL:                   // 人行通道控制器
        case EGSC_TYPE_ENTRA_MACHINE:               // 门口机
        case EGSC_TYPE_FENCE_MACHINE:               // 围墙机
        case EGSC_TYPE_INDOOR_MACHINE:              // 室内机
        case EGSC_TYPE_MGMT_MACHINE:                // 管理机
        case EGSC_TYPE_ELE_LINK_CTRL:               // 电梯联动控制器
        case EGSC_TYPE_PATROL_DEV:                  // 巡更设备
        case EGSC_TYPE_SCREEN_CTRL:                 // 信息发布屏控制器
        case EGSC_TYPE_BROADCAST_CTRL:              // 广播控制器
        case EGSC_TYPE_ELEVATOR_CTRL:               // 电梯厂商控制器
        case EGSC_TYPE_SMART_CTRL_KB:               // 智能控制终端
        case EGSC_TYPE_CARPARK_CAM:                 // 车位检测相机
        case EGSC_TYPE_ELEC_LPN_CTRL:               // 电子车位控制器
        case EGSC_TYPE_PARKING_LOCK_CONTROLLER:     // 车位锁控制器
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_device_id(char *id)
{
    if((NULL != id) &&
        (strlen(id) == 12))
    {
        return 1;
    }

    return -1;
}

static int user_check_subdevice_type(int dev_type, int subdevice_type)
{
    int valid = -1;
    switch(dev_type)
    {
        case EGSC_TYPE_PARKING_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_PARKING_CTRL  = 2005, // 停车场控制器
                case EGSC_SUBTYPE_PARKING_SPACE_DISPLAY:                        // 门口显示剩余车位数大屏
                case EGSC_SUBTYPE_PARKING_BARRIER_GATE:                         // 停车场道闸
                case EGSC_SUBTYPE_PARKING_FLOOR_SENSOR:                         // 停车场地感
                case EGSC_SUBTYPE_PARKING_CPU_CARD_READER:                      // CPU读头
                case EGSC_SUBTYPE_PARKING_QR_READER:                            // 二维码读头
                case EGSC_SUBTYPE_PARKING_RFID_READER:                          // RFID读头
                case EGSC_SUBTYPE_PARKING_IC_CARD_READER:                       // 停车场IC读头
                case EGSC_SUBTYPE_PARKING_ID_CARD_READER:                       // 停车场ID读头
                case EGSC_SUBTYPE_PARKING_TICKET_BOX:                           // 停车场票箱
                case EGSC_SUBTYPE_PARKING_CARD_BOX:                             // 停车场卡箱
                case EGSC_SUBTYPE_PARKING_LCD_DISPLAY:                          // 停车场LCD显示屏
                case EGSC_SUBTYPE_PARKING_LED_DISPLAY:                          // 停车场LED显示屏
                case EGSC_SUBTYPE_PARKING_INTERCOM:                             // 停车场对讲
                case EGSC_SUBTYPE_PARKING_SPEAKER:                              // 停车场语音喇叭
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSC_TYPE_DOOR_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_DOOR_CTRL     = 2009, // 门禁控制器
                case EGSC_SUBTYPE_DOOR_READER:                                  // 门禁读头
                case EGSC_SUBTYPE_DOOR_FACE_READER:                             // 门禁人脸读卡器
                case EGSC_SUBTYPE_DOOR_FINGER_READER:                           // 门禁指纹识别读卡器
                case EGSC_SUBTYPE_DOOR_QR_READER:                               // 门禁二维码读卡器
                case EGSC_SUBTYPE_DOOR_BLUETOOTH_READER:                        // 门禁蓝牙读卡器
                case EGSC_SUBTYPE_DOOR_PASSWORD_KEYBOARD:                       // 门禁密码输入键盘
                case EGSC_SUBTYPE_DOOR_IC_CARD_READER:                          // 门禁IC读头
                case EGSC_SUBTYPE_DOOR_CPU_CARD_READER:                         // 门禁CPU读头
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSC_TYPE_GATE_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_GATE_CTRL     = 2010, // 人行通道控制器
                case EGSC_SUBTYPE_GATE_MACHINE:                                 // 人证读卡器（闸机）
                case EGSC_SUBTYPE_GATE_IQR_READER:                              // 二维码读卡器（闸机）
                case EGSC_SUBTYPE_GATE_IC_CARD_READER:                          // IC卡读卡器（闸机）
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSC_TYPE_ELE_LINK_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_ELE_LINK_CTRL = 2016, // 电梯联动控制器
                case EGSC_SUBTYPE_ELEVATOR_SUB_CONTROLLER:                      // 电梯联动控制子设备（电梯联动控制器里的虚拟子设备）
                case EGSC_SUBTYPE_ELEVATOR_CAR:                                 // 电梯轿厢
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSC_TYPE_SCREEN_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_SCREEN_CTRL   = 2018, // 信息发布屏控制器
                case EGSC_SUBTYPE_INFORMATION_SCREEN:                           // 信息发布屏
                case EGSC_SUBTYPE_INFORMATION_LED_SCREEN:                       // 信息LED大屏
                case EGSC_SUBTYPE_INFORMATION_LCD_SCREEN:                       // 信息LCD大屏
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_ELEVATOR_CTRL = 2020, // 电梯厂商控制器
                case EGSC_SUBTYPE_ELEVATOR_IC_CARD_READER:                      // 电梯IC卡读头
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSC_TYPE_SMART_CTRL_KB:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_SMART_CTRL_KB = 2021, // 智能控制终端
                case EGSC_SUBTYPE_ELECTRIC_FENCE:                               // 电子围栏
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSC_TYPE_ELEC_LPN_CTRL:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_ELEC_LPN_CTRL = 2023, // 电子车位控制器
                case EGSC_SUBTYPE_ELECTRIC_LPN_DISPLAY:                         // 电子车位显示屏
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        case EGSC_TYPE_PARKING_LOCK_CONTROLLER:
        {
            switch(subdevice_type)
            {
                //EGSC_TYPE_PARKING_LOCK_CONTROLLER = 2025, // 车位锁控制器
                case EGSC_SUBTYPE_PARKING_LOCK:                                 // 车位锁
                {
                    valid = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_credence_type(int credence_type)
{
    int valid = -1;
    switch(credence_type)
    {
        case CREDENCE_TYPE_BUTTON:          //按钮（协议文档当前未定义，临时添加）
        case CREDENCE_TYPE_EM:              //ID卡
        case CREDENCE_TYPE_MF1_A:           //IC卡，TypeA 标准
        case CREDENCE_TYPE_MF1_B:           //IC卡，TypeB 标准
        case CREDENCE_TYPE_CPU:             //CPU 卡
        case CREDENCE_TYPE_CAR_PLATE:       //车牌
        case CREDENCE_TYPE_FINGER_PRINT:    //指纹
        case CREDENCE_TYPE_FACE:            //人脸
        case CREDENCE_TYPE_TICKET:          //纸票/二维码
        case CREDENCE_TYPE_BLUETOOTH:       //蓝牙
        case CREDENCE_TYPE_SECRET:          //固定密码
        case CREDENCE_TYPE_DYNAMICPASSWORD: //动态密码
        case CREDENCE_TYPE_NFC:             //NFC
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_user_type(int user_type)
{
    int valid = -1;
    switch(user_type)
    {
        case USER_TYPE_MONTH_A:     //固定用户A (物业管理人员)
        case USER_TYPE_MONTH_B:     //固定用户B（业主）
        case USER_TYPE_MONTH_C:     //固定用户C（其他工作人员）
        case USER_TYPE_MONTH_D:     //固定用户D
        case USER_TYPE_TEMP_A:      //临时用户A
        case USER_TYPE_TEMP_B:      //临时用户B
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_record_type(int record_type)
{
    int valid = -1;
    switch(record_type)
    {
        case RECORD_TYPE_BURSH_CARD_OPEN_PARK_RECORD:           //刷卡开闸事件上报
        case RECORD_TYPE_CARNO_OPEN_PARK_RECORD:                //车牌开闸事件上报
        case RECORD_TYPE_REMOTE_OPEN_PARK_RECORD:               //车场远程开闸事件上报
        case RECORD_TYPE_MANUAL_OPEN_PARK_RECORD:               //手动开闸事件上报
        case RECORD_TYPE_FACE_OPEN_DOOR_RECORD:                 //人脸开门上报
        case RECORD_TYPE_REMOTE_OPEN_DOOR_RECORD:               //远程开门上报
        case RECORD_TYPE_BURSH_CARD_OPEN_DOOR_RECORD:           //刷卡开门上报
        case RECORD_TYPE_QR_CODE_OPEN_DOOR_RECORD:              //二维码开门上报
        case RECORD_TYPE_FINGER_OPEN_DOOR_RECORD:               //指纹开门上报
        case RECORD_TYPE_PASSWORD_OPEN_DOOR_RECORD:             //密码开门上报
        case RECORD_TYPE_BUTTON_OPEN_DOOR_RECORD:               //按钮开门上报
        case RECORD_TYPE_BLACKLIST_OPEN_DOOR_RECORD:            //黑名单开门上报
        case RECORD_TYPE_FACE_OPEN_DOOR_FAIL_RECORD:            //人脸验证失败上报
        case RECORD_TYPE_REMOTE_OPEN_DOOR_FAIL_RECORD:          //远程验证失败上报
        case RECORD_TYPE_CPU_CARD_OPEN_DOOR_FAIL_RECORD:        //CPU卡验证失败上报
        case RECORD_TYPE_QR_CODE_OPEN_DOOR_FAIL_RECORD:         //二维码验证失败上报
        case RECORD_TYPE_FINGER_OPEN_DOOR_FAIL_RECORD:          //指纹验证失败上报
        case RECORD_TYPE_PASSWORD_OPEN_DOOR_FAIL_RECORD:        //密码验证失败上报
        case RECORD_TYPE_DYNAMICPASSWORD_OPEN_DOOR_RECORD:      //动态密码开门上报
        case RECORD_TYPE_DYNAMICPASSWORD_OPEN_DOOR_FAIL_RECORD: //动态密码开门失败上报
        case RECORD_TYPE_NFC_OPEN_DOOR_RECORD:                  //NFC开门上报
        case RECORD_TYPE_NFC_OPEN_DOOR_FAIL_RECORD:             //NFC开门失败上报
        case RECORD_TYPE_BHT_OPEN_DOOR_RECORD:                  //蓝牙开门成功上报
        case RECORD_TYPE_BHT_OPEN_DOOR_FAIL_RECORD:             //蓝牙开门失败上报
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_door_ctrl_event_type(int event_type)
{
    int valid = -1;
    switch(event_type)
    {
        case EVENT_TYPE_TAMPER_ALARM:                   //防拆报警
        case EVENT_TYPE_OPEN_DOOR_TIMEOUT_ALARM:        //开门超时报警
        case EVENT_TYPE_OPEN_DOOR_ABNORMAL_ALARM:       //异常开门报警
        case EVENT_TYPE_DANGLE_AFTER_ALARM:             //尾随报警
        case EVENT_TYPE_DEVICE_FAILURE_ALARM:           //设备故障报警
        case EVENT_TYPE_BOUN_ZONE_ALARM:                //防区报警
        case EVENT_TYPE_BOUN_ZONE_FAULT:                //防区故障
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_smart_ctrl_event_type(int event_type)
{
    int valid = -1;
    switch(event_type)
    {
        case EVENT_TYPE_PERIMETER_ZONE_ALARM:       //周界防区报警
        case EVENT_TYPE_PERIMETER_ZONE_GUARD:       //周界防区布防
        case EVENT_TYPE_PERIMETER_ZONE_UNGUARD:     //周界防区撤防
        case EVENT_TYPE_PERIMETER_SUBCHAN_CANCEL:   //周界子系统消警
        case EVENT_TYPE_PERIMETER_ZONE_RECOVER:     //周界防区恢复
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_common_event_type(int event_type)
{
    int valid = -1;
    switch(event_type)
    {
        case EVENT_TYPE_DEVICE_UPGRADE_FAULT:       //上报固件升级异常事件
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_dev_device_type(user_dev_info *user_dev, int device_type)
{
    int valid = -1;
    int loop = 0;
    egsc_subdev_info *sub_devinfo = NULL;

    if(NULL == user_dev)
    {
        return valid;
    }

    if( user_dev->dev_info.dev_type == device_type )
    {
        return 1;
    }
    else if(user_dev->dev_info.subdev_count > 0)
    {
        for(loop=0; loop<user_dev->dev_info.subdev_count; loop++)
        {
            sub_devinfo = user_dev->dev_info.subdev_info+loop;
            if( sub_devinfo->subdev_id.subdev_type == device_type )
            {
                return 1;
            }
        }
    }

    return valid;
}

static int user_check_optime_valid(char *time)
{
    int loop = 0;
    char cur_time[128];
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if(NULL == time)
    {
        return -1;
    }

    memset(cur_time, 0, sizeof(cur_time));
    egsc_platform_time_get(NULL, cur_time, sizeof(cur_time));
    if(strlen(time) != 19)
    {
        return -1;
    }
    if((user_check_is_num(*(time+0))>=0) &&
        (user_check_is_num(*(time+1))>=0) &&
        (user_check_is_num(*(time+2))>=0) &&
        (user_check_is_num(*(time+3))>=0) &&
        ('-' == *(time+4)) &&
        (user_check_is_num(*(time+5))>=0) &&
        (user_check_is_num(*(time+6))>=0) &&
        ('-' == *(time+7)) &&
        (user_check_is_num(*(time+8))>=0) &&
        (user_check_is_num(*(time+9))>=0) &&
        (' ' == *(time+10)) &&
        (user_check_is_num(*(time+11))>=0) &&
        (user_check_is_num(*(time+12))>=0) &&
        (':' == *(time+13)) &&
        (user_check_is_num(*(time+14))>=0) &&
        (user_check_is_num(*(time+15))>=0) &&
        (':' == *(time+16)) &&
        (user_check_is_num(*(time+17))>=0) &&
        (user_check_is_num(*(time+18))>=0))
    {
        year = (*(time+0)-'0')*1000 + (*(time+1)-'0')*100 + (*(time+2)-'0')*10+ (*(time+3)-'0')*1;
        month = (*(time+5)-'0')*10 + (*(time+6)-'0')*1;
        day = (*(time+8)-'0')*10 + (*(time+9)-'0')*1;
        hour = (*(time+11)-'0')*10 + (*(time+12)-'0')*1;
        minute = (*(time+14)-'0')*10 + (*(time+15)-'0')*1;
        second = (*(time+17)-'0')*10 + (*(time+18)-'0')*1;

        if (((year % 4 == 0) &&
            (year % 100 != 0)) ||
            (year % 400 == 0))
        {
            month_days[1] = 29; 
        }
        if(!((month > 0) &&
            (month <= 12) &&
            (day > 0) &&
            (day <= month_days[month - 1]) &&
            (hour >= 0) &&
            (hour < 24) &&
            (minute >= 0) &&
            (minute < 60) &&
            (second >= 0) &&
            (second < 60)))
        {
            return -1;
        }
        
        for(loop=0;loop<19;loop++)
        {
            if( (4 != loop) &&
                (7 != loop) &&
                (10 != loop) &&
                (13 != loop) &&
                (16 != loop))
            {
                if((*(cur_time+loop) - *(time+loop)) < 0)
                {
                    return -1;
                }
                else if((*(cur_time+loop) - *(time+loop)) > 0)
                {
                    return 1;
                }
            }
        }
        return 1;
    }

    return -1;
}

static int user_check_door_ctrl_pass_type(int pass_type)
{
    if( (0 == pass_type) ||
        (1 == pass_type))
    {
        return 1;
    }

    return -1;
}

static int user_check_parking_floor_sensor_device_status(int device_status)
{
    int valid = -1;
    switch(device_status)
    {
        case PARKING_FLOOR_SENSOR_HAVE_CAR:				// 有车
        case PARKING_FLOOR_SENSOR_NO_CAR:				// 无车
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_parking_barrier_gate_device_status(int device_status)
{
    int valid = -1;
    switch(device_status)
    {
        case PARKING_BARRIER_GATE_OPEN:			    // 防拆报警
        case PARKING_BARRIER_GATE_CLOSE:			// 开门超时报警
        case PARKING_BARRIER_GATE_RUNING:			// 异常开门报警
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_parking_lock_controller_device_status(int device_status)
{
    int valid = -1;
    switch(device_status)
    {
        case PARKING_LOCK_CONTROLLER_OFF_LINE:			//控制器离线
        case PARKING_LOCK_CONTROLLER_ON_LINE:			//控制器在线
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_parking_lock_device_status(int device_status)
{
    int valid = -1;
    switch(device_status)
    {
        case PARKING_LOCK_RISE_LOCK:			// 升锁状态
        case PARKING_LOCK_DROP_LOCK:			// 降锁状态（仅不带车辆探测功能的车位锁使用）
        case PARKING_LOCK_ABNORMAL:				// 异常状态
        case PARKING_LOCK_DROP_OCCUPIED:		// 降锁状态，且车位已停车（带车辆探测功能的车位锁使用）
        case PARKING_LOCK_DROP_VACANT:			// 降锁状态，且车位无车（带车辆探测功能的车位锁使用）
        case PARKING_LOCK_ON_LINE:				// 车位锁在线
        case PARKING_LOCK_OFF_LINE:				// 车位锁离线
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_information_screen_device_status(int device_status)
{
    int valid = -1;
    switch(device_status)
    {
        case INFORMATION_SCREEN_ON_LINE:				//在线
        case INFORMATION_SCREEN_OFF_LINE:				//离线
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_paking_ctrl_gate_open_mode(int open_mode)
{
    int valid = -1;
    switch(open_mode)
    {
        case GATE_OPEN_MODE_AUTO:			//自动开闸
        case GATE_OPEN_MODE_CONFIRM:		//确认开闸
        case GATE_OPEN_MODE_MANUAL:			//手动开闸
        case GATE_OPEN_MODE_OFFLINE:		//脱机开闸
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_paking_ctrl_entry_type(int entry_type)
{
    int valid = -1;
    switch(entry_type)
    {
        case DEVICE_ENTRY_TYPE_BIG_ENTER:				//大车场入口
        case DEVICE_ENTRY_TYPE_BIG_EXIT:				//大车场出口
        case DEVICE_ENTRY_TYPE_SMALL_ENTER:				//小车场入口
        case DEVICE_ENTRY_TYPE_SMALL_EXIT:				//小车场出口
        case DEVICE_ENTRY_TYPE_CENTER_CHARGE:			//中央收费
        case DEVICE_ENTRY_TYPE_CENTER_CHARGE_EXIT:		//中央收费出口
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_fac_work_mode(char *work_mode)
{
    #define FAC_WORK_MODE_NORMAL            "0"             //普通模式
    #define FAC_WORK_MODE_AUTHORIZATION     "1"             //授权模式
    
    int valid = -1;
    if(NULL == work_mode)
    {
        return valid;
    }

    if((strcmp(work_mode, FAC_WORK_MODE_NORMAL) == 0) ||
        (strcmp(work_mode, FAC_WORK_MODE_AUTHORIZATION) == 0))
    {
        valid = 1;
    }

    return valid;
}

static int user_check_fac_device_state(int device_state)
{
    int valid = -1;
    switch(device_state)
    {
        case FAC_DEVICE_STATE_RUN:				//正常运行
        case FAC_DEVICE_STATE_FAULT:	        //故障状态
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_fac_direction(int fac_direction)
{
    int valid = -1;
    switch(fac_direction)
    {
        case FAC_DIRECTION_STOP:				//停
        case FAC_DIRECTION_UP:	                //上
        case FAC_DIRECTION_DOWN:	            //下
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_fac_light_mode(char *light_mode)
{
    #define FAC_LIGHT_MODE_MANUAL       "0"             //手动点亮，凭证校验通过后只授权楼层不点亮
    #define FAC_LIGHT_MODE_AUTO         "1"             //自动点亮，凭证校验通过后自动点亮对应楼层
    
    int valid = -1;
    if(NULL == light_mode)
    {
        return valid;
    }

    if((strcmp(light_mode, FAC_LIGHT_MODE_MANUAL) == 0) ||
        (strcmp(light_mode, FAC_LIGHT_MODE_AUTO) == 0))
    {
        valid = 1;
    }

    return valid;
}

static int user_check_electric_fence_sensor_type(char *sensor_type)
{
    #define ELECTRIC_FENCE_SENSOR_AC_VOLTAGE    "SENSOR_AC_VOLTAGE"     //交流电压
    #define ELECTRIC_FENCE_SENSOR_AC_CURRENT    "SENSOR_AC_CURRENT"     //交流电流
    #define ELECTRIC_FENCE_SENSOR_DC_VOLTAGE    "SENSOR_DC_VOLTAGE"     //直流电压
    #define ELECTRIC_FENCE_SENSOR_DC_CURRENT    "SENSOR_DC_CURRENT"     //直流电流
    
    int valid = -1;
    if(NULL == sensor_type)
    {
        return valid;
    }

    if((strcmp(sensor_type, ELECTRIC_FENCE_SENSOR_AC_VOLTAGE) == 0) ||
        (strcmp(sensor_type, ELECTRIC_FENCE_SENSOR_AC_CURRENT) == 0) ||
        (strcmp(sensor_type, ELECTRIC_FENCE_SENSOR_DC_VOLTAGE) == 0) ||
        (strcmp(sensor_type, ELECTRIC_FENCE_SENSOR_DC_CURRENT) == 0))
    {
        valid = 1;
    }

    return valid;
}

static int user_check_fac_ba_car_status(char *ba_car_status)
{
    #define FAC_BA_CAR_STATUS_STOP          "00"        //停止
    #define FAC_BA_CAR_STATUS_UP_NOW        "01"        //向上运行中
    #define FAC_BA_CAR_STATUS_UP_STOP       "11"        //上行过程的停止
    #define FAC_BA_CAR_STATUS_DOWN_NOW      "02"        //向下运行中
    #define FAC_BA_CAR_STATUS_DOWN_STOP     "22"        //下行过程的停止

    int valid = -1;
    if(NULL == ba_car_status)
    {
        return valid;
    }

    if((strcmp(ba_car_status, FAC_BA_CAR_STATUS_STOP) == 0) ||
        (strcmp(ba_car_status, FAC_BA_CAR_STATUS_UP_NOW) == 0) ||
        (strcmp(ba_car_status, FAC_BA_CAR_STATUS_UP_STOP) == 0) ||
        (strcmp(ba_car_status, FAC_BA_CAR_STATUS_DOWN_NOW) == 0) ||
        (strcmp(ba_car_status, FAC_BA_CAR_STATUS_DOWN_STOP) == 0))
    {
        valid = 1;
    }

    return valid;
}

static int user_check_fac_ba_door_status(char *ba_door_status)
{
    #define FAC_BA_DOOR_STATUS_OPENNING     "10"        //电梯门打开中
    #define FAC_BA_DOOR_STATUS_OPENED       "11"        //电梯门已打开（全开）
    #define FAC_BA_DOOR_STATUS_CLOSING      "12"        //电梯门关闭中
    #define FAC_BA_DOOR_STATUS_CLOSED       "23"        //电梯门已关闭（全关）

    int valid = -1;
    if(NULL == ba_door_status)
    {
        return valid;
    }

    if((strcmp(ba_door_status, FAC_BA_DOOR_STATUS_OPENNING) == 0) ||
        (strcmp(ba_door_status, FAC_BA_DOOR_STATUS_OPENED) == 0) ||
        (strcmp(ba_door_status, FAC_BA_DOOR_STATUS_CLOSING) == 0) ||
        (strcmp(ba_door_status, FAC_BA_DOOR_STATUS_CLOSED) == 0))
    {
        valid = 1;
    }

    return valid;
}

static int user_check_fac_ba_error_status(int error_status)
{
    int valid = -1;
    switch(error_status)
    {
        case FAC_BA_ERROR_STATUS_OK:		    //无故障
        case FAC_BA_ERROR_STATUS_FAULT:	        //有故障
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_fac_ba_fire_ctrl_status(int fire_ctrl_status)
{
    int valid = -1;
    switch(fire_ctrl_status)
    {
        case FAC_BA_FIRE_CTRL_STATUS_NORMAL:	        //非消防状态
        case FAC_BA_FIRE_CTRL_STATUS_ONFIRE:	        //消防状态
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_check_pak_intercom_ctrl_command_type(int command_type)
{
    int valid = -1;
    switch(command_type)
    {
        case PAK_INTERCOM_CONTROL_COMMAND_TYPE_REQ:	        //请求对讲
        case PAK_INTERCOM_CONTROL_COMMAND_TYPE_ACK:	        //应答对讲
        case PAK_INTERCOM_CONTROL_COMMAND_TYPE_END:	        //结束/拒绝对讲
        {
            valid = 1;
            break;
        }
        default:
        {
            break;
        }
    }

    return valid;
}

static int user_dev_enqueue(struct list_head *head, user_dev_info *dev_obj, int *magic_num)
{
    if( (NULL == dev_obj) ||
        (NULL == head))
    {
        egsc_log_error("input head/req_obj NULL.\n");
        return EGSC_RET_ERROR;
    }

    s_mydev_dev_magic_num++;
    dev_obj->magic_num = s_mydev_dev_magic_num;
    list_add(&(dev_obj->node), head);
    if(NULL != magic_num)
    {
        *magic_num = dev_obj->magic_num;
    }

    return EGSC_RET_SUCCESS;
}

static int user_dev_dequeue(struct list_head *head, char *id)
{
    if( (NULL == head) ||
        (NULL == id))
    {
        egsc_log_error("input head/id NULL.\n");
        return EGSC_RET_ERROR;
    }

    user_dev_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( strcmp(pos->dev_info.id, id) == 0 )
        {
            egsc_log_debug("dev(id:%s)(head:0x%x) de queue list success.\n", id, head);
            list_del(&(pos->node));

            return EGSC_RET_SUCCESS;
        }
    }

    egsc_log_error("dev(id:%s) de queue list failed.\n", id);

    return EGSC_RET_ERROR;
}

static int user_dev_get_id(struct list_head *head, char *id, user_dev_info **dev_obj)
{
    int loop = 0;
    egsc_subdev_info *sub_devinfo = NULL;
    user_dev_info *pos, *n;

    if( (NULL == dev_obj) ||
        (NULL == id) ||
        (NULL == head))
    {
        egsc_log_error("input id/head/req_obj NULL.\n");
        return EGSC_RET_ERROR;
    }

    list_for_each_entry_safe(pos,n,head,node)
    {
        if( strcmp(pos->dev_info.id, id) == 0 )
        {
            egsc_log_debug("dev(id:%s)(head:0x%x) de queue list success.\n", id, head);
            *dev_obj = pos;
            return EGSC_RET_SUCCESS;
        }
        else if(pos->dev_info.subdev_count > 0)
        {
            for(loop=0; loop<pos->dev_info.subdev_count; loop++)
            {
                sub_devinfo = pos->dev_info.subdev_info+loop;
                if( strcmp(sub_devinfo->subdev_id.subdev_mac, id) == 0 )
                {
                    egsc_log_debug("dev(id:%s)(head:0x%x) de queue list success.\n", id, head);
                    *dev_obj = pos;
                    return EGSC_RET_SUCCESS;
                }
            }
        }
    }

    egsc_log_error("dev(id:%s) de queue list failed.\n", id);

    return EGSC_RET_ERROR;
}

static int user_dev_get_type(struct list_head *head, EGSC_DEV_TYPE dev_type, user_dev_info **dev_obj)
{
    user_dev_info *pos, *n;

    if( (NULL == dev_obj) ||
        (NULL == head))
    {
        egsc_log_error("input head/req_obj NULL.\n");
        return EGSC_RET_ERROR;
    }

    list_for_each_entry_safe(pos,n,head,node)
    {
        if(pos->dev_info.dev_type == dev_type)
        {
            egsc_log_debug("dev(type:%d)(head:0x%x) de queue list success.\n", dev_type, head);
            *dev_obj = pos;
            return EGSC_RET_SUCCESS;
        }
    }

    egsc_log_error("dev(type:%d) de queue list failed.\n", dev_type);

    return EGSC_RET_ERROR;
}


static int mydev_get_dev_id_str(struct list_head *head, int handle, egsc_subdev_id *sub_id, char *dev_id_str, int id_buff_len)
{
    int loop = 0;
    user_dev_info *pos, *n;
    egsc_subdev_info *sub_devinfo = NULL;

    if( (NULL == dev_id_str) ||
        (0 == id_buff_len) )
    {
        egsc_log_error(" input dev_id_str param NULL\n");
        return EGSC_RET_ERROR;
    }

    list_for_each_entry_safe(pos,n,head,node)
    {
        if( handle == pos->dev_handle )
        {
            if( (NULL != sub_id) &&
                (pos->dev_info.subdev_count > 0))
            {
                for(loop=0; loop<pos->dev_info.subdev_count; loop++)
                {
                    sub_devinfo = pos->dev_info.subdev_info+loop;
                    if( (strcmp(sub_devinfo->subdev_id.subdev_mac, sub_id->subdev_mac) == 0) &&
                        (sub_devinfo->subdev_id.subdev_type == sub_id->subdev_type) &&
                        (sub_devinfo->subdev_id.subdev_num == sub_id->subdev_num))
                    {
                        snprintf(dev_id_str, id_buff_len, "%04d%s%04d", sub_id->subdev_type, sub_id->subdev_mac, sub_id->subdev_num);
                        return EGSC_RET_SUCCESS;
                    }
                }
            }
            else
            {
                snprintf(dev_id_str, id_buff_len, "%04d%04d%s", pos->dev_info.vendor_num, pos->dev_info.dev_type, pos->dev_info.id);
                return EGSC_RET_SUCCESS;
            }
        }
    }

    return EGSC_RET_ERROR;
}

static int mydev_get_subdev_id(char *devid, egsc_subdev_id *subdev_id_p, egsc_subdev_id **p_id)
{
    char tmp_buf[8] = {0};
    int dev_type = 0;

    if((NULL == subdev_id_p) || (NULL == p_id))
    {
        egsc_log_user("mydev_get_subdev_id param NULL\n");
        return EGSC_RET_ERROR;
    }

    memset(tmp_buf, 0x00, sizeof(tmp_buf));
    memcpy(tmp_buf, devid, 4);// get type
    dev_type = atoi(tmp_buf);
    subdev_id_p->subdev_type = dev_type;

    if((1000 < dev_type) && (3000 > dev_type))// master device ID
    {
        memcpy(subdev_id_p->subdev_mac, devid+4+4, 12);
        memcpy(tmp_buf, devid+4, 4);
        subdev_id_p->subdev_num = atoi(tmp_buf);
        *p_id = NULL;
    }
    else if((3000 < dev_type) && (4000 > dev_type)) // sub device ID
    {
        memcpy(subdev_id_p->subdev_mac, devid+4, 12);
        memcpy(tmp_buf, devid+4+12, 4);
        subdev_id_p->subdev_num = atoi(tmp_buf);
        *p_id = subdev_id_p;
    }
    else
    {
        egsc_log_user("dev_type[%d] error\n", dev_type);
        return EGSC_RET_ERROR;
    }

    return EGSC_RET_SUCCESS;
}

static int user_dev_get_handle(struct list_head *head, int handle, user_dev_info **dev_obj)
{
    if( (NULL == dev_obj) ||
        (NULL == head))
    {
        egsc_log_error("input id/head/req_obj NULL.\n");
        return EGSC_RET_ERROR;
    }

    user_dev_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( handle == pos->dev_handle )
        {
            egsc_log_debug("dev(handle:%d)(head:0x%x) de queue list success.\n", handle, head);
            *dev_obj = pos;
            return EGSC_RET_SUCCESS;
        }
    }

    egsc_log_error("dev(handle:%d) de queue list failed.\n", handle);

    return EGSC_RET_ERROR;
}

static int user_dev_clear(struct list_head *head)
{
    if(NULL == head)
    {
        egsc_log_error("input head/device manager NULL.\n");
        return EGSC_RET_ERROR;
    }

    user_dev_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        egsc_log_debug("magic_num(%s) clear dev queue success.\n", pos->magic_num);
        list_del(&(pos->node));
        if(NULL != pos->dev_info.subdev_info)
        {
            egsc_platform_free(pos->dev_info.subdev_info);
        }
        egsc_platform_free(pos);
    }

    return EGSC_RET_SUCCESS;
}

static int user_certificate_enqueue(struct list_head *head, char *device_id, char *credence_no, user_certificate_info *cert_obj)
{
    if( (NULL == cert_obj) ||
        (NULL == head))
    {
        egsc_log_error("input head/req_obj NULL.\n");
        return EGSC_RET_ERROR;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( (strcmp(pos->device_id, device_id) == 0)&&
            (strcmp(pos->credence_no, credence_no) == 0))
        {
            egsc_log_error("device_id(%s) credence_no(%s) already in queue, new en queue failed.\n", device_id, credence_no);
            return EGSC_RET_ERROR;
        }
    }
    list_add(&(cert_obj->node), head);

    egsc_log_info("device_id(%s) credence_no(%s) (head:%p)add list success \n", device_id, credence_no, head);

    return EGSC_RET_SUCCESS;
}

static int user_certificate_dequeue(struct list_head *head, char *device_id, char *credence_no, user_certificate_info **cert_obj)
{
    if( (NULL == cert_obj) ||
        (NULL == head))
    {
        egsc_log_error("input head/req_obj NULL.\n");
        return EGSC_RET_ERROR;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( strcmp(pos->credence_no, credence_no) == 0)
        {
            egsc_log_debug("device_id(%s) credence_no(%s)(head:0x%x) de queue list success.\n", device_id, credence_no, head);
            *cert_obj = pos;
            list_del(&(pos->node));

            return EGSC_RET_SUCCESS;
        }
    }

    egsc_log_error("device_id(%s) credence_no(%s) de queue list failed.\n", device_id, credence_no);
    return EGSC_RET_ERROR;
}

static int user_file_load_device_config()
{
    int index = 0;
    int error_cnt = 0;
    int subdev_cnt = 0;
    int ch = 0;
    int loop = 0;
    int conf_file_size = 0;
    int comment_start = 0;
    int comment_end = 0;
    int single_comment = 0;
    int multi_comment = 0;
    int array_size = 0;
    int array_index = 0;
    int valid_dev_cnt = 0;
    int valid_subdev_cnt = 0;
    user_dev_info *user_dev = NULL;
    mydev_json_obj dev_item_obj = NULL;
    mydev_json_obj subdev_array_obj = NULL;
    mydev_json_obj array_item_obj = NULL;
    char *param_buff = NULL;
    char dev_name[128] = "";
    FILE *fd_conf = NULL;                   //配置文件描述符
    egsc_subdev_info *subdev_info;

    egsc_log_debug("load device config start.\n");
    egsc_log_user("load device config file(%s).\n", TEST_DEVICE_CONFIG_FILE_NAME);
    fd_conf = fopen(TEST_DEVICE_CONFIG_FILE_NAME,"r");
    if(NULL == fd_conf)
    {
        egsc_log_error("parameters file open failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        egsc_log_user("no found device config file(%s), door will exit.\n", TEST_DEVICE_CONFIG_FILE_NAME);
        return EGSC_RET_ERROR;
    }

    fseek(fd_conf,0L,SEEK_END);
    conf_file_size = ftell(fd_conf);

    param_buff = (char *)egsc_platform_malloc(conf_file_size);
    if(NULL == param_buff)
    {
        egsc_log_error("malloc  failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        egsc_log_user("parser device config file(%s) failed, exit.\n", TEST_DEVICE_CONFIG_FILE_NAME);
        fclose(fd_conf);
        return EGSC_RET_ERROR;
    }
    memset(param_buff, 0, conf_file_size);

    fseek(fd_conf,0L,SEEK_SET);
    while (1)
    {
        ch = fgetc(fd_conf);
        if (EOF == ch)
        {
            break;
        }
        if (' ' == ch)
        {
            continue;
        }

        if (('/' == ch) && (0 == comment_start))
        {
            comment_start = 1;
            continue;
        }

        if ((1 == comment_start) &&
            (0 == single_comment) &&
            (0 == multi_comment))
        {
            if ('/' == ch)
            {
                single_comment = 1;
            }
            else if ('*' == ch)
            {
                multi_comment = 1;
            }
            else
            {
                continue;
            }
        }

        if (1 == single_comment)
        {
            if ('\n' == ch)
            {
                comment_start = 0;
                single_comment = 0;
            }
            continue;
        }

        if (1 == multi_comment)
        {
            if (('*' == ch) && (0 == comment_end))
            {
                comment_end = 1;
            }
            if (('/' == ch) && (1 == comment_end))
            {
                comment_start = 0;
                multi_comment = 0;
            }
            continue;
        }

        if ('\n' == ch)
        {
            continue;
        }
        param_buff[index++] = ch;
    }

    param_buff[index] = '\0';
    egsc_log_debug("dev config(%s).\n", param_buff);

    s_device_config_obj = mydev_json_parse(param_buff);
    if(NULL == s_device_config_obj)
    {
        egsc_log_info("parser failed, skip(%s).\n", param_buff);
    }
    egsc_platform_free(param_buff);

    mydev_json_get_array_size(s_device_config_obj, &array_size);
    for(array_index=0;array_index<array_size;array_index++)
    {
        dev_item_obj = mydev_json_get_array_item(s_device_config_obj,array_index);
        if(NULL != dev_item_obj)
        {
            memset(dev_name, 0, sizeof(dev_name));
            mydev_json_get_string(dev_item_obj, "name", dev_name, sizeof(dev_name));

            user_dev = (user_dev_info *)egsc_platform_malloc(sizeof(user_dev_info));
            if(NULL == user_dev)
            {
                egsc_log_info("malloc failed, skip parser dev(name:%s).\n", dev_name);
                continue;
            }
            memset(user_dev, 0, sizeof(user_dev_info));
            mydev_json_get_string(dev_item_obj, "server_addr", user_dev->dev_info.srv_addr, sizeof(user_dev->dev_info.srv_addr));
            mydev_json_get_string(dev_item_obj, "ip", user_dev->dev_info.ip, sizeof(user_dev->dev_info.ip));
            mydev_json_get_int(dev_item_obj, "encrpyt_enable", &user_dev->dev_info.encrpyt_enable);
            mydev_json_get_int(dev_item_obj, "dev_type", (int *)&user_dev->dev_info.dev_type);
            mydev_json_get_int(dev_item_obj, "vendor_num", (int *)&user_dev->dev_info.vendor_num);
            mydev_json_get_string(dev_item_obj, "id", user_dev->dev_info.id, sizeof(user_dev->dev_info.id));
            mydev_json_get_string(dev_item_obj, "vendor_name", user_dev->dev_info.vendor_name, sizeof(user_dev->dev_info.vendor_name));
            mydev_json_get_int(dev_item_obj, "machine_num", &user_dev->dev_info.machine_num);
            mydev_json_get_string(dev_item_obj, "location", user_dev->dev_info.location, sizeof(user_dev->dev_info.location));
            mydev_json_get_string(dev_item_obj, "name", user_dev->dev_info.name, sizeof(user_dev->dev_info.name));
            mydev_json_get_string(dev_item_obj, "version", user_dev->dev_info.version, sizeof(user_dev->dev_info.version));
            error_cnt = 0;
            if(user_check_vendor_num(user_dev->dev_info.vendor_num)<0)
            {
                egsc_log_info("invalid device vendornum(%04d), skip parser dev(name:%s).\n", user_dev->dev_info.vendor_num, dev_name);
                error_cnt++;
            }
            if(user_check_device_type(user_dev->dev_info.dev_type)<0)
            {
                egsc_log_info("invalid device dev_type(%d), skip parser dev(name:%s).\n", user_dev->dev_info.dev_type, dev_name);
                error_cnt++;
            }
            if(user_check_device_id(user_dev->dev_info.id)<0)
            {
                egsc_log_info("invalid device id(%s), skip parser dev(name:%s).\n", user_dev->dev_info.id, dev_name);
                error_cnt++;
            }
            if(error_cnt>0)
            {
                egsc_platform_free(user_dev);
                user_dev = NULL;
                continue;
            }
            subdev_array_obj = mydev_json_get_object(dev_item_obj, "subdev");
            if(NULL != subdev_array_obj)
            {
                mydev_json_get_array_size(subdev_array_obj, &user_dev->dev_info.subdev_count);
                subdev_cnt = user_dev->dev_info.subdev_count;
                if(subdev_cnt>0)
                {
                    user_dev->dev_info.subdev_info = (egsc_subdev_info *)egsc_platform_malloc(sizeof(egsc_subdev_info)*user_dev->dev_info.subdev_count);
                    if(NULL != user_dev->dev_info.subdev_info)
                    {
                        for(loop=0;loop<subdev_cnt;loop++)
                        {
                            subdev_info = user_dev->dev_info.subdev_info + loop;
                            array_item_obj = mydev_json_get_array_item(subdev_array_obj,loop);
                            if(NULL != array_item_obj)
                            {
                                mydev_json_get_int(array_item_obj, "subdev_type", (int *)&subdev_info->subdev_id.subdev_type);
                                mydev_json_get_string(array_item_obj, "id", subdev_info->subdev_id.subdev_mac, sizeof(subdev_info->subdev_id.subdev_mac));
                                mydev_json_get_int(array_item_obj, "subdev_num", &subdev_info->subdev_id.subdev_num);
                                mydev_json_get_string(array_item_obj, "dev_name", subdev_info->dev_name, sizeof(subdev_info->dev_name));
                                mydev_json_get_string(array_item_obj, "vendor_name", subdev_info->vendor_name, sizeof(subdev_info->vendor_name));
                                mydev_json_get_int(array_item_obj, "detail_type", &subdev_info->detail_type);
                                mydev_json_get_int(array_item_obj, "register_type", &subdev_info->register_type);
                                mydev_json_get_string(array_item_obj, "ip", subdev_info->ip, sizeof(subdev_info->ip));
                                mydev_json_get_int(array_item_obj, "port", &subdev_info->port);
                                mydev_json_get_string(array_item_obj, "user_name", subdev_info->user_name, sizeof(subdev_info->user_name));
                                mydev_json_get_string(array_item_obj, "user_pw", subdev_info->user_pw, sizeof(subdev_info->user_pw));
                                mydev_json_get_string(array_item_obj, "version", subdev_info->version, sizeof(subdev_info->version));
                                mydev_json_get_int(array_item_obj, "number", &subdev_info->number);
                                mydev_json_get_string(array_item_obj, "soft_version", subdev_info->soft_version, sizeof(subdev_info->soft_version));
                                error_cnt = 0;
                                if(user_check_subdevice_type(user_dev->dev_info.dev_type, subdev_info->subdev_id.subdev_type)<0)
                                {
                                    egsc_log_info("invalid subdev_type(%04d), skip parser dev(name:%s).\n", subdev_info->subdev_id.subdev_type, subdev_info->dev_name);
                                    error_cnt++;
                                }
                                if(user_check_device_id(subdev_info->subdev_id.subdev_mac)<0)
                                {
                                    egsc_log_info("invalid subdev mac(%s), skip parser dev(name:%s).\n", subdev_info->subdev_id.subdev_mac, subdev_info->dev_name);
                                    error_cnt++;
                                }
                                if(error_cnt>0)
                                {
                                    egsc_platform_free(user_dev->dev_info.subdev_info);
                                    user_dev->dev_info.subdev_info = NULL;
                                    continue;
                                }
                                else
                                {
                                    valid_subdev_cnt++;
                                }
                            }
                        }
                        user_dev->dev_info.subdev_count = valid_subdev_cnt;
                    }
                    else
                    {
                        user_dev->dev_info.subdev_count = 0;
                    }
                }
            }

            valid_dev_cnt++;
            user_dev_enqueue(&s_mydev_dev_list_head, user_dev, NULL);
        }
    }

    fclose(fd_conf);

    if(valid_dev_cnt<=0)
    {
        egsc_log_user("no found valid device.\n");
        return EGSC_RET_ERROR;
    }
    else
    {
        return EGSC_RET_SUCCESS;
    }
}

static int mydev_boot_load_parameters()
{
    int error_cnt = 1;
    char *param_string = NULL;
    char param_buff[1024] = {0};
    FILE *fd_conf = NULL;                   //配置文件描述符

    egsc_log_debug("load parameters start.\n");
    egsc_log_debug("load parameters file(%s).\n", TEST_PARAM_FILE_NAME);
    fd_conf = fopen(TEST_PARAM_FILE_NAME,"r");
    if(NULL != fd_conf)
    {
        if(fgets(param_buff, sizeof(param_buff), fd_conf))
        {
            s_mydev_params_obj = mydev_json_parse(param_buff);
            if(NULL != s_mydev_params_obj)
            {
                param_string = mydev_json_print_tostring(s_mydev_params_obj);
                if(NULL != param_string)
                {
                    if(*param_string == '[')
                    {
                        error_cnt = 0;
                    }
                    egsc_platform_free(param_string);
                    param_string = NULL;
                }
            }
        }
    }

    if(1 == error_cnt)
    {
        if(NULL != fd_conf)
        {
            fclose(fd_conf);
        }
        fd_conf = fopen(TEST_PARAM_FILE_NAME,"w+");
        if(NULL != fd_conf)
        {
            s_mydev_params_obj = mydev_json_create_array();
            if(NULL != s_mydev_params_obj)
            {
                param_string = mydev_json_print_tostring(s_mydev_params_obj);
                if(NULL != param_string)
                {
                    fwrite(param_string, strlen(param_string), 1, fd_conf);
                    free(param_string);
                    param_string = NULL;
                }
            }
        }

    }

    if(NULL != fd_conf)
    {
        fclose(fd_conf);
    }

    return EGSC_RET_SUCCESS;
}

static int user_file_store_parameters(int handle, egsc_dev_parameters *dev_params)
{
    FILE *fd_conf = NULL;                   //文件描述符
    char *param_string = NULL;
    mydev_json_obj single_param_obj = NULL;
    egsc_door_ctrl_setting_parameters_param *special_door_ctrl_param;
    egsc_elevator_setting_parameters_param *special_elevator_param;
    egsc_parking_ctrl_setting_parameters_param *special_parking_param;
    user_dev_info *user_dev = NULL;
    char device_id[32];
    char device_id_dst[32];
    int array_size = 0;
    int array_index = 0;
    mydev_json_obj item_obj = NULL;

    egsc_log_debug("store parameters start.\n");
    egsc_log_debug("store parameters file(%s).\n", TEST_PARAM_FILE_NAME);

    if(NULL == s_mydev_params_obj)
    {
        egsc_log_error("param boot load failed.\n");
        return EGSC_RET_ERROR;
    }

    memset(device_id, 0, sizeof(device_id));
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    user_dev_get_handle(&s_mydev_dev_list_head, handle, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_error("no found device.\n");
        return EGSC_RET_ERROR;
    }

    fd_conf = fopen(TEST_PARAM_FILE_NAME,"w");
    if(NULL == fd_conf)
    {
        egsc_log_error("parameters file open failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        return EGSC_RET_ERROR;
    }

    single_param_obj = mydev_json_create_object();
    if(NULL == single_param_obj)
    {
        egsc_log_error("new create store param obj failed.\n");
        return EGSC_RET_ERROR;
    }

    mydev_json_get_array_size(s_mydev_params_obj, &array_size);
    for(array_index=0;array_index<array_size;array_index++)
    {
        item_obj = mydev_json_get_array_item(s_mydev_params_obj,array_index);
        if(NULL != item_obj)
        {
            mydev_json_get_string(item_obj, "deviceID", device_id, sizeof(device_id));
            if(strcmp(device_id, device_id_dst) == 0)
            {
                mydev_json_delete_index(s_mydev_params_obj, array_index);
            }
        }
    }

    mydev_json_add_string(single_param_obj, "deviceID", device_id_dst);
    mydev_json_add_string(single_param_obj, "fileServerUrl", dev_params->fileserver_url);
    mydev_json_add_string(single_param_obj, "ntpServer", dev_params->ntp_server);
    mydev_json_add_string(single_param_obj, "httpUserName", dev_params->http_username);
    mydev_json_add_string(single_param_obj, "httpPassword", dev_params->http_password);
    if(EGSC_TYPE_DOOR_CTRL == user_dev->dev_info.dev_type)
    {
        special_door_ctrl_param = (egsc_door_ctrl_setting_parameters_param *)dev_params->dev_specail_param;
        mydev_json_add_string(single_param_obj, "alarmTimeout", special_door_ctrl_param->alarm_timeout);
        mydev_json_add_string(single_param_obj, "openDuration", special_door_ctrl_param->open_duration);
    }
    else if((EGSC_TYPE_ELE_LINK_CTRL == user_dev->dev_info.dev_type) ||
            (EGSC_TYPE_ELEVATOR_CTRL == user_dev->dev_info.dev_type))
    {
        special_elevator_param = (egsc_elevator_setting_parameters_param *)dev_params->dev_specail_param;
        mydev_json_add_string(single_param_obj, "liftCarNumber", special_elevator_param->liftcar_number);
        mydev_json_add_string(single_param_obj, "workMode", special_elevator_param->work_mode);
        mydev_json_add_string(single_param_obj, "lightMode", special_elevator_param->light_mode);
        if(strlen(special_elevator_param->interval_time) > 0)
        {
            mydev_json_add_string(single_param_obj, "intervalTime", special_elevator_param->interval_time);
        }
    }
    else if(EGSC_TYPE_PARKING_CTRL == user_dev->dev_info.dev_type)
    {
        special_parking_param = (egsc_parking_ctrl_setting_parameters_param *)dev_params->dev_specail_param;
        if(strlen(special_parking_param->talk_ip1)>0)
        {
            mydev_json_add_string(single_param_obj, "talkIP1", special_parking_param->talk_ip1);
        }
        if(strlen(special_parking_param->talk_port1)>0)
        {
            mydev_json_add_string(single_param_obj, "talkPort1", special_parking_param->talk_port1);
        }
        if(strlen(special_parking_param->start_time)>0)
        {
            mydev_json_add_string(single_param_obj, "startTime", special_parking_param->start_time);
        }
        if(strlen(special_parking_param->end_time)>0)
        {
            mydev_json_add_string(single_param_obj, "endTime", special_parking_param->end_time);
        }
        if(strlen(special_parking_param->talk_ip2)>0)
        {
            mydev_json_add_string(single_param_obj, "talkIP2", special_parking_param->talk_ip2);
        }
        if(strlen(special_parking_param->talk_port2)>0)
        {
            mydev_json_add_string(single_param_obj, "talkPort2", special_parking_param->talk_port2);
        }
        if(strlen(special_parking_param->mac_no)>0)
        {
            mydev_json_add_string(single_param_obj, "macNo", special_parking_param->mac_no);
        }
        if(strlen(special_parking_param->camera1_rtsp_addr)>0)
        {
            mydev_json_add_string(single_param_obj, "camera1RtspAddr", special_parking_param->camera1_rtsp_addr);
        }
        if(strlen(special_parking_param->camera2_rtsp_addr)>0)
        {
            mydev_json_add_string(single_param_obj, "camera2RtspAddr", special_parking_param->camera2_rtsp_addr);
        }
        if(strlen(special_parking_param->cameras_synergism)>0)
        {
            mydev_json_add_string(single_param_obj, "camerasSynergism", special_parking_param->cameras_synergism);
        }
        if(special_parking_param->device_entry_type >= 0)
        {
            mydev_json_add_int(single_param_obj, "DeviceEntryType", special_parking_param->device_entry_type);
        }
    }

    mydev_json_add_item(s_mydev_params_obj, NULL, single_param_obj);
    param_string = mydev_json_print_tostring(s_mydev_params_obj);
    if(NULL != param_string)
    {
        fwrite(param_string, strlen(param_string), 1, fd_conf);
        egsc_platform_free(param_string);
        param_string = NULL;
    }

    fclose(fd_conf);

    return EGSC_RET_SUCCESS;
}

static int user_file_load_parameters(int handle, egsc_dev_parameters *dev_params)
{
    char device_id[32];
    char device_id_dst[32];
    user_dev_info *user_dev = NULL;
    int array_size = 0;
    int array_index = 0;
    mydev_json_obj item_obj = NULL;
    egsc_door_ctrl_setting_parameters_param *special_door_ctrl_param;
    egsc_elevator_setting_parameters_param *special_elevator_param;
    egsc_parking_ctrl_setting_parameters_param *special_parking_param;

    memset(device_id, 0, sizeof(device_id));
    memset(device_id_dst, 0, sizeof(device_id_dst));
    user_dev_get_handle(&s_mydev_dev_list_head, handle, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_error("no found device.\n");
        return EGSC_RET_ERROR;
    }
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    if(strlen(device_id_dst) == 0)
    {
        egsc_log_error("no found device.\n");
        return EGSC_RET_ERROR;
    }

    mydev_json_get_array_size(s_mydev_params_obj, &array_size);
    for(array_index=0;array_index<array_size;array_index++)
    {
        item_obj = mydev_json_get_array_item(s_mydev_params_obj,array_index);
        if(NULL != item_obj)
        {
            memset(device_id, 0, sizeof(device_id));
            mydev_json_get_string(item_obj, "deviceID", device_id, sizeof(device_id));
            if(strcmp(device_id, device_id_dst) == 0)
            {
                break;
            }
            else
            {
                item_obj = NULL;
            }
        }
    }

    if(NULL != item_obj)
    {
        mydev_json_get_string(item_obj, "fileServerUrl", dev_params->fileserver_url, sizeof(dev_params->fileserver_url));
        mydev_json_get_string(item_obj, "ntpServer", dev_params->ntp_server, sizeof(dev_params->ntp_server));
        mydev_json_get_string(item_obj, "httpUserName", dev_params->http_username, sizeof(dev_params->http_username));
        mydev_json_get_string(item_obj, "httpPassword", dev_params->http_password, sizeof(dev_params->http_password));
        if(EGSC_TYPE_DOOR_CTRL == user_dev->dev_info.dev_type)
        {
            special_door_ctrl_param = (egsc_door_ctrl_setting_parameters_param *)dev_params->dev_specail_param;
            mydev_json_get_string(item_obj, "alarmTimeout", special_door_ctrl_param->alarm_timeout, sizeof(special_door_ctrl_param->alarm_timeout));
            mydev_json_get_string(item_obj, "openDuration", special_door_ctrl_param->open_duration, sizeof(special_door_ctrl_param->open_duration));
        }
        else if((EGSC_TYPE_ELE_LINK_CTRL == user_dev->dev_info.dev_type) ||
                (EGSC_TYPE_ELEVATOR_CTRL == user_dev->dev_info.dev_type))
        {
            special_elevator_param = (egsc_elevator_setting_parameters_param *)dev_params->dev_specail_param;
            mydev_json_get_string(item_obj, "liftCarNumber", special_elevator_param->liftcar_number, sizeof(special_elevator_param->liftcar_number));
            mydev_json_get_string(item_obj, "workMode", special_elevator_param->work_mode, sizeof(special_elevator_param->work_mode));
            mydev_json_get_string(item_obj, "lightMode", special_elevator_param->light_mode, sizeof(special_elevator_param->light_mode));
            mydev_json_get_string(item_obj, "intervalTime", special_elevator_param->interval_time, sizeof(special_elevator_param->interval_time));
        }
        else if(EGSC_TYPE_PARKING_CTRL == user_dev->dev_info.dev_type)
        {
            special_parking_param = (egsc_parking_ctrl_setting_parameters_param *)dev_params->dev_specail_param;
            mydev_json_get_string(item_obj, "talkIP1", special_parking_param->talk_ip1, sizeof(special_parking_param->talk_ip1));
            mydev_json_get_string(item_obj, "talkPort1", special_parking_param->talk_port1, sizeof(special_parking_param->talk_port1));
            mydev_json_get_string(item_obj, "startTime", special_parking_param->start_time, sizeof(special_parking_param->start_time));
            mydev_json_get_string(item_obj, "endTime", special_parking_param->end_time, sizeof(special_parking_param->end_time));
            mydev_json_get_string(item_obj, "talkIP2", special_parking_param->talk_ip2, sizeof(special_parking_param->talk_ip2));
            mydev_json_get_string(item_obj, "talkPort2", special_parking_param->talk_port2, sizeof(special_parking_param->talk_port2));
            mydev_json_get_string(item_obj, "macNo", special_parking_param->mac_no, sizeof(special_parking_param->mac_no));
            mydev_json_get_string(item_obj, "camera1RtspAddr", special_parking_param->camera1_rtsp_addr, sizeof(special_parking_param->camera1_rtsp_addr));
            mydev_json_get_string(item_obj, "camera2RtspAddr", special_parking_param->camera2_rtsp_addr, sizeof(special_parking_param->camera2_rtsp_addr));
            mydev_json_get_int(item_obj, "DeviceEntryType", &special_parking_param->device_entry_type);
        }
    }

    return EGSC_RET_SUCCESS;
}

static int user_file_store_certificate()
{
    int floor = 0;
    int array_index = 0;
    char new_line = '\n';
    char *cert_string = NULL;
    struct list_head *head = NULL;
    mydev_json_obj cert_obj = NULL;
    mydev_json_obj auth_floor_obj = NULL;
    user_certificate_info *pos, *n;

    FILE *fd_conf = NULL;                   //配置文件描述符

    egsc_log_debug("store certificate start.\n");
    egsc_log_debug("store certificate file(%s).\n", TEST_CERT_FILE_NAME);
    fd_conf = fopen(TEST_CERT_FILE_NAME,"w");
    if(NULL == fd_conf)
    {
        egsc_log_error("certificate file open failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        return EGSC_RET_ERROR;
    }

    head = &s_mydev_cert_list_head;
    list_for_each_entry_safe(pos,n,head,node)
    {
        cert_obj = mydev_json_create_object();
        if(NULL == cert_obj)
        {
            egsc_log_error("new create store certificate obj failed.\n");
            continue;
        }
        mydev_json_add_int(cert_obj, "DeviceType", pos->dev_type);
        mydev_json_add_string(cert_obj, "startTime", pos->start_time);
        mydev_json_add_string(cert_obj, "endTime", pos->end_time);
        mydev_json_add_string(cert_obj, "deviceID", pos->device_id);
        mydev_json_add_int(cert_obj, "UserType", pos->user_type);
        mydev_json_add_int(cert_obj, "CredenceType", pos->credence_type);
        mydev_json_add_string(cert_obj, "credenceNo", pos->credence_no);
        mydev_json_add_string(cert_obj, "userName", pos->user_name);
        mydev_json_add_string(cert_obj, "userID", pos->user_id);
        mydev_json_add_string(cert_obj, "opTime", pos->op_time);
        if( (EGSC_TYPE_ELE_LINK_CTRL == pos->dev_type) ||
            (EGSC_TYPE_ELEVATOR_CTRL == pos->dev_type))
        {
            auth_floor_obj = mydev_json_create_array();
            if(NULL != auth_floor_obj)
            {
                for(array_index=0; array_index<pos->auth_floor_num;array_index++)
                {
                    floor = *(pos->auth_floor+array_index);
                    if(floor >= 0)
                    {
                        mydev_json_add_int(auth_floor_obj, NULL, floor);
                    }
                }
                mydev_json_add_item(cert_obj, "elevatorAuthFloor", auth_floor_obj);
            }
            else
            {
                egsc_log_error("malloc auth floor failed, discard.\n");
            }
        }
        else if(EGSC_TYPE_PARKING_CTRL == pos->dev_type)
        {
            if(strlen(pos->place_no) > 0)
            {
                mydev_json_add_string(cert_obj, "placeNo", pos->place_no);
            }
            if(strlen(pos->place_lock_no) > 0)
            {
                mydev_json_add_string(cert_obj, "placeLockNo", pos->place_lock_no);
            }
        }

        cert_string = mydev_json_print_tostring(cert_obj);
        if(NULL != cert_string)
        {
            fwrite(cert_string, strlen(cert_string), 1, fd_conf);
            fwrite(&new_line, 1, 1, fd_conf);
            egsc_platform_free(cert_string);
            cert_string = NULL;
        }
        mydev_json_clear(cert_obj);
        cert_obj = NULL;
    }

    fclose(fd_conf);

    return EGSC_RET_SUCCESS;
}

static int user_file_load_certificate()
{
    int ret = EGSC_RET_ERROR;
    int valid_cert_num = 0;
    int array_size = 0;
    int array_index = 0;
    char cert_buff[2048];
    mydev_json_obj cert_obj = NULL;
    mydev_json_obj auth_floor_obj = NULL;
    mydev_json_obj auth_item_obj = NULL;
    user_certificate_info *cert_item = NULL;
    user_certificate_info cert_info;
    FILE *fd_conf = NULL;                   //配置文件描述符

    egsc_log_debug("load certificate start.\n");
    egsc_log_user("load conf file(%s).\n", TEST_CERT_FILE_NAME);
    fd_conf = fopen(TEST_CERT_FILE_NAME,"r");
    if(NULL == fd_conf)
    {
        egsc_log_debug("conf file open failed, errno(%d):%s .\n",
                    errno,
                    strerror(errno));
        return EGSC_RET_ERROR;
    }

    while(fgets(cert_buff, sizeof(cert_buff), fd_conf))
    {
        memset(&cert_info, 0, sizeof(cert_info));
        cert_obj = mydev_json_parse(cert_buff);
        if(NULL == cert_obj)
        {
            egsc_log_info("parser failed, skip line(%s).\n", cert_buff);
            continue;
        }

        mydev_json_get_int(cert_obj, "DeviceType", &cert_info.dev_type);
        mydev_json_get_string(cert_obj, "startTime", cert_info.start_time, sizeof(cert_info.start_time));
        mydev_json_get_string(cert_obj, "endTime", cert_info.end_time, sizeof(cert_info.end_time));
        mydev_json_get_string(cert_obj, "deviceID", cert_info.device_id, sizeof(cert_info.device_id));
        mydev_json_get_int(cert_obj, "UserType", &cert_info.user_type);
        mydev_json_get_int(cert_obj, "CredenceType", &cert_info.credence_type);
        mydev_json_get_string(cert_obj, "credenceNo", cert_info.credence_no, sizeof(cert_info.credence_no));
        mydev_json_get_string(cert_obj, "userName", cert_info.user_name, sizeof(cert_info.user_name));
        mydev_json_get_string(cert_obj, "userID", cert_info.user_id, sizeof(cert_info.user_id));
        mydev_json_get_string(cert_obj, "opTime", cert_info.op_time, sizeof(cert_info.op_time));
        if( (0 != cert_info.dev_type) &&
            (strlen(cert_info.start_time) != 0) &&
            (strlen(cert_info.end_time) != 0) &&
            (strlen(cert_info.device_id) != 0) &&
            (strlen(cert_info.credence_no) != 0) &&
            (0 != cert_info.user_type) &&
            (0 != cert_info.credence_type))
        {
            ret = user_certificate_search(&s_mydev_cert_list_head, cert_info.device_id, cert_info.credence_no);
            if(EGSC_RET_SUCCESS != ret)
            {
                cert_item = (user_certificate_info *)egsc_platform_malloc(sizeof(user_certificate_info));
                if(NULL != cert_item)
                {
                    memcpy(cert_item, &cert_info, sizeof(user_certificate_info));
                    if( (EGSC_TYPE_ELE_LINK_CTRL == cert_item->dev_type) ||
                        (EGSC_TYPE_ELEVATOR_CTRL == cert_item->dev_type))
                    {
                        auth_floor_obj = mydev_json_get_object(cert_obj, "elevatorAuthFloor");
                        if(NULL != auth_floor_obj)
                        {
                            mydev_json_get_array_size(auth_floor_obj, &array_size);
                            if(array_size>0)
                            {
                                cert_item->auth_floor_num = 0;
                                cert_item->auth_floor = (int *)egsc_platform_malloc(array_size*sizeof(int));
                                if(NULL != cert_item->auth_floor)
                                {
                                    for(array_index=0;array_index<array_size;array_index++)
                                    {
                                        auth_item_obj = mydev_json_get_array_item(auth_floor_obj,array_index);
                                        if(NULL != auth_item_obj)
                                        {
                                            ret = mydev_json_get_int(auth_item_obj, NULL, &cert_item->auth_floor[cert_item->auth_floor_num]);
                                            if(0 == ret)
                                            {
                                                cert_item->auth_floor_num++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if(EGSC_TYPE_PARKING_CTRL == cert_item->dev_type)
                    {
                        mydev_json_get_string(cert_obj, "placeNo", cert_item->place_no, sizeof(cert_item->place_no));
                        mydev_json_get_string(cert_obj, "placeLockNo", cert_item->place_lock_no, sizeof(cert_item->place_lock_no));
                    }

                    ret = user_certificate_enqueue(&s_mydev_cert_list_head, cert_item->device_id, cert_item->credence_no, cert_item);
                    if(EGSC_RET_SUCCESS != ret)
                    {
                        if( (EGSC_TYPE_ELE_LINK_CTRL == cert_item->dev_type) ||
                            (EGSC_TYPE_ELEVATOR_CTRL == cert_item->dev_type))
                        {
                            if(NULL != cert_item->auth_floor)
                            {
                                egsc_platform_free(cert_item->auth_floor);
                            }
                        }
                        egsc_platform_free(cert_item);
                        cert_item = NULL;
                    }
                    valid_cert_num++;
                }
            }
            else
            {
                egsc_log_info("device_id(%s), credence_no(%s) credence already in skip.\n", cert_info.device_id, cert_info.credence_no);
            }
        }

        mydev_json_clear(cert_obj);
        cert_obj = NULL;
    }

    fclose(fd_conf);
    egsc_log_user("load certificate end, valid num(%d).\n", valid_cert_num);

    return EGSC_RET_SUCCESS;
}

static int user_certificate_get(struct list_head *head, char *credence_no, user_certificate_info **cert_obj)
{
    if( (NULL == head) ||
        (NULL == credence_no) ||
        (NULL == cert_obj))
    {
        egsc_log_error("input head/device manager NULL.\n");
        return EGSC_RET_ERROR;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( strcmp(pos->credence_no, credence_no) == 0 )
        {
            egsc_log_debug("credence_no(%s) get dev queue success.\n", pos->credence_no);
            *cert_obj = pos;
            return EGSC_RET_SUCCESS;
        }
    }

    return EGSC_RET_ERROR;
}

static int user_certificate_search(struct list_head *head, char *device_id, char *credence_no)
{
    if( (NULL == head) ||
        (NULL == credence_no))
    {
        egsc_log_error("input head/device manager NULL.\n");
        return EGSC_RET_ERROR;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( (strcmp(pos->device_id, device_id) == 0)&&
            (strcmp(pos->credence_no, credence_no) == 0))
        {
            egsc_log_info("device_id(%s) credence_no(%s) in dev credece queue.\n", device_id, credence_no);
            return EGSC_RET_SUCCESS;
        }
    }

    return EGSC_RET_ERROR;
}

static int user_certificate_check_credence_type(struct list_head *head, char *device_id, int credence_type)
{
    if( (NULL == head) ||
        (NULL == device_id))
    {
        egsc_log_error("input head/id manager NULL.\n");
        return -1;
    }

    if(CREDENCE_TYPE_BUTTON == credence_type)
    {
        return 1;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( (strcmp(pos->device_id, device_id) == 0) &&
            (pos->credence_type == credence_type))
        {
            return 1;
        }
    }

    return -1;
}

static int user_certificate_check_credence_no(struct list_head *head, char *device_id, char *credence_no)
{
    if( (NULL == head) ||
        (NULL == device_id) ||
        (NULL == credence_no))
    {
        egsc_log_error("input head/id/credence_no manager NULL.\n");
        return -1;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( (strcmp(pos->device_id, device_id) == 0) &&
            (strcmp(pos->credence_no, credence_no) == 0))
        {
            return 1;
        }
    }

    return -1;
}

static int user_certificate_check_user_id(struct list_head *head, char *device_id, char *user_id)
{
    if( (NULL == head) ||
        (NULL == device_id) ||
        (NULL == user_id))
    {
        egsc_log_error("input head/id/user_id manager NULL.\n");
        return -1;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( (strcmp(pos->device_id, device_id) == 0) &&
            (strcmp(pos->user_id, user_id) == 0))
        {
            return 1;
        }
    }

    return -1;
}

static int user_certificate_check_user_type(struct list_head *head, char *device_id, int user_type)
{
    if( (NULL == head) ||
        (NULL == device_id))
    {
        egsc_log_error("input head/id manager NULL.\n");
        return -1;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if( (strcmp(pos->device_id, device_id) == 0) &&
            (pos->user_type  == user_type))
        {
            return 1;
        }
    }

    return -1;
}

static int user_certificate_clear(struct list_head *head, const char *device_id)
{
    if( (NULL == head) ||
        (NULL == device_id))
    {
        egsc_log_error("input head/device manager NULL.\n");
        return EGSC_RET_ERROR;
    }

    user_certificate_info *pos, *n;
    list_for_each_entry_safe(pos,n,head,node)
    {
        if(strcmp(pos->device_id, device_id) == 0)
        {
            egsc_log_debug("credence_no(%s) clear dev queue success.\n", pos->credence_no);
            list_del(&(pos->node));
            egsc_platform_free(pos);
        }
    }

    return EGSC_RET_SUCCESS;
}

static void mydev_status_callback(int handle, EGSC_DEV_STATUS_CODE status,char *desc_info)
{
    char device_id_dst[32];
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);

    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) status callback\n", device_id_dst);
    egsc_log_user("status(%d).\n", status);
    egsc_log_user("desc_info(%s).\n", desc_info);
}

static EGSC_RET_CODE mydev_reset_cb(int handle, char *user_id)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) reset\n", device_id_dst);
    egsc_log_user("userID(%s).\n", user_id);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_correction_cb(int handle)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) correction\n", device_id_dst);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_notify_update_cb(int handle, char *file_url, char *ftp_Addr, char *fm_version)
{
    int array_size = 0;
    int array_index = 0;
    mydev_json_obj item_obj = NULL;
    char *device_config = NULL;
    char device_id_dst[32];
    char id[16];
    user_dev_info *user_dev = NULL;
    FILE *fd_conf = NULL;                   //配置文件描述符

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(id, 0, sizeof(id));
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) update\n", device_id_dst);
    egsc_log_user("FileURL(%s).\n", file_url);
    egsc_log_user("FtpAddr(%s).\n", ftp_Addr);
    egsc_log_user("FirmwareVersion(%s).\n", fm_version);

    user_dev_get_handle(&s_mydev_dev_list_head, handle, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_error("no found device.\n");
        return EGSC_RET_ERROR;
    }

    if(NULL != fm_version)
    {
        snprintf(user_dev->dev_info.version, sizeof(user_dev->dev_info.version), "%s", fm_version);

        if(NULL != s_device_config_obj)
        {
            mydev_json_get_array_size(s_device_config_obj, &array_size);
            for(array_index=0;array_index<array_size;array_index++)
            {
                item_obj = mydev_json_get_array_item(s_device_config_obj,array_index);
                if(NULL != item_obj)
                {
                    mydev_json_get_string(item_obj, "id", id, sizeof(id));
                    if(strcmp(id, user_dev->dev_info.id) == 0)
                    {
                        mydev_json_delete_key(item_obj, "version");
                        mydev_json_add_string(item_obj, "version", fm_version);
                        device_config = mydev_json_print_tostring(s_device_config_obj);
                        if(NULL != device_config)
                        {
                            fd_conf = fopen(TEST_DEVICE_CONFIG_FILE_NAME,"w+");
                            if(NULL == fd_conf)
                            {
                                egsc_log_error("conf file open failed, errno(%d):%s .\n",
                                            errno,
                                            strerror(errno));
                            }
                            fwrite(device_config, strlen(device_config), 1, fd_conf);
                            if(NULL != fd_conf)
                            {
                                fclose(fd_conf);
                            }
                            egsc_platform_free(device_config);
                            device_config = NULL;
                        }
                        break;
                    }
                }
            }
        }
        user_dev->updated = 1;
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_read_parameter_cb(int handle, egsc_dev_parameters *dev_params)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) read parameter\n", device_id_dst);

    user_file_load_parameters(handle, dev_params);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_setting_parameters_cb(int handle, egsc_dev_parameters *dev_params)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) setting parameter\n", device_id_dst);
    egsc_log_user("FtpAddr(%s).\n", dev_params->fileserver_url);
    egsc_log_user("ntpServer(%s).\n", dev_params->ntp_server);
    egsc_log_user("httpUserName(%s).\n", dev_params->http_username);
    egsc_log_user("httpPassword(%s).\n", dev_params->http_password);
    egsc_log_debug("dev_specail_param(0x%x).\n", dev_params->dev_specail_param);

    user_file_store_parameters(handle, dev_params);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_load_certificate_cb(int handle, egsc_subdev_id *p_dev_id, egsc_dev_cb_certificate_param *cert_param)
{
    int ret = EGSC_RET_ERROR;
    char device_id[32];
    user_dev_info *user_dev = NULL;
    user_certificate_info *cert_obj = NULL;
    egsc_elevator_certificate_special_param *elevator_param;
    egsc_parking_ctrl_certificate_special_param *certificate_sp_param;

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id, 0, sizeof(device_id));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id, sizeof(device_id));
    egsc_log_user("device(%s) load certificate\n", device_id);
    egsc_log_user("startTime(%s).\n", cert_param->start_time);
    egsc_log_user("endTime(%s).\n", cert_param->end_time);
    egsc_log_user("UserType(%d).\n", cert_param->user_type);
    egsc_log_user("CredenceType(%d).\n", cert_param->credence_type);
    egsc_log_user("credenceNo(%s).\n", cert_param->credence_no);
    egsc_log_user("userName(%s).\n", cert_param->user_name);
    egsc_log_user("userID(%s).\n", cert_param->user_id);
    egsc_log_user("opTime(%s).\n", cert_param->op_time);

    if(NULL != p_dev_id)
    {
        egsc_log_debug(">>device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }
    else
    {
        egsc_log_debug(">>p_dev_id NULL\n");
    }

    if(strlen(cert_param->credence_no) == 0)
    {
        egsc_log_user("authority skip to file.\n");
        return EGSC_RET_SUCCESS;
    }

    user_dev_get_handle(&s_mydev_dev_list_head, handle, &user_dev);
    if(NULL != user_dev)
    {
        ret = user_certificate_search(&s_mydev_cert_list_head, device_id, cert_param->credence_no);
        if(EGSC_RET_SUCCESS != ret)
        {
            egsc_log_user("device_id(%s), credence_no(%s) credence new in.\n", device_id, cert_param->credence_no);
            cert_obj = (user_certificate_info *)egsc_platform_malloc(sizeof(user_certificate_info));
            if(NULL != cert_obj)
            {
                memset(cert_obj, 0, sizeof(user_certificate_info));
                cert_obj->dev_type = user_dev->dev_info.dev_type;
                strcpy(cert_obj->start_time, cert_param->start_time);
                strcpy(cert_obj->end_time, cert_param->end_time);
                strcpy(cert_obj->device_id, device_id);
                cert_obj->user_type = cert_param->user_type;
                cert_obj->credence_type = cert_param->credence_type;
                strcpy(cert_obj->credence_no, cert_param->credence_no);
                strcpy(cert_obj->user_name, cert_param->user_name);
                strcpy(cert_obj->user_id, cert_param->user_id);
                strcpy(cert_obj->op_time, cert_param->op_time);
                if( (EGSC_TYPE_ELE_LINK_CTRL == cert_obj->dev_type) ||
                    (EGSC_TYPE_ELEVATOR_CTRL == cert_obj->dev_type))
                {
                    elevator_param = (egsc_elevator_certificate_special_param *)cert_param->dev_special_param;
                    if(elevator_param->elevator_auth_floor_num > 0)
                    {
                        cert_obj->auth_floor = (int *)egsc_platform_malloc(elevator_param->elevator_auth_floor_num*sizeof(int));
                        if(NULL != cert_obj->auth_floor)
                        {
                            memset(cert_obj->auth_floor, 0, elevator_param->elevator_auth_floor_num*sizeof(int));
                            memcpy(cert_obj->auth_floor, elevator_param->elevator_auth_floor, elevator_param->elevator_auth_floor_num*sizeof(int));
                            cert_obj->auth_floor_num = elevator_param->elevator_auth_floor_num;
                        }
                        else
                        {
                            egsc_log_error("malloc auth floor failed, discard auth info.\n");
                        }
                    }
                }
                else if(EGSC_TYPE_PARKING_CTRL == cert_obj->dev_type)
                {
                    certificate_sp_param = (egsc_parking_ctrl_certificate_special_param *)cert_param->dev_special_param;
                    if(strlen(certificate_sp_param->place_no) > 0)
                    {
                        strcpy(cert_obj->place_no, certificate_sp_param->place_no);
                    }
                    if(strlen(certificate_sp_param->place_lock_no) > 0)
                    {
                        strcpy(cert_obj->place_lock_no, certificate_sp_param->place_lock_no);
                    }
                }

                ret = user_certificate_enqueue(&s_mydev_cert_list_head, cert_obj->device_id, cert_param->credence_no, cert_obj);
                if(EGSC_RET_SUCCESS != ret)
                {
                    if(NULL != cert_obj->auth_floor)
                    {
                        egsc_platform_free(cert_obj->auth_floor);
                        cert_obj->auth_floor = NULL;
                    }
                    egsc_platform_free(cert_obj);
                    cert_obj = NULL;
                }
                else
                {
                    //更新凭证文件
                    user_file_store_certificate();
                }
            }
        }
        else
        {
            egsc_log_user("device_id(%s), credence_no(%s) credence already in skip.\n", device_id, cert_param->credence_no);
        }
    }

    return ret;
}

static EGSC_RET_CODE mydev_read_certificate_cb(int handle, int credence_type, char *credence_no,
                                        egsc_dev_cb_certificate_param *dev_param)
{
    int ret = EGSC_RET_ERROR;
    char device_id[32];
    user_certificate_info *cert_obj;
    egsc_elevator_certificate_special_param *elevator_param;
    egsc_parking_ctrl_certificate_special_param *certificate_sp_param;

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id, 0, sizeof(device_id));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id, sizeof(device_id));
    egsc_log_user("device(%s) read certificate\n", device_id);
    egsc_log_user("CredenceType(%d).\n", credence_type);
    egsc_log_user("credenceNo(%s).\n", credence_no);

    if((NULL == credence_no) ||
        (strlen(credence_no) == 0))
    {
        egsc_log_user("authority skip read.\n");
        return EGSC_RET_SUCCESS;
    }

    ret = user_certificate_get(&s_mydev_cert_list_head, credence_no, &cert_obj);
    if(EGSC_RET_SUCCESS == ret)
    {
        egsc_log_debug("start_time(%s).\n", cert_obj->start_time);
        egsc_log_debug("end_time(%s).\n", cert_obj->end_time);
        egsc_log_debug("device_id(%s).\n", cert_obj->device_id);
        egsc_log_debug("user_type(%d).\n", cert_obj->user_type);
        egsc_log_debug("credence_type(%d).\n", cert_obj->credence_type);
        egsc_log_debug("credence_no(%s).\n", cert_obj->credence_no);
        egsc_log_debug("user_name(%s).\n", cert_obj->user_name);
        egsc_log_debug("user_id(%s).\n", cert_obj->user_id);
        egsc_log_debug("op_time(%s).\n", cert_obj->op_time);
        if(NULL != dev_param)
        {
            snprintf(dev_param->start_time, sizeof(dev_param->start_time), "%s", cert_obj->start_time);
            snprintf(dev_param->end_time, sizeof(dev_param->end_time), "%s", cert_obj->end_time);
            dev_param->credence_type = cert_obj->credence_type;
            snprintf(dev_param->credence_no, sizeof(dev_param->credence_no), "%s", cert_obj->credence_no);
            snprintf(dev_param->user_name, sizeof(dev_param->user_name), "%s", cert_obj->user_name);
            snprintf(dev_param->op_time, sizeof(dev_param->op_time), "%s", cert_obj->op_time);
            if( (EGSC_TYPE_ELE_LINK_CTRL == cert_obj->dev_type) ||
                (EGSC_TYPE_ELEVATOR_CTRL == cert_obj->dev_type))
            {
                elevator_param = (egsc_elevator_certificate_special_param *)dev_param->dev_special_param;
                if(NULL != elevator_param->elevator_auth_floor)
                {
                    memcpy(elevator_param->elevator_auth_floor, cert_obj->auth_floor, cert_obj->auth_floor_num*sizeof(int));
                    elevator_param->elevator_auth_floor_num = cert_obj->auth_floor_num;
                }
            }
            else if(EGSC_TYPE_PARKING_CTRL == cert_obj->dev_type)
            {
                certificate_sp_param = (egsc_parking_ctrl_certificate_special_param *)dev_param->dev_special_param;
                if(strlen(cert_obj->place_no) > 0)
                {
                    strcpy(certificate_sp_param->place_no, cert_obj->place_no);
                }
                if(strlen(cert_obj->place_lock_no) > 0)
                {
                    strcpy(certificate_sp_param->place_lock_no, cert_obj->place_lock_no);
                }
            }
        }
    }

    return ret;
}

static EGSC_RET_CODE mydev_delete_certificate_cb(int handle, egsc_subdev_id *dev_id, int credence_type, char *credence_no,
                                                      char *user_id)
{
    int ret = EGSC_RET_ERROR;
    int loop = 0;
    char device_id[32];
    user_certificate_info *cert_obj = NULL;
    user_dev_info *user_dev = NULL;

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id, 0, sizeof(device_id));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, dev_id, device_id, sizeof(device_id));
    egsc_log_user("device(%s) delete certificate\n", device_id);
    egsc_log_user("CredenceType(%d).\n", credence_type);
    egsc_log_user("credenceNo(%s).\n", credence_no);
    egsc_log_user("userID(%s).\n", user_id);

    if(NULL != dev_id)
    {
        egsc_subdev_id *p_dev_id = dev_id;
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }

    if((NULL == credence_no) ||
        (strlen(credence_no) == 0))
    {
        egsc_log_user("authority skip delete.\n");
        return EGSC_RET_SUCCESS;
    }

    user_dev_get_handle(&s_mydev_dev_list_head, handle, &user_dev);
    ret = user_certificate_dequeue(&s_mydev_cert_list_head, device_id, credence_no, &cert_obj);
    if( (EGSC_RET_SUCCESS == ret) &&
        (NULL != cert_obj))
    {
        egsc_log_debug("device_id(%s), credence_no(%s) credence delece success.\n", device_id, credence_no);
        egsc_log_debug("dev_type(%04d).\n", cert_obj->dev_type);
        egsc_log_debug("start_time(%s).\n", cert_obj->start_time);
        egsc_log_debug("end_time(%s).\n", cert_obj->end_time);
        egsc_log_debug("device_id(%s).\n", cert_obj->device_id);
        egsc_log_debug("user_type(%d).\n", cert_obj->user_type);
        egsc_log_debug("credence_type(%d).\n", cert_obj->credence_type);
        egsc_log_debug("credence_no(%s).\n", cert_obj->credence_no);
        egsc_log_debug("user_name(%s).\n", cert_obj->user_name);
        egsc_log_debug("user_id(%s).\n", cert_obj->user_id);
        egsc_log_debug("op_time(%s).\n", cert_obj->op_time);

        if( (EGSC_TYPE_ELE_LINK_CTRL == cert_obj->dev_type) ||
            (EGSC_TYPE_ELEVATOR_CTRL == cert_obj->dev_type))
        {
            egsc_log_debug("auth_floor_num(%s).\n", cert_obj->auth_floor_num);
            for(loop=0;loop<cert_obj->auth_floor_num;loop++)
            {
                egsc_log_debug("auth_floor[%d](%d).\n", loop, cert_obj->auth_floor[loop]);
            }
            if(NULL != cert_obj->auth_floor)
            {
                egsc_platform_free(cert_obj->auth_floor);
                cert_obj->auth_floor = NULL;
            }
        }
        else if(EGSC_TYPE_PARKING_CTRL == cert_obj->dev_type)
        {
            egsc_log_debug("place_no(%s).\n", cert_obj->place_no);
            egsc_log_debug("place_lock_no(%s).\n", cert_obj->place_lock_no);
        }

        egsc_platform_free(cert_obj);
        cert_obj = NULL;

        //更新凭证文件
        user_file_store_certificate();
    }
    else
    {
        egsc_log_debug("device_id(%s), credence_no(%s) credence delece failed.\n", device_id, credence_no);
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_load_certificate_in_batch_cb(int handle, egsc_dev_cb_certificate_in_batch_param *cert_batch_param)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("file_id(%s).\n", cert_batch_param->file_id);
    egsc_log_debug("op_time(%s).\n", cert_batch_param->op_time);

    snprintf(s_batch_cert_file_id, sizeof(s_batch_cert_file_id), "%s", cert_batch_param->file_id);
    snprintf(s_batch_cert_op_time, sizeof(s_batch_cert_op_time), "%s", cert_batch_param->op_time);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_read_certificate_in_batch_cb(int handle, egsc_dev_cb_certificate_in_batch_param *cert_batch_param)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);

    snprintf(cert_batch_param->file_id, sizeof(cert_batch_param->file_id), "%s", s_batch_cert_file_id);
    snprintf(cert_batch_param->op_time, sizeof(cert_batch_param->op_time), "%s", s_batch_cert_op_time);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_delete_certificate_in_batch_cb(int handle, char *op_time, int credence_type)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("op_time(%s).\n", op_time);
    egsc_log_debug("credence_type(%d).\n", credence_type);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_download_black_and_white_list_cb(int handle, egsc_dev_cb_black_and_white_list_param *bw_param)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) download black and white list\n", device_id_dst);
    egsc_log_user("status(%d).\n", bw_param->status);
    egsc_log_user("startTime(%s).\n", bw_param->start_time);
    egsc_log_user("endTime(%s).\n", bw_param->end_time);
    egsc_log_user("carNo(%s).\n", bw_param->car_no);
    egsc_log_user("userID(%s).\n", bw_param->user_id);
    egsc_log_user("userName(%s).\n", bw_param->user_name);
    egsc_log_user("opTime(%s).\n", bw_param->op_time);
    egsc_log_user("remark(%s).\n", bw_param->remark);
    egsc_log_user("sheetType(%d).\n", bw_param->sheet_type);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_load_left_car_seat_cb(int handle, egsc_dev_cb_left_car_seat_param *seat_param)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) load left car seat\n", device_id_dst);
    egsc_log_user("remaningSpace(%d).\n", seat_param->remaning_space);
    egsc_log_user("allSpace(%d).\n", seat_param->all_space);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_gate_ctrl_cb(int handle, egsc_subdev_id *p_dev_id, int op_type, int user_type,char *user_id,
                            void * dev_specail_param)
{
    int manual_ctrl = 0;
    user_dev_info *user_dev = NULL;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) gate ctrl\n", device_id_dst);
    egsc_log_user("operateType(%d).\n", op_type);
    egsc_log_user("UserType(%d).\n", user_type);
    egsc_log_user("userID(%s).\n", user_id);
    egsc_log_debug("dev_specail_param(0x%x).\n", dev_specail_param);

    if(NULL != p_dev_id)
    {
         egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }
    else
    {
        egsc_log_debug("master p_dev_id is NULL ok\n");
    }

    user_dev_get_handle(&s_mydev_dev_list_head, handle, &user_dev);
    if(NULL != user_dev)
    {
        switch(op_type)
        {
            case 0:
            {
                egsc_log_user("gate: close \n");
                break;
            }
            case 1:
            {
                egsc_log_user("gate: open \n");
                break;
            }
            case 2:
            {
                egsc_log_user("gate: keep open \n");
                break;
            }
            case 3:
            {
                egsc_log_user("gate: keep close \n");
                break;
            }
            case 4:
            {
                egsc_log_user("gate: recovery \n");
                break;
            }
            default:
            {
                egsc_log_user("gate: close \n");
                break;
            }
            s_gate_optype = op_type;
        }

        if(EGSC_TYPE_PARKING_CTRL == user_dev->dev_info.dev_type)
        {
            manual_ctrl = *(int *)dev_specail_param;
            if(manual_ctrl >=0)
            {
                egsc_log_user("gate control: %s \n", (1==manual_ctrl)?"Manual":"Auto");
            }
        }
    }
    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_play_voice_cb(int handle, char *text)
{
    char device_id_dst[32];
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) play voice\n", device_id_dst);

    egsc_log_user("text(%s).\n", text);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_read_vol_cb(int handle, egsc_dev_vol_param *param)
{
    char device_id_dst[32];
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n");
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) read vol\n", device_id_dst);

    snprintf(param->start_time, sizeof(param->start_time), "%s", s_start_time);
    snprintf(param->start_time, sizeof(param->end_time), "%s", s_end_time);
    param->level1 = s_level1;
    param->level2 = s_level2;

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_set_vol_cb(int handle, egsc_dev_vol_param *param)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) set vol\n", device_id_dst);
    egsc_log_user("level1(%d).\n", param->level1);
    egsc_log_user("level2(%d).\n", param->level2);
    egsc_log_user("startTime(%s).\n", param->start_time);
    egsc_log_user("endTime(%s).\n", param->end_time);

    s_level1 = param->level1;
    s_level2 = param->level2;
    snprintf(s_start_time, sizeof(s_start_time), "%s", param->start_time);
    snprintf(s_end_time, sizeof(s_end_time), "%s", param->end_time);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_fac_visit_control_cb(int handle, egsc_subdev_id *p_dev_id, int source_floor, int direction, char *source_room,
                                        int dest_floor_num, int *dest_floor, char *dest_room)
{
    int loop = 0;
    char device_id_dst[32];
    egsc_log_user("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) elevator visit control\n", device_id_dst);
    egsc_log_user("SourceFloor(0x%x).\n", source_floor);
    egsc_log_user("Direction(%d).\n", direction);
    egsc_log_user("sourceRoom(%s).\n", source_room);

    for(loop=0; loop<dest_floor_num; loop++)
    {
        egsc_log_user("DestFloor[%d](0x%x).\n", loop, dest_floor[loop]);
    }
    egsc_log_user("destRoom(%s).\n", dest_room);

    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_fac_key_control_cb(int handle, egsc_subdev_id *p_dev_id, int control_mode,
    int dest_floor_num, int *dest_floor)
{
    int loop = 0;
    char device_id_dst[32];

    egsc_log_user("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) elevator key control\n", device_id_dst);
    egsc_log_user("ControlMode(%d).\n", control_mode);
    for(loop=0; loop<dest_floor_num; loop++)
    {
        egsc_log_user("DestFloor[%d](0x%x).\n", loop, dest_floor[loop]);
    }
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_fac_calling_cb(int handle, egsc_subdev_id *p_dev_id, int source_floor,
    int direction, char *source_room)
{
    char device_id_dst[32];

    egsc_log_user("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) elevator calling control\n", device_id_dst);
    egsc_log_user("SourceFloor(0x%x).\n", source_floor);
    egsc_log_user("Direction(%d).\n", direction);
    egsc_log_user("sourceRoom(%s).\n", source_room);
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }
    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_fac_inter_call_auth_cb(int handle, egsc_subdev_id *p_dev_id, char *elevator_addr,
                                            int dest_floor_num, int *dest_floor, char *dest_room)
{
    int loop = 0;
    char device_id_dst[32];

    egsc_log_user("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) elevator inter call auth\n", device_id_dst);
    egsc_log_user("elevatorAddr(%s).\n", elevator_addr);
    for(loop=0; loop<dest_floor_num; loop++)
    {
        egsc_log_user("DestFloor[%d](0x%x).\n", loop, dest_floor[loop]);
    }
    egsc_log_user("destRoom(%s).\n", dest_room);
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_fac_inter_call_lighting_cb(int handle, egsc_subdev_id *p_dev_id, char *elevator_addr,
                                                int dest_floor_num, int *dest_floor, char *dest_room)
{
    int loop = 0;
    char device_id_dst[32];

    egsc_log_user("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) elevator inter call lighting\n", device_id_dst);
    egsc_log_user("elevatorAddr(%s).\n", elevator_addr);
    for(loop=0; loop<dest_floor_num; loop++)
    {
        egsc_log_user("DestFloor[%d](0x%x).\n", loop, dest_floor[loop]);
    }
    egsc_log_user("destRoom(%s).\n", dest_room);
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }
    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_fac_lift_lighting_cb(int handle, egsc_subdev_id *p_dev_id, int source_floor)
{
    char device_id_dst[32];

    egsc_log_user("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) elevator lift lighting\n", device_id_dst);
    egsc_log_user("SourceFloor(0x%x).\n", source_floor);
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }
    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_fac_delayed_closing_cb(int handle, egsc_subdev_id *p_dev_id)
{
    char device_id_dst[32];

    egsc_log_user("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) elevator delayed closing\n", device_id_dst);
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }
    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_fac_status_req_cb(int handle, egsc_subdev_id *p_dev_id, egsc_dev_cb_fac_status_param *fac_status_param)
{
    char device_id_dst[32];

    egsc_log_user("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) elevator status req\n", device_id_dst);
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }

    return EGSC_RET_SUCCESS;
}
static EGSC_RET_CODE mydev_fac_lift_ba_status_cb(int handle, egsc_dev_cb_fac_lift_ba_status_param *fac_status_param)
{
    int loop = 0;
    egsc_dev_cb_fac_lift_car_status_param *lift_car_status;
    user_dev_info *user_dev = NULL;

    egsc_log_debug("enter.\n");
    for(loop=0;loop<fac_status_param->car_id_list_len;loop++)
    {
        egsc_log_user("carIDList(%d)=%d.\n", loop, fac_status_param->car_id_list[loop]);
    }

    egsc_log_user("timestamp(%s).\n", fac_status_param->time_stamp);

    user_dev_get_handle(&s_mydev_dev_list_head, handle, &user_dev);
    if(NULL != user_dev)
    {
        fac_status_param->res_lift_car_num = 2;
        if(NULL != fac_status_param->lift_car_status)
        {
            lift_car_status = fac_status_param->lift_car_status;
            snprintf(lift_car_status[0].subdev_id.subdev_mac, sizeof(lift_car_status[0].subdev_id.subdev_mac), "%s", user_dev->dev_info.id);
            lift_car_status[0].car_id = 1;
            lift_car_status[0].physical_floor = 1;
            snprintf(lift_car_status[0].display_floor, sizeof(lift_car_status[0].display_floor), "%s", "14A");
            snprintf(lift_car_status[0].car_status, sizeof(lift_car_status[0].car_status), "%s", "11");
            snprintf(lift_car_status[0].door_status, sizeof(lift_car_status[0].door_status), "%s", "11");
            lift_car_status[0].error_status = 1;
            snprintf(lift_car_status[0].error_message, sizeof(lift_car_status[0].error_message), "%s", "error");
            lift_car_status[0].fire_ctrl_status = 1;
            snprintf(lift_car_status[1].subdev_id.subdev_mac, sizeof(lift_car_status[1].subdev_id.subdev_mac), "%s", user_dev->dev_info.id);
            lift_car_status[1].car_id = 2;
            lift_car_status[1].physical_floor = 2;
            snprintf(lift_car_status[1].display_floor, sizeof(lift_car_status[1].display_floor), "%s", "14A");
            snprintf(lift_car_status[1].car_status, sizeof(lift_car_status[1].car_status), "%s", "22");
            snprintf(lift_car_status[1].door_status, sizeof(lift_car_status[1].door_status), "%s", "22");
            lift_car_status[1].error_status = 2;
            snprintf(lift_car_status[1].error_message, sizeof(lift_car_status[1].error_message), "%s", "error");
            lift_car_status[1].fire_ctrl_status = 2;
        }
    }
    else
    {
        fac_status_param->res_lift_car_num = 0;
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_pak_send_showinfo_cb(int handle, egsc_subdev_id *p_dev_id, char *text)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) pak send showinfo\n", device_id_dst);
    egsc_log_user("text: %s \n", text);

    snprintf(s_showinfo, sizeof(s_showinfo), "%s", text);
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_pak_control_lock_cb(int handle, char *place_no, char *placelock_no, int operate_type)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) pak control lock\n", device_id_dst);
    egsc_log_user("placeNo(%s).\n", place_no);
    egsc_log_user("placeLockNo(%s).\n", placelock_no);
    egsc_log_user("operateType(%d).\n", operate_type);

    switch(operate_type)
    {
        case 0:
        {
            egsc_log_user("lock: landing \n");
            break;
        }
        case 1:
        {
            egsc_log_user("lock: rising \n");
            break;
        }
        default:
        {
            egsc_log_user("lock: landing \n");
            break;
        }
        s_lock_optype = operate_type;
    }
    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_boun_chan_setupalarm_cb(int handle, int setup_type, char *alarmzone_chan)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) boun chan setupalarm\n", device_id_dst);
    egsc_log_user("setupType(%d).\n", setup_type);
    egsc_log_user("alarmZoneChan(%s).\n", alarmzone_chan);

    s_setup_type = setup_type;
    if(1 == s_setup_type)
    {
        egsc_log_user("setup_type: withdrawal alarm\n");
    }
    else
    {
        egsc_log_user("setup_type: setup alarm\n");
    }
    snprintf(s_alarmzone_chan, sizeof(s_alarmzone_chan), "%s", alarmzone_chan);
    egsc_log_user("alarmzone_chan: %s \n", s_alarmzone_chan);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_boun_subchan_clearalarm_cb(int handle, int subsystem_num)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) boun subchan_clearalarm\n", device_id_dst);
    egsc_log_user("subSystemNum(%d).\n", subsystem_num);

    s_subsystem_num = subsystem_num;

    egsc_log_user("boun subchan clearalarm(subsystemnum:%d) \n", s_subsystem_num);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_query_dev_status_cb(int handle, egsc_subdev_id *p_dev_id, int *state)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, p_dev_id, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) query dev status\n", device_id_dst);
    egsc_log_debug("state(%s).\n", s_mydev_status);

    *state = s_mydev_status;
    if(NULL != p_dev_id)
    {
        egsc_log_debug("device_inner_id[%04d][%s][%04d]\n", p_dev_id->subdev_type, p_dev_id->subdev_mac, p_dev_id->subdev_num);
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_snap_picture_cb(int handle, char *image_id, int len)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) snap picture\n", device_id_dst);

    if(NULL != image_id && len > 10)
    {
        memcpy(image_id, "1234567890", strlen("1234567890"));
    }
    else
    {
        egsc_log_debug("param error\n");
        return EGSC_RET_ERROR;
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_pak_reset_dg_cb(int handle)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) pak reset dg\n", device_id_dst);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_pak_led_display_cb(int handle, char *text)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) pak led display\n", device_id_dst);

    if(NULL != text)
    {
        egsc_log_user("text[%s]\n", text);
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_pak_load_led_info_cb(int handle, char *text)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) pak load led info\n", device_id_dst);

    if(NULL != text)
    {
        egsc_log_user("text[%s]\n", text);
    }

    return EGSC_RET_SUCCESS;
}

#if 0
static EGSC_RET_CODE mydev_pak_control_lock_cb(int handle, char *place_no, char *place_lockno, int op_type)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);

    if((NULL != place_no) && (NULL != place_lockno))
    {
        egsc_log_user(">>op_type[%s][%s][%d]\n", place_no, place_lockno, op_type);
    }

    return EGSC_RET_SUCCESS;
}
#endif

static EGSC_RET_CODE mydev_rsp_pak_intercom_control_cb(int handle, int command_type, char *sdp)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) pak intercom control\n", device_id_dst);

    egsc_log_user("command_type[%d]\n", command_type);
    egsc_log_user("sdp[%s]\n", sdp);

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_rsp_ads_trans_material_cb(int handle, egsc_dev_trans_material *material_param)
{
    int i = 0;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads trans materiall\n", device_id_dst);

    if(NULL != material_param)
    {
        egsc_log_user("scheduleId(%d).\n", material_param->schedule_id);
        egsc_log_user("seq(%d).\n", material_param->seq);
        for(i = 0; i < material_param->terminal_cnt; i++)
        {
            egsc_log_user("terminalNo[%d][%s]\n", i, material_param->terminal_no[i].id);
        }
        for(i = 0; i < material_param->material_cnt; i++)
        {
            egsc_log_user("material[%d].accountName(%s)\n", i, material_param->material[i].account_name);
            egsc_log_user("material[%d].accountPasswd(%s)\n", i, material_param->material[i].account_passwd);
            egsc_log_user("material[%d].materialId(%d)\n", i, material_param->material[i].material_id);
            egsc_log_user("material[%d].url(%s)\n", i, material_param->material[i].url);
            egsc_log_user("material[%d].materialName(%s)\n", i, material_param->material[i].material_name);
            egsc_log_user("material[%d].materialEncrypt(%s)\n", i, material_param->material[i].material_encrypt);
            egsc_log_user("material[%d].materialType(%s)\n", i, material_param->material[i].material_type);
            egsc_log_user("material[%d].format(%s)\n", i, material_param->material[i].format);
            egsc_log_user("material[%d].fileSize(%d)\n", i, material_param->material[i].file_size);
            egsc_log_user("material[%d].duration(%d)\n", i, material_param->material[i].duration);
            egsc_log_user("material[%d].character(%s)\n", i, material_param->material[i].character);
        }
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_rsp_ads_query_term_cb(int handle, egsc_dev_ads_query_term *query_param, egsc_query_recv *recv_param)
{
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads query term\n", device_id_dst);

    if(NULL != query_param)
    {
        egsc_log_user("searchId(%d).\n", query_param->search_id);
        egsc_log_user("terminalNameLike(%s).\n", query_param->terminal_name_like);
        egsc_log_user("terminalRemarksLike(%s).\n", query_param->terminal_remarks_like);
        egsc_log_user("onlineState(%s).\n", query_param->online_state);
        egsc_log_user("PublishState(%s).\n", query_param->publish_state);
        egsc_log_user("PlayState(%s).\n", query_param->play_state);
        egsc_log_user("maxResults(%d).\n", query_param->max_result);
        egsc_log_user("searchResultsPosition(%d).\n", query_param->search_results_position);
        egsc_log_user("serial[%s][%s][%d].\n", query_param->serial->subdev_type,
            query_param->serial->subdev_mac, query_param->serial->subdev_num);
    }

    if(NULL != recv_param)
    {
        memcpy(recv_param->search_id, "12345", strlen("12345"));
        memcpy(recv_param->response_status, "true", strlen("true"));
        memcpy(recv_param->response_status_string, "OK", strlen("OK"));
        recv_param->total_matchs = 3;
        recv_param->num_of_matchs = 100;
        recv_param->terminal_info.id = 123;
        memcpy(recv_param->terminal_info.terminal_name, "terminal_name", strlen("terminal_name"));
        memcpy(recv_param->terminal_info.terminal_type, "terminal_type", strlen("terminal_type"));
        memcpy(recv_param->terminal_info.terminal_remarks, "terminal_remarks", strlen("terminal_remarks"));
        memcpy(recv_param->terminal_info.terminal_remarks_like, "terminal_remarks_like", strlen("terminal_remarks_like"));
        memcpy(recv_param->terminal_info.org_name, "org_name", strlen("org_name"));
        memcpy(recv_param->terminal_info.online_state, "online_state", strlen("online_state"));

        memcpy(recv_param->terminal_info.ip_address.ip_version, "ip_version", strlen("ip_version"));
        memcpy(recv_param->terminal_info.ip_address.ip_address, "192.168.1.11", strlen("192.168.1.11"));

        recv_param->terminal_info.port = 7888;
        memcpy(recv_param->terminal_info.serial_no, "serial_no", strlen("serial_no"));
        memcpy(recv_param->terminal_info.software_version, "111.1", strlen("111.1"));
        memcpy(recv_param->terminal_info.publish_state, "publish_state", strlen("publish_state"));
        memcpy(recv_param->terminal_info.insert_state, "insert_state", strlen("insert_state"));
        memcpy(recv_param->terminal_info.play_state, "play_state", strlen("play_state"));

        memcpy(recv_param->terminal_info.resolution.width, "1080", strlen("1080"));
        memcpy(recv_param->terminal_info.resolution.height, "720", strlen("720"));
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_rsp_ads_add_program_cb(int handle, egsc_dev_ads_program *param, int *program_id)
{
    int i;
    int program_id_inner = 0;
    int plat_i = 0;
    int win_i = 0;
    int plat_cnt = 0;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads add program\n", device_id_dst);

    if(NULL != param)
    {
        program_id_inner = param->id;
        egsc_log_user("id(%d).\n", param->id);
        egsc_log_user("programName(%s).\n", param->program_name);
        egsc_log_user("programRemark(%s).\n", param->program_remark);
        egsc_log_user("approveState(%s).\n", param->approve_state);
        egsc_log_user("programType(%s).\n", param->program_type);
        egsc_log_user("imageWidth(%d).\n", param->image_width);
        egsc_log_user("imageHeight(%d).\n", param->image_height);
        for(i = 0; i < param->pag_list_cnt; i++)
        {
            egsc_log_user("Paglist[%d].id[%d]\n", i, param->pag_list[i].id);
            egsc_log_user("Paglist[%d].PagBaseInfo.pagName[%s]\n", i,
                param->pag_list[i].pag_base_info.pag_name);
            egsc_log_user("Paglist[%d].PagBaseInfo.backgroundColor[%d]\n", i,
                param->pag_list[i].pag_base_info.back_ground_color);
            egsc_log_user("Paglist[%d].PagBaseInfo.playTimeMod[%s]\n", i,
                param->pag_list[i].pag_base_info.play_time_mod);
            egsc_log_user("Paglist[%d].PagBaseInfo.playTime[%d]\n", i,
                param->pag_list[i].pag_base_info.play_time);
            egsc_log_user("Paglist[%d].PagBaseInfo.backgroundPidId[%d]\n", i,
                param->pag_list[i].pag_base_info.back_ground_pid_id);

            for(win_i = 0; win_i < param->pag_list[i].windows_list_len; win_i++)
            {
                egsc_log_user("Paglist[%d].WindowsList[%d].id[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].id);
                egsc_log_user("Paglist[%d].WindowsList[%d].layerNo[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].layer_no);
                egsc_log_user("Paglist[%d].WindowsList[%d].Position.xPosition[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].position.x_position);
                egsc_log_user("Paglist[%d].WindowsList[%d].Position.yPosition[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].position.y_position);
                egsc_log_user("Paglist[%d].WindowsList[%d].Position.height[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].position.height);
                egsc_log_user("Paglist[%d].WindowsList[%d].Position.width[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].position.width);
                egsc_log_user("Paglist[%d].WindowsList[%d].WinMateriaInfo.winMateriaType[%s]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].win_material_info.win_material_type);
                egsc_log_user("Paglist[%d].WindowsList[%d].WinMateriaInfo.staticMaterialType[%s]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].win_material_info.static_material_type);
                egsc_log_user("Paglist[%d].WindowsList[%d].plat_item_cnt[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].plat_item_cnt);
                plat_cnt = param->pag_list[i].windows_list[win_i].plat_item_cnt;
                for(plat_i = 0; plat_i < plat_cnt; plat_i++)
                {
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].id[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].id);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].materialNo[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].material_no);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].PageTime[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].page_time);

                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.fontsize[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.font_size);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.color[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.color);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.backColor[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.back_color);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.backTransparent[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.back_transparent);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.scrollDirection[%s]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.scroll_direction);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.scrolSpeed[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.scrol_speed);

                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].PlayDuration.durationType[%s]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].play_duration.duration_type);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].PlayDuration.duration[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].play_duration.duration);
                }
            }
        }
    }

    if(NULL != program_id)
    {
        *program_id = program_id_inner;
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_rsp_ads_delete_program_cb(int handle, egsc_dev_ads_delete *param)
{
    int i = 0;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads delete program\n", device_id_dst);

    if(NULL != param && NULL != param->id)
    {
        for(i = 0; i < param->del_cnt; i++)
        {
            egsc_log_user("IdList[%d].id(%d).\n", i, param->id[i]);
        }
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_rsp_ads_set_program_cb(int handle, egsc_dev_ads_program *param, int *program_id)
{
    int i;
    int program_id_inner = 0;
    int plat_i = 0;
    int win_i = 0;
    int plat_cnt = 0;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads delete program\n", device_id_dst);

    if(NULL != param)
    {
        program_id_inner = param->id;
        egsc_log_user("id(%d).\n", param->id);
        egsc_log_user("programName(%s).\n", param->program_name);
        egsc_log_user("programRemark(%s).\n", param->program_remark);
        egsc_log_user("approveState(%s).\n", param->approve_state);
        egsc_log_user("programType(%s).\n", param->program_type);
        egsc_log_user("imageWidth(%d).\n", param->image_width);
        egsc_log_user("imageHeight(%d).\n", param->image_height);
        for(i = 0; i < param->pag_list_cnt; i++)
        {
            egsc_log_user("Paglist[%d].id[%d]\n", i, param->pag_list[i].id);
            egsc_log_user("Paglist[%d].PagBaseInfo.pagName[%s]\n", i,
                param->pag_list[i].pag_base_info.pag_name);
            egsc_log_user("Paglist[%d].PagBaseInfo.backgroundColor[%d]\n", i,
                param->pag_list[i].pag_base_info.back_ground_color);
            egsc_log_user("Paglist[%d].PagBaseInfo.playTimeMod[%s]\n", i,
                param->pag_list[i].pag_base_info.play_time_mod);
            egsc_log_user("Paglist[%d].PagBaseInfo.playTime[%d]\n", i,
                param->pag_list[i].pag_base_info.play_time);
            egsc_log_user("Paglist[%d].PagBaseInfo.backgroundPidId[%d]\n", i,
                param->pag_list[i].pag_base_info.back_ground_pid_id);

            for(win_i = 0; win_i < param->pag_list[i].windows_list_len; win_i++)
            {
                egsc_log_user("Paglist[%d].WindowsList[%d].id[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].id);
                egsc_log_user("Paglist[%d].WindowsList[%d].layerNo[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].layer_no);
                egsc_log_user("Paglist[%d].WindowsList[%d].Position.xPosition[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].position.x_position);
                egsc_log_user("Paglist[%d].WindowsList[%d].Position.yPosition[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].position.y_position);
                egsc_log_user("Paglist[%d].WindowsList[%d].Position.height[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].position.height);
                egsc_log_user("Paglist[%d].WindowsList[%d].Position.width[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].position.width);
                egsc_log_user("Paglist[%d].WindowsList[%d].WinMateriaInfo.winMateriaType[%s]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].win_material_info.win_material_type);
                egsc_log_user("Paglist[%d].WindowsList[%d].WinMateriaInfo.staticMaterialType[%s]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].win_material_info.static_material_type);
                egsc_log_user("Paglist[%d].WindowsList[%d].plat_item_cnt[%d]\n", i,
                    win_i, param->pag_list[i].windows_list[win_i].plat_item_cnt);
                plat_cnt = param->pag_list[i].windows_list[win_i].plat_item_cnt;
                for(plat_i = 0; plat_i < plat_cnt; plat_i++)
                {
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].id[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].id);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].materialNo[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].material_no);
                    egsc_log_user("pag_list[%d].WindowsList[%d].PlatItemList[%d].PageTime[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].page_time);

                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.fontsize[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.font_size);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.color[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.color);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.backColor[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.back_color);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.backTransparent[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.back_transparent);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.scrollDirection[%s]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.scroll_direction);
                    egsc_log_user("Paglist[%d].WindowsList[%d].PlatItemList[%d].CharacterEffect.scrolSpeed[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].character_effect.scrol_speed);

                    egsc_log_info("Paglist[%d].WindowsList[%d].PlatItemList[%d].PlayDuration.durationType[%s]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].play_duration.duration_type);
                    egsc_log_info("Paglist[%d].WindowsList[%d].PlatItemList[%d].PlayDuration.duration[%d]\n", i, win_i, plat_i,
                        param->pag_list[i].windows_list[win_i].plat_item_list[plat_i].play_duration.duration);
                }
            }
        }
    }

    if(NULL != program_id)
    {
        *program_id = program_id_inner;
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_rsp_ads_add_schedule_cb(int handle, egsc_dev_ads_schedule *param, int *schedule_id)
{
    int i = 0;
    int j = 0;
    int schedule_id_in = 0;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads add schedule\n", device_id_dst);

    if(NULL != param)
    {
        schedule_id_in = param->id;
        egsc_log_user("id(%d).\n", param->id);
        egsc_log_user("scheduleName(%s).\n", param->schedule_name);
        egsc_log_user("scheduleRemarks(%s).\n", param->schedule_remarks);
        egsc_log_user("approveRemarks(%s).\n", param->approve_remarks);
        egsc_log_user("approveState(%s).\n", param->approve_state);
        egsc_log_user("scheduleMode(%s).\n", param->schedule_mode);
        egsc_log_user("scheduleType(%s).\n", param->schedule_type);

        for(i = 0; i < param->daily_cnt; i++)
        {
            egsc_log_user("DailySchedule[%d].id(%d).\n", i, param->daily_schedule[i].id);
            egsc_log_user("DailySchedule[%d].programNo(%d).\n", i, param->daily_schedule[i].program_no);
            egsc_log_user("DailySchedule[%d].begintime(%s).\n", i, param->daily_schedule[i].begin_time);
            egsc_log_user("DailySchedule[%d].endtime(%s).\n", i, param->daily_schedule[i].end_time);
        }
        for(j = 0; j < param->weeky_cnt; j++)
        {
            egsc_log_user("WeekySchedule[%d].id(%d).\n", j, param->weeky_schedule[j].id);
            egsc_log_user("WeekySchedule[%d].DayOfWeek(%s).\n", j, param->weeky_schedule[j].day_of_week);
            for(i = 0; i < param->weeky_schedule[j].weeky_daily_cnt; i++)
            {
                egsc_log_user("WeekySchedule[%d].DailySchedule[%d].id(%d).\n", j, i,
                    param->weeky_schedule[j].daily_schedule[i].id);
                egsc_log_user("WeekySchedule[%d].DailySchedule[%d].programNo(%d).\n", j, i,
                    param->weeky_schedule[j].daily_schedule[i].program_no);
                egsc_log_user("WeekySchedule[%d].DailySchedule[%d].begintime(%s).\n", j, i,
                    param->weeky_schedule[j].daily_schedule[i].begin_time);
                egsc_log_user("WeekySchedule[%d].DailySchedule[%d].endtime(%s).\n", j, i,
                    param->weeky_schedule[j].daily_schedule[i].end_time);
            }
        }
    }
    egsc_log_user("priority(%d).\n", param->priority);
    egsc_log_user("startDate(%s).\n", param->start_date);
    egsc_log_user("endDate(%s).\n", param->end_date);

    if(NULL != schedule_id)
    {
        *schedule_id = schedule_id_in;
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_rsp_ads_delete_schedule_cb(int handle, egsc_dev_ads_delete *param)
{
    int i = 0;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads delete schedule\n", device_id_dst);

    if(NULL != param)
    {
        for(i = 0; i < param->del_cnt; i++)
        {
            egsc_log_user("IdList[%d].id(%d).\n", i, param->id[i]);
        }
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_rsp_ads_set_schedule_cb(int handle, egsc_dev_ads_schedule *param, int *schedule_id)
{
    int i = 0;
    int j = 0;
    int schedule_id_in = 0;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads set schedule\n", device_id_dst);

    if(NULL != param)
    {
        schedule_id_in = param->id;
        egsc_log_user("id(%d).\n", param->id);
        egsc_log_user("scheduleName(%s).\n", param->schedule_name);
        egsc_log_user("scheduleRemarks(%s).\n", param->schedule_remarks);
        egsc_log_user("approveRemarks(%s).\n", param->approve_remarks);
        egsc_log_user("approveState(%s).\n", param->approve_state);
        egsc_log_user("scheduleMode(%s).\n", param->schedule_mode);
        egsc_log_user("scheduleType(%s).\n", param->schedule_type);

        for(i = 0; i < param->daily_cnt; i++)
        {
            egsc_log_user("DailySchedule[%d].id(%d).\n", i, param->daily_schedule[i].id);
            egsc_log_user("DailySchedule[%d].programNo(%d).\n", i, param->daily_schedule[i].program_no);
            egsc_log_user("DailySchedule[%d].begintime(%s).\n", i, param->daily_schedule[i].begin_time);
            egsc_log_user("DailySchedule[%d].endtime(%s).\n", i, param->daily_schedule[i].end_time);
        }
        for(j = 0; j < param->weeky_cnt; j++)
        {
            egsc_log_user("WeekySchedule[%d].id(%d).\n", j, param->weeky_schedule[j].id);
            egsc_log_user("WeekySchedule[%d].DayOfWeek(%s).\n", j, param->weeky_schedule[j].day_of_week);
            for(i = 0; i < param->weeky_schedule[j].weeky_daily_cnt; i++)
            {
                egsc_log_user("WeekySchedule[%d].DailySchedule[%d].id(%d).\n", j, i,
                    param->weeky_schedule[j].daily_schedule[i].id);
                egsc_log_user("WeekySchedule[%d].DailySchedule[%d].programNo(%d).\n", j, i,
                    param->weeky_schedule[j].daily_schedule[i].program_no);
                egsc_log_user("WeekySchedule[%d].DailySchedule[%d].begintime(%s).\n", j, i,
                    param->weeky_schedule[j].daily_schedule[i].begin_time);
                egsc_log_user("WeekySchedule[%d].DailySchedule[%d].endtime(%s).\n", j, i,
                    param->weeky_schedule[j].daily_schedule[i].end_time);
            }
        }
    }
    egsc_log_user("priority(%d).\n", param->priority);
    egsc_log_user("startDate(%s).\n", param->start_date);
    egsc_log_user("endDate(%s).\n", param->end_date);

    if(NULL != schedule_id)
    {
        *schedule_id = schedule_id_in;
    }

    return EGSC_RET_SUCCESS;
}


static EGSC_RET_CODE mydev_rsp_ads_publish_schedule_cb(int handle, egsc_dev_ads_publish_schedule *param, int *schedule_id)
{
    int i = 0;
    int schedule_id_in = 0;
    char device_id_dst[32];

    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    memset(device_id_dst, 0, sizeof(device_id_dst));
    mydev_get_dev_id_str(&s_mydev_dev_list_head, handle, NULL, device_id_dst, sizeof(device_id_dst));
    egsc_log_user("device(%s) ads publish schedule\n", device_id_dst);

    if(NULL != param)
    {
        schedule_id_in = param->id;
        egsc_log_user("id(%d).\n", param->id);
        egsc_log_user("effectiveTime(%s).\n", param->effective_time);
        egsc_log_user("releaseType(%s).\n", param->release_type);
        for(i = 0; i < param->list_cnt; i++)
        {
            egsc_log_user("TerminalNoList[%d].terminalNo[%04d][%s][%04d].\n", i,
                param->terminal_nolist[i].terminal_no->subdev_type,
                param->terminal_nolist[i].terminal_no->subdev_mac,
                param->terminal_nolist[i].terminal_no->subdev_num);
        }
    }

    if(NULL != schedule_id)
    {
        *schedule_id = schedule_id_in;
    }

    return EGSC_RET_SUCCESS;
}



// mydev设备 处理服务器回复的回调
static void mydev_upload_dev_status_res_cb(int handle, int req_id, EGSC_RET_CODE ret)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("req_id(%d).\n", req_id);
    egsc_log_debug("ret(%d).\n",ret);
}

static void mydev_upload_record_res_cb(int handle, int req_id, EGSC_RET_CODE ret)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("req_id(%d).\n", req_id);
    egsc_log_debug("ret(%d).\n",ret);
	char buf[MAXBUF+1];
	bzero(buf, MAXBUF + 1);
	snprintf(buf,MAXBUF+1,"Server Response ret = %d",ret);
	int len = send(new_fd, buf, strlen(buf), 0);
	if (len >= 0)
		printf("消息:%s\t发送成功，共发送了%d个字节！\n", buf, len);
	else{
		printf("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n",buf, errno, strerror(errno));
	}
}

static void mydev_upload_event_res_cb(int handle, int req_id, EGSC_RET_CODE ret)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("req_id(%d).\n", req_id);
    egsc_log_debug("ret(%d).\n",ret);
}
static void mydev_upload_credence_load_result_cb(int handle, int req_id, EGSC_RET_CODE ret)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("req_id(%d).\n", req_id);
    egsc_log_debug("ret(%d).\n",ret);
}
static void mydev_upload_fac_dev_status_res_cb(int handle, int req_id, EGSC_RET_CODE ret)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("req_id(%d).\n", req_id);
    egsc_log_debug("ret(%d).\n",ret);
}
static void mydev_upload_fac_elevator_record_res_cb(int handle, int req_id, EGSC_RET_CODE ret)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("req_id(%d).\n", req_id);
    egsc_log_debug("ret(%d).\n",ret);
}
static void mydev_upload_fac_ba_status_res_cb(int handle, int req_id, EGSC_RET_CODE ret)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("req_id(%d).\n", req_id);
    egsc_log_debug("ret(%d).\n",ret);
}

static void mydev_req_pak_intercom_control_cb(int handle, int req_id, EGSC_RET_CODE ret)
{
    egsc_log_debug("enter.\n");
    egsc_log_debug("handle(%d).\n", handle);
    egsc_log_debug("req_id(%d).\n", req_id);
    egsc_log_debug(">>ret(%d).\n",ret);
}

static EGSC_RET_CODE mydev_upload_dev_status(char *dev_id, char *dev_status)
{
    int ret = -1;
    int device_type = 1;
    int lock_device_id = -1;
    int device_status = 1;
    mydev_json_obj status_obj = NULL;
    char device_id_inner[32] = "";
    user_dev_info *user_dev = NULL;
    egsc_subdev_id subdev_id;
    egsc_subdev_id *p_subdev_id = NULL;

    egsc_dev_upload_dev_status_param status_param;
    if( (NULL == dev_status) ||
        (NULL == dev_id))
    {
        egsc_log_error("input device status/id NULL.\n");
        return EGSC_RET_ERROR;
    }

    memset(&subdev_id, 0x00, sizeof(subdev_id));
    if(EGSC_RET_ERROR == mydev_get_subdev_id(dev_id, &subdev_id, &p_subdev_id))
    {
        return EGSC_RET_ERROR;
    }
    egsc_log_debug("dev id[%04d][%s][%04d]\n", subdev_id.subdev_type, subdev_id.subdev_mac, subdev_id.subdev_num);

    user_dev_get_id(&s_mydev_dev_list_head, subdev_id.subdev_mac, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_user("no found dev(%s), skip report.\n", subdev_id.subdev_mac);
        return EGSC_RET_ERROR;
    }

    status_obj = mydev_json_parse(dev_status);
    if(NULL == status_obj)
    {
        egsc_log_user("parser dev status input(%s) failed, skip report.\n", dev_status);
        return EGSC_RET_ERROR;
    }

    memset(&status_param, 0, sizeof(status_param));
    memset(device_id_inner, 0, sizeof(device_id_inner));
    mydev_json_get_string(status_obj, "deviceID", device_id_inner, sizeof(device_id_inner));
    mydev_json_get_int(status_obj, "DeviceType", &device_type);
    mydev_json_get_int(status_obj, "lockDeviceID", &lock_device_id);
    mydev_json_get_int(status_obj, "DeviceStatus", &device_status);

    if( (strlen(device_id_inner)==0) ||
        (0 == device_type) ||
        (device_status < 0) )
    {
        mydev_json_clear(status_obj);
        status_obj = NULL;

        egsc_log_user("parser dev status input(%s) get deviceID/DeviceType/DeviceStatus failed ,please confirm.\n", dev_status);
        return EGSC_RET_ERROR;
    }

    if(strcmp(device_id_inner, dev_id) != 0)
    {
        mydev_json_clear(status_obj);
        status_obj = NULL;

        egsc_log_user("deviceID not match, skip report.\n");
        return EGSC_RET_ERROR;
    }

    ret = user_check_dev_device_type(user_dev, device_type);
    if(ret < 0)
    {
        mydev_json_clear(status_obj);
        status_obj = NULL;

        egsc_log_user("dev type(%04d) not match device(id:%s), skip report.\n", device_type, user_dev->dev_info.id);
        return EGSC_RET_ERROR;
    }

    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_PARKING_CTRL:
        {
            if(EGSC_SUBTYPE_PARKING_FLOOR_SENSOR == device_type)
            {
                ret = user_check_parking_floor_sensor_device_status(device_status);
                if(ret < 0)
                {
                    mydev_json_clear(status_obj);
                    status_obj = NULL;

                    egsc_log_user("DeviceStatus(%d) not match device(id:%s), skip report.\n", device_status, user_dev->dev_info.id);
                    return EGSC_RET_ERROR;
                }
            }
            if(EGSC_SUBTYPE_PARKING_BARRIER_GATE == device_type)
            {
                ret = user_check_parking_barrier_gate_device_status(device_status);
                if(ret < 0)
                {
                    mydev_json_clear(status_obj);
                    status_obj = NULL;

                    egsc_log_user("DeviceStatus(%d) not match device(id:%s), skip report.\n", device_status, user_dev->dev_info.id);
                    return EGSC_RET_ERROR;
                }
            }
            break;
        }
        case EGSC_TYPE_DOOR_CTRL:
        case EGSC_TYPE_GATE_CTRL:
        {
            break;
        }
        case EGSC_TYPE_ELE_LINK_CTRL:
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            break;
        }
        case EGSC_TYPE_SCREEN_CTRL:
        {
            ret = user_check_information_screen_device_status(device_status);
            if(ret < 0)
            {
                mydev_json_clear(status_obj);
                status_obj = NULL;

                egsc_log_user("DeviceStatus(%d) not match device(id:%s), skip report.\n", device_status, user_dev->dev_info.id);
                return EGSC_RET_ERROR;
            }
            break;
        }
        case EGSC_TYPE_SMART_CTRL_KB:
        {
            break;
        }
        case EGSC_TYPE_ELEC_LPN_CTRL:
        {
            break;
        }
        case EGSC_TYPE_PARKING_LOCK_CONTROLLER:
        {
            if(EGSC_TYPE_PARKING_LOCK_CONTROLLER == device_type)
            {
                ret = user_check_parking_lock_controller_device_status(device_status);
                if(ret < 0)
                {
                    mydev_json_clear(status_obj);
                    status_obj = NULL;

                    egsc_log_user("DeviceStatus(%d) not match device(id:%s), skip report.\n", device_status, user_dev->dev_info.id);
                    return EGSC_RET_ERROR;
                }
            }
            if(EGSC_SUBTYPE_PARKING_LOCK == device_type)
            {
                ret = user_check_parking_lock_device_status(device_status);
                if(ret < 0)
                {
                    mydev_json_clear(status_obj);
                    status_obj = NULL;

                    egsc_log_user("DeviceStatus(%d) not match device(id:%s), skip report.\n", device_status, user_dev->dev_info.id);
                    return EGSC_RET_ERROR;
                }
            }            
            break;
        }
        default:
        {
            break;
        }
    }
    status_param.device_type = device_type;
    status_param.lock_device_id = lock_device_id;
    status_param.device_status = device_status;
    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_DOOR_CTRL:
        {
            s_door_ctrl_req_if_tbl.upload_dev_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_dev_status_res_cb, &status_param);
            break;
        }
        case EGSC_TYPE_ELEC_LPN_CTRL:
        {
            s_elec_lpn_req_if_tbl.upload_dev_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_dev_status_res_cb, &status_param);
            break;
        }
        case EGSC_TYPE_PARKING_LOCK_CONTROLLER:
        {
            s_parking_lock_req_if_tbl.upload_dev_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_dev_status_res_cb, &status_param);
            break;
        }
        case EGSC_TYPE_SMART_CTRL_KB:
        {
            s_smart_ctrl_req_if_tbl.upload_dev_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_dev_status_res_cb, &status_param);
            break;
        }
        case EGSC_TYPE_ELE_LINK_CTRL:
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            s_elevator_req_if_tbl.upload_dev_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_dev_status_res_cb, &status_param);
            break;
        }
        case EGSC_TYPE_PARKING_CTRL:
        {
            s_parking_ctrl_req_if_tbl.upload_dev_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_dev_status_res_cb, &status_param);
            break;
        }
        case EGSC_TYPE_SCREEN_CTRL:
        {
            s_screen_ctrl_req_if_tbl.upload_dev_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_dev_status_res_cb, &status_param);
            break;
        }
        default:
        {
            break;
        }
    }
    mydev_json_clear(status_obj);
    status_obj = NULL;

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_upload_record(char *dev_id, char *record)
{
    int ret = -1;
    char record_time[128] = "";
    int record_type = -1;
    int credence_type = -1;
    char user_id[128] = "";
    int user_type = -1;
    char recognise_capture_image[128] = "";
    int pass_type = -1;
    char credence_no[128] = "";
    char device_id[64] = "";
    user_dev_info *user_dev = NULL;
    egsc_subdev_id subdev_id;
    egsc_subdev_id *p_subdev_id = NULL;

    mydev_json_obj record_obj = NULL;
    egsc_dev_upload_record_param record_param;
    egsc_door_ctrl_upload_record_param door_record_param;
    egsc_parking_ctrl_upload_record_param parking_record_param;

    if( (NULL == record) ||
        (NULL == dev_id))
    {
        egsc_log_error("input record NULL.\n");
        return EGSC_RET_ERROR;
    }

    memset(&subdev_id, 0x00, sizeof(subdev_id));
    if(EGSC_RET_ERROR == mydev_get_subdev_id(dev_id, &subdev_id, &p_subdev_id))
    {
        return EGSC_RET_ERROR;
    }

    user_dev_get_id(&s_mydev_dev_list_head, subdev_id.subdev_mac, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_user("no found dev(%s), skip report.\n", subdev_id.subdev_mac);
        return EGSC_RET_ERROR;
    }

    record_obj = mydev_json_parse(record);
    if(NULL == record_obj)
    {
        egsc_log_user("parser dev record input(%s) failed, skip report.\n", record);
        return EGSC_RET_ERROR;
    }

    memset(user_id, 0, sizeof(user_id));
    memset(recognise_capture_image, 0, sizeof(recognise_capture_image));
    memset(credence_no, 0, sizeof(credence_no));
    memset(device_id, 0, sizeof(device_id));
    memset(&record_param, 0, sizeof(record_param));
    mydev_json_get_string(record_obj, "deviceID", device_id, sizeof(device_id));
    mydev_json_get_string(record_obj, "recordTime", record_time, sizeof(record_time));
    mydev_json_get_int(record_obj, "RecordType", &record_type);
    mydev_json_get_int(record_obj, "CredenceType", &credence_type);
    mydev_json_get_string(record_obj, "userID", user_id, sizeof(user_id));
    mydev_json_get_int(record_obj, "UserType", &user_type);
    mydev_json_get_string(record_obj, "recogniseCaptureImage", recognise_capture_image, sizeof(recognise_capture_image));
    if(EGSC_TYPE_DOOR_CTRL == user_dev->dev_info.dev_type)
    {
        memset(&door_record_param, 0, sizeof(door_record_param));
        mydev_json_get_int(record_obj, "passType", &pass_type);
        mydev_json_get_string(record_obj, "credenceNo", credence_no, sizeof(credence_no));
        if(pass_type < 0)
        {
            mydev_json_clear(record_obj);
            record_obj = NULL;

            egsc_log_user("parser record input(%s) get passType failed ,please confirm.\n", record);
            return EGSC_RET_ERROR;
        }
    }
    else if(EGSC_TYPE_PARKING_CTRL == user_dev->dev_info.dev_type)
    {
        memset(&parking_record_param, 0, sizeof(parking_record_param));
        parking_record_param.gate_open_mode = -1;
        parking_record_param.device_entry_type = -1;
        mydev_json_get_int(record_obj, "gateOpenMode", &parking_record_param.gate_open_mode);
        mydev_json_get_string(record_obj, "credenceNo", parking_record_param.credence_no, sizeof(parking_record_param.credence_no));
        mydev_json_get_string(record_obj, "recCarNOColor", parking_record_param.rec_car_no_color, sizeof(parking_record_param.rec_car_no_color));
        mydev_json_get_string(record_obj, "carLogo", parking_record_param.car_logo, sizeof(parking_record_param.car_logo));
        mydev_json_get_string(record_obj, "carType", parking_record_param.car_type, sizeof(parking_record_param.car_type));
        mydev_json_get_int(record_obj, "DeviceEntryType", &parking_record_param.device_entry_type);
        if(strlen(parking_record_param.credence_no) <= 0)
        {
            mydev_json_clear(record_obj);
            record_obj = NULL;

            egsc_log_user("parser record input(%s) get credenceNo failed ,please confirm.\n", record);
            return EGSC_RET_ERROR;
        }        
    }

    if( (strlen(device_id) == 0) ||
        (strlen(record_time) == 0) ||
        (-1 == record_type) ||
        (-1 == credence_type))
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;

        egsc_log_user("parser record input(%s) get deviceID/recordTime/RecordType/CredenceType failed ,please confirm.\n", record);
        return EGSC_RET_ERROR;
    }

    if(strcmp(device_id, dev_id) != 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;

        egsc_log_user("deviceID not match, skip report.\n");
        return EGSC_RET_ERROR;
    }

    ret = user_check_optime_valid(record_time);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;

        egsc_log_user("recordTime(%s) not valid, skip report.\n", record_time);
        return EGSC_RET_ERROR;
    }

    ret = user_check_record_type(record_type);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;

        egsc_log_user("RecordType(%d) not valid, skip report.\n", record_type);
        return EGSC_RET_ERROR;
    }

    ret = user_check_credence_type(credence_type);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;

        egsc_log_user("CredenceType(%d) not valid, skip report.\n", credence_type);
        return EGSC_RET_ERROR;
    }

    ret = user_certificate_check_credence_type(&s_mydev_cert_list_head, dev_id, credence_type);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;

        //egsc_log_user("dev_id(%s) CredenceType(%d) not match, skip report.\n", dev_id, credence_type);
        //return EGSC_RET_ERROR;
    }

    if(strlen(user_id) > 0)
    {
        ret = user_certificate_check_user_id(&s_mydev_cert_list_head, dev_id, user_id);
        if(ret < 0)
        {
            mydev_json_clear(record_obj);
            record_obj = NULL;

            egsc_log_user("dev_id(%s) userID(%s) not match, skip report.\n", dev_id, user_id);
            return EGSC_RET_ERROR;
        }
    }

    if(user_type >= 0)
    {
        ret = user_check_user_type(user_type);
        if(ret < 0)
        {
            mydev_json_clear(record_obj);
            record_obj = NULL;

            egsc_log_user("UserType(%d) not valid, skip report.\n", user_type);
            return EGSC_RET_ERROR;
        }
        ret = user_certificate_check_user_type(&s_mydev_cert_list_head, dev_id, user_type);
        if(ret < 0)
        {
            mydev_json_clear(record_obj);
            record_obj = NULL;

            egsc_log_user("dev_id(%s) UserType(%d) not match, skip report.\n", dev_id, user_type);
            return EGSC_RET_ERROR;
        }
    }

    if(EGSC_TYPE_DOOR_CTRL == user_dev->dev_info.dev_type)
    {
        ret = user_check_door_ctrl_pass_type(pass_type);
        if(ret < 0)
        {
            mydev_json_clear(record_obj);
            record_obj = NULL;

            egsc_log_user("passType(%d) not match, skip report.\n", pass_type);
            return EGSC_RET_ERROR;
        }
        if(strlen(credence_no) > 0)
        {
            ret = user_certificate_search(&s_mydev_cert_list_head, dev_id, credence_no);
            if(ret < 0)
            {
                mydev_json_clear(record_obj);
                record_obj = NULL;

                egsc_log_user("dev_id(%s) credenceNo(%s) not match, skip report.\n", dev_id, credence_no);
                return EGSC_RET_ERROR;
            }
        }
    }
    else if(EGSC_TYPE_PARKING_CTRL == user_dev->dev_info.dev_type)
    {
        if(parking_record_param.gate_open_mode >= 0)
        {
            ret = user_check_paking_ctrl_gate_open_mode(parking_record_param.gate_open_mode);
            if(ret < 0)
            {
                mydev_json_clear(record_obj);
                record_obj = NULL;

                egsc_log_user("gateOpenMode(%d) not valid, skip report.\n", parking_record_param.gate_open_mode);
                return EGSC_RET_ERROR;
            }
        }
        if(strlen(credence_no) > 0)
        {
            ret = user_certificate_search(&s_mydev_cert_list_head, dev_id, credence_no);
            if(ret < 0)
            {
                mydev_json_clear(record_obj);
                record_obj = NULL;

                egsc_log_user("dev_id(%s) credenceNo(%s) not match, skip report.\n", dev_id, credence_no);
                return EGSC_RET_ERROR;
            }
        }
        if(parking_record_param.device_entry_type >= 0)
        {
            ret = user_check_paking_ctrl_entry_type(parking_record_param.device_entry_type);
            if(ret < 0)
            {
                mydev_json_clear(record_obj);
                record_obj = NULL;

                egsc_log_user("DeviceEntryType(%d) not valid, skip report.\n", parking_record_param.device_entry_type);
                return EGSC_RET_ERROR;
            }
        }
    }

    record_param.record_time = record_time;
    record_param.record_type = record_type;
    record_param.credence_type = credence_type;
    record_param.user_id = user_id;
    record_param.user_type = user_type;
    record_param.recognise_capture_image = recognise_capture_image;
    if(EGSC_TYPE_DOOR_CTRL == user_dev->dev_info.dev_type)
    {
        door_record_param.pass_type = pass_type;
        door_record_param.credence_no = credence_no;
        record_param.dev_special_param = &door_record_param;
    }
    else if(EGSC_TYPE_PARKING_CTRL == user_dev->dev_info.dev_type)
    {
        record_param.dev_special_param = &parking_record_param;
    }
    else
    {
        record_param.dev_special_param = NULL;
    }
    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_DOOR_CTRL:
        {
            s_door_ctrl_req_if_tbl.upload_record_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_record_res_cb, &record_param);
            break;
        }
        case EGSC_TYPE_PARKING_CTRL:
        {
            s_parking_ctrl_req_if_tbl.upload_record_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_record_res_cb, &record_param);
            break;
        }
        default:
        {
            break;
        }
    }
    mydev_json_clear(record_obj);
    record_obj = NULL;

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_upload_event(char *dev_id, char *event)
{
    int ret = -1;
    int event_type = -1;
    char sub_dev_id[128] = "";
    char lock_no[128] = "";
    char address[128] = "";
    char time[128] = "";
    char description[128] = "";
    char pic_id[128] = "";
    char abs_time[128] = "";
    mydev_json_obj event_obj = NULL;
    user_dev_info *user_dev = NULL;
    egsc_subdev_id subdev_id;
    egsc_subdev_id *p_subdev_id = NULL;
    egsc_dev_upload_event_param event_param;
    egsc_smart_ctrl_upload_event_param smart_event_param;

    if( (NULL == event) ||
        (NULL == dev_id))
    {
        egsc_log_error("input event NULL.\n");
        return EGSC_RET_ERROR;
    }

    memset(&subdev_id, 0x00, sizeof(subdev_id));
    if(EGSC_RET_ERROR == mydev_get_subdev_id(dev_id, &subdev_id, &p_subdev_id))
    {
        return EGSC_RET_ERROR;
    }


    user_dev_get_id(&s_mydev_dev_list_head, subdev_id.subdev_mac, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_user("no found dev(%s), skip report.\n", subdev_id.subdev_mac);
        return EGSC_RET_ERROR;
    }

    event_obj = mydev_json_parse(event);
    if(NULL == event_obj)
    {
        egsc_log_user("parser dev record input(%s) failed, skip report.\n", event);
        return EGSC_RET_ERROR;
    }

    memset(&sub_dev_id, 0, sizeof(sub_dev_id));
    memset(&lock_no, 0, sizeof(lock_no));
    memset(&address, 0, sizeof(address));
    memset(&time, 0, sizeof(time));
    memset(&description, 0, sizeof(description));
    memset(&pic_id, 0, sizeof(pic_id));
    memset(&abs_time, 0, sizeof(abs_time));
    mydev_json_get_int(event_obj, "EventType", &event_type);
    if(EGSC_TYPE_PARKING_LOCK_CONTROLLER != user_dev->dev_info.dev_type)
    {
        mydev_json_get_string(event_obj, "subDeviceID", sub_dev_id, sizeof(sub_dev_id));
    }
    else
    {
        mydev_json_get_string(event_obj, "lockNo", lock_no, sizeof(lock_no));
    }
    mydev_json_get_string(event_obj, "address", address, sizeof(address));
    mydev_json_get_string(event_obj, "time", time, sizeof(time));
    mydev_json_get_string(event_obj, "description", description, sizeof(description));
    mydev_json_get_string(event_obj, "picID", pic_id, sizeof(pic_id));
    mydev_json_get_string(event_obj, "absTime", abs_time, sizeof(abs_time));

    if(EGSC_TYPE_SMART_CTRL_KB == user_dev->dev_info.dev_type)
    {
        memset(&smart_event_param, 0, sizeof(smart_event_param));
        mydev_json_get_string(event_obj, "alarmZoneName", smart_event_param.alarmzone_name, sizeof(smart_event_param.alarmzone_name));
        mydev_json_get_string(event_obj, "alarmZoneChan", smart_event_param.alarmzone_chan, sizeof(smart_event_param.alarmzone_chan));
        mydev_json_get_string(event_obj, "sensorType", smart_event_param.sensor_type, sizeof(smart_event_param.sensor_type));
    }

    if( (strlen(time) == 0) ||
        (0 == event_type) )
    {
        mydev_json_clear(event_obj);
        event_obj = NULL;

        egsc_log_user("parser event input(%s) get time/EventType failed ,please confirm.\n", event);
        return EGSC_RET_ERROR;
    }

    if(EGSC_TYPE_PARKING_LOCK_CONTROLLER != user_dev->dev_info.dev_type)
    {
        if( (strlen(sub_dev_id) > 0) &&
            (strcmp(sub_dev_id, dev_id) != 0))
        {
            mydev_json_clear(event_obj);
            event_obj = NULL;

            egsc_log_user("dev(%s) not match, skip report.\n", sub_dev_id);
            return EGSC_RET_ERROR;
        }
    }

    if(EGSC_TYPE_DOOR_CTRL == user_dev->dev_info.dev_type)
    {
        ret = user_check_door_ctrl_event_type(event_type);
        if(ret < 0)
        {
            ret = user_check_common_event_type(event_type);
            if(ret < 0)
            {
                mydev_json_clear(event_obj);
                event_obj = NULL;

                egsc_log_user("EventType(%d) not valid, skip report.\n", event_type);
                return EGSC_RET_ERROR;
            }
        }
    }
    else if(EGSC_TYPE_SMART_CTRL_KB == user_dev->dev_info.dev_type)
    {
        ret = user_check_smart_ctrl_event_type(event_type);
        if(ret < 0)
        {
            ret = user_check_common_event_type(event_type);
            if(ret < 0)
            {
                mydev_json_clear(event_obj);
                event_obj = NULL;

                egsc_log_user("EventType(%d) not valid, skip report.\n", event_type);
                return EGSC_RET_ERROR;
            }
        }
    }

    ret = user_check_optime_valid(time);
    if(ret < 0)
    {
        mydev_json_clear(event_obj);
        event_obj = NULL;

        egsc_log_user("time(%s) not valid, skip report.\n", time);
        return EGSC_RET_ERROR;
    }

    if(EGSC_TYPE_SMART_CTRL_KB == user_dev->dev_info.dev_type)
    {
        if(strlen(smart_event_param.sensor_type)>0)
        {
            ret = user_check_electric_fence_sensor_type(smart_event_param.sensor_type);
            if(ret < 0)
            {
                mydev_json_clear(event_obj);
                event_obj = NULL;

                egsc_log_user("sensorType(%s) not valid, skip report.\n", smart_event_param.sensor_type);
                return EGSC_RET_ERROR;
            }
        }
    }

    event_param.event_type = event_type;
    event_param.time = time;
    if((EGSC_TYPE_PARKING_LOCK_CONTROLLER == user_dev->dev_info.dev_type)
        || (EGSC_TYPE_PARKING_CTRL == user_dev->dev_info.dev_type))
    {
        event_param.lock_no = lock_no;
    }
    event_param.addr = address;
    event_param.desc = description;
    event_param.pic_id = pic_id;
    event_param.abs_time = abs_time;
    event_param.dev_special_param = NULL;
    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_DOOR_CTRL:
        {
            s_door_ctrl_req_if_tbl.upload_event_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_event_res_cb, &event_param);
            break;
        }
        case EGSC_TYPE_ELEC_LPN_CTRL:
        {
            s_elec_lpn_req_if_tbl.upload_event_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_event_res_cb, &event_param);
            break;
        }
        case EGSC_TYPE_PARKING_LOCK_CONTROLLER:
        {
            s_parking_lock_req_if_tbl.upload_event_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_event_res_cb, &event_param);
            break;
        }
        case EGSC_TYPE_SMART_CTRL_KB:
        {
            event_param.dev_special_param = &smart_event_param;
            s_smart_ctrl_req_if_tbl.upload_event_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_event_res_cb, &event_param);
            break;
        }
        case EGSC_TYPE_ELE_LINK_CTRL:
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            s_elevator_req_if_tbl.upload_event_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_event_res_cb, &event_param);
            break;
        }
        case EGSC_TYPE_PARKING_CTRL:
        {
            s_parking_ctrl_req_if_tbl.upload_event_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_event_res_cb, &event_param);
            break;
        }
        case EGSC_TYPE_SCREEN_CTRL:
        {
            s_screen_ctrl_req_if_tbl.upload_event_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_event_res_cb, &event_param);
            break;
        }
        default:
        {
            break;
        }
    }

    mydev_json_clear(event_obj);
    event_obj = NULL;

    return EGSC_RET_SUCCESS;
}
static int socketServerStart(unsigned int myport,unsigned int lisnum,char *serveraddr)
{
	int sockfd;
	socklen_t len;  
    struct sockaddr_in my_addr, their_addr;   
    char buf[MAXBUF + 1];  
    fd_set rfds;  
    struct timeval tv;  
    int retval, maxfd = -1;  
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)   
    {  
        perror("socket");  
        exit(1);  
    }  
    bzero(&my_addr, sizeof(my_addr));  
    my_addr.sin_family = PF_INET;  
    my_addr.sin_port = htons(myport);  
    printf("port=%d lisnum=%d\n",myport,lisnum);
    if (strlen(serveraddr)!=0){  
		printf("addr=%s\n",serveraddr);
        my_addr.sin_addr.s_addr = inet_addr(serveraddr);
    }
    else{  
        my_addr.sin_addr.s_addr = INADDR_ANY; 
		printf("addr is INADDR_ANY\n");
    }
          
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1)   
    {  
        perror("bind");  
        exit(1);  
    }  
      
    if (listen(sockfd, lisnum) == -1)   
    {  
        perror("listen");  
        exit(1);  
    }  
      
    while (1)   
    {  
        egsc_log_user("\n----Waiting for connecting……\n");  
        len = sizeof(struct sockaddr);  
          
        if ((new_fd =accept(sockfd, (struct sockaddr *) &their_addr,&len)) == -1)   
        {  
            perror("accept");  
            exit(errno);  
        }   
        else  
            printf("server: got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port), new_fd);  
              
        /* 开始处理每个新连接上的数据收发 */  
        //printf("\n准备就绪，可以开始聊天了……直接输入消息回车即可发信息给对方\n");  
        while (1)   
        {  
            /* 把集合清空 */  
            FD_ZERO(&rfds);  
              
            /* 把标准输入句柄0加入到集合中 */  
            FD_SET(0, &rfds);  
            maxfd = 0;  
              
            /* 把当前连接句柄new_fd加入到集合中 */  
            FD_SET(new_fd, &rfds);  
            if (new_fd > maxfd)  
                maxfd = new_fd;  
                  
            /* 设置最大等待时间 */  
            tv.tv_sec = 1;  
            tv.tv_usec = 0;  
              
            /* 开始等待 */  
            retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);  
            if (retval == -1)   
            {  
                printf("将退出，select出错！ %s", strerror(errno));  
                break;  
            }   
            else if (retval == 0)   
            {  
                //printf("没有任何消息到来，用户也没有按键，继续等待……\n");
                continue;  
            }   
            else   
            {  
                if (FD_ISSET(0, &rfds))   
                {  
                    /* 用户按键了，则读取用户输入的内容发送出去 */  
                    bzero(buf, MAXBUF + 1);  
                    fgets(buf, MAXBUF, stdin);  
                    if (!strncasecmp(buf, "quit", 4))   
                    {  
                        printf("自己请求终止聊天！\n");  
                        break;  
                    }  
                    printf("strlen=%d\n",strlen(buf));
                    len = send(new_fd, buf, strlen(buf) - 1, 0);  
                    if (len >= 0)  
                        printf("消息:%s\t发送成功，共发送了%d个字节！\n", buf, len);  
                    else   
                    {  
                        printf("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n",buf, errno, strerror(errno));  
                        break;  
                    }  
                }  
                if (FD_ISSET(new_fd, &rfds))   
                {  
                    /* 当前连接的socket上有消息到来则接收对方发过来的消息并显示 */  
                    bzero(buf, MAXBUF + 1);  
                    /* 接收客户端的消息 */  
                    len = recv(new_fd, buf, MAXBUF, 0);
                    if (len > 0){  
                        printf("接收消息成功:'%s'，共%d个字节的数据\n",buf, len);
						if(strcmp(buf,"open_door")==0){
							char input_req_dev[128] = {0};
							char input_req_cont[1024] = {0}; 
							strcpy(input_req_dev,"30102019012300010001");
							strcpy(input_req_cont,"{\"deviceID\":\"30102019012300010001\",\"recordTime\":\"2018-12-12 15:52:00\",\"RecordType\":30004,\"CredenceType\":2,\"passType\":1}");
							egsc_log_user("get user req devid(%s).\n", input_req_dev);
							egsc_log_user("get user req content(%s).\n", input_req_cont);
							int send_result = mydev_upload_record(input_req_dev, input_req_cont);
							if(send_result!=EGSC_RET_SUCCESS){
								strcpy(buf,"upload record fail!");
								len = send(new_fd, buf, strlen(buf) - 1, 0);
								if (len >= 0)
									printf("消息:%s\t发送成功，共发送了%d个字节！\n", buf, len);
								else{
									printf("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n",buf, errno, strerror(errno));
									break;
								}
							}
						}
                    }
                    else   
                    {  
                        if (len < 0)  
                            printf("消息接收失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));  
                        else  
                            printf("client disconnected...\n");  
                        break;  
                    }  
                }  
            }  
        }  
          
        close(new_fd);  
          
        /* 处理每个新连接上的数据收发结束 */  
        //printf("还要和其它连接聊天吗？(no->退出)");  
        fflush(stdout);  
        bzero(buf, MAXBUF + 1);  
        //fgets(buf, MAXBUF, stdin);  
        //if (!strncasecmp(buf, "no", 2))   
        //{  
        //    printf("终止聊天！\n");  
        //    break;  
        //} 
    }
	close(sockfd);
	return 0;
}

static EGSC_RET_CODE mydev_upload_credence_load_result(char *dev_id, char *credence_load_result)
{
    int device_type = -1;
    char device_id_inner[32] = "";
    int array_size = 0;
    int array_index = 0;
    user_dev_info *user_dev = NULL;
    mydev_json_obj result_obj = NULL;
    mydev_json_obj result_array_obj = NULL;
    mydev_json_obj result_item_obj = NULL;
    egsc_subdev_id subdev_id;
    egsc_subdev_id *p_subdev_id = NULL;
    egsc_dev_upload_credence_load_result_param result_param;
    egsc_dev_upload_credence_load_result_list_item_param *item_param;

    if( (NULL == credence_load_result) ||
        (NULL == dev_id))
    {
        egsc_log_error("input credence load result NULL.\n");
        return EGSC_RET_ERROR;
    }

    memset(&subdev_id, 0x00, sizeof(subdev_id));
    if(EGSC_RET_ERROR == mydev_get_subdev_id(dev_id, &subdev_id, &p_subdev_id))
    {
        return EGSC_RET_ERROR;
    }

    user_dev_get_id(&s_mydev_dev_list_head, subdev_id.subdev_mac, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_user("no found dev(%s), skip report.\n", subdev_id.subdev_mac);
        return EGSC_RET_ERROR;
    }

    memset(&result_param, 0, sizeof(result_param));
    result_obj = mydev_json_parse(credence_load_result);
    if(NULL == result_obj)
    {
        egsc_log_user("parser dev result input(%s) failed, skip report.\n", credence_load_result);
        return EGSC_RET_ERROR;
    }

    mydev_json_get_int(result_obj, "DeviceType", &device_type);
    mydev_json_get_string(result_obj, "deviceID", device_id_inner, sizeof(device_id_inner));

    result_array_obj = mydev_json_get_object(result_obj, "ResultList");
    if(NULL != result_array_obj)
    {
        mydev_json_get_array_size(result_array_obj, &array_size);
        result_param.item = (egsc_dev_upload_credence_load_result_list_item_param *)egsc_platform_malloc(array_size*sizeof(egsc_dev_upload_credence_load_result_list_item_param));
        if(NULL == result_param.item)
        {
            mydev_json_clear(result_obj);
            result_obj = NULL;

            egsc_log_user("ResultList memory malloc failed, skip report.\n");
            return EGSC_RET_ERROR;
        }
        for(array_index=0;array_index<array_size;array_index++)
        {
            result_item_obj = mydev_json_get_array_item(result_array_obj,array_index);
            if(NULL != result_item_obj)
            {
                item_param = result_param.item+array_index;
                item_param->credence_type = -1;
                memset(item_param->credence_no, 0, sizeof(item_param->credence_no));
                memset(item_param->user_id, 0, sizeof(item_param->user_id));
                item_param->error_code = -1;
                memset(item_param->error_message, 0, sizeof(item_param->error_message));
                mydev_json_get_int(result_item_obj, "CredenceType", &item_param->credence_type);
                mydev_json_get_string(result_item_obj, "CredenceNo", item_param->credence_no, sizeof(item_param->credence_no));
                mydev_json_get_string(result_item_obj, "UserID", item_param->user_id, sizeof(item_param->user_id));
                mydev_json_get_int(result_item_obj, "ErrorCode", &item_param->error_code);
                mydev_json_get_string(result_item_obj, "ErrorMessage", item_param->error_message, sizeof(item_param->error_message));
                if( (-1 == item_param->credence_type) ||
                    (strlen(item_param->credence_no) == 0) ||
                    (strlen(item_param->user_id) == 0) ||
                    (-1 == item_param->error_code))
                {
                    egsc_platform_free(result_param.item);
                    result_param.item = NULL;
                    mydev_json_clear(result_obj);
                    result_obj = NULL;

                    egsc_log_user("parser result input(%s) get deviceID/CredenceType/CredenceNo/UserID/ErrorCode failed ,please confirm.\n", credence_load_result);
                    return EGSC_RET_ERROR;
                }

                egsc_log_info("parser valid credence result(type:%d credence_no:%s user_id:%s error_code:%d message:%s) .\n",
                        item_param->credence_type, item_param->credence_no,
                        item_param->user_id, item_param->error_code, item_param->error_message);
                result_param.list_len++;
            }
        }
    }

    if( (-1 == device_type) ||
        (strlen(device_id_inner) == 0))
    {
        mydev_json_clear(result_obj);
        result_obj = NULL;
        if(NULL != result_param.item)
        {
            egsc_platform_free(result_param.item);
            result_param.item = NULL;
        }
        egsc_log_user("parser result input(%s) get DeviceType/device_id failed ,please confirm.\n", credence_load_result);
        return EGSC_RET_ERROR;
    }

    if(strcmp(device_id_inner, dev_id) != 0)
    {
        mydev_json_clear(result_obj);
        result_obj = NULL;
        if(NULL != result_param.item)
        {
            egsc_platform_free(result_param.item);
            result_param.item = NULL;
        }

        egsc_log_user("deviceID not match, skip report.\n");
        return EGSC_RET_ERROR;
    }

    result_param.device_type = device_type;
    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_DOOR_CTRL:
        {
            s_door_ctrl_req_if_tbl.upload_credence_load_result_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_credence_load_result_cb, &result_param);
            break;
        }
        case EGSC_TYPE_ELE_LINK_CTRL:
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            s_door_ctrl_req_if_tbl.upload_credence_load_result_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_credence_load_result_cb, &result_param);
            break;
        }
        case EGSC_TYPE_PARKING_CTRL:
        {
            s_parking_ctrl_req_if_tbl.upload_credence_load_result_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_credence_load_result_cb, &result_param);
            break;
        }
        default:
        {
            break;
        }
    }

    if(NULL != result_param.item)
    {
        egsc_platform_free(result_param.item);
        result_param.item = NULL;
    }
    mydev_json_clear(result_obj);
    result_obj = NULL;

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_upload_fac_dev_status(char *dev_id, char *dev_status)
{
    int ret = -1;
    char dev_id_inner[128] = "";
    char work_mode[128] = "";
    int state = -1;
    int floor = -1;
    int dicrection = -1;
    user_dev_info *user_dev = NULL;
    mydev_json_obj status_obj = NULL;
    egsc_subdev_id subdev_id;
    egsc_subdev_id *p_subdev_id = NULL;
    egsc_dev_upload_fac_dev_status_param fac_status_param;

    if( (NULL == dev_status) ||
        (NULL == dev_id))
    {
        egsc_log_error("input id/dev_status NULL.\n");
        return EGSC_RET_ERROR;
    }

    memset(&subdev_id, 0x00, sizeof(subdev_id));
    if(EGSC_RET_ERROR == mydev_get_subdev_id(dev_id, &subdev_id, &p_subdev_id))
    {
        return EGSC_RET_ERROR;
    }

    user_dev_get_id(&s_mydev_dev_list_head, subdev_id.subdev_mac, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_user("no found dev(%s), skip report.\n", subdev_id.subdev_mac);
        return EGSC_RET_ERROR;
    }

    status_obj = mydev_json_parse(dev_status);
    if(NULL == status_obj)
    {
        egsc_log_user("parser dev record input(%s) failed, skip report.\n", dev_status);
        return EGSC_RET_ERROR;
    }

    mydev_json_get_string(status_obj, "deviceID", dev_id_inner, sizeof(dev_id_inner));
    mydev_json_get_string(status_obj, "workMode", work_mode, sizeof(work_mode));
    mydev_json_get_int(status_obj, "State", &state);
    mydev_json_get_int(status_obj, "Floor", &floor);
    mydev_json_get_int(status_obj, "Dicrection", &dicrection);

    if( (strlen(dev_id_inner) == 0) ||
        (strlen(work_mode) == 0) ||
        (state < 0) ||
        (floor < 0) ||
        (dicrection < 0))
    {
        mydev_json_clear(status_obj);
        status_obj = NULL;

        egsc_log_user("parser fac dev status input(%s) get deviceID/workMode/State/Floor/Dicrection failed ,please confirm.\n", dev_status);
        return EGSC_RET_ERROR;
    }

    if(strcmp(dev_id_inner, dev_id) != 0)
    {
        mydev_json_clear(status_obj);
        status_obj = NULL;

        egsc_log_user("deviceID not match, skip report.\n");
        return EGSC_RET_ERROR;
    }

    ret = user_check_fac_work_mode(work_mode);
    if(ret < 0)
    {
        mydev_json_clear(status_obj);
        status_obj = NULL;

        egsc_log_user("workMode(%s) not valid, skip report.\n", work_mode);
        return EGSC_RET_ERROR;
    }

    ret = user_check_fac_device_state(state);
    if(ret < 0)
    {
        mydev_json_clear(status_obj);
        status_obj = NULL;

        egsc_log_user("State(%d) not valid, skip report.\n", state);
        return EGSC_RET_ERROR;
    }

    ret = user_check_fac_direction(dicrection);
    if(ret < 0)
    {
        mydev_json_clear(status_obj);
        status_obj = NULL;

        egsc_log_user("Dicrection(%d) not valid, skip report.\n", dicrection);
        return EGSC_RET_ERROR;
    }

    fac_status_param.work_mode = work_mode;
    fac_status_param.state = state;
    fac_status_param.floor = floor;
    fac_status_param.dicrection = dicrection;
    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_ELE_LINK_CTRL:
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            s_elevator_req_if_tbl.upload_fac_dev_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_fac_dev_status_res_cb, &fac_status_param);
            break;
        }
        default:
        {
            break;
        }
    }

    mydev_json_clear(status_obj);
    status_obj = NULL;

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_upload_fac_elevator_record(char *dev_id, char *record)
{
    int ret = -1;
    int record_type = -1;
    int user_type = -1;
    int credence_type = -1;
    char sub_dev_id[128] = "";
    char credence_no[128] = "";
    char user_id[128] = "";
    char light_mode[128] = "";
    char op_time[128] = "";
    int array_index = 0;
    int array_size = 0;
    user_dev_info *user_dev = NULL;
    mydev_json_obj record_obj = NULL;
    mydev_json_obj array_obj = NULL;
    mydev_json_obj item_obj = NULL;
    egsc_subdev_id subdev_id;
    egsc_subdev_id *p_subdev_id = NULL;
    egsc_dev_upload_fac_elevator_record_param record_param;

    if( (NULL == record) ||
        (NULL == dev_id))
    {
        egsc_log_error("input record NULL.\n");
        return EGSC_RET_ERROR;
    }

    memset(&subdev_id, 0x00, sizeof(subdev_id));
    if(EGSC_RET_ERROR == mydev_get_subdev_id(dev_id, &subdev_id, &p_subdev_id))
    {
        return EGSC_RET_ERROR;
    }

    user_dev_get_id(&s_mydev_dev_list_head, subdev_id.subdev_mac, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_user("no found dev(%s), skip report.\n", subdev_id.subdev_mac);
        return EGSC_RET_ERROR;
    }

    record_obj = mydev_json_parse(record);
    if(NULL == record_obj)
    {
        egsc_log_user("parser dev record input(%s) failed, skip report.\n", record);
        return EGSC_RET_ERROR;
    }

    memset(&sub_dev_id, 0, sizeof(sub_dev_id));
    memset(&credence_no, 0, sizeof(credence_no));
    memset(&user_id, 0, sizeof(user_id));
    memset(&light_mode, 0, sizeof(light_mode));
    memset(&op_time, 0, sizeof(op_time));
    memset(&record_param, 0, sizeof(record_param));
    mydev_json_get_string(record_obj, "deviceID", sub_dev_id, sizeof(sub_dev_id));
    mydev_json_get_int(record_obj, "RecordType", &record_type);
    mydev_json_get_int(record_obj, "UserType", &user_type);
    mydev_json_get_int(record_obj, "CredenceType", &credence_type);
    mydev_json_get_string(record_obj, "credenceNo", credence_no, sizeof(credence_no));
    mydev_json_get_string(record_obj, "userID", user_id, sizeof(user_id));

    array_obj = mydev_json_get_object(record_obj, "DestFloor");
    if(NULL != array_obj)
    {
        mydev_json_get_array_size(array_obj, &array_size);
        record_param.dest_floor = (int *)egsc_platform_malloc(array_size*sizeof(int));
        if(NULL != record_param.dest_floor)
        {
            for(array_index=0;array_index<array_size;array_index++)
            {
                item_obj = mydev_json_get_array_item(array_obj,array_index);
                if(NULL != item_obj)
                {
                    *(record_param.dest_floor+array_index) = -1;
                    mydev_json_get_int(item_obj, NULL, record_param.dest_floor+array_index);

                    record_param.dest_floor_num++;
                }
            }
        }
        else
        {
            egsc_log_user("memory malloc failed, discard DestFloor.\n");
        }
    }

    mydev_json_get_string(record_obj, "lightMode", light_mode, sizeof(light_mode));
    mydev_json_get_string(record_obj, "opTime", op_time, sizeof(op_time));

    if( (strlen(sub_dev_id) == 0) ||
        (strlen(credence_no) == 0) ||
        (strlen(light_mode) == 0) ||
        (strlen(op_time) == 0) ||
        (record_type < 0) ||
        (credence_type < 0))
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;
        if(NULL != record_param.dest_floor)
        {
            egsc_platform_free(record_param.dest_floor);
            record_param.dest_floor = NULL;
        }

        egsc_log_user("parser event input(%s) get deviceID/RecordType/CredenceType/credenceNo/lightMode/opTime failed ,please confirm.\n", record);
        return EGSC_RET_ERROR;
    }

    if(strcmp(sub_dev_id, dev_id) != 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;
        if(NULL != record_param.dest_floor)
        {
            egsc_platform_free(record_param.dest_floor);
            record_param.dest_floor = NULL;
        }

        egsc_log_user("deviceID not match, skip report.\n");
        return EGSC_RET_ERROR;
    }

    ret = user_check_record_type(record_type);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;
        if(NULL != record_param.dest_floor)
        {
            egsc_platform_free(record_param.dest_floor);
            record_param.dest_floor = NULL;
        }

        egsc_log_user("RecordType(%d) not valid, skip report.\n", record_type);
        return EGSC_RET_ERROR;
    }

    #if 0
    if(user_type >= 0)
    {
        ret = user_certificate_check_user_type(&s_mydev_cert_list_head, dev_id, user_type);
        if(ret < 0)
        {
            mydev_json_clear(record_obj);
            record_obj = NULL;
            if(NULL != record_param.dest_floor)
            {
                egsc_platform_free(record_param.dest_floor);
                record_param.dest_floor = NULL;
            }

            egsc_log_user("dev_id(%s) UserType(%d) not valid, skip report.\n", dev_id, user_type);
            return EGSC_RET_ERROR;
        }
    }

    ret = user_certificate_check_credence_type(&s_mydev_cert_list_head, dev_id, credence_type);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;
        if(NULL != record_param.dest_floor)
        {
            egsc_platform_free(record_param.dest_floor);
            record_param.dest_floor = NULL;
        }

        egsc_log_user("dev_id(%s) CredenceType(%d) not valid, skip report.\n", dev_id, credence_type);
        return EGSC_RET_ERROR;
    }

    ret = user_certificate_check_credence_no(&s_mydev_cert_list_head, dev_id, credence_no);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;
        if(NULL != record_param.dest_floor)
        {
            egsc_platform_free(record_param.dest_floor);
            record_param.dest_floor = NULL;
        }

        egsc_log_user("dev_id(%s) credenceNo(%s) not valid, skip report.\n", dev_id, credence_no);
        return EGSC_RET_ERROR;
    }

    if(strlen(user_id)>0)
    {
        ret = user_certificate_check_user_id(&s_mydev_cert_list_head, dev_id, user_id);
        if(ret < 0)
        {
            mydev_json_clear(record_obj);
            record_obj = NULL;
            if(NULL != record_param.dest_floor)
            {
                egsc_platform_free(record_param.dest_floor);
                record_param.dest_floor = NULL;
            }

            egsc_log_user("dev_id(%s) userID(%s) not match, skip report.\n", dev_id, user_id);
            return EGSC_RET_ERROR;
        }
    }
    #endif

    ret = user_check_fac_light_mode(light_mode);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;
        if(NULL != record_param.dest_floor)
        {
            egsc_platform_free(record_param.dest_floor);
            record_param.dest_floor = NULL;
        }

        egsc_log_user("lightMode(%s) not valid, skip report.\n", light_mode);
        return EGSC_RET_ERROR;
    }

    ret = user_check_optime_valid(op_time);
    if(ret < 0)
    {
        mydev_json_clear(record_obj);
        record_obj = NULL;
        if(NULL != record_param.dest_floor)
        {
            egsc_platform_free(record_param.dest_floor);
            record_param.dest_floor = NULL;
        }

        egsc_log_user("opTime(%s) not valid, skip report.\n", op_time);
        return EGSC_RET_ERROR;
    }

    record_param.record_type = record_type;
    record_param.user_type = user_type;
    record_param.credence_type = credence_type;
    record_param.credence_no = credence_no;
    record_param.user_id = user_id;
    record_param.light_mode = light_mode;
    record_param.op_time = op_time;
    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_ELE_LINK_CTRL:
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            s_elevator_req_if_tbl.upload_fac_elevator_record_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_fac_elevator_record_res_cb, &record_param);
            break;
        }
        default:
        {
            break;
        }
    }

    mydev_json_clear(record_obj);
    record_obj = NULL;
    if(NULL != record_param.dest_floor)
    {
        egsc_platform_free(record_param.dest_floor);
        record_param.dest_floor = NULL;
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_upload_fac_ba_status(char *dev_id, char *ba_status)
{
    int ret = -1;
    int error_cnt = 0;
    char time_stamp[128] = "";
    int array_index = 0;
    int array_size = 0;
    mydev_json_obj ba_status_obj = NULL;
    mydev_json_obj status_array_obj = NULL;
    mydev_json_obj status_item_obj = NULL;
    user_dev_info *user_dev = NULL;
    egsc_subdev_id subdev_id;
    egsc_subdev_id *p_subdev_id = NULL;
    egsc_dev_upload_fac_ba_status_param ba_status_param;
    egsc_dev_cb_fac_lift_car_status_param *status_item;
    char subdev_id_str[32] = {0};

    if( (NULL == ba_status) ||
        (NULL == dev_id))
    {
        egsc_log_error("input ba_status/id NULL.\n");
        return EGSC_RET_ERROR;
    }

    memset(&subdev_id, 0x00, sizeof(subdev_id));
    if(EGSC_RET_ERROR == mydev_get_subdev_id(dev_id, &subdev_id, &p_subdev_id))
    {
        return EGSC_RET_ERROR;
    }

    user_dev_get_id(&s_mydev_dev_list_head, subdev_id.subdev_mac, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_user("no found dev(%s), skip report.\n", subdev_id.subdev_mac);
        return EGSC_RET_ERROR;
    }

    ba_status_obj = mydev_json_parse(ba_status);
    if(NULL == ba_status_obj)
    {
        egsc_log_user("parser dev record input(%s) failed, skip report.\n", ba_status);
        return EGSC_RET_ERROR;
    }

    memset(time_stamp, 0, sizeof(time_stamp));
    memset(&ba_status_param, 0, sizeof(ba_status_param));
    mydev_json_get_string(ba_status_obj, "timestamp", time_stamp, sizeof(time_stamp));

    status_array_obj = mydev_json_get_object(ba_status_obj, "statusList");
    if(NULL != status_array_obj)
    {
        mydev_json_get_array_size(status_array_obj, &array_size);
        ba_status_param.lift_car_status = (egsc_dev_cb_fac_lift_car_status_param *)egsc_platform_malloc(array_size*sizeof(egsc_dev_cb_fac_lift_car_status_param));
        if(NULL == ba_status_param.lift_car_status)
        {
            mydev_json_clear(ba_status_obj);
            ba_status_obj = NULL;

            egsc_log_user("new create lift car status obj failed.\n");
            return EGSC_RET_ERROR;
        }
        for(array_index=0;array_index<array_size;array_index++)
        {
            status_item_obj = mydev_json_get_array_item(status_array_obj,array_index);
            if(NULL != status_item_obj)
            {
                status_item = ba_status_param.lift_car_status+array_index;
                memset(status_item, 0, sizeof(egsc_dev_cb_fac_lift_car_status_param));
                status_item->car_id = -1;
                status_item->physical_floor = -1;
                status_item->error_status = -1;
                status_item->fire_ctrl_status = -1;
                snprintf(subdev_id_str, 4+1,"%04d", status_item->subdev_id.subdev_type);
                snprintf(subdev_id_str+4, 12+1,"%s", status_item->subdev_id.subdev_mac);
                snprintf(subdev_id_str+4+12, 4+1,"%04d", status_item->subdev_id.subdev_num);
                mydev_json_get_string(status_item_obj, "deviceID", subdev_id_str, sizeof(subdev_id_str));
                mydev_json_get_int(status_item_obj, "carID", &status_item->car_id);
                mydev_json_get_int(status_item_obj, "physicalfloor", &status_item->physical_floor);
                mydev_json_get_string(status_item_obj, "displayfloor", status_item->display_floor, sizeof(status_item->display_floor));
                mydev_json_get_string(status_item_obj, "carStatus", status_item->car_status, sizeof(status_item->car_status));
                mydev_json_get_string(status_item_obj, "doorStatus", status_item->door_status, sizeof(status_item->door_status));
                mydev_json_get_int(status_item_obj, "ErrorStatus", &status_item->error_status);
                mydev_json_get_string(status_item_obj, "errorMessage", status_item->error_message, sizeof(status_item->error_message));
                mydev_json_get_int(status_item_obj, "fireCtrlStatus", &status_item->fire_ctrl_status);

                if( (strlen(subdev_id_str) == 0) ||
                    (strlen(status_item->car_status) == 0) ||
                    (strlen(status_item->door_status) == 0) ||
                    (status_item->car_id < 0) ||
                    (status_item->physical_floor < 0) ||
                    (status_item->error_status < 0) ||
                    (status_item->fire_ctrl_status < 0))
                {
                    egsc_platform_free(ba_status_param.lift_car_status);
                    mydev_json_clear(ba_status_obj);
                    ba_status_param.lift_car_status = NULL;
                    ba_status_obj = NULL;

                    egsc_log_user("parser result input(%s) get deviceID/carID/physicalfloor/carStatus/doorStatus/ErrorStatus/fireCtrlStatus failed ,please confirm.\n", ba_status);
                    return EGSC_RET_ERROR;
                }

                if(strcmp(subdev_id_str, dev_id) != 0)
                {
                    error_cnt++;
                    egsc_log_user("deviceID not match, skip report.\n");
                }

                ret = user_check_fac_ba_car_status(status_item->car_status);
                if(ret < 0)
                {
                    error_cnt++;
                    egsc_log_user("carStatus(%s) not valid, skip report.\n", status_item->car_status);
                }
                ret = user_check_fac_ba_door_status(status_item->door_status);
                if(ret < 0)
                {
                    error_cnt++;
                    egsc_log_user("doorStatus(%s) not valid, skip report.\n", status_item->door_status);
                }
                ret = user_check_fac_ba_error_status(status_item->error_status);
                if(ret < 0)
                {
                    error_cnt++;
                    egsc_log_user("ErrorStatus(%d) not valid, skip report.\n", status_item->error_status);
                }
                ret = user_check_fac_ba_fire_ctrl_status(status_item->fire_ctrl_status);
                if(ret < 0)
                {
                    error_cnt++;
                    egsc_log_user("fireCtrlStatus(%d) not valid, skip report.\n", status_item->fire_ctrl_status);
                }

                if(error_cnt > 0)
                {
                    egsc_platform_free(ba_status_param.lift_car_status);
                    mydev_json_clear(ba_status_obj);
                    ba_status_param.lift_car_status = NULL;
                    ba_status_obj = NULL;

                    return EGSC_RET_ERROR;
                }
                ba_status_param.lift_car_status_num++;
            }
        }
    }

    if( strlen(time_stamp) == 0)
    {
        mydev_json_clear(ba_status_obj);
        ba_status_obj = NULL;
        if(NULL != ba_status_param.lift_car_status)
        {
            egsc_platform_free(ba_status_param.lift_car_status);
            ba_status_param.lift_car_status = NULL;
        }
        egsc_log_user("parser event input(%s) get timestamp failed ,please confirm.\n", ba_status);
        return EGSC_RET_ERROR;
    }

    ret = user_check_optime_valid(time_stamp);
    if(ret < 0)
    {
        mydev_json_clear(ba_status_obj);
        ba_status_obj = NULL;
        if(NULL != ba_status_param.lift_car_status)
        {
            egsc_platform_free(ba_status_param.lift_car_status);
            ba_status_param.lift_car_status = NULL;
        }

        egsc_log_user("timestamp(%s) not valid, skip report.\n", time_stamp);
        return EGSC_RET_ERROR;
    }

    ba_status_param.timestamp = time_stamp;
    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_ELE_LINK_CTRL:
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            s_elevator_req_if_tbl.upload_fac_ba_status_if(0, p_subdev_id, s_mydev_req_id++, mydev_upload_fac_ba_status_res_cb, &ba_status_param);
            break;
        }
        default:
        {
            break;
        }
    }

    mydev_json_clear(ba_status_obj);
    ba_status_obj = NULL;
    if(NULL != ba_status_param.lift_car_status)
    {
        egsc_platform_free(ba_status_param.lift_car_status);
        ba_status_param.lift_car_status = NULL;
    }

    return EGSC_RET_SUCCESS;
}

static EGSC_RET_CODE mydev_pak_intercom_control(char *id, char *credence_load_result)
{
    int ret = -1;
    int command_type = -1;
    char sdp[128] = {0};
    user_dev_info *user_dev = NULL;
    egsc_subdev_id subdev_id;
    egsc_subdev_id *p_subdev_id = NULL;

    mydev_json_obj intercom_obj = NULL;
    egsc_dev_pak_intercom_control_param intercom_param;

    if(NULL == credence_load_result)
    {
        egsc_log_error("input credence load result NULL.\n");
        return EGSC_RET_ERROR;
    }

    memset(&subdev_id, 0x00, sizeof(subdev_id));
    if(EGSC_RET_ERROR == mydev_get_subdev_id(id, &subdev_id, &p_subdev_id))
    {
        return EGSC_RET_ERROR;
    }

    user_dev_get_id(&s_mydev_dev_list_head, subdev_id.subdev_mac, &user_dev);
    if(NULL == user_dev)
    {
        egsc_log_user("no found dev(%s), skip report.\n", subdev_id.subdev_mac);
        return EGSC_RET_ERROR;
    }

    memset(sdp, 0, sizeof(sdp));
    memset(&intercom_param, 0, sizeof(intercom_param));
    intercom_obj = mydev_json_parse(credence_load_result);
    if(NULL == intercom_obj)
    {
        egsc_log_info("parser dev result input(%s) failed, skip report.\n", credence_load_result);
        return EGSC_RET_ERROR;
    }
    
    mydev_json_get_int(intercom_obj, "CommandType", &command_type);
    mydev_json_get_string(intercom_obj, "SDP", sdp, sizeof(sdp));

    if(command_type < 0)
    {
        mydev_json_clear(intercom_obj);
        intercom_obj = NULL;

        egsc_log_info("parser result input(%s) get DeviceType/device_id failed ,please confirm.\n", credence_load_result);
        return EGSC_RET_ERROR;
    }

    ret = user_check_pak_intercom_ctrl_command_type(command_type);
    if(ret < 0)
    {
        mydev_json_clear(intercom_obj);
        intercom_obj = NULL;

        egsc_log_user("CommandType(%d) not valid, skip request.\n", command_type);
        return EGSC_RET_ERROR;
    }

    intercom_param.command_type = command_type;
    snprintf(intercom_param.sdp, sizeof(sdp), "%s", sdp);

    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_PARKING_CTRL:
        {
            s_parking_ctrl_req_if_tbl.pak_intercom_control_if(0, s_mydev_req_id++, mydev_req_pak_intercom_control_cb, &intercom_param);
            break;
        }
        default:
        {
            break;
        }
    }
    mydev_json_clear(intercom_obj);
    intercom_obj = NULL;

    return EGSC_RET_SUCCESS;
}

static void mydev_socket_test_task_fn(unsigned long arg)
{
	char addr[20] = {0};
	socketServerStart(SOCKET_SERVER_PORT,SOCKET_SERVER_LISNUM, addr);
}
static void mydev_input_test_task_fn(unsigned long arg)
{
    int ret = -1;
    char *read_input = NULL;
    char input_req[1024];
    char input_req_cmd[128];
    char input_req_dev[128];
    char input_req_cont[1024];

    while(1)
    {
        memset(input_req, 0, sizeof(input_req));
        read_input = fgets(input_req, sizeof(input_req), stdin);
        if(NULL == read_input)
        {
            egsc_log_user("input error,please retry\n");
            continue;
        }
        if('\n' == input_req[0])
        {
            continue;
        }

        if(strncmp(input_req, "110120", strlen("110120")) == 0)
        {
            egsc_log_user("support cmd list: \n");
            egsc_log_user("    110120\n");
            egsc_log_user("    30102293573571590001 status {\"deviceID\":\"30102293573571590001\",\"DeviceType\":2009,\"DeviceStatus\":1}\n");
            egsc_log_user("    30102293573571590001 record {\"deviceID\":\"30102293573571590001\",\"recordTime\":\"2018-12-12 15:52:00\",\"RecordType\":30004,\"CredenceType\":2,\"passType\":1}\n");
            egsc_log_user("    30102293573571590001 event {\"EventType\":30301,\"subDeviceID\":\"30102293573571590001\",\"time\":\"2018-11-27 15:21:00\"}\n");
            egsc_log_user("    30102293573571590001 result {\"DeviceType\":1,\"deviceID\":\"30102293573571590001\",\"ResultList\":[{\"CredenceType\":1,\"CredenceNo\":\"1111\",\"UserID\":\"test1\",\"ErrorCode\":1,\"ErrorMessage\":\"test11\"}]}\n");
            egsc_log_user("    10052016229357159360 fac_status {\"deviceID\":\"10052016229357159360\",\"workMode\":\"1\",\"State\":1,\"Floor\":1,\"Dicrection\":1}\n");
            egsc_log_user("    10012020669357159357 elevator_record {\"deviceID\":\"10012020669357159357\",\"RecordType\":10001,\"UserType\":5,\"CredenceType\":0,\"credenceNo\":\"test123\",\"userID\":\"00cf697cf131451285663c425742453b\",\"DestFloor\":[2,3,4,5],\"lightMode\":\"0\",\"opTime\":\"2018-11-27 15:21:00\"}\n");
            egsc_log_user("    10052016229357159360 fac_ba_status {\"statusList\":[{\"deviceID\":\"10052016229357159360\",\"carID\":1,\"physicalfloor\":1,\"displayfloor\":\"1234\",\"carStatus\":\"00\",\"doorStatus\":\"10\",\"ErrorStatus\":1,\"errorMessage\":\"test1234\",\"fireCtrlStatus\":1},{\"deviceID\":\"10052016229357159360\",\"carID\":2,\"physicalfloor\":1,\"displayfloor\":\"1234\",\"carStatus\":\"00\",\"doorStatus\":\"10\",\"ErrorStatus\":1,\"errorMessage\":\"test1234\",\"fireCtrlStatus\":1}],\"timestamp\":\"2018-11-27 15:21:00\"}\n");
            egsc_log_user("    intercom  {\"CommandType\":1,\"SDP\":\"v=0\"}\n");
            continue;
        }

        memset(input_req_dev, 0, sizeof(input_req_dev));
        memset(input_req_cmd, 0, sizeof(input_req_cmd));
        memset(input_req_cont, 0, sizeof(input_req_cont));
        ret = sscanf(input_req, "%s %s %[^\n]", input_req_dev, input_req_cmd, input_req_cont);
        if(3 != ret)
        {
            egsc_log_user("input format error ,please retry\n", input_req);
            continue;
        }

        egsc_log_user("get user req dev(%s).\n", input_req_dev);
        egsc_log_user("get user req command(%s).\n", input_req_cmd);
        egsc_log_user("get user req content(%s).\n", input_req_cont);
        if(strcmp(input_req_cmd, "status") == 0)
        {
            mydev_upload_dev_status(input_req_dev, input_req_cont);
        }
        else if(strcmp(input_req_cmd, "record") == 0)
        {
            mydev_upload_record(input_req_dev, input_req_cont);
        }
        else if(strcmp(input_req_cmd, "event") == 0)
        {
            mydev_upload_event(input_req_dev, input_req_cont);
        }
        else if(strcmp(input_req_cmd, "result") == 0)
        {
            mydev_upload_credence_load_result(input_req_dev, input_req_cont);
        }
        else if(strcmp(input_req_cmd, "fac_status") == 0)
        {
            mydev_upload_fac_dev_status(input_req_dev, input_req_cont);
        }
        else if(strcmp(input_req_cmd, "elevator_record") == 0)
        {
            mydev_upload_fac_elevator_record(input_req_dev, input_req_cont);
        }
        else if(strcmp(input_req_cmd, "fac_ba_status") == 0)
        {
            mydev_upload_fac_ba_status(input_req_dev, input_req_cont);
        }
        else if(strcmp(input_req_cmd, "intercom") == 0)
        {
            mydev_pak_intercom_control(input_req_dev, input_req_cont);
        }
        else
        {
            egsc_log_user("no found req(%s), skip.\n", input_req_cmd);
        }
    }
}

static int start_mydev_test()
{
    int ret = -1;
    int arg = 0;
    s_test_task_id = -1;
    //ret = egsc_platform_task_create("mydev_test_task", &s_test_task_id, mydev_input_test_task_fn, arg, 1024, 0);
	ret = egsc_platform_task_create("mydev_test_task", &s_test_task_id, mydev_socket_test_task_fn, arg, 1024, 0);
    if( (ret < 0) ||
        (s_test_task_id < 0))
    {
        egsc_log_error("mydev test task create failed.\n");
    }

    return 0;
}

static int stop_mydev_test()
{
    int ret = -1;
    if(s_test_task_id >= 0)
    {
        ret = egsc_platform_task_delete(s_test_task_id);
        if(ret < 0)
        {
            egsc_log_error("stop mydev test task failed.\n");
        }
    }

    return 0;
}

static void mydev_misc_task_fn(unsigned long arg)
{
    int ret = 0;
    struct list_head *head = NULL;
    user_dev_info *pos, *n;

    while(1)
    {
        head = &s_mydev_dev_list_head;
        pos = NULL;
        n = NULL;
        list_for_each_entry_safe(pos,n,head,node)
        {
            if( 0 != pos->updated )
            {
                pos->update_delay++;
                if(pos->update_delay<10)
                {
                    continue;
                }
                pos->updated = 0;
                pos->update_delay = 0;
                egsc_log_debug("got dev(id:%s) updated, ready re-register.\n", pos->dev_info.id);
                egsc_dev_stop(pos->dev_handle);
                egsc_dev_delete(pos->dev_handle);

                ret = mydev_create_single(pos);
                if(EGSC_RET_SUCCESS != ret)
                {
                    egsc_log_error("dev(id:%s) re-create failed.\n", pos->dev_info.id);
                    continue;
                }
                ret = egsc_dev_start(pos->dev_handle);
                if(EGSC_RET_SUCCESS != ret)
                {
                    egsc_dev_delete(pos->dev_handle);
                    egsc_log_error("(name:%s) re-start failed.\n", pos->dev_info.name);
                    continue;
                }
            }
        }
        egsc_platform_sleep(1000);
    }
}
static int start_mydev_misc()
{
    int ret = -1;
    int arg = 0;
    s_misc_task_id = -1;
    ret = egsc_platform_task_create("mydev_misc_task", &s_misc_task_id, mydev_misc_task_fn, arg, 1024, 0);
    if( (ret < 0) ||
        (s_misc_task_id < 0))
    {
        egsc_log_error("mydev misc task create failed.\n");
    }

    return 0;
}

static int stop_mydev_misc()
{
    int ret = -1;
    if(s_misc_task_id >= 0)
    {
        ret = egsc_platform_task_delete(s_misc_task_id);
        if(ret < 0)
        {
            egsc_log_error("stop mydev misc task failed.\n");
        }
    }

    return 0;
}
void init_door_ctrl()
{
	egsc_log_debug("enter.\n");
	s_door_ctrl_status_cb = mydev_status_callback;
    s_door_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_door_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_door_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_door_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_door_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_door_ctrl_srv_req_cb_tbl.load_certificate_cb = mydev_load_certificate_cb;
    s_door_ctrl_srv_req_cb_tbl.read_certificate_cb = mydev_read_certificate_cb;
    s_door_ctrl_srv_req_cb_tbl.delete_certificate_cb = mydev_delete_certificate_cb;
    s_door_ctrl_srv_req_cb_tbl.load_certificate_in_batch_cb = mydev_load_certificate_in_batch_cb;
    s_door_ctrl_srv_req_cb_tbl.read_certificate_in_batch_cb = mydev_read_certificate_in_batch_cb;
    s_door_ctrl_srv_req_cb_tbl.delete_certificate_in_batch_cb = mydev_delete_certificate_in_batch_cb;
    s_door_ctrl_srv_req_cb_tbl.gate_ctrl_cb = mydev_gate_ctrl_cb;
    s_door_ctrl_srv_req_cb_tbl.play_voice_cb = mydev_play_voice_cb;
    s_door_ctrl_srv_req_cb_tbl.read_vol_cb = mydev_read_vol_cb;
    s_door_ctrl_srv_req_cb_tbl.set_vol_cb = mydev_set_vol_cb;
    s_door_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
}
void init_gate_ctrl()
{
	egsc_log_debug("enter.\n");
	s_gate_ctrl_status_cb = mydev_status_callback;
    s_gate_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_gate_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_gate_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_gate_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_gate_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_gate_ctrl_srv_req_cb_tbl.load_certificate_cb = mydev_load_certificate_cb;
    s_gate_ctrl_srv_req_cb_tbl.read_certificate_cb = mydev_read_certificate_cb;
    s_gate_ctrl_srv_req_cb_tbl.delete_certificate_cb = mydev_delete_certificate_cb;
    s_gate_ctrl_srv_req_cb_tbl.load_certificate_in_batch_cb = mydev_load_certificate_in_batch_cb;
    s_gate_ctrl_srv_req_cb_tbl.read_certificate_in_batch_cb = mydev_read_certificate_in_batch_cb;
    s_gate_ctrl_srv_req_cb_tbl.delete_certificate_in_batch_cb = mydev_delete_certificate_in_batch_cb;
    s_gate_ctrl_srv_req_cb_tbl.gate_ctrl_cb = mydev_gate_ctrl_cb;
    s_gate_ctrl_srv_req_cb_tbl.play_voice_cb = mydev_play_voice_cb;
    s_gate_ctrl_srv_req_cb_tbl.read_vol_cb = mydev_read_vol_cb;
    s_gate_ctrl_srv_req_cb_tbl.set_vol_cb = mydev_set_vol_cb;
    s_gate_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
}
void init_elec_lpn()
{
	egsc_log_debug("enter.\n");
	s_elec_lpn_status_cb = mydev_status_callback;
    s_elec_lpn_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_elec_lpn_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_elec_lpn_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_elec_lpn_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_elec_lpn_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_elec_lpn_srv_req_cb_tbl.pak_send_showinfo_cb = mydev_pak_send_showinfo_cb;
    s_elec_lpn_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
}
void init_parking_lock()
{
	egsc_log_debug("enter.\n");
	s_parking_lock_status_cb = mydev_status_callback;
    s_parking_lock_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_parking_lock_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_parking_lock_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_parking_lock_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_parking_lock_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_parking_lock_srv_req_cb_tbl.pak_control_lock_cb = mydev_pak_control_lock_cb;
    s_parking_lock_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
}
void init_smart_ctrl()
{
	egsc_log_debug("enter.\n");
	s_smart_ctrl_status_cb = mydev_status_callback;
    s_smart_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_smart_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_smart_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_smart_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_smart_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_smart_ctrl_srv_req_cb_tbl.boun_chan_setupalarm_cb = mydev_boun_chan_setupalarm_cb;
    s_smart_ctrl_srv_req_cb_tbl.boun_subchan_clearalarm_cb = mydev_boun_subchan_clearalarm_cb;
    s_smart_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
}
void init_elevator()
{
	egsc_log_debug("enter.\n");
	s_elevator_status_cb = mydev_status_callback;
    s_elevator_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_elevator_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_elevator_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_elevator_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_elevator_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_elevator_srv_req_cb_tbl.load_certificate_cb = mydev_load_certificate_cb;
    s_elevator_srv_req_cb_tbl.read_certificate_cb = mydev_read_certificate_cb;
    s_elevator_srv_req_cb_tbl.delete_certificate_cb = mydev_delete_certificate_cb;
    s_elevator_srv_req_cb_tbl.load_certificate_in_batch_cb = mydev_load_certificate_in_batch_cb;
    s_elevator_srv_req_cb_tbl.read_certificate_in_batch_cb = mydev_read_certificate_in_batch_cb;
    s_elevator_srv_req_cb_tbl.delete_certificate_in_batch_cb = mydev_delete_certificate_in_batch_cb;
    s_elevator_srv_req_cb_tbl.fac_visit_control_cb = mydev_fac_visit_control_cb;
    s_elevator_srv_req_cb_tbl.fac_key_control_cb = mydev_fac_key_control_cb;
    s_elevator_srv_req_cb_tbl.fac_calling_cb = mydev_fac_calling_cb;
    s_elevator_srv_req_cb_tbl.fac_inter_call_auth_cb = mydev_fac_inter_call_auth_cb;
    s_elevator_srv_req_cb_tbl.fac_inter_call_lighting_cb = mydev_fac_inter_call_lighting_cb;
    s_elevator_srv_req_cb_tbl.fac_lift_lighting_cb = mydev_fac_lift_lighting_cb;
    s_elevator_srv_req_cb_tbl.fac_delayed_closing_cb = mydev_fac_delayed_closing_cb;
    s_elevator_srv_req_cb_tbl.fac_status_req_cb = mydev_fac_status_req_cb;
    s_elevator_srv_req_cb_tbl.fac_lift_ba_status_cb = mydev_fac_lift_ba_status_cb;
    s_elevator_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
}
void init_parking_ctrl()
{
	egsc_log_debug("enter.\n");
	s_parking_ctrl_status_cb = mydev_status_callback;
    s_parking_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_parking_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_parking_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_parking_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_parking_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_parking_ctrl_srv_req_cb_tbl.load_certificate_cb = mydev_load_certificate_cb;
    s_parking_ctrl_srv_req_cb_tbl.read_certificate_cb = mydev_read_certificate_cb;
    s_parking_ctrl_srv_req_cb_tbl.delete_certificate_cb = mydev_delete_certificate_cb;
    s_parking_ctrl_srv_req_cb_tbl.load_certificate_in_batch_cb = mydev_load_certificate_in_batch_cb;
    s_parking_ctrl_srv_req_cb_tbl.read_certificate_in_batch_cb = mydev_read_certificate_in_batch_cb;
    s_parking_ctrl_srv_req_cb_tbl.delete_certificate_in_batch_cb = mydev_delete_certificate_in_batch_cb;
    s_parking_ctrl_srv_req_cb_tbl.download_black_and_white_list_cb = mydev_download_black_and_white_list_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_load_left_car_seat_cb = mydev_load_left_car_seat_cb;
    s_parking_ctrl_srv_req_cb_tbl.gate_ctrl_cb = mydev_gate_ctrl_cb;
    s_parking_ctrl_srv_req_cb_tbl.play_voice_cb = mydev_play_voice_cb;
    s_parking_ctrl_srv_req_cb_tbl.read_vol_cb = mydev_read_vol_cb;
    s_parking_ctrl_srv_req_cb_tbl.set_vol_cb = mydev_set_vol_cb;
    s_parking_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
    s_parking_ctrl_srv_req_cb_tbl.snap_picture_cb = mydev_snap_picture_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_reset_dg_cb = mydev_pak_reset_dg_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_led_display_cb = mydev_pak_led_display_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_load_led_info_cb = mydev_pak_load_led_info_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_control_lock_cb = mydev_pak_control_lock_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_intercom_control_cb = mydev_rsp_pak_intercom_control_cb;
}
void init_screen_ctrl()
{
	egsc_log_debug("enter.\n");
	s_screen_ctrl_status_cb = mydev_status_callback;
    s_screen_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_screen_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_screen_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_screen_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_screen_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_screen_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
    s_screen_ctrl_srv_req_cb_tbl.trans_material_cb = mydev_rsp_ads_trans_material_cb;
    s_screen_ctrl_srv_req_cb_tbl.query_term_cb = mydev_rsp_ads_query_term_cb;
    s_screen_ctrl_srv_req_cb_tbl.add_program_cb = mydev_rsp_ads_add_program_cb;
    s_screen_ctrl_srv_req_cb_tbl.delete_program_cb = mydev_rsp_ads_delete_program_cb;
    s_screen_ctrl_srv_req_cb_tbl.set_program_cb = mydev_rsp_ads_set_program_cb;
    s_screen_ctrl_srv_req_cb_tbl.add_schedule_cb = mydev_rsp_ads_add_schedule_cb;
    s_screen_ctrl_srv_req_cb_tbl.delete_schedule_cb = mydev_rsp_ads_delete_schedule_cb;
    s_screen_ctrl_srv_req_cb_tbl.set_schedule_cb = mydev_rsp_ads_set_schedule_cb;
    s_screen_ctrl_srv_req_cb_tbl.publish_schedule_cb = mydev_rsp_ads_publish_schedule_cb;
}
int mydev_init()
{
    int ret = 0;
    egsc_log_debug("enter\n");
    egsc_log_user("soft version:%s\n", MYDEV_SOFT_VERSION);

    INIT_LIST_HEAD(&(s_mydev_cert_list_head));
    INIT_LIST_HEAD(&(s_mydev_dev_list_head));

    start_mydev_test();
    start_mydev_misc();
    //加载设备配置
    mydev_boot_load_parameters();
    //启动加载用户凭证数据
    user_file_load_certificate();

    ret = user_file_load_device_config();
    if(ret < 0)
    {
        egsc_log_user("load dev conf file failed, please check.\n");
        return ret;
    }

    s_mydev_status = 1;

    s_door_ctrl_status_cb = mydev_status_callback;
    s_door_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_door_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_door_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_door_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_door_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_door_ctrl_srv_req_cb_tbl.load_certificate_cb = mydev_load_certificate_cb;
    s_door_ctrl_srv_req_cb_tbl.read_certificate_cb = mydev_read_certificate_cb;
    s_door_ctrl_srv_req_cb_tbl.delete_certificate_cb = mydev_delete_certificate_cb;
    s_door_ctrl_srv_req_cb_tbl.load_certificate_in_batch_cb = mydev_load_certificate_in_batch_cb;
    s_door_ctrl_srv_req_cb_tbl.read_certificate_in_batch_cb = mydev_read_certificate_in_batch_cb;
    s_door_ctrl_srv_req_cb_tbl.delete_certificate_in_batch_cb = mydev_delete_certificate_in_batch_cb;
    s_door_ctrl_srv_req_cb_tbl.gate_ctrl_cb = mydev_gate_ctrl_cb;
    s_door_ctrl_srv_req_cb_tbl.play_voice_cb = mydev_play_voice_cb;
    s_door_ctrl_srv_req_cb_tbl.read_vol_cb = mydev_read_vol_cb;
    s_door_ctrl_srv_req_cb_tbl.set_vol_cb = mydev_set_vol_cb;
    s_door_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;

    s_gate_ctrl_status_cb = mydev_status_callback;
    s_gate_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_gate_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_gate_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_gate_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_gate_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_gate_ctrl_srv_req_cb_tbl.load_certificate_cb = mydev_load_certificate_cb;
    s_gate_ctrl_srv_req_cb_tbl.read_certificate_cb = mydev_read_certificate_cb;
    s_gate_ctrl_srv_req_cb_tbl.delete_certificate_cb = mydev_delete_certificate_cb;
    s_gate_ctrl_srv_req_cb_tbl.load_certificate_in_batch_cb = mydev_load_certificate_in_batch_cb;
    s_gate_ctrl_srv_req_cb_tbl.read_certificate_in_batch_cb = mydev_read_certificate_in_batch_cb;
    s_gate_ctrl_srv_req_cb_tbl.delete_certificate_in_batch_cb = mydev_delete_certificate_in_batch_cb;
    s_gate_ctrl_srv_req_cb_tbl.gate_ctrl_cb = mydev_gate_ctrl_cb;
    s_gate_ctrl_srv_req_cb_tbl.play_voice_cb = mydev_play_voice_cb;
    s_gate_ctrl_srv_req_cb_tbl.read_vol_cb = mydev_read_vol_cb;
    s_gate_ctrl_srv_req_cb_tbl.set_vol_cb = mydev_set_vol_cb;
    s_gate_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;

    s_elec_lpn_status_cb = mydev_status_callback;
    s_elec_lpn_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_elec_lpn_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_elec_lpn_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_elec_lpn_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_elec_lpn_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_elec_lpn_srv_req_cb_tbl.pak_send_showinfo_cb = mydev_pak_send_showinfo_cb;
    s_elec_lpn_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;

    s_parking_lock_status_cb = mydev_status_callback;
    s_parking_lock_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_parking_lock_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_parking_lock_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_parking_lock_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_parking_lock_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_parking_lock_srv_req_cb_tbl.pak_control_lock_cb = mydev_pak_control_lock_cb;
    s_parking_lock_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;

    s_smart_ctrl_status_cb = mydev_status_callback;
    s_smart_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_smart_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_smart_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_smart_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_smart_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_smart_ctrl_srv_req_cb_tbl.boun_chan_setupalarm_cb = mydev_boun_chan_setupalarm_cb;
    s_smart_ctrl_srv_req_cb_tbl.boun_subchan_clearalarm_cb = mydev_boun_subchan_clearalarm_cb;
    s_smart_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;

    s_elevator_status_cb = mydev_status_callback;
    s_elevator_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_elevator_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_elevator_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_elevator_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_elevator_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_elevator_srv_req_cb_tbl.load_certificate_cb = mydev_load_certificate_cb;
    s_elevator_srv_req_cb_tbl.read_certificate_cb = mydev_read_certificate_cb;
    s_elevator_srv_req_cb_tbl.delete_certificate_cb = mydev_delete_certificate_cb;
    s_elevator_srv_req_cb_tbl.load_certificate_in_batch_cb = mydev_load_certificate_in_batch_cb;
    s_elevator_srv_req_cb_tbl.read_certificate_in_batch_cb = mydev_read_certificate_in_batch_cb;
    s_elevator_srv_req_cb_tbl.delete_certificate_in_batch_cb = mydev_delete_certificate_in_batch_cb;
    s_elevator_srv_req_cb_tbl.fac_visit_control_cb = mydev_fac_visit_control_cb;
    s_elevator_srv_req_cb_tbl.fac_key_control_cb = mydev_fac_key_control_cb;
    s_elevator_srv_req_cb_tbl.fac_calling_cb = mydev_fac_calling_cb;
    s_elevator_srv_req_cb_tbl.fac_inter_call_auth_cb = mydev_fac_inter_call_auth_cb;
    s_elevator_srv_req_cb_tbl.fac_inter_call_lighting_cb = mydev_fac_inter_call_lighting_cb;
    s_elevator_srv_req_cb_tbl.fac_lift_lighting_cb = mydev_fac_lift_lighting_cb;
    s_elevator_srv_req_cb_tbl.fac_delayed_closing_cb = mydev_fac_delayed_closing_cb;
    s_elevator_srv_req_cb_tbl.fac_status_req_cb = mydev_fac_status_req_cb;
    s_elevator_srv_req_cb_tbl.fac_lift_ba_status_cb = mydev_fac_lift_ba_status_cb;
    s_elevator_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;

    s_parking_ctrl_status_cb = mydev_status_callback;
    s_parking_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_parking_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_parking_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_parking_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_parking_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_parking_ctrl_srv_req_cb_tbl.load_certificate_cb = mydev_load_certificate_cb;
    s_parking_ctrl_srv_req_cb_tbl.read_certificate_cb = mydev_read_certificate_cb;
    s_parking_ctrl_srv_req_cb_tbl.delete_certificate_cb = mydev_delete_certificate_cb;
    s_parking_ctrl_srv_req_cb_tbl.load_certificate_in_batch_cb = mydev_load_certificate_in_batch_cb;
    s_parking_ctrl_srv_req_cb_tbl.read_certificate_in_batch_cb = mydev_read_certificate_in_batch_cb;
    s_parking_ctrl_srv_req_cb_tbl.delete_certificate_in_batch_cb = mydev_delete_certificate_in_batch_cb;
    s_parking_ctrl_srv_req_cb_tbl.download_black_and_white_list_cb = mydev_download_black_and_white_list_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_load_left_car_seat_cb = mydev_load_left_car_seat_cb;
    s_parking_ctrl_srv_req_cb_tbl.gate_ctrl_cb = mydev_gate_ctrl_cb;
    s_parking_ctrl_srv_req_cb_tbl.play_voice_cb = mydev_play_voice_cb;
    s_parking_ctrl_srv_req_cb_tbl.read_vol_cb = mydev_read_vol_cb;
    s_parking_ctrl_srv_req_cb_tbl.set_vol_cb = mydev_set_vol_cb;
    s_parking_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
    s_parking_ctrl_srv_req_cb_tbl.snap_picture_cb = mydev_snap_picture_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_reset_dg_cb = mydev_pak_reset_dg_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_led_display_cb = mydev_pak_led_display_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_load_led_info_cb = mydev_pak_load_led_info_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_control_lock_cb = mydev_pak_control_lock_cb;
    s_parking_ctrl_srv_req_cb_tbl.pak_intercom_control_cb = mydev_rsp_pak_intercom_control_cb;

    s_screen_ctrl_status_cb = mydev_status_callback;
    s_screen_ctrl_srv_req_cb_tbl.reset_cb = mydev_reset_cb;
    s_screen_ctrl_srv_req_cb_tbl.correction_cb = mydev_correction_cb;
    s_screen_ctrl_srv_req_cb_tbl.notify_update_cb = mydev_notify_update_cb;
    s_screen_ctrl_srv_req_cb_tbl.read_parameter_cb = mydev_read_parameter_cb;
    s_screen_ctrl_srv_req_cb_tbl.setting_parameters_cb = mydev_setting_parameters_cb;
    s_screen_ctrl_srv_req_cb_tbl.query_dev_status_cb = mydev_query_dev_status_cb;
    s_screen_ctrl_srv_req_cb_tbl.trans_material_cb = mydev_rsp_ads_trans_material_cb;
    s_screen_ctrl_srv_req_cb_tbl.query_term_cb = mydev_rsp_ads_query_term_cb;
    s_screen_ctrl_srv_req_cb_tbl.add_program_cb = mydev_rsp_ads_add_program_cb;
    s_screen_ctrl_srv_req_cb_tbl.delete_program_cb = mydev_rsp_ads_delete_program_cb;
    s_screen_ctrl_srv_req_cb_tbl.set_program_cb = mydev_rsp_ads_set_program_cb;
    s_screen_ctrl_srv_req_cb_tbl.add_schedule_cb = mydev_rsp_ads_add_schedule_cb;
    s_screen_ctrl_srv_req_cb_tbl.delete_schedule_cb = mydev_rsp_ads_delete_schedule_cb;
    s_screen_ctrl_srv_req_cb_tbl.set_schedule_cb = mydev_rsp_ads_set_schedule_cb;
    s_screen_ctrl_srv_req_cb_tbl.publish_schedule_cb = mydev_rsp_ads_publish_schedule_cb;

    return 0;
}

int genDevIDs(char *devid,int idlens, int offset)
{
	int ret = EGSC_RET_SUCCESS;
	const int minlen = 4;
	int result = 0;
	if(idlens<minlen){
		egsc_log_error("idlens[%d]<minlen[%d]\n",idlens,minlen);
		return EGSC_RET_ERROR;
	}
	if(offset>0){
		char endStr[minlen+1];
		strcpy(endStr,devid+idlens-minlen);
		//TODO:进位超过10000可能会计算错误，初始值弄小点就好
		result = atoi(endStr)+offset;
		if(result>10000)
			return EGSC_RET_ERROR;
		snprintf(endStr,sizeof(endStr),"%04d",result);
		egsc_log_debug("size=%d,len=%d,endStr=%s\n",sizeof(endStr),strlen(endStr),endStr);
		strcpy(devid+idlens-minlen,endStr);
	}
	return ret;
}
int mydev_init_V2()
{
	int ret = 0;
    egsc_log_debug("enter\n");
    INIT_LIST_HEAD(&(s_mydev_dev_list_head));
    ret = user_file_load_device_config();
    if(ret < 0)
    {
        egsc_log_user("load dev conf file failed, please check.\n");
        return ret;
    }
	/*test code
	struct list_head *head;
    user_dev_info *pos;
    head = &s_mydev_dev_list_head;
	pos = list_first_entry(head, typeof(*pos), node);
	printf("size=%d devid=%s\n",strlen(pos->dev_info.id),pos->dev_info.id);
	int offset = 100;
	int len =strlen(pos->dev_info.id);
	genDevIDs(pos->dev_info.id, len, offset);
	pos = list_first_entry(head, typeof(*pos), node);
	printf("size=%d devid=%s\n",strlen(pos->dev_info.id),pos->dev_info.id);*/
	s_mydev_status = 1;
	return 0;
}
int mydev_init_by_type(EGSC_DEV_TYPE dev_type)
{
	switch (dev_type)
		{
			case EGSC_TYPE_DOOR_CTRL:
			{
				init_door_ctrl();
				break;
			}
			case EGSC_TYPE_GATE_CTRL:
			{
				init_gate_ctrl();
			}
			case EGSC_TYPE_ELEC_LPN_CTRL:
			{
				init_elec_lpn();
				break;
			}
			case EGSC_TYPE_PARKING_LOCK_CONTROLLER:
			{
				init_parking_lock();
				break;
			}
			case EGSC_TYPE_SMART_CTRL_KB:
			{
				init_smart_ctrl();
			}
			case EGSC_TYPE_ELE_LINK_CTRL:
			case EGSC_TYPE_ELEVATOR_CTRL:
			{
				init_elevator();
				break;
			}
			case EGSC_TYPE_PARKING_CTRL:
			{
				init_parking_ctrl();
				break;
			}
			case EGSC_TYPE_SCREEN_CTRL:
			{
				init_screen_ctrl();
				break;
			}
			default:
			{
				egsc_log_error("Not support devType:%d\n",dev_type);
				return EGSC_RET_ERROR;
			}
		}
	return EGSC_RET_SUCCESS;
}
int mydev_uninit()
{
    egsc_log_debug("enter\n");

    stop_mydev_test();
    stop_mydev_misc();
    user_dev_clear(&s_mydev_dev_list_head);
    if(NULL != s_mydev_params_obj)
    {
        mydev_json_clear(s_mydev_params_obj);
    }

    return 0;
}

static int mydev_create_single(user_dev_info *user_dev)
{
    int ret = 0;
    int get_len = 0;
    int srv_req_cb_len = 0;
    int dev_req_if_len = 0;
    switch(user_dev->dev_info.dev_type)
    {
        case EGSC_TYPE_DOOR_CTRL:
        {
            user_dev->status_cb_func = (void *)s_door_ctrl_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&s_door_ctrl_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&s_door_ctrl_req_if_tbl;
            srv_req_cb_len = sizeof(egsc_door_ctrl_cb_tbl);
            dev_req_if_len = sizeof(egsc_door_ctrl_if_tbl);
            break;
        }
        case EGSC_TYPE_GATE_CTRL:
        {
            user_dev->status_cb_func = (void *)s_gate_ctrl_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&s_gate_ctrl_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&s_gate_ctrl_req_if_tbl;
            srv_req_cb_len = sizeof(egsc_gate_ctrl_cb_tbl);
            dev_req_if_len = sizeof(egsc_gate_ctrl_if_tbl);
            break;
        }
        case EGSC_TYPE_ELEC_LPN_CTRL:
        {
            user_dev->status_cb_func = (void *)s_elec_lpn_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&s_elec_lpn_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&s_elec_lpn_req_if_tbl;
            srv_req_cb_len = sizeof(egsc_elec_lpn_cb_tbl);
            dev_req_if_len = sizeof(egsc_elec_lpn_if_tbl);
            break;
        }
        case EGSC_TYPE_PARKING_LOCK_CONTROLLER:
        {
            user_dev->status_cb_func = (void *)s_parking_lock_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&s_parking_lock_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&s_parking_lock_req_if_tbl;
            srv_req_cb_len = sizeof(egsc_parking_lock_cb_tbl);
            dev_req_if_len = sizeof(egsc_parking_lock_if_tbl);
            break;
        }
        case EGSC_TYPE_SMART_CTRL_KB:
        {
            user_dev->status_cb_func = (void *)s_smart_ctrl_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&s_smart_ctrl_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&s_smart_ctrl_req_if_tbl;
            srv_req_cb_len = sizeof(egsc_smart_ctrl_cb_tbl);
            dev_req_if_len = sizeof(egsc_smart_ctrl_if_tbl);
            break;
        }
        case EGSC_TYPE_ELE_LINK_CTRL:
        case EGSC_TYPE_ELEVATOR_CTRL:
        {
            user_dev->status_cb_func = (void *)s_elevator_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&s_elevator_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&s_elevator_req_if_tbl;
            srv_req_cb_len = sizeof(egsc_elevator_cb_tbl);
            dev_req_if_len = sizeof(egsc_elevator_if_tbl);
            break;
        }
        case EGSC_TYPE_PARKING_CTRL:
        {
            user_dev->status_cb_func = (void *)s_parking_ctrl_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&s_parking_ctrl_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&s_parking_ctrl_req_if_tbl;
            srv_req_cb_len = sizeof(egsc_parking_ctrl_cb_tbl);
            dev_req_if_len = sizeof(egsc_parking_ctrl_if_tbl);
            break;
        }
        case EGSC_TYPE_SCREEN_CTRL:
        {
            user_dev->status_cb_func = (void *)s_screen_ctrl_status_cb;
            user_dev->srv_req_cb_tbl = (void *)&s_screen_ctrl_srv_req_cb_tbl;
            user_dev->dev_req_if_tbl = (void *)&s_screen_ctrl_req_if_tbl;
            srv_req_cb_len = sizeof(egsc_screen_ctrl_cb_tbl);
            dev_req_if_len = sizeof(egsc_screen_ctrl_if_tbl);
            break;
        }
        default :
        {
            break;
        }
    }
    ret = egsc_dev_create(&user_dev->dev_info, (egsc_dev_status_callback)user_dev->status_cb_func, &user_dev->dev_handle);
    if(ret != EGSC_RET_SUCCESS)
    {
        egsc_log_error("(id:%s) egsc_dev_create failed.\n", user_dev->dev_info.id);
        return ret;
    }

    ret = egsc_dev_func_register(user_dev->dev_handle, user_dev->srv_req_cb_tbl, srv_req_cb_len,
                                    user_dev->dev_req_if_tbl, dev_req_if_len, &get_len);
    if(ret != EGSC_RET_SUCCESS || get_len != dev_req_if_len)
    {
        egsc_dev_delete(user_dev->dev_handle);
        egsc_log_error(" dev(id:%s) func_register failed, ret(%d).\n", user_dev->dev_info.id, ret);
        return ret;
    }
    egsc_log_info("(id:%s) dev create success.\n", user_dev->dev_info.id);
    return EGSC_RET_SUCCESS;
}

int mydev_create()
{
    int ret = 0;
    struct list_head *head;
    user_dev_info *pos, *n;

    head = &s_mydev_dev_list_head;
    list_for_each_entry_safe(pos,n,head,node)
    {
        ret = mydev_create_single(pos);
        if(EGSC_RET_SUCCESS != ret)
        {
            egsc_log_error(" dev(name:%s) create failed, ret(%d).\n", pos->dev_info.name, ret);
            continue;
        }
    }

    return 0;
}

int mydev_delete()
{
    struct list_head *head;
    user_dev_info *pos, *n;

    head = &s_mydev_dev_list_head;
    list_for_each_entry_safe(pos,n,head,node)
    {
        egsc_dev_delete(pos->dev_handle);
        list_del(&pos->node);
        if(NULL != pos->dev_info.subdev_info)
        {
            egsc_platform_free(pos->dev_info.subdev_info);
        }
        egsc_platform_free(pos);
        pos = NULL;
    }

    return 0;
}

int mydev_start()
{
    int ret = 0;
    struct list_head *head;
    user_dev_info *pos, *n;

    head = &s_mydev_dev_list_head;
    list_for_each_entry_safe(pos,n,head,node)
    {
        ret = egsc_dev_start(pos->dev_handle);
        if(ret != EGSC_RET_SUCCESS)
        {
            egsc_log_error("(name:%s) egsc_dev_start failed.\n", pos->dev_info.name);
            continue;
        }
        else
        {
            egsc_log_user("(name:%s) dev start success.\n", pos->dev_info.name);
        }
    }

    return 0;
}

int mydev_stop()
{
    struct list_head *head;
    user_dev_info *pos, *n;

    head = &s_mydev_dev_list_head;
    list_for_each_entry_safe(pos,n,head,node)
    {
        egsc_dev_stop(pos->dev_handle);
    }

    return 0;
}

int my_dev_single_init(EGSC_DEV_TYPE dev_type, int dev_offset)
{
	int ret;
	user_dev_info *user_dev = NULL;
	ret = user_dev_get_type(&s_mydev_dev_list_head, dev_type, &user_dev);
    if(user_dev == NULL)
    {
        egsc_log_user("no found dev_type(%d), skip report.\n", dev_type);
        return ret;
    }
	ret = genDevIDs(user_dev->dev_info.id, strlen(user_dev->dev_info.id),dev_offset);
	if(ret != EGSC_RET_SUCCESS){
		return ret;
	}
	ret = egsc_sdk_init();
    if(ret != EGSC_RET_SUCCESS)
    {
        egsc_log_error("[%s]egsc_sdk_init failed\n",user_dev->dev_info.id);
        return ret;
    }
	ret = mydev_init_by_type(user_dev->dev_info.dev_type);
	if(ret!=EGSC_RET_SUCCESS){
		egsc_log_error("(id:%s) mydev_init_by_type failed.\n", user_dev->dev_info.id);
        return ret;
	}
    //ret = mydev_create();
    ret = mydev_create_single(user_dev);
    if(EGSC_RET_SUCCESS != ret){
		egsc_log_error(" dev(id:%s) create failed, ret(%d).\n", user_dev->dev_info.id, ret);
        return ret;
    }
    //ret = mydev_start();
	ret = egsc_dev_start(user_dev->dev_handle);
	if(ret != EGSC_RET_SUCCESS){
		egsc_log_error("(id:%s) egsc_dev_start failed, ret(%d).\n", user_dev->dev_info.id, ret);
		return ret;
    }
	else{
		egsc_log_user("(id:%s) dev start success.\n", user_dev->dev_info.id);
	}
	Child_process_loop(user_dev,dev_offset);
	egsc_log_info("[id:%s]DelDispatchMQ ret = %d\n",user_dev->dev_info.id,ret);
    ret = mydev_stop();
	egsc_log_info("[id:%s]mydev_stop ret = %d\n",user_dev->dev_info.id,ret);
    ret = mydev_delete();
	egsc_log_info("[id:%s]mydev_delete ret = %d\n",user_dev->dev_info.id,ret);
    egsc_sdk_uninit();
	return EGSC_RET_SUCCESS;
}
void main_process_loop(unsigned int *dev_arr, int arr_size)
{
    char *read_input = NULL;
    char input_req[1024];
	char input_req_cont[1024];
	msg_struct msgbuff;
	int arr_index = 0;
	int ret;

    while(1)
    {
        memset(input_req, 0, sizeof(input_req));
        read_input = fgets(input_req, sizeof(input_req), stdin);
        if(NULL == read_input)
        {
            egsc_log_user("input error,please retry\n");
            continue;
        }
        if('\n' == input_req[0])
        {
            continue;
        }

        if(strncmp(input_req, "110120", strlen("110120")) == 0)
        {
            egsc_log_user("support cmd list: \n");
            egsc_log_user("    110120\n");
            egsc_log_user("    30102293573571590001 status {\"deviceID\":\"30102293573571590001\",\"DeviceType\":2009,\"DeviceStatus\":1}\n");
            egsc_log_user("    30102293573571590001 record {\"deviceID\":\"30102293573571590001\",\"recordTime\":\"2018-12-12 15:52:00\",\"RecordType\":30004,\"CredenceType\":2,\"passType\":1}\n");
            egsc_log_user("    30102293573571590001 event {\"EventType\":30301,\"subDeviceID\":\"30102293573571590001\",\"time\":\"2018-11-27 15:21:00\"}\n");
            egsc_log_user("    30102293573571590001 result {\"DeviceType\":1,\"deviceID\":\"30102293573571590001\",\"ResultList\":[{\"CredenceType\":1,\"CredenceNo\":\"1111\",\"UserID\":\"test1\",\"ErrorCode\":1,\"ErrorMessage\":\"test11\"}]}\n");
            egsc_log_user("    10052016229357159360 fac_status {\"deviceID\":\"10052016229357159360\",\"workMode\":\"1\",\"State\":1,\"Floor\":1,\"Dicrection\":1}\n");
            egsc_log_user("    10012020669357159357 elevator_record {\"deviceID\":\"10012020669357159357\",\"RecordType\":10001,\"UserType\":5,\"CredenceType\":0,\"credenceNo\":\"test123\",\"userID\":\"00cf697cf131451285663c425742453b\",\"DestFloor\":[2,3,4,5],\"lightMode\":\"0\",\"opTime\":\"2018-11-27 15:21:00\"}\n");
            egsc_log_user("    10052016229357159360 fac_ba_status {\"statusList\":[{\"deviceID\":\"10052016229357159360\",\"carID\":1,\"physicalfloor\":1,\"displayfloor\":\"1234\",\"carStatus\":\"00\",\"doorStatus\":\"10\",\"ErrorStatus\":1,\"errorMessage\":\"test1234\",\"fireCtrlStatus\":1},{\"deviceID\":\"10052016229357159360\",\"carID\":2,\"physicalfloor\":1,\"displayfloor\":\"1234\",\"carStatus\":\"00\",\"doorStatus\":\"10\",\"ErrorStatus\":1,\"errorMessage\":\"test1234\",\"fireCtrlStatus\":1}],\"timestamp\":\"2018-11-27 15:21:00\"}\n");
            egsc_log_user("    intercom  {\"CommandType\":1,\"SDP\":\"v=0\"}\n");
            continue;
        }
        memset(input_req_cont, 0, sizeof(input_req_cont));
        ret = sscanf(input_req, "%[^\n]", input_req_cont);
        if(1 != ret)
        {
            egsc_log_user("input format error ,please retry\n", input_req);
            continue;
        }
		if(strncmp(input_req_cont, "stop", strlen("stop")) == 0||
			strncmp(input_req_cont, "status", strlen("status")) == 0||
			strncmp(input_req_cont, "record", strlen("record")) == 0||
			strncmp(input_req_cont, "event", strlen("event")) == 0||
			strncmp(input_req_cont, "result", strlen("result")) == 0||
			strncmp(input_req_cont, "fac_status", strlen("fac_status")) == 0||
			strncmp(input_req_cont, "elevator_record", strlen("elevator_record")) == 0||
			strncmp(input_req_cont, "fac_ba_status", strlen("fac_ba_status")) == 0||
			strncmp(input_req_cont, "intercom", strlen("intercom")) == 0)
		{
			for(arr_index=0;arr_index<arr_size;++arr_index){
				if(!*(dev_arr+arr_index)){
					break;
				}
				unsigned int dev_type = GetDevType(*(dev_arr+arr_index));
				unsigned int dev_count = GetDevCount(*(dev_arr+arr_index));
				unsigned int dev_index = 0;
				egsc_log_info("dev_type:%d dev_count:%d\n",dev_type,dev_count);
				for(;dev_index<dev_count;++dev_index){
					msgbuff.msgType = GetMQMsgType(dev_type, dev_index);
					msgbuff.msgData.devType = dev_type;
					msgbuff.msgData.offset = dev_index;
					strcpy(msgbuff.msgData.info,input_req_cont);
					PutDispatchMQ(msgbuff);
				}
			}
			if(strncmp(input_req_cont, "stop", strlen("stop")) == 0){
				break;
			}
			continue;
		}
		egsc_log_user("Invalid cmd [%s]\n",input_req_cont);
		continue;
    }
}
void Child_process_loop(user_dev_info *user_dev,int dev_offset)
{
	int ret;
	msg_struct msgbuff;
	msgbuff.msgType = GetMQMsgType(user_dev->dev_info.dev_type,dev_offset);
	memset(msgbuff.msgData.info, 0, sizeof(msgbuff.msgData.info));
	while(1)
    {
		ret = GetDispatchMQ(msgbuff.msgType,&msgbuff);
		if(ret >= EGSC_RET_SUCCESS){
			egsc_log_info("[id:%s] get msg:[type:%d,offset:%d info:%s]\n",user_dev->dev_info.id,msgbuff.msgData.devType,msgbuff.msgData.offset,msgbuff.msgData.info);
			if(strncmp(msgbuff.msgData.info, "stop", strlen("stop"))==0){
				egsc_log_info("ready to break\n");
				break;
			}
			memset(msgbuff.msgData.info, 0, sizeof(msgbuff.msgData.info));
        }
		else{
			egsc_platform_sleep(1000);
		}
    }
	egsc_log_info("I'm out\n");
	ret = DelDispatchMQ(msgbuff.msgType);
}
