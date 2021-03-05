/**
  ******************************************************************************
  * @file    combo_cmd.c
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "bus/bus_spi.c"
// TODO: include new bus src here

BUS_DRIVER bus_drivers[]={
    BUS_SPI,
    // TODO: add new bus driver here
};

uint32_t BUS_DRIVER_NUM = sizeof(bus_drivers) / sizeof(BUS_DRIVER);
