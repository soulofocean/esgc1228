#ifndef _MYPROTOCOL_H
#define _MYPROTOCOL_H
#include "myMQ.h"
#include "egsc_def.h"

#define MQ_SEND_BUFF sizeof(msg_struct)-sizeof(long) //ENQueue_MQ消息的大小
#define MQ_RSV_BUFF sizeof(msg_struct)				 //DeQueue_MQ消息的大小 必须比ENQueue_MQ的大否则报错
#define MQ_SEND_BUFF_SHORT sizeof(msg_short_struct) - sizeof(long)
#define MQ_RSV_BUFF_SHORT sizeof(msg_short_struct)
#define SOCKET_RSV_MQ_KEY 8888	//接收到的SOCKET消息缓存的MQKEY ftok(".",8) 
#define SOCKET_SEND_MQ_KEY 9999 //将要通过SOCKET发送出去的MSG的缓存MQKEY ftok(".",9) 
#define SOCKET_SEND_SHORT_MQ_KEY 6666 //将要通过SOCKET发送出去的SHORT_MSG的缓存MQKEY ftok(".",6) 
#define SOCKET_RSV_MSG_TYPE 1		//Socket接收到的数据放入MQ默认类型
#define SOCKET_SEND_MSG_TYPE 2		//Socket发送出去的数据放入MQ默认类型
#define DEV_INDEX_OFFSET 16			//预留16bit放设备序号，DEVTYPE<<16
#define DEV_OFFSET_OP 0xFFFF		//和上面的16bits对应
#define DEV_FORK_LIST_MAX_SIZE 16	//最多支持16种设备
#define DEV_MAX_COUNT 1000			//每种设备最大数目
extern unsigned int global_fork_us;
typedef enum _dev_msg_ack_enum{
	NO_ACK = 0,
	SHORT_ACK = 1,
	LONG_ACK = 2
}DEV_MSG_ACK_ENUM;
typedef enum _Rsv_Msg_Process_Result{
	No_Need_Rsp = 0,
	DSP_MSG = 1,
	SEND_MSG = 2
}RsvMsgProcResultEnum;
extern DEV_MSG_ACK_ENUM global_ack_type;
extern long global_msg_type;
unsigned int GetMQMsgType(int dev_type,int dev_offset);
unsigned int GetDevType(unsigned int msg_type);
unsigned int GetDevCount(unsigned int msg_type);
int Update_Dev_Fork_List(unsigned         int arr[], int arrIndex, EGSC_DEV_TYPE devType, int devCount);
int replace_err_code(char *result,char *source, int err_code);
int replace_dev_status(char *result,char *source, int dev_status);
int replace_event_type(char *result,char *source, int event_type);
int replace_disp_floor(char* result, char* source, char* disp_floor);
int replace_car_status(char* result, char* source, char* car_status);
int replace_door_status(char* result, char* source, char* door_status);
int replace_err_msg(char* result, char* source, char* err_msg);
int replace_car_id(char *result,char *source, int car_id);
int replace_phy_floor(char *result,char *source, int phy_floor);
int replace_err_status(char *result,char *source, int err_status);
int replace_fire_ctl_status(char *result,char *source, int fire_ctl_status);
int replace_user_id(char* result, char* source, char* user_id);
int replace_light_mode(char* result, char* source, char* light_mode);
int replace_user_type(char *result,char *source, int user_type);
int replace_dest_floor(char *result,char *source, char* dest_floor);
int replace_work_mode(char* result, char* source, char* work_mode);
int replace_status(char *result,char *source, int status);
int replace_floor(char *result,char *source, int floor);
int replace_dicrection(char *result,char *source, int dicrection);
int replace_system_time(char *result,char *source);
int replace_dev_id(char *result,char *source, char *dev_id);
int replace_sub_dev_id(char *result,char *source, char *sub_dev_id);
int replace_dev_type(char *result,char *source, int dev_type);
int replace_sub_dev_type(char *result,char *source, int sub_dev_type);
int replace_record_type(char *result,char *source, int record_type);
int replace_credence_type(char *result,char *source, int credence_type);
int replace_credence_no(char *result,char *source, char *credence_no);
int replace_entry_type(char *result,char *source, int entry_type);
int replace_dev_mac(char *result,char *source, char *dev_mac);
int replace_gate_open_mode(char *result,char *source, int gate_open_mode);
int replace_img_path(char *result,char *source, char *img_path);
int replace_pass_type(char *result,char *source, int pass_type);
int ForkMulDev(unsigned int dev_arr[],msgQueenDataType *myarg);
int PutRsvMQ(msg_struct msgs);
int PutSendMQ(int code,const char* func_name,char * info);
int PutSendShortMQ(int status_code);
int PutDispatchMQ(int dev_type,int dev_index,char* info);
int PutDispatchNMQ(msg_struct msgs,int put_count);
int GetRsvMQ(msg_struct *msgbuff);
int GetSendMQ(msg_struct *msgbuff);
int GetSendShortMQ(msg_short_struct *msgbuff);
int GetDispatchMQ(long msgType,msg_struct *msgbuff);
int DelDispatchMQ(long msgType);
void DevMsgAck(int code,const char* func_name,char* msg);
#endif
