/**
  ******************************************************************************
  * @file    bll_att.c
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "bll/bll_att.h"
#include "bll/bll_pha.h"

#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL int32_t *val = 0;
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

int32_t set_pha(uint32_t ch, int32_t val)
{
    // TODO: send att value to DAL layer

    return RET_OK;
}

int32_t get_pha(uint32_t ch)
{
    return RET_OK;
}

int32_t init_pha(uint32_t channel, uint32_t maxval, uint32_t step)
{
    val = malloc(sizeof(int32_t) * channel);
    return RET_OK;
}
