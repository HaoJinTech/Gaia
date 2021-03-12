/**
  ******************************************************************************
  * @file    CSINFO.c
  * @author  
  * @version V0.1.0
  * @date    02-03-2021
  * @brief   list all the cases in folder
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
#include "platform.h"
#include "cmd_prototype.h"
#include "bll_case_manager.h"

#include <fcntl.h>
#include <unistd.h>

#define KEY_CSINFO "CSINFO"

LOCAL void cmd_CSINFO_print_stuff(char *case_name, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    uint32_t times = 0, line_max = 0;
    CASE_STATE state;
    struct Case_item *case_item = NULL;

    case_item = get_case_item(case_name);
    times = get_case_times(case_item);
    line_max = get_case_line_max(case_item);
    state = get_case_state(case_item, NULL, 0);
    switch(state){
      case CASE_STATE_BUSY:
        send_buf_fun(dest_fd, "%s %s %d %d %d\r\n", KEY_CSINFO, case_name, state, times, line_max);
        break;
      case CASE_STATE_RUN:
      case CASE_STATE_STOP:
      case CASE_STATE_PAUSE:
        send_buf_fun(dest_fd, "%s %s %d %d %d\r\n", KEY_CSINFO, case_name, state, times, line_max);
        break;
      case CASE_STATE_UNLOADED:
        send_buf_fun(dest_fd, "%s %s %d %d %d\r\n", KEY_CSINFO, case_name, state, 0, 0);
        break;
    }
}

LOCAL void cmd_CSINFO(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    CMD_PARSE_OBJ *obj = NULL;
#define CASE_FILE_NAME_LEN  128
    char case_name[CASE_FILE_NAME_LEN];
  	DIR* dirp;
    int32_t ret;

    obj = parse_cmd(recv_buf, CMD_TOK);
    case_name = cmd_obj_get_str(obj, 1);
    if(case_name){
      cmd_CSINFO_print_stuff(case_name, dest_fd, send_buf_fun);
    }else{
      dirp = search_folder_start();
      if(!dirp) {
          send_buf_fun(dest_fd, "%s can not open dir(%s)", KEY_CSINFO, get_case_full_path());
      }
      do{
          ret = search_folder_get_name(dirp, case_name, CASE_FILE_NAME_LEN, NULL);
          if(ret != RET_OK) break;
          cmd_CSINFO_print_stuff(case_name, dest_fd, send_buf_fun);

      }while(ret == RET_OK);
      search_folder_end(dirp);
    }
    free_cmd_obj(obj);
}

#define CMDOBJ_CSINFO {"CSINFO", "show cases infomation.", cmd_CSINFO}
