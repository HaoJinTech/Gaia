/**
  ******************************************************************************
  * @file    bll_case_manager.h
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _BLL_CASE_MANAGER_H_
#define  _BLL_CASE_MANAGER_H_

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_config.h"

#include <dirent.h>

/* Exported types ------------------------------------------------------------*/
typedef enum Case_state{
    CASE_STATE_UNLOADED     =0x0000,
    CASE_STATE_STOP         =0x0001,
    CASE_STATE_RUN          =0x0002,
    CASE_STATE_PAUSE        =0x0003,
    CASE_STATE_BUSY         =0x0004
} CASE_STATE;

struct Case_item;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define RET_ERROR_CASE_UNLOAD     -101
#define RET_ERROR_CASE_BUSY       -102

/* Exported functions --------------------------------------------------------*/ 
int32_t init_model_case_manager(json_object *case_json_obj);

// get set status and attributes
const char *get_case_full_path(void);
struct Case_item *get_case_item(char *name);
CASE_STATE get_case_state(struct Case_item *case_item, char *out_state, uint32_t strlen);
uint32_t get_case_line_max(struct Case_item *case_item);
uint32_t get_case_times(struct Case_item *case_item);
uint32_t set_case_times(struct Case_item *case_item, uint32_t times);
uint32_t get_case_current_line(struct Case_item *case_item);
uint32_t set_case_current_line(struct Case_item *case_item, uint32_t line);
uint32_t set_case_interval(struct Case_item *case_item, uint32_t interval);

// msg related stuffs
int32_t send_upload_misson(char *case_name);
int32_t send_unload_misson(struct Case_item *case_item);
int32_t send_run_misson(struct Case_item *case_item);
int32_t send_stop_misson(struct Case_item *case_item);
int32_t send_pause_misson(struct Case_item *case_item);
int32_t send_countinu_misson(struct Case_item *case_item);

// file related stuffs
int32_t check_case_exist(char *name);
DIR* search_folder_start(void);
int32_t search_folder_get_name(DIR* dirp, char *out_name, uint32_t strlen, uint32_t *out_filesize);
void search_folder_end(DIR *dirp);

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
