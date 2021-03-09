/**
  ******************************************************************************
  * @file    file name
  * @author  YORK
  * @version V0.1.0
  * @date    03-02-2021
  * @brief   
  *       
	********** Copyright (C), 2014-2015,HJ technologies **************************
	*/
	
/* Includes ------------------------------------------------------------------*/
#include "file_reader.h"
#include "app_debug.h"
#include "platform.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FILEREADER_DEBUG        APP_DBG_ON

#define FILE_FULL_PATH_SIZE     128
#define FILE_BUFFER_SIZE        512

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
LOCAL int16_t get_file_row(int fd);

/* Public functions ----------------------------------------------------------*/
LOCAL int16_t get_subchar_count(uint8_t *buffer, uint16_t buffer_len, char flag)
{
	uint16_t i = 0;
	uint16_t count =0;
	
	for(i=0; i<buffer_len; i++){
		if(buffer[i] == '\0') return count;
		if(buffer[i] == flag) count++;
	}
	return count;
}

LOCAL int16_t get_file_row(int fd)
{
	const uint16_t temp_len = 64;
	int16_t read_len;
	int16_t count = 0;
	uint8_t rx_buffer[temp_len];
	
	APP_ASSERT("fd error.\r\n", fd>-1);

	while(1){
		read_len = read(fd, rx_buffer, temp_len-1);
		if(read_len>0) {
			rx_buffer[read_len] = 0;
			count += get_subchar_count(rx_buffer, read_len, '\n');
		}else{
			APP_DEBUGF(FILEREADER_DEBUG | APP_DBG_TRACE,
			("line = %d\r\n",  count));
			break;
		}
	}
	return count;
}

uint32_t csv_read_file(const char *filename, File_reader file_reader, void *dest_obj)
{
	char full_path[FILE_FULL_PATH_SIZE];
	int read_len, put_len;
	uint8_t rx_buffer[FILE_BUFFER_SIZE];
	struct rb *rb = NULL;
	int fd;
	uint16_t line;

	APP_ASSERT("filename == NULL.\r\n", filename);
    APP_ASSERT("file_reader == NULL.\r\n", file_reader);

/*	mkdir(RELATIVE_FILE_PATH, O_CREAT);*/
	snprintf(full_path, FILE_FULL_PATH_SIZE, "%s/%s", PRJ_FILE_PATH, filename);
	APP_DEBUGF(FILEREADER_DEBUG | APP_DBG_TRACE, ("open file: %s\r\n", full_path));
	fd = open(full_path, O_RDONLY, 0);
	if(fd<0){
		APP_DEBUGF(FILEREADER_DEBUG | APP_DBG_LEVEL_WARNING | APP_DBG_TRACE,
		    ("open file failed.\r\n"));
		return 0;
	}
	line = get_file_row(fd);
	if(line ==0){
		goto end;
	}
	lseek(fd, 0, SEEK_SET);
	rb = rb_malloc("conf_rb", FILE_BUFFER_SIZE*2, FILE_BUFFER_SIZE*2);
	if(!rb){
		APP_DEBUGF(FILEREADER_DEBUG | APP_DBG_LEVEL_SERIOUS | APP_DBG_TRACE,
		    ("rb malloc failed.\r\n"));
	}
	do{
		read_len = read(fd, rx_buffer, FILE_BUFFER_SIZE-2);
		if(read_len>0) {
			put_len = rb_put(rb, rx_buffer, read_len);
			if(put_len == 0){
				APP_DEBUGF(FILEREADER_DEBUG | APP_DBG_TRACE,
				    ("rb_put mem full.\r\n"));
				goto end;
			}
			
			file_reader(rb, dest_obj, line);
		}else{
			APP_DEBUGF(FILEREADER_DEBUG | APP_DBG_TRACE,
			    ("file read complate.(fn = \"%s\",len = %d)\r\n", filename, read_len));
			break;
		}
	}while(1);
	
end:
	close(fd);
	if(rb){
		rb_free(rb);
		rb=NULL;
	}

	return line;
}
