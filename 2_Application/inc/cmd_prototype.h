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
#ifndef  TCP_SERVER_H
#define  TCP_SERVER_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "cmd_msg.h"
#include "list.h"

/* Exported types ------------------------------------------------------------*/
#define KEY_SIZE			32
#define DESC_SIZE			128

typedef void (*cmdscall_func)(char* recv_buf, SEND_BUF send_buf_fun);

struct Cmd_reg_item
{
	char	key[KEY_SIZE];		/* the name of system call */
	char    desc[DESC_SIZE];		/* describe the function */ 
	cmdscall_func func;		/* the function address of system call */
	
    struct list_head list;
};
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 

#endif
