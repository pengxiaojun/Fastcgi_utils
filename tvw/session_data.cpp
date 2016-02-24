#include "session_data.h"
#include "session.h"
#include "logger.h"


SessionData::SessionData(Session *session):
    m_session(session),
    m_element(NULL)
{
}

SessionData::~SessionData()
{
    if (m_element)
    {
        delete m_element;
        m_element = NULL;
    }
}

void SessionData::Initialize()
{
    m_element = new PKT::ElementDat;
}

void SessionData::SetData(const PKT_ReqSession::ClientRequireAck& ack)
{
    PKT::ElementDat element = ack.elementdata();
    m_element->set_servername(element.servername());
    m_element->set_tvwallindex(element.tvwallindex());
    m_element->set_maxplaybackrate(element.maxplaybackrate());
    m_element->set_anoleversion(element.anoleversion());
    m_element->set_sn(element.sn());
    *(m_element->mutable_guid()) = element.guid();

    if (ack.type() & PKT_ReqSession::CAMERA)
    {
        *(m_element->mutable_camgrp()) = element.camgrp();
    }
    if (ack.type() & PKT_ReqSession::LOCAL_MOVIE)
    {
        *(m_element->mutable_localmovie()) = element.localmovie();
    }

    if (ack.type() & PKT_ReqSession::FLASH)
    {
        *(m_element->mutable_flash()) = element.flash();
    }
    if (ack.type() & PKT_ReqSession::LAYOUT)
    {
        *(m_element->mutable_servlayout()) = element.servlayout();
    }
    if (ack.type() & PKT_ReqSession::HOT_POINT)
    {
        *(m_element->mutable_hotpoint()) = element.hotpoint();
    }
    if (ack.type() & PKT_ReqSession::RECORDINFO)
    {
    }
    if (ack.type() & PKT_ReqSession::MAPINFO)
    {
        *(m_element->mutable_map()) = element.map();
    }
    if (ack.type() & PKT_ReqSession::LAYOUTLOOP)
    {
        *(m_element->mutable_currentlayoutloop()) = element.currentlayoutloop();
        *(m_element->mutable_currentlayout()) = element.currentlayout();
        *(m_element->mutable_servlayoutloop()) = element.servlayoutloop();
    }
    if (ack.type() & PKT_ReqSession::SCREEN)
    {
        *(m_element->mutable_screen()) = element.screen();
    }
    if(ack.type() & PKT_ReqSession::VIEW)
    {
        *(m_element->mutable_view()) = element.view();
    }
    if (ack.type() & PKT_ReqSession::NVR_LIST)
    {
        *(m_element->mutable_nvrlst()) = element.nvrlst();
    }
    if (ack.type() & PKT_ReqSession::FMP)
    {
        *(m_element->mutable_fmp()) = element.fmp();
    }
}

void SessionData::SetNetworkData(const PKT_ReqSession::NetRequireAck& ack)
{
    if (!ack.has_net())
    {
        return;
    }
    m_network = ack.net();
}

void SessionData::SetRecordAck(const PKT_ReqSession::RecordRequireAck& ack)
{
    m_recordack = ack;
}

void SessionData::SetNotice(const PKT::Notice& notice)
{
    if (notice.has_statinfo())
    {
        const PKT_STAT& statinfo = notice.statinfo();
        if (statinfo.has_screenstat())
        {
            const PKT_STAT::ScreenSpliStat& splitstat = statinfo.screenstat();
            SetScreenSplitStat(splitstat);
        }
        if (statinfo.has_viewstat())
        {
            const PKT_VIEW_STAT& viewstat = statinfo.viewstat();
            SetViewStat(viewstat);
        }
        if (statinfo.has_camerastat())
        {
            const PKT_STAT::CameraStat& s = statinfo.camerastat();
            SetCameraStat(s);
        }
        if (statinfo.has_hotpointstat())
        {
            const PKT_STAT::HotPointStat& s = statinfo.hotpointstat();
            SetHotPointStat(s);
        }
        if (statinfo.has_lyoutstat())
        {
            const PKT_STAT::LayoutStat& s = statinfo.lyoutstat();
            SetLayoutStat(s);
        }
        if (statinfo.has_lyoutloopstat())
        {
            const PKT_STAT::LayoutLoopStat& s = statinfo.lyoutloopstat();
            SetLayoutLoopStat(s);
        }
        if (statinfo.has_mapgrpstat())
        {
        }
        if (statinfo.has_errstat())
        {
        }
        if (statinfo.has_alarmstat())
        {
        }
    }
    if (notice.has_masterstat())
    {
    }
}

void SessionData::SetScreenSplitStat(const PKT_STAT::ScreenSpliStat& split)
{
    int i, size;
    size = m_element->screen_size();
    for (i = 0; i<size; ++i)
    {
        PKT::ElementDat::Screen *screen = m_element->mutable_screen(i);
        if (screen->screenindex() == split.screenindex())
        {
            screen->set_row(split.row());
            screen->set_col(split.col());

            //add to screen notice
            m_scrn_notice.insert(screen->screenindex());
            break;
        }
    }
}

