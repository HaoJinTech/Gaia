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
#define DATA_TYPE_CCMV      3 // continuous channel multi value (offset and lenth)
#define DATA_TYPE_CCSV      4 // continuous channel single value

/* Exported functions --------------------------------------------------------*/ 
int32_t subbd_send_FLEX(char dset, char protocol_id, char port_id, char msg_type, void *data);
int32_t subbd_send_SCSV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t channel, int32_t value);
int32_t subbd_send_MCMV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t *channel, int32_t *value, uint32_t lenth);
int32_t subbd_send_CCMV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t ch_offset, int32_t *value, uint32_t lenth);
int32_t subbd_send_CCSV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t ch_offset, int32_t value, uint32_t lenth);

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
