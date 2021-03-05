/**
  ******************************************************************************
  * @file    RR485.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   legacy radio rack rs-485 protocol , for legacy PHA/ATT matrix radio rack system
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "subbd_protocol.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
BUS_DRIVER *init_bus = 0;  // used for subboard init, usually is rs232

/* Private function prototypes -----------------------------------------------*/
LOCAL int32_t radio_rack_485_init(void *param);
LOCAL int32_t radio_rack_485_open(void *param);
LOCAL int32_t radio_rack_485_write(BUS_DRIVER *bus, void *data);
LOCAL void    *radio_rack_485_read(int len);
LOCAL int32_t radio_rack_485_ioctrl(PROTOCL_CTRL_MSG *ctrl);
LOCAL int32_t radio_rack_485_close(void *param);

/* Private functions ----------------------------------------------------------*/
LOCAL char *make_new_buf(long dest_type, uint32_t *ch, uint32_t *val, uint32_t ch_num, uint32_t val_num, uint32_t *out_len)
{
  char *buf = 0;
  *out_len = sizeof(char) * (ch_num + ch_num * val_num ) + 3;

  buf = (char*)malloc(out_len);

  if(dest_type == DEST_ATT){

  }else if(dest_type == DEST_PHA){

  }else if(dest_type == DEST_ATT_PHA){

  }else{
    return 0;
  }

  uint32_t buf_i=1;
  uint32_t i =0;
  uint32_t j =0;
  for(i=0; i<ch_num; i++){
    // channel;
    buf[buf_i] = ((ch[i] >> 7) | 0x7f);
    buf_i++;
    buf[buf_i] = (ch[i] | 0x7f);
    buf_i++;
    // value;
    for(j=0; j<val_num; j++){
      buf[buf_i] = ((val[i*val_num + j] >> 7) | 0x7f);
      buf_i++;
      buf[buf_i] = (val[i*val_num + j] | 0x7f);
      buf_i++;
    }
  }
  buf[buf_i] = 255;
  buf_i++;
  buf[buf_i] = 255;
  buf_i++;

  return buf;
}

LOCAL int32_t radio_rack_485_init(void *param)
{
  init_bus = (BUS_DRIVER*) param;

  return RET_OK;
}

LOCAL int32_t radio_rack_485_open(void *param)
{
  if(!init_bus)
    return RET_ERROR;


  return RET_OK;
}

LOCAL int32_t radio_rack_485_write(BUS_DRIVER *bus, void *data)
{
  long data_type = *data;
  char *buf = 0;

  switch(data_type){
    case DATA_TYPE_FLEX:

      break;
    case DATA_TYPE_SCSV:
      uint32_t ch = data->channel;
      uint32_t val = data->value;
      SCSV *scsv = (SCSV*)data;
      buf = make_new_buf(scsv->dest_type, &(data->channel), &(data->value), 1, 1);
      bus->write(buf);


      break;
    case DATA_TYPE_MCMV:
      break;
    case DATA_TYPE_CCMV:
      break;
    case DATA_TYPE_CCSV:
      break;
    case DATA_TYPE_CCMMV:
      break;
    default:
      break;
  }

  free(data);

  return RET_OK;
}

LOCAL void *radio_rack_485_read(int len)
{
  return 0;
}

LOCAL int32_t radio_rack_485_ioctrl(PROTOCL_CTRL_MSG *ctrl)
{
  return 0;
}

LOCAL int32_t radio_rack_485_close(void *param)
{
  return 0;
}

/* Public functions ----------------------------------------------------------*/
#define RR485 {PROTOCOL_ID_RR485, \
        radio_rack_485_init, \
        radio_rack_485_open, \
        radio_rack_485_write, \
        radio_rack_485_read, \
        radio_rack_485_ioctrl,\
        radio_rack_485_close}