void SessionData::SetViewStat(const PKT_VIEW_STAT& viewstat)
{
    if (viewstat.has_viewgeomotrystat())
    {
        const PKT_VIEW_STAT::ViewGeomotryStat& geostat = viewstat.viewgeomotrystat();
        const PKT::ElementDat::View& view = geostat.view();

        //add view notice
        PKT_VIEW_STAT::ViewGeomotryStat *vg = new PKT_VIEW_STAT::ViewGeomotryStat(geostat);
        m_viewgeo_notice.push_back(vg);

        if (geostat.type() == PKT_VIEW_STAT::ViewGeomotryStat::CREATE)
        {
            *(m_element->add_view()) = view;
        }
        else
        {
            int i;
            PKT::ElementDat::View *v = LookupView(view.winid(), &i);
            if (v == NULL)
                return;

            const ::TvWall::Rect& r = view.rect();

            if (geostat.type() == PKT_VIEW_STAT::ViewGeomotryStat::WINDOW_CTRL)
            {
                *v->mutable_rect() = r;
                if (view.has_windowstack())
                    v->set_windowstack(view.windowstack());
                if (view.has_showmaxfullscreen())
                    v->set_showmaxfullscreen(view.showmaxfullscreen());
                if (view.has_showfullscreen())
                    v->set_showfullscreen(view.showfullscreen());
                if (view.has_url())
                    v->set_url(view.url());
                if (view.has_gpuid())
                    v->set_gpuid(view.gpuid());
            }
            else if (geostat.type() == PKT_VIEW_STAT::ViewGeomotryStat::REMOVE)
            {
                m_element->mutable_view()->DeleteSubrange(i, 1);
            }
        }
    }
    if (viewstat.has_flashstat() ||
            viewstat.has_htmlstat() ||
            viewstat.has_localmoviestat() ||
            viewstat.has_mapstat())
    {
        int winid = 0;
        std::string url;
        PKT::ElementDat::View *v = NULL;
        if (viewstat.has_flashstat())
        {
            winid = viewstat.flashstat().winid();
            url = viewstat.flashstat().url();
            v = LookupView(winid);
            if (v) v->set_url(url);
            m_fhlm_notice.insert(winid);
        }
        if (viewstat.has_htmlstat())
        {
            winid = viewstat.htmlstat().winid();
            url = viewstat.htmlstat().url();
            v = LookupView(winid);
            if (v) v->set_url(url);
            m_fhlm_notice.insert(winid);
        }
        if (viewstat.has_localmoviestat())
        {
            winid = viewstat.localmoviestat().winid();
            url = viewstat.localmoviestat().url();
            v = LookupView(winid);
            if (v) v->set_url(url);
            m_fhlm_notice.insert(winid);
        }
        if (viewstat.has_mapstat())
        {
            winid = viewstat.mapstat().winid();
            url = viewstat.mapstat().url();
            v = LookupView(winid);
            if (v) v->set_url(url);
            m_fhlm_notice.insert(winid);
        }
    }
    if (viewstat.has_videostat())
    {
        const PKT_VIEW_STAT::VideoStat& s = viewstat.videostat();
        PKT::ElementDat::View *v = LookupView(s.winid());
        if (v == NULL)
            return;

        if (s.type() == PKT_VIEW_STAT::VideoStat::CAMERA_ADD ||
                s.type() == PKT_VIEW_STAT::VideoStat::CAMERA_REMOVE)
        {
            v->clear_cameraid();
            *(v->mutable_cameraid()) = s.cameraid();
        }
        else if (s.type() == PKT_VIEW_STAT::VideoStat::KEEP_STRETCH)
        {
            v->set_keepstretch(s.keepstretch());
        }
        else if (s.type() == PKT_VIEW_STAT::VideoStat::POLL_INTERVAL)
        {
            v->set_pollinterval(s.pollinterval());
        }
        //PKT_VIEW_STAT::VideoStat *n = new PKT_VIEW_STAT::VideoStat(s);
        //m_video_notice.push_back(n);
        m_video_notice.insert(s.winid());
    }
    if (viewstat.has_recordstat())
    {
    }
}

void SessionData::SetCameraStat(const PKT_STAT::CameraStat& s)
{
    bool save_stat = false;

    if (s.type() == PKT_STAT::CameraStat::NETSTAT ||
        s.type() == PKT_STAT::CameraStat::PLAYING_STAT ||
        s.type() == PKT_STAT::CameraStat::PLAYBACK_STAT ||
        s.type() == PKT_STAT::CameraStat::UPDATE ||
        s.type() == PKT_STAT::CameraStat::REMOVE)
    {
        bool remove = s.type() == PKT_STAT::CameraStat::REMOVE;
        PKT::ElementDat::Camera* cam = LookupCamera(s.cam().cameraid(), remove);
        if (cam){
            if (cam->parentid().data().data() != s.cam().parentid().data().data())
            {
                LookupCamera(s.cam().cameraid(), true);
                LookupAddCamera(s.cam()); 
            }
            else{
                *cam = s.cam(); //update camera
            }
        }
        if (s.type() == PKT_STAT::CameraStat::NETSTAT)
        {
            RemoveCamDupNetStat(s.cam().cameraid());
        }
        save_stat = true;
    }
    else if (s.type() == PKT_STAT::CameraStat::ADD)
    {
        LookupAddCamera(s.cam()); 
        save_stat = true;
    }
    else if (s.type() == PKT_STAT::CameraStat::ADD_GRP)
    {
        LookupAddCameraGroup(s.grp());
        save_stat = true;
    }
    else if (s.type() == PKT_STAT::CameraStat::REMOVE_GRP)
    {
        LookupCameraGroup(s.grp().grpid(), true);
        save_stat = true;
    }
    else if (s.type() == PKT_STAT::CameraStat::UPDATE_GRP)
    {
        PKT::ElementDat::CameraGrp *grp = LookupCameraGroup(s.grp().grpid());
        if (grp){
            if (grp->parentid().data().data() != s.grp().parentid().data().data())
            {
                LookupCameraGroup(s.grp().grpid(), true);
                LookupAddCameraGroup(s.grp());
            }
        }
        save_stat = true;
    }

    //m_element->clear_camgrp();
    //*(m_element->mutable_camgrp()) = s.camgrp();
    //m_camera_changed = true;
    
    //save camera status
    if (save_stat)
    {
        PKT_STAT::CameraStat *stat = new PKT_STAT::CameraStat(s);
        m_camera_notice.push_back(stat);
    }
}

