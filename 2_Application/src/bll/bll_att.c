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
#include "bll/bll_att.h"
#include "subboard_manager.h"

#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ATT_DEBUG                       APP_DBG_ON
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL uint32_t g_ch_max = 0;
LOCAL uint32_t g_val_max = 440;
LOCAL int32_t *g_att_vals = 0;
LOCAL int32_t g_logic_step = 0;
LOCAL int32_t g_bd_step = 0;

LOCAL int32_t g_protocol_id = 0;
LOCAL int32_t g_bus_id = 0;
/* Private function prototypes -----------------------------------------------*/
LOCAL int16_t att_get_step_offset(int16_t val);

/* Public functions ----------------------------------------------------------*/
int32_t calibration_proc_att(uint32_t ch, int32_t att, int32_t pha)
{
    return att;
}

int32_t set_att(uint32_t ch, int32_t val)
{
    int32_t val_t;

    // TODO: send att value to DAL layer
    if(ch >= g_ch_max || val > g_val_max) 
        return RET_ERROR;

    val_t = att_get_step_offset(val);
    calibration_proc_att(ch, val_t, 0);
    subbd_send_SCSV(DEST_ATT, PROTOCOL_ID_RR485, BUS_ID_SPI, ch, val_t);

    return RET_OK;
}

int32_t get_att(uint32_t ch)
{
    return RET_OK;
}

int32_t init_att(void)
{
    int i=0;
    g_ch_max = 32;
    g_val_max = 440;
    g_att_vals = (int32_t*)malloc(sizeof(int32_t) * g_ch_max);
    g_protocol_id = PROTOCOL_ID_RR485;
    g_bus_id = BUS_ID_SPI;
    g_logic_step = 2;
    g_bd_step = 2;

    for(i=0; i<g_ch_max; i++) {
        g_att_vals[i] = 0;
    }

    subbd_send_CCSV(DEST_ATT, PROTOCOL_ID_RR485, BUS_ID_SPI, 0, 0, g_ch_max);

    return RET_OK;
}

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
