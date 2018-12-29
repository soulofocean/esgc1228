#ifndef _EGSC_DEV_COMMON_H_
#define _EGSC_DEV_COMMON_H_

#include "egsc_sdk.h"
#include "egsc_def.h"

// 服务器请求回调函数定义 -----------------------------------------------------------------------

// 恢复出厂设置 COM_DEV_RESET
// userID 可选，用户编号
typedef EGSC_RET_CODE (*egsc_dev_reset_cb)(int handle, char *user_id);


// 立即校时 COM_CORRECTION
typedef EGSC_RET_CODE (*egsc_dev_correction_cb)(int handle);

// 通知设备升级 COM_NOTIFY_UPDATE
// file_url 必选，HTTP文件列表，升级文件的完整路径 
// ftp_Addr 必选，FTP文件列表，带用户名和密码的完整路径
// fm_version 可选，固件版本号，根据厂商定义，每次版本号要能区分
typedef EGSC_RET_CODE (*egsc_dev_notify_update_cb)(int handle, char *file_url, char *ftp_Addr, char *fm_version);

typedef struct _egsc_dev_parameters
{
    char fileserver_url[128];       //必选，文件HTTP服务器url 
    char ntp_server[128];           //必选，NTP服务器地址 “ip(域名)：端口”
    char http_username[128];        //可选，访问文件服务器 用户名
    char http_password[128];        //可选，访问文件服务器 密码
    void *dev_specail_param;
}egsc_dev_parameters;

// 读取参数 COM_READ_PARAMETER
typedef EGSC_RET_CODE (*egsc_dev_read_parameter_cb)(int handle, egsc_dev_parameters *dev_params);
// 设置参数 COM_SETTING_PARAMETERS
typedef EGSC_RET_CODE (*egsc_dev_setting_parameters_cb)(int handle, egsc_dev_parameters *dev_params);

typedef struct _egsc_dev_cb_certificate_param
{
    char start_time[128];       //必选，有效期开始时间yyyy-MM-dd hh:mm:ss
    char end_time[128];         //必选，有效期结束时间yyyy-MM-dd hh:mm:ss
    int credence_type;          //必选，凭证类型
    char credence_no[128];      //必选，凭证编号（车牌号、IC卡号、人脸图片ID(即人脸ID)、指纹特征码、二维码编号、NFC卡、密码号等）;其中卡号按读卡器读出来的卡片原始卡号数据的16进制表示。
    int user_type;              //下发必选，用户类别
    char user_name[128];        //可选，用户名称
    char user_id[128];          //下发可选，用户编号，电梯厂商控制器的该字段是必须下载的
    char op_time[128];          //可选，操作时间
    void *dev_special_param;    //可选，设备专用参数结构指针，与具体设备有关
}egsc_dev_cb_certificate_param;

// 下发固定凭证信息 COM_LOAD_CERTIFICATE
//  dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_load_certificate_cb)(int handle, egsc_subdev_id *dev_id, egsc_dev_cb_certificate_param *cert_param);

// 读取固定凭证信息 COM_READ_CERTIFICATE
// credence_type 必选，凭证类型
// credence_no 必选，凭证编号（车牌号、IC卡号、人脸图片ID(即人脸ID)、指纹特征码、二维码编号、NFC卡、密码号等）;其中卡号按读卡器读出来的卡片原始卡号数据的16进制表示。
typedef EGSC_RET_CODE (*egsc_dev_read_certificate_cb)(int handle, int credence_type, char *credence_no,
                                                                egsc_dev_cb_certificate_param *dev_param);

// 删除固定凭证信息 COM_DELETE_CERTIFICATE
// credence_type 必选，凭证类型
// credence_no 必选，凭证编号（车牌号、IC卡号、人脸图片ID(即人脸ID)、指纹特征码、二维码编号、NFC卡、密码号等）;其中卡号按读卡器读出来的卡片原始卡号数据的16进制表示。
// user_id 可选，用户编号，电梯厂商控制器的该字段是必须下载的
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_delete_certificate_cb)(int handle, egsc_subdev_id *dev_id, int credence_type, char *credence_no,
                                                                char *user_id);
typedef struct _egsc_dev_cb_certificate_in_batch_param
{
    char file_id[128];  //必选，文件在文件服务器上的ID，设备需要拼接文件服务器地址形成完整的文件路径。
    char op_time[128];  //必选，操作时间
}egsc_dev_cb_certificate_in_batch_param;

// 批量下发固定凭证信息 COM_LOAD_CERTIFICATE_IN_BATCH
typedef EGSC_RET_CODE (*egsc_dev_load_certificate_in_batch_cb)(int handle, egsc_dev_cb_certificate_in_batch_param *cert_batch_param);

// 批量读取固定凭证信息 COM_READ_CERTIFICATE_IN_BATCH
typedef EGSC_RET_CODE (*egsc_dev_read_certificate_in_batch_cb)(int handle, egsc_dev_cb_certificate_in_batch_param *cert_batch_param);

// 清除固定凭证操作 COM_DELETE_CERTIFICATE_IN_BATCH
// op_time 必选，操作时间
// credence_type 必选，凭证类型
typedef EGSC_RET_CODE (*egsc_dev_delete_certificate_in_batch_cb)(int handle, char *op_time, int credence_type);

