/**
  ******************************************************************************
  * @file    tcp_server.h
  * @author  YORK
  * @version V0.1.0
  * @date    02-28-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  TCP_PROTOTYPE_H
#define  TCP_PROTOTYPE_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "cmd_msg.h"
#include "list.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*cmdscall_func)(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun);

typedef struct cmd_obj
{
	char	*key;		/* the name of system call */
	char    *desc;		/* describe the function */ 
	cmdscall_func func;		/* the function address of system call */
}CMD_OBJ;

typedef struct cmd_parse_obj{
    char **words;
    uint32_t num;
} CMD_PARSE_OBJ;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define KEY_SIZE			32
#define DESC_SIZE			128
#define CMD_TOK       " "

#define CMD_INVALID_PARAM "Invalid parameter\r\n"

/* Exported functions --------------------------------------------------------*/ 
CMD_PARSE_OBJ *parse_cmd(char *str, char *tok);
int32_t free_cmd_obj(CMD_PARSE_OBJ *obj);
int32_t cmd_obj_get_int(CMD_PARSE_OBJ *obj, uint32_t index);
char *cmd_obj_get_str(CMD_PARSE_OBJ *obj, uint32_t index);

#endif
