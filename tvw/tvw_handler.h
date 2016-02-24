#ifndef _TVW_HANDLER
#define _TVW_HANDLER

#include <sysinc.hpp>
#include "types.h"
#include "handler.h"
#include "session_manager.h"
#include "session_parser.h"


class HttpRequest;
class HttpResponse;


class HttpTVWHandler : public HttpHandler
{
public:
    HttpTVWHandler();
    virtual ~HttpTVWHandler();

    virtual void Initialize();
    virtual int GetType();
    virtual void Process(HttpRequest* request, HttpResponse *response);
private:
    void DoLogin(HttpRequest* request, HttpResponse *response);
    void DoLogout(HttpRequest* request, HttpResponse *response);
    void QueryLoginAck(HttpRequest* request, HttpResponse *response);
    void QueryData(HttpRequest* request, HttpResponse *response);
    void QueryDataAck(HttpRequest* request, HttpResponse *response, int action);
    void QueryNotice(HttpRequest* request, HttpResponse *response, int action);
    void QueryNetworkStat(HttpRequest* request, HttpResponse *response, int action);
    void QueryNetwork(HttpRequest* request, HttpResponse *response, int action);
    void Control(HttpRequest* request, HttpResponse *response, int action);
    //
    Session* HasLogin(HttpRequest *request, http_ret_t& r);
private:
    SessionManager m_sm;
    SessionDataParser m_parser;
};


#endif
