/**
  ******************************************************************************
  * @file    spi_bus.c
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

#include <stdio.h>
#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/
#define SPI_DEBUG  APP_DBG_ON
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

int32_t bus_spi_init(uint32_t port, uint32_t freq, void *other)
{
  return RET_OK;
}

int32_t bus_spi_open(void)
{
  return RET_OK;
}

int32_t bus_spi_write(char *data, uint32_t len)
{
 	APP_DEBUGF(SPI_DEBUG | APP_DBG_TRACE, ("write data lenth: %d\r\n", len));
  APP_DEBUGF_HEX(SPI_DEBUG | APP_DBG_TRACE, data, len);
  return 0;
}

void *bus_spi_read(int len)
{
  return 0;
}

int32_t bus_spi_ioctrl(BUS_CTRL_MSG *msg)
{
  return RET_OK;
}

int32_t bus_spi_close(void *param)
{
  return RET_OK;
}

#define BUS_SPI   {BUS_ID_SPI,   \
                  bus_spi_init,  \
                  bus_spi_open,  \
                  bus_spi_write, \
                  bus_spi_read,  \
                  bus_spi_ioctrl,\
                  bus_spi_close}
