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
#include "bll/bll_ch_remap.h"
#include "bll/bll_case_manager.h"

#include "platform.h"
#include "sys_config.h"

/* Private typedef -----------------------------------------------------------*/
typedef int32_t (*INIT_OBJ)(json_object *json_obj);

typedef struct bll_objects{
    char      *obj_name;
    INIT_OBJ   init_fun;
}BLL_OBJ;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
BLL_OBJ bll_objs[] ={
    {"ATT", init_att},
    {"PHA", init_pha},
    {"CALIBRATION", init_calibration},
    {"CHREMAP", init_ch_remap},
    {"CASE", init_model_case_manager}
};
uint32_t BLL_OBJ_SIZE = sizeof(bll_objs) / sizeof(BLL_OBJ);

/* Private function prototypes -----------------------------------------------*/
LOCAL int32_t bll_assembly(void);
/* Private functions ----------------------------------------------------------*/

LOCAL int32_t bll_assembly(void)
{
    int i=0;
    json_object *obj = 0;

    for(i=0; i< BLL_OBJ_SIZE; i++){
        obj = json_object_object_get(config_json_obj, bll_objs[i].obj_name);
        if(obj) {
            bll_objs[i].init_fun(obj);            
        }
    }

    return RET_OK;
}
/* Public functions ----------------------------------------------------------*/

int32_t init_bll_manager(void)
{
    return bll_assembly();
}
