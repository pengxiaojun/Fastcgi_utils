#include <sysinc.h>
#include <fcgi_stdio.h>
#include <execinfo.h>
#include "types.h"
#include "request.h"
#include "response.h"
#include "handler.h"
#include "controller.h"
#include "trace.h"
#include "logger.h"
#include "config.h"


#ifndef WIN32
void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
    log_error("Segment fault at address %p", si->si_addr);

    void *buffer[64];
    char **strings;
    int i, nptrs;

    nptrs = backtrace(buffer, 64);
    strings = backtrace_symbols(buffer, nptrs);

    for (i = 0; i<nptrs; ++i)
    {
        log_error("%s", strings[i]);
    }
    free(strings);
    exit(0);
}
#else
#endif

void initialize()
{
#ifndef WIN32
    //setup signal
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
#endif

    //load config
    backend_load_config("backend.ini");
}

void event_loop()
{
    HttpController controller;

    log_info("Event loop starts");

    while (FCGI_Accept() >= 0)
    {
#ifdef DEBUG
        trace();
#endif
        HttpResponse response;
        HttpRequest request;
        if (request.Prepare() < 0)
        {
            response.ResponseError("Invalid http request");	
            continue;
        }
        if (request.IsUpload())
        {
            response.ResponseRet(request.UploadFile());
            continue;
        }

        HttpHandler *handler = controller.LookupHandler(&request);
        if (handler == NULL){
            response.ResponseError("No hanlders for this request");	
            continue;
        }
        handler->Process(&request, &response);
    }//end while
    log_info("Event loop stops");
}

int main(int argc, const char *argv[])
{
    if (argc > 1)
    {
#include "fcgi_test.h"
        //test mode
        if (!strcmp(argv[1], "-t"))
        {
            fcgi_test(argc, argv);
            return 0;
        }
        else if (!strcmp(argv[1], "-i"))
        {
            fcgi_interactive();
            return 0;
        }
    }
    initialize();
    event_loop();
    return 0;
}