// 下载黑白名单 COM_DOWNLOAD_BLACK_AND_WHITE_LIST
typedef struct _egsc_dev_cb_black_and_white_list_param
{
    int status;                     // 必选 状态 1启用，0不启用
    char start_time[128];           // 必选 开始时间yyyy-MM-dd hh:mm:ss
    char end_time[128];             // 必选 结束时间yyyy-MM-dd hh:mm:ss
    char car_no[128];               // 必选 车牌号
    char user_id[128];              // 必选 用户编号
    char user_name[128];            // 必选 用户姓名
    char op_time[128];              // 必选 操作时间 yyyy-MM-dd HH:mm:ss
    char remark[128];               // 可选 备注
    int sheet_type;                 // 必选 名单类型     1: 黑名单    2: 灰名单    3: 白名单
}egsc_dev_cb_black_and_white_list_param;

typedef EGSC_RET_CODE (*egsc_dev_download_black_and_white_list_cb)(int handle, egsc_dev_cb_black_and_white_list_param *dev_param);

// 下载剩余车位数 PAK_LOAD_LEFT_CAR_SEAT
typedef struct _egsc_dev_cb_left_car_seat_param
{
    int remaning_space;             // 必选，剩余车位数
    int all_space;                  // 可选，总车位数
}egsc_dev_cb_left_car_seat_param;
typedef EGSC_RET_CODE (*egsc_dev_pak_load_left_car_seat_cb)(int handle, egsc_dev_cb_left_car_seat_param *dev_param);



// 开关闸门 COM_GATE_CONTROL
// op_type 必选，开关指令 0- 关闭，1- 打开，2- 常开，3- 常关，4- 恢复开关指令
// user_type 必选，参考用户类型定义
// user_id 必选，用户ID
// loc_addr 可选，门号（子设备ID）
// dev_specail_param 可选，设备专用参数结构指针，与具体设备有关
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_gate_ctrl_cb)(int handle, egsc_subdev_id *dev_id, int op_type, int user_type,char *user_id,
                                                          void * dev_specail_param);
// 报语音 COM_PLAY_VOICE
// text 必选，需要播报的文字
typedef EGSC_RET_CODE (*egsc_dev_play_voice_cb)(int handle, char *text);

typedef struct _egsc_dev_vol_param
{
    int level1;             // level1 必选，设定时段声音级别
    int level2;             // level2 必选，非设定时段声音级别
    char start_time[128];   // start_time 必选，开始时间 HH:mm
    char end_time[128];     // end_time 必选，结束时间 HH:mm
}egsc_dev_vol_param;

// 读取音量参数 COM_READ_VOL
typedef EGSC_RET_CODE (*egsc_dev_read_vol_cb)(int handle, egsc_dev_vol_param *param);

// 设置音量参数 COM_SET_VOL
typedef EGSC_RET_CODE (*egsc_dev_set_vol_cb)(int handle, egsc_dev_vol_param *param);

// 电梯访客联动控制 FAC_VISIT_CONTROL
// source_floor     必选，   源楼层
//                  bit15-13			bit12-10		Bit9		Bit8-bit0
//                  000：前门(默认)		    000：保留		    0：地上		楼层
//                  001：后门			    (默认000)		    1：地下		1-511层
//                  其它：保留
//                  111					111				1			1FF：即全FFFF代表所有楼层
// direction        可选，   方向1上，2下
// source_room      可选，   房屋号
// dest_floor_num   必选，目标楼层个数
// dest_floor       必选，   目标楼层可以多个，
//                  bit15-13			bit12-10		Bit9		Bit8-bit0
//                  000：前门(默认)		    000：保留		    0：地上		楼层
//                  001：后门			    (默认000)		    1：地下		1-511层
//                  其它：保留
//                  111					111				1			1FF：即全FFFF代表所有楼层
// dest_room        可选，   房屋号
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_fac_visit_control_cb)(int handle, egsc_subdev_id *dev_id, int source_floor, int direction, char *source_room,
                                                         int dest_floor_num, int *dest_floor, char *dest_room);
// 电梯按键权限控制 FAC_KEY_CONTROL
// control_mode     必选，   控制模式，1表示指定控制，2表示电梯自动选择控制
// dest_floor_num   必选，目标楼层个数
// dest_floor       必选，   目标楼层可以多个，
//                  bit15-13			bit12-10		Bit9		Bit8-bit0
//                  000：前门(默认)		    000：保留		    0：地上		楼层
//                  001：后门			    (默认000)		    1：地下		1-511层
//                  其它：保留
//                  111					111				1			1FF：即全FFFF代表所有楼层
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_fac_key_control_cb)(int handle, egsc_subdev_id *dev_id, int control_mode, int dest_floor_num, int *dest_floor);

// 外呼叫电梯控制 FAC_CALLING
// source_floor  必选，   源楼层
//                  bit15-13			bit12-10		Bit9		Bit8-bit0
//                  000：前门(默认)		    000：保留		    0：地上		楼层
//                  001：后门			    (默认000)		    1：地下		1-511层
//                  其它：保留
//                  111					111				1			1FF：即全FFFF代表所有楼层
// direction    必选，   方向1上，2下
// source_room  可选，   房屋号
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_fac_calling_cb)(int handle, egsc_subdev_id *dev_id, int source_floor, int direction, char *source_room);

