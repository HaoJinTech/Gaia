/**
  ******************************************************************************
  * @file    CSUPLD.c
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

#define KEY_CSUPLD "CSUPLD"

LOCAL void cmd_CSUPLD(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
	char *case_name;
	struct Case_item *case_item;
	CMD_PARSE_OBJ *obj = NULL;
	int32_t case_exist = 0;
    CASE_STATE state;
    char casestate[32];

	obj = parse_cmd(recv_buf, CMD_TOK);
    case_name = cmd_obj_get_str(obj, 1);
    if(!case_name){
    	goto failed_end;
    }

	case_item = *get_case_item(case_name);
	if(case_item){
		state = get_case_state(case_item, casestate, 32);
		if(state != CASE_STATE_UNLOADED){
			free_cmd_obj(obj);
			send_buf_fun(dest_fd, "FAIL\r\n%s %s %s\r\n",KEY_CSUPLD, case_name, casestate);
			return;
		}
	}else{
		case_exist = check_case_exist(case_name);
		if(0 == case_exist){
			free_cmd_obj(obj);
			send_buf_fun(dest_fd, "FAIL\r\n%s case file can not find.\r\n",KEY_CSUPLD);
			return;
		}
	}
	send_buf_fun(dest_fd, "%s %s\r\n", KEY_CSUPLD, case_name);

	send_upload_misson(case_name);

end:
    send_buf_fun(dest_fd, CMD_INVALID_PARAM);
    free_cmd_obj(obj);
}

#define CMDOBJ_CSUPLD {"CSUPLD", "show case infomation.", cmd_CSUPLD}
