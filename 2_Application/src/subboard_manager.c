/**
  ******************************************************************************
  * @file    rfboard_manager.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-03-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_config.h"
#include "app_debug.h"
#include "bus_prototype.h"
#include "subbd_protocol.h"

#include <pthread.h>
#include <sys/msg.h>
#include <sys/ipc.h>

/* Private typedef -----------------------------------------------------------*/
// msg(channel value)-> protocal ->data(0x51 0x01 ...) ->port(spi,uart...)

typedef struct subbd_msg{
    long        msg_type;

    char        protocol_type;  // 0: legacy 485/232 protocal. 1: high speed new protocal.
    char        port_id;        // 0: spi0. 1: uart0 ... device type with function?

    void        *data;          // check subbd_protocol.h for data type
} SUBBD_MSG;

/* Private define ------------------------------------------------------------*/
#define RF_DEBUG       APP_DBG_ON

#define SBBD_MSG_TYPE        0x0020

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL int msq_id = -1;

/* extern variables */
extern BUS_DRIVER bus_drivers[];
extern uint32_t BUS_DRIVER_NUM;

extern SUBBD_PROTOCOL protocols[];
extern uint32_t SUBBD_PROTOCOL_SIZE;

/* Private function prototypes -----------------------------------------------*/
LOCAL void *rfboard_manager(void *param);
LOCAL int32_t subbd_send_data(char protocol_id, char port_id, char msg_type, void *data);

/* Public functions ----------------------------------------------------------*/

int32_t init_rfboard_manager(void)
{
    pthread_t	tid;

	pthread_create(&tid, NULL, rfboard_manager, NULL);

    return RET_OK;
}

LOCAL void *rfboard_manager(void *param)
{
    SUBBD_MSG msg;
    ssize_t size= 0;
    msq_id =  msgget( IPC_PRIVATE, 0666 ) ;
	if(msq_id == -1){
		APP_DEBUGF(RF_DEBUG | APP_DBG_LEVEL_SERIOUS , ("msgget failed (%d).\r\n",msq_id));
		return 0;
    }

    while(1){
        size=msgrcv(msq_id, (void *)&msg,sizeof(SUBBD_MSG)-sizeof(long),SBBD_MSG_TYPE,0);
        if(size < 0){
            APP_DEBUGF(RF_DEBUG | APP_DBG_LEVEL_SERIOUS , ("msgrcv failed (%d).\r\n", size));
        }
    }
}

LOCAL int32_t subbd_send_data(char protocol_id, char port_id, char msg_type, void *data)
{
    
    return RET_OK;
}

