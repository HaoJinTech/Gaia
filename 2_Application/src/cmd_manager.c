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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/ipc.h>

/* Private typedef -----------------------------------------------------------*/
typedef struct cmd_msg{
    long        msg_type;

    char        *recv_buf;
    SEND_BUF    send_buf_fun;
    uint32_t    dest_fd;
} CMD_MSG;

/* Private define ------------------------------------------------------------*/
#define CMD_DEBUG                       APP_DBG_ON

/* Private macro -------------------------------------------------------------*/
#define CMD_MSG_TYPE        0x0015
/* Private variables ---------------------------------------------------------*/
LOCAL int msq_id = -1;
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

int32_t cmd_obj_get_int(CMD_PARSE_OBJ *obj, uint32_t index)
{
    if(index >= obj->num)
        return 0;
    return atoi(obj->words[index]);
}

CMD_PARSE_OBJ *parse_cmd(char *str, char *tok)
{
    CMD_PARSE_OBJ *obj = NULL;
    char *inner_ptr=NULL;
    char *str_iter = str;
    int i=0, word_num = 0;

    if(!str || !tok)
        return NULL;
    
    // check how many tok it has
    while(str_iter[i]){
        if(tok[0] == str_iter[i]){
            word_num++;
        }
        i++;
    }

    // remove the \r\n token
    if(str_iter[i-1] == '\r' || str_iter[i-1] == '\n'){
        str_iter[i-1] = 0;
    }
    if(str_iter[i-2] == '\r' || str_iter[i-2] == '\n'){
        str_iter[i-2] = 0;
    }

    word_num++; // last word
    APP_DEBUGF(CMD_DEBUG | APP_DBG_TRACE , ("word num (%d).\r\n",word_num));
    
    obj = (CMD_PARSE_OBJ*)malloc(sizeof(CMD_PARSE_OBJ));
    obj->words = (char**)malloc(sizeof(char*) * word_num);
    obj->num = word_num;

    i =0;
    obj->words[i] = strtok_r((char *)str_iter, tok, &inner_ptr);
    do{
        i++; 
        obj->words[i] = strtok_r(NULL, tok, &inner_ptr);
        APP_DEBUGF(CMD_DEBUG | APP_DBG_TRACE , 
            ("parse word [%d](%s).\r\n",i,obj->words[i]));
    }while(i < word_num-1);
    
    return obj;
}

int32_t free_cmd_obj(CMD_PARSE_OBJ *obj)
{
    if(obj){
        if(obj->words){
            free(obj->words);
        }
        free(obj);
    }
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
        APP_DEBUGF(CMD_DEBUG | APP_DBG_LEVEL_SERIOUS, (" msgsnd failed (%d).\r\n",ret));
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
		APP_DEBUGF(CMD_DEBUG | APP_DBG_LEVEL_SERIOUS , ("msgget failed (%d).\r\n",msq_id));
		return 0;
    }
    APP_DEBUGF(CMD_DEBUG | APP_DBG_TRACE , ("start msgrcv ...\r\n"));
    while(1){
        size=msgrcv(msq_id, (void *)&msg,sizeof(CMD_MSG)-sizeof(long),CMD_MSG_TYPE,0);
        if(size < 0){
            APP_DEBUGF(CMD_DEBUG | APP_DBG_LEVEL_SERIOUS , ("msgrcv failed (%d).\r\n", size));
        }
        APP_DEBUGF(CMD_DEBUG | APP_DBG_TRACE , ("rx msg buf: \r\n%s", msg.recv_buf));

        check_key_and_find_cmd_entry(&msg);

        free(msg.recv_buf);
    }
    // should never reach here.
}

LOCAL int32_t check_key(char *line, char *key, uint32_t key_len)
{
    int i =0;
    for(i=0; i<key_len; i++){
        if(line[i] != key[i])
            return 0;
    }
    if(line[i] == ' ' || line[i] == '\r' ||  line[i] == '\n')
        return 1;
    return 0;
}

LOCAL void check_key_and_find_cmd_entry(CMD_MSG *msg)
{
    uint32_t i=0;
    for(i=0; i<CMD_OBJ_NUM-1; i++){
        if(check_key(msg->recv_buf, cmd_objs[i].key, sizeof(cmd_objs[i].key)-1)){
            break;
        }
    }
    cmd_objs[i].func(msg->recv_buf, msg->dest_fd, msg->send_buf_fun);
}


