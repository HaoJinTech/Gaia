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
#include <semaphore.h>

/* Exported types ------------------------------------------------------------*/
typedef struct bus_ctrl_msg
{
    uint32_t  type;
    void      *val;
}BUS_CTRL_MSG;

typedef int32_t (*bus_init)(uint32_t port, uint32_t freq, void *res);
typedef int32_t (*bus_open)(void);
typedef int32_t (*bus_write)(const char *data, uint32_t len);
typedef int32_t (*bus_read)(char *data, uint32_t len);
typedef int32_t (*bus_ioctrl)(BUS_CTRL_MSG *msg);
typedef int32_t (*bus_close)(void *param);

typedef struct bus_driver
{
    int     bus_id;

    sem_t      *sem_rx_ready;
    bus_init   init;
    bus_open   open;
    bus_write  write;
    bus_read   read;
    bus_ioctrl ioctrl;
    bus_close  close;

}BUS_DRIVER;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/ 
extern BUS_DRIVER bus_drivers[];
extern uint32_t BUS_DRIVER_NUM;

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
