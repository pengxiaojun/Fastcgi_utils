#include "trace.h"
#include "logger.h"

#define MAX_HEADERS 32

static char* headers[MAX_HEADERS] = {
    "DOCUMENT_ROOT",
    "GATEWAY_INTERFACE",
    "HTTP_ACCEPT",
    "HTTP_ACCEPT_ENCODING",
    "HTTP_ACCEPT_LANGUAGE",
    "HTTP_CACHE_CONTROL",
    "HTTP_CONNECTION",
    "HTTP_HOST",
    "HTTP_PRAGMA",
    "HTTP_RANGE",
    "HTTP_REFERER",
    "HTTP_TE",
    "HTTP_USER_AGENT",
    "HTTP_X_FORWARDED_FOR",
    "PATH",
    "QUERY_STRING",
    "REMOTE_ADDR",
    "REMOTE_HOST",
    "REMOTE_PORT",
    "REQUEST_METHOD",
    "REQUEST_URI",
    "SCRIPT_FILENAME",
    "SCRIPT_NAME",
    "SERVER_ADDR",
    "SERVER_ADMIN",
    "SERVER_NAME",
    "SERVER_PORT",
    "SERVER_PROTOCOL",
    "SERVER_SIGNATURE",
    "SERVER_SOFTWARE",
    "Content-Disposition",
    "Content-Type"
};

void trace()
{
    int i;
    char *v;
    i = 0;
    for (; i<MAX_HEADERS; ++i)
    {
        v = getenv(headers[i]);
        if (v == NULL){
            log_debug("%s:", headers[i]);
        }else{
            log_debug("%s: %s", headers[i], v);
        }
    }
}
