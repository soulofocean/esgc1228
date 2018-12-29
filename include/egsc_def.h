#ifndef _EGSC_DEF_H_
#define _EGSC_DEF_H_

// 设备状态编码
typedef enum _EGSC_DEV_STATUS_CODE
{
    EGSC_DEV_STATUS_STOPED          = 0,
    EGSC_DEV_STATUS_TO_CONNECT      = 1,
    EGSC_DEV_STATUS_TO_REGISTER     = 2,
    EGSC_DEV_STATUS_WORKING         = 3,
}EGSC_DEV_STATUS_CODE;

// 返回值定义
typedef enum _EGSC_RET_CODE
{
    EGSC_RET_SUCCESS                    = 0,  // 返回成功
    EGSC_RET_ERROR                      = -1, // 一般错误，没有明确定义的错误
    EGSC_RET_NO_SURPORT                 =  1, // 设备不支持
    EGSC_RET_DATA_ERROR                 =  2, // 数据错误
    EGSC_RET_SUBDEVICE_TIMEOUT          =  3, // 子设备超时未返回
    EGSC_RET_CREDENCE_FMT_ERR           =  4, // 凭证格式不符
    EGSC_RET_CREDENCE_DATA_GET_FAIL     =  5, // 凭证资料获取失败
    EGSC_RET_EXCEED_MAX_CREDENCE_NUM    =  6, // 超出最大凭证数量
    EGSC_RET_DEV_INTERNAL_PROC_FAIL     =  7, // 设备内部处理失败
    EGSC_RET_CREDENCE_FILE_RESOLVE_FAIL =  8, // 解析固定凭证文件失败
    EGSC_RET_DEV_DISCONNECT_FROM_SEEVER =  9, // 设备与图片服务器断开
    EGSC_RET_CHILD_DEV_OFFLINE          =  10,// 子设备不在线
    EGSC_RET_DEV_NOT_SUPPORT_THE_INST   =  11,// 设备不支持该指令
    EGSC_RET_CREDENCE_LOW_QUALITY       =  12,// 凭证质量差

}EGSC_RET_CODE;


// 设备类型
typedef enum _EGSC_DEV_TYPE
{
    EGSC_TYPE_CAMERA        = 2001, // IPC枪机
    EGSC_TYPE_EAGLEEYE_CAM  = 2002, // 鹰眼摄像机
    EGSC_TYPE_BALLHEAD_CAM  = 2003, // 球机
    EGSC_TYPE_FACE_CAP_CAM  = 2004, // 人脸抓拍机
    EGSC_TYPE_PARKING_CTRL  = 2005, // 停车场控制器
    EGSC_TYPE_PARK_LPN_ID   = 2006, // 停车场车牌识别仪
    EGSC_TYPE_DOOR_CTRL     = 2009, // 门禁控制器
    EGSC_TYPE_GATE_CTRL     = 2010, // 人行通道控制器
    EGSC_TYPE_ENTRA_MACHINE = 2011, // 门口机
    EGSC_TYPE_FENCE_MACHINE = 2012, // 围墙机
    EGSC_TYPE_INDOOR_MACHINE= 2013, // 室内机
    EGSC_TYPE_MGMT_MACHINE  = 2014, // 管理机
    EGSC_TYPE_ELE_LINK_CTRL = 2016, // 电梯联动控制器
    EGSC_TYPE_PATROL_DEV    = 2017, // 巡更设备
    EGSC_TYPE_SCREEN_CTRL   = 2018, // 信息发布屏控制器
    EGSC_TYPE_BROADCAST_CTRL= 2019, // 广播控制器
    EGSC_TYPE_ELEVATOR_CTRL = 2020, // 电梯厂商控制器
    EGSC_TYPE_SMART_CTRL_KB = 2021, // 智能控制终端
    EGSC_TYPE_CARPARK_CAM   = 2022, // 车位检测相机
    EGSC_TYPE_ELEC_LPN_CTRL = 2023, // 电子车位控制器
    EGSC_TYPE_PARKING_LOCK_CONTROLLER = 2025, // 车位锁控制器
}EGSC_DEV_TYPE;

