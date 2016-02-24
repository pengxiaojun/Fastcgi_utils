#ifndef _CONTROLLER
#define _CONTROLLER

#include <sysinc.hpp>

class HttpRequest;
class HttpHandler;


class HttpController
{
public:
    HttpController();
    ~HttpController();

    void RegisterHandler(HttpHandler *handler);
    void UnRegisterHandler(int target);

    HttpHandler* LookupHandler(HttpRequest* request);
private:
    void Initialize();
private:
    std::map<int, HttpHandler*> m_handlers;	
};

#endif
