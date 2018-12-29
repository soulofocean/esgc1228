#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <errno.h>

#include "mydev.h"

void dev_status_cb(int handle, EGSC_DEV_STATUS_CODE status, char *desc_info)
{
    egsc_log_debug("enter\n");
    egsc_log_debug("handle(%d) status(%d)\n", handle, status);
    return;
}

EGSC_RET_CODE user_req_cb(int handle, char *command, char *param, char *result_buf, int buf_len)
{
    egsc_log_debug("enter\n");
    egsc_log_debug("handle(%d) command(%s) param(%s).\n", handle, command, param);
    snprintf(result_buf, buf_len, "%s", param);

    return EGSC_RET_SUCCESS;
}

void server_response_callback(int handle, int req_id, int result)
{
    egsc_log_debug("enter\n");
    egsc_log_debug("handle(%d) req_id(%d) result(%d).\n", handle, req_id, result);
    return;
}

int main(int argc, char *argv[])
{
    egsc_log_debug("main enter\n");

    EGSC_RET_CODE ret = EGSC_RET_ERROR;

    ret = egsc_sdk_init();
    if(ret != EGSC_RET_SUCCESS)
    {
        egsc_log_user("egsc_sdk_init failed\n");
        return -1;
    }

    ret = mydev_init();
    if(ret != EGSC_RET_SUCCESS)
    {
        egsc_log_user("dev init failed\n");
        return -1;
    }

    ret = mydev_create();
    if(ret != EGSC_RET_SUCCESS)
    {
        egsc_log_user("dev create failed\n");
        return -1;
    }

    ret = mydev_start();
    if(ret != EGSC_RET_SUCCESS)
    {
        egsc_log_user("dev start failed\n");
        return -1;
    }

    while(1)
    {
        egsc_platform_sleep(1000);
    }

    mydev_stop();
    mydev_delete();
    egsc_sdk_uninit();

    return 0;
}