void SessionData::SetHotPointStat(const PKT_STAT::HotPointStat& s)
{
    m_element->clear_hotpoint();
    *(m_element->mutable_hotpoint()) = s.hotpoint();
    m_hotpoint_changed = true;
}

void SessionData::SetLayoutStat(const PKT_STAT::LayoutStat& s)
{
    if (s.type() == PKT_STAT::LayoutStat::USE)
    {
        m_element->clear_currentlayout();
        m_element->set_currentlayout(s.currentlayout());
    }
    else if (s.type() == PKT_STAT::LayoutStat::ADD ||
            s.type() == PKT_STAT::LayoutStat::REMOVE)
    {
        m_element->clear_servlayout();
        *(m_element->mutable_servlayout()) = s.servlayout();
    }
    m_layout_changed = true;
}

void SessionData::SetLayoutLoopStat(const PKT_STAT::LayoutLoopStat& s)
{
    if (s.type() == PKT_STAT::LayoutLoopStat::ADD ||
            s.type() == PKT_STAT::LayoutLoopStat::REMOVE ||
            s.type() == PKT_STAT::LayoutLoopStat::ADD_LAYOUT ||
            s.type() == PKT_STAT::LayoutLoopStat::SET_LOOP)
    {
        m_element->clear_servlayoutloop();
        *(m_element->mutable_servlayoutloop()) = s.servlayoutloop();
    }
    else if (s.type() == PKT_STAT::LayoutLoopStat::USE ||
            s.type() == PKT_STAT::LayoutLoopStat::STOP)
    {
        m_element->clear_currentlayoutloop();
        *(m_element->mutable_currentlayoutloop()) = s.currentlayoutloop();
    }
    m_layoutloop_changed = true;
}

void SessionData::SerializeData(json_value *obj)
{
    //basic info
    SerializeBasicInfo(obj);
    //group
    SerializeCameraGroup(obj);
    //localmovie
    SerializeLocalMovie(obj);
    //Flash
    SerializeFlash(obj);
    //Layout
    SerializeLayout(obj);
    //Hotpoint
    SerializeHotPoint(obj);
    //Map
    SerializeMap(obj);
    //Layout loop
    SerializeLayoutLoop(obj);
    //Screen
    SerializeScreen(obj);
    //View
    SerializeView(obj);
}

void SessionData::SerializeNotice(json_value *obj)
{
    //serialize all notice. not used now
}

void SessionData::SerializeBasicInfo(json_value *obj)
{
    json_value *basic_obj = json_object_new(0);

    json_value *name = json_string_new(m_element->servername().data());
    json_object_push(basic_obj, "Name", name);

    if (m_element->has_currentlayout())
    {
        json_value *current_layout = json_string_new(m_element->currentlayout().data());
        json_object_push(basic_obj, "CurrentLayout", current_layout);
    }

    if (m_element->has_currentlayoutloop())
    {
        json_value *current_layout_loop = json_string_new(m_element->currentlayoutloop().data());
        json_object_push(basic_obj, "CurrentLayoutLoop", current_layout_loop);
    }
    if (m_element->has_guid())
    {
        json_value *id = json_string_new(m_element->guid().data().data());
        json_object_push(basic_obj, "Id", id);
    }
    if (m_element->has_tvwallindex())
    {
        json_value *index = json_integer_new(m_element->tvwallindex());
        json_object_push(basic_obj, "Index", index);
    }
    if (m_element->has_maxplaybackrate())
    {
        json_value *rate = json_integer_new(m_element->maxplaybackrate());
        json_object_push(basic_obj, "MaxPlaybackRate", rate);
    }
    if (m_element->has_anoleversion())
    {
        json_value *version = json_string_new(m_element->anoleversion().data());
        json_object_push(basic_obj, "Version", version);
    }
    if (m_element->has_sn())
    {
        json_value *sn = json_string_new(m_element->sn().data());
        json_object_push(basic_obj, "SN", sn);
    }
    json_object_push(obj, "Basic", basic_obj);
}

void SessionData::SerializeCameraGroup(json_value* obj)
{
    int i, size;
    size = m_element->camgrp_size();
    json_value *groups = json_array_new(0);


    for (i = 0; i<size; i++)
    {
        json_value* group_obj = _SerializeCameraGroup(m_element->camgrp(i));
        json_array_push(groups, group_obj);
    }
    json_object_push(obj, "Groups", groups);
}

void SessionData::SerializeLocalMovie(json_value *obj)
{
    int i, size;
    size = m_element->localmovie_size();

    json_value *array = json_array_new(0);

    for (i = 0; i<size; ++i)
    {
        json_value *val = json_string_new(m_element->localmovie(i).filename().data());
        json_array_push(array, val);
    }
    json_object_push(obj, "LocalMovies", array);
}

void SessionData::SerializeFlash(json_value *obj)
{
    int i, size;
    size = m_element->flash_size();

    json_value *array = json_array_new(0);

    for (i = 0; i<size; ++i)
    {
        json_value *val = json_string_new(m_element->flash(i).url().data());
        json_array_push(array, val);
    }
    json_object_push(obj, "Flashs", array);
}

