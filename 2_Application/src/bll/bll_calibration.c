/**
  ******************************************************************************
  * @file    bll_calibration.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "bll/bll_calibration.h"
#include "bll/bll_pha.h"
#include "bll/bll_att.h"
#include "list.h"
#include "platform.h"
#include "app_debug.h"
#include "file_reader.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
struct Cal_info{
	const char *cal_id;

	uint16_t point_num;
	int16_t *att_point;
	int16_t *pha_offset;

	int16_t pha_map_item_count;
	int16_t *pha_map_cal;
	
	int16_t cal_ch_max;
	uint32_t *line_start;
	
	int16_t *stretch;

	struct list_head  		list;
};

/* Private define ------------------------------------------------------------*/
#define FILE_PATH_LEN     128
#define FILE_PATH_TELATIVE_PATH  "./" 
#define CALI_DEBUG        APP_DBG_ON

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int32_t cali_enable = 0;

LOCAL struct Cal_info cal_info_root;
LOCAL uint32_t stretch_point =0;
uint8_t 			*scal_id = NULL;					// the calibration object index of every channel
int16_t				*scal_offset = NULL;
uint8_t				s_att_effect = 0;					// effect by att
uint8_t				s_pha_by_table_enable = 0;			// enable table calibration
uint8_t				s_stretch_enable = 0;

/* Private function prototypes -----------------------------------------------*/
LOCAL void get_inf_cal_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line);
LOCAL void get_pha_cal_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line);
LOCAL void get_pha_stretch_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line);
LOCAL int16_t calc_remainder(int16_t num, uint16_t rem);

/* Private functions ---------------------------------------------------------*/
// att inf pha cal data.
LOCAL void get_inf_cal_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line)
{
	char *inner_ptr = NULL; 
	char *buf;
	char *c_key, *c_val, *c_val_2;
	int16_t val, val_2, key;
	uint32_t read_size;

	APP_ASSERT("get_inf_cal_data: rb == NULL or cal_info == NULL.\r\n", rb && cal_info);

	if(cal_info->att_point == NULL ){
		cal_info->att_point = (int16_t*)malloc(line* sizeof(int16_t));
		cal_info->pha_offset = (int16_t*)malloc(line* sizeof(int16_t));
		cal_info->point_num = 0;
	}
	
	while(1){
		buf = (char *)rb_getline(rb, "\r\n", 2, &read_size);
		if(!buf){
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE,
			  ("round buffer read complite.\r\n"));
			break;
		}

		c_key = strtok_r((char *)buf, ",", &inner_ptr);
		c_val = strtok_r(NULL, ",", &inner_ptr);
		c_val_2 = strtok_r(NULL, "\r\n", &inner_ptr);

		if(c_key && c_val){

			key = (int16_t)atoi(c_key);
			val = (int16_t)atoi(c_val);
			val_2 = (int16_t)atoi(c_val_2);
			
			if(cal_info->att_point && 
				 cal_info->pha_offset &&
				 key < line){
				cal_info->point_num ++;
				cal_info->att_point[key] = val;
				cal_info->pha_offset[key] = val_2;
			}else{
				APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_WARNING ,
				  ("get_inf_cal_data: cal_info not initalised.\r\n"));
			}
		}else{
			APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS ,
			  ("get_inf_cal_data: file format error, read next line.\r\n"));
		}
		free(buf);
	}	
}

LOCAL void get_pha_cal_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line)
{
	char *inner_ptr = NULL; 
	char *buf;
	char *c_key, *c_val;
	int16_t val, key;
	uint32_t read_size;

	APP_ASSERT("get_pha_cal_data: rb == NULL or cal_info == NULL.\r\n", rb && cal_info);

	if(cal_info->pha_map_cal == NULL){
		cal_info->pha_map_cal = (int16_t*)malloc(sizeof(int16_t) * line);
		cal_info->pha_map_item_count = 0;
	}
	
	while(1){
		buf = (char *)rb_getline(rb, "\r\n", 2, &read_size);
		if(!buf){
			break;
		}

		c_key = strtok_r((char *)buf, ",", &inner_ptr);
		c_val = strtok_r(NULL, "\r\n", &inner_ptr);

		if(c_key && c_val){
			key = (int16_t)atoi(c_key);
			val = (int16_t)atoi(c_val);
			if(cal_info->pha_map_cal && key < line){
				cal_info->pha_map_cal[key] = val;
				cal_info->pha_map_item_count ++;
			}else{
				APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_WARNING ,
				("cal_info not initalised.\r\n"));
			}
		}else{
			APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS ,
			("file format error, read next line.\r\n"));
		}
		free(buf);
	}
}

