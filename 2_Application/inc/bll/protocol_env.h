/**
  ******************************************************************************
  * @file    protocol_env.h
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _PROTOCOL_ENV_H_
#define  _PROTOCOL_ENV_H_

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "subboard_manager.h"
#include "sys_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 
SUBBD_PROTOCOL  *get_protocol_obj(uint32_t index);
BUS_DRIVER  *get_bus_obj(uint32_t index);

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
