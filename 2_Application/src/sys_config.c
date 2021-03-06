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
#include "app_debug.h"

#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CONF_DEBUG        APP_DBG_ON
#define SYS_CONFIG_NAME   "sysconf.json"

#define SYS_CONF_PATH_LEN 128
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
json_object *config_json_obj = 0;

/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

void config_init(void)
{
  char full_path[SYS_CONF_PATH_LEN];

  strncpy(full_path, PRJ_FILE_PATH, SYS_CONF_PATH_LEN );
  strncat(full_path, SYS_CONFIG_NAME, SYS_CONF_PATH_LEN - strlen(PRJ_FILE_PATH));

  config_json_obj = json_object_from_file(full_path);
  if(!config_json_obj){
    APP_DEBUGF(CONF_DEBUG | APP_DBG_LEVEL_WARNING, ("json error: %s", json_util_get_last_err()));
  }
/*
  json_object *testobj = json_object_new_object();
  json_object *obj = json_object_new_object();
  json_object_object_add(testobj, "ATT", obj);
  json_object_object_add(obj, "ATT_MAX_CH", json_object_new_int(2048));
  json_object_to_file("./new.json", testobj);
  */
}

const char* config_get_string(json_object *obj_base, char *key, const char *defval)
{
  const char *val;
  json_object *obj = json_object_object_get(obj_base, key);

  if(!obj){
    val = defval;
  }else{
    val = json_object_get_string(obj);
    if(!val)
      val = defval;
  }
  APP_DEBUGF(CONF_DEBUG | APP_DBG_TRACE , ("%s : %s \r\n", key, val));
  return val;
}

int config_get_int(json_object *obj_base, char *key, int defval)
{
  int val;
  json_object *obj = json_object_object_get(obj_base, key);

  if(!obj){
    val = defval;
  }
  val = json_object_get_int(obj);
  APP_DEBUGF(CONF_DEBUG | APP_DBG_TRACE , ("%s : %d \r\n", key, val));
  return val;
}

int config_get_bool(json_object *obj_base, char *key, int defval)
{
  int val;
  json_object *obj = json_object_object_get(obj_base, key);

  if(!obj){
    val = defval;
  }
  val = json_object_get_boolean(obj);
  APP_DEBUGF(CONF_DEBUG | APP_DBG_TRACE , ("%s : %d \r\n", key, val));
  return val;
}

int config_get_array_lenth(json_object *obj_base, char *key)
{
  int len;
  json_object *array_obj = json_object_object_get(obj_base, key);
  json_type type = json_object_get_type(array_obj);
  if(type != json_type_array)
    len = 0;

  len = json_object_array_length(array_obj);
  APP_DEBUGF(CONF_DEBUG | APP_DBG_TRACE , ("array %s[%d] \r\n", key, len));
  return len;
}

json_object *config_get_array_obj(json_object *obj_base, char *key, int index)
{
  json_object *array_obj = json_object_object_get(obj_base, key);
  json_type type = json_object_get_type(array_obj);
  if(type != json_type_array)
    return 0;

  return json_object_array_get_idx(array_obj, index);
}

#if 0
const char *config_get_array_string(json_object *obj_base, char *arr_key, int index, const char *defval)
{
  const char *val;
  json_object *obj = config_get_array_obj(obj_base, arr_key, index);

  if(!obj) val = defval;
  val = json_object_get_string(obj);

  APP_DEBUGF(CONF_DEBUG | APP_DBG_TRACE , ("%s[%d] : %s \r\n", key, index, val));
  return val;
}

int config_get_array_int(json_object *obj_base, char *key, int index, int defval)
{
  int val;
  json_object *obj = config_get_array_obj(obj_base, key, index);

  if(!obj) val = defval;
  val = json_object_get_int(obj);

  APP_DEBUGF(CONF_DEBUG | APP_DBG_TRACE , ("%s[%d] : %d \r\n", key, index, val));
  return val;
}

int config_get_array_bool(json_object *obj_base, char *key, int index, int defval)
{
  int val;
  json_object *obj = config_get_array_obj(obj_base, key, index);

  if(!obj) val = defval;
  val = json_object_get_boolean(obj);

  APP_DEBUGF(CONF_DEBUG | APP_DBG_TRACE , ("%s[%d] : %d \r\n", key, index, val));
  return val;
}
#endif