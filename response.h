#ifndef _BACKEND_RESPONSE
#define _BACKEND_RESPONSE


#include <sysinc.hpp>
#include <3rd/json-builder.h>
#include "types.h"


class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void ResponseError(const char* error);
    void ResponseRet(const http_ret_t& ret);
    json_value* BeginWrite(const http_ret_t& r);
    void EndWrite(json_value* obj);
private:
    void ResponseHeader(int type);
    void ResponseVBody(const char *fmt,...);
};

#endif
