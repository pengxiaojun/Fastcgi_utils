#include "session.h"
#include "logger.h"
#include <fcgi_stdio.h>


static int GRCALL coder(const void *ud, const void *buf, int len)
{
    char* data = (char*)buf;
    int dlen = *(int*)data;
    if(dlen + (int)sizeof(int) > len)
    {
        return 0;
    }
    PKT pkt;
    if(!pkt.ParseFromArray(data + sizeof(int), dlen))
    {
        log_error("[%s:%d]Unknow error when encode", __FILE__, __LINE__);
        return len;
    }
    else
    {
        return dlen + sizeof(int);
    }
}

Session::Session():
    m_logined(false),
    m_link(NULL),
    m_data(this)
{
    m_sid = gr_guid_new();
    m_conn = new UniConnector(coder, coder);
    m_conn->OnConnected = UniDelegate<HandlerLinkData_t>(this, (HandlerLinkData_t)&Session::OnConnected);
    m_conn->OnConnectFail = UniDelegate<HandlerDataAddr_t>(this, (HandlerDataAddr_t)&Session::OnConnectFail);
}

Session::~Session()
{
    delete m_conn;
    m_conn = NULL;
}

const guid_t* Session::GetId()
{
    return &m_sid;
}

int Session::Send(PKT& pkg)
{
    if(m_link == NULL)
        return E_LINKDOWN;

    int pkg_size;
    char *buf;


    pkg_size = pkg.ByteSize();
    std::vector<char> v(pkg_size + sizeof(int));
    buf = &v[0];
    pkg.SerializeToArray(buf + sizeof(int), pkg_size);
    *(int*)buf = pkg_size;
    return m_link->Send(buf, pkg_size + sizeof(int));
}

void Session::Connect(const PKT_SESSION::Login& login)
{
    m_login = login;
    m_conn->Start();
    m_conn->Connect(m_login.version().c_str()); //TODO: use 127.0.0.1 instead
}

void Session::Disconnect()
{
    m_conn->Stop();
}

void Session::OnConnected(IUniLink *lnk, const void *ud)
{
    m_link = lnk;
    lnk->SetState(uns_Connected);
    m_link->OnPDU = UniDelegate<HandlerBlock_t>(this, (HandlerBlock_t)&Session::OnPDU);
    m_link->OnDisconnected = UniDelegate<HandlerData_t>(this, (HandlerData_t)&Session::OnDisconnected);
    //do login
    m_login.set_version("v1.0.0");
    m_login.set_encryption("md5");
    m_login.set_ismaster(false);
    Login(m_login);
    log_info("[%s:%d]Link %p connected", __FILE__, __LINE__,  m_link);
}

void Session::OnConnectFail(void *ud, const sockaddr_inet &peer)
{
    log_info("[%s:%d]Connect %s failure ", __FILE__, __LINE__,  SockAddrInet(peer).ToString().c_str());
    m_link = NULL;
}

void Session::Login(PKT_SESSION::Login& login)
{
    PKT pkt;
    PKT_SESSION* session = pkt.mutable_session();
    PKT_SESSION::Login* mlogin = session->mutable_login();
    *mlogin = login;
    Send(pkt);
}

void Session::OnPDU(const void *ud, const void *buf, int len)
{
    PKT pkt;
    pkt.ParseFromArray((char*)buf + sizeof(int), len - sizeof(int));
    int r;

    if(pkt.has_session())
    {
        PKT_SESSION session = pkt.session();
        if(session.has_loginack())
        {
            r = session.loginack().result();
            if (r < 0)
            {
                Disconnect();
                //m_link = NULL;
                log_info("[%s:%d]Login failure on ack", __FILE__, __LINE__, r);
                return;
            }
            m_logined = true;
            m_data.Initialize();
        }
        else if(session.has_logoutack())
        {
            m_logined = false;
            //this code will not be executed. For simpley, because I disocnnect network when user logout
            Disconnect();
            log_info("[%s:%d]Session logout ack", __FILE__, __LINE__);
        }
    }
    if(pkt.has_requiresession())
    {
        PKT::RequireSession requireSession = pkt.requiresession();
        if(requireSession.has_clientrequireack())
        {
            m_data.SetData(requireSession.clientrequireack());
        }
        if (requireSession.has_netrequireack())
        {
            m_data.SetNetworkData(requireSession.netrequireack());
        }
        if (requireSession.has_recordrequireack())
        {
            m_data.SetRecordAck(requireSession.recordrequireack());
        }
    }
    if(pkt.has_notice())
    {
        m_data.SetNotice(pkt.notice());
    }
}

