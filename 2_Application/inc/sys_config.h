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
/* Exported value ------------------------------------------------------------*/
extern json_object *config_json_obj;

/* Exported functions --------------------------------------------------------*/ 
void config_init(void);

/*single*/
const char *config_get_string(json_object *obj_base, char *key,const char* defval);
int config_get_int(json_object *obj_base, char *key, int defval);
int config_get_bool(json_object *obj_base, char *key, int defval);

/*array*/
int config_get_array_lenth(json_object *obj_base, char *key);
json_object *config_get_array_obj(json_object *obj_base, char *key, int index);
const char *config_get_array_string(json_object *obj_base, char *key, int index,const char* defval);
int config_get_array_int(json_object *obj_base, char *key, int index, int defval);
int config_get_array_bool(json_object *obj_base, char *key, int index, int defval);

#endif