void SessionData::SerializeLayout(json_value *obj)
{
    int i, j, k, size, screen_size, view_size;

    size = m_element->servlayout_size();

    json_value *cur_layout = json_string_new(m_element->currentlayout().data());
    json_object_push(obj, "CurrentLayout", cur_layout);

    json_value *cur_layout_loop = json_string_new(m_element->currentlayoutloop().data());
    json_object_push(obj, "CurrentLayoutLoop", cur_layout_loop);

    json_value *layout_array = json_array_new(0);
    for (i = 0; i<size; ++i)
    {
        const PKT::ElementDat::ServerLayout& servlayout = m_element->servlayout(i);

        json_value *layout_obj = json_object_new(0);
        json_value *name = json_string_new(servlayout.name().data());
        json_object_push(layout_obj, "Name", name);

        const PKT::ElementDat& ed = servlayout.layoutdat();
        screen_size = ed.screen_size();

        json_value *screen_array = json_array_new(0);

        for (j = 0; j<screen_size; ++j)
        {
            json_value *screen_obj = _SerializeScreen(ed.screen(j));
            json_array_push(screen_array, screen_obj);
        }
        json_object_push(layout_obj, "Screens", screen_array);

        view_size = ed.view_size();
        json_value *view_array = json_array_new(0);
        for (k = 0; k<view_size; ++k)
        {
            json_value *view_obj = _SerializeView(ed.view(k));
            json_array_push(view_array, view_obj);
        }
        json_object_push(layout_obj, "Views", view_array);

        json_array_push(layout_array, layout_obj);
    }
    json_object_push(obj, "ServerLayouts", layout_array);
}

void SessionData::SerializeLayoutLoop(json_value *obj)
{
    int i, j, size;
    size = m_element->servlayoutloop_size();

    json_value *array = json_array_new(0);

    for (i = 0; i<size; ++i)
    {
        const PKT::ElementDat::ServerLayoutLoop loop = m_element->servlayoutloop(i);

        json_value *loop_obj = json_object_new(0);
        json_value *name = json_string_new(loop.name().data());
        json_value *layout_array = json_array_new(0);

        for (j = 0; j<loop.layoutname_size(); ++j)
        {
            json_value *layout_name = json_string_new(loop.layoutname(j).data());
            json_array_push(layout_array, layout_name);
        }
        json_value *poll_interval = json_integer_new(loop.pollinterval());
        json_object_push(loop_obj, "Name", name);
        json_object_push(loop_obj, "PollInterval", poll_interval);
        json_object_push(loop_obj, "Layouts", layout_array);

        json_array_push(array, loop_obj);
    }
    json_object_push(obj, "LayoutLoops", array);
}

void SessionData::SerializeHotPoint(json_value *obj)
{
    if (!m_element->has_hotpoint())
    {
        return;
    }

    json_value *hot_obj = json_object_new(0);
    SerializeRect(hot_obj, m_element->hotpoint().geometry());
    json_value *screen_index = json_integer_new(m_element->hotpoint().screenindex());
    json_value *limit_map = json_boolean_new(m_element->hotpoint().limitedmap());
    json_value *limit_record = json_boolean_new(m_element->hotpoint().limitedrecord());
    json_object_push(hot_obj, "ScreenIndex", screen_index);
    json_object_push(hot_obj, "LimitMap", limit_map);
    json_object_push(hot_obj, "LimitRecord", limit_record);

    json_object_push(obj, "HotPoint", hot_obj);
}

void SessionData::SerializeView(json_value *obj)
{
    int i, size;
    size = m_element->view_size();

    json_value *views = json_array_new(0);
    for (i = 0; i<size; i++)
    {
        json_value *view_obj = _SerializeView(m_element->view(i));
        json_array_push(views, view_obj);
    }

    json_object_push(obj, "Views", views);
}

void SessionData::SerializeMap(json_value *obj)
{
    int i, size;

    size = m_element->map_size();

    json_value *maps = json_array_new(0);

    for (i = 0; i<size; ++i)
    {
        json_value *map_obj = _SerializeMap(m_element->map(i));
        json_array_push(maps, map_obj);
    }

    json_object_push(obj, "Maps", maps);
}



void SessionData::SerializeScreen(json_value *obj)
{
    int i, size;

    size = m_element->screen_size();

    json_value *array = json_array_new(0);

    for (i = 0; i<size; ++i)
    {
        json_value *screen_obj = _SerializeScreen(m_element->screen(i));
        json_array_push(array, screen_obj);
    }
    json_object_push(obj, "Screens", array);
}

void SessionData::SerializeNvr(json_value *obj)
{
    int i, size;

    size = m_element->nvrlst_size();

    json_value *array = json_array_new(0);

    for (i = 0; i<size; ++i)
    {
        json_value *nvr_obj = _SerializeNvr(m_element->nvrlst(i));
        json_array_push(array, nvr_obj);
    }
    json_object_push(obj, "Nvrs", array);
}

void SessionData::SerializeFmp(json_value *obj)
{
    json_value *fmp = _SerializeFmp(m_element->fmp());
    json_object_push(obj, "Fmp", fmp);
}

void SessionData::SerializeNetwork(json_value *obj)
{
    int i, size;

    json_value *network_obj = json_object_new(0);

    json_value *host_name = json_string_new(m_network.hostname().data());
    json_value *gateway = json_string_new(m_network.gateway().data());
    json_value *gateway6 = json_string_new(m_network.gateway6().data());
    json_value *gadev = json_string_new(m_network.gwdev().data());
    json_value *nat = json_string_new(m_network.nat().data());
    json_value *dns = json_string_new(m_network.dns().data());
    json_value *interface_array = json_array_new(0);

    size = m_network.interfaces_size();

    for (i = 0; i<size; ++i)
    {
        json_value *interface = _SerializeNetInterface(m_network.interfaces(i));
        json_array_push(interface_array, interface);
    }

    json_object_push(network_obj, "Host", host_name);
    json_object_push(network_obj, "Gateway", gateway);
    json_object_push(network_obj, "Gateway6", gateway6);
    json_object_push(network_obj, "GatewayDevice", gadev);
    json_object_push(network_obj, "NAT", nat);
    json_object_push(network_obj, "DNS", dns);
    json_object_push(network_obj, "Interfaces", interface_array);

    json_object_push(obj, "Network", network_obj);
}

