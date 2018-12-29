#ifndef _EGSC_SMART_CTRL_H_
#define _EGSC_SMART_CTRL_H_

#include "../egsc_command.h"

// 设备专有参数
typedef struct _egsc_smart_ctrl_upload_event_param
{
	char alarmzone_name[128];   //可选，防区名称
	char alarmzone_chan[128];   //必选，开门时长
	char sensor_type[128];      //必选，用于周界报警设备上报告警信息，传感器类型：
}egsc_smart_ctrl_upload_event_param;

// 服务器请求回调函数表
typedef struct _egsc_smart_ctrl_cb_tbl
{
    egsc_dev_reset_cb                       reset_cb;
    egsc_dev_correction_cb                  correction_cb;
    egsc_dev_notify_update_cb               notify_update_cb;
    egsc_dev_read_parameter_cb              read_parameter_cb;
    egsc_dev_setting_parameters_cb          setting_parameters_cb;
    egsc_dev_boun_chan_setupalarm_cb        boun_chan_setupalarm_cb;
    egsc_dev_boun_subchan_clearalarm_cb     boun_subchan_clearalarm_cb;
    egsc_dev_query_dev_status_cb            query_dev_status_cb;
}egsc_smart_ctrl_cb_tbl;

// 设备请求接口函数表
typedef struct _egsc_smart_ctrl_if_tbl
{
    egsc_dev_upload_dev_status_if   upload_dev_status_if;
    egsc_dev_upload_event_if        upload_event_if;
}egsc_smart_ctrl_if_tbl;

#endif