// 内呼叫电梯授权控制 FAC_INTER_CALL_AUTH
// elevator_addr    必选，   轿厢编号，范围: 0x01-0x0A  最多为10个轿厢，0xFF代表所有轿厢
// dest_floor_num   必选，目标楼层个数
// dest_floor       必选，   目标楼层可以多个，
//                  bit15-13			bit12-10		Bit9		Bit8-bit0
//                  000：前门(默认)		    000：保留		    0：地上		楼层
//                  001：后门			    (默认000)		    1：地下		1-511层
//                  其它：保留
//                  111					111				1			1FF：即全FFFF代表所有楼层
// dest_room        可选，   房屋号
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_fac_inter_call_auth_cb)(int handle, egsc_subdev_id *dev_id, char *elevator_addr, int dest_floor_num, int *dest_floor, char *dest_room);

// 内呼叫电梯点亮控制 FAC_INTER_CALL_LIGHTING
// elevator_addr    必选，   轿厢编号，范围: 0x01-0x0A  最多为10个轿厢，0xFF代表所有轿厢
// dest_floor_num   必选，目标楼层个数
// dest_floor       必选，   目标楼层可以多个，
//                  bit15-13			bit12-10		Bit9		Bit8-bit0
//                  000：前门(默认)		    000：保留		    0：地上		楼层
//                  001：后门			    (默认000)		    1：地下		1-511层
//                  其它：保留
//                  111					111				1			1FF：即全FFFF代表所有楼层
// dest_room        可选，   房屋号
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_fac_inter_call_lighting_cb)(int handle, egsc_subdev_id *dev_id, char *elevator_addr, int dest_floor_num, int *dest_floor, char *dest_room);

// 电梯轿厢按键点亮控制 FAC_LIFT_LIGHTING
// source_floor  必选，   源楼层
//                  bit15-13			bit12-10		Bit9		Bit8-bit0
//                  000：前门(默认)		    000：保留		    0：地上		楼层
//                  001：后门			    (默认000)		    1：地下		1-511层
//                  其它：保留
//                  111					111				1			1FF：即全FFFF代表所有楼层
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_fac_lift_lighting_cb)(int handle, egsc_subdev_id *dev_id, int source_floor);

// 电梯轿厢延迟关门控制 FAC_DELAYED_CLOSING
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_fac_delayed_closing_cb)(int handle, egsc_subdev_id *dev_id);

// 电梯轿厢状态查询 FAC_STATUS_REQ
// device_id    必选，轿厢状态
// 
// door_status  必选，   轿厢门状态
//                  10：电梯门打开中      11：电梯门已打开（全开）
//                  12：电梯门关闭中		13：电梯门已关闭（全关）
//                  80：故障
// floor        必选，   目标楼层可以多个，
//                  bit15-13			bit12-10		Bit9		Bit8-bit0
//                  000：前门(默认)		    000：保留		    0：地上		楼层
//                  001：后门			    (默认000)		    1：地下		1-511层
//                  其它：保留
//                  111					111				1			1FF：即全FFFF代表所有楼层
typedef struct _egsc_dev_cb_fac_status_param
{
    char status[128];           //回复必选，轿厢状态
                                //      00：停止， 		80：故障
                                //      01：向上运行中，11上行过程的停止
                                //      02：向下运行中，12下行过程的停止
    char door_status[128];      //回复必选，轿厢门状态
                                //      10：电梯门打开中      11：电梯门已打开（全开）
                                //      12：电梯门关闭中		13：电梯门已关闭（全关）
                                //      80：故障
    int floor;                  //回复必选，轿厢楼层信息
                                //      bit15-13			bit12-10		Bit9		Bit8-bit0
                                //      000：前门(默认)		    000：保留		    0：地上		楼层
                                //      001：后门			    (默认000)		    1：地下		1-511层
                                //      其它：保留
                                //      111					111				1			1FF：即全FFFF代表所有楼层
}egsc_dev_cb_fac_status_param;
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_fac_status_req_cb)(int handle, egsc_subdev_id *dev_id,
                                                            egsc_dev_cb_fac_status_param *fac_status_param);

// 批量查询电梯轿厢状态 FAC_LIFT_BA_STATUS
// car_id_list  必选，轿厢编号，轿厢编号的定义: 1-10，最多为10个轿厢，FF代表所有轿厢。在本接口中，允许一次可以查询本电梯厂商控制器下的一个轿厢，部分轿厢，或者全部轿厢，如查询多个轿厢，轿厢编号间拥抱过逗号隔开。
// timestamp    必选，   平台发出查询指令的时间，
typedef struct _egsc_dev_cb_lift_car_status_param
{
    egsc_subdev_id subdev_id;   //必选，电梯厂商控制器ID，子设备id结构，主设备则填空
    int car_id;                 //必选，轿厢编号
    int physical_floor;         //必选，电梯轿厢当前所在的楼层，例如：14
    char display_floor[128];    //可选，电梯轿厢当前所在的显示楼层，例如：13A
    char car_status[128];       //必选，轿厢状态
                                //      00：停止，
                                //      01：向上运行中，11上行过程的停止
                                //      02：向下运行中，12下行过程的停止
    char door_status[128];      //必选，轿厢门状态
                                //      10：电梯门打开中      11：电梯门已打开（全开）
                                //      12：电梯门关闭中		13：电梯门已关闭（全关）
    int error_status;           //必选，电梯故障状态，0：表示无故障；1：表示有故障。
    char error_message[128];    //必选，电梯故障描述
    int fire_ctrl_status;       //可选，消防状态，接收到大楼消防信号后，启动消防迫降。0:非消防状态，1:消防状态
}egsc_dev_cb_fac_lift_car_status_param;
typedef struct _egsc_dev_cb_fac_lift_ba_status_param
{
    int car_id_list_len;        //下发必选，轿厢编号数组有效长度
    int car_id_list[10];        //下发必选，轿厢编号，轿厢编号的定义: 1-10，最多为10个轿厢，FF代表所有轿厢。在本接口中，允许一次可以查询本电梯厂商控制器下的一个轿厢，部分轿厢，或者全部轿厢，如查询多个轿厢，轿厢编号间拥抱过逗号隔开。
    int res_lift_car_num;       //回复必选，应答轿厢状态数组长度
    egsc_dev_cb_fac_lift_car_status_param *lift_car_status;     //回复必选，轿厢状态数组，参考：轿厢状态LiftCarStatus的定义。如电梯厂商控制器下绑定了多个轿厢，则需要将控制器下面的每个轿厢的当前状态返回给平台
    char time_stamp[128];       //必选，平台发出查询指令的时间（请求）/电梯厂商控制器进行查询的时间（应答）
}egsc_dev_cb_fac_lift_ba_status_param;
typedef EGSC_RET_CODE (*egsc_dev_fac_lift_ba_status_cb)(int handle, egsc_dev_cb_fac_lift_ba_status_param *lift_ba_status_param);