void SessionData::SerializeRect(json_value *obj, const ::TvWall::Rect& r)
{
    json_value* geometry = json_object_new(0);
    json_value *x = json_integer_new(r.x());
    json_value *y = json_integer_new(r.y());
    json_value *w = json_integer_new(r.w());
    json_value *h = json_integer_new(r.h());

    json_object_push(geometry, "X", x);
    json_object_push(geometry, "Y", y);
    json_object_push(geometry, "Width", w);
    json_object_push(geometry, "Height", h );
    json_object_push(obj, "Geometry", geometry);
}

void SessionData::SerializePoint(json_value *obj, const ::TvWall::Point& p)
{
    json_value* point = json_object_new(0);
    json_value *x = json_integer_new(p.x());
    json_value *y = json_integer_new(p.y());

    json_object_push(point, "X", x);
    json_object_push(point, "Y", y);
    json_object_push(obj, "Point", point);
}

json_value* SessionData::_SerializeCameraGroup(const PKT::ElementDat::CameraGrp& camgrp)
{
    int i;
    json_value *group = json_object_new(0);
    json_value *id = json_string_new(camgrp.grpid().data().data());
    json_value *name = json_string_new(camgrp.grpname().data());
    json_object_push(group, "Id", id);
    json_object_push(group, "Name", name);

    if (camgrp.has_isdel())
    {
        json_value *is_del = json_boolean_new(camgrp.isdel());
        json_object_push(group, "IsDel", is_del);
    }

    if (camgrp.has_parentid())
    {
        json_value *parent_id = json_string_new(camgrp.parentid().data().data());
        json_object_push(group, "ParentId", parent_id);
    }

    //serialize subgroup
    json_value *subgroups = json_array_new(0);
    for (i = 0; i<camgrp.camgrp_size(); i++)
    {
        json_value *subgroup = _SerializeCameraGroup(camgrp.camgrp(i));
        json_array_push(subgroups, subgroup);
    }
    json_object_push(group, "Children", subgroups);

    //serialize camera in this group
    json_value *cameras = json_array_new(0);
    for (i = 0; i<camgrp.cam_size(); i++)
    {
        json_value *cam_obj = _SerializeCamera(camgrp.cam(i));
        json_array_push(cameras, cam_obj);
    }
    json_object_push(group, "Cameras", cameras);
    return group;
}

json_value* SessionData::_SerializeScreen(const PKT::ElementDat::Screen& scrn)
{
    json_value *screen_obj = json_object_new(0);
    json_value *screen_index = json_integer_new(scrn.screenindex());
    json_value *row = json_integer_new(scrn.row());
    json_value *col = json_integer_new(scrn.col());
    SerializeRect(screen_obj, scrn.geometry());
    json_object_push(screen_obj, "Index", screen_index);
    json_object_push(screen_obj, "Row", row);
    json_object_push(screen_obj, "Col", col);
    return screen_obj;
}

json_value* SessionData::_SerializeView(const PKT::ElementDat::View& v)
{
    int i;
    json_value *view_obj = json_object_new(0);
    json_value *winid = json_integer_new(v.winid());
    json_object_push(view_obj, "WinId", winid);
    SerializeRect(view_obj, v.rect());
    json_value *view_type = json_integer_new(v.viewtype());
    json_object_push(view_obj, "Type", view_type);

    json_value *display_obj = json_object_new(0);
    json_object_push(view_obj, "Display", display_obj);

    if (v.has_url())
    {
        json_value *url = json_string_new(v.url().data());
        json_object_push(view_obj, "Url", url);
    }

    json_value *camera_ids = json_array_new(0);
    for (i = 0; i<v.cameraid_size(); ++i)
    {
        json_value *camid = json_string_new(v.cameraid(i).data().data());
        json_array_push(camera_ids, camid);
    }
    json_object_push(view_obj, "Cameras", camera_ids);
    if (v.has_starttime())
    {
        json_value *starttime = json_double_new((double)(v.starttime()));
        json_object_push(view_obj, "StartTime", starttime);
    }

    if (v.has_keepstretch())
    {
        json_value *keep_stretch = json_boolean_new((v.keepstretch()));
        json_object_push(display_obj, "KeepStretch", keep_stretch);
    }
    if (v.has_borderwidth())
    {
        json_value *border_width = json_integer_new(v.borderwidth());
        json_object_push(display_obj, "BorderWidth", border_width);
    }

    if (v.has_showtitle())
    {
        json_value *show_title = json_boolean_new(v.showtitle());
        json_object_push(display_obj, "ShowTitle", show_title);
    }

    if (v.has_title())
    {
        json_value *title = json_string_new(v.title().data());
        json_object_push(display_obj, "Title", title);
    }

    if (v.has_pollinterval())
    {
        json_value *poll_interval = json_integer_new(v.pollinterval());
        json_object_push(view_obj, "PollInterval", poll_interval);
    }

    if (v.has_gpuid())
    {
        json_value *gpu_id = json_integer_new(v.gpuid());
        json_object_push(view_obj, "GPUId", gpu_id);
    }

    if (v.has_showfullscreen())
    {
        json_value *show_full_screen = json_boolean_new(v.showfullscreen());
        json_object_push(display_obj, "ShowFullScreen", show_full_screen);
    }

    if (v.has_windowstack())
    {
        json_value *window_stack = json_integer_new(v.windowstack());
        json_object_push(display_obj, "WindowStack", window_stack);
    }

    if (v.has_isalarm())
    {
        json_value *is_alarm = json_boolean_new(v.isalarm());
        json_object_push(view_obj, "IsAlarm", is_alarm);
    }

    if (v.has_alarminterval())
    {
        json_value *alarm_interval = json_integer_new(v.alarminterval());
        json_object_push(view_obj, "AlarmInterval", alarm_interval);
    }

    if (v.has_showmaxfullscreen())
    {
        json_value *show_max_full_screen = json_boolean_new(v.showmaxfullscreen());
        json_object_push(display_obj, "ShowMaxFullScreen", show_max_full_screen);
    }

    if (v.has_showminfullscreen())
    {
        json_value *show_min_full_screen = json_boolean_new(v.showminfullscreen());
        json_object_push(display_obj, "ShowMaxFullScreen", show_min_full_screen);
    }

    if (v.has_fullscreenrect())
    {
        SerializeRect(display_obj, v.fullscreenrect());
    }
    return view_obj;
}

