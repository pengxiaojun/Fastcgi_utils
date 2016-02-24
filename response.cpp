#include "response.h"
#include "logger.h"
#include <fcgi_stdio.h>

#define MAX_CONTENT_TYPE 5

static const char *content_types[] = {
    "unsupported",
    "application/json;charset=utf8",
    "text/html;charset=utf8",
    "text/plain;charset=utf8",
    "application/octec-stream"
};


HttpResponse::HttpResponse()
{
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::ResponseError(const char* error)
{
    http_ret_t r;
    r.code = -1;
    r.action = 0;
    r.message = std::string(error);
    ResponseRet(r);
}

void HttpResponse::ResponseRet(const http_ret_t& r)
{
    ResponseHeader(rc_json);
    ResponseVBody("{\"retcode\":%d, \"action\":%d,\"message\":\"%s\"}", r.code, r.action, r.message.c_str());
}


json_value* HttpResponse::BeginWrite(const http_ret_t& r)
{
    json_value* obj = json_object_new(0);
    json_value *ret = json_integer_new((json_int_t)r.code);
    json_value *act = json_integer_new((json_int_t)r.action);
    json_value *msg = json_string_new((const json_char*)r.message.c_str());
    json_object_push(obj, (const json_char*)"retcode", ret);
    json_object_push(obj, (const json_char*)"action", act);
    json_object_push(obj, (const json_char*)"message", msg);
    return obj;
}

void HttpResponse::EndWrite(json_value* obj)
{
    if (obj == NULL)
        return;

    char *buf = (char*)malloc(json_measure(obj));
    json_serialize(buf, obj);

    FCGI_printf("Content-type: %s\r\n\r\n", content_types[1]);
    FCGI_printf("%s", buf);
    log_debug(buf);
    free(buf);

    json_builder_free(obj);
}

void HttpResponse::ResponseHeader(int type)
{
    if (type < 0 || type >= MAX_CONTENT_TYPE)
    {
        FCGI_printf("Content-type: %s\r\n\r\n", content_types[0]);
        return;
    }
    FCGI_printf("Content-type: %s\r\n\r\n", content_types[type]);
}

void HttpResponse::ResponseVBody(const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    FCGI_vfprintf(FCGI_stdout, fmt, args);
    va_end(args);
}
