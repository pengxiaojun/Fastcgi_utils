#include "fcgi_test.h"
#include "tvw/session.h"

static FCGITest g_test;

static int get_input(char *input, int len, const char *indent)
{
    if (indent){
        fprintf(stdout, "%s: ",  indent);
        fflush(stdout);
    }
    if (fgets(input, len, stdin) == NULL)
        return -1;

    input[strlen(input) - 1] = '\0';
    return 0;
}

static void assert_login()
{
    if (!g_test.IsLogin())
    {
        fprintf(stderr, "Not login yet, Bye !\n");
        exit(0);
    }
}

bool confirm(const char *hint)
{
    char ack[8];
    printf("%s [Yes/No] ? ", hint);
    fflush(stdout);
    get_input(ack, 8, NULL);

    if (!strcmp(ack, "Yes") ||
        !strcmp(ack, "Y") ||
        !strcmp(ack, "y"))
    {
        return true;
    }
    return false;
}

static void do_help();
static void do_login();
static void do_logout();
static void do_query_biz();
static void do_query_stat();
static void do_query_net();
static void do_ctrl_screen();
static void do_add_nvr();
static void do_del_nvr();
static void do_config_network();
static void do_config_host();
static void do_set_fmp();

struct fn_object_t{
    char cmd[8];
    char desc[64];
    void (*fn)(void);
};

struct fn_object_t fn_table[] = {
    {"h", "show this message", do_help},
    {"q", "quit program", do_logout},
    {"in", "login to tvwall", do_login},
    {"qb", "query biz data", do_query_biz},
    {"qs", "query status", do_query_stat},
    {"qn", "query network", do_query_net},
    {"css", "control screen split", do_ctrl_screen},
    {"an", "add nvr", do_add_nvr},
    {"dn", "delete nvr", do_del_nvr},
    {"cni", "ccnfigure network interface", do_config_network},
    {"ch", "change host name", do_config_host},
    {"sf", "set fmp", do_set_fmp}
};


static void do_help()
{
    size_t i;

    for (i = 0; i < sizeof(fn_table)/sizeof(fn_table[0]); ++i)
    {
        struct fn_object_t obj = fn_table[i];
        printf("%-8s-- %s\n", obj.cmd, obj.desc);
    }
}

static void do_login()
{
    char peer[32] = {0};
    char *local = "127.0.0.1:4555";
    if (get_input(peer, 32, "[127.0.0.1:4555]? ") < 0)
    {
        strcpy(peer, local);
    }
    if (strlen(peer) == 0)
        strcpy(peer, local);

    fprintf(stderr, "Login to %s...\n", peer);
    g_test.TestLogin(peer);
    gr_sleep(1000);
    assert_login();
    fprintf(stderr, "Login to %s success \n", peer);
}

static void do_logout()
{
    printf("Logout\n");
    g_test.TestLogout();
    exit(0);
}

static void do_query_biz()
{
    assert_login();
    char type[8];
    int t;
    if (get_input(type, 8, "[bizdata type]? ") < 0)
        t = 0xffff;
    else
        t = atoi(type);

    if (t <= 0)
        t = 0xffff;

    printf("Query bizdata %d\n", t);
    g_test.TestQueryData(t);
    gr_sleep(3000);

    g_test.TestSerializeAck(ra_basic_ack);
    g_test.TestSerializeAck(ra_camera_ack);
    g_test.TestSerializeAck(ra_localmovie_ack);
    g_test.TestSerializeAck(ra_flash_ack);
    g_test.TestSerializeAck(ra_layout_ack);
    g_test.TestSerializeAck(ra_hotpoint_ack);
    g_test.TestSerializeAck(ra_map_ack);
    g_test.TestSerializeAck(ra_layout_loop_ack);
    g_test.TestSerializeAck(ra_screen_ack);
    g_test.TestSerializeAck(ra_view_ack);
    g_test.TestSerializeAck(ra_nvr_ack);
    g_test.TestSerializeAck(ra_fmp_ack);
}

static void do_query_stat()
{
    assert_login();
    char type[8];
    int t;

    if (get_input(type, 8, "[stat type]? ") < 0)
        t = 0xffff;
    else
        t = atoi(type);

    if (t <= 0)
        t = 0xffff;

    if (t == 0xffff)
    {
        printf("Query all notice...\n");
        g_test.TestNotice(ra_screen_stat);
        g_test.TestNotice(ra_view_geo_stat);
        g_test.TestNotice(ra_layout_stat);
        g_test.TestNotice(ra_layout_loop_stat);
        g_test.TestNotice(ra_camera_stat);
        g_test.TestNotice(ra_network_stat);
        g_test.TestNotice(ra_hotpoint_stat);
        g_test.TestNotice(ra_record_stat);
    }
    else{
        printf("Query notice %d...\n", t);
        g_test.TestNotice(t);
    }
}

static void do_query_net()
{
    assert_login();
    printf("Query network...\n");
    g_test.TestQueryNetwork();
    gr_sleep(2000);
    g_test.TestSerializeAck(ra_network_ack);
}