// 子设备类型
typedef enum _EGSC_SUBDEV_TYPE
{
    //EGSC_TYPE_CAMERA        = 2001, // IPC枪机
    EGSC_SUBTYPE_CAMERA_VIDEO_CHANNEL           = 3001,     // IPC枪机通道
    EGSC_SUBTYPE_CAMERA_ALERT_IN_CHANNEL        = 3002,     // IPC枪机告警输入通道
    EGSC_SUBTYPE_CAMERA_ALERT_OUT_CHANNEL       = 3003,     // IPC枪机告警输出通道

    //EGSC_TYPE_EAGLEEYE_CAM  = 2002, // 鹰眼摄像机
    EGSC_SUBTYPE_EAGLEEYE_VIDEO_CHANNEL         = 3004,     // 鹰眼摄像机通道
    EGSC_SUBTYPE_EAGLEEYE_ALERT_IN_CHANNEL      = 3005,     // 鹰眼摄像机告警输入通道
    EGSC_SUBTYPE_EAGLEEYE_ALERT_OUT_CHANNEL     = 3006,     // 鹰眼摄像机告警输出通道

    //EGSC_TYPE_BALLHEAD_CAM  = 2003, // 球机
    EGSC_SUBTYPE_BALLHEAD_CHANNEL               = 3007,     //  球机通道
    EGSC_SUBTYPE_BALLHEAD_ALERT_IN_CHANNEL      = 3008,     //  球机告警输入通道
    EGSC_SUBTYPE_BALLHEAD_ALERT_OUT_CHANNEL     = 3009,     //  球机告警输出通道

    //EGSC_TYPE_FACE_CAP_CAM  = 2004, // 人脸抓拍机
    EGSC_SUBTYPE_FACE_CAPTURE_CHANNEL           = 3041,     // 人脸抓拍机通道
    EGSC_SUBTYPE_FACE_CAPTURE_ALERT_IN_CHANNEL  = 3042,     // 人脸抓拍机告警输入通道
    EGSC_SUBTYPE_FACE_CAPTURE_ALERT_OUT_CHANNEL = 3043,     // 人脸抓拍机告警输出通道

    //EGSC_TYPE_PARKING_CTRL  = 2005, // 停车场控制器
    EGSC_SUBTYPE_PARKING_SPACE_DISPLAY          = 3023,     // 门口显示剩余车位数大屏
    EGSC_SUBTYPE_PARKING_BARRIER_GATE           = 3024,     // 停车场道闸
    EGSC_SUBTYPE_PARKING_FLOOR_SENSOR           = 3025,     // 停车场地感
    EGSC_SUBTYPE_PARKING_CPU_CARD_READER        = 3026,     // CPU读头
    EGSC_SUBTYPE_PARKING_QR_READER              = 3027,     // 二维码读头
    EGSC_SUBTYPE_PARKING_RFID_READER            = 3028,     // RFID读头
    EGSC_SUBTYPE_PARKING_IC_CARD_READER         = 3029,     // 停车场IC读头
    EGSC_SUBTYPE_PARKING_ID_CARD_READER         = 3030,     // 停车场ID读头
    EGSC_SUBTYPE_PARKING_TICKET_BOX             = 3031,     // 停车场票箱
    EGSC_SUBTYPE_PARKING_CARD_BOX               = 3032,     // 停车场卡箱
    EGSC_SUBTYPE_PARKING_LCD_DISPLAY            = 3033,     // 停车场LCD显示屏
    EGSC_SUBTYPE_PARKING_LED_DISPLAY            = 3034,     // 停车场LED显示屏
    EGSC_SUBTYPE_PARKING_INTERCOM               = 3035,     // 停车场对讲
    EGSC_SUBTYPE_PARKING_SPEAKER                = 3036,     // 停车场语音喇叭

    //EGSC_TYPE_DOOR_CTRL     = 2009, // 门禁控制器
    EGSC_SUBTYPE_DOOR_READER                    = 3010,     // 门禁读头
    EGSC_SUBTYPE_DOOR_FACE_READER               = 3011,     // 门禁人脸读卡器
    EGSC_SUBTYPE_DOOR_FINGER_READER             = 3012,     // 门禁指纹识别读卡器
    EGSC_SUBTYPE_DOOR_QR_READER                 = 3013,     // 门禁二维码读卡器
    EGSC_SUBTYPE_DOOR_BLUETOOTH_READER          = 3014,     // 门禁蓝牙读卡器
    EGSC_SUBTYPE_DOOR_PASSWORD_KEYBOARD         = 3015,     // 门禁密码输入键盘
    EGSC_SUBTYPE_DOOR_IC_CARD_READER            = 3016,     // 门禁IC读头
    EGSC_SUBTYPE_DOOR_CPU_CARD_READER           = 3017,     // 门禁CPU读头

    //EGSC_TYPE_GATE_CTRL     = 2010, // 人行通道控制器
    EGSC_SUBTYPE_GATE_MACHINE                   = 3018,     // 人证读卡器（闸机）
    EGSC_SUBTYPE_GATE_IQR_READER                = 3039,     // 二维码读卡器（闸机）
    EGSC_SUBTYPE_GATE_IC_CARD_READER            = 3040,     // IC卡读卡器（闸机）

    //EGSC_TYPE_ENTRA_MACHINE = 2011, // 门口机
    EGSC_SUBTYPE_ENTRANCE_READER                = 3044,     // 门口机读头
    EGSC_SUBTYPE_ENTRANCE_FACE_READER           = 3045,     // 门口机人脸读卡器
    EGSC_SUBTYPE_ENTRANCE_FINGER_READER         = 3046,     // 门口机指纹识别读卡器
    EGSC_SUBTYPE_ENTRANCE_QR_READER             = 3047,     // 门口机二维码读卡器
    EGSC_SUBTYPE_ENTRANCE_BLUETOOTH_READER      = 3048,     // 门口机蓝牙读卡器
    EGSC_SUBTYPE_ENTRANCE_PASSWORD_KEYBOARD     = 3049,     // 门口机密码输入键盘
    EGSC_SUBTYPE_ENTRANCE_IC_CARD_READER        = 3050,     // 门口机IC读头
    EGSC_SUBTYPE_ENTRANCE_CPU_CARD_READER       = 3051,     // 门口机CPU读头

    //EGSC_TYPE_FENCE_MACHINE = 2012, // 围墙机
    EGSC_SUBTYPE_FENCE_READER                   = 3052,     // 围墙机读头
    EGSC_SUBTYPE_FENCE_FACE_READER              = 3053,     // 围墙机人脸读卡器
    EGSC_SUBTYPE_FENCE_FINGER_READER            = 3054,     // 围墙机指纹识别读卡器
    EGSC_SUBTYPE_FENCE_QR_READER                = 3055,     // 围墙机二维码读卡器
    EGSC_SUBTYPE_FENCE_BLUETOOTH_READER         = 3056,     // 围墙机蓝牙读卡器
    EGSC_SUBTYPE_FENCE_PASSWORD_KEYBOAR         = 3057,     // 围墙机密码输入键盘
    EGSC_SUBTYPE_FENCE_IC_CARD_READER           = 3058,     // 围墙机IC读头
    EGSC_SUBTYPE_FENCE_CPU_CARD_READER          = 3059,     // 围墙机CPU读头

    //EGSC_TYPE_ELE_LINK_CTRL = 2016, // 电梯联动控制器
    EGSC_SUBTYPE_ELEVATOR_SUB_CONTROLLER        = 3037,    // 电梯联动控制子设备（电梯联动控制器里的虚拟子设备）
    EGSC_SUBTYPE_ELEVATOR_CAR                   = 3063,    // 电梯轿厢

    //EGSC_TYPE_SCREEN_CTRL   = 2018, // 信息发布屏控制器
    EGSC_SUBTYPE_INFORMATION_SCREEN             = 3019,    // 信息发布屏
    EGSC_SUBTYPE_INFORMATION_LED_SCREEN         = 3020,    // 信息LED大屏
    EGSC_SUBTYPE_INFORMATION_LCD_SCREEN         = 3021,    // 信息LCD大屏

    //EGSC_TYPE_BROADCAST_CTRL= 2019, // 广播控制器
    EGSC_SUBTYPE_BROADCAST_GROUP                = 3022,    // 广播分区

    //EGSC_TYPE_ELEVATOR_CTRL = 2020, // 电梯厂商控制器
    EGSC_SUBTYPE_ELEVATOR_IC_CARD_READER        = 3038,     // 电梯IC卡读头

    //EGSC_TYPE_SMART_CTRL_KB = 2021, // 智能控制终端
    EGSC_SUBTYPE_ELECTRIC_FENCE                 = 3060,     // 电子围栏

    //EGSC_TYPE_CARPARK_CAM   = 2022, // 车位检测相机
    EGSC_SUBTYPE_CARPORT_CAMERA_VIDEO_CHANNEL   = 3061,     // 车位检测相机通道

    //EGSC_TYPE_ELEC_LPN_CTRL = 2023, // 电子车位控制器
    EGSC_SUBTYPE_ELECTRIC_LPN_DISPLAY           = 3062,     // 电子车位显示屏

    //EGSC_TYPE_PARKING_LOCK_CONTROLLER = 2025, // 车位锁控制器
    EGSC_SUBTYPE_PARKING_LOCK                   = 3065,     // 车位锁
}EGSC_SUBDEV_TYPE;

// 厂商编号
typedef enum _EGSC_VENDOR_NUMBER
{
    EGSC_VENDOR_NUM_HIKVISION   = 1001, // 海康
    EGSC_VENDOR_NUM_DAHUA       = 1002, // 大华
    EGSC_VENDOR_NUM_JIESHUN     = 1003, // 捷顺
    EGSC_VENDOR_NUM_ANJUBAO     = 1004, // 安居宝
    EGSC_VENDOR_NUM_LEELEN      = 1005, // 立林
    EGSC_VENDOR_NUM_IBM         = 1006, // IBM
    EGSC_VENDOR_NUM_HONEYWELL   = 1007, // 霍尼韦尔
    EGSC_VENDOR_NUM_EVERGRANDE  = 1008, // 恒大
    EGSC_VENDOR_NUM_HITACHI     = 1009, // 日立电梯
    EGSC_VENDOR_NUM_QLDT        = 1010, // 三菱电梯
    EGSC_VENDOR_NUM_OTIS        = 1011, // 奥的斯电梯
}EGSC_VENDOR_NUMBER;

#endif
