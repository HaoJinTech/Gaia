/**
  ******************************************************************************
  * @file    bll_case_manager.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
    ********** Copyright (C), 2014-2015,HJ technologies **************************
    */
    
/* Includes ------------------------------------------------------------------*/
#include "bll_case_manager.h"
#include "platform.h"
#include "app_debug.h"
#include "list.h"

#include <time.h>
#include <signal.h>  
#include <semaphore.h>

/* Private typedef -----------------------------------------------------------*/
typedef enum Case_state{
    CASE_STATE_UNLOADED     =0x0000,
    CASE_STATE_STOP         =0x0001,
    CASE_STATE_RUN          =0x0002,
    CASE_STATE_PAUSE        =0x0003,
    CASE_STATE_BUSY         =0x0004
} CASE_STATE;

typedef struct Case_item{
    /* case stuff */
    char                    *case_name;      // case file name
    CASE_STATE              state;           // state of the case
    timer_t                 timer;           // posix timer
    int32_t                 times;           // repit times
    uint8_t                 case_type;       // case type,   1 for att, 2 for pha, 3 for both.
    struct Case_mission     *exe_mis;        // missions

    /* file stuff */
    int                     fd;              // file handler
    char                    *full_path;      // file full path
    uint16_t                ch_max;          // channel number
    uint16_t                *cha_array;      // 
    uint16_t                first_line;      // 
    uint32_t                seek_cur;        // 
    
    /* buffer stuff */
    char                    *buffer;         // 
    char                    *inner_ptr;      // 

    /* middle file */ /* create when the case file has been loaded. */
    int                 mid_fd;              // 
    char                *mid_full_path;      // 
    uint16_t            *val_array;          // 
    uint16_t            val_count;           //
    uint32_t            line_max;            // 
#define VAL_ARRAY_LENGTH    4                //
    uint16_t            mid_read_pt;         //
    uint16_t            mid_write_pt;        //
    pthread_t           bf_ld_th;            //
    sem_t *             bf_ld_sem;           //
    sem_t *             bf_ld_rd_sem;        //
    uint32_t            current_line;        //

    /* extral information of hspeed mode */
    char                flag_hspeed;            // high speed mode flag
    uint32_t            interval_hspeed;        // interval time(ms) for each line, only used in high speed mode
    uint32_t            interval_hs_arr[65535]; // array for interavl time(ms) for every line, read from file.
    uint32_t            interval_load_line;     // the line number of allocated.
#define INTERVAL_BUF_REALLOC_SIZE        1024   //reallocate size of interval buffer.
    struct list_head    list;
}CASE_ITEM;

/* Private define ------------------------------------------------------------*/
#define CASE_M_DEBUG                       APP_DBG_ON

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/*
void init_model_case_manager(void)
{
    APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE, 
        ("initialize case manager.\r\n"));

    get_case_runner_conf();
    
    init_runner_thread();
    
    rt_list_init(&case_item_root);

    regist_cmd_item_list(case_runner_cmd_items);
}
*/