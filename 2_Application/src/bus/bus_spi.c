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
#include "bus_prototype.h"
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
/* Private typedef -----------------------------------------------------------*/
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
  return 0;
}

void *bus_spi_read(int len)
{
  return 0;
}

int32_t bus_spi_ioctrl(CTRL_MSG *msg)
{
  return RET_OK;
}

int32_t bus_spi_close(void *param)
{
  return RET_OK;
}

#define BUS_SPI   {BUS_ID_SPI   \
                  bus_spi_init  \
                  bus_spi_open  \
                  bus_spi_write \
                  bus_spi_read  \
                  bus_spi_ioctrl\
                  bus_spi_close}
