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
#include "sys_config.h"
#include "subboard_manager.h"
#include "bll/protocol_env.h"
#include "bll/bll_calibration.h"
#include "bll/bll_att.h"
#include "bll/bll_pha.h"

#include <semaphore.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>

/* Private typedef -----------------------------------------------------------*/

typedef enum Case_mission_item{
	CASE_MISSION_LOAD 	    =0x0000,
	CASE_MISSION_UNLD		=0x0001,
	CASE_MISSION_RUN		=0x0002,
	CASE_MISSION_STOP		=0x0003,
	CASE_MISSION_PAU		=0x0004,
	CASE_MISSION_CON		=0x0005,
	CASE_MISSION_HS_LOAD    =0x0006,
	
	CASE_MISSION_EXE		=0x0020
} CASE_MISSION;

typedef struct Case_mission{
    long                    msg_id;  // CASE_MISSION_MSG_ID

	CASE_MISSION			mission_id;
	void 					*data;
}CASE_MISSION_MSG;

#define CASE_NAME_CSV_LEN   		256
#define CASE_FULL_PATH_LEN  		256
#define FULL_PATH_AND_CASE_NAME		CASE_FULL_PATH_LEN + CASE_NAME_CSV_LEN
typedef struct Case_item{
    /* case stuff */
#define CASE_NAME_LEN							256
    char                    case_name[CASE_NAME_LEN];      // case file name
    CASE_STATE              state;           // state of the case
	struct timespec			ts;				 // get real time every loop
    timer_t                 timer;           // posix timer
    int32_t                 times;           // repit times
    uint8_t                 case_type;       // case type,   1 for att, 2 for pha, 3 for both.
    struct Case_mission     *exe_mis;        // missions
    /* file stuff */
    int                     fd;              // file handler
    char                    full_path[FULL_PATH_AND_CASE_NAME];      // file full path
    uint32_t                ch_max;          // channel number
    int32_t                *cha_array;      // 
    uint32_t                first_line;      // 
    uint32_t                seek_cur;        // 
    
    /* buffer stuff */
    char                    *buffer;         // 
    char                    *inner_ptr;      // 

    /* middle file */ /* create when the case file has been loaded. */
    int                 mid_fd;              // 
    char                *mid_full_path;      // 
    uint32_t            *val_array;          // 
    uint32_t            val_count;           //
    uint32_t            line_max;            // 
#define VAL_ARRAY_LENGTH    4                //
    uint32_t            mid_read_pt;         //
    uint32_t            mid_write_pt;        //
    pthread_t           bf_ld_th;            //
    sem_t               *bf_ld_sem;           //
    sem_t               *bf_ld_rd_sem;        //
    uint32_t            current_line;        //

    /* extral information of hspeed mode */
    char                flag_hspeed;            // high speed mode flag
    uint32_t            interval_hspeed;        // interval time(ms) for each line, only used in high speed mode
    uint32_t            interval_hs_arr[65535]; // array for interavl time(ms) for every line, read from file.
    uint32_t            interval_load_line;     // the line number of allocated.
#define INTERVAL_BUF_REALLOC_SIZE        1024   //reallocate size of interval buffer.
    struct list_head    list;
}CASE_ITEM;

typedef void (*frame_exe)(struct Case_item *case_item);

struct Case_frame_exe_item
{
	char			key;		/* the name of system call */
	frame_exe 		func;		/* the function address of system call */
};

/* Private define ------------------------------------------------------------*/
#define CASE_M_DEBUG                       APP_DBG_ON
#define CASE_MISSION_MSG_ID                0x71 		// magic number
#define BUFF_SEM_TIMEOUT				   2  			// 2s

#define TIME_NS_PER_S   1000000000
#define TIME_NS_PER_MS	1000000
#define TIME_MS_PER_S	1000

#define FILE_EXTENSION_LOWERCASE	".csv" 
#define FILE_EXTENSION_CAPITAL		".CSV" 
#define FILE_EXTENSION_MIDDLE		".bufferfile" 
#define FILE_LINE_TERMINAL			"\r\n"
#define FILE_WORD_TERMINAL			","
#define WORD_TMN					','
#define WORD_INTER_TMN				'|'
#define LINE_TMN					'\n'

#define CASE_BUFFER_SIZE			8192
#define CHANNEL_BUFFER_ADD			256

#define CASE_TYPE_ATT				0x0001
#define CASE_TYPE_PHA				0x0002

#define CASE_ERROR_OK				1
#define CASE_ERROR					-1
#define CASE_ERROR_FORMAT			-40

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LOCAL const char 	*s_folder;
LOCAL char          case_full_path[CASE_FULL_PATH_LEN];
LOCAL pthread_t 	cs_thread;
LOCAL int 			   s_mq_cs;
LOCAL CASE_ITEM		   case_item_root;
LOCAL SUBBD_PROTOCOL  *g_protocol_obj = 0;
LOCAL BUS_DRIVER      *g_bus_obj = 0;

#if 0
LOCAL void case_exe_att(struct Case_item *case_item);
LOCAL void case_exe_pha(struct Case_item *case_item);
LOCAL void case_exe_att_pha(struct Case_item *case_item);
LOCAL void case_exe_unknow(struct Case_item *case_item);

