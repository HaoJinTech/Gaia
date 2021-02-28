#include "platform.h"
#include "sys_config.h"
#include "tcp_server.h"

#include <stdio.h>

int main(void)
{
    int32_t ret = RET_OK;
    config_init();

    ret = tcp_server_init();

    return ret;
}

