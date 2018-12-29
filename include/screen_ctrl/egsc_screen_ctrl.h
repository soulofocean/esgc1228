#ifndef _EGSC_SCREEN_CTRL_H_
#define _EGSC_SCREEN_CTRL_H_

#include "../egsc_command.h"

// 信息发布屏控制器 EGSC_TYPE_SCREEN_CTRL -------------------------------------------------
// 设备专有参数

// 服务器请求回调函数表
typedef struct _egsc_screen_ctrl_cb_tbl
{
    egsc_dev_reset_cb                       reset_cb;
    egsc_dev_correction_cb                  correction_cb;
    egsc_dev_notify_update_cb               notify_update_cb;
    egsc_dev_read_parameter_cb              read_parameter_cb;
    egsc_dev_setting_parameters_cb          setting_parameters_cb;
    egsc_dev_query_dev_status_cb            query_dev_status_cb;
    egsc_dev_trans_material_cb              trans_material_cb;
    egsc_dev_ads_query_term_cb              query_term_cb;
    egsc_dev_ads_add_program_cb             add_program_cb;
    egsc_dev_ads_delete_program_cb          delete_program_cb;
    egsc_dev_ads_set_program_cb             set_program_cb;
    egsc_dev_ads_add_schedule_cb            add_schedule_cb;
    egsc_dev_ads_delete_schedule_cb         delete_schedule_cb;
    egsc_dev_ads_set_schedule_cb            set_schedule_cb;
    egsc_dev_ads_publish_schedule_cb        publish_schedule_cb;
}egsc_screen_ctrl_cb_tbl;

// 设备请求接口函数表
typedef struct _egsc_screen_ctrl_if_tbl
{
    egsc_dev_upload_dev_status_if   upload_dev_status_if;
    egsc_dev_upload_event_if        upload_event_if;
}egsc_screen_ctrl_if_tbl;

#endif


