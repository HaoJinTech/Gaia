#include "platform.h"
#include "sys_config.h"
#include "tcp_server.h"
#include "cmd_manager.h"
#include "bll_manager.h"
#include "subboard_manager.h"
#include "app_debug.h"

#include <stdio.h>

#if 0
int main(void)
{
    test_loop();

    json_object *obj = 0;

    config_init();

    obj = json_object_object_get(config_json_obj, "CASE");
    if(obj) {
        init_model_case_manager(obj);            
    }
    send_upload_misson("case1");
    while(1){
        sleep(1);
    }
}
#else
int main(void)
{
    int32_t ret = RET_OK;
    log_dbg_init();
    config_init();
    ret = init_rfboard_manager();
    ret = init_bll_manager();
    ret = init_cmd_manager();
    ret = tcp_server_init();

    return ret;
}
#endif
