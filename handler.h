#ifndef _HANDLER
#define _HANDLER

#include <sysinc.hpp>
#include "types.h"

class HttpRequest;
class HttpResponse;

/*
 *Deride from this class to handler your request
 *
 */

class HttpHandler
{
public:
    HttpHandler();
    virtual ~HttpHandler();
    virtual void Initialize();
    virtual void Process(HttpRequest* request, HttpResponse *response);
    virtual int GetType();
private:

private:
};

#endif
