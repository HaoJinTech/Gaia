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

#include "subboard_protocol/RR485.c"
// TODO: include new protocol src code here.

SUBBD_PROTOCOL protocols[] =
{
    RR485,
    // TODO: add new protocol declear here.
};

uint32_t SUBBD_PROTOCOL_SIZE = sizeof(protocols)/ sizeof(SUBBD_PROTOCOL);

