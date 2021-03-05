/**
  ******************************************************************************
  * @file    sys_config.h
  * @author  YORK
  * @version V0.1.0
  * @date    02-28-2021
  * @brief   
  *
	********** Copyright (C), 2014-2022,HJ technologies **************************
	*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  SYS_CONFIG_H
#define  SYS_CONFIG_H
/* Includes ------------------------------------------------------------------*/
#include "json.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 
extern json_object *pobj;
extern void config_init(void);

#endif