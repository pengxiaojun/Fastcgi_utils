#ifndef _LOGGER
#define _LOGGER


#include <sysinc.h>


#ifdef __cpluscplus
extern "C" {
#endif

typedef enum{
   bll_debug = 1,
   bll_info = 2, 
   bll_warn = 3,
   bll_error = 4,
}backend_log_level_t;


void GRCALL log_startup();
void GRCALL log_cleanup();

void GRCALL log_debug(const char *fmt,...);
void GRCALL log_info(const char *fmt,...);
void GRCALL log_warn(const char *fmt,...);
void GRCALL log_error(const char *fmt, ...);



#ifdef __cpluscplus
}
#endif



#endif
