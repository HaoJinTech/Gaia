/**
  ******************************************************************************
  * @file    app_debug.h
  * @author  YORK
  * @version V0.1.0
  * @date    06-12-2015
  * @brief   
  *
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  APP_DEBUG_H
#define  APP_DEBUG_H
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "stdio.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define APP_DEBUG                       APP_DBG_ON
#define APP_ASSERT_ENABLE

#ifdef APP_ASSERT_ENABLE
#include <assert.h>
#endif
#ifdef DEBUG_INTO_FILE
#	define APP_DEBUG_TO_LOG							1
#else
#endif

#define APP_DBG_TYPES_ON                (APP_DBG_ON | APP_DBG_TRACE | APP_DBG_STATE)
#define APP_DBG_MIN_LEVEL               APP_DBG_LEVEL_ALL    //APP_DBG_LEVEL_ALL APP_DBG_LEVEL_WARNING


void log_dbg_printf(const char *fmt, ...);
void sys_arch_assert(const char* file, int line);
#ifdef APP_DEBUG_TO_LOG
#	define APP_PLATFORM_DIAG(x)	do {log_dbg_printf x;} while(0)
#	define APP_PLATFORM_ASSERT(x) do {log_dbg_printf(x); assert(0);}while(0)
#else
#   define APP_PLATFORM_DIAG(x)	do {printf x;} while(0)
#   define APP_PLATFORM_ASSERT(x) do {rt_kprintf(x); assert(0);}while(0)
#endif
#	define APP_PLATFORM_DIAG_USART(x)	do {printf x;} while(0)
/** lower two bits indicate debug level
 * - 0 all
 * - 1 warning
 * - 2 serious
 * - 3 severe
 */
#define APP_DBG_LEVEL_ALL     0x00
#define APP_DBG_LEVEL_OFF     LWIP_DBG_LEVEL_ALL /* compatibility define only */
#define APP_DBG_LEVEL_INFO    0x00
#define APP_DBG_LEVEL_WARNING 0x01 /* file open failed, buffer overflow, ... */
#define APP_DBG_LEVEL_SERIOUS 0x02 /* memory allocation failures, ... */
#define APP_DBG_LEVEL_SEVERE  0x03
#define APP_DBG_MASK_LEVEL    0x03

/** flag for APP_DEBUGF to enable that debug message */
#define APP_DBG_ON            0x80U
/** flag for APP_DEBUGF to disable that debug message */
#define APP_DBG_OFF           0x00U
/** flag for APP_DEBUGF indicating a tracing message (to follow program flow) */
#define APP_DBG_TRACE         0x40U
/** flag for APP_DEBUGF indicating a state debug message (to follow module states) */
#define APP_DBG_STATE         0x20U
/** flag for APP_DEBUGF indicating newly added code, not thoroughly tested yet */
#define APP_DBG_FRESH         0x10U
/** flag for APP_DEBUGF to halt after printing this debug message */
#define APP_DBG_HALT          0x08U

#ifdef APP_ASSERT_ENABLE
/*#define APP_ASSERT(message, assertion) do { if(!(assertion)) \
  APP_PLATFORM_ASSERT(message); } while(0)*/
#define APP_ASSERT(assertion) assert(assertion)
#else
#define APP_ASSERT(assertion) 
#endif

/** if "expression" isn't true, then print "message" and execute "handler" expression */
#ifndef APP_ERROR
#define APP_ERROR(message, expression, handler) do { if (!(expression)) { \
  APP_PLATFORM_ASSERT(message); handler;}} while(0)
#endif /* LWIP_ERROR */

#ifdef APP_DEBUG
/** print debug message only if debug message type is enabled
 *  AND is of correct type AND is at least APP_DBG_LEVEL
 */
#define APP_DEBUGF(debug, message) do { \
                                   if ( \
                                   ((debug) & APP_DBG_ON) && \
                                   ((debug) & APP_DBG_TYPES_ON) && \
                                   ((uint16_t)((debug) & APP_DBG_MASK_LEVEL) >= APP_DBG_MIN_LEVEL)) { \
                                      APP_PLATFORM_DIAG(("[%s:%s:%d]: ",__FILE__, __FUNCTION__, __LINE__));\
                                      APP_PLATFORM_DIAG(message); \
                                      if ((debug) & APP_DBG_HALT) { \
                                        while(1); \
                                      } \
                                    } \
                                  } while(0)


#else  /* APP_DEBUG */
#define APP_DEBUGF(debug, message) 
#endif /* APP_DEBUG */

/* Exported functions --------------------------------------------------------*/ 

#endif
/********************** (C) COPYRIGHT HJ technologies *************************/