// 下发信息到电子车位显示屏 PAK_SEND_SHOWINFO
// text         必选，显示屏需要显示的文字，如需换行显示时用“|”进行分隔。示例如下：“A区022|京A12345 京A888888 ” 表示第一行显车位号第二行显示车牌信息
// subdevice_id 必选，子设备ID号，当一个控制机下有多块显示屏时需要指定该ID号。
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_pak_send_showinfo_cb)(int handle, egsc_subdev_id *dev_id, char *text);

// 升降车位锁 PAK_CONTROL_LOCK
// place_no     必选，车位编号
// placelock_no 必选，车位锁编号: 规则为“车位锁控制机设备编号:锁编号”例如：001899242401:1 表示车位锁控制机001899242401下的1号车位锁
// operate_type 必选，操作指令 0-降落，1-升起
typedef EGSC_RET_CODE (*egsc_dev_pak_control_lock_cb)(int handle, char *place_no, char *placelock_no, int operate_type);

// 周界防区布防设置 BOUN_CHAN_SETUPALARM
// setup_type       必选，设置类型 0-撤防，1-布防
// alarmzone_chan   可选，防区通道标记，可变长，从左到右依次用1或0标记1至X号防区，1代表需要设置，0则不需要。例如：对4和9防区操作，发送"000100001"
typedef EGSC_RET_CODE (*egsc_dev_boun_chan_setupalarm_cb)(int handle, int setup_type, char *alarmzone_chan);

// 周界子系统消警 BOUN_SUBCHAN_CLEARALARM
// subsystem_num    必选，子系统号
typedef EGSC_RET_CODE (*egsc_dev_boun_subchan_clearalarm_cb)(int handle, int subsystem_num);

// 设备状态查询 COM_QUERY_DEV_STATUS
// state    必选，工作状态
// dev_id       必选 子设备id结构，主设备则填NULL
typedef EGSC_RET_CODE (*egsc_dev_query_dev_status_cb)(int handle, egsc_subdev_id *dev_id, int *state);


// 图片抓拍 COM_SNAP_PICTURE
// image_id    文件在文件服务器上的ID
typedef EGSC_RET_CODE (*egsc_dev_snap_picture_cb)(int handle, char *image_id, int len);

// 地感复位 PAK_RESET_DG
typedef EGSC_RET_CODE (*egsc_dev_pak_reset_dg_cb)(int handle);


// 停车场LED显示屏显示数据 PAK_LED_DISPLAY
// text 必选，需要显示的文字
typedef EGSC_RET_CODE (*egsc_dev_pak_led_display_cb)(int handle, char *text);


// 加载停车场LED广告信息 PAK_LOAD_LED_INFO
// text 必选，需要显示的文字
typedef EGSC_RET_CODE (*egsc_dev_pak_load_led_info_cb)(int handle, char *text);

// 加载停车场LED广告信息 PAK_INTERCOM_CONTROL
// command_type 命令类型，1：请求对讲；2：应答对讲；3：结束/拒绝对讲
// sdp SDP协商信息(请求对讲和应答对讲时必填)

typedef EGSC_RET_CODE (*egsc_dev_rsp_pak_intercom_control_cb)(int handle, int command_type, char *sdp);

// 透传素材属性 ADS_TRANS_MATERIAL
typedef struct _screen_terminal_id
{
    egsc_subdev_id *id;   // 必选 子设备id结构, 主设备为NULL
}screen_terminal_id;
typedef struct _screen_material
{
    char account_name[128];     // 可选 素材服务器账户名
    char account_passwd[128];   // 可选 素材服务器密码
    int  material_id;           // 必选 必须和节目的页面中窗口包含的素材ID一致
    char url[1024];             // 必选 素材的url，下载素材地址索引
    char material_name[128];    // 可选 素材的名称,最大长度128字节
    char material_encrypt[128]; // 可选 素材的加密串,最大长度256字节
    char material_type[128];    // 必选 素材分类，picture,vedio,audio,text
    char format[128];           // 必选 素材格式 picture--jpg,bmp,gif,png; vedio--asf,avi,mpg,3gp,mov,mkv; audio--mp3,wav,wma;text--txt
    int  file_size;             // 必选 素材大小，单个素材最大支持4G
    int  duration;              // 必选 音视频素材的时长不建议小于10s，非音视频时长写0
    char character[4096];       // 可选 最大支持1024中文，如需更多文字则必须下载文本
}screen_material;

