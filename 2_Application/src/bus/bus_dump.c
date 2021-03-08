/**
  ******************************************************************************
  * @file    bus_dump.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "bus_prototype.h"
#include "app_debug.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BUS_DUMP_DEBUG        APP_DBG_ON

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
int32_t bus_dump_init(uint32_t port, uint32_t freq, void *res)
{
	APP_DEBUGF(BUS_DUMP_DEBUG | APP_DBG_TRACE, ("init, port:%d, freq:%d, other:0x%x.\r\n", port, freq, res));
    return RET_OK;
}

int32_t bus_dump_open(void)
{
	APP_DEBUGF(BUS_DUMP_DEBUG | APP_DBG_TRACE, ("open.\r\n"));
    return RET_OK;
}

int32_t bus_dump_write(char *data, uint32_t len)
{
	APP_DEBUGF(BUS_DUMP_DEBUG | APP_DBG_TRACE, ("write data:0x%x, len:%d.\r\n",data,len));
    APP_DEBUGF_HEX(BUS_DUMP_DEBUG | APP_DBG_TRACE, data, len);
    return len;
}

void   *bus_dump_read(char *data, int len)
{
	APP_DEBUGF(BUS_DUMP_DEBUG | APP_DBG_TRACE, ("read data:0x%x, len:%d.\r\n",data,len));
    return NULL;
}

int32_t bus_dump_ioctrl(BUS_CTRL_MSG *msg)
{
	APP_DEBUGF(BUS_DUMP_DEBUG | APP_DBG_TRACE, ("ioctrl msg:0x%x.\r\n",msg));
    return RET_OK;
}

int32_t bus_dump_close(void *param)
{
    APP_DEBUGF(BUS_DUMP_DEBUG | APP_DBG_TRACE, ("close.\r\n"));
    return RET_OK;
}

#define BUS_DUMP   {BUS_ID_DUMP,   \
                   bus_dump_init,  \
                   bus_dump_open,  \
                   bus_dump_write, \
                   bus_dump_read,  \
                   bus_dump_ioctrl,\
                   bus_dump_close}
