#include "tvw_handler.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include "session.h"
#include "config.h"


HttpTVWHandler::HttpTVWHandler()
{
}

HttpTVWHandler::~HttpTVWHandler()
{
}


void HttpTVWHandler::Initialize()
{
}

int HttpTVWHandler::GetType()
{
    return rt_tvw;
}

Session* HttpTVWHandler::HasLogin(HttpRequest *request, http_ret_t& r)
{
    std::string sid = request->GetSid();

    if (sid.empty())
    {
        r.code = E_NOSESSION;
        r.message = "No session id in data";
        return NULL;
    }
    guid_t id;
    gr_guid_from_str(sid.c_str(), &id);
    Session *s = m_sm.LookupSession(&id);
    if (s == NULL)
    {
        r.code = E_NOSESSION;
        r.message = "Invalid session id " + sid;
        return NULL;
    }
    if (!s->IsEstablish())
    {
        m_sm.Remove(s->GetId());
        delete s;
        s = NULL;
        r.code = E_CONNDOWN;
        r.message = "Establish connection failure";
        return NULL;
    }
    return s;
}

void HttpTVWHandler::Process(HttpRequest* request, HttpResponse *response)
{
    int action = request->GetAction();

    if (action == ra_login)
    {
        DoLogin(request, response);
    }
    else if (action == ra_logout)
    {
        DoLogout(request, response);
    }
    else if (action == ra_login_ack)
    {
        QueryLoginAck(request, response);
    }
    else if (action == ra_network)
    {
        QueryNetwork(request, response, action);
    }
    else if (action == ra_query_data)
    {
        QueryData(request, response);
    }
    else if (action >= ra_basic_ack && action < ra_screen_stat)
    {
        QueryDataAck(request, response, action);
    }
    else if (action == ra_network_stat)
    {
        QueryNetworkStat(request, response, action);
    }
    else if (action >= ra_screen_stat && action < ra_ctl_screen_split)
    {
        QueryNotice(request, response, action);
    }
    else if (action >= ra_ctl_screen_split && action <= ra_ctl_master)
    {
        Control(request, response, action);
    }
}

void HttpTVWHandler::DoLogin(HttpRequest* request, HttpResponse *response)
{
    http_ret_t r;
    const backend_cfg_t *cfg = backend_get_cfg();
    char local[32] = {0};
    sprintf(local, "127.0.0.1:%d", cfg->serv_port);
    PKT_SESSION::Login login_data;
    login_data.set_version(local); //default is localhost

    r = m_parser.ParseLogin(request, login_data);
    r.action = ra_login;
    if (r.code != E_SUCCESS)
    {
        response->ResponseRet(r);
        return;
    }

    Session	*session = new Session();
    m_sm.Add(session);
    session->Connect(login_data);
    const guid_t *sid = session->GetId();
    char id[48] = {0};
    gr_guid_to_str(sid, id);

    json_value *obj = response->BeginWrite(r);
    json_value *jsid = json_string_new((const json_char*)id);
    json_object_push(obj, (const json_char*)"sid", jsid);
    response->EndWrite(obj);
    log_info("[%s:%d]Add session %s [%s:%s@%s]", __FILE__, __LINE__, id,
            login_data.username().c_str(),
            login_data.passwd().c_str(),
            login_data.version().c_str());
}

void HttpTVWHandler::DoLogout(HttpRequest* request, HttpResponse *response)
{
    http_ret_t r;

    Session *s = HasLogin(request, r);
    if (s == NULL)
    {
        response->ResponseRet(r);
        return;
    }

    const guid_t *sid = s->GetId();
    char id[48] = {0};
    gr_guid_to_str(sid, id);

    r = s->Logout();
    m_sm.Remove(s->GetId());
    delete s;
    s = NULL;
    json_value *obj = response->BeginWrite(r);
    response->EndWrite(obj);
    log_info("[%s:%d]Remove session %s", __FILE__, __LINE__, id);
}

void HttpTVWHandler::QueryLoginAck(HttpRequest* request, HttpResponse *response)
{
    http_ret_t r;

    Session *s = HasLogin(request, r);
    if (s == NULL)
    {
        response->ResponseRet(r);
        return;
    }
    r.code = 0;
    json_value *obj = response->BeginWrite(r);
    response->EndWrite(obj);
}

void HttpTVWHandler::QueryData(HttpRequest* request, HttpResponse *response)
{
    http_ret_t r;
    Session *s = HasLogin(request, r);
    if (s == NULL)
    {
        response->ResponseRet(r);
        return;
    }
    int type = PKT_ReqSession::ALL;
    r = m_parser.ParseQuery(request, &type);
    r = s->QueryData(type);
    json_value *obj = response->BeginWrite(r);
    response->EndWrite(obj);
}

