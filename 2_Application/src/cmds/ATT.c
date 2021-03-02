/**
  ******************************************************************************
  * @file    ATT.c
  * @author  
  * @version V0.1.0
  * @date    02-03-2021
  * @brief   Basic att cmd
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/

#include "cmd_prototype.h"
#include "bll_att.h"

void cmd_att(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    // TODO: parse the buf and send data to rfboard and reply to user.
    send_buf_fun(dest_fd, "ATT\r\n", 0);

    set_att(1,1);
}

#define CMDOBJ_ATT {"ATT", "describe the function", cmd_att}

