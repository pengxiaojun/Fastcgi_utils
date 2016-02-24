#ifndef _TVW_SESSION
#define _TVW_SESSION

#include <sysinc.hpp>
#include <grutil.hpp>
#include <uninet.hpp>
#include "types.h"
#include "session_data.h"


class Session
{
public:
    Session();
    ~Session();

    const guid_t *GetId();

    int Send(PKT& pkg);

    //connect
    void Connect(const PKT_SESSION::Login& login);
    void Disconnect();
    void OnConnected(IUniLink *lnk, const void *ud);
    void OnConnectFail(void *ud, const sockaddr_inet &peer);
    bool IsEstablish();


    //data
    http_ret_t Logout();
    http_ret_t QueryData(int type = PKT_ReqSession::ALL);
    http_ret_t QueryDataAck(int action);
    http_ret_t QueryNotice(int action);
    http_ret_t QueryNetworkStat();
    http_ret_t QueryNetwork();
    http_ret_t QueryNetworkAck();

    //control
    http_ret_t CtrlScreenSplit(PKT_CTRL::ScreenSplitCtrl& ssc);
    http_ret_t CtrlViewGeometry(PKT_ViewGeoCtrl& vgc);
    http_ret_t CtrlVideoView(PKT_VIEW_CTRL::VideoCtrl& vc);
    http_ret_t CtrlRecordView(TvWall::Packet_Ctrl_ViewCtrl::RecordCtrl& rc);
    http_ret_t CtrlFlashView(PKT_VIEW_CTRL::FlashCtrl& fc);
    http_ret_t CtrlHTMLView(PKT_VIEW_CTRL::HTMLCtrl& hc);
    http_ret_t CtrlTextView(PKT_VIEW_CTRL::TextCtrl& tc);
    http_ret_t CtrlLocalMovie(PKT_VIEW_CTRL::LocalMovieCtrl& clm);
    http_ret_t CtrlMap(PKT_VIEW_CTRL::MapCtrl& mc);
    http_ret_t CtrlCamera(PKT_CTRL::CameraCtrl& cc);
    http_ret_t CtrlHotPoint(PKT_CTRL::HotPointCtrl& hpc);
    http_ret_t CtrlLayout(PKT_CTRL::LayoutCtrl& lc);
    http_ret_t CtrlLayoutLoop(PKT::Ctrl::LayoutLoopCtrl& llc);
    http_ret_t CtrlMachine(PKT_CTRL::MachineCtrl& mc);
    http_ret_t CtrlKey(PKT_CTRL::KeyCtrl& kc);
    http_ret_t CtrlKeyCmd(PKT_CTRL::KeyCmdCtrl& kcc);
    http_ret_t CtrlAlarm(PKT_CTRL::AlarmCtrl& ac);
    http_ret_t CtrlMLY(PKT_CtrlMasterLyout& cml);

    // data serialize
    void SerializeData(json_value *obj);
    void SerializeNotice(json_value *obj, int action);
    void SerializeAck(json_value *obj, int action);

    //
    bool LookupView(int winid, PKT::ElementDat::View& view);
private:
    //receive
    void OnPDU(const void* ud, const void* buf, int len);
    //disconnect
    void OnDisconnected(void* ud);

    //protobuf
    void Login(PKT_SESSION::Login& login);
private:
    guid_t m_sid;
    bool m_logined;
    UniConnector *m_conn;
    IUniLink *m_link;
    PKT_SESSION::Login m_login;
    SessionData m_data;
};


#endif
