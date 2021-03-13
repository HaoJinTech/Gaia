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

#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL uint32_t g_ch_max = 0;
LOCAL uint32_t g_val_max = 0;
LOCAL int32_t *g_pha_vals = 0;

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
        if(att != att_out)
            subbd_send_SCSV(DEST_ATT, g_protocol_obj, g_bus_obj, ch, att_out);
    }

    subbd_send_SCSV(DEST_PHA, g_protocol_obj, g_bus_obj, ch, val);
    return RET_OK;
}

int32_t set_pha(uint32_t ch, int32_t val)
{
    if(ch >= g_ch_max )
        return RET_ERROR;

    g_pha_vals[ch] = val;
    if(calibration_is_enabled()){
        int32_t att=0;
        int32_t att_out=0;
        att = get_att(ch);
        val = calibration_proc(ch, att, val, &att_out);
        if(att != att_out)
            subbd_send_SCSV(DEST_ATT, g_protocol_obj, g_bus_obj, ch, att_out);
    }

    subbd_send_SCSV(DEST_PHA, g_protocol_obj, g_bus_obj, ch, val);
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

    protocol_id =   config_get_int(pha_obj, "ATT_PROTOCOL", PROTOCOL_ID_RR485);
    if(protocol_id > SUBBD_PROTOCOL_SIZE) protocol_id = PROTOCOL_ID_RR485;
    g_protocol_obj = get_protocol_obj(protocol_id);

    bus_id =        config_get_int(pha_obj, "ATT_BUS", BUS_ID_SPI);
    if(bus_id > BUS_DRIVER_NUM) bus_id = BUS_ID_SPI;
    g_bus_obj =     get_bus_obj(bus_id);

    g_pha_vals = malloc(sizeof(int32_t) * g_ch_max);
    
    return RET_OK;
}