json_value* SessionData::_SerializeCamera(const PKT::ElementDat::Camera& cam)
{
    json_value *obj = json_object_new(0);
    json_value *id = json_string_new(cam.cameraid().data().data());

    json_object_push(obj, "Id", id);

    if (cam.has_camerainfo())
    {
        json_value *ip = json_string_new(cam.camerainfo().cameraip().data());
        json_value *port = json_integer_new(cam.camerainfo().cameraport());

        json_object_push(obj, "IP", ip);
        json_object_push(obj, "Port", port);

        if (cam.camerainfo().has_cameraname())
        {
            json_value *name = json_string_new(cam.camerainfo().cameraname().data());
            json_object_push(obj, "Name", name);
        }
        if (cam.camerainfo().has_width())
        {
            json_value *w = json_integer_new(cam.camerainfo().width());
            json_object_push(obj, "Width", w);
        }
        if (cam.camerainfo().has_height())
        {
            json_value *h = json_integer_new(cam.camerainfo().width());
            json_object_push(obj, "Height", h);
        }
        if (cam.camerainfo().has_canptz())
        {
            json_value *canptz = json_boolean_new(cam.camerainfo().canptz());
            json_object_push(obj, "CanPTZ", canptz);
        }
        if (cam.camerainfo().has_composite())
        {
            json_value *composite = json_boolean_new(cam.camerainfo().composite());
            json_object_push(obj, "Composite", composite);
        }
    }
    if (cam.has_recordinfo())
    {
        json_value *begin = json_double_new(cam.recordinfo().begin());
        json_object_push(obj, "Begin", begin);
    }
    if (cam.has_isplaying())
    {
        json_value *playing = json_boolean_new(cam.isplaying());
        json_object_push(obj, "IsPlaying", playing);
    }
    if (cam.has_isconnected())
    {
        json_value *connected = json_boolean_new(cam.isconnected());
        json_object_push(obj, "IsConnected", connected);
    }
    if (cam.has_isdel())
    {
        json_value *del = json_boolean_new(cam.isdel());
        json_object_push(obj, "IsDel", del);
    }

    if (cam.has_isplayback())
    {
        json_value *is_playback = json_boolean_new(cam.isplayback());
        json_object_push(obj, "IsPlayback", is_playback);
    }

    if (cam.has_parentid())
    {
        json_value *parent_id = json_string_new(cam.parentid().data().data());
        json_object_push(obj, "ParentId", parent_id);
    }
    return obj;
}

json_value* SessionData::_SerializeMap(const PKT::ElementDat::Map& map)
{
    int i;

    json_value *obj = json_object_new(0);
    json_value *url = json_string_new(map.mapurl().data());

    json_object_push(obj, "Url", url);

    json_value *submaps = json_array_new(0);
    for (i = 0; i<map.map_size(); ++i)
    {
        json_value *map_obj = _SerializeMap(map.map(i));
        json_array_push(submaps, map_obj);
    }
    json_object_push(obj, "SubMaps", submaps);
    return obj;
}

json_value* SessionData::_SerializeNvr(const ::TvWall::NVR& nvr)
{
    json_value *obj = json_object_new(0);
    json_value *ip = json_string_new(nvr.ip().data());
    json_value *user = json_string_new(nvr.username().data());
    json_value *pass = json_string_new(nvr.password().data());
    json_value *connected = json_boolean_new(nvr.connected());

    json_object_push(obj, "IP", ip);
    json_object_push(obj, "UserName", user);
    json_object_push(obj, "Password", pass);
    json_object_push(obj, "Connected", connected);

    return obj;
}

json_value* SessionData::_SerializeFmp(const ::TvWall::FMP& fmp)
{
    json_value *obj = json_object_new(0);
    json_value *ip = json_string_new(fmp.ip().data());
    json_value *user = json_string_new(fmp.username().data());
    json_value *pass = json_string_new(fmp.password().data());

    json_object_push(obj, "IP", ip);
    json_object_push(obj, "UserName", user);
    json_object_push(obj, "Password", pass);
    return obj;
}

