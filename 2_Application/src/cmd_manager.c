/**
  ******************************************************************************
  * @file    rbuffer.c
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   Round buffer stuffs, used for telnet.
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "cmd_msg.h"
#include "cmd_manager.h"
#include "platform.h"
#include "app_debug.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/ipc.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

typedef struct cmd_msg{
    long        msg_type;

    char        *recv_buf;
    SEND_BUF    send_buf_fun;
    uint32_t    dest_fd;
} CMD_MSG;

/* Private macro -------------------------------------------------------------*/
#define CMD_MSG_TYPE        0x0015
/* Private variables ---------------------------------------------------------*/
int msq_id = -1;
/* Private function prototypes -----------------------------------------------*/
LOCAL void *cmd_manager(void *param);
LOCAL void check_key_and_find_cmd_entry(CMD_MSG *msg);

/* Public functions ---------------------------------------------------------*/
int32_t init_cmd_manager(void)
{
    pthread_t	tid;

	pthread_create(&tid, NULL, cmd_manager, NULL);

    return RET_OK;
}

int32_t send_cmd_msg(uint32_t dest_fd, char* recv_buf, SEND_BUF send_buf_fun)
{
    int ret = 0;
    CMD_MSG msg;

    msg.msg_type = CMD_MSG_TYPE;
    msg.dest_fd = dest_fd;
    msg.recv_buf = recv_buf;
    msg.send_buf_fun = send_buf_fun;

    ret = msgsnd(msq_id, (void *)&msg, sizeof(CMD_MSG)-sizeof(long), 0);
    if(ret<0){
        APP_DEBUGF(CMD_DEBUG | APP_DBG_LEVEL_SERIOUS, ("[send_cmd_msg] msgsnd failed (%d).\r\n",ret));
        return RET_ERROR;
    }
/*
#if (CMD_DEBUG == APP_DBG_ON)
    int flag;
    struct msqid_ds info;
    flag = msgctl( msq_id, IPC_STAT, &info ) ;

    if ( flag < 0 ) {
        perror("get message status error") ;
        return -1 ;
    }
    printf("uid:%d, gid = %d, cuid = %d, cgid= %d\n" ,info.msg_perm.uid,info.msg_perm.gid, info.msg_perm.cuid, info.msg_perm.cgid ) ;
    printf("read-write:%03o, cbytes = %lu, qnum = %lu, qbytes= %lu\n" ,info.msg_perm.mode&0777, info.msg_cbytes, info.msg_qnum, info.msg_qbytes ) ;
#endif
*/
    return RET_OK;
}


/* Private functions ---------------------------------------------------------*/

LOCAL void *cmd_manager(void *param)
{
    ssize_t size= 0;
    CMD_MSG msg;

    msq_id =  msgget( IPC_PRIVATE, 0666 ) ;
	if(msq_id == -1){
		APP_DEBUGF(CMD_DEBUG | APP_DBG_LEVEL_SERIOUS , ("[cmd_manager] msgget failed (%d).\r\n",msq_id));
		return 0;
    }
    while(1){
        size=msgrcv(msq_id, (void *)&msg,sizeof(CMD_MSG)-sizeof(long),CMD_MSG_TYPE,0);
        if(size < 0){
            APP_DEBUGF(CMD_DEBUG | APP_DBG_LEVEL_SERIOUS , ("[cmd_manager] msgrcv failed (%d).\r\n", size));
        }
        APP_DEBUGF(CMD_DEBUG | APP_DBG_TRACE , ("[cmd_manager] rx msg buf: (%s).\r\n", msg.recv_buf));

        check_key_and_find_cmd_entry(&msg);

        free(msg.recv_buf);
    }
    // should never reach here.
}

LOCAL void check_key_and_find_cmd_entry(CMD_MSG *msg)
{

}
