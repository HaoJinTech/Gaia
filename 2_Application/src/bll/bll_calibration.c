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
#include <fcntl.h>
#include <unistd.h>

/* Private typedef -----------------------------------------------------------*/
struct Cal_info{
	const char *cal_id;

	// att effect pha
	uint16_t point_num;
	int16_t *att_point;
	int16_t *pha_offset;

	// pha effect att
	uint16_t point_num_att;
	int16_t *pha_point;
	int16_t *att_offset;

	// pha value remap
	int16_t pha_map_item_count;
	int16_t *pha_map_cal;
	
	int16_t cal_ch_max;
	uint32_t *line_start;
	
	int16_t *stretch;

	struct list_head  		list;
};

/* Private define ------------------------------------------------------------*/
#define FILE_PATH_TELATIVE_PATH  "./" 
#define CALI_DEBUG        APP_DBG_ON

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int32_t cali_enable = 0;

LOCAL struct Cal_info 	cal_info_root;
LOCAL uint32_t 			g_cal_info_num;
LOCAL uint8_t 			*scal_id = NULL;					// the calibration object index of every channel
LOCAL int16_t			*scal_offset = NULL;
LOCAL uint8_t			s_att_effect = 0;					// is pha effect by att
LOCAL uint8_t			s_pha_effect = 0; 					// is att effect by pha
LOCAL uint8_t			s_pha_by_table_enable = 0;			// is effect by table calibration
LOCAL uint8_t			s_stretch_enable = 0;				// is effect by stretch
LOCAL uint32_t 			stretch_point =0;
LOCAL const char		*cali_pt_filename = 0;
LOCAL const char 		*relative_path = 0;

/* Private function prototypes -----------------------------------------------*/
// File reader call back function 
LOCAL void get_inf_cal_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line);
LOCAL void get_pha_cal_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line);
LOCAL void get_pha_stretch_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line);
LOCAL void get_offset_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line);
LOCAL void get_freq_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line);

LOCAL int16_t calc_remainder(int16_t num, uint16_t rem);
LOCAL struct Cal_info *get_cal_info_by_iter(uint16_t i);
LOCAL uint8_t getMapPosition(struct Cal_info *cal_info, int32_t val);
LOCAL int16_t cal_info_id2iter(char *id);

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

