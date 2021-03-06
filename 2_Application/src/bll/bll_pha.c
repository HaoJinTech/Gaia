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
LOCAL uint32_t g_ch_max = 0;
LOCAL uint32_t g_val_max = 0;
LOCAL int32_t *g_pha_vals = 0;
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
int32_t get_pha_ch_max()
{
    return g_ch_max;
}

int32_t set_pha(uint32_t ch, int32_t val)
{
    // TODO: send att value to DAL layer

    return RET_OK;
}

int32_t get_pha(uint32_t ch)
{
    if(ch >= g_ch_max )
        return RET_ERROR;
   
    return g_pha_vals[ch];
}

int32_t init_pha(json_object *json_obj)
{
    g_ch_max = 32;
    g_val_max = 365;
    g_pha_vals = malloc(sizeof(int32_t) * g_ch_max);
    
    return RET_OK;
}
