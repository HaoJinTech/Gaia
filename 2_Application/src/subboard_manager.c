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
#include "subboard_manager.h"

#include <pthread.h>
#include <sys/msg.h>
#include <sys/ipc.h>

/* Private typedef -----------------------------------------------------------*/
// msg(channel value)-> protocal ->data(0x51 0x01 ...) ->port(spi,uart...)
typedef struct subbd_msg{
    long        msg_type;

    SUBBD_PROTOCOL *protocol_obj;    // PROTOCOL_ID_RR485, PROTOCOL_ID_HSSPI ...
    BUS_DRIVER     *bus_obj;         // BUS_ID_SPI, BUS_ID_UART ...

    void        *data;          // check subbd_protocol.h for data type
} SUBBD_MSG;

/* Private define ------------------------------------------------------------*/
#define SBBD_DEBUG       APP_DBG_ON

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
LOCAL int32_t subbd_send_data(SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, void *data);
LOCAL void do_protocal(SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, void *data);

/* Private functions ----------------------------------------------------------*/
LOCAL void *rfboard_manager(void *param)
{
    SUBBD_MSG msg;
    ssize_t size= 0;
    msq_id =  msgget( IPC_PRIVATE, 0666 ) ;
	if(msq_id == -1){
		APP_DEBUGF(SBBD_DEBUG | APP_DBG_LEVEL_SERIOUS , ("msgget failed (%d).\r\n",msq_id));
		return 0;
    }
	APP_DEBUGF(SBBD_DEBUG | APP_DBG_TRACE , ("start msgrcv ...\r\n"));

    while(1){
        size=msgrcv(msq_id, (void *)&msg,sizeof(SUBBD_MSG)-sizeof(long),SBBD_MSG_TYPE,0);
        if(size < 0){
            APP_DEBUGF(SBBD_DEBUG | APP_DBG_LEVEL_SERIOUS , ("msgrcv failed (%d).\r\n", size));
        }

        do_protocal(msg.protocol_obj, msg.bus_obj, msg.data);
    }
}

LOCAL void do_protocal(SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, void *data)
{
	APP_DEBUGF(SBBD_DEBUG | APP_DBG_TRACE , ("send to subboard (protocol:%d)(bus:%d).\r\n", 
        protocol_obj->protocol_id, bus_obj->bus_id));

    protocol_obj->write(protocol_obj, bus_obj, data);
}

LOCAL int32_t subbd_send_data(SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, void *data)
{
    int ret = 0;
    SUBBD_MSG msg;
    msg.msg_type = SBBD_MSG_TYPE;
    msg.bus_obj = bus_obj;
    msg.protocol_obj = protocol_obj;
    msg.data = data;

    ret = msgsnd(msq_id, (void *)&msg, sizeof(SUBBD_MSG)-sizeof(long), 0);
    if(ret<0){
        APP_DEBUGF(SBBD_DEBUG | APP_DBG_LEVEL_SERIOUS, ("[send_cmd_msg] msgsnd failed (%d).\r\n",ret));
        return RET_ERROR;
    }

    return RET_OK;
}

/* Public functions ----------------------------------------------------------*/

int32_t init_rfboard_manager(void)
{
    pthread_t	tid;

	pthread_create(&tid, NULL, rfboard_manager, NULL);

    return RET_OK;
}


int32_t subbd_send_FLEX(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, void *value)
{
    FLEX *data = (FLEX *)malloc(sizeof(FLEX));
    data->data_type = DATA_TYPE_FLEX;
    data->dest_type = dset;

    data->value = value;

    return subbd_send_data(protocol_obj, bus_obj, data);
}

int32_t subbd_send_SCSV(char dset, SUBBD_PROTOCOL *protocol_obj, BUS_DRIVER *bus_obj, int32_t channel, int32_t value)
{
    SCSV *data = (SCSV *)malloc(sizeof(SCSV));
    data->channel = channel;
    data->data_type = DATA_TYPE_SCSV;
    data->dest_type = dset;

    data->value = value;

    return subbd_send_data(protocol_obj, bus_obj, data);
}

int32_t subbd_send_MCMV(char dset, SUBBD_PROTOCOL *protocol_obj, 
    BUS_DRIVER *bus_obj, int32_t *channel, int32_t *value, uint32_t ch_lenth)
{
    MCMV *data = (MCMV *)malloc(sizeof(MCMV));
    data->channel = channel;
    data->data_type = DATA_TYPE_MCMV;
    data->dest_type = dset;

    data->value = value;
    data->ch_lenth = ch_lenth;

    return subbd_send_data(protocol_obj, bus_obj, data);
}

int32_t subbd_send_CCMV(char dset, SUBBD_PROTOCOL *protocol_obj, 
    BUS_DRIVER *bus_obj, int32_t ch_offset, int32_t *value, uint32_t ch_lenth)
{
    CCMV *data = (CCMV *)malloc(sizeof(CCMV));
    data->data_type = DATA_TYPE_CCMV;
    data->dest_type = dset;

    data->offset = ch_offset;
    data->value = value;
    data->ch_lenth = ch_lenth;

    return subbd_send_data(protocol_obj, bus_obj, data);   
}

int32_t subbd_send_CCSV(char dset, SUBBD_PROTOCOL *protocol_obj, 
    BUS_DRIVER *bus_obj, int32_t ch_offset, int32_t value, uint32_t ch_lenth)
{
    CCSV *data = (CCSV *)malloc(sizeof(CCSV));
    data->data_type = DATA_TYPE_CCSV;
    data->dest_type = dset;
    
    data->offset = ch_offset;
    data->value = value;
    data->ch_lenth = ch_lenth;

    return subbd_send_data(protocol_obj, bus_obj, data);   
}

int32_t subbd_send_MCMMV(char dset, SUBBD_PROTOCOL *protocol_obj, 
    BUS_DRIVER *bus_obj, int32_t *channel, int32_t *value, int32_t val_count, uint32_t ch_lenth)
{
    MCMMV *data = (MCMMV *)malloc(sizeof(MCMMV));
    data->data_type = DATA_TYPE_MCMMV;
    data->dest_type = dset;

    data->channel = channel;
    data->ch_lenth = ch_lenth;
    data->value = value;
    data->val_count = val_count;

    return subbd_send_data(protocol_obj, bus_obj, data);   
}

int32_t subbd_send_CCMMV(char dset, SUBBD_PROTOCOL *protocol_obj, 
    BUS_DRIVER *bus_obj, int32_t ch_offset, int32_t *value, int32_t val_count, uint32_t ch_lenth)
{
    CCMMV *data = (CCMMV *)malloc(sizeof(CCMMV));
    data->data_type = DATA_TYPE_CCMMV;
    data->dest_type = dset;

    data->offset = ch_offset;
    data->value = value;
    data->ch_lenth = ch_lenth;
    data->val_count = val_count;

    return subbd_send_data(protocol_obj, bus_obj, data);   
}

