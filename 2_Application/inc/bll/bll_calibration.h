/**
  ******************************************************************************
  * @file    bll_calibration.h
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _BLL_CALIBRATION_H_
#define  _BLL_CALIBRATION_H_
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 
int32_t calibration_is_enabled(void);
int32_t calibration_proc(uint32_t ch, int32_t att, int32_t pha, int32_t *o_attval);
void save_cscd_file(void);

char *cali_get_freq(uint32_t ch);
int32_t cali_set_freq(uint32_t ch, char *freq_str);

uint32_t get_cali_info_num(void);
char *get_cali_info_name_by_index(uint32_t i);

int32_t init_calibration(json_object *json_obj);

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