LOCAL struct Case_frame_exe_item case_exe_items[] = {
	CASE_TYPE_ATT, case_exe_att,
	CASE_TYPE_PHA, case_exe_pha,
	(CASE_TYPE_ATT | CASE_TYPE_PHA), case_exe_att_pha,
};

LOCAL uint32_t CASE_EXE_ITEMS_NUM = sizeof(case_exe_items) / sizeof(struct Case_frame_exe_item) ;
#endif
/* Private function prototypes -----------------------------------------------*/
LOCAL void init_runner_thread(void);

LOCAL void *case_runner_thread_entry(void* parameter);
LOCAL void case_frame_exe(CASE_ITEM *case_item);
LOCAL struct Case_item *upload_case(char *name);
LOCAL int case_start(struct Case_item *case_item);
LOCAL void case_stop(struct Case_item *case_item);
LOCAL void case_pause(struct Case_item *case_item);
LOCAL void case_continue(struct Case_item *case_item);
LOCAL void unload_case(struct Case_item *case_item);
LOCAL uint8_t case_end(struct Case_item *case_item);

LOCAL void case_frame_sender(union sigval param);
LOCAL uint32_t reload_buffer(struct Case_item* case_item, int fd);
LOCAL char* split_word(char *src, char **inner_ptr, uint8_t *new_line);
LOCAL char* case_get_word(struct Case_item *case_item, uint8_t *new_line, uint32_t *count);
LOCAL void full_case_name(char *dest, char *src, uint32_t dest_len); // attach .csv or .CSV to end of case name
LOCAL CASE_MISSION_MSG *init_misson_msg(CASE_MISSION_MSG *miss_msg, CASE_MISSION mis, void *data);
LOCAL int snd_misson_msg(CASE_MISSION_MSG *case_mission_msg);

/* Private functions ----------------------------------------------------------*/
LOCAL void init_runner_thread(void)
{    
    pthread_t	tid;

    s_mq_cs =  msgget( IPC_PRIVATE, 0666 ) ;
	if(s_mq_cs == -1){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS, ("msgget failed (%d).\r\n", s_mq_cs));
		return;
    }

	cs_thread = pthread_create(&tid, NULL, case_runner_thread_entry, NULL);
}

LOCAL uint32_t reload_buffer(struct Case_item* case_item, int fd)
{
	uint32_t read_len;
	uint32_t str_count;
	if(case_item->inner_ptr == NULL){
		str_count = 0;
	}else{
		str_count = strlen(case_item->inner_ptr) * sizeof(char);
		memmove(case_item->buffer, case_item->inner_ptr, str_count);
	}

	read_len = read(fd, &(case_item->buffer[str_count]), CASE_BUFFER_SIZE - str_count);
	case_item->inner_ptr = case_item->buffer;
	case_item->buffer[read_len + str_count] = '\0';
	
	return read_len;
}

LOCAL char* split_word(char *src, char **inner_ptr, uint8_t *new_line)
{
	char *str = src;
	char *inner_temp = *inner_ptr;
	if(src == NULL) str = *inner_ptr;
	if(*inner_ptr == NULL) inner_temp = src;
	if(inner_temp == NULL) return NULL;
	if(*inner_temp == '\0') return NULL;
	while(str!= NULL){
		if(*str == WORD_TMN || *str == WORD_INTER_TMN){
			*str = '\0';
			*inner_ptr = str+1;
			if(new_line) *new_line = 0;
			return inner_temp;
		}else if(*str == LINE_TMN){
			*str = '\0';
			*(str-1) = '\0';
			*inner_ptr = str+1;
			if(new_line) *new_line = 1;
			return inner_temp;
		}else if(*str == '\0'){
			*new_line = 0;
			return NULL;
		}
		str++;
	}
	if(new_line) *new_line = 0;
	return NULL;
}

LOCAL char* case_get_word(struct Case_item *case_item, uint8_t *new_line, uint32_t *count)
{
	char *word;
	word = split_word(NULL, &case_item->inner_ptr, new_line);
	if(word == NULL){
		uint32_t read_len;
		read_len = reload_buffer(case_item, case_item->fd);  
		if(read_len <= 0)
			return NULL;
		word = split_word(case_item->buffer, &case_item->inner_ptr, new_line);
	}
	if(count) *count = case_item->inner_ptr - word;
//	APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE ,("get word: %s.\r\n", word));
	return word;
}

LOCAL uint8_t case_end(struct Case_item *case_item)
{
	APP_ASSERT("case_end: case_item == NULL.\r\n",case_item);

	case_item->current_line = 0;
	APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE ,
		("case(%s) times left(%d).\r\n",case_item->case_name,case_item->times));
	if(case_item->times == 0){
		case_stop(case_item);
		return 1;
	}else if(case_item->times >0){
		case_item->times--;
	}
	return 0;
}

static void sendCMD_NextStep_tosunboard(struct Case_item *case_item)
{
	APP_ASSERT("sendCMD_NextStep_tosunboard: Free pointer.\r\n", case_item!= NULL);
	
	g_protocol_obj->ioctrl(g_protocol_obj, IO_CTRL_MSG_UPDATE_CASE_LINE, g_bus_obj, case_item->current_line);
}

