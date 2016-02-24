#ifndef TVW_CONFIG_H
#define TVW_CONFIG_H

#include <sysinc.hpp>

typedef struct{
    int log_level; //enable debug
    int serv_port; //server port
}backend_cfg_t;

void backend_load_config(const char *file);

const backend_cfg_t *backend_get_cfg();

#endif
