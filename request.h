#ifndef _BACKEND_REQUEST
#define _BACKEND_REQUEST


#include <sysinc.hpp>

struct http_ret_t;

class HttpQueryString;

class HttpRequest
{
public: 
    HttpRequest();
    ~HttpRequest();

    int Prepare();
    int GetMethod();
    int GetContentLength();
    int GetTarget();
    int GetAction();
    std::string GetSid();
    const char* GetQueryString();
    const char* GetPostData();
    bool IsUpload();
    http_ret_t UploadFile();
private:
    const char* GetHeader(const char* name);
    void ReadQueryString();
    void ReadPostData();
    int ParsePostRequest();
    int ParseGetRequest();
    int HandleFileUpload();
private:
    int m_method;  //request method
    int m_clen;    //content length
    int m_target;  //which target will process request
    int m_action; //perform action
    char* m_querystrig;
    char* m_postdata;
    std::string m_sid;    //session id
    HttpQueryString *m_hqs;
    bool m_isupload;
};

#endif
