#ifndef _EGSC_PARKING_CTRL_H_
#define _EGSC_PARKING_CTRL_H_

#include "../egsc_command.h"

// 车场控制器 EGSC_TYPE_PARKING_CTRL -------------------------------------------------
// 设备专有参数
typedef struct _egsc_parking_ctrl_setting_parameters_param
{
    char talk_ip1[32];              // 可选，对讲IP1
    char talk_port1[8];             // 可选，对讲端口1
    char start_time[128];           // 可选，开始时间
    char end_time[128];             // 可选，结束时间
    char talk_ip2[32];              // 可选，对讲IP2
    char talk_port2[8];             // 可选，对讲端口2
    char mac_no[128];               // 可选，设备机号
    char camera1_rtsp_addr[32];     // 可选，第一个摄像头rtsp地址
    char camera2_rtsp_addr[32];     // 可选，第二个摄像头rtsp地址
    char cameras_synergism[128];    // 可选，摄像头协同工作模式：0不协同，1双路协同
    int  device_entry_type;         // 可选，进出类型
}egsc_parking_ctrl_setting_parameters_param;

typedef struct _egsc_parking_ctrl_certificate_special_param
{
    char place_no[32];              // 可选，车位编号
    char place_lock_no[32];         // 可选，车位锁编号 30640018992424010001:1
}egsc_parking_ctrl_certificate_special_param;

typedef struct _egsc_parking_ctrl_upload_record_param
{
	int gate_open_mode;             // 可选，开闸方式，1:自动开闸,2:确认开闸，3:手动开闸，4:脱机开闸
	char credence_no[32];           // 必选，用来让平台区分开门次数
	char rec_car_no_color[32];      // 可选，车颜色,车场设备提供
    char car_logo[32];              // 可选，车标，车场设备提供
    char car_type[32];              // 可选，车型，车场设备提供
    int device_entry_type;          // 可选，进出类型
}egsc_parking_ctrl_upload_record_param;

// 服务器请求回调函数表
typedef struct _egsc_parking_ctrl_cb_tbl
{
    egsc_dev_reset_cb                       reset_cb;
    egsc_dev_correction_cb                  correction_cb;
    egsc_dev_notify_update_cb               notify_update_cb;
    egsc_dev_read_parameter_cb              read_parameter_cb;
    egsc_dev_setting_parameters_cb          setting_parameters_cb;
    egsc_dev_load_certificate_cb            load_certificate_cb;
    egsc_dev_read_certificate_cb            read_certificate_cb;
    egsc_dev_delete_certificate_cb          delete_certificate_cb;
    egsc_dev_load_certificate_in_batch_cb   load_certificate_in_batch_cb;
    egsc_dev_read_certificate_in_batch_cb   read_certificate_in_batch_cb;
    egsc_dev_delete_certificate_in_batch_cb delete_certificate_in_batch_cb;
    egsc_dev_gate_ctrl_cb                   gate_ctrl_cb;
    egsc_dev_play_voice_cb                  play_voice_cb;
    egsc_dev_read_vol_cb                    read_vol_cb;
    egsc_dev_set_vol_cb                     set_vol_cb;
    egsc_dev_query_dev_status_cb            query_dev_status_cb;
    egsc_dev_snap_picture_cb                snap_picture_cb;
    egsc_dev_pak_reset_dg_cb                pak_reset_dg_cb;
    egsc_dev_pak_led_display_cb             pak_led_display_cb;
    egsc_dev_pak_load_led_info_cb           pak_load_led_info_cb;
    egsc_dev_pak_control_lock_cb            pak_control_lock_cb;
    egsc_dev_rsp_pak_intercom_control_cb    pak_intercom_control_cb;
    egsc_dev_download_black_and_white_list_cb   download_black_and_white_list_cb;
    egsc_dev_pak_load_left_car_seat_cb      pak_load_left_car_seat_cb;
}egsc_parking_ctrl_cb_tbl;

// 设备请求接口函数表
typedef struct _egsc_parking_ctrl_if_tbl
{
    egsc_dev_upload_dev_status_if   upload_dev_status_if;
    egsc_dev_upload_record_if       upload_record_if;
    egsc_dev_upload_event_if        upload_event_if;
    egsc_dev_upload_credence_load_result_if      upload_credence_load_result_if;
    egsc_dev_pak_intercom_control_if    pak_intercom_control_if;
}egsc_parking_ctrl_if_tbl;

#endif

