#include <stdio.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <string.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <sys/wait.h>  
#include <unistd.h>  
#include <arpa/inet.h>  
#include <sys/time.h>  
#include <sys/types.h>

#include "mySocket.h"
#include "egsc_util.h"
#include "myMQ.h"

static int socket_new_fd;


int socketServerInit(unsigned int myport, unsigned int lisnum, char * serveraddr,int *socketID)
{
	int sockfd; 
    struct sockaddr_in my_addr;
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)   
    {  
        egsc_log_error("socket init error");  
        return -1;
    }  
	//下2行代码解决了一个Ctrl+C中断客户端会导致下次连接端口被绑定的问题
	int mw_optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&mw_optval,sizeof(mw_optval));
    bzero(&my_addr, sizeof(my_addr));  
    my_addr.sin_family = PF_INET;  
    my_addr.sin_port = htons(myport);  
    egsc_log_info("port=%d lisnum=%d\n",myport,lisnum);
    if (strlen(serveraddr)!=0){  
		egsc_log_info("addr=%s\n",serveraddr);
        my_addr.sin_addr.s_addr = inet_addr(serveraddr);
    }
    else{  
        my_addr.sin_addr.s_addr = INADDR_ANY; 
		egsc_log_info("addr is INADDR_ANY\n");
    }
          
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1)   
    {  
        egsc_log_error("socket bind error!");  
        return -1;  
    }  
      
    if (listen(sockfd, lisnum) == -1)   
    {  
        egsc_log_error("socket listen error");  
        return -1;  
    }
	*socketID = sockfd;
	return 0;
}

int socketServerLoopRsv(int sockfd)
{
	socklen_t len;  
    struct sockaddr_in their_addr;   
    char buf[SOCKET_RCV_BUFF + 1];  
	fd_set rfds;  
    struct timeval tv;  
    int retval, maxfd = -1;  
	while (1)   
    {  
        egsc_log_info("----Waiting for connecting……\n");  
        len = sizeof(struct sockaddr);  
          
        if ((socket_new_fd =accept(sockfd, (struct sockaddr *) &their_addr,&len)) == -1)   
        {  
            egsc_log_error("accept error! errno=%d[%s]\n",errno,strerror(errno));  
            return -1;  
        }   
        else  
            egsc_log_info("server: got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port), socket_new_fd);  
              
        /* 开始处理每个新连接上的数据收发 */  
        while (1){  
            /* 把集合清空 */  
            FD_ZERO(&rfds); 
            maxfd = 0;  
              
            /* 把当前连接句柄new_fd加入到集合中 */  
            FD_SET(socket_new_fd, &rfds);  
            if (socket_new_fd > maxfd)  
                maxfd = socket_new_fd;  
                  
            /* 设置最大等待时间 */  
            tv.tv_sec = 1;  
            tv.tv_usec = 0;  
              
            /* 开始等待 */  
            retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);  
            if (retval == -1)   
            {  
                egsc_log_error("select error, will disconnect! errno=%d[%s]\n",errno,strerror(errno)); 
                break;  
            }   
            else if (retval == 0)   
            {  
                //printf("没有任何消息到来，用户也没有按键，继续等待……\n");
                continue;  
            }   
            else   
            { 
                if (FD_ISSET(socket_new_fd, &rfds))
                {  
                    /* 当前连接的socket上有消息到来则接收对方发过来的消息并显示 */  
                    bzero(buf, SOCKET_RCV_BUFF + 1);
                    /* 接收客户端的消息 */  
                    len = recv(socket_new_fd, buf, SOCKET_RCV_BUFF, 0);
                    if (len > 0){  
                        egsc_log_debug("Rsv msg success:'%s'，共%d个字节的数据\n",buf, len);
						msg_struct msgs;
						msgs.msgType = SOCKET_RSV_MSG_TYPE;
						strncpy(msgs.msgData.info,buf,MQ_INFO_BUFF);
						PutRsvMQ(msgs);
                    }
                    else
                    {  
                        if (len < 0)  
                            egsc_log_error("Rsv msg fail!errno=%d，errmsg='%s'\n",errno, strerror(errno));  
                        else  
                            egsc_log_error("client disconnected...\n");  
                        break;  
                    }  
                }  
            }  
        }  
	}
	close(socket_new_fd);
    fflush(stdout);  
    bzero(buf, SOCKET_RCV_BUFF + 1);
    close(sockfd);
	return 0;
}
int socketServerLoopSend()
{
	msg_struct msgs_out;
	socklen_t len;
	int ret;
	char buf[MQ_INFO_BUFF] = {0};
	while(1){
		ret = Dequeue_MQ(SOCKET_SEND_MQ_KEY, 0, &msgs_out,MQ_RSV_BUFF,ipc_need_wait);
		if(ret<0){
			egsc_log_error("socketServerLoopSend dequeue failed!\n");
			sleep(SOCKET_SEND_SLEEP_SEC);
			continue;
		}
		strncpy(buf,msgs_out.msgData.info,strlen(msgs_out.msgData.info)+1);
		len = send(socket_new_fd, buf, strlen(buf), 0);
		if (len > 0)
			egsc_log_info("msg:%s\tsend success，len = %d bytes！\n", buf, len);
		else{
			egsc_log_info("msg'%s'send fail！error=%d，errinfo='%s'\n",buf, errno, strerror(errno));
			sleep(SOCKET_SEND_SLEEP_SEC);
			continue;
		} 
	}
	return 0;
}
int socketServerLoopSendShort()
{
	msg_short_struct msgs_out;
	socklen_t len;
	int ret;
	char buf[MQ_INFO_BUFF] = {0};
	while(1){
		ret = Dequeue_MQ_Short(SOCKET_SEND_SHORT_MQ_KEY, 0, &msgs_out,MQ_RSV_BUFF_SHORT,ipc_need_wait);
		if(ret<0){
			egsc_log_error("socketServerLoopSendShort dequeue failed!\n");
			sleep(SOCKET_SEND_SLEEP_SEC);
			continue;
		}
		snprintf(buf,MQ_INFO_BUFF-1,"{\"code\":%d}",msgs_out.msgData.statusCode);
		len = send(socket_new_fd, buf, strlen(buf), 0);
		if (len > 0)
			egsc_log_info("msg:%s\tsend success，len = %d bytes！\n", buf, len);
		else{
			egsc_log_info("msg'%s'send fail！error=%d，errinfo='%s'\n",buf, errno, strerror(errno));
			sleep(SOCKET_SEND_SLEEP_SEC);
			continue;
		} 
	}
	return 0;
}