#include "logger.h"
#include "config.h"
#include <grutil.h>


void GRCALL log_trace(const char *level, int nlevel, const char *fmt, va_list args);

/* 
 *  Don't use any function in stdio. those functions redefined by fastcgi
 *  this logger module may be replaced
 */
void GRCALL log_startup()
{
}

void GRCALL log_cleanup()
{
}

void GRCALL log_debug(const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    log_trace("DEBUG ", bll_debug, fmt, args);
    va_end(args);
}

void GRCALL log_info(const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    log_trace("INFO ", bll_info, fmt, args);
    va_end(args);
}

void GRCALL log_warn(const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    log_trace("INFO ", bll_warn, fmt, args);
    va_end(args);
}


void GRCALL log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_trace("ERROR ", bll_error, fmt, args);
    va_end(args);
}

void GRCALL log_trace(const char *level, int nlevel, const char *fmt, va_list args)
{
    const backend_cfg_t *cfg = backend_get_cfg();
    if (nlevel < cfg->log_level)
        return;

    char time[32];
    char dst[4096] = {0};
    struct timeval tv;
    struct tm tm;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);
    sprintf(time, "%04d-%02d-%02d %02d:%02d:%02d.%03d ",
            1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (int)tv.tv_usec / 1000);

    vsnprintf(dst, 4096, fmt, args);

    int fd = open("backend.log", O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
    write(fd, time, strlen(time));
    write(fd, level, strlen(level));
    write(fd, dst, strlen(dst));
    write(fd, "\n", 1);
    close(fd);
}