LOCAL void get_pha_stretch_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line)
{
	char *inner_ptr = NULL; 
	char *buf;
	char *c_ch, *c_val;
	int16_t ch, val;
	uint32_t read_size;
	int i =0;

	APP_ASSERT("get_offset_data: rb == NULL\r\n", rb);
	APP_ASSERT("get_offset_data: cal_info == NULL\r\n", cal_info);

	if(cal_info->stretch == NULL){
		cal_info->stretch = (int16_t*)malloc(get_pha_ch_max() * sizeof(int16_t) * (stretch_point+1));
	}
	if(!cal_info->stretch){
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS | APP_DBG_TRACE,
		("get_offset_data: cal_info->stretch out of memory.\r\n"));
		return;
	}
	while(1){
		buf = (char *)rb_getline(rb, "\r\n", 2, &read_size);
		if(!buf){
			break;
		}
		c_ch = strtok_r((char *)buf, ",", &inner_ptr);
		if(!c_ch){
			APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS | APP_DBG_TRACE,
			("get_offset_data: file format error.\r\n"));
			free(buf);
			return;
		}
		ch = (int16_t)atoi(c_ch);
		for(i=0; i<stretch_point+1; i++){
			if(i == stretch_point){
				c_val = strtok_r(NULL, "\r", &inner_ptr);
			}else{
				c_val = strtok_r(NULL, ",", &inner_ptr);
			}
			
			if(!c_val){
				APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS | APP_DBG_TRACE,
				("get_offset_data: file format error.\r\n"));
				free(buf);
				return;
			}
			val = (int16_t)atoi(c_val);
			if(ch< get_pha_ch_max()){
				cal_info->stretch[ch*(stretch_point+1) + i] = val;
			}
		}
		
		free(buf);
		buf = NULL;
	}
}

LOCAL struct Cal_info *get_cal_info_by_iter(uint16_t i)
{
	uint16_t count = 0;
	
    struct Cal_info *iter;
    list_for_each_entry(iter, &cal_info_root.list, list){
		if(count == i){
			return iter;
		}
		count++;
    }

	return NULL;
}

LOCAL int16_t calc_remainder(int16_t num, uint16_t rem)
{
	uint16_t temp_num;
	if(rem == 0) return num;
	while(num<0){
		num += rem;
	}
	temp_num = num;
	
	return temp_num % rem;
}

uint8_t getMapPosition(struct Cal_info *cal_info, int32_t val)
{
	uint8_t i=0;
	for(i=0; i<stretch_point; i++){
		if(val<=cal_info->stretch[i+1] && val>=cal_info->stretch[i])
			return i;
	}
	return 0;
}

