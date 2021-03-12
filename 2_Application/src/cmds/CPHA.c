/**
  ******************************************************************************
  * @file    CPHA.c
  * @author  
  * @version V0.1.0
  * @date    02-03-2021
  * @brief   Basic att cmd
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
#include "platform.h"
#include "cmd_prototype.h"
#include "bll_pha.h"

LOCAL void cmd_cpha(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    CMD_PARSE_OBJ *obj = NULL;
    uint32_t i = 1;
    obj = parse_cmd(recv_buf, CMD_TOK);

    // no paramiter so list all att values.
    send_buf_fun(dest_fd, "CPHA ");
    if(obj->num == 1){
      for(i=0; i<get_att_ch_max(); i++){
        send_buf_fun(dest_fd, "%d %d;", i+1, get_pha(i));
      }
    // more than 3 word so set att.
    }else if(obj->num >=3){
      int32_t ch = 0, val = 0;
      while(i < obj->num){
        ch = cmd_obj_get_int(obj, i);
        if(ch >0) {
          ch--;
        }else {
          goto failed_end;
        } 
        i++;
        if(i >= obj->num) break;
        val = cmd_obj_get_int(obj, i);
        i++;
        set_pha(ch, val);
        send_buf_fun(dest_fd, "%d %d;", ch+1, get_pha(ch));
      }
    }else{
      goto failed_end;      
    }

    send_buf_fun(dest_fd, "\r\n");
    free_cmd_obj(obj);
    return;

failed_end:
    send_buf_fun(dest_fd, CMD_INVALID_PARAM);
    free_cmd_obj(obj);
}

#define CMDOBJ_CPHA {"CPHA", "set phase shifter using calibration table.", cmd_cpha}
