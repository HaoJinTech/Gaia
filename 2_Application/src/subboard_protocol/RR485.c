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

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <errno.h>

/* Private typedef -----------------------------------------------------------*/
/* protocol cmds **************************************************************/
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
236 | INDEXH | INDEXL | CHH | CHL | VALH | VALL | ... | 0xFF | 0xFF */
#define CMD_TYPE_ATT_LOAD				234

/*
used for load all pha values from main board , than will be saved into flash.
0   | 1      |2     | 3   | 4   | 5    | 6    | ... | n-1  | n
235 | INDEXH |INDEXL| CHH | CHL | VALH | VALL | ... | 0xFF | 0xFF */
#define CMD_TYPE_PHA_LOAD				233

#define CMD_TYPE_UPDATE_VAL     232

/*
used for load all the channels with both pha and att.
|0   | 1    | 2    | 3    | ... | n-1  | n    |
|230 | VALH | VALM | VALL | ... | 0xFF | 0xFF |
-----------------------------------------------------
| N|     ATT     |      PHA      |
| 0 111 1111 1111 1111 1111 1111 |
|   VALH    |  VALM   |   VALL   | 
ATT = ((VALH & 0x7f) << 4) | ((VALH & 0xf0) >> 4);
PHA = ((VALM & 0xf) << 8) | VALL
*/
#define CMD_TYPE_ATT_PHA_LOAD_EX  230

/******************************************************************************/

#define START_LOAD_WAIT_TIME_S  3  
#define RF_OPEN_RX_TIMEOUT_S    2
#define RF_DEV_OPEN_RETRY_TIME  3

/* Private define ------------------------------------------------------------*/
#define RR485_DEBUG   APP_DBG_OFF

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
typedef struct rr485_data{
  BUS_DRIVER *init_bus;  // used for subboard init, usually is rs232
  uint32_t    s_board_num;
  uint32_t    s_board_ch;
}RR485_DATA;

LOCAL const char init_subboard_msg[] = {CMD_TYPE_INIT_EX, 0, 0, 0, 0, 255, 255};
LOCAL const char start_load_msg[] = {CMD_TYPE_START_LOAD, 255, 255};

/* Private function prototypes -----------------------------------------------*/
LOCAL int32_t radio_rack_485_init(SUBBD_PROTOCOL *devs,void *param);
LOCAL int32_t radio_rack_485_open(SUBBD_PROTOCOL *devs,void *param);
LOCAL int32_t radio_rack_485_write(SUBBD_PROTOCOL *devs,BUS_DRIVER *bus, void *data);
LOCAL void   *radio_rack_485_read(SUBBD_PROTOCOL *devs,BUS_DRIVER *bus, int len);
LOCAL int32_t radio_rack_485_ioctrl(SUBBD_PROTOCOL *devs,int request, ...);
LOCAL int32_t radio_rack_485_close(SUBBD_PROTOCOL *devs,void *param);

LOCAL char *make_new_buf(long dest_type, int32_t *ch, int32_t *val, uint32_t ch_num, uint32_t val_num, uint32_t *out_len);