LOCAL void case_frame_exe(CASE_ITEM *case_item)
{
	uint32_t    interval;
	int			err;
	
	APP_ASSERT("Free pointer.\r\n", case_item);
	if(case_item->state == CASE_STATE_UNLOADED){
		unload_case(case_item);
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING ,("case unload.\r\n"));
	}

	if(case_item->state == CASE_STATE_STOP){
		return;
	}
	if(!case_item->flag_hspeed) {
		struct timespec ts;
	    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	        APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING ,("clock_gettime error.\r\n"));
		ts.tv_sec += BUFF_SEM_TIMEOUT;
		err = sem_timedwait(case_item->bf_ld_sem, &ts);
		if(err <0){
			APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING, ("rt_sem_take timeout(code:%d).\r\n", err));
			case_stop(case_item);
			return;
		}
	}
	if(case_item->current_line >= case_item->line_max){
		if(case_end(case_item)){
			return;
		}
	}

	if(case_item->state == CASE_STATE_STOP){
		return;
	}

	if(!case_item->flag_hspeed) {
		interval = case_item->val_array[case_item->mid_read_pt*case_item->val_count];
	}else{
		interval = case_item->interval_hs_arr[case_item->current_line];
	}
	if(interval<1) interval = case_item->interval_hspeed;

	case_item->ts.tv_sec += (interval / TIME_MS_PER_S);
	case_item->ts.tv_nsec += (interval % TIME_MS_PER_S) * TIME_NS_PER_MS;
	if(case_item->ts.tv_nsec> TIME_NS_PER_S){
		case_item->ts.tv_sec++;
		case_item->ts.tv_nsec -= TIME_NS_PER_S;
	}
	struct itimerspec its;
	memset(&its, 0, sizeof(struct itimerspec));

	// minus the time err to make it correct
	its.it_value.tv_sec = case_item->ts.tv_sec;
    its.it_value.tv_nsec = case_item->ts.tv_nsec;

//	APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE ,
//		("offset_sec:%ld,offset_nsec%ld.\r\n",its.it_value.tv_sec,its.it_value.tv_nsec));
	timer_settime(case_item->timer, TIMER_ABSTIME, &its, NULL);

	if(!case_item->flag_hspeed) {
#if 0
		uint32_t    i = 0;
		uint32_t	spacing;
		while(case_exe_items[i].key){
			if(case_exe_items[i].key == case_item->case_type)
				break;
			i++;
		}
		case_exe_items[i].func(case_item);
		case_item->mid_read_pt = case_item->mid_read_pt >= (VAL_ARRAY_LENGTH-1) ? 0 : case_item->mid_read_pt+1;
		if(case_item->mid_read_pt <= case_item->mid_write_pt){
			spacing = case_item->mid_write_pt - case_item->mid_read_pt;
		}else{
			spacing = (VAL_ARRAY_LENGTH - case_item->mid_read_pt) + case_item->mid_write_pt;
		}
		if(spacing < VAL_ARRAY_LENGTH/2){
			rt_sem_release(case_item->bf_ld_rd_sem);
		}
#endif
	}else{
		sendCMD_NextStep_tosunboard(case_item);
	}
	case_item->current_line ++;
	return;
}

LOCAL int8_t upload_to_subboard(struct Case_item *case_item)
{
	uint8_t new_line;
	int i;
	char *c_val;
	uint32_t count;
	int32_t *temp_att= NULL, *temp_pha=NULL;

	g_protocol_obj->ioctrl(g_protocol_obj, IO_CTRL_MSG_START_CASE_UPLOAD, g_bus_obj);

	case_item->val_count = 1;
	if(case_item->case_type & CASE_TYPE_ATT){
		temp_att = (int32_t*)malloc(sizeof(int32_t) * (case_item->ch_max+1));
		case_item->val_count += case_item->ch_max;
	}
	if(case_item->case_type & CASE_TYPE_PHA){
		temp_pha = (int32_t*)malloc(sizeof(int32_t) * (case_item->ch_max+1));
		case_item->val_count += case_item->ch_max;
	}
	
	case_item->line_max = 0;
	while(1){
		/* this c_val is interval(ms) for every line */
		c_val = case_get_word(case_item, &new_line, &count);
		if(!c_val){
			APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE,
				(" middle file created.\r\n"));
			break;
		}
		/* line_max is keep increasing during the loading proccess, set the line delay */
		case_item->interval_hs_arr[case_item->line_max] = atoi(c_val);
		for(i=0; i<case_item->ch_max; i++){
			if(case_item->case_type & CASE_TYPE_ATT){
				if(new_line == 1){
					temp_att[i] = 0;
				}else{
					c_val = case_get_word(case_item, &new_line, &count);
					temp_att[i] = atoi(c_val);
				}
			}
			if(case_item->case_type & CASE_TYPE_PHA){
				if(new_line == 1){
					temp_pha[i] = 0;
				}else{
					c_val = case_get_word(case_item, &new_line, &count);
					temp_pha[i] = atoi(c_val);
				}
			}
			if(calibration_is_enabled()){
				if(case_item->case_type & CASE_TYPE_PHA &&
				   case_item->case_type & CASE_TYPE_ATT)
					temp_pha[i] = calibration_proc(case_item->cha_array[i], temp_att[i], temp_pha[i], &temp_att[i]);
				else if(case_item->case_type & CASE_TYPE_PHA)
					temp_pha[i] = calibration_proc(case_item->cha_array[i], get_att(case_item->cha_array[i]), 
										temp_pha[i], NULL);
				else
					calibration_proc(case_item->cha_array[i], temp_att[i], get_pha(case_item->cha_array[i]), &temp_att[i]);
			}
		}

		/* get new line */
		while(!new_line)
			case_get_word(case_item, &new_line, &count);
		
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE, ("upload line:%d\r\n",case_item->line_max));
		/* send data to sub board after */
		if(case_item->case_type & CASE_TYPE_ATT)
			subbd_send_MCMV(DEST_UPLD_ATT, g_protocol_obj, g_bus_obj, case_item->cha_array, temp_att, case_item->ch_max);
		if(case_item->case_type & CASE_TYPE_PHA)
			subbd_send_MCMV(DEST_UPLD_PHA, g_protocol_obj, g_bus_obj, case_item->cha_array, temp_pha, case_item->ch_max);		

		case_item->line_max += 1;
	}

	if(temp_att) free(temp_att);
	if(temp_pha) free(temp_pha);
	return CASE_ERROR_OK;
}