typedef struct _egsc_dev_trans_material
{
    int schedule_id;                    // 必选 scheduleId 该素材列表是属于哪个日程ID下的
    int seq;                            // 必选 seq 该日程ID的唯一标示，避免同一日程不同数据
    int terminal_cnt;                   // 必选 终端ID值列表个数
    screen_terminal_id *terminal_no;    // 必选 terminalNo 终端ID值列表，最多1000，具体数据参考9.13, 该字段为对象数组
    int material_cnt;                   // 必选 素材列表个数
    screen_material *material;          // 必选 material 素材列表数据，最多支持512个不重复素材
}egsc_dev_trans_material;
typedef EGSC_RET_CODE (*egsc_dev_trans_material_cb)(int handle, egsc_dev_trans_material *screen_param);


// 查询终端 ADS_QUERY_TERM
typedef struct _egsc_dev_ads_query_term
{
    int search_id;                      // 必选 查找ID（大于1的整数）
    char terminal_name_like[128];       // 必选 终端名称（中文32个，英文64个）
    char terminal_remarks_like[128];    // 必选 终端名称描述
    egsc_subdev_id *serial;             // 可选 子设备id结构, 主设备为NULL
    char online_state[16];              // 可选 在线状态，Online，offline，all（首字母需小写）
    char publish_state[32];             // 可选 发布状态列表,发布状态，publishing，success，failed，null
    char play_state[32];                // 必选 播放状态列表,播放状态，HDMI，VGA，schedulePlay，scheduleStop, ScreenOff
    int  max_result;                    // 必选 最大返回的终端数目,默认100
    int  search_results_position;       // 必选 本次搜索的起始结果索引值（第一次搜索传1，例如返回100个后，第二次传101）
}egsc_dev_ads_query_term;

typedef struct _egsc_ip_address
{
    char ip_version[32];                // 必选 v4
    char ip_address[32];                // 必选 V4地址信息
}egsc_ip_address;

typedef struct _egsc_resolution
{
    char width[8];                      // 必选 宽分辨率（分辨率取值范围：0 - 1920）
    char height[8];                     // 必选 高分辨率（分辨率取值范围：0 - 1920）
}egsc_resolution;


typedef struct _egsc_terminalinfo
{
    int id;                             // 必选 终端号
    char terminal_name[128];            // 必选 终端名称
    char terminal_type[64];             // 必选 终端类型normal,decode,touch, decodeTouch
    char terminal_remarks[64];          // 必选 终端名称模糊搜索
    char terminal_remarks_like[64];     // 必选 终端描述模糊搜索
    char org_name[32];                  // 必选 组织名称
    char online_state[32];              // 必选 在线状态
    egsc_ip_address ip_address;         // 必选 IP
    int port;                           // 必选 端口号
    char serial_no[32];                 // 必选 终端序列号
    char software_version[32];          // 必选 软件版本
    char publish_state[32];             // 必选 发布状态
    char insert_state[32];              // 必选 插播状态
    char play_state[32];                // 必选 播放状态
    egsc_resolution resolution;         // 必选 分辨率
}egsc_terminalinfo;

typedef struct _egsc_query_recv
{
    char search_id[16];                 // 必选 由客户端分配的搜索ID
    char response_status[16];           // 必选 搜索是否成功（true  或者  false）
    char response_status_string[32];    // 必选 搜索状态字符串描述，true+OK表示没有更多的结果了，true+MORE表示（取值说明：OK、FAILED、MORE、NO、MATCH、PARAM、ERROR、TIMEOUT）
    int  total_matchs;                  // 必选 搜索条件匹配的结果总数
    int  num_of_matchs;                 // 必选 本次搜索匹配的结果数（例如：总的匹配数为500，单次返回100，这个值就是100）
    egsc_terminalinfo terminal_info;    // 必选 具体的终端参数
}egsc_query_recv;
typedef EGSC_RET_CODE (*egsc_dev_ads_query_term_cb)(int handle, egsc_dev_ads_query_term *query_param, egsc_query_recv *recv_param);


// 新增节目 ADS_ADD_PROGRAM

typedef struct _egsc_play_duration
{
    char duration_type[32];                         // 必选 时长类型, materialTime,selfDefine
    int duration;                                   // 必选 时长-秒
}egsc_play_duration;

typedef struct _egsc_character_effect
{
    int font_size;                                  // 必选 字体大小（取值范围 30-400）
    int color;                                      // 必选 颜色（信息发布涉及颜色取值范围 0-16777215）
    int back_color;                                 // 必选 背景色
    int back_transparent;                           // 必选 透明度（取值范围 0-255）
    char scroll_direction[16];                      // 必选 滚动方向，left， right，up，down
    int scrol_speed;                                // 必选 滚动速度（取值范围 1-9）
}egsc_character_effect;

typedef struct _egsc_plat_item
{
    int id;                                         // 必选 播放序号
    int material_no;                                // 必选 素材标号
    egsc_character_effect character_effect;         // 可选 文字效果 
    egsc_play_duration play_duration;               // 必选 播放时长
    int page_time;                                  // 必选 翻页时间  当素材为word,ppt,pdf,excel时有效（取值范围：15-60秒）
}egsc_plat_item;

