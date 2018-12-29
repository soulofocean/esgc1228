#ifndef _EGSC_GATE_CTRL_H_
#define _EGSC_GATE_CTRL_H_

#include "../egsc_command.h"

// 人行通道控制器 EGSC_TYPE_GATE_CTRL -------------------------------------------------
typedef struct _egsc_gate_ctrl_upload_record_param
{
    int pass_type;          //必选，进出类型，0为入；1为出；失败默认为0
    char *credence_no;      //可选，用来让平台区分开门次数
}egsc_gate_ctrl_upload_record_param;

// 设备专有参数

// 服务器请求回调函数表
typedef struct _egsc_gate_ctrl_cb_tbl
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
}egsc_gate_ctrl_cb_tbl;

// 设备请求接口函数表
typedef struct _egsc_gate_ctrl_if_tbl
{
    egsc_dev_upload_dev_status_if   upload_dev_status_if;
    egsc_dev_upload_record_if       upload_record_if;
    egsc_dev_upload_event_if        upload_event_if;
    egsc_dev_upload_credence_load_result_if      upload_credence_load_result_if;
}egsc_gate_ctrl_if_tbl;

#endif