LOCAL int8_t upload_to_subboard_ex(struct Case_item *case_item)
{
	uint8_t new_line;
	int i;
	char *c_val;
	uint32_t count;
	int32_t *temp_att_pha;

	g_protocol_obj->ioctrl(g_protocol_obj, IO_CTRL_MSG_START_CASE_UPLOAD, g_bus_obj);

	case_item->val_count = 1;
	if(!(case_item->case_type & (CASE_TYPE_ATT | CASE_TYPE_PHA))){
		return RET_ERROR;
	}
	if(case_item->case_type & CASE_TYPE_ATT){
		case_item->val_count += case_item->ch_max;
	}
	if(case_item->case_type & CASE_TYPE_PHA){
		case_item->val_count += case_item->ch_max;
	}
	temp_att_pha = (int32_t*)malloc(sizeof(int32_t) * (case_item->val_count+1));
	
	case_item->line_max = 0;
	while(1){
		/* this c_val is interval(ms) for every line */
		c_val = case_get_word(case_item, &new_line, &count);
		if(!c_val){
			APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE,
				(" middle file created.\r\n"));
			break;
		}
		/* line_max is keep increasing during the loading proccess, set the line delay */
		case_item->interval_hs_arr[case_item->line_max] = atoi(c_val);
		for(i=0; i<case_item->ch_max; i++){
			if(case_item->case_type & CASE_TYPE_ATT){
				if(new_line == 1){
					temp_att_pha[i*2] = 0;

				}else{
					c_val = case_get_word(case_item, &new_line, &count);
					temp_att_pha[i*2] = atoi(c_val);
				}
			}
			if(case_item->case_type & CASE_TYPE_PHA){
				if(new_line == 1){
					temp_att_pha[i*2+1] = 0;
				}else{
					c_val = case_get_word(case_item, &new_line, &count);
					temp_att_pha[i*2+1] = atoi(c_val);
				}
			}
			if(calibration_is_enabled()){
				// ATT = i*2, PHA = i*2+1
				temp_att_pha[i*2+1] = calibration_proc(case_item->cha_array[i], 
								temp_att_pha[i*2], temp_att_pha[i*2+1], &temp_att_pha[i*2]);
			}
		}

		/* get new line */
		while(!new_line)
			case_get_word(case_item, &new_line, &count);
		
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE, ("upload line:%d\r\n",case_item->line_max));
		/* send data to sub board after */
		subbd_send_CCMMV(DEST_UPLD_ATT_PHA_EX, g_protocol_obj, g_bus_obj, 0, temp_att_pha, 2, case_item->ch_max);

		case_item->line_max += 1;
	}

	free(temp_att_pha);
	return CASE_ERROR_OK;
}

LOCAL void case_frame_sender(union sigval param)
{
	int ret;
	struct Case_item *case_item = (struct Case_item *)param.sival_ptr;

	APP_ASSERT("case_frame_sender: param == NULL.\r\n", case_item);

    ret = msgsnd(s_mq_cs, (void *)case_item->exe_mis, sizeof(CASE_MISSION_MSG)-sizeof(long), 0);
    if(ret<0){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE, 
			("msg send failed.(code:%d)\r\n",ret));
	}

	return ;
}


LOCAL void full_case_name(char *dest, char *src, uint32_t dest_len)
{
	memset(dest,0,dest_len);
	strncpy(dest, src, dest_len);
	if(!strstr(src, FILE_EXTENSION_LOWERCASE) &&
		!strstr(src, FILE_EXTENSION_CAPITAL)){
		strcat(dest, ".csv\0");
	}
}

