#include "fcgi_test.h"
#include "tvw/session.h"


void fcgi_test(int argc, const char *argv[])
{
    const char *default_srv = "192.168.1.25:4555";
    if (argc > 2)
    {
        default_srv = argv[2];
    }
    printf("Login to [%s]...\n", default_srv);
    FCGITest test;
    test.TestLogin(default_srv);
    gr_sleep(2000);
    if (!test.IsLogin())
    {
        fprintf(stderr, "Login to %s failure\n", argv[2]);
        return;
    }
    fprintf(stderr, "Login to %s suceess\n", argv[2]);
    test.TestQueryData();
    gr_sleep(1000);
    test.TestSerialize();
    test.TestSerializeAck(ra_basic_ack);
    test.TestSerializeAck(ra_camera_ack);
    test.TestSerializeAck(ra_localmovie_ack);
    test.TestSerializeAck(ra_flash_ack);
    test.TestSerializeAck(ra_layout_ack);
    test.TestSerializeAck(ra_hotpoint_ack);
    test.TestSerializeAck(ra_map_ack);
    test.TestSerializeAck(ra_layout_loop_ack);
    test.TestSerializeAck(ra_screen_ack);
    test.TestSerializeAck(ra_view_ack);
    test.TestSerializeAck(ra_nvr_ack);

    test.TestScreenSplit(0, 2, 2);
    gr_sleep(200);
    test.TestNotice(ra_screen_stat);
    test.TestNotice(ra_view_geo_stat);
    test.TestNotice(ra_camera_stat);
    gr_sleep(1000);
    test.TestNotice(ra_view_geo_stat);

    test.TestNotice(ra_layout_stat);
    gr_sleep(1000);
    test.TestNotice(ra_layout_loop_stat);

    test.TestQueryNetwork();
    gr_sleep(1000);
    test.TestSerializeAck(ra_network_ack);

    //test.TestCtrlMachine(PKT_CTRL::MachineCtrl::NVR_ADD);
    gr_sleep(1000);
    //test.TestCtrlMachine(PKT_CTRL::MachineCtrl::NVR_DEL);
    gr_sleep(1000);
    //test.TestCtrlMachine(PKT_CTRL::MachineCtrl::NET_INTERFACE);

    //modify network
}

FCGITest::FCGITest()
{
    m_session = new Session();
}

FCGITest::~FCGITest()
{
    delete m_session;
    m_session = NULL;
}

bool FCGITest::IsLogin()
{
    return m_session->IsEstablish();
}

void FCGITest::PrintJsonData(json_value *obj)
{
    char *buf = (char*)malloc(json_measure(obj));
    json_serialize(buf, obj);
    printf("\n\nJSON[[[\n");
    printf("%s", buf);
    printf("\n]]]\n");
    free(buf);
}

void FCGITest::TestLogin(const char *peer)
{
    PKT_SESSION::Login login;
    login.set_version(peer);
    login.set_username("admin");
    login.set_passwd("admin");
    m_session->Connect(login);
}

void FCGITest::TestLogout()
{
    m_session->Logout();
}

void FCGITest::TestQueryData(int type)
{
    m_session->QueryData(type);
}

void FCGITest::TestSerialize()
{
    json_value *obj = json_object_new(0);
    m_session->SerializeData(obj);

    char *buf = (char*)malloc(json_measure(obj));
    json_serialize(buf, obj);
    printf("%s", buf);
    free(buf);
}

void FCGITest::TestSerializeAck(int action)
{
    json_value *obj = json_object_new(0);
    m_session->SerializeAck(obj, action);
    PrintJsonData(obj);
}

void FCGITest::TestScreenSplit(int index, int row, int col)
{
    printf("\n-----------------screen 0 split----------------------\n");
    PKT_CTRL::ScreenSplitCtrl ssc;
    ssc.set_screenindex(index);
    ssc.set_row(row);
    ssc.set_col(col);
    m_session->CtrlScreenSplit(ssc);
}

