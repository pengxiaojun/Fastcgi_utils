#include "request.h"
#include "types.h"
#include "logger.h"
#include "querystring.h"
#include "upload.h"
#include <fcgi_stdio.h>
#include <3rd/json.h>
#include <limits.h>   //include ULONG_MAX


struct request_method_type_t
{
    char name[16];
    int value;
};

struct request_method_type_t methods[] = {
    {"", rm_unsupported},
    {"GET", rm_get},
    {"POST", rm_post},
    {"PUT", rm_put},
    {"HEAD", rm_head}
};

HttpRequest::HttpRequest():
    m_method(0),
    m_clen(0),
    m_action(0),
    m_querystrig(NULL),
    m_postdata(NULL),
    m_hqs(NULL),
    m_isupload(false)
{

}

HttpRequest::~HttpRequest()
{
    if (m_querystrig != NULL)
    {
        free(m_querystrig);
        m_querystrig = NULL;
    }

    if (m_postdata != NULL)
    {
        free(m_postdata);
        m_postdata = NULL;
    }
    if (m_hqs)
    {
        delete m_hqs;
        m_hqs = NULL;
    }
}

int HttpRequest::Prepare()
{
    m_isupload = false;

    const char *qs = GetHeader("QUERY_STRING");
    if (!strncmp(qs, "upload", strlen("upload")))
    {
        m_isupload = true;
        return 0;
    }
    size_t i;
    const char *head = GetHeader("REQUEST_METHOD");
    for (i = 0; head && i < (sizeof(methods)/sizeof(methods[0])); ++i)
    {
        if (!strcmp(head, methods[i].name))
        {
            m_method = methods[i].value;
            break;
        }
    }
    const char *clen = GetHeader("CONTENT_LENGTH");
    if (clen != NULL){
        m_clen = atoi(clen);
    }
    ReadQueryString();
    ReadPostData();
    if (m_method == rm_post)
    {
        return ParsePostRequest();
    }
    if (m_method == rm_get)
    {
        return ParseGetRequest();
    }
    return 0;
}

int HttpRequest::GetMethod()
{
    return m_method;
}

int HttpRequest::GetTarget()
{
    return m_target;
}

int HttpRequest::GetContentLength()
{
    return m_clen;
}

int HttpRequest::GetAction()
{
    return m_action;
}

std::string HttpRequest::GetSid()
{
    return m_sid;
}

const char* HttpRequest::GetQueryString()
{
    return m_querystrig;
}

const char* HttpRequest::GetPostData()
{
    return m_postdata;
}

bool HttpRequest::IsUpload()
{
    return m_isupload;
}

http_ret_t HttpRequest::UploadFile()
{
    http_ret_t r;
    r.code = upload_file_save_as("");
    return r;
}

const char* HttpRequest::GetHeader(const char* name)
{
    return getenv(name);
}

void HttpRequest::ReadQueryString()
{
    if (m_querystrig != NULL)
        return;

    char *qs = getenv("QUERY_STRING");
    if (qs != NULL)
    {
        m_querystrig = strdup(qs);
    }
}

void HttpRequest::ReadPostData()
{
    m_postdata = (char*)calloc(1, m_clen);
    int nread = fread(m_postdata, 1, m_clen, FCGI_stdin);
    log_debug("[%s:%d]Read %d bytes post data:%s", __FILE__, __LINE__, nread, m_postdata);
    if (nread != m_clen)
    {
        log_error("[%s:%d] Less read(%d<%d) post data", __FILE__, __LINE__, nread, m_clen);
    }
}

int HttpRequest::ParseGetRequest()
{
    //parse query string like '?a=b&c=1'
    if (m_querystrig == NULL)
    {
        log_error("[%s:%d] GET no query string", __FILE__, __LINE__);
        return -1;
    }
    m_hqs = new HttpQueryString();
    if (m_hqs->Parse(m_querystrig) != 0)
    {
        log_error("[%s:%d] Invalid query string %s", __FILE__, __LINE__, m_querystrig);
        return -1;
    }

    //parse target
    std::string target = m_hqs->Get("target");
    if (target.empty())
    {
        log_error("[%s:%d] Not specify target in query string %s", __FILE__, __LINE__, m_querystrig);
        return -1;
    }

    if (target == "nvr")
    {
        m_target = rt_nvr;
    }
    else if (target == "tvw")
    {
        m_target = rt_tvw;
    }
    else{
        m_target = rm_unsupported;
    }

    //parse action
    std::string action = m_hqs->Get("action");
    if (action.empty())
    {
        log_error("[%s:%d] Not specify action in query string %s", __FILE__, __LINE__, m_querystrig);
        return -1;
    }
    unsigned long int r = strtoul(action.c_str(), NULL, 0);
    if (r == ULONG_MAX)
    {
        log_error("[%s:%d] Action is invalid in query string %s", __FILE__, __LINE__, m_querystrig);
        return -1;
    }
    m_action = (int)r;
    //parse session id. session id will be empty before login to tvw
    m_sid = m_hqs->Get("sid");
    return 0;
}


int HttpRequest::ParsePostRequest()
{
    //parse post data in json fomrat
    json_char *json;
    json_value *value;

    json = (json_char*)m_postdata;
    value = json_parse(json, m_clen);

    if (value == NULL)
    {
        log_error("[%s:%d] Unable to parse json post data %s", __FILE__, __LINE__, m_postdata);
        return -1;
    }

    if (value->type != json_object)
    {
        log_error("[%s:%d] Json post data is Not a object %s", __FILE__, __LINE__, m_postdata);
        json_value_free(value);
        return -1;
    }

    int length, x;
    char *name;
    json_value *attr;

    length = value->u.object.length;
    for (x = 0; x<length; ++x)
    {
        name = value->u.object.values[x].name;
        attr = value->u.object.values[x].value;
        if (!strcmp(name, "target"))
        {
            if (attr->type != json_string || attr->u.string.ptr == NULL)
            {
                log_error("[%s:%d] Target json value is not string. %s", __FILE__, __LINE__, m_postdata);
                json_value_free(value);
                return -1;	
            }
            if (!strcmp(attr->u.string.ptr, "nvr"))
            {
                m_target = rt_nvr;
            }
            else if (!strcmp(attr->u.string.ptr, "tvw"))
            {
                m_target =  rt_tvw;
            }
        }
        else if (!strcmp(name, "action"))
        {
            if (attr->type != json_integer)
            {
                log_error("[%s:%d] Action json value is not a number. %s", __FILE__, __LINE__, m_postdata);
                json_value_free(value);
                return -1;	
            }
            m_action = attr->u.integer;
        }
        else if (!strcmp(name, "sid"))
        {
            if (attr->type == json_string && attr->u.string.ptr != NULL)
            {
                m_sid = std::string(attr->u.string.ptr);
            }
        }
    }
    log_debug("[%s:%d]target=%d action=%d sid=%s", __FILE__, __LINE__, m_target, m_action, m_sid.c_str());
    json_value_free(value);
    return 0;
}