/* Private functions ----------------------------------------------------------*/
LOCAL char *make_new_buf(long dest_type, int32_t *ch, int32_t *val, uint32_t ch_num, uint32_t val_num, uint32_t *out_len)
{
  char *buf = 0;
  uint32_t len = 0;
  len = sizeof(char) * ((ch_num + ch_num * val_num ) * 2 + 3 + 3);
  *out_len = len;
  buf = (char*)malloc(len);
  uint32_t buf_i=1;

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
    case DEST_UPLD_ATT:
      buf[0] = CMD_TYPE_ATT_LOAD;
    break;
    case DEST_UPLD_PHA:
      buf[0] = CMD_TYPE_PHA_LOAD;
    break;
  }

  uint32_t i =0;
  uint32_t j =0;
  int32_t ch_temp = 0;
  for(i=0; i<ch_num; i++){
    ch_temp = ch[i]+1; // start at 1, not 0
    // channel;
    buf[buf_i] = ((ch_temp >> 7) & 0x7f);
    buf_i++;
    buf[buf_i] = (ch_temp & 0x7f);
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
  *out_len = buf_i;

  return buf;
}
//该方法只给UPLD_ATT_PHA_EX使用
LOCAL char *make_new_ex_buf(long dest_type, int32_t *val, uint32_t ch_num, uint32_t val_num, uint32_t *out_len)
{
  //该方法只给UPLD_ATT_PHA_EX使用
  if(dest_type != DEST_UPLD_ATT_PHA_EX){
    return NULL;
  }
  //初始化值，进行内存分配
  char *buf = 0;
  uint32_t len = 0;
  len = sizeof(char) * (ch_num * 3  + 3);
  *out_len = len;
  buf = (char*)malloc(len);
  
  buf[0] = CMD_TYPE_ATT_PHA_LOAD_EX;
  //逐index获取值并解码
  uint32_t buf_i=1;
  uint32_t i =0;
  /*
used for load all the channels with both pha and att.
|0   | 1    | 2    | 3    | ... | n-1  | n    |
|230 | VALH | VALM | VALL | ... | 0xFF | 0xFF |
-----------------------------------------------------
| N|     ATT     |      PHA      |
| 0 111 1111 1111 1111 1111 1111 |
|   VALH    |  VALM   |   VALL   | 
ATT = ((VALH & 0x7f) << 4) | ((VALH & 0xf0) >> 4);
PHA = ((VALM & 0xf) << 8) | VALL
*/
  for(i=0; i<ch_num; i++){
    buf[buf_i] = ((val[i * 2] >> 4) & 0x7f);
    buf_i++;
    buf[buf_i] = (val[i * 2] & 0xf) << 4;
    buf[buf_i] |= (val[i * 2 + 1] >> 8) &0Xf;
    buf_i++;
    buf[buf_i] = (val[i * 2 + 1] & 0xff);
    buf_i++;
  }
  buf[buf_i] = 255;
  buf_i++;
  buf[buf_i] = 255;
  buf_i++;
  *out_len = buf_i;
  return buf;
}

LOCAL int32_t radio_rack_485_init(SUBBD_PROTOCOL *devs, void *param)
{
  devs->data = malloc(sizeof(RR485_DATA));
  if(!devs->data)
    return RET_ERROR;
  RR485_DATA *data = (RR485_DATA*)devs->data;
  data->init_bus = (BUS_DRIVER*)param;
  if(!data->init_bus)
    return RET_ERROR;

  return RET_OK;
}

LOCAL int32_t radio_rack_485_open(SUBBD_PROTOCOL *devs, void *param)
{
  RR485_DATA *data = (RR485_DATA*)devs->data;
  BUS_DRIVER *init_bus = data->init_bus;
  if(!init_bus)
    return RET_ERROR;

  int32_t open_retry_times=0;
  int32_t read_size;
  int err;

open_retry:
  data->s_board_ch = 0;
  data->s_board_num = 0;
  init_bus->write(init_subboard_msg, sizeof(init_subboard_msg));

  struct timespec ts;
  char *rxbuf = malloc(sizeof(char) * 8);
  do{
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        APP_DEBUGF(RR485_DEBUG | APP_DBG_LEVEL_WARNING, ("clock_gettime error.\r\n"));
    ts.tv_sec += RF_OPEN_RX_TIMEOUT_S;
    if(init_bus->sem_rx_ready){
      err = sem_timedwait(init_bus->sem_rx_ready, &ts);
      if(err == ETIMEDOUT) break;
      else if(err != 0) {
        APP_DEBUGF(RR485_DEBUG | APP_DBG_LEVEL_WARNING, ("sem_timedwait error.\r\n"));
        break;
      }
    }else{
      usleep(100000);
    }
    read_size = init_bus->read(rxbuf, 8);
    if(read_size == 6 && rxbuf[0] == 253){ //CMD_TYPE_INIT
      if(data->s_board_num < rxbuf[1]+1){
        data->s_board_num = rxbuf[1]+1;
        data->s_board_ch = rxbuf[2]+data->s_board_ch;
      }
      APP_DEBUGF(RR485_DEBUG | APP_DBG_TRACE, 
        ("get rf info: board_num:%d,board_ch:%d,board_step:%d\r\n",rxbuf[1]+1,rxbuf[2],rxbuf[3]));
    }else{
      break;
    }
  }while(1);

  if(data->s_board_ch == 0){
    APP_DEBUGF(RR485_DEBUG | APP_DBG_STATE, 
    ("get rf board init info failed,try again.\r\n"));
    if(open_retry_times<RF_DEV_OPEN_RETRY_TIME){
      init_bus->close(NULL);   
      open_retry_times++;
      goto open_retry;
    }
  }
  free(rxbuf);
  APP_DEBUGF(RR485_DEBUG | APP_DBG_STATE, 
    ("s_board_num = %d,s_board_ch = %d.\r\n", data->s_board_num, data->s_board_ch));
  return RET_OK;
}