LOCAL struct Case_item *upload_case(char *name)
{
	char full_path[FULL_PATH_AND_CASE_NAME];
	int fd;
	struct Case_item *case_item;
	uint8_t new_line;
	uint32_t count;
	char *c_case_type, *c_ch;
	uint32_t iter;
	uint32_t first_line = 0;
	char case_name_csv[CASE_NAME_CSV_LEN];
	int8_t res;
	
	APP_ASSERT("upload_case: Free pointer.\r\n", name!= NULL);
	full_case_name(case_name_csv, name, CASE_NAME_CSV_LEN);
	snprintf(full_path, FULL_PATH_AND_CASE_NAME, "%s%s", case_full_path, case_name_csv);

	fd = open(full_path, O_RDONLY , 0);
	if(fd < 0){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS, ("file can not open (%s).\r\n", full_path));	
		return NULL;
	}
	
	case_item = (struct Case_item*)malloc(sizeof(struct Case_item));	
	if(!case_item){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS, ("case_item malloc failed.\r\n"));		
		goto end_failed;
	}
	memset(case_item, 0, sizeof(struct Case_item));
	case_item->state = CASE_STATE_BUSY;
	case_item->exe_mis = (struct Case_mission*)malloc(sizeof(struct Case_mission));
	if(!case_item->exe_mis){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS, ("case_item->exe_mis malloc failed.\r\n"));		
		goto end_failed;
	}
	case_item->flag_hspeed = 1;
	case_item->interval_hspeed = 10; // 10 ms for default
	case_item->exe_mis->mission_id = CASE_MISSION_EXE;
	case_item->exe_mis->data = (void*)case_item;
	case_item->exe_mis->msg_id = CASE_MISSION_MSG_ID;
	case_item->fd = -1;
	case_item->mid_fd = -1;
	case_item->buffer = (char*)malloc(sizeof(char)* CASE_BUFFER_SIZE+ 16);
	if(!case_item->buffer){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS ,
			("case_item->buffer malloc failed.\r\n"));		
		goto end_failed;
	}
	memset(case_item->buffer, 0, sizeof(char)*CASE_BUFFER_SIZE);
	case_item->fd = fd;
	strncpy(case_item->case_name, case_name_csv, CASE_NAME_LEN);

	strncpy(case_item->full_path, full_path, FULL_PATH_AND_CASE_NAME);

	c_case_type = case_get_word(case_item, &new_line, &count);
	first_line += count;
	case_item->case_type = atoi(c_case_type);
	APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE ,
			("case_type = %d.\r\n", case_item->case_type));	
	if(!(case_item->case_type & CASE_TYPE_ATT) && 
		 !(case_item->case_type & CASE_TYPE_PHA)){
		goto end_failed;
	}
	case_item->ch_max = CHANNEL_BUFFER_ADD;
	case_item->cha_array = (int32_t *)malloc(sizeof(int32_t)*case_item->ch_max);	
	if(!case_item->cha_array){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS ,
			("cha_array malloc failed.\r\n"));		
		goto end_failed;
	}

	case_item->interval_load_line = 0;
	iter = 0;
	while(1){
		c_ch = case_get_word(case_item, &new_line, &count);
		first_line +=count;
		if(iter >= case_item->ch_max){
			int32_t *cha_array_temp = NULL;
			int32_t cha_max_temp = case_item->ch_max + CHANNEL_BUFFER_ADD;
			cha_array_temp = (int32_t *)malloc(sizeof(int32_t)*cha_max_temp);	
			if(!cha_array_temp){
				APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS,
					("cha_array_temp malloc failed.\r\n"));		
				goto end_failed;
			}
			memset(cha_array_temp, 0, cha_max_temp*sizeof(int32_t));
			memcpy(cha_array_temp, case_item->cha_array, case_item->ch_max*sizeof(int32_t));
			free(case_item->cha_array);
			case_item->cha_array = cha_array_temp;
			case_item->ch_max = cha_max_temp;
		}
		if(c_ch){
			case_item->cha_array[iter] = atoi(c_ch)-1;
			iter++;
		}else{ 
			case_item->ch_max = iter;
			case_item->first_line = first_line;
			break;
		}
		if(new_line){
			case_item->ch_max = iter;
			case_item->first_line = first_line;
			break;
		}
	}

	INIT_LIST_HEAD(&case_item->list);
    list_add(&case_item->list, &case_item_root.list);
	
	res = upload_to_subboard(case_item);
	if(res<0){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS, ("upload_to_subboard failed.\r\n"));		
		goto end_failed;
	}
	free(case_item->buffer);
	case_item->buffer = NULL;
	case_item->inner_ptr = NULL;
	
	close(fd);
	case_item->fd=-1;
	case_item->state = CASE_STATE_STOP;
	
	return case_item;
end_failed:	
	if(case_item->exe_mis){
		free(case_item->exe_mis);
		case_item->exe_mis = NULL;
	}
	if(case_item->cha_array){
		free(case_item->cha_array);
		case_item->cha_array = NULL;
	}
	if(case_item->buffer){
		free(case_item->buffer);
		case_item->buffer = NULL;
	}
	if(case_item)
		free(case_item);
	close(fd);
	return NULL;
}

