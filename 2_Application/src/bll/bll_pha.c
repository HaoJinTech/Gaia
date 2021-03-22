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
#include "subboard_manager.h"
#include "bll/bll_att.h"
#include "bll/bll_pha.h"
#include "bll/bll_calibration.h"
#include "bll/protocol_env.h"
#include "bll/bll_ch_remap.h"

#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL uint32_t g_ch_max = 0;
LOCAL uint32_t g_val_max = 0;
LOCAL int32_t *g_pha_vals = 0;
LOCAL int     g_remap_enable = 0;
LOCAL int32_t g_remap_index = 0;

LOCAL SUBBD_PROTOCOL  *g_protocol_obj = 0;
LOCAL BUS_DRIVER      *g_bus_obj = 0;
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
int32_t get_pha_ch_max(void)
{
    return g_ch_max;
}

int32_t get_pha_val_max(void)
{
    return g_val_max;
}

int32_t refresh_pha_val(uint32_t ch)
{
    int32_t val = 0;
    if(ch >= g_ch_max )
        return RET_ERROR;

    val = g_pha_vals[ch];
    if(calibration_is_enabled()){
        int32_t att=0;
        int32_t att_out=0;
        att = get_att(ch);
        val = calibration_proc(ch, att, val, &att_out);
        subbd_send_SCSV(DEST_ATT, g_protocol_obj, g_bus_obj, ch, att_out);
    }

    subbd_send_SCSV(DEST_PHA, g_protocol_obj, g_bus_obj, ch, val);
    return RET_OK;
}

int32_t pha_refresh_all(void)
{
    uint32_t i =0;
    int32_t *ch = (int32_t*)malloc(sizeof(int32_t) * g_ch_max);
    for(i=0; i<g_ch_max; i++){
        ch[i] = i;
    }
    set_pha_array(ch, g_pha_vals, g_ch_max);
    if(ch) free(ch);
    return RET_OK;
}

int32_t set_pha(uint32_t ch, int32_t val)
{
    int32_t remap_ch =0;
    if(ch >= g_ch_max )
        return RET_ERROR;

    g_pha_vals[ch] = val;

    // remap the channel
    if(g_remap_enable){
        remap_ch = ch_remap(g_remap_index, ch);
    }else{
        remap_ch = ch;
    }
    if(calibration_is_enabled()){
        int32_t att=0;
        int32_t att_out=0;
        att = get_att(ch);
        val = calibration_proc(ch, att, val, &att_out);
        subbd_send_SCSV(DEST_ATT, g_protocol_obj, g_bus_obj, remap_ch, att_out);
    }

    subbd_send_SCSV(DEST_PHA, g_protocol_obj, g_bus_obj, remap_ch, val);
    return RET_OK;
}

int32_t set_pha_array(const int32_t *ch, const int32_t *value, uint32_t val_num)
{
    uint32_t i = 0;
    int32_t *ch_pha = NULL;
    int32_t *ch_att = NULL;
    int32_t *ch_pha = NULL;
    int32_t *val_pha = NULL;
    int32_t *val_att = NULL;


    ch_pha = (int32_t*) malloc(sizeof(int32_t)*val_num);
    val_pha = (int32_t*) malloc(sizeof(int32_t)*val_num);
    if(calibration_is_enabled()){
        ch_att = (int32_t*)malloc(sizeof(int32_t)*val_num);
        val_att = (int32_t*)malloc(sizeof(int32_t)*val_num);
    }

    for(i=0;i<val_num;i++){
        ch_pha[i] = ch[i];
        if(ch[i]>= g_ch_max) continue;
        ch_pha[i] = ch[i];
        val_pha[i] = value[i];
        if(val_pha[i] >= g_val_max) val_pha[i] = g_val_max;
        g_pha_vals[ch[i]] = val_pha[i];

        if(calibration_is_enabled()){
            val_att[i] = get_att(ch[i]);
            val_att[i] = att_get_step_offset(val_att[i]);
            val_pha[i] = calibration_proc(ch_pha[i], val_att[i], val_pha[i], &val_att[i]);
        }

        // remap the channel
        if(g_remap_enable){
            ch_pha[i] = ch_remap(g_remap_index, ch_pha[i]);
        }
        if(calibration_is_enabled()){
            ch_att[i] = ch_pha[i];
        }
    }

    subbd_send_MCMV(DEST_PHA, g_protocol_obj, g_bus_obj, ch_pha, val_pha, val_num);
    if(calibration_is_enabled()){
        subbd_send_MCMV(DEST_ATT, g_protocol_obj, g_bus_obj, ch_att, val_att, val_num);
    }
    return RET_OK;
}


int32_t get_pha(uint32_t ch)
{
    if(ch >= g_ch_max )
        return RET_ERROR;
   
    return g_pha_vals[ch];
}

int32_t init_pha(json_object *pha_obj)
{
    int protocol_id = 0;
    int bus_id = 0;

    g_ch_max =      config_get_int(pha_obj, "PHA_MAX_CH", 8);
    g_val_max =     config_get_int(pha_obj, "PHA_MAX_VAL", 360);
    g_remap_enable =config_get_bool(pha_obj, "PHA_REMAP_ENABLE", 0);
    g_remap_index = config_get_int(pha_obj, "PHA_REMAP_INDEX", 0);

    protocol_id =   config_get_int(pha_obj, "PHA_PROTOCOL", PROTOCOL_ID_RR485);
    if(protocol_id > SUBBD_PROTOCOL_SIZE) protocol_id = PROTOCOL_ID_RR485;
    g_protocol_obj = get_protocol_obj(protocol_id);

    bus_id =        config_get_int(pha_obj, "PHA_BUS", BUS_ID_SPI);
    if(bus_id > BUS_DRIVER_NUM) bus_id = BUS_ID_SPI;
    g_bus_obj =     get_bus_obj(bus_id);

    g_pha_vals = malloc(sizeof(int32_t) * g_ch_max);
    
    return RET_OK;
}
