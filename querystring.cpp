#include "querystring.h"

HttpQueryString::HttpQueryString()
{

}

HttpQueryString::~HttpQueryString()
{
}

std::string HttpQueryString::Get(const std::string& key)
{
    std::map<std::string, std::string>::iterator it;
    it = m_qs.find(key);

    if (it == m_qs.end())
        return std::string();
    return it->second;
}

int HttpQueryString::Parse(const char *url)
{
    if (url == NULL){
        return -1;
    }

    char *phase;
    char *urlstr;

    urlstr = (char*) url;

    for (; (phase = strtok(urlstr, "&")) != NULL; urlstr = NULL)
    {
        if (ParsePhase(phase) != 0){
            return -1;
        }
    }
    return 0;
}

int HttpQueryString::ParsePhase(char *phase)
{
    if (phase == NULL){
        return -1;
    }
    char *vname, *val;
    int c;
    char buf[3];
    char *r;

    //skip ?
    if (*phase == '?'){
        phase++;
    }
    vname = phase;
    //find equal quota
    while (*phase && (*phase != '=')) phase++;

    if (!*phase){
        return -1;
    }
    *(phase++) = '\0';

    std::vector<char> v(strlen(phase)+1);
    r = &v[0];

    for (val = phase; *val; val++)
    {
        switch(*val)
        {
            case '%':
                buf[0] = *(++val); buf[1] = *(++val);
                buf[2] = '\0';
                sscanf(buf, "%02x", &c);
                break;
            case '+':
                c = ' ';
                break;
            default:
                c = *val;
        }
        switch(c)
        {
            case '\\': *r++='\\'; *r++='\\'; break;
            case '\'': *r++='\\'; *r++='\''; break;
            case '\n': *r++='\\'; *r++='n'; break;
            default: *r++ = c; break;
        }
    }

    std::string name(vname);
    std::string value(v.begin(), v.end());
    m_qs.insert(std::make_pair(name, value));
    return 0;
}
