#ifndef _EGSC_ELEVATOR_H_
#define _EGSC_ELEVATOR_H_

#include "../egsc_command.h"

// 门禁控制器 EGSC_TYPE_DOOR_CTRL -------------------------------------------------
typedef struct _egsc_elevator_setting_parameters_param
{
	char liftcar_number[128];   //必选，轿厢数量
	char work_mode[128];        //读取必选，工作模式：0:普通模式；1:授权模式；默认：0
	char light_mode[128];       //读取必选，点亮模式，当wordMode为1时，此参数才有效；0:手动点亮，凭证校验通过后只授权楼层不点亮；1:自动点亮，凭证校验通过后自动点亮对应楼层
	char interval_time[128];    //可选，电梯轿厢状态上报的间隔时间，单位为毫秒，以便根据监控大屛接收数据的性能调整电梯厂商控制上报轿厢状态的频率
}egsc_elevator_setting_parameters_param;

typedef struct _egsc_elevator_certificate_special_param
{
    int elevator_auth_floor_num;    // 可选
    int *elevator_auth_floor;       // 可选 梯控权限楼层；2个字节表示楼层；可多个楼层。
                                    // bit15-13	            bit12-10	        Bit9	    Bit8-bit0
                                    // 000:前门(默认)           000:保留(默认000)       0：地上        楼层1-511层
                                    // 001：后门其它：保留
                                    // 111                  1FFF：即全FFFF代表所有楼层
}egsc_elevator_certificate_special_param;

typedef struct _egsc_elevator_upload_record_param
{
	int pass_type;          //必选，进出类型，0为入；1为出；失败默认为0
	char *credence_no;      //可选，用来让平台区分开门次数
}egsc_elevator_upload_record_param;

// 设备专有参数

// 服务器请求回调函数表
typedef struct _egsc_elevator_cb_tbl
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
    egsc_dev_fac_visit_control_cb           fac_visit_control_cb;
    egsc_dev_fac_key_control_cb             fac_key_control_cb;
    egsc_dev_fac_calling_cb                 fac_calling_cb;
    egsc_dev_fac_inter_call_auth_cb         fac_inter_call_auth_cb;
    egsc_dev_fac_inter_call_lighting_cb     fac_inter_call_lighting_cb;
    egsc_dev_fac_lift_lighting_cb           fac_lift_lighting_cb;
    egsc_dev_fac_delayed_closing_cb         fac_delayed_closing_cb;
    egsc_dev_fac_status_req_cb              fac_status_req_cb;
    egsc_dev_fac_lift_ba_status_cb          fac_lift_ba_status_cb;
    egsc_dev_query_dev_status_cb            query_dev_status_cb;
}egsc_elevator_cb_tbl;

// 设备请求接口函数表
typedef struct _egsc_elevator_if_tbl
{
    egsc_dev_upload_dev_status_if           upload_dev_status_if;
    egsc_dev_upload_record_if               upload_record_if;
    egsc_dev_upload_fac_dev_status_if       upload_fac_dev_status_if;
    egsc_dev_upload_fac_elevator_record_if  upload_fac_elevator_record_if;
    egsc_dev_upload_fac_ba_status_if        upload_fac_ba_status_if;
    egsc_dev_upload_event_if                upload_event_if;
    egsc_dev_upload_credence_load_result_if      upload_credence_load_result_if;
}egsc_elevator_if_tbl;

#endif
