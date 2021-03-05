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
#include "sys_config.h"
#include "platform.h"
#include "subboard_manager.h"
#include "bll/bll_att.h"
#include "bll/bll_pha.h"
#include "bll/bll_calibration.h"

#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ATT_DEBUG                       APP_DBG_ON
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL uint32_t g_ch_max = 0;
LOCAL uint32_t g_val_max = 0;
LOCAL int32_t *g_att_vals = 0;
LOCAL int32_t g_logic_step = 0;
LOCAL int32_t g_bd_step = 0;

LOCAL SUBBD_PROTOCOL  *g_protocol_obj = 0;
LOCAL BUS_DRIVER      *g_bus_obj = 0;
/* Private function prototypes -----------------------------------------------*/
LOCAL int16_t att_get_step_offset(int16_t val);
/* Private functions ----------------------------------------------------------*/

LOCAL int16_t att_get_step_offset(int16_t val)
{
	int16_t rt_val;
	if(g_logic_step < g_bd_step){					
		rt_val = val << (g_bd_step - g_logic_step);
	}else{
		rt_val = val >> (g_logic_step - g_bd_step);
	}
	return rt_val;
}


int32_t set_att(uint32_t ch, int32_t val)
{
    int32_t att;

    if(ch >= g_ch_max || val > g_val_max) 
        return RET_ERROR;

    att = att_get_step_offset(val);
    subbd_send_SCSV(DEST_ATT, g_protocol_obj, g_bus_obj, ch, att);
    if(cali_enable){
        int32_t pha;
        pha = get_pha(ch);
        pha = calibration_proc(ch, att, pha);
        subbd_send_SCSV(DEST_PHA, g_protocol_obj, g_bus_obj, ch, pha);
    }

    return RET_OK;
}

int32_t get_att(uint32_t ch)
{
    if(ch >= g_ch_max )
        return RET_ERROR;
   
    return g_att_vals[ch];
}

int32_t init_att(json_object *json_obj)
{
    int i=0;
    g_ch_max = 32;
    g_val_max = 440;
    g_att_vals = (int32_t*)malloc(sizeof(int32_t) * g_ch_max);
    g_protocol_obj = &protocols[PROTOCOL_ID_RR485];
    g_bus_obj = &bus_drivers[BUS_ID_SPI];
    g_logic_step = 2;
    g_bd_step = 2;

    for(i=0; i<g_ch_max; i++) {
        g_att_vals[i] = 0;
    }

    return RET_OK;
}


