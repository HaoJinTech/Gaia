/**
  ******************************************************************************
  * @file    UNKNOW.c
  * @author  
  * @version V0.1.0
  * @date    02-03-2021
  * @brief   Basic att cmd
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/

#include "cmd_prototype.h"

#define UNKNOW_CMD_REPLY	 "command not support.\r\n"

void cmd_unknow(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    // todo: parse the buf and send data to rfboard and reply to user.
    send_buf_fun(dest_fd, UNKNOW_CMD_REPLY, 0);
}


#define CMDOBJ_UNKNOW {"NULL", "NULL", cmd_unknow}

