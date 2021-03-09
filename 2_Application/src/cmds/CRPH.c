/**
  ******************************************************************************
  * @file    CRCD.c
  * @author  
  * @version V0.1.0
  * @date    02-03-2021
  * @brief   Basic att cmd
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/

#include "cmd_prototype.h"
#include "bll_calibration.h"
#include "bll_pha.h"

void cmd_crph(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    CMD_PARSE_OBJ *obj = NULL;
    uint32_t i = 0;
    obj = parse_cmd(recv_buf, CMD_TOK);
    char *freq_name = 0;
    uint32_t ch = 0;

    send_buf_fun(dest_fd, "CRPH ");
    // no paramiter so list all att values.
    if(obj->num == 1){
      for(i=0; i<get_pha_ch_max(); i++){
        send_buf_fun(dest_fd, "%d %d;", i+1, get_pha(i));
      }
    }else if(obj->num == 2){
      ch = cmd_obj_get_int(obj, 1);
      if(ch ==0){
        for(i=0; i<get_pha_ch_max(); i++){
          send_buf_fun(dest_fd, "%d %d;", i+1, get_pha(i));
        }
      }else{
        goto failed_end;
      }
    }else{
      for(i =1; i<obj->num -1; i++){
        ch = cmd_obj_get_int(obj, i);
        if(ch >= get_pha_ch_max())
          goto failed_end;
        send_buf_fun(dest_fd, "%d %d;", ch+1, get_pha(ch));
      }
    }

    send_buf_fun(dest_fd, "\r\n");
    free_cmd_obj(obj);
    return;

failed_end:
    send_buf_fun(dest_fd, CMD_INVALID_PARAM);
    free_cmd_obj(obj);
}

#define CMDOBJ_CRPH {"CRPH", "read phase shifter using calibration table.", cmd_crph}

