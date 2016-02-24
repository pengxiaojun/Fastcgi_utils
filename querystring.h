#ifndef _QUERYSTRING
#define _QUERYSTRING

#include <sysinc.hpp>

class HttpQueryString
{
public:
    HttpQueryString();
    ~HttpQueryString();

    int Parse(const char *str);
    std::string Get(const std::string& key);
private:
    int ParsePhase(char *phase);
private:
    std::map<std::string, std::string> m_qs;
};

#endif