LOCAL int case_start(struct Case_item *case_item)
{
	int res;

	APP_ASSERT("case_start: Free pointer.\r\n", case_item!= NULL);

	if(case_item->state == CASE_STATE_RUN){
		case_stop(case_item);
	}else if(case_item->state == CASE_STATE_UNLOADED){
		return RET_ERROR_CASE_UNLOAD;
	}else if(case_item->state == CASE_STATE_BUSY){
		return RET_ERROR_CASE_BUSY;
	}

	struct sigevent sev;
	memset(&sev, 0, sizeof(struct sigevent));
	sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = case_item;
    sev.sigev_notify_function = case_frame_sender;

    /* create timer */
    if (timer_create (CLOCK_REALTIME, &sev, &case_item->timer) == -1){
    	APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS, ("timer_create failed.\r\n"));		
    }
	
	if(!case_item->flag_hspeed){
	/* buffer loader begin */
#if 0
		res = case_buffer_load_start(case_item);
		if(res < 0){
			APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS,
				("case_buffer_load_start failed.\r\n"));
			return CASE_ERROR;
		}
#endif
	}

	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING ,("clock_gettime error.\r\n"));
	case_item->ts.tv_sec = ts.tv_sec;
	case_item->ts.tv_nsec = ts.tv_nsec;
	
	struct itimerspec its;
	memset(&its, 0, sizeof(struct itimerspec));
	its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1;

	case_item->state = CASE_STATE_RUN;
	res = timer_settime(case_item->timer,0,&its,NULL);

	return res;
}

static void case_stop(struct Case_item *case_item)
{
	int err;
	APP_ASSERT("case_stop: Free pointer.\r\n", case_item!= NULL);

	if(case_item->state == CASE_STATE_STOP){
		return;
	}
	case_item->state = CASE_STATE_STOP;

	if(case_item->timer){
		err =timer_delete(case_item->timer);
		if(err<0){
			APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING,
				("timer stop failed(code:%d).\r\n",err));
		}
	}
	if(case_item->bf_ld_rd_sem){
		sem_post(case_item->bf_ld_rd_sem);
	}
	/*
	while(case_item->bf_ld_th){
		rt_thread_yield();
	}*/
	if(case_item->bf_ld_sem){
		sem_destroy(case_item->bf_ld_sem);
		case_item->bf_ld_sem = NULL;
	}
	if(case_item->bf_ld_rd_sem){
		sem_destroy(case_item->bf_ld_rd_sem);
		case_item->bf_ld_rd_sem = NULL;
	}
	if(case_item->val_array){
		free(case_item->val_array);
		case_item->val_array = NULL;
	}
	if(case_item->mid_fd>=0){
		close(case_item->mid_fd);
		case_item->mid_fd = -1;
	}

	case_item->mid_read_pt = 0;
	case_item->mid_write_pt = 0;
	case_item->current_line = 0;

	return;
}

LOCAL void case_pause(struct Case_item *case_item)
{
	int err;
	APP_ASSERT("case_pause: Free pointer.\r\n", case_item!= NULL);
	if(case_item->timer){
		struct itimerspec its;
		memset(&its, 0, sizeof(struct itimerspec));
		// set 0 to stop the timer.
		err = timer_settime(case_item->timer,0,&its,NULL);
		if(err<0){
			APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING,
				("timer pause failed(code:%d).\r\n",err));
		}
	}

	if(!case_item->flag_hspeed && case_item->fd>=0){
		case_item->seek_cur = lseek(case_item->fd, 0, SEEK_CUR);
		close(case_item->fd);
	}
	case_item->state = CASE_STATE_PAUSE;
}

LOCAL void case_continue(struct Case_item *case_item)
{
	int err;
	APP_ASSERT("case_continue: Free pointer.\r\n", case_item!= NULL);
	if(!case_item->flag_hspeed){
		case_item->fd = open(case_item->full_path, O_RDONLY, 0);
		lseek(case_item->fd, case_item->seek_cur, SEEK_SET);
	}
	case_item->state = CASE_STATE_RUN;

	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING ,("clock_gettime error.\r\n"));
	case_item->ts.tv_sec = ts.tv_sec;
	case_item->ts.tv_nsec = ts.tv_nsec;

	struct itimerspec its;
	memset(&its, 0, sizeof(struct itimerspec));
	// set 0 to stop the timer.	
	its.it_value.tv_nsec = 1;
	err = timer_settime(case_item->timer,0,&its,NULL);
	if(err<0){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING,
			("timer pause failed(code:%d).\r\n",err));
	}
}


LOCAL void unload_case(struct Case_item *case_item)
{
	APP_ASSERT("unload_case: Free pointer.\r\n", case_item!= NULL);

	if(case_item->state != CASE_STATE_STOP){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING,
			("case is running, shoud not reach here.\r\n"));
		return;
	}
	list_del(&case_item->list);

	if(case_item->timer)
		timer_delete(case_item->timer);
	if(case_item->fd>=0)
		close(case_item->fd);
	if(case_item->buffer)
		free(case_item->buffer);
	if(case_item->cha_array)
		free(case_item->cha_array);
	if(case_item->exe_mis)
		free(case_item->exe_mis);
	if(case_item->mid_full_path)
		free(case_item->mid_full_path);
		
	free(case_item);
}


