/**
  ******************************************************************************
  * @file    app_debug.c
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "app_debug.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LINE_LEN    32
#define FULL_LOG_FILE_SIZE  256
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef APP_DEBUG_TO_LOG
LOCAL char log_full_path[FULL_LOG_FILE_SIZE];
FILE *log_flie = NULL;
#endif

/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void print_hex(const char* data, int data_len)
{
    int i, j;
    unsigned int offset = 0;
    int total_row = data_len / LINE_LEN;
    int left_data_len = data_len % LINE_LEN;

    /* print data_len / 16 */
    for (j = 0; j < total_row; j++)
    {
        printf("# %08X   ", offset);

        for (i = 0; i < LINE_LEN; i++)
        {
            //if(i%8 == 0) printf(" ");
            printf("%02X ", data[j*LINE_LEN+i]);
        }

        for (i = 0; i < LINE_LEN; i++)
        {
            //if(i%8 == 0) printf(" ");
            if ((data[j*LINE_LEN+i] < 0x20) || (data[j*LINE_LEN+i] > 0x7F))
            {
                printf(".");
            }
            else
            {
                printf("%c", data[j*LINE_LEN+i]);
            }
        }
        offset += LINE_LEN;
        printf("\n");
    }

    /* print data_len % 16 */
    if (left_data_len > 0)
    {
        printf("# %08X   ", offset);
        for (i = 0; i < left_data_len; i++)
        {
            printf("%02X ", data[total_row*LINE_LEN+i]);
        }
        for (i = 0; i < LINE_LEN - left_data_len; i++)
        {
            printf("   ");
        }
        for (i = 0; i < left_data_len; i++)
        {
            if ((data[total_row*LINE_LEN+i] < 0x20) || (data[total_row*LINE_LEN+i] > 0x7F))
            {
                printf(".");
            }
            else
            {
                printf("%c", data[j*LINE_LEN+i]);
            }
        }
        printf("\n");
    }
    return;
} /* print_data */
#ifdef APP_DEBUG_TO_LOG
void print_hex_file(const char* data, int data_len)
{
    int i, j;
    unsigned int offset = 0;
    int total_row = data_len / LINE_LEN;
    int left_data_len = data_len % LINE_LEN;

    /* print data_len / 16 */
    for (j = 0; j < total_row; j++)
    {
        fprintf(log_flie, "# %08X   ", offset);

        for (i = 0; i < LINE_LEN; i++)
        {
            if(i%8 == 0) fprintf(log_flie, " ");
            fprintf(log_flie, "%02X ", data[j*LINE_LEN+i]);
        }

        for (i = 0; i < LINE_LEN; i++)
        {
            if(i%8 == 0) fprintf(log_flie, " ");
            if ((data[j*LINE_LEN+i] < 0x20) || (data[j*LINE_LEN+i] > 0x7F))
            {
                fprintf(log_flie, ".");
            }
            else
            {
                fprintf(log_flie, "%c", data[j*LINE_LEN+i]);
            }
        }
        offset += LINE_LEN;
        fprintf(log_flie, "\n");
    }

    /* print data_len % 16 */
    if (left_data_len > 0)
    {
        fprintf(log_flie, "# %08X   ", offset);
        for (i = 0; i < left_data_len; i++)
        {
            fprintf(log_flie, "%02X ", data[total_row*LINE_LEN+i]);
        }
        for (i = 0; i < LINE_LEN - left_data_len; i++)
        {
            fprintf(log_flie, "   ");
        }
        for (i = 0; i < left_data_len; i++)
        {
            if ((data[total_row*LINE_LEN+i] < 0x20) || (data[total_row*LINE_LEN+i] > 0x7F))
            {
                fprintf(log_flie, ".");
            }
            else
            {
                fprintf(log_flie, "%c", data[j*LINE_LEN+i]);
            }
        }
        fprintf(log_flie, "\n");
    }
    return;
} /* print_data */

#define LOG_PRINT_SIZE 8192
void fprintf_file(const char *fmt, ...)
{
    va_list args;
    char cmd_buf[LOG_PRINT_SIZE];

   	va_start(args, fmt);
	vsnprintf(cmd_buf, LOG_PRINT_SIZE, fmt, args);
	va_end(args);

    fprintf(log_flie, cmd_buf);
//    fflush(log_flie);
    return;
}
#endif
void log_dbg_init(void)
{
#if APP_DEBUG_TO_LOG
    snprintf(log_full_path, FULL_LOG_FILE_SIZE, "%s/Gaia.log", PRJ_FILE_PATH);
    printf(log_full_path);
    log_flie = fopen (log_full_path,"a");
//    setbuf(log_flie,NULL);
#endif
}