void FCGITest::TestCtrlViewVideo()
{
}

void FCGITest::TestNotice(int action)
{
    printf("Query notice %d\n", action);
    json_value *obj = json_object_new(0);
    m_session->SerializeNotice(obj, action);
    PrintJsonData(obj);
}

void FCGITest::TestQueryNetwork()
{
    m_session->QueryNetwork();
}

void FCGITest::TestCtrlMachine(int action)
{
    PKT_CTRL::MachineCtrl mc;
    mc.set_type((PKT_CTRL::MachineCtrl::MachineCtrlType)action);

    if (action == PKT_CTRL::MachineCtrl::REBOOT ||
        action == PKT_CTRL::MachineCtrl::SOFT_REBOOT ||
        action == PKT_CTRL::MachineCtrl::POWEROFF)
    {
        printf("[Machine]reboot|poweroff\n");
    }
    else if (action == PKT_CTRL::MachineCtrl::NET_HOST)
    {
        ::TvWall::Network *net = mc.mutable_network();
        net->set_hostname("Hello");
        net->set_dns("202.96.128.166");
        net->set_nat("nat");
        printf("[Machine]setting host\n");
    }
    else if (action == PKT_CTRL::MachineCtrl::NET_GATEWAY)
    {
        ::TvWall::Network *net = mc.mutable_network();
        net->set_gateway("192.168.1.1");
        net->set_gwdev("eth0");
        printf("[Machine]setting gateway\n");
    }
    else if (action == PKT_CTRL::MachineCtrl::NET_INTERFACE)
    {
        ::TvWall::NetInterface *it = mc.mutable_netinterface();
        it->set_ifname("enp4s0");
        it->set_ip("192.168.1.69");
        it->set_netmask("255.255.255.0");
        printf("[Machine]setting network interface\n");
    }
    else if (action == PKT_CTRL::MachineCtrl::NVR_ADD ||
            action == PKT_CTRL::MachineCtrl::NVR_DEL)
    {
        ::TvWall::NVR *nvr = mc.mutable_nvr();
        nvr->set_ip("192.168.1.228");
        nvr->set_username("admin");
        nvr->set_password("admin");
        printf("[Machine]add|del NVR\n");
    }
    m_session->CtrlMachine(mc);
}

void FCGITest::TestCtrlNVR(int action, const char *ip, const char *user, const char *pass)
{
    PKT_CTRL::MachineCtrl mc;
    mc.set_type((PKT_CTRL::MachineCtrl::MachineCtrlType)action);

    ::TvWall::NVR *nvr = mc.mutable_nvr();
    nvr->set_ip(ip);
    nvr->set_username(user);
    nvr->set_password(pass);
    m_session->CtrlMachine(mc);
}

void FCGITest::TestCtrlFMP(const char *ip, const char *user, const char *pass)
{
    PKT_CTRL::MachineCtrl mc;
    mc.set_type(PKT_CTRL::MachineCtrl::FMP);

    ::TvWall::FMP *fmp= mc.mutable_fmp();
    fmp->set_ip(ip);
    fmp->set_username(user);
    fmp->set_password(pass);
    m_session->CtrlMachine(mc);
}

void FCGITest::TestCtrlNetwork(const char *name, const char *ip, const char *mask)
{
    PKT_CTRL::MachineCtrl mc;
    mc.set_type(PKT_CTRL::MachineCtrl::NET_INTERFACE);
    ::TvWall::NetInterface *it = mc.mutable_netinterface();
    it->set_ifname(name);
    it->set_ip(ip);
    it->set_netmask(mask);
    m_session->CtrlMachine(mc);
}

void FCGITest::TestConfigHost(const char *name, const char *dns)
{
    PKT_CTRL::MachineCtrl mc;
    mc.set_type(PKT_CTRL::MachineCtrl::NET_HOST);
    ::TvWall::Network *net = mc.mutable_network();
    net->set_hostname(name);
    net->set_dns(dns);
    m_session->CtrlMachine(mc);
}
