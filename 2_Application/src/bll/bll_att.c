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

int32_t init_att(json_object *att_obj)
{
    int i=0;
    int protocol_id = 0;
    int bus_id = 0;

    if(!att_obj)
        return RET_ERROR;

    g_ch_max =      config_get_int(att_obj, "ATT_MAX_CH", 8);
    g_val_max =     config_get_int(att_obj, "ATT_MAX_VAL", 110);
    g_logic_step =  config_get_int(att_obj, "ATT_LOGIC_STEP", 2);
    g_bd_step =     config_get_int(att_obj, "ATT_BD_STEP", 2);

    protocol_id =   config_get_int(att_obj, "ATT_PROTOCOL", PROTOCOL_ID_RR485);
    if(protocol_id > SUBBD_PROTOCOL_SIZE) protocol_id = PROTOCOL_ID_RR485;
    g_protocol_obj = &protocols[protocol_id];

    bus_id =        config_get_int(att_obj, "ATT_BUS", BUS_ID_SPI);
    if(bus_id > BUS_DRIVER_NUM) bus_id = BUS_ID_SPI;
    g_bus_obj =     &bus_drivers[bus_id];

    g_att_vals =    (int32_t*)malloc(sizeof(int32_t) * g_ch_max);

    for(i=0; i<g_ch_max; i++) {
        g_att_vals[i] = 0;
    }

    return RET_OK;
}


