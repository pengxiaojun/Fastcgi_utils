#include "config.h"
#include "logger.h"
#include <grutil.h>


static backend_cfg_t g_cfg;

static void backend_cfg_handler(const char *key, const char *val, void *ud)
{
    if (!strcmp(key, "log.level"))
    {
        g_cfg.log_level = atoi(val);
    }
    else if (!strcmp(key, "server.port"))
    {
        g_cfg.serv_port = atoi(val);
    }
}

void backend_load_config(const char *file)
{
    memset(&g_cfg, 0, sizeof(g_cfg));
    g_cfg.log_level = bll_info; //default log info
    g_cfg.serv_port = 4555;     //default server port

    int r;
    r = gr_parse_ini(file, backend_cfg_handler, NULL);

    if (r != 0){
        log_error("Open %s error %d", file, r);
        return;
    }
}

const backend_cfg_t* backend_get_cfg()
{
    return &g_cfg;
}
