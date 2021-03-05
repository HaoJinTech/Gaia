/**
  ******************************************************************************
  * @file    bll_manager.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "bll/bll_att.h"
#include "bll/bll_pha.h"
#include "bll/bll_calibration.h"

#include "platform.h"
#include "sys_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
LOCAL int32_t bll_assembly(void);
/* Private functions ----------------------------------------------------------*/

LOCAL int32_t bll_assembly(void)
{
    init_att(pobj);
    init_pha(pobj);
    init_calibration(pobj);

    return RET_OK;
}
/* Public functions ----------------------------------------------------------*/

int32_t init_bll_manager(void)
{
    return bll_assembly();
}
