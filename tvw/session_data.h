#ifndef _TVW_SESSION_DATA
#define _TVW_SESSION_DATA

#include <sysinc.hpp>
#include <grthread.hpp>
#include "afx.h"
#include "3rd/json-builder.h"

class Session;
	
typedef std::set<int> ScreenNotice;
typedef std::set<int>::iterator ScreenNoticeIt;

typedef std::vector<PKT_VIEW_STAT::ViewGeomotryStat*> ViewGeometryNotice;
typedef std::vector<PKT_VIEW_STAT::ViewGeomotryStat*>::iterator ViewGeometryNoticeIt;

typedef std::vector<PKT_STAT::CameraStat*> CameraStatNotice;
typedef std::vector<PKT_STAT::CameraStat*>::iterator CameraStatNoticeIt;

//typedef std::vector<PKT_VIEW_STAT::VideoStat*> ViewVideoNotice;
//typedef std::vector<PKT_VIEW_STAT::VideoStat*>::iterator ViewVideoNoticeIt;
typedef std::set<int> ViewVideoNotice;
typedef std::set<int>::iterator ViewVideoNoticeIt;

typedef std::set<int> ViewFHLMNotice;   //FHLM=flash+html+localmovie+map
typedef std::set<int>::iterator ViewFHLMNoticeIt;


class SessionData
{
public:
    SessionData(Session* session);
    ~SessionData();
    void Initialize();

    void SetData(const PKT_ReqSession::ClientRequireAck& ack);
    void SetNetworkData(const PKT_ReqSession::NetRequireAck& ack);
    void SetRecordAck(const PKT_ReqSession::RecordRequireAck& ack);
    void SerializeData(json_value *obj);

    void SetNotice(const PKT::Notice& notice);
    void SerializeNotice(json_value *obj);

    //session data
    void SerializeBasicInfo(json_value *obj);
    void SerializeCameraGroup(json_value* obj);
    void SerializeLocalMovie(json_value *obj);
    void SerializeFlash(json_value *obj);
    void SerializeLayout(json_value *obj);
    void SerializeLayoutLoop(json_value *obj);
    void SerializeHotPoint(json_value *obj);
    void SerializeView(json_value *obj);
    void SerializeMap(json_value *obj);
    void SerializeScreen(json_value *obj);
    void SerializeNvr(json_value *obj);
    void SerializeFmp(json_value *obj);
    void SerializeNetwork(json_value *obj);

    //notice
    void SerializeScreenNotice(json_value *obj);
    void SerializeViewGeoNotice(json_value *obj);
    void SerializeViewVideoNotice(json_value *obj);
    void SerializeViewFHLMNotice(json_value *obj);
    void SerializeLayoutNotice(json_value *obj);
    void SerializeLayoutLoopNotice(json_value *obj);
    void SerializeCameraNotice(json_value *obj);
    void SerializeHotPointNotice(json_value *obj);
    void SerializeRecordNotice(json_value *obj);

    //
    PKT::ElementDat::View* LookupView(int winid, int *index=NULL);
    PKT::ElementDat::CameraGrp* LookupCameraGroup(const TvWall::Guid& id, bool remove=false);
    PKT::ElementDat::Camera* LookupCamera(const TvWall::Guid& id, bool remove=false);
    void LookupAddCameraGroup(const PKT::ElementDat::CameraGrp& grp);
    void LookupAddCamera(const PKT::ElementDat::Camera& cam); 
private:
    //element
    void SerializeRect(json_value *obj, const ::TvWall::Rect& r);
    void SerializePoint(json_value *obj, const ::TvWall::Point& p);
    json_value* _SerializeCameraGroup(const PKT::ElementDat::CameraGrp& camgrp);
    json_value* _SerializeCamera(const PKT::ElementDat::Camera& cam);
    json_value* _SerializeScreen(const PKT::ElementDat::Screen& screen);
    json_value* _SerializeView(const PKT::ElementDat::View& view);
    json_value* _SerializeMap(const PKT::ElementDat::Map& map);
    json_value* _SerializeNvr(const ::TvWall::NVR& nvr);
    json_value* _SerializeFmp(const ::TvWall::FMP& fmp);
    json_value* _SerializeNetInterface(const ::TvWall::NetInterface& it);
    json_value* _SerializeCameraStat(const PKT_STAT::CameraStat& s);

    //notice
    void SetScreenSplitStat(const PKT_STAT::ScreenSpliStat& split);
    void SetViewStat(const PKT_VIEW_STAT& viewstat);
    void SetCameraStat(const PKT_STAT::CameraStat& s);
    void SetHotPointStat(const PKT_STAT::HotPointStat& s);
    void SetLayoutStat(const PKT_STAT::LayoutStat& s);
    void SetLayoutLoopStat(const PKT_STAT::LayoutLoopStat& s);

    //lookup
    PKT::ElementDat::CameraGrp* _LookupCameraGroup(PKT::ElementDat::CameraGrp* grp, const TvWall::Guid& id, bool remove);
    PKT::ElementDat::Camera* _LookupCamera(PKT::ElementDat::CameraGrp* grp, const TvWall::Guid& id, bool remove);

    void RemoveCamDupNetStat(const ::TvWall::Guid& camid);
private:
    Session *m_session;
    PKT::ElementDat* m_element;
    TvWall::Network m_network;
    PKT_ReqSession::RecordRequireAck m_recordack;

    //screen notice
    ScreenNotice m_scrn_notice;

    //store view notice when status changed
    ViewGeometryNotice m_viewgeo_notice;
    ViewFHLMNotice m_fhlm_notice;
    ViewVideoNotice m_video_notice;
    CameraStatNotice m_camera_notice;

    //the following boolean vlaue indicate state changed
    bool m_layout_changed;
    bool m_layoutloop_changed;
    bool m_hotpoint_changed;

    GrRecursiveMutex m_camstat_mutex;
};


#endif
