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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int32_t cali_enable = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ----------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
int32_t calibration_proc(uint32_t ch, int32_t att, int32_t pha)
{
    return pha;
}

int32_t init_calibration(json_object *json_obj)
{
    cali_enable = 1;

    return RET_OK;
}
