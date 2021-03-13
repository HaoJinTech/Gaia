/**
  ******************************************************************************
  * @file    CLCD.c
  * @author  
  * @version V0.1.0
  * @date    02-03-2021
  * @brief   Basic att cmd
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
#include "platform.h"
#include "cmd_prototype.h"
#include "bll_calibration.h"

LOCAL void cmd_clcd(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    CMD_PARSE_OBJ *obj = NULL;
    uint32_t i = 0;
    const char *freq_name = 0;
    uint32_t cali_info_num = 0;

    obj = parse_cmd(recv_buf, CMD_TOK);
    send_buf_fun(dest_fd, "CLCD ");

    cali_info_num = get_cali_info_num();
    for(i=0; i<cali_info_num; i++){
      freq_name = get_cali_info_name_by_index(i);
      send_buf_fun(dest_fd, "%s;", freq_name);
    }

    send_buf_fun(dest_fd, "\r\n");
    free_cmd_obj(obj);
    return;
}

#define CMDOBJ_CLCD {"CLCD", "list the table name for the channel calibration.", cmd_clcd}

