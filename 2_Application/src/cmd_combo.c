/**
  ******************************************************************************
  * @file    cmd_combo.c
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "cmd_prototype.h"
#include "platform.h"

#include "ATT.c"
#include "PHA.c"
// TODO: add new cmd src file here

CMD_OBJ cmd_objs[] = {
    CMDOBJ_ATT,
    CMDOBJ_PHA,
    // TODO: add new cmds here

    CMDOBJ_UNKNOW
};

#define CMD_OBJ_NUM   (sizeof(cmd_objs) / sizeof(CMD_OBJ))
