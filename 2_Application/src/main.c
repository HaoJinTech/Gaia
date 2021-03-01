#include "platform.h"
#include "sys_config.h"
#include "tcp_server.h"
#include "cmd_manager.h"

#include <stdio.h>

int main(void)
{
    int32_t ret = RET_OK;
    config_init();

    ret = init_cmd_manager();
    ret = tcp_server_init();

    return ret;
}

