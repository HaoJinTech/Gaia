/**
  ******************************************************************************
  * @file    cmd_manager.h
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _CMD_MANAGER_H_
#define  _CMD_MANAGER_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "cmd_prototype.h"
/* Exported types ------------------------------------------------------------*/
extern CMD_OBJ cmd_objs[];
uint32_t CMD_OBJ_NUM;

int32_t init_cmd_manager(void);

#endif

/********************** (C) COPYRIGHT HJ technologies *************************/
