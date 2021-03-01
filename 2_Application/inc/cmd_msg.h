/**
  ******************************************************************************
  * @file    cmd_msg.h
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _CMD_MSG_H_
#define  _CMD_MSG_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Exported types ------------------------------------------------------------*/
typedef  int32_t (*SEND_BUF) (uint32_t dest_fd, char *buf, uint32_t len);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/ 
int32_t send_cmd_msg(uint32_t dest_fd, char* recv_buf, SEND_BUF send_buf_fun);
#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