LOCAL void *case_runner_thread_entry(void* parameter)
{
	ssize_t size;
	CASE_MISSION_MSG case_mission;

	while(1){
        size= msgrcv(s_mq_cs, (void *)&case_mission, sizeof(CASE_MISSION_MSG)-sizeof(long), CASE_MISSION_MSG_ID, 0);
		if(size<0){
			APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SEVERE, ("mq recive failed.(code:%d)\r\n",size));
			break;
		}
		//APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE, ("recive mq, ID:%d\r\n",(int)case_mission.mission_id));
		switch(case_mission.mission_id){
			case CASE_MISSION_EXE:
				case_frame_exe((CASE_ITEM *)case_mission.data);
			break;
			case CASE_MISSION_LOAD:
//				load_case((char*)case_mission.data);
//				free(case_mission.data);
			break;
			case CASE_MISSION_HS_LOAD:
				upload_case((char*)case_mission.data);
				free(case_mission.data);
			break;
			case CASE_MISSION_UNLD:
				unload_case((CASE_ITEM *)case_mission.data);
			break;
			case CASE_MISSION_RUN:
				case_start((CASE_ITEM *)case_mission.data);
			break;
			case CASE_MISSION_STOP:
				case_stop((CASE_ITEM *)case_mission.data);
			break;
			case CASE_MISSION_PAU:
				case_pause((CASE_ITEM *)case_mission.data);
			break;
			case CASE_MISSION_CON:
				case_continue((CASE_ITEM *)case_mission.data);
			break;
		}
	}

	return NULL;
}

LOCAL CASE_MISSION_MSG *init_misson_msg(CASE_MISSION_MSG *miss_msg, CASE_MISSION mis, void *data)
{
	APP_ASSERT("miss_msg = NULL",miss_msg);
	miss_msg->msg_id = CASE_MISSION_MSG_ID;
	miss_msg->mission_id = mis;
	miss_msg->data = data;

	return miss_msg;
}

LOCAL int snd_misson_msg(CASE_MISSION_MSG *case_mission_msg)
{
	int ret = 0;
    ret = msgsnd(s_mq_cs, (void *)case_mission_msg, sizeof(CASE_MISSION_MSG)-sizeof(long), 0);
    if(ret<0){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_WARNING, 
			("msg send failed.(code:%d)\r\n",ret));
		return ret;
	}
	return RET_OK;
}
/* Public functions ----------------------------------------------------------*/

int32_t send_upload_misson(char *case_name)
{
	if(!case_name)
		return RET_ERROR;
	uint32_t caselen= strlen(case_name);

	caselen++;

	char *case_name_cpy = malloc(sizeof(char) * caselen+3);
	memset(case_name_cpy, 0 ,caselen+1);
	strncpy(case_name_cpy, case_name, caselen+1);

	CASE_MISSION_MSG case_mission_msg;
	init_misson_msg(&case_mission_msg, CASE_MISSION_HS_LOAD, case_name_cpy);
	return snd_misson_msg(&case_mission_msg);
}

int32_t send_unload_misson(struct Case_item *case_item)
{
	if(!case_item)
		return RET_ERROR;

	CASE_MISSION_MSG case_mission_msg;
	init_misson_msg(&case_mission_msg, CASE_MISSION_UNLD, case_item);
	return snd_misson_msg(&case_mission_msg);
}

int32_t send_run_misson(struct Case_item *case_item)
{
	if(!case_item)
		return RET_ERROR;

	CASE_MISSION_MSG case_mission_msg;
	init_misson_msg(&case_mission_msg, CASE_MISSION_RUN, case_item);
	return snd_misson_msg(&case_mission_msg);
}

int32_t send_stop_misson(struct Case_item *case_item)
{
	if(!case_item)
		return RET_ERROR;

	CASE_MISSION_MSG case_mission_msg;
	init_misson_msg(&case_mission_msg, CASE_MISSION_STOP, case_item);
	return snd_misson_msg(&case_mission_msg);
}

int32_t send_pause_misson(struct Case_item *case_item)
{
	if(!case_item)
		return RET_ERROR;

	CASE_MISSION_MSG case_mission_msg;
	init_misson_msg(&case_mission_msg, CASE_MISSION_PAU, case_item);
	return snd_misson_msg(&case_mission_msg);
}

int32_t send_countinu_misson(struct Case_item *case_item)
{
	if(!case_item)
		return RET_ERROR;

	CASE_MISSION_MSG case_mission_msg;
	init_misson_msg(&case_mission_msg, CASE_MISSION_CON, case_item);
	return snd_misson_msg(&case_mission_msg);
}


int32_t init_model_case_manager(json_object *case_json_obj)
{
	int protocol_id = 0;
    int bus_id = 0;

    APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE, ("initialize case manager.\r\n"));
    s_folder = config_get_string(case_json_obj, "CASE_PATH", "case/");

	snprintf(case_full_path, CASE_FULL_PATH_LEN, "%s/%s", PRJ_FILE_PATH, s_folder);

    protocol_id = config_get_int(case_json_obj, "CASE_PROTOCOL", PROTOCOL_ID_RR485);
    if(protocol_id > SUBBD_PROTOCOL_SIZE) protocol_id = PROTOCOL_ID_RR485;
    g_protocol_obj = &protocols[protocol_id];

    bus_id =    config_get_int(case_json_obj, "CASE_BUS", BUS_ID_SPI);
    if(bus_id > BUS_DRIVER_NUM) bus_id = BUS_ID_SPI;
    g_bus_obj = &bus_drivers[bus_id];

    init_runner_thread();
	INIT_LIST_HEAD(&case_item_root.list);
	
	return RET_OK;
}