/* Public functions ----------------------------------------------------------*/
int32_t calibration_proc(uint32_t ch, int32_t att_val, int32_t val)
{
	uint16_t j = 0;
	int16_t offset = 0;
	struct Cal_info *cal_info;

	cal_info = get_cal_info_by_iter(scal_id[ch]);
	if(!cal_info) {
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS ,
			("calibration infomation not found.\r\n"));	
		return 0;
	}
	if(cal_info->point_num == 0) {
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS ,
			("calibration infomation number is 0.\r\n"));	
		return 0;
	}
	// step1:  offset
	val += scal_offset[ch];
	
	// step2:  attinf
	if(s_att_effect && cal_info->point_num){
		for(j=1; j<cal_info->point_num; j++){
			if(cal_info->att_point[j-1]<= att_val && cal_info->att_point[j]> att_val){
				break;
			}
		}
		offset = cal_info->pha_offset[j-1];
		val += offset;
	}

	// step3:  remainder
	val = calc_remainder(val, cal_info->pha_map_item_count);
	
	// step4:  phamap
	val = cal_info->pha_map_cal[val];
	// step5:  stretch (optional)
	if(stretch_point){
		uint8_t map_min_i = getMapPosition(cal_info, val);
		int16_t map_min = cal_info->stretch[map_min_i];
		int16_t map_max = cal_info->stretch[map_min_i+1];
		int16_t min_stretch = cal_info->stretch[ch*(stretch_point+1)+map_min_i];
		int16_t max_stretch = cal_info->stretch[ch*(stretch_point+1)+map_min_i+1];
		int16_t map_diff = map_max - map_min;
		int16_t stretch_diff = max_stretch - min_stretch;
		float temp_multiplier = (float)stretch_diff/(float)map_diff;
		val =  (val-map_min) * temp_multiplier + min_stretch;
	}
	return val;
}

int32_t init_calibration(json_object *cali_obj)
{
    int i =0, cali_obj_num = 0, flag = 0;
    const char *filename;
	const char *relative_path;
	char full_path[FILE_PATH_LEN];
	json_object *array_obj = 0;

    cali_enable = 1;

    if(!cali_obj)
      return RET_ERROR;

	INIT_LIST_HEAD(&cal_info_root.list);

	relative_path = config_get_string(cali_obj, "CALI_FOLDER_PATH", "Calibration/");
    cali_obj_num = config_get_array_lenth(cali_obj, "CALI_OBJS");
    for(i=0; i<cali_obj_num; i++){
		struct Cal_info *cal_info = (struct Cal_info *)malloc(sizeof(struct Cal_info));
		memset(cal_info, 0, sizeof(struct Cal_info));
		array_obj = config_get_array_obj(cali_obj, "CALI_OBJS", i);

		s_att_effect = config_get_bool(array_obj, "ADJ_PHA_BY_ATT", 0);
		if(flag){
			filename = config_get_string(array_obj, "ADJ_PHA_BY_ATT_FILE", "");
			strncpy(full_path, relative_path, FILE_PATH_LEN);
			strncat(full_path, filename, FILE_PATH_LEN - strlen(relative_path));
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE, ("load file:(%s).\r\n", full_path));
			csv_cal_read_file(full_path, (File_reader)get_inf_cal_data, cal_info);
		}

		s_pha_by_table_enable = config_get_bool(array_obj, "ADJ_PHA_BY_TABLE", 0);
		if(flag){
			filename = config_get_string(array_obj, "ADJ_PHA_BY_TABLE_FILE", "");
			strncpy(full_path, relative_path, FILE_PATH_LEN);
			strncat(full_path, filename, FILE_PATH_LEN - strlen(relative_path));
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE, ("load file:(%s).\r\n", full_path));
			csv_cal_read_file(full_path, (File_reader)get_pha_cal_data, cal_info);
		}

		s_stretch_enable = config_get_bool(array_obj, "ADJ_PHA_BY_STRETCH", 0);
		if(flag){
			stretch_point = config_get_int(array_obj, "ADJ_PHA_BY_STRETCH_POINT", 0);
			filename = config_get_string(array_obj, "ADJ_PHA_BY_STRETCH_FILE", "");
			strncpy(full_path, relative_path, FILE_PATH_LEN);
			strncat(full_path, filename, FILE_PATH_LEN - strlen(relative_path));
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE, ("load file:(%s).\r\n", full_path));
			csv_cal_read_file(full_path, (File_reader)get_pha_stretch_data, cal_info);
		}

		// set calibration point name
		cal_info->cal_id = config_get_string(array_obj, "NAME", "0G0");
		list_add(&cal_info->list, &cal_info_root.list);
    }

	scal_id = (uint8_t*)malloc(sizeof(uint8_t) * get_pha_ch_max());
	if(!scal_id){
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS,
			("get_phase_cal_config: cal_id malloc failed.\r\n"));
		return RET_ERROR;
	}
	memset(scal_id, 0, sizeof(uint8_t) * get_pha_ch_max());
    return RET_OK;
}
