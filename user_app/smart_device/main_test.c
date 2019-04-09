#include<string.h>
#include<stdio.h>
#include "egsc_util.h"
#include "mySocket.h"
#include "myMQ.h"
#include "myProtocol.h"

int main_test(int argc, char * argv [ ])
{
	egsc_log_level = EGSC_LOG_DEBUG;
	egsc_log_debug("main enter\n");
    int ret=0;
	int mqKey = SOCKET_RSV_MQ_KEY;
	msg_struct msgs,msgs_out={0};
	//msg_struct *p=(msg_struct*)malloc(sizeof(msg_struct));
	msgs.msgData.devType = 9;
	msgs.msgData.offset = 3;
	msgs.msgType = (msgs.msgData.devType << 16) + msgs.msgData.offset;
	strncpy(msgs.msgData.info,"HelloMQ",MQ_INFO_BUFF);
	printf("msgType:%ld %ld %ld\n",msgs.msgType,msgs.msgType >> 16,msgs.msgType & 0xFFFF);
	ret = Enqueue_MQ(mqKey, msgs, sizeof(msg_struct)-sizeof(long),ipc_no_wait);
	msgs.msgData.offset = 5;
	ret = Enqueue_MQ(mqKey, msgs, MQ_SEND_BUFF,ipc_no_wait);
	while(1){
		ret = Dequeue_MQ(mqKey, 0, &msgs_out,sizeof(msg_struct),ipc_no_wait);
		if(ret<0)
			break;
		printf("main:devtype=[%d] offset=[%d]\ninfo=%s\n",msgs_out.msgData.devType,msgs_out.msgData.offset,msgs_out.msgData.info);
	}
	char buff[SOCKET_RCV_BUFF] = {0};
	int socket_id = -1;
	socketServerInit(SOCKET_SERVER_PORT,SOCKET_SERVER_LISNUM, buff, &socket_id);
	socketServerLoopRsv(socket_id);
	ret = Delete_MQ(mqKey);//删除消息队列
	return ret;
}