struct Case_item *get_case_item(char *name)
{
	struct Case_item *iter;
	uint32_t case_name_len = 0, name_len = 0;
	char case_name_csv[CASE_NAME_CSV_LEN];
	
	full_case_name(case_name_csv, name, CASE_NAME_CSV_LEN);

    list_for_each_entry(iter, &case_item_root.list, list){
		case_name_len = strlen(iter->case_name);
		name_len = strlen(case_name_csv);
		if(name_len == case_name_len &&
		0 == strncmp(iter->case_name, case_name_csv, case_name_len)){
            return iter;
		}
    }

	return NULL;
}

CASE_STATE get_case_state(struct Case_item *case_item, char *out_state, uint32_t strlen)
{
	if(!case_item){
		if(out_state){
			snprintf(out_state, strlen, "UNLOAD");
		}
		return CASE_STATE_UNLOADED;
	}else{
		switch (case_item->state){
		case CASE_STATE_STOP:
			if(out_state){
				snprintf(out_state, strlen, "STOP");
			}			
			break;
		case CASE_STATE_RUN:
			if(out_state){
				snprintf(out_state, strlen, "RUN");
			}			
			break;
		case CASE_STATE_BUSY:
			if(out_state){
				snprintf(out_state, strlen, "BUSY");
			}			
			break;
		case CASE_STATE_PAUSE:
			if(out_state){
				snprintf(out_state, strlen, "PAUSE");
			}
			break;	
		default:
			if(out_state){
				snprintf(out_state, strlen, "UNKNOW");
			}
			break;
		}
	}

	return case_item->state;
}

uint32_t get_case_line_max(struct Case_item *case_item)
{
	if(case_item){
		return case_item->line_max;
	}
	return 0;
}

uint32_t get_case_times(struct Case_item *case_item)
{
	if(case_item){
		return case_item->times;
	}
	return 0;
}

uint32_t set_case_times(struct Case_item *case_item, uint32_t times)
{
	if(case_item){
		case_item->times = times;
		return RET_OK;
	}
	return RET_ERROR;
}

uint32_t get_case_current_line(struct Case_item *case_item)
{
	if(case_item){
		return case_item->current_line;
	}
	return 0;
}

uint32_t set_case_current_line(struct Case_item *case_item, uint32_t line)
{
	if(case_item){
		case_item->current_line = line;
		return RET_OK;
	}
	return RET_ERROR;
}

uint32_t set_case_interval(struct Case_item *case_item, uint32_t interval)
{
	if(case_item){
		case_item->interval_hspeed = interval;
		return RET_OK;
	}
	return RET_ERROR;
}

int32_t check_case_exist(char *name)
{
	char case_name_csv[CASE_NAME_CSV_LEN];
	char full_path[FULL_PATH_AND_CASE_NAME];
	struct Case_item *case_item = NULL;
	int fd;

	if(!name)
		return 0;

	case_item = get_case_item(name);
	if(case_item){
		return 1;
	}else{
		full_case_name(case_name_csv, name, CASE_NAME_CSV_LEN);
		snprintf(full_path, FULL_PATH_AND_CASE_NAME, "%s%s", case_full_path, case_name_csv);
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE, ("full_path:%s.\r\n", full_path));		

		fd = open(full_path, O_RDONLY , 0);
		if(fd < 0){
			return 0;
		}
		close(fd);
		return 1;
	}
}

DIR* search_folder_start(void)
{
	return opendir(case_full_path);
}

int32_t search_folder_get_name(DIR* dirp, char *out_name, uint32_t strlen, uint32_t *out_filesize)
{
	struct dirent* entry;
#define LINE_BUF_SIZE		512
	char line_buffer[LINE_BUF_SIZE];
	struct stat s;

	while (1){
		entry = readdir(dirp);
		if (entry == NULL) return RET_ERROR;
		snprintf(line_buffer, LINE_BUF_SIZE, "%s/%s", case_full_path, entry->d_name);
		if (stat(line_buffer, &s) == 0){
			if(!(s.st_mode & S_IFDIR) && 
            (strstr(entry->d_name, FILE_EXTENSION_CAPITAL) || 
            strstr(entry->d_name, FILE_EXTENSION_LOWERCASE))){
				if(strstr(entry->d_name, FILE_EXTENSION_MIDDLE)){
					continue;
				}
				if(out_name){
					strncpy(out_name, entry->d_name, strlen);
				}
				if(out_filesize){
					*out_filesize = s.st_size;
				}
				return RET_OK;
			}
		}else{
			return RET_ERROR;
		}
	}
	return RET_ERROR;
}

void search_folder_end(DIR *dirp)
{
	if(dirp){
		closedir(dirp);
	}
}

const char *get_case_full_path(void)
{
	return case_full_path;
}