typedef struct _egsc_win_material_info
{
    char win_material_type[32];                     // 必选 素材类型，只支持静态素材，static（默认）
    char static_material_type[32];                  // 必选 素材类型：picture ， audio video ， document ， pdf
}egsc_win_material_info;

typedef struct _egsc_position
{
    int x_position;                                 // 必选 X坐标 按1920*1920
    int y_position;                                 // 必选 Y坐标
    int height;                                     // 必选 高度
    int width;                                      // 必选 宽度
}egsc_position;

typedef struct _egsc_pag_base_info
{
    char pag_name[64];                              // 必选 页面名称
    int back_ground_color;                          // 必选 背景色（取值范围 0-16777215）
    char play_time_mod[32];                         // 必选 播放时间模式, selfDefine,auto
    int play_time;                                  // 必选 播放时间（（单位/秒），取值范围60 - 7*24*3600）
    int back_ground_pid_id;                         // 必选 背景图片素材id
}egsc_pag_base_info;

typedef struct _egsc_windows
{
    int id;                                         // 必选 内容编号
    egsc_position position;                         // 必选 内容位置
    int layer_no;                                   // 必选 图层信息（从1开始依次递增）
    egsc_win_material_info win_material_info;       // 必选 窗口素材
    int plat_item_cnt;                              // 必选 播放列表长度
    egsc_plat_item *plat_item_list;                 // 必选 播放列表
}egsc_windows;

typedef struct _egsc_pag_list
{
    int id;                                         // 必选 页面id（从1开始依次递增）
    egsc_pag_base_info pag_base_info;               // 必选 页面基本信息
    int windows_list_len;                           // 必选 窗口信息长度
    egsc_windows *windows_list;                     // 必选 窗口信息
}egsc_pag_list;

typedef struct _egsc_dev_ads_program
{
    int id;                         // 必选 节目索引
    char program_name[128];         // 必选 节目名称
    char program_remark[512];       // 可选 节目描述
    char approve_state[32];         // 必选 审核状态 approved 已审核(默认)
    char program_type[16];          // 必选 节目类型  normal 标准（默认）
    int pag_list_cnt;               // 必选 页面列表长度
    egsc_pag_list *pag_list;        // 必选 页面列表
    int  image_width;               // 可选 分辨率宽
    int  image_height;              // 可选 分辨率高（分辨率为必须的，且只能为 1920*1080 或者 1080*1920）
}egsc_dev_ads_program;
typedef EGSC_RET_CODE (*egsc_dev_ads_add_program_cb)(int handle, egsc_dev_ads_program *add_param, int *program_id);

// 删除节目 ADS_DELETE_PROGRAM
typedef struct _egsc_dev_ads_delete
{
    int del_cnt;                    // 必选 节目id列表个数
    int *id;                        // 必选 节目id列表
}egsc_dev_ads_delete;
typedef EGSC_RET_CODE (*egsc_dev_ads_delete_program_cb)(int handle, egsc_dev_ads_delete *screen_param);

// 修改节目 ADS_SET_PROGRAM
typedef EGSC_RET_CODE (*egsc_dev_ads_set_program_cb)(int handle, egsc_dev_ads_program *screen_param, int *program_id);


// 新增日程 ADS_ADD_SCHEDULE
typedef struct _egsc_daily_schedule
{
    int id;                                 // 必选 时间段序号（从1开始，依次递增）
    int program_no;                         // 必选 节目ID
    char begin_time[128];                   // 必选 开始时间, ISO8601格式的串(见timeSpan注释)
    char end_time[128];                     // 必选 结束时间
}egsc_daily_schedule;

typedef struct _egsc_weeky_schedule
{
    int id;                                 // 必选 天序号 周一~周日 1-7
    char day_of_week[32];                   // 必选 monday,tuesday,…,sunday
    int weeky_daily_cnt;
    egsc_daily_schedule *daily_schedule;    // 必选 日计划
}egsc_weeky_schedule;

typedef struct _egsc_dev_ads_schedule
{
    int id;                                 // 必选 日程id--大于0的任意值，最终id由信息发布服务器分配
    char schedule_name[128];                // 必选 日程名称
    char schedule_remarks[128];             // 可选 日程描述
    char approve_remarks[128];              // 可选 审核描述（空）
    char approve_state[128];                // 必选 审核状态 approved已审核（默认值）
    char schedule_mode[32];                 // 必选 日程模式，节目类型 1-标准（默认值）
    char schedule_type[32];                 // 必选 日程类型 1-daily 2-weekly
    int daily_cnt;
    egsc_daily_schedule *daily_schedule;    // 可选 日计划
    int weeky_cnt;
    egsc_weeky_schedule *weeky_schedule;    // 可选 周计划
    int  priority;                          // 可选 优先级，范围：0~9    0：highest, 9：lowest    空：默认9，最低优先级
    char start_date[128];                   // 可选 开始日期, ISO8601格式的串(见timeSpan注释),    示例：20180101
    char end_date[128];                     // 可选 结束日期
}egsc_dev_ads_schedule;
typedef EGSC_RET_CODE (*egsc_dev_ads_add_schedule_cb)(int handle, egsc_dev_ads_schedule *screen_param, int *schedule_id);


// 删除日程 ADS_DELETE_SCHEDULE
typedef EGSC_RET_CODE (*egsc_dev_ads_delete_schedule_cb)(int handle, egsc_dev_ads_delete *screen_param);

// 修改日程 ADS_SET_SCHEDULE
typedef EGSC_RET_CODE (*egsc_dev_ads_set_schedule_cb)(int handle, egsc_dev_ads_schedule *screen_param, int *set_id);


