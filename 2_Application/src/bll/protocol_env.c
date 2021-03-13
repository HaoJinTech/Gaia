/**
  ******************************************************************************
  * @file    protocol_env.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "subboard_manager.h"
#include "sys_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL SUBBD_PROTOCOL  *g_protocol_obj = 0;
LOCAL BUS_DRIVER      *g_bus_obj = 0;

/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
LOCAL int32_t init_protocol_env(void)
{
    int protocol_id = 0;
    int bus_id = 0;

    g_protocol_obj = &protocols[protocol_id];
    g_bus_obj =     &bus_drivers[bus_id];

    g_protocol_obj->init(g_protocol_obj, g_bus_obj);
    g_protocol_obj->open(g_protocol_obj, NULL);

    g_bus_obj->init(0,0,0);
    g_bus_obj->sem_rx_ready = 0;
    return RET_OK;
}

SUBBD_PROTOCOL  *get_protocol_obj(uint32_t index)
{
  if(!g_protocol_obj){
    init_protocol_env();
  }

  return g_protocol_obj;
}

BUS_DRIVER  *get_bus_obj(uint32_t index)
{
  if(!g_bus_obj){
    init_protocol_env();
  }

  return g_bus_obj;
}
