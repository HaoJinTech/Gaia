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

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define RET_ERROR_CASE_UNLOAD     -101
#define RET_ERROR_CASE_BUSY       -102

/* Exported functions --------------------------------------------------------*/ 
int32_t init_model_case_manager(json_object *case_json_obj);
CASE_STATE get_case_state(char *case_name, char *out_state, uint32_t strlen);
const char *get_case_full_path(void);

DIR* search_folder_start(void);
int32_t search_folder_get_name(DIR* dirp, char *out_name, uint32_t strlen, uint32_t *out_filesize);
void search_folder_end(DIR *dirp);

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
