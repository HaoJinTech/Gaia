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
#include "app_debug.h"
#include "subbd_protocol.h"
#include "subboard_manager.h"

#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* command list */
/*
0   | 1   |2		| 3         | 4 				 | 5   | 6
253 | IDH |IDL	| CHOFFSETH | CHOFFSETL  | 255 | 255
*/
#define CMD_TYPE_INIT_EX_SIZE  8
#define CMD_TYPE_INIT_EX       248

/*
0   | 1  | 2       | 3   | 4
253 | ID | CHOFFSET| 255 | 255
*/
#define CMD_TYPE_INIT_SIZE  5
#define CMD_TYPE_INIT       253

/*
0   | 1   | 2   | 3    | 4    | ...| n-1  | n 
252 | CHH | CHL | VALH | VALL | ...| 0xFF | 0xFF
*/
#define CMD_TYPE_SET				252

/*
0   | 1    | 2    | 3     | 4     | 5    | 6    | 7    | 8
251 | POSH | POSL | SIZEH | SIZEL | VALH | VALL | 0xFF | 0xFF
*/
#define CMD_TYPE_MSET_SIZE	9
#define CMD_TYPE_MSET				251

/*
0   | 1   | 2   | 3    | 4    | ...| n-1  | n 
250 | CHH | CHL | VALH | VALL | ...| 0xFF | 0xFF
*/
#define CMD_TYPE_SET_PHA				250

/*
0   | 1   | 2   | 3    | 4    | 5     | 6     ... | n-1  | n 
249 | CHH | CHL | VALH | VALL | VALH2 | VALL2 ... | 0xFF | 0xFF
*/
#define CMD_TYPE_SET_PHA_ATT		249

#define CMD_TYPE_CAL						248
/*
0   | 1   | 2   | 3    | 4    | ...| n-1  | n 
248 | CHH | CHL | VALH | VALL | ...| 0xFF | 0xFF
*/
#define CMD_TYPE_CAL_SAV				247

#define CMD_TYPE_MOTOR_SET			242

#define CMD_TYPE_START_LOAD			235
/*
used for load all att values from main board , than will be saved into flash.
0   | 1      | 2      | 3   | 4   | 5    | 6    | ... | n-1  | n
236 | INDEXH | INDEXL | CHH | CHL | VALH | VALL | ... | 0xFF | 0xFF
*/
#define CMD_TYPE_ATT_LOAD				234

/*
used for load all pha values from main board , than will be saved into flash.
0   | 1      |2     | 3   | 4   | 5    | 6    | ... | n-1  | n
235 | INDEXH |INDEXL| CHH | CHL | VALH | VALL | ... | 0xFF | 0xFF
*/
#define CMD_TYPE_PHA_LOAD				233

#define CMD_TYPE_UPDATE_VAL     232
/* Private define ------------------------------------------------------------*/
#define RR485_DEBUG   APP_DBG_ON

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
BUS_DRIVER *init_bus = 0;  // used for subboard init, usually is rs232

/* Private function prototypes -----------------------------------------------*/
LOCAL int32_t radio_rack_485_init(void *param);
LOCAL int32_t radio_rack_485_open(void *param);
LOCAL int32_t radio_rack_485_write(BUS_DRIVER *bus, void *data);
LOCAL void    *radio_rack_485_read(BUS_DRIVER *bus, int len);
LOCAL int32_t radio_rack_485_ioctrl(PROTOCL_CTRL_MSG *ctrl);
LOCAL int32_t radio_rack_485_close(void *param);
LOCAL char *make_new_buf(long dest_type, int32_t *ch, int32_t *val, uint32_t ch_num, uint32_t val_num, uint32_t *out_len);

/* Private functions ----------------------------------------------------------*/
LOCAL char *make_new_buf(long dest_type, int32_t *ch, int32_t *val, uint32_t ch_num, uint32_t val_num, uint32_t *out_len)
{
  char *buf = 0;
  uint32_t len = 0;
  len = sizeof(char) * ((ch_num + ch_num * val_num ) * 2 + 3);
  *out_len = len;
  buf = (char*)malloc(len);
  switch(dest_type){
    case DEST_ATT:
      buf[0] = CMD_TYPE_SET;
    break;
    case DEST_PHA:
      buf[0] = CMD_TYPE_SET_PHA;
    break;
    case DEST_ATT_PHA:
      buf[0] = CMD_TYPE_SET_PHA_ATT;
    break;
  }

  uint32_t buf_i=1;
  uint32_t i =0;
  uint32_t j =0;
  for(i=0; i<ch_num; i++){
    // channel;
    buf[buf_i] = ((ch[i] >> 7) & 0x7f);
    buf_i++;
    buf[buf_i] = (ch[i] & 0x7f);
    buf_i++;
    // value;
    for(j=0; j<val_num; j++){
      buf[buf_i] = ((val[i*val_num + j] >> 7) & 0x7f);
      buf_i++;
      buf[buf_i] = (val[i*val_num + j] & 0x7f);
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
  long data_type = *(long*)data;
  char *buf = 0;
  uint32_t out_len;
  
	APP_DEBUGF(RR485_DEBUG | APP_DBG_TRACE, ("write data(type:%ld).\r\n", data_type));
	
  switch(data_type){
    case DATA_TYPE_FLEX:

      break;
    case DATA_TYPE_SCSV:{
      SCSV *scsv = (SCSV*)data;
      buf = make_new_buf(scsv->dest_type, &(scsv->channel), &(scsv->value), 1, 1, &out_len);
      bus->write(buf, out_len);
      break;
    }
    case DATA_TYPE_MCMV:{
      MCMV *mcmv = (MCMV*)data;
      buf = make_new_buf(mcmv->dest_type, mcmv->channel, mcmv->value, mcmv->ch_lenth, 1, &out_len);
      bus->write(buf, out_len);
      break;
    }
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

LOCAL void *radio_rack_485_read(BUS_DRIVER *bus, int len)
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

