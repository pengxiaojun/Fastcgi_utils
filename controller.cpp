#include "controller.h"
#include "handler.h"
#include "request.h"
#include "tvw/tvw_handler.h"


HttpController::HttpController()
{
    Initialize();
}

HttpController::~HttpController()
{
}

void HttpController::Initialize()
{
    HttpHandler *handler = new HttpTVWHandler();
    handler->Initialize();
    RegisterHandler(handler);
}

void HttpController::RegisterHandler(HttpHandler* handler)
{
    if (handler == NULL)
        return;

    m_handlers.insert(std::make_pair(handler->GetType(), handler));
}

void HttpController::UnRegisterHandler(int target)
{
    m_handlers.erase(target);
}

HttpHandler* HttpController::LookupHandler(HttpRequest* request)
{
    if (request == NULL)
        return NULL;

    std::map<int, HttpHandler*>::iterator it;

    it = m_handlers.find(request->GetTarget());
    if (it == m_handlers.end())
        return NULL;

    return it->second;
}
