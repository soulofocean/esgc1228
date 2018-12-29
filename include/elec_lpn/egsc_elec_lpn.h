#ifndef _EGSC_ELEC_LPN_H_
#define _EGSC_ELEC_LPN_H_

#include "../egsc_command.h"

// 设备专有参数

// 服务器请求回调函数表
typedef struct _egsc_elec_lpn_cb_tbl
{
    egsc_dev_reset_cb                       reset_cb;
    egsc_dev_correction_cb                  correction_cb;
    egsc_dev_notify_update_cb               notify_update_cb;
    egsc_dev_read_parameter_cb              read_parameter_cb;
    egsc_dev_setting_parameters_cb          setting_parameters_cb;
    egsc_dev_pak_send_showinfo_cb           pak_send_showinfo_cb;
    egsc_dev_query_dev_status_cb            query_dev_status_cb;
}egsc_elec_lpn_cb_tbl;

// 设备请求接口函数表
typedef struct _egsc_elec_lpn_if_tbl
{
    egsc_dev_upload_dev_status_if   upload_dev_status_if;
    egsc_dev_upload_event_if        upload_event_if;
}egsc_elec_lpn_if_tbl;

#endif
