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
    CMD_PARSE_OBJ *obj = NULL;
    uint32_t i = 1;
    obj = parse_cmd(recv_buf, CMD_TOK);
    int32_t ch = 0, val = 0;

    while(i+1 < obj->num){
      ch = cmd_obj_get_int(obj, i);
      i++;
      val = cmd_obj_get_int(obj, i);
      i++;
      set_att(ch, val);
    }
    send_buf_fun(dest_fd, "ATT\r\n", 0);

    free_cmd_obj(obj);
}

#define CMDOBJ_ATT {"ATT", "set the att value.", cmd_att}