LOCAL int32_t radio_rack_485_write(SUBBD_PROTOCOL *devs,BUS_DRIVER *bus, void *data)
{
  long data_type = *(long*)data;
  char *buf = 0;
  uint32_t out_len;

  APP_ASSERT("bus == NULL", bus);
  APP_ASSERT("data == NULL", data);
  
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
      free(mcmv->channel);
      free(mcmv->value);
      break;
    }
    case DATA_TYPE_MCMMV:{
      MCMMV *mcmmv = (MCMMV*)data;
      buf = make_new_buf(mcmmv->dest_type, mcmmv->channel, mcmmv->value, mcmmv->ch_lenth, mcmmv->val_count, &out_len);
      bus->write(buf, out_len);
      free(mcmmv->channel);
      free(mcmmv->value);
      break;
    }
    case DATA_TYPE_CCMV:
      break;
    case DATA_TYPE_CCSV:
      break;
    case DATA_TYPE_CCMMV:{
      CCMMV *ccmmv = (CCMMV*)data;
      if(ccmmv->dest_type == DEST_UPLD_ATT_PHA_EX){
        buf = make_new_ex_buf(ccmmv->dest_type, ccmmv->value, ccmmv->ch_lenth, ccmmv->val_count, &out_len);
        bus->write(buf, out_len);
        free(ccmmv->value);
      }else{                                    
      }
      break;
    }
    default:
      break;
  }

  if(buf)
    free(buf);
  if(data)
    free(data);

  return RET_OK;
}

LOCAL void *radio_rack_485_read(SUBBD_PROTOCOL *devs, BUS_DRIVER *bus, int len)
{
  APP_ASSERT("bus == NULL", bus);

  return 0;
}

LOCAL int32_t radio_rack_485_ioctrl(SUBBD_PROTOCOL *devs, int request, ...)
{
  BUS_DRIVER *bus;
  va_list arg_ptr;

  va_start(arg_ptr, request);

  switch (request){
    case IO_CTRL_MSG_START_CASE_UPLOAD:
      bus = va_arg(arg_ptr, BUS_DRIVER*);
      if(bus){
        bus->write(start_load_msg, sizeof(start_load_msg));
        sleep(START_LOAD_WAIT_TIME_S); // thread sleep for 3s to wait for the sub board get ready.
      }
      break;
    case IO_CTRL_MSG_UPDATE_CASE_LINE:{
        uint32_t line_num;
        bus = va_arg(arg_ptr, BUS_DRIVER*);
        if(bus){
          line_num = va_arg(arg_ptr, uint32_t);
          char line_update_msg[] = {CMD_TYPE_UPDATE_VAL, ((line_num >> 7) & 0x7f), (line_num & 0x7f), 255, 255};
        	APP_DEBUGF_HEX(RR485_DEBUG | APP_DBG_TRACE, line_update_msg, 5);
          bus->write(line_update_msg, sizeof(line_update_msg));
        }
      }
      break;
    default:
      break;
  }
  va_end(arg_ptr);
  return RET_OK;
}

LOCAL int32_t radio_rack_485_close(SUBBD_PROTOCOL *devs, void *param)
{
  return 0;
}

/* Public functions ----------------------------------------------------------*/
#define RR485 {PROTOCOL_ID_RR485,   \
              NULL,NULL,\
              radio_rack_485_init,  \
              radio_rack_485_open,  \
              radio_rack_485_write, \
              radio_rack_485_read,  \
              radio_rack_485_ioctrl,\
              radio_rack_485_close}