LOCAL void get_attinf_cal_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line)
{
	char *inner_ptr = NULL; 
	char *buf;
	char *c_key, *c_val, *c_val_2;
	int16_t val, val_2, key;
	uint32_t read_size;

	APP_ASSERT("get_attinf_cal_data: rb == NULL or cal_info == NULL.\r\n", rb && cal_info);

	if(cal_info->pha_point == NULL ){
		cal_info->pha_point = (int16_t*)malloc(line* sizeof(int16_t));
		cal_info->att_offset = (int16_t*)malloc(line* sizeof(int16_t));
		cal_info->point_num_att = 0;
	}
	
	while(1){
		buf = (char *)rb_getline(rb, "\r\n", 2, &read_size);
		if(!buf){
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE,
				("get_inf_cal_data: round buffer read complite.\r\n"));
			break;
		}

		c_key = strtok_r((char *)buf, ",", &inner_ptr);
		c_val = strtok_r(NULL, ",", &inner_ptr);
		c_val_2 = strtok_r(NULL, "\r\n", &inner_ptr);

		if(c_key && c_val){

			key = (int16_t)atoi(c_key);
			val = (int16_t)atoi(c_val);
			val_2 = (int16_t)atoi(c_val_2);
			
			if(cal_info->pha_point && 
				 cal_info->att_offset &&
				 key < line){
				cal_info->point_num_att ++;
				cal_info->pha_point[key] = val;
				cal_info->att_offset[key] = val_2;
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

	APP_ASSERT(" rb == NULL\r\n", rb);
	APP_ASSERT(" cal_info == NULL\r\n", cal_info);

	if(cal_info->stretch == NULL){
		cal_info->stretch = (int16_t*)malloc(get_pha_ch_max() * sizeof(int16_t) * (stretch_point+1));
	}
	if(!cal_info->stretch){
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS | APP_DBG_TRACE,
			(" cal_info->stretch out of memory.\r\n"));
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
				(" file format error.\r\n"));
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
					(" file format error.\r\n"));
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

LOCAL uint8_t getMapPosition(struct Cal_info *cal_info, int32_t val)
{
	uint8_t i=0;
	for(i=0; i<stretch_point; i++){
		if(val<=cal_info->stretch[i+1] && val>=cal_info->stretch[i])
			return i;
	}
	return 0;
}

LOCAL int16_t cal_info_id2iter(char *id)
{
	uint16_t count = 0;
	struct Cal_info *iter;

    list_for_each_entry(iter, &cal_info_root.list, list){
		if(0== strncmp(id, iter->cal_id, strlen(iter->cal_id))){
            return count;
		}
		count++;
    }
	return -1;
}

LOCAL void get_freq_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line)
{
	char *inner_ptr = NULL; 
	char *buf;
	char *c_ch, *c_val;
	int16_t ch,val;
	uint32_t read_size;

	APP_ASSERT("rb == NULL or scal_id == NULL\r\n", rb && scal_id);
	
	while(1){
		buf = (char *)rb_getline(rb, "\r\n", 2, &read_size);
		if(!buf){
			break;
		}
		c_ch = strtok_r((char *)buf, ",", &inner_ptr);
		if(!c_ch){
			APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS,
				("file format error.\r\n"));
			free(buf);
			return;
		}
		c_val = strtok_r(NULL, "\r", &inner_ptr);
		if(!c_val){
			APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS,
				("file format error.\r\n"));
			free(buf);
			return;
		}
		ch = (int16_t)atoi(c_ch);
		val = cal_info_id2iter(c_val);
		if(ch< get_pha_ch_max() && val >=0){
			scal_id[ch] = val;
		}
		free(buf);
		buf = NULL;
	}
}

LOCAL void get_offset_data(struct rb *rb, struct Cal_info *cal_info, uint16_t line)
{
	char *inner_ptr = NULL; 
	char *buf;
	char *c_ch, *c_val;
	int16_t ch,val;
	uint32_t read_size;

	APP_ASSERT("rb == NULL\r\n", rb);

	while(1){
		buf = (char *)rb_getline(rb, "\r\n", 2, &read_size);
		if(!buf) break;

		c_ch = strtok_r((char *)buf, ",", &inner_ptr);
		if(!c_ch){
			APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS, ("file format error.\r\n"));
			free(buf);
			return;
		}
		c_val = strtok_r(NULL, "\r", &inner_ptr);
		if(!c_val){
			APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS, ("file format error.\r\n"));
			free(buf);
			return;
		}
		ch = (int16_t)atoi(c_ch);
		val = (int16_t)atoi(c_val);
		
		if(ch< get_pha_ch_max()){
			scal_offset[ch]=val;
		}
		free(buf);
		buf = NULL;
	}
}


/* Public functions ----------------------------------------------------------*/
int32_t calibration_is_enabled(void)
{
	return cali_enable;
}

uint32_t get_cali_info_num(void)
{
	return g_cal_info_num;
}

char *get_cali_info_name_by_index(uint32_t i)
{
	struct Cal_info *cal_info = NULL;

	if(i < g_cal_info_num){
		cal_info = get_cal_info_by_iter(i);
		return cal_info->cal_id;
	}
	return NULL;
}

