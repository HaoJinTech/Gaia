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

LOCAL void print_freq_info(uint32_t dest_fd, uint32_t ch, SEND_BUF send_buf_fun)
{
    const char *freq_name = 0;

    freq_name = cali_get_freq(ch);
    if(freq_name) send_buf_fun(dest_fd, "%d %s;", ch+1, freq_name);
    else send_buf_fun(dest_fd, "%d F;", ch+1);
}

void cmd_crcd(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    CMD_PARSE_OBJ *obj = NULL;
    uint32_t i = 0;
    obj = parse_cmd(recv_buf, CMD_TOK);
    uint32_t ch = 0;

    send_buf_fun(dest_fd, "CRCD ");
    // no paramiter so list all att values.
    if(obj->num == 1){
print_all:
      for(i=0; i<get_pha_ch_max(); i++){
        print_freq_info(dest_fd, i, send_buf_fun);
      }
    }else if(obj->num == 2){
      ch = cmd_obj_get_int(obj, 1);
      if(ch ==0){
        goto print_all;
      }else{
        goto print_onebyone;
      }
    }else{
print_onebyone:
      for(i =1; i<obj->num; i++){
        ch = cmd_obj_get_int(obj, i);
        ch --;
        if(ch >= get_pha_ch_max())
          goto failed_end;
        print_freq_info(dest_fd, ch, send_buf_fun);
      }
    }

    send_buf_fun(dest_fd, "\r\n");
    free_cmd_obj(obj);
    return;

failed_end:
    send_buf_fun(dest_fd, CMD_INVALID_PARAM);
    free_cmd_obj(obj);
}

#define CMDOBJ_CRCD {"CRCD", "list the table name for the channel calibration.", cmd_crcd}

