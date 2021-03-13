/**
  ******************************************************************************
  * @file    CSRUN.c
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

#define KEY_CSRUN "CSRUN"
#define HELP_CSRUN "run case. CSRUN <case name> <intervals> <repeat times> <start line>"

LOCAL void cmd_CSRUN(char* recv_buf, uint32_t dest_fd, SEND_BUF send_buf_fun)
{
	char *case_name;
	struct Case_item *case_item;
	CMD_PARSE_OBJ *obj = NULL;
	int32_t case_exist = 0;
    CASE_STATE state;
    char casestate[32];
    int32_t ret = 0;
	int32_t interval = 0, times = 0, start_line = 0;

	obj = parse_cmd(recv_buf, CMD_TOK);
    case_name = cmd_obj_get_str(obj, 1);
    if(!case_name){
	    send_buf_fun(dest_fd, CMD_INVALID_PARAM);
    	goto end;
    }else if(case_name[0] == '?'){
		send_buf_fun(dest_fd, HELP_CSRUN);
		send_buf_fun(dest_fd, "\r\n");
		goto end;
	}

	case_item = get_case_item(case_name);
	state = get_case_state(case_item, casestate, 32);
	if(case_item){
		if(state != CASE_STATE_STOP){
			send_buf_fun(dest_fd, "FAIL\r\n%s %s %s\r\n", KEY_CSRUN, case_name, casestate);
			goto end;
		}
		
		interval = cmd_obj_get_int(obj,2);
		set_case_interval(case_item, interval);
		times = cmd_obj_get_int(obj,3);
		set_case_times(case_item, times);
		start_line = cmd_obj_get_int(obj,4);
		set_case_current_line(case_item, start_line);
	
		ret = send_run_misson(case_item);
		if(ret!= RET_OK){
			send_buf_fun(dest_fd, "FAIL\r\ncode:%d", KEY_CSRUN, ret);
			goto end;
		}
		send_buf_fun(dest_fd, "%s %s run\r\n", KEY_CSRUN, case_name);
	}else{
		case_exist = check_case_exist(case_name);
		if(0 == case_exist){
			send_buf_fun(dest_fd, "FAIL\r\n%s case file can not find.\r\n", KEY_CSRUN);
			goto end;
		}
		send_buf_fun(dest_fd, "FAIL\r\n%s %s %s\r\n", KEY_CSRUN, case_name, casestate);
	}

end:
    free_cmd_obj(obj);
}

#define CMDOBJ_CSRUN {KEY_CSRUN, HELP_CSRUN, cmd_CSRUN}

