#ifndef _UPLOAD
#define _UPLOAD


#include <sysinc.h>


#define EUPLOAD_SUCCESS 0 
#define EUPLOAD_NO_DATA -1
#define EUPLOAD_READ    -2
#define EUPLOAD_WRITE   -3


#ifdef __cpluscplus
extern "C" {
#endif

int GRCALL upload_file_save_as(const char *save_path);

#ifdef __cpluscplus
}
#endif


#endif
