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

int32_t radio_rack_485_init(void *param)
{
  return RET_OK;
}

int32_t radio_rack_485_open(void *param)
{
  return RET_OK;
}

int32_t radio_rack_485_write(void *data)
{
  return RET_OK;
}

void *radio_rack_485_read(int len)
{
  return 0;
}


int32_t radio_rack_485_ioctrl(PROTOCL_CTRL_MSG *ctrl)
{
  return 0;
}

int32_t radio_rack_485_close(void *param)
{
  return 0;
}

#define RR485 {PROTOCOL_ID_RR485, \
        radio_rack_485_init, \
        radio_rack_485_open, \
        radio_rack_485_write, \
        radio_rack_485_read, \
        radio_rack_485_ioctrl,\
        radio_rack_485_close}

