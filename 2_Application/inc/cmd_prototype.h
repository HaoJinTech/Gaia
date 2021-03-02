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
#define KEY_SIZE			32
#define DESC_SIZE			128

typedef void (*cmdscall_func)(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun);

typedef struct cmd_obj
{
	char	*key;		/* the name of system call */
	char    *desc;		/* describe the function */ 
	cmdscall_func func;		/* the function address of system call */
}CMD_OBJ;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 

#endif