void HttpTVWHandler::QueryDataAck(HttpRequest* request, HttpResponse *response, int action)
{
    http_ret_t r = {E_SUCCESS, action, ""};

    Session *s = HasLogin(request, r);
    if (s == NULL)
    {
        response->ResponseRet(r);
        return;
    }
    json_value *obj;
    r = s->QueryDataAck(action);
    obj = response->BeginWrite(r);
    s->SerializeAck(obj, action);
    response->EndWrite(obj);
}

void HttpTVWHandler::QueryNotice(HttpRequest* request, HttpResponse *response, int action)
{
    http_ret_t r = {E_SUCCESS, action, ""};
    Session *s = HasLogin(request, r);
    if (s == NULL)
    {
        response->ResponseRet(r);
        return;
    }
    json_value *obj;
    r = s->QueryNotice(action);
    obj = response->BeginWrite(r);
    s->SerializeNotice(obj, action);
    response->EndWrite(obj);
}

void HttpTVWHandler::QueryNetworkStat(HttpRequest* request, HttpResponse *response, int action)
{
    http_ret_t r = {E_SUCCESS, action, ""};
    Session *s = HasLogin(request, r);
    if (s == NULL)
    {
        response->ResponseRet(r);
        return;
    }
    json_value *obj;
    r = s->QueryNetworkStat();
    obj = response->BeginWrite(r);
    response->EndWrite(obj);
}

void HttpTVWHandler::QueryNetwork(HttpRequest* request, HttpResponse *response, int action)
{
    http_ret_t r = {E_SUCCESS, action, ""};
    Session *s = HasLogin(request, r);
    if (s == NULL)
    {
        response->ResponseRet(r);
        return;
    }
    json_value *obj;
    r = s->QueryNetwork();
    obj = response->BeginWrite(r);
    response->EndWrite(obj);
}

void HttpTVWHandler::Control(HttpRequest* request, HttpResponse *response, int action)
{
    http_ret_t r = {E_SUCCESS, action, ""};
    Session *s = HasLogin(request, r);
    if (s == NULL)
    {
        response->ResponseRet(r);
        return;
    }

    if (action == ra_ctl_screen_split)
    {
        PKT_CTRL::ScreenSplitCtrl ssc;
        r = m_parser.ParseScreenSplit(request, ssc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlScreenSplit(ssc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_view_geo)
    {
        PKT_ViewGeoCtrl vgc;
        r = m_parser.ParseViewGeometry(request, vgc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlViewGeometry(vgc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_view_video)
    {
        log_info("control view video");
        PKT_VIEW_CTRL::VideoCtrl vc;
        r = m_parser.ParseViewVideo(request, vc);
        log_debug("control view video ret:%d", r.code);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        log_debug("control view :%d", r.code);
        r = s->CtrlVideoView(vc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_view_record)
    {
        PKT_VIEW_CTRL::RecordCtrl rc;
        r = m_parser.ParseViewRecord(request, rc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlRecordView(rc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_view_flash)
    {
        PKT_VIEW_CTRL::FlashCtrl fc;
        r = m_parser.ParseViewFlash(request, fc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlFlashView(fc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_view_html)
    {
        PKT_VIEW_CTRL::HTMLCtrl hc;
        r = m_parser.ParseViewHtml(request, hc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlHTMLView(hc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_view_text)
    {
        PKT_VIEW_CTRL::TextCtrl tc;
        r = m_parser.ParseViewText(request, tc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlTextView(tc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_view_localmovie)
    {
        PKT_VIEW_CTRL::LocalMovieCtrl lmc;
        r = m_parser.ParseViewLocalMovie(request, lmc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlLocalMovie(lmc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_view_map)
    {
        PKT_VIEW_CTRL::MapCtrl mc;
        r = m_parser.ParseViewMap(request, mc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlMap(mc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_camera)
    {
        PKT_CTRL::CameraCtrl cc;
        r = m_parser.ParseCameraCtrl(request, cc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlCamera(cc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_hotpoint)
    {
    }
    else if (action == ra_ctl_layout)
    {
        PKT_CTRL::LayoutCtrl lc;
        r = m_parser.ParseLayoutCtrl(request, lc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlLayout(lc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_layout_loop)
    {
        PKT_CTRL::LayoutLoopCtrl llc;
        r = m_parser.ParseLayoutLoopCtrl(request, llc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlLayoutLoop(llc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_machine)
    {
        PKT_CTRL::MachineCtrl mc;
        r = m_parser.ParseMachineCtrl(request, mc);

        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
        r = s->CtrlMachine(mc);
        if (r.code != E_SUCCESS)
        {
            response->ResponseRet(r);
            return;
        }
    }
    else if (action == ra_ctl_master)
    {
    }
    json_value *obj;
    obj = response->BeginWrite(r);
    response->EndWrite(obj);
}
