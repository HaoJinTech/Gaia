#include "sys_config.h"
#include "json.h"
#include "platform.h"

json_object *pobj;

void config_init(void){
    pobj = json_object_from_file(SYS_CONFIG_PATH);

}