void Session::OnDisconnected(void *Adata)
{
    log_info("[%s:%d]Link %p disconnect", __FILE__, __LINE__, m_link);
    m_link = NULL;
}

bool Session::IsEstablish()
{
    return (m_link != NULL && m_logined);
}

http_ret_t Session::Logout()
{
    http_ret_t r = {E_SUCCESS, ra_logout, ""};
    PKT pkt;
    PKT_SESSION *session = pkt.mutable_session();
    PKT_SESSION::Logout *logout = session->mutable_logout();
    logout->set_reason(PKT_SESSION::USER_LOGOUT);
    r.code = Send(pkt);
    Disconnect();
    return r;
}

http_ret_t Session::QueryData(int type)
{
    http_ret_t r = {E_SUCCESS, ra_query_data, ""};
    PKT_ReqSession query;
    PKT_ReqSession::ClientRequire* client_req = query.mutable_clientrequire();
    client_req->set_type((PKT_ReqSession::ClientRequireType)type);
    PKT pkt;
    PKT_ReqSession *req = pkt.mutable_requiresession();
    *req = query;
    Send(pkt);
    return r;
}

http_ret_t Session::QueryDataAck(int action)
{
    http_ret_t r = {E_SUCCESS, action, ""};
    return r;
}

http_ret_t Session::QueryNotice(int action)
{
    http_ret_t r = {E_SUCCESS, action, ""};
    return r;
}

http_ret_t Session::QueryNetworkStat()
{
    http_ret_t r = {E_SUCCESS, ra_network_stat, ""};

    if (m_link == NULL)
    {
        r.code = E_LINKDOWN;
        r.message = "Network disconnected";
    }
    return r;
}

http_ret_t Session::QueryNetwork()
{
    http_ret_t r = {E_SUCCESS, ra_network, ""};
    PKT_ReqSession query;
    PKT_ReqSession::NetRequire *net_req = query.mutable_netrequire();
    PKT pkt;
    PKT_ReqSession *req = pkt.mutable_requiresession();
    *req = query;
    r.code = Send(pkt);
    net_req->Clear(); //dispear warning
    return r;
}

http_ret_t Session::QueryNetworkAck()
{
    http_ret_t r = {E_SUCCESS, ra_network_ack, ""};
    return r;
}

void Session::SerializeData(json_value *obj)
{
    m_data.SerializeData(obj);
}

void Session::SerializeAck(json_value *obj, int action)
{
    if (action == ra_basic_ack)
        m_data.SerializeBasicInfo(obj);
    else if (action == ra_camera_ack)
        m_data.SerializeCameraGroup(obj);
    else if (action == ra_localmovie_ack)
        m_data.SerializeLocalMovie(obj);
    else if (action == ra_flash_ack)
        m_data.SerializeFlash(obj);
    else if (action == ra_layout_ack)
        m_data.SerializeLayout(obj);
    else if (action == ra_layout_loop_ack)
        m_data.SerializeLayoutLoop(obj);
    else if (action == ra_hotpoint_ack)
        m_data.SerializeHotPoint(obj);
    else if (action == ra_map_ack)
        m_data.SerializeMap(obj);
    else if (action == ra_view_ack)
        m_data.SerializeView(obj);
    else if (action == ra_screen_ack)
        m_data.SerializeScreen(obj);
    else if (action == ra_nvr_ack)
        m_data.SerializeNvr(obj);
    else if(action == ra_network_ack)
        m_data.SerializeNetwork(obj);
    else if (action == ra_fmp_ack)
        m_data.SerializeFmp(obj);
}