// 发布日程 ADS_PUBLISH_SCHEDULE
typedef struct _egsc_terminal_nolist
{
    egsc_subdev_id *terminal_no;    // 必选 子设备id结构, 主设备为NULL
}egsc_terminal_nolist;

typedef struct _egsc_dev_ads_publish_schedule
{
    int id;                                     // 必选 日程id--大于0的任意值，最终id由信息发布服务器分配
    char effective_time[128];                   /* 必选 生效时间，默认立即生效（1、传入空字符串时表示立即生效，
                                                    2、若不希望立即生效，可以传入其他时间，但需大于当前时间。
                                                    时间格式如下：ISO8601格式 %04d%02d%02dT%02d%02d%02d+%02d 
                                                    示例：20171219T024400+08）*/
    char release_type[128];                     // 必选 发布类型，按照终端发布 byTerminal
    int list_cnt;
    egsc_terminal_nolist *terminal_nolist;       // 必选 按照终端发布时的终端列表
}egsc_dev_ads_publish_schedule;

typedef EGSC_RET_CODE (*egsc_dev_ads_publish_schedule_cb)(int handle, egsc_dev_ads_publish_schedule *screen_param, int *pub_id);



// ---------------------------------------------------------------------------------------------------------


// 设备请求接口函数定义 --------------------------------------------------------------------------

// 设备状态上报 COM_UPLOAD_DEV_STATUS
typedef void (*egsc_dev_upload_dev_status_res_cb)(int handle, int req_id, EGSC_RET_CODE ret);
/*
    lockDeviceID 必选，当设备类型为车位锁时，该字段为必填，取值锁控制机下的锁编号
    status 必选，设备状态值
    sub_dev_id 可选， 子设备的唯一标识
*/
typedef struct _egsc_dev_upload_dev_status_param
{
    int device_type;        //必选，设备类型
    int lock_device_id;     //可选，当设备类型为车位锁时，该字段为必填，取值锁控制机下的锁编号
    int device_status;      //必选，设备状态
}egsc_dev_upload_dev_status_param;
typedef EGSC_RET_CODE (*egsc_dev_upload_dev_status_if)(int handle, egsc_subdev_id *dev_id, int req_id, egsc_dev_upload_dev_status_res_cb res_cb,
                                                                egsc_dev_upload_dev_status_param *param);

// 记录上传 COM_UPLOAD_RECORD
typedef void (*egsc_dev_upload_record_res_cb)(int handle, int req_id, EGSC_RET_CODE ret);
typedef struct _egsc_dev_upload_record_param
{
    char *record_time;              //必选，时间，格式化为”yyyy-MM-dd HH:mm:ss”
    int record_type;                //必选，记录类型
    int credence_type;              //必选，凭证类型
    char *user_id;                  //可选，用户ID，失败记录不提供
    int user_type;                  //可选，用户类型
    char *recognise_capture_image;  //可选，图片ID
    void *dev_special_param;        //可选，设备专用参数结构指针，与具体设备有关
}egsc_dev_upload_record_param;
typedef EGSC_RET_CODE (*egsc_dev_upload_record_if)(int handle, egsc_subdev_id *dev_id, int req_id, egsc_dev_upload_record_res_cb res_cb,
                                                             egsc_dev_upload_record_param *param);

// 电梯设备状态上报 FAC_UPLOAD_DEV_STATUS
typedef void (*egsc_dev_upload_fac_dev_status_res_cb)(int handle, int req_id, EGSC_RET_CODE ret);
typedef struct _egsc_dev_upload_fac_dev_status_param
{
    char *work_mode;    // 必选， 工作模式：0:普通模式；1:授权模式；默认：0
    int state;          // 必选， 工作状态，1：正常运行，2：故障状态
    int floor;          // 必选， 楼层，按照物理楼层定义
    int dicrection;     // 必选， 方向：0停，1上，2下
}egsc_dev_upload_fac_dev_status_param;
typedef EGSC_RET_CODE (*egsc_dev_upload_fac_dev_status_if)(int handle, egsc_subdev_id *dev_id, int req_id, egsc_dev_upload_fac_dev_status_res_cb res_cb,
                                                      egsc_dev_upload_fac_dev_status_param *param);

// 乘梯事件上报 FAC_ELEVATOR_RECORD
typedef void (*egsc_dev_upload_fac_elevator_record_res_cb)(int handle, int req_id, EGSC_RET_CODE ret);
typedef struct _egsc_dev_upload_fac_elevator_record_param
{
    int record_type;        // 必选， 记录类型
    int user_type;          // 可选， 用户类别
    int credence_type;      // 必选， 凭证类型，若无凭证类型则填-1
    char *credence_no;      // 必选， 凭证编号，若无凭证编号则填空串
    char *user_id;          // 可选， 凭证编号，若无凭证编号则填空串
    int dest_floor_num;     // 可选， 有效卡可带目标楼层个数
    int *dest_floor;        // 可选， 有效卡可带目标楼层，楼层表示方法：
                            // bit15-13            bit12-10        Bit9        Bit8-bit0
                            // 000：前门(默认)            000：保留            0：地上      楼层
                            // 001：后门                (默认000)         1：地下      1-511层
                            // 其它：保留
                            // 111                 111             1           1FF：即全FFFF代表所有楼层
    char *light_mode;       // 必选， 点亮模式，当wordMode为1时，此参数才有效
    char *op_time;          // 必选， 操作时间
}egsc_dev_upload_fac_elevator_record_param;
typedef EGSC_RET_CODE (*egsc_dev_upload_fac_elevator_record_if)(int handle, egsc_subdev_id *dev_id, int req_id, egsc_dev_upload_fac_elevator_record_res_cb res_cb,
                                                      egsc_dev_upload_fac_elevator_record_param *param);

