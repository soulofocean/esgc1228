#ifndef _EGSC_PARKING_LOCK_H_
#define _EGSC_PARKING_LOCK_H_

#include "../egsc_command.h"

// 设备专有参数

// 服务器请求回调函数表
typedef struct _egsc_parking_lock_cb_tbl
{
    egsc_dev_reset_cb                       reset_cb;
    egsc_dev_correction_cb                  correction_cb;
    egsc_dev_notify_update_cb               notify_update_cb;
    egsc_dev_read_parameter_cb              read_parameter_cb;
    egsc_dev_setting_parameters_cb          setting_parameters_cb;
    egsc_dev_pak_control_lock_cb            pak_control_lock_cb;
    egsc_dev_query_dev_status_cb            query_dev_status_cb;
}egsc_parking_lock_cb_tbl;

// 设备请求接口函数表
typedef struct _egsc_parking_lock_if_tbl
{
    egsc_dev_upload_dev_status_if   upload_dev_status_if;
    egsc_dev_upload_event_if        upload_event_if;
}egsc_parking_lock_if_tbl;

#endif
