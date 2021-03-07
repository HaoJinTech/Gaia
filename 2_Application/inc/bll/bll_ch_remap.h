/**
  ******************************************************************************
  * @file    bll_ch_remap.h
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _BLL_CH_REMAP_H_
#define  _BLL_CH_REMAP_H_
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_config.h"

/* Exported types ------------------------------------------------------------*/
typedef struct Mapping_model{
	const char *name;
	uint16_t num;
	int32_t* map;
}MAPPING_MODEL;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 
int32_t init_ch_remap(json_object *pha_obj);

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
