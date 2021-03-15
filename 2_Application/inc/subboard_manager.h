/**
  ******************************************************************************
  * @file    subboard_manager.h
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _SUBBOARD_MANAGER_H_
#define  _SUBBOARD_MANAGER_H_
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "bus_prototype.h"
#include "subbd_protocol.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define DATA_TYPE_FLEX      0 // flexible value
#define DATA_TYPE_SCSV      1 // single channel single value
#define DATA_TYPE_MCMV      2 // multi channel multi value
#define DATA_TYPE_MCMMV     3 // multi channel multi differents value (att and pha at same time)
#define DATA_TYPE_CCMV      4 // continuous channel multi value (offset and lenth)
#define DATA_TYPE_CCSV      5 // continuous channel single value
#define DATA_TYPE_CCMMV     6 // continuous channel multi differents value (att and pha at same time)

/* Exported functions --------------------------------------------------------*/ 
int32_t init_rfboard_manager(void);

uint32_t get_msg_rsv_index(void);
uint32_t get_msg_snd_index(void);

int32_t subbd_send_FLEX(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, void *value);
int32_t subbd_send_SCSV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t channel, int32_t value);
int32_t subbd_send_MCMV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t *channel, int32_t *value, uint32_t ch_lenth);
int32_t subbd_send_CCMV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t ch_offset, int32_t *value, uint32_t ch_lenth);
int32_t subbd_send_CCSV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t ch_offset, int32_t value, uint32_t ch_lenth);
/* if set att and pha at same time ,val_count should be 2, and value[0] is att and value[1] is pha */
int32_t subbd_send_MCMMV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t *channel, int32_t *value, int32_t val_count, uint32_t ch_lenth);
int32_t subbd_send_CCMMV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t ch_offset, int32_t *value, int32_t val_count, uint32_t ch_lenth);
#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
