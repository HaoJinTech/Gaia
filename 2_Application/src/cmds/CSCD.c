/**
  ******************************************************************************
  * @file    CSCD.c
  * @author  
  * @version V0.1.0
  * @date    02-03-2021
  * @brief   Basic att cmd
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/

#include "cmd_prototype.h"
#include "bll_calibration.h"

void cmd_cscd(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    CMD_PARSE_OBJ *obj = NULL;
    uint32_t i = 1;
    obj = parse_cmd(recv_buf, CMD_TOK);

    // no paramiter so list all att values.
    if(obj->num <3){
      goto failed_end;
    }
    send_buf_fun(dest_fd, "CSCD ");
    int32_t ch = 0;
    const char *val = 0;
    while(i < obj->num){
      ch = cmd_obj_get_int(obj, i);
      if(ch >0) ch--;
      else {i++;continue;} 
      i++;
      if(i >= obj->num) break;
      val = cmd_obj_get_str(obj, i);
      i++;
      if(!val){
        goto failed_end;
      }
      cali_set_freq(ch, val);
    }
    
    send_buf_fun(dest_fd, "done.\r\n");
    free_cmd_obj(obj);
    return;

failed_end:
    send_buf_fun(dest_fd, CMD_INVALID_PARAM);
    free_cmd_obj(obj);
}

#define CMDOBJ_CSCD {"CSCD", "set the table name for the channel calibration.", cmd_cscd}

