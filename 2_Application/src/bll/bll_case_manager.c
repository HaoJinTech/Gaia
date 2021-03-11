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

#include <time.h>
#include <signal.h>  
#include <semaphore.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
typedef enum Case_state{
    CASE_STATE_UNLOADED     =0x0000,
    CASE_STATE_STOP         =0x0001,
    CASE_STATE_RUN          =0x0002,
    CASE_STATE_PAUSE        =0x0003,
    CASE_STATE_BUSY         =0x0004
} CASE_STATE;

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

typedef struct Case_item{
    /* case stuff */
#define CASE_NAME_LEN							256
    char                    case_name[CASE_NAME_LEN];      // case file name
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
    sem_t               bf_ld_sem;           //
    sem_t               bf_ld_rd_sem;        //
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
LOCAL char 			*s_folder;
LOCAL pthread_t 	cs_thread;
LOCAL int 			s_mq_cs;
LOCAL CASE_ITEM			case_item_root;
LOCAL SUBBD_PROTOCOL  *g_protocol_obj = 0;
LOCAL BUS_DRIVER      *g_bus_obj = 0;

static void case_exe_att(struct Case_item *case_item);
static void case_exe_pha(struct Case_item *case_item);
static void case_exe_att_pha(struct Case_item *case_item);
static void case_exe_unknow(struct Case_item *case_item);

static struct Case_frame_exe_item case_exe_items[] = {
	CASE_TYPE_ATT, case_exe_att,
	CASE_TYPE_PHA, case_exe_pha,
	(CASE_TYPE_ATT | CASE_TYPE_PHA), case_exe_att_pha,
	
	NULL, case_exe_unknow
};

/* Private function prototypes -----------------------------------------------*/
LOCAL void init_runner_thread(void);

LOCAL void case_runner_thread_entry(void* parameter);
LOCAL void case_frame_exe(CASE_ITEM *case_item);
LOCAL struct Case_item *upload_case(char *name);

LOCAL void case_frame_sender(void *param);
LOCAL int16_t reload_buffer(struct Case_item* case_item, int fd);
LOCAL char* split_word(char *src, char **inner_ptr, uint8_t *new_line);
LOCAL char* case_get_word(struct Case_item *case_item, uint8_t *new_line, uint16_t *count);

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

LOCAL int16_t reload_buffer(struct Case_item* case_item, int fd)
{
	uint16_t read_len;
	uint16_t str_count;
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

LOCAL char* case_get_word(struct Case_item *case_item, uint8_t *new_line, uint16_t *count)
{
	char *word;
	word = split_word(NULL, &case_item->inner_ptr, new_line);
	if(word == NULL){
		uint16_t read_len;
		read_len = reload_buffer(case_item, case_item->fd);  
		if(read_len <= 0)
			return NULL;
		word = split_word(case_item->buffer, &case_item->inner_ptr, new_line);
	}
	if(count) *count = case_item->inner_ptr - word;
	
	return word;
}

LOCAL void case_frame_exe(CASE_ITEM *case_item)
{
	uint16_t    interval;
	uint16_t    i = 0;
	int			err;
	uint16_t	spacing;
	
	APP_ASSERT("Free pointer.\r\n", case_item!= NULL);
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
		err = sem_timedwait(&case_item->bf_ld_sem, &ts);
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
	if(interval<1) interval = 1;
	struct itimerspec its;
	memset(&its, 0, sizeof(struct itimerspec));
	its.it_value.tv_sec = interval / 1000;
    its.it_value.tv_nsec = (interval % 1000) * 1000000;
	timer_settime(case_item->timer,0,&its,NULL);

	if(!case_item->flag_hspeed) {
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
	}else{
		sendCMD_NextStep_tosunboard(case_item);
	}
	case_item->current_line ++;
	return;
}

LOCAL int8_t upload_to_subboard(struct Case_item *case_item)
{
	int fd_mid;
	uint8_t new_line;
	int i;
	char *c_val;
	int16_t *temp_buf;
	uint32_t index;
	size_t file_len;
	uint16_t count;
	struct rf_dev_msg *msg_start_load;
	PROTOCL_CTRL_MSG ioctrl_msg;

	ioctrl_msg.type = IO_CTRL_MSG_START_CASE_UPLOAD;
	ioctrl_msg.params = g_bus_obj;
	g_protocol_obj->ioctrl(&ioctrl_msg);

	case_item->val_count = 1;
	if(case_item->case_type & CASE_TYPE_ATT){
		temp_att = (int16_t*)malloc(sizeof(int16_t) * (case_item->ch_max+1));
		case_item->val_count += case_item->ch_max;
	}
	if(case_item->case_type & CASE_TYPE_PHA){
		temp_pha = (int16_t*)malloc(sizeof(int16_t) * (case_item->ch_max+1));
		case_item->val_count += case_item->ch_max;
	}
	
	if(!temp_buf){
		APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS,
			("temp_buf malloc failed.\r\n"));
		return CASE_ERROR;
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
		index =0;
		for(i=0; i<case_item->ch_max; i++){
			if(case_item->case_type & CASE_TYPE_ATT){
				if(new_line == 1){
					temp_att[index] = 0;
				}else{
					c_val = case_get_word(case_item, &new_line, &count);
					temp_att[index] = atoi(c_val);
				}
				index +=1;
			}
			if(case_item->case_type & CASE_TYPE_PHA){
				if(new_line == 1){
					temp_pha[index] = 0;
				}else{
					c_val = case_get_word(case_item, &new_line, &count);
					temp_pha[index] = atoi(c_val);
				}
				index +=1;
			}
		}

		/* get new line */
		while(!new_line)
			case_get_word(case_item, &new_line, &count);
		
		/* send data to sub board after */
		subbd_send_MCMV(DEST_UPLD_ATT, g_protocol_obj, g_bus_obj, case_item->cha_array, temp_att, case_item->ch_max);
		subbd_send_MCMV(DEST_UPLD_PHA, g_protocol_obj, g_bus_obj, case_item->cha_array, temp_pha, case_item->ch_max);		

		case_item->line_max += 1;
	}

	free(temp_att);
	free(temp_pha);
	return CASE_ERROR_OK;
}

LOCAL void case_frame_sender(void *param)
{
	rt_err_t err;
	struct Case_item *case_item = (struct Case_item *)param;

	APP_ASSERT("case_frame_sender: param == NULL.\r\n",param);

	err = rt_mq_send(s_mq_cs, &(case_item->exe_mis), sizeof(struct Case_mission*));
	if(err < 0){
		APP_DEBUGF(RFDEV_DEBUG | APP_DBG_TRACE | APP_DBG_LEVEL_SEVERE, 
			("case_frame_sender: msg send failed.(code:%d)\r\n",err));
	}

	return ;
}

LOCAL struct Case_item *upload_case(char *name)
{
	char *full_path = NULL;
	int fd;
	struct Case_item *case_item;
	uint8_t new_line;
	uint16_t count;
	char *c_case_type, *c_ch;
	uint16_t iter;
	uint16_t first_line = 0;
	int8_t res;
	
	APP_ASSERT("upload_case: Free pointer.\r\n", name!= NULL);

	full_path = (char*)malloc(strlen(s_folder)+strlen(name)+5);
	sprintf(full_path, "/%s/%s", s_folder, name);

	fd = open(full_path, O_RDONLY , 0);

	if(fd < 0){
		free(full_path);
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
	strncpy(case_item->case_name, name, CASE_NAME_LEN);

	case_item->full_path = full_path;

	c_case_type = case_get_word(case_item, &new_line, &count);
	first_line += count;
	case_item->case_type = atoi(c_case_type);
	if(!(case_item->case_type & CASE_TYPE_ATT) && 
		 !(case_item->case_type & CASE_TYPE_PHA)){
		goto end_failed;
	}
	case_item->ch_max = CHANNEL_BUFFER_ADD;
	case_item->cha_array = (uint16_t *)malloc(sizeof(uint16_t)*case_item->ch_max);	
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
			uint16_t *cha_array_temp = NULL;
			uint16_t cha_max_temp = case_item->ch_max + CHANNEL_BUFFER_ADD;
			cha_array_temp = (uint16_t *)malloc(sizeof(uint16_t)*cha_max_temp);	
			if(!cha_array_temp){
				APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS,
					("cha_array_temp malloc failed.\r\n"));		
				goto end_failed;
			}
			memset(cha_array_temp, 0, cha_max_temp*sizeof(uint16_t));
			memcpy(cha_array_temp, case_item->cha_array, case_item->ch_max*sizeof(uint16_t));
			free(case_item->cha_array);
			case_item->cha_array = cha_array_temp;
			case_item->ch_max = cha_max_temp;
		}
		if(c_ch){
			case_item->cha_array[iter] = atoi(c_ch);
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
    struct sigevent sev;
	memset(&sev, 0, sizeof(struct sigevent));
	sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = "case_timer";
    sev.sigev_notify_function = case_frame_sender;
    sev.sigev_notify_attributes = case_item;

    /* create timer */
    if (timer_create (CLOCK_REALTIME, &sev, &case_item->timer) == -1){
    	APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SERIOUS, ("timer_create failed.\r\n"));		
    	goto end_failed;
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
	if(case_item->full_path){
		free(case_item->full_path);
		case_item->full_path = NULL;
		full_path = NULL;
	}
	if(case_item)
		free(case_item);
	close(fd);
	return NULL;
}

LOCAL int16_t case_start(struct Case_item *case_item)
{
	rt_err_t err;
	uint16_t time_i = 0;
	int16_t res;

	APP_ASSERT("case_start: Free pointer.\r\n", case_item!= NULL);
	
	if(case_item->timer == NULL){
		case_item->timer = rt_timer_create(case_item->case_name,
												case_frame_sender, (void*)case_item,	1, RT_TIMER_FLAG_ONE_SHOT);
	}else{
		rt_timer_control(case_item->timer, RT_TIMER_CTRL_SET_TIME, (void*)&time_i);
	}
	if(!case_item->flag_hspeed){
	/* buffer loader begin */
		res = case_buffer_load_start(case_item);
		if(res < 0){
			APP_DEBUGF(CASE_DEBUG | APP_DBG_LEVEL_SERIOUS,
				("case_start: case_buffer_load_start failed.\r\n"));
			return CASE_ERROR;
		}
	}

	case_item->state = CASE_STATE_RUN;
	err = rt_timer_start(case_item->timer);
	
	return err;
}

LOCAL void case_runner_thread_entry(void* parameter)
{
	ssize_t size;
	CASE_MISSION_MSG case_mission;

	while(1){
        size= msgrcv(s_mq_cs, (void *)&case_mission, sizeof(CASE_MISSION_MSG)-sizeof(long), CASE_MISSION_MSG_ID, 0);
		if(size<0){
			APP_DEBUGF(CASE_M_DEBUG | APP_DBG_LEVEL_SEVERE, ("mq recive failed.(code:%d)\r\n",size));
			break;
		}
		switch(case_mission.mission_id){
			case CASE_MISSION_EXE:
				case_frame_exe((CASE_ITEM *)case_mission.data);
			break;
/*			case CASE_MISSION_LOAD:
				load_case((char*)case_mission.data);
				free(case_mission.data);
			break;*/
			case CASE_MISSION_HS_LOAD:
				upload_case((char*)case_mission.data);
				free(case_mission.data);
			break;
/*			case CASE_MISSION_UNLD:
				unload_case((CASE_ITEM *)case_mission.data);
			break;*/
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
}


/* Public functions ----------------------------------------------------------*/
void init_model_case_manager(json_object *case_json_obj)
{
	int protocol_id = 0;
    int bus_id = 0;

    APP_DEBUGF(CASE_M_DEBUG | APP_DBG_TRACE, ("initialize case manager.\r\n"));
    s_folder = config_get_string(case_json_obj, "CASE_PATH", "case/");

    protocol_id =   config_get_int(case_json_obj, "CASE_PROTOCOL", PROTOCOL_ID_RR485);
    if(protocol_id > SUBBD_PROTOCOL_SIZE) protocol_id = PROTOCOL_ID_RR485;
    g_protocol_obj = &protocols[protocol_id];

    bus_id =        config_get_int(case_json_obj, "CASE_BUS", BUS_ID_SPI);
    if(bus_id > BUS_DRIVER_NUM) bus_id = BUS_ID_SPI;
    g_bus_obj =     &bus_drivers[bus_id];

    init_runner_thread();
    
    /*
    rt_list_init(&case_item_root);
    regist_cmd_item_list(case_runner_cmd_items);*/
}
