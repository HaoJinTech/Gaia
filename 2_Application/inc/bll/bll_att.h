/**
  ******************************************************************************
  * @file    bll_att.h
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _BLL_ATT_H_
#define  _BLL_ATT_H_
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 
int32_t att_is_enable(void);
int16_t att_get_step_offset(int16_t val);
int32_t set_att(uint32_t ch, int32_t val);
int32_t set_att_array(const int32_t *ch, const int32_t *value, uint32_t val_num);
int32_t get_att(uint32_t ch);
int32_t get_att_ch_max(void);

int32_t init_att(json_object *json_obj);
#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