// 电梯轿厢状态上报 FAC_UPLOAD_BA_STATUS
typedef void (*egsc_dev_upload_fac_ba_status_res_cb)(int handle, int req_id, EGSC_RET_CODE ret);
typedef struct _egsc_dev_upload_fac_ba_status_param
{
    int lift_car_status_num;                                //必选， 轿厢状态数组长度
    egsc_dev_cb_fac_lift_car_status_param *lift_car_status; //必选， 轿厢状态数组，参考：轿厢状态LiftCarStatus的定义
    char *timestamp;        // 必选， 轿厢状态发生变化的时间
}egsc_dev_upload_fac_ba_status_param;
typedef EGSC_RET_CODE (*egsc_dev_upload_fac_ba_status_if)(int handle, egsc_subdev_id *dev_id, int req_id, egsc_dev_upload_fac_ba_status_res_cb res_cb,
                                                      egsc_dev_upload_fac_ba_status_param *param);

// 事件上报 COM_UPLOAD_EVENT
typedef void (*egsc_dev_upload_event_res_cb)(int handle, int req_id, EGSC_RET_CODE ret);
typedef struct _egsc_dev_upload_event_param
{
    int event_type;             // 必选， 事件类型
    char * time;                // 必选， 设置的时间，格式 ” yyyy-MM-dd HH:mm:ss”
    char * lock_no;             // 可选， 车位锁编号
    char * addr;                // 可选， 事件地址
    char * desc;                // 可选， 事件描述
    char * pic_id;              // 可选， 图片ID路径
    char * abs_time;            // 可选， 绝对时标
    void * dev_special_param;   // 可选，设备专用参数结构指针，与具体设备有关
}egsc_dev_upload_event_param;
typedef EGSC_RET_CODE (*egsc_dev_upload_event_if)(int handle, egsc_subdev_id *dev_id, int req_id, egsc_dev_upload_event_res_cb res_cb,
                                                             egsc_dev_upload_event_param *param);

// 子设备信息上报 COM_UPLOAD_SUB_DEV
typedef void (*egsc_dev_upload_sub_dev_res_cb)(int handle, int req_id, EGSC_RET_CODE ret);
typedef struct _egsc_dev_upload_sub_dev_param
{
    char *device_id;            // 必选　设备ID
    char *name;                 // 必选  设备名称
    char *manufacturer;         // 可选  厂商
    int device_type;            // 可选  设备类型
    int device_detail_type;     // 可选  设备型号
    int register_type;          // 可选 注册类型
    char *ip;                   // 可选 IP地址
    int port;                   // 可选 端口
    char *user_name;            // 可选 用户名
    char *pass_word;            // 可选 密码
    char *version;              // 必选 系统版本号
    int sub_device_type;        // 必选 子设备编号
    int number;                 // 可选 编号
    char *software_version;     // 可选 软件版本号
}egsc_dev_upload_sub_dev_param;
typedef EGSC_RET_CODE (*egsc_dev_upload_sub_dev_if)(int handle, int req_id,
    egsc_dev_upload_sub_dev_res_cb res_cb, egsc_dev_upload_sub_dev_param *param);


// 凭证批量下发处理状态上报 COM_CREDENCE_LOAD_RESULT
typedef void (*egsc_dev_upload_credence_load_result_cb)(int handle, int req_id, EGSC_RET_CODE ret);
typedef struct _egsc_dev_upload_credence_load_result_list_item_param
{
    int credence_type;          // 必选，凭证类型
    char credence_no[128];      // 必选，凭证编号
    char user_id[128];          // 必选，用户编号
    int error_code;             // 必选，错误码（参考章节9.1异常代码定义 ErrorCodeType）
    char error_message[128];    // 可选，错误说明
}egsc_dev_upload_credence_load_result_list_item_param;
typedef struct _egsc_dev_upload_credence_load_result_param
{
    int device_type;        // 必选，设备类型
    int list_len;           // 必选，list长度
    egsc_dev_upload_credence_load_result_list_item_param *item;     // 可选，详细凭证列表
}egsc_dev_upload_credence_load_result_param;
typedef EGSC_RET_CODE (*egsc_dev_upload_credence_load_result_if)(int handle, egsc_subdev_id *dev_id, int req_id, egsc_dev_upload_credence_load_result_cb res_cb,
                                                            egsc_dev_upload_credence_load_result_param *param);

// 停车场对讲控制 PAK_INTERCOM_CONTROL
typedef void (*egsc_dev_req_pak_intercom_control_cb)(int handle, int req_id, EGSC_RET_CODE ret);
typedef struct _egsc_dev_pak_intercom_control_param
{
    int command_type;    // 必选，　设备ID
    char sdp[2048];
}egsc_dev_pak_intercom_control_param;
typedef EGSC_RET_CODE (*egsc_dev_pak_intercom_control_if)(int handle, int req_id,
    egsc_dev_req_pak_intercom_control_cb res_cb, egsc_dev_pak_intercom_control_param *param);

// ---------------------------------------------------------------------------------------------------------



#endif
