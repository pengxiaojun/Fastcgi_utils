#ifndef _FCGI_TEST
#define _FCGI_TEST

#include <sysinc.hpp>
#include <3rd/json-builder.h>


class Session;
class SessionDataParser;

void fcgi_test(int argc, const char *argv[]);
void fcgi_interactive();

class FCGITest
{
public:
    FCGITest();
    ~FCGITest();
    bool IsLogin();

    void TestLogin(const char *peer);
    void TestLogout();
    void TestQueryData(int type=0xffff);
    void TestSerialize();
    void TestSerializeAck(int action);

    void TestScreenSplit(int index, int row, int col);
    void TestCtrlViewVideo();
    void TestNotice(int action);

    void TestQueryNetwork();
    void TestCtrlMachine(int action);
    void TestCtrlNVR(int action, const char *ip, const char *user, const char *pass);
    void TestCtrlFMP(const char *ip, const char *user, const char *pass);
    void TestCtrlNetwork(const char *name, const char *ip, const char *mask);
    void TestConfigHost(const char *name, const char *dns);
private:
    void PrintJsonData(json_value *obj);
private:
    Session *m_session;
    SessionDataParser *m_parser;
};

#endif