json_value* SessionData::_SerializeNetInterface(const ::TvWall::NetInterface& it)
{
    json_value *obj = json_object_new(0);
    json_value *name = json_string_new(it.ifname().data());
    json_value *enable = json_boolean_new(it.enabled());
    json_value *plug = json_boolean_new(it.plug());
    json_value *dhcp = json_boolean_new(it.dhcp());
    json_value *speed = json_integer_new(it.speed());
    json_value *ip = json_string_new(it.ip().data());
    json_value *mask = json_string_new(it.netmask().data());

    json_object_push(obj, "Name", name);
    json_object_push(obj, "IP", ip);
    json_object_push(obj, "Mask", mask);
    json_object_push(obj, "DHCP", dhcp);
    json_object_push(obj, "Plugged", plug);
    json_object_push(obj, "Enabled", enable);
    json_object_push(obj, "Speed", speed);

    return obj;
}

json_value* SessionData::_SerializeCameraStat(const PKT_STAT::CameraStat& s)
{
    json_value *obj = json_object_new(0);
    json_value *type = json_integer_new(s.type());

    json_object_push(obj, "Type", type);
    
    if (s.type() == PKT_STAT::CameraStat::NETSTAT ||
        s.type() == PKT_STAT::CameraStat::PLAYING_STAT ||
        s.type() == PKT_STAT::CameraStat::PLAYBACK_STAT ||
        s.type() == PKT_STAT::CameraStat::UPDATE ||
        s.type() == PKT_STAT::CameraStat::ADD ||
        s.type() == PKT_STAT::CameraStat::REMOVE)
    {
        json_value *cam_obj = _SerializeCamera(s.cam());
        json_object_push(obj, "Camera", cam_obj);
    }
    else if (s.type() == PKT_STAT::CameraStat::ADD_GRP ||
             s.type() == PKT_STAT::CameraStat::REMOVE_GRP ||
             s.type() == PKT_STAT::CameraStat::UPDATE_GRP)
    {
        json_value *grp_obj = _SerializeCameraGroup(s.grp());
        json_object_push(obj, "Group", grp_obj);
    }
    return obj;
}

PKT::ElementDat::View* SessionData::LookupView(int winid, int *index)
{
    PKT::ElementDat::View *v = NULL;
    int i;
    for (i = 0; i<m_element->view_size(); i++)
    {
        PKT::ElementDat::View *p = m_element->mutable_view(i);
        if (winid == p->winid())
        {
            v = p;
            break;
        }
    }
    if (index) *index = i;
    return v;
}

PKT::ElementDat::CameraGrp* SessionData::_LookupCameraGroup(PKT::ElementDat::CameraGrp* grp, const TvWall::Guid& id, bool remove)
{
    if (grp->grpid().data() == id.data())
    {
        return grp;
    }
    
    PKT::ElementDat::CameraGrp *found;
    int i, size;
    size = grp->camgrp_size();
    for (i = 0; i<size; ++i)
    {
        found = _LookupCameraGroup(grp->mutable_camgrp(i), id, remove);
        if (found){
            if (remove){
                grp->mutable_camgrp()->DeleteSubrange(i, 1);
            }
            return found;
        }
    }
    return NULL;
}

PKT::ElementDat::Camera* SessionData::_LookupCamera(PKT::ElementDat::CameraGrp* grp, const TvWall::Guid& id, bool remove)
{
    PKT::ElementDat::Camera *found;
    int i, size;
    size = grp->cam_size();
    for (i = 0; i<size; ++i)
    {
        found = grp->mutable_cam(i);
        if (found->cameraid().data() == id.data())
        {
            if (remove)
            {
                grp->mutable_cam()->DeleteSubrange(i, 1);
            }
            return found;
        }
    }

    size = grp->camgrp_size();
    for (i = 0; i<size; ++i)
    {
        found = _LookupCamera(grp->mutable_camgrp(i), id, remove);
        if (found){
            return found;
        }
    }
    return NULL;
}

PKT::ElementDat::CameraGrp* SessionData::LookupCameraGroup(const TvWall::Guid& id, bool remove)
{
    int i, size;
    size = m_element->camgrp_size();
    PKT::ElementDat::CameraGrp *found;

    for (i = 0; i<size; ++i)
    {
        PKT::ElementDat::CameraGrp *grp = m_element->mutable_camgrp(i);
        if (grp->grpid().data() == id.data())
        {
            if (remove){
                m_element->mutable_camgrp()->DeleteSubrange(i, 1);
            }
            return grp;
        }
        found = _LookupCameraGroup(grp, id, remove);
        if (found){
            return found;
        }
    }
    return NULL;
}

PKT::ElementDat::Camera* SessionData::LookupCamera(const TvWall::Guid& id, bool remove)
{
    int i, size;
    size = m_element->camgrp_size();
    PKT::ElementDat::Camera *found;

    for (i = 0; i<size; ++i)
    {
        PKT::ElementDat::CameraGrp *grp = m_element->mutable_camgrp(i);
        found = _LookupCamera(grp, id, remove);
        if (found){
            return found;
        }
    }
    return NULL;
}

void SessionData::LookupAddCameraGroup(const PKT::ElementDat::CameraGrp& grp)
{
    PKT::ElementDat::CameraGrp* parent = LookupCameraGroup(grp.parentid());

    if (parent){
        *(parent->add_camgrp()) = grp;
    }else{
        *(m_element->add_camgrp()) = grp;
    }
}

void SessionData::LookupAddCamera(const PKT::ElementDat::Camera& cam)
{
    PKT::ElementDat::CameraGrp* parent = LookupCameraGroup(cam.parentid());

    if (parent){
        *(parent->add_cam()) = cam;
    }else{
        *(m_element->mutable_camgrp(0)->add_cam()) = cam; //TODO: add to first group
    }
}