int32_t calibration_proc(uint32_t ch, int32_t att_val, int32_t pha_val, int32_t *o_attval)
{
	uint16_t j = 0;
	int16_t offset = 0;
	struct Cal_info *cal_info;
	int32_t val = pha_val;

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
	
	// step4:  phainf
	if(s_pha_effect && cal_info->point_num_att){
		for(j=1; j<cal_info->point_num_att; j++){
			if(cal_info->pha_point[j-1]<= val && cal_info->pha_point[j]> val){
				break;
			}
		}
		offset = cal_info->att_offset[j-1];
		if(o_attval){
			(*o_attval) = att_val + offset;
		}
	}

	// step5:  phamap
	val = cal_info->pha_map_cal[val];

	// step6:  stretch (optional)
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

int32_t cali_set_freq(uint32_t ch, char *freq_str)
{
	struct Cal_info *cal_info;
	int16_t cal_inter;
	cal_inter = cal_info_id2iter(freq_str);

	if(-1 == cal_inter){
		APP_DEBUGF(CONF_DEBUG | APP_DBG_LEVEL_WARNING ,
			("can not find calibration info.(ch = %d, id = %s)\r\n", ch, freq_str));
		return RET_ERROR;
	}

	cal_info = get_cal_info_by_iter(cal_inter);
	if(!cal_info){
		APP_DEBUGF(CONF_DEBUG | APP_DBG_LEVEL_WARNING ,
			("can not find calibration info.(id = %s)\r\n", cal_inter));
		return RET_ERROR;
	}
	scal_id[ch] = cal_inter;

	refresh_pha_val(ch);
}

char *cali_get_freq(uint32_t ch)
{
	struct Cal_info *cal_info;
	cal_info = get_cal_info_by_iter(scal_id[ch]);
	if(!cal_info){
		cmd_kprintf(ts, "%d F;", i+1);
		APP_DEBUGF(CONF_DEBUG | APP_DBG_LEVEL_WARNING | APP_DBG_TRACE,
		("can not find calibration info.(ch = %d)\r\n", ch));
		continue;
	}else{
		return cal_info->cal_id;
	}
	return NULL;
}

void save_cscd_file(void)
{
	char full_path[FILE_PATH_LEN];
#define LINE_TEMP_SIZE	64
	char line_temp[LINE_TEMP_SIZE];
	int write_len;
	int fd;
	int16_t i;

	snprintf(full_path, FILE_PATH_LEN, "%s/%s%s", 
		PRJ_FILE_PATH, relative_path, cali_pt_filename);

/*	mkdir(CALIBRATION_DIR, O_CREAT);*/
	fd = open(full_path, O_WRONLY | O_CREAT | O_TRUNC, 0);
	if(fd<0){
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_WARNING ,
			(" open file failed.\r\n"));
		return;
	}

	for(i = 0; i<get_pha_ch_max(); i++){
		struct Cal_info * cal_info = get_cal_info_by_iter(scal_id[i]);

		snprintf(line_temp, LINE_TEMP_SIZE, "%d,%s\r\n", i, cal_info->cal_id);
		write_len = write(fd, line_temp, strlen(line_temp));
		if(write_len<0){
			APP_DEBUGF(CALI_DEBUG ,
				("file read complate.(fn = \"%s\",code = %d)\r\n", full_path, write_len));
			break;
		}
	}
	close(fd);
}

