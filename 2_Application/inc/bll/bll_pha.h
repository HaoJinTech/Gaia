/**
  ******************************************************************************
  * @file    bll_pha.h
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _BLL_PHA_H_
#define  _BLL_PHA_H_
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 
int32_t set_pha(uint32_t ch, int32_t val);
int32_t get_pha(uint32_t ch);
int32_t get_pha_ch_max(void);
int32_t get_pha_val_max(void);
int32_t refresh_pha_val(uint32_t ch);

int32_t init_pha(json_object *json_obj);
#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
