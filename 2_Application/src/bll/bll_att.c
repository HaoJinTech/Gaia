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
#include "bll/bll_ch_remap.h"
#include "bll/protocol_env.h"

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
LOCAL int     g_remap_enable = 0;
LOCAL int32_t g_remap_index = 0;
LOCAL int32_t g_att_enable = 0;

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

/* Public functions ----------------------------------------------------------*/
int32_t att_is_enable(void)
{
    return g_att_enable;
}

int32_t get_att_ch_max(void)
{
    return g_ch_max;
}

int32_t set_att(uint32_t ch, int32_t val)
{
    int32_t att;
    int32_t remap_ch;

    if(ch >= g_ch_max || val > g_val_max) 
        return RET_ERROR;
    g_att_vals[ch] = val;
    att = att_get_step_offset(val);

    // remap the channel
    if(g_remap_enable){
        remap_ch = ch_remap(g_remap_index, ch);
    }else{
        remap_ch = ch;
    }
   
    if(calibration_is_enabled()){
        int32_t pha;
        pha = calibration_proc(ch, att, get_pha(ch), &att);
        subbd_send_SCSV(DEST_PHA, g_protocol_obj, g_bus_obj, remap_ch, pha);
    }
    subbd_send_SCSV(DEST_ATT, g_protocol_obj, g_bus_obj, remap_ch, att);

    return RET_OK;
}

int32_t set_att_array(int32_t *ch, int32_t *value, uint32_t val_num)
{
    uint32_t i = 0;
    int32_t *ch_att = NULL;
    int32_t *ch_pha = NULL;
    int32_t *pha = NULL;

    ch_att = (int32_t*)malloc(sizeof(int32_t)*val_num);
    if(calibration_is_enabled()){
        ch_pha = (int32_t*) malloc(sizeof(int32_t)*val_num);
        pha = (int32_t*) malloc(sizeof(int32_t)*val_num);
    }

    for(i=0;i<val_num;i++){
        if(ch[i]>= g_ch_max) continue;
        ch_att[i] = ch[i];
        if(value[i] >= g_val_max) value[i] = g_val_max;
        g_att_vals[ch[i]] = value[i];
        value[i] = att_get_step_offset(value[i]);

        if(calibration_is_enabled()){
            pha[i] = calibration_proc(ch[i], value[i], get_pha(ch[i]), &value[i]);
        }

        // remap the channel
        if(g_remap_enable){
            ch_att[i] = ch_remap(g_remap_index, ch[i]);
            if(calibration_is_enabled()){
                ch_pha[i] = ch_att[i];
            }
        }
    }

    subbd_send_MCMV(DEST_ATT, g_protocol_obj, g_bus_obj, ch_att, value, val_num);
    if(calibration_is_enabled()){
        subbd_send_MCMV(DEST_PHA, g_protocol_obj, g_bus_obj, ch_pha, pha, val_num);
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

    g_att_enable = 1;
    g_ch_max =      config_get_int(att_obj, "ATT_MAX_CH", 8);
    g_val_max =     config_get_int(att_obj, "ATT_MAX_VAL", 110);
    g_logic_step =  config_get_int(att_obj, "ATT_LOGIC_STEP", 2);
    g_bd_step =     config_get_int(att_obj, "ATT_BD_STEP", 2);
    g_remap_enable =config_get_bool(att_obj, "ATT_REMAP_ENABLE", 0);
    g_remap_index = config_get_int(att_obj, "ATT_REMAP_INDEX", 0);
    protocol_id =   config_get_int(att_obj, "ATT_PROTOCOL", PROTOCOL_ID_RR485);
    if(protocol_id > SUBBD_PROTOCOL_SIZE) protocol_id = PROTOCOL_ID_RR485;
    g_protocol_obj = get_protocol_obj(protocol_id);

    bus_id =        config_get_int(att_obj, "ATT_BUS", BUS_ID_SPI);
    if(bus_id > BUS_DRIVER_NUM) bus_id = BUS_ID_SPI;
    g_bus_obj =     get_bus_obj(bus_id);

    g_att_vals =    (int32_t*)malloc(sizeof(int32_t) * g_ch_max);

    for(i=0; i<g_ch_max; i++) {
        g_att_vals[i] = 0;
    }

    return RET_OK;
}


