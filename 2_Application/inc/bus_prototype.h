/**
  ******************************************************************************
  * @file    bus_prototype.h
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  _BUS_PROTOTYPE_H_
#define  _BUS_PROTOTYPE_H_
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
/* Exported types ------------------------------------------------------------*/
typedef struct bus_ctrl_msg
{
    uint32_t  type;
    void      *val;
}BUS_CTRL_MSG;

typedef int32_t (*bus_init)(uint32_t port, uint32_t freq, void *other);
typedef int32_t (*bus_open)(void);
typedef int32_t (*bus_write)(char *data, uint32_t len);
typedef void *(*bus_read)(int len);
typedef int32_t (*bus_ioctrl)(BUS_CTRL_MSG *msg);
typedef int32_t (*bus_close)(void *param);

typedef struct bus_driver
{
    int     bus_id;

    bus_init   init;
    bus_open   open;
    bus_write  write;
    bus_read   read;
    bus_ioctrl ioctrl;
    bus_close  close;

}BUS_DRIVER;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define BUS_ID_SPI      2
#define BUS_ID_UART     3

/* Exported functions --------------------------------------------------------*/ 


#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
