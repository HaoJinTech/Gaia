/**
  ******************************************************************************
  * @file    bll_case_runner.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "bll_case_runner.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void init_model_case_runner(void)
{
  APP_DEBUGF(USER_DEBUG | APP_DBG_TRACE, 
  ("init_model_case_runner: initialize matrix switch model.\r\n"));

  get_case_runner_conf();
  
#if (defined __CaseRunner_Triger__) && (!defined STPM)
  Triger_GPIO_Init();
#endif
  init_runner_thread();
  
  rt_list_init(&case_item_root);

  regist_cmd_item_list(case_runner_cmd_items);
}
