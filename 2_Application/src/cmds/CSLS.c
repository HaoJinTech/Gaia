/**
  ******************************************************************************
  * @file    CSLS.c
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

LOCAL void cmd_CSLS(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
    DIR* dirp;
#define CASE_FILE_NAME_LEN  128
    char case_file_name[CASE_FILE_NAME_LEN];
#define STATE_STR_LEN 32
    char state[STATE_STR_LEN];
    uint32_t filesize;
    int32_t ret;
    struct Case_item *case_item;

    dirp = search_folder_start();
    if(!dirp) {
        send_buf_fun(dest_fd, "CSLS can not open dir(%s)",  get_case_full_path());
    }
    send_buf_fun(dest_fd, "CSLS\r\nDirectory :%s\r\n", get_case_full_path());
    do{
        ret = search_folder_get_name(dirp, case_file_name, CASE_FILE_NAME_LEN, &filesize);
        if(ret != RET_OK) break;
        case_item = get_case_item(case_file_name);
        get_case_state(case_item, state, STATE_STR_LEN);
        send_buf_fun(dest_fd, "%s %s\t%d\r\n", state, case_file_name, filesize);
    }while(ret == RET_OK);
    search_folder_end(dirp);
}

#define CMDOBJ_CSLS {"CSLS", "list all cases in the case folder.", cmd_CSLS}