void Session::SerializeNotice(json_value *obj, int action)
{
    if (action == ra_screen_stat)
        m_data.SerializeScreenNotice(obj);
    else if (action == ra_view_geo_stat)
        m_data.SerializeViewGeoNotice(obj);
    else if (action == ra_view_video_stat)
        m_data.SerializeViewVideoNotice(obj);
    else if (action == ra_view_fhlm_stat)
        m_data.SerializeViewFHLMNotice(obj);
    else if (action == ra_layout_stat)
        m_data.SerializeLayoutNotice(obj);
    else if (action == ra_layout_loop_stat)
        m_data.SerializeLayoutLoopNotice(obj);
    else if (action == ra_camera_stat)
        m_data.SerializeCameraNotice(obj);
    else if (action == ra_hotpoint_stat)
        m_data.SerializeHotPointNotice(obj);
    else if (action == ra_record_stat)
        m_data.SerializeRecordNotice(obj);
}

http_ret_t Session::CtrlScreenSplit(PKT_CTRL::ScreenSplitCtrl& ssc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_screen_split, ""};
    PKT pkt;
    PKT_CTRL *pc = pkt.mutable_ctrl();
    PKT_CTRL::ScreenSplitCtrl *pcs = pc->mutable_screenctrl();
    *pcs = ssc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlViewGeometry(PKT_ViewGeoCtrl& vgc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_view_geo, ""};

    if (vgc.type() == PKT_ViewGeoCtrl::CREATE)
    {
        PKT::ElementDat::View *v = vgc.mutable_view();
        v->set_winid(-1);
    }
    else{
        PKT::ElementDat::View v;

        if (!LookupView(vgc.view().winid(), v))
        {
            r.code = E_NOVIEW;
            return r;
        }
        /*TvWall::Rect *rect = v.mutable_rect();
         *(rect) = vgc.view().rect();
         *(v.mutable_cameraid()) = vgc.view().cameraid();*/

        if (vgc.type() == PKT_ViewGeoCtrl::SET_BORDER_WIDTH)
        {
            v.set_borderwidth(vgc.view().borderwidth());
        }
        else if (vgc.type() == PKT_ViewGeoCtrl::SHOW_TITLE)
        {
            v.set_showtitle(vgc.view().showtitle());
        }
        //*(vgc.mutable_view()) = v;
    }

    log_info("[%s:%d]Control view type %u", __FILE__, __LINE__,  vgc.type());
    PKT pkt;
    PKT_CTRL * ctrl = pkt.mutable_ctrl();
    PKT_VIEW_CTRL* viewCtrl = ctrl->mutable_viewctrl();
    PKT_VIEW_CTRL::ViewGeomotryCtrl *viewGeomotryCtrl = viewCtrl->mutable_viewgeomotryctrl();
    *viewGeomotryCtrl = vgc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlVideoView(PKT_VIEW_CTRL::VideoCtrl& vc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_view_video, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_VIEW_CTRL* viewCtrl = ctrl->mutable_viewctrl();
    PKT_VIEW_CTRL::VideoCtrl *videoCtrl = viewCtrl->mutable_videoctrl();
    *videoCtrl = vc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlRecordView(TvWall::Packet_Ctrl_ViewCtrl::RecordCtrl& rc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_view_record, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_VIEW_CTRL* viewCtrl = ctrl->mutable_viewctrl();
    PKT_VIEW_CTRL::RecordCtrl *recordCtrl = viewCtrl->mutable_recordctrl();
    *recordCtrl = rc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlFlashView(PKT_VIEW_CTRL::FlashCtrl& fc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_view_flash, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_VIEW_CTRL* viewCtrl = ctrl->mutable_viewctrl();
    PKT_VIEW_CTRL::FlashCtrl* flashCtrl = viewCtrl->mutable_flashctrl();
    *flashCtrl = fc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlHTMLView(PKT_VIEW_CTRL::HTMLCtrl& hc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_view_html, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_VIEW_CTRL* viewCtrl = ctrl->mutable_viewctrl();
    PKT_VIEW_CTRL::HTMLCtrl *htmlCtrl = viewCtrl->mutable_htmlctrl();
    *htmlCtrl = hc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlTextView(PKT_VIEW_CTRL::TextCtrl& tc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_view_text, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_VIEW_CTRL* viewCtrl = ctrl->mutable_viewctrl();
    PKT_VIEW_CTRL::TextCtrl * textCtrl = viewCtrl->mutable_textctrl();
    *textCtrl = tc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlLocalMovie(PKT_VIEW_CTRL::LocalMovieCtrl& clm)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_view_localmovie, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_VIEW_CTRL* viewCtrl = ctrl->mutable_viewctrl();
    PKT_VIEW_CTRL::LocalMovieCtrl *localMovie = viewCtrl->mutable_localmovie();
    *localMovie = clm;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlMap(PKT_VIEW_CTRL::MapCtrl& mc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_view_map, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_VIEW_CTRL* viewCtrl = ctrl->mutable_viewctrl();
    PKT_VIEW_CTRL::MapCtrl *mapCtrl = viewCtrl->mutable_mapctrl();
    *mapCtrl = mc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlCamera(PKT_CTRL::CameraCtrl& cc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_camera, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_CTRL::CameraCtrl * cameraCtrl = ctrl->mutable_cameractrl();
    *cameraCtrl = cc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlHotPoint(PKT_CTRL::HotPointCtrl& hpc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_hotpoint, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_CTRL::HotPointCtrl * hotPointCtrl = ctrl->mutable_hotpointctrl();
    *hotPointCtrl = hpc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlLayout(PKT_CTRL::LayoutCtrl& lc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_layout, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT_CTRL::LayoutCtrl *lyoutCtrl = ctrl->mutable_lyoutctrl();
    *lyoutCtrl = lc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlLayoutLoop(PKT::Ctrl::LayoutLoopCtrl& llc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_layout_loop, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    PKT::Ctrl::LayoutLoopCtrl *lyoutCtrl = ctrl->mutable_lyoutloopctrl();
    *lyoutCtrl = llc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlMachine(PKT_CTRL::MachineCtrl& mc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_machine, ""};
    PKT pkt;
    PKT_CTRL* ctrl = pkt.mutable_ctrl();
    PKT_CTRL::MachineCtrl* cm = ctrl->mutable_machinectrl();
    *cm = mc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlKey(PKT_CTRL::KeyCtrl& kc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_key, ""};
    PKT pkt;
    PKT_CTRL* ctrl = pkt.mutable_ctrl();
    PKT_CTRL::KeyCtrl * key_ctrl = ctrl->mutable_keyctrl();
    *key_ctrl = kc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlKeyCmd(PKT_CTRL::KeyCmdCtrl& kcc)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_keycmd, ""};
    PKT pkt;
    PKT_CTRL* ctrl = pkt.mutable_ctrl();
    PKT_CTRL::KeyCmdCtrl * key_cmd_ctrl = ctrl->mutable_keycmdctrl();
    *key_cmd_ctrl = kcc;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlAlarm(PKT_CTRL::AlarmCtrl& ac)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_alarm, ""};
    PKT pkt;
    PKT_CTRL* ctrl = pkt.mutable_ctrl();
    PKT_CTRL::AlarmCtrl *alarm_ctrl = ctrl->mutable_alarmctrl();
    *alarm_ctrl = ac;
    r.code = Send(pkt);
    return r;
}

http_ret_t Session::CtrlMLY(PKT_CtrlMasterLyout& cml)
{
    http_ret_t r = {E_SUCCESS, ra_ctl_master, ""};
    PKT pkt;
    PKT_CTRL *ctrl = pkt.mutable_ctrl();
    ::TvWall::Packet_Ctrl_CtrlMaster* cMaster = ctrl->mutable_masterctrl();
    PKT_CtrlMasterLyout* ml_ctrl = cMaster->mutable_ctrllyout();
    *ml_ctrl = cml;
    r.code = Send(pkt);
    return r;
}

bool Session::LookupView(int winid, PKT::ElementDat::View& view)
{
    PKT::ElementDat::View *v = m_data.LookupView(winid);
    if (v == NULL)
    {
        return false;
    }
    view = *v;
    return true;
}
