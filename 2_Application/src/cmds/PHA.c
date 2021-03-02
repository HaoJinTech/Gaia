/**
  ******************************************************************************
  * @file    PHA.c
  * @author  
  * @version V0.1.0
  * @date    02-03-2021
  * @brief   Basic att cmd
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/

#include "cmd_prototype.h"

void cmd_pha(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    // todo: parse the buf and send data to rfboard and reply to user.
    send_buf_fun(dest_fd, "PHA\r\n", 0);
}


#define CMDOBJ_PHA {"PHA", "describe the function", cmd_pha}