int32_t init_calibration(json_object *cali_obj)
{
    int i =0, cali_obj_num = 0;
    const char *filename;

	char full_path[FILE_PATH_LEN];
	json_object *array_obj = 0;

    cali_enable = 1;

    if(!cali_obj)
      return RET_ERROR;

	INIT_LIST_HEAD(&cal_info_root.list);

	relative_path = config_get_string(cali_obj, "CALI_FOLDER_PATH", "Calibration/");
    cali_obj_num = config_get_array_lenth(cali_obj, "CALI_OBJS");
    g_cal_info_num = 0;
    for(i=0; i<cali_obj_num; i++){
		struct Cal_info *cal_info = (struct Cal_info *)malloc(sizeof(struct Cal_info));
		memset(cal_info, 0, sizeof(struct Cal_info));
		array_obj = config_get_array_obj(cali_obj, "CALI_OBJS", i);
		// load 2G6_inf.csv 
		s_att_effect = config_get_bool(array_obj, "ADJ_PHA_BY_ATT", 0);
		if(s_att_effect){
			filename = config_get_string(array_obj, "ADJ_PHA_BY_ATT_FILE", "");
			strncpy(full_path, relative_path, FILE_PATH_LEN);
			strncat(full_path, filename, FILE_PATH_LEN - strlen(relative_path));
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE, ("load file:(%s).\r\n", full_path));
			csv_read_file(full_path, (File_reader)get_inf_cal_data, cal_info);
		}
		// load 2G6_attinf.csv 
		s_pha_effect = config_get_bool(array_obj, "ADJ_ATT_BY_PHA", 0);
		if(s_pha_effect){
			filename = config_get_string(array_obj, "ADJ_ATT_BY_PHA_FILE", "");
			strncpy(full_path, relative_path, FILE_PATH_LEN);
			strncat(full_path, filename, FILE_PATH_LEN - strlen(relative_path));
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE, ("load file:(%s).\r\n", full_path));
			csv_read_file(full_path, (File_reader)get_attinf_cal_data, cal_info);
		}
		// load 2G6_pha.csv 
		s_pha_by_table_enable = config_get_bool(array_obj, "ADJ_PHA_BY_TABLE", 0);
		if(s_pha_by_table_enable){
			filename = config_get_string(array_obj, "ADJ_PHA_BY_TABLE_FILE", "");
			strncpy(full_path, relative_path, FILE_PATH_LEN);
			strncat(full_path, filename, FILE_PATH_LEN - strlen(relative_path));
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE, ("load file:(%s).\r\n", full_path));
			csv_read_file(full_path, (File_reader)get_pha_cal_data, cal_info);
		}
		// load 2G6_stretch.csv
		s_stretch_enable = config_get_bool(array_obj, "ADJ_PHA_BY_STRETCH", 0);
		if(s_stretch_enable){
			stretch_point = config_get_int(array_obj, "ADJ_PHA_BY_STRETCH_POINT", 0);
			filename = config_get_string(array_obj, "ADJ_PHA_BY_STRETCH_FILE", "");
			strncpy(full_path, relative_path, FILE_PATH_LEN);
			strncat(full_path, filename, FILE_PATH_LEN - strlen(relative_path));
			APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE, ("load file:(%s).\r\n", full_path));
			csv_read_file(full_path, (File_reader)get_pha_stretch_data, cal_info);
		}

		// set calibration point name
		cal_info->cal_id = config_get_string(array_obj, "NAME", "0G0");
		list_add(&cal_info->list, &cal_info_root.list);
		g_cal_info_num ++;
    }
	// load offset
	scal_offset = (int16_t*)malloc(sizeof(int16_t) * get_pha_ch_max());
	if(!scal_offset){
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS ,
			("get_phase_cal_config: scal_offset malloc failed.\r\n"));
		return RET_ERROR;
	}
	memset(scal_offset, 0, sizeof(int16_t) * get_pha_ch_max());

	filename = config_get_string(cali_obj, "CALI_OFFSET_FILE", "pha_freq");
	if(!filename){
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_WARNING,
			("get_phase_cal_config: unspecified calibreation phase shifter offset file name.\r\n"));
		return RET_ERROR;
	}
	strncpy(full_path, relative_path, FILE_PATH_LEN);
	strncat(full_path, filename, FILE_PATH_LEN - strlen(relative_path));
	csv_read_file(full_path, (File_reader)get_offset_data, NULL);

	// load calibration point
	scal_id = (uint8_t*)malloc(sizeof(uint8_t) * get_pha_ch_max());
	if(!scal_id){
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_SERIOUS,
			("get_phase_cal_config: cal_id malloc failed.\r\n"));
		return RET_ERROR;
	}
	memset(scal_id, 0, sizeof(uint8_t) * get_pha_ch_max());

	cali_pt_filename = config_get_string(cali_obj, "CALI_SAVED_POINTS_FILE", "pha_freq.csv");
	if(!cali_pt_filename){
		APP_DEBUGF(CALI_DEBUG | APP_DBG_LEVEL_WARNING ,
			("get_phase_cal_config: unspecified calibreation phase shifter frequnce file name.\r\n"));
		return RET_ERROR;
	}
	strncpy(full_path, relative_path, FILE_PATH_LEN);
	strncat(full_path, cali_pt_filename, FILE_PATH_LEN - strlen(relative_path));
	APP_DEBUGF(CALI_DEBUG | APP_DBG_TRACE, ("sending calibreation sheet ...\r\n"));
	csv_read_file(full_path, (File_reader)get_freq_data, NULL);
	save_cscd_file();

    return RET_OK;
}
