/**
  ******************************************************************************
  * @file    sys_config.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "sys_config.h"
#include "json.h"
#include "platform.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CONF_DEBUG                      APP_DBG_ON
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

json_object *pobj;

void config_init(void){
    pobj = json_object_from_file(SYS_CONFIG_PATH);

}

