/**
  ******************************************************************************
  * @file    bll_ch_remap.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_config.h"
#include "bll_ch_remap.h"
#include "app_debug.h"
#include "cmd_prototype.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define REMAP_DEBUG        APP_DBG_ON

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL const char *remap_path;
LOCAL uint32_t remap_obj_num = 0;
MAPPING_MODEL *mapping_objs = 0;

/* Private function prototypes -----------------------------------------------*/
int32_t remap_read_file( MAPPING_MODEL *map_model)
{
  char full_path[FILE_PATH_LEN];
	int read_len;
  char *buf;
  long FileLength = 0;
  const char *filename = map_model->name;
  FILE *stream;

	APP_ASSERT("filename == NULL.\r\n", filename);

	snprintf(full_path, FILE_PATH_LEN, "%s/%s%s", PRJ_FILE_PATH, remap_path, filename);
	APP_DEBUGF(REMAP_DEBUG | APP_DBG_TRACE, ("open file: %s\r\n", full_path));
  stream = fopen(full_path, "r");
	if(stream == NULL){
		APP_DEBUGF(REMAP_DEBUG | APP_DBG_LEVEL_WARNING ,
		    ("open file failed.\r\n"));
		return RET_ERROR;
	}

  fseek(stream, 0, SEEK_END);
  FileLength = ftell(stream);
  APP_DEBUGF(REMAP_DEBUG | APP_DBG_TRACE, ("input file length: %ld\n", FileLength));
  fseek(stream, 0, SEEK_SET);

	buf = (char*)malloc(sizeof(char)* (FileLength+1));
	if(!buf){
		APP_DEBUGF(REMAP_DEBUG | APP_DBG_LEVEL_SERIOUS, 
      ("buf malloc failed.\r\n"));
    goto end;
	}
  memset(buf, 0, FileLength+1);

  read_len = fread(buf, sizeof(char), FileLength, stream);
  if(read_len != FileLength) {
    APP_DEBUGF(REMAP_DEBUG | APP_DBG_LEVEL_WARNING, 
      ("file size error(%ld,%d).\r\n", FileLength, read_len));
  }

  // parse values
  CMD_PARSE_OBJ *parse_obj = parse_cmd(buf, ",");
  int i = 0;

	map_model->num = parse_obj->num;
  map_model->map = (int32_t*)malloc(sizeof(int32_t) * map_model->num +1);
  for(i=0; i<map_model->num; i++){
    map_model->map[i] = cmd_obj_get_int(parse_obj, i);
  }

  free_cmd_obj(parse_obj);
end:
	fclose(stream);
	if(buf) free(buf);

  return RET_OK;
}

/* Public functions ----------------------------------------------------------*/
int32_t ch_remap(uint32_t index, uint32_t ch)
{
  if(index<remap_obj_num && ch < mapping_objs[index].num){
    // APP_DEBUGF(REMAP_DEBUG | APP_DBG_TRACE, ("%d->%d\n", ch, mapping_objs[index].map[ch]));
     return mapping_objs[index].map[ch];
  }else return ch;
}

int32_t init_ch_remap(json_object *chremap_obj)
{
  uint32_t i =0;
  remap_path = config_get_string(chremap_obj, "REMAP_PATH", "Ch_remap/");

  remap_obj_num = config_get_array_lenth(chremap_obj, "REMAP_OBJ");
  if(remap_obj_num ==0) return RET_ERROR;
  mapping_objs = (MAPPING_MODEL*) malloc(sizeof(MAPPING_MODEL) * remap_obj_num);

  for(i=0; i<remap_obj_num; i++){
 		json_object *array_obj = config_get_array_obj(chremap_obj, "REMAP_OBJ", i);

    mapping_objs[i].name = config_get_string(array_obj, "REMAP_FILE", "NULL");
    remap_read_file(&mapping_objs[i]);
  }

  return RET_OK;
}