void SessionData::RemoveCamDupNetStat(const ::TvWall::Guid& camid)
{
    GrAutoLockT<GrRecursiveMutex> guard(m_camstat_mutex);

    CameraStatNoticeIt it;
    PKT_STAT::CameraStat *stat;
    for (it = m_camera_notice.begin(); it != m_camera_notice.end(); ++it)
    {
        stat = *it; 

        if (stat->type() == PKT_STAT::CameraStat::NETSTAT &&
            stat->cam().cameraid().data() == camid.data())
        {
            delete stat;
            m_camera_notice.erase(it);
            break;
        }
    }
}

/*******************************notice serialization***********************/

void SessionData::SerializeScreenNotice(json_value *obj)
{
    int i, size, screen_index;
    ScreenNoticeIt it;
    size = m_element->screen_size();
    it = m_scrn_notice.begin();

    json_value *screen_array = json_array_new(0);

    while (m_scrn_notice.size() > 0)
    {
        it = m_scrn_notice.begin();
        screen_index = *it;
        m_scrn_notice.erase(it);

        for (i = 0; i<size; ++i)
        {
            const PKT::ElementDat::Screen &screen = m_element->screen(i);
            if (screen.screenindex() == screen_index)
            {
                json_value *screen_obj = _SerializeScreen(screen);
                json_array_push(screen_array, screen_obj);
                break;
            }
        }
    }
    json_object_push(obj, "Screens", screen_array);
}

void SessionData::SerializeViewGeoNotice(json_value *obj)
{
    ViewGeometryNoticeIt it;
    json_value *view_geo_array = json_array_new(0);

    while (m_viewgeo_notice.size() > 0)
    {
        it = m_viewgeo_notice.begin();
        const PKT_VIEW_STAT::ViewGeomotryStat* v = *it;
        json_value *geo_obj = json_object_new(0);
        json_value *type = json_integer_new(int(v->type()));
        json_object_push(geo_obj, "Type", type);
        json_value *view_obj = _SerializeView(v->view());
        json_object_push(geo_obj, "View", view_obj);
        json_array_push(view_geo_array, geo_obj);
        delete v;
        m_viewgeo_notice.erase(it);
    }
    json_object_push(obj, "Views", view_geo_array);
}

void SessionData::SerializeViewVideoNotice(json_value *obj)
{
    ViewVideoNoticeIt it;
    json_value *view_array = json_array_new(0);

    while (m_video_notice.size() > 0)
    {
        it = m_video_notice.begin();
        PKT::ElementDat::View *v = LookupView(*it);
        if (v)
        {
            json_value *view_obj = _SerializeView(*v);
            json_array_push(view_array, view_obj);
        }
        m_video_notice.erase(it);
    }
    json_object_push(obj, "Views", view_array);
}

void SessionData::SerializeViewFHLMNotice(json_value *obj)
{
    ViewFHLMNoticeIt it;
    json_value *view_array = json_array_new(0);

    while (m_fhlm_notice.size() > 0)
    {
        it = m_fhlm_notice.begin();
        PKT::ElementDat::View *v = LookupView(*it);
        if (v)
        {
            json_value *view_obj = _SerializeView(*v);
            json_array_push(view_array, view_obj);
        }
        m_fhlm_notice.erase(it);
    }
    json_object_push(obj, "Views", view_array);
}

void SessionData::SerializeLayoutNotice(json_value *obj)
{
    if (!m_layout_changed)
    {
        json_value *layout_array = json_array_new(0);
        json_object_push(obj, "ServerLayouts", layout_array);
        return;
    }

    m_layout_changed = false;
    SerializeLayout(obj);
}

void SessionData::SerializeLayoutLoopNotice(json_value *obj)
{
    if (!m_layoutloop_changed)
    {
        json_value *array = json_array_new(0);
        json_object_push(obj, "LayoutLoops", array);
        return;
    }

    m_layoutloop_changed = false;
    SerializeLayoutLoop(obj);
}

void SessionData::SerializeCameraNotice(json_value *obj)
{
    /*if (!m_camera_changed)
    {
        json_value *groups = json_array_new(0);
        json_object_push(obj, "Groups", groups);
        return;
    }
    SerializeCameraGroup(obj);*/

    GrAutoLockT<GrRecursiveMutex> guard(m_camstat_mutex);

    CameraStatNoticeIt it;
    json_value *stat_array = json_array_new(0);
    PKT_STAT::CameraStat *stat;

    while (m_camera_notice.size() > 0)
    {
        it = m_camera_notice.begin();
        stat = *it;
        json_value *stat_obj = _SerializeCameraStat(*stat);
        json_array_push(stat_array, stat_obj);
        delete stat;
        m_camera_notice.erase(it);
    }
    json_object_push(obj, "CameraStats", stat_array);
}

void SessionData::SerializeHotPointNotice(json_value *obj)
{
    //if (!m_hotpoint_changed)
    //    return;

    //m_hotpoint_changed = false;
    SerializeHotPoint(obj);
}

void SessionData::SerializeRecordNotice(json_value *obj)
{
    json_value *record_ack_obj = json_object_new(0);
    json_value *winid = json_integer_new(m_recordack.winid());
    json_value *camid = json_string_new(m_recordack.cameraid().data().data());
    json_value *curtime = json_integer_new(m_recordack.currenttime()/1000); //convert to second
    json_value *rate = json_integer_new(m_recordack.vary());
    json_value *pause = json_boolean_new(m_recordack.ispause());

    json_object_push(record_ack_obj, "WinId", winid);
    json_object_push(record_ack_obj, "CameraId", camid);
    json_object_push(record_ack_obj, "CurrentTime", curtime);
    json_object_push(record_ack_obj, "Rate", rate);
    json_object_push(record_ack_obj, "IsPause", pause);

    json_object_push(obj, "RecordStatus", record_ack_obj);
}