static void do_ctrl_screen()
{
    assert_login();

    char index[4];
    char row[4];
    char col[4];
    
    int i, r, c;

    if (get_input(index, 4, "[index]? ") < 0)
        i = 0;
    else
        i = atoi(index);
            
    if (get_input(row, 4, "[row]? ") < 0)
        r = 2;
    else
        r = atoi(row);
    if (get_input(col, 4, "[col]? ") < 0)
        c = 2;
    else
        c = atoi(col);

    printf("Split screen %d to %dx%d\n", i, r, c);
    g_test.TestScreenSplit(i, r, c);
    printf("Query screen status...\n");
    gr_sleep(300);
    g_test.TestNotice(ra_screen_stat);
}

static void add_del_nvr(int action)
{
    assert_login();

    char ip[32];
    char user[16];
    char pass[16];

    if (get_input(ip, sizeof(ip), "[ip]? ") < 0)
        strcpy(ip, "192.168.1.25");
    if (get_input(user, sizeof(user), "[user]? ") < 0)
        strcpy(ip, "admin");
    if (get_input(ip, sizeof(ip), "[password]? ") < 0)
        strcpy(ip, "admin");

    printf("%s nvr:%s:%s@%s...\n", action == PKT_CTRL::MachineCtrl::NVR_ADD ? "Add" : "Delete", user, pass, ip);
    g_test.TestCtrlNVR(action, ip, user, pass);
    gr_sleep(1000);
    printf("Query nvr list...\n");
    g_test.TestQueryData(PKT_ReqSession::NVR_LIST);
    gr_sleep(1000);
    g_test.TestSerializeAck(ra_nvr_ack);
}

static void do_add_nvr()
{
    add_del_nvr(PKT_CTRL::MachineCtrl::NVR_ADD);
}

static void do_del_nvr()
{
    add_del_nvr(PKT_CTRL::MachineCtrl::NVR_DEL);
}

static void do_config_network()
{
    assert_login();

    char name[16];
    char ip[16];
    char mask[16];
    char hint[128];

    if (get_input(name, sizeof(ip), "[interface]? ") < 0)
        strcpy(name, "enp2s0");
    if (get_input(ip, sizeof(ip), "[ip]? ") < 0)
        strcpy(ip, "192.168.1.25");
    if (get_input(mask, sizeof(mask), "[netmask]? ") < 0)
        strcpy(ip, "255.255.255.0");

    if (strlen(ip) == 0)
        strcpy(ip, "192.168.1.73");
    if (strlen(mask) == 0)
        strcpy(mask, "255.255.255.0");

    snprintf(hint, sizeof(hint), "Config %s ip=%s netmask=%s", name, ip, mask);
    if (!confirm(hint)){
        return;
    }
    g_test.TestCtrlNetwork(name, ip, mask);
    gr_sleep(2000);
    printf("Try to ping %s to test effected\n", ip);
    exit(0);
}

static void do_config_host()
{
    assert_login();

    char name[16];
    char dns[16];

    get_input(name, sizeof(name), "[host name]? ");
    get_input(dns, sizeof(dns), "[dns]? ");

    if (strlen(name) == 0)
        strcpy(name, "Unknow name");
    if (strlen(dns) == 0)
        strcpy(dns, "202.96.128.166");

    printf("Config host=%s dns=%s\n", name, dns);
    g_test.TestConfigHost(name, dns);
    do_query_net();
}

static void do_set_fmp()
{
    assert_login();

    char ip[32];
    char user[16];
    char pass[16];

    if (get_input(ip, sizeof(ip), "[ip]? ") < 0)
        strcpy(ip, "192.168.1.25");
    if (get_input(user, sizeof(user), "[user]? ") < 0)
        strcpy(user, "admin");
    if (get_input(pass, sizeof(pass), "[password]? ") < 0)
        strcpy(pass, "admin");

    printf("set fmp:%s:%s@%s...\n", user, pass, ip);
    g_test.TestCtrlFMP(ip, user, pass);
    gr_sleep(1000);
    printf("Query fmp...\n");
    g_test.TestQueryData(PKT_ReqSession::FMP);
    gr_sleep(1000);
    g_test.TestSerializeAck(ra_fmp_ack);
}

void fcgi_interactive()
{
    printf("^_^, Press 'h' for help, 'q' for exit.\n\n");
    size_t i;
    char cmd[8];

    while (true)
    {
        fprintf(stdout, ">> ");
        fflush(stdout);
        get_input(cmd, sizeof(cmd), NULL);
        for (i = 0; i < sizeof(fn_table)/sizeof(fn_table[0]); ++i)
        {
            struct fn_object_t obj = fn_table[i];
            if (!strcmp(cmd, obj.cmd))
            {
                obj.fn();
                break;
            }
        }
        if (i == sizeof(fn_table) / sizeof(fn_table[0]))
        {
            printf("Unknow command %s. Press 'h' for help.\n", cmd);    
        }
    }
}

