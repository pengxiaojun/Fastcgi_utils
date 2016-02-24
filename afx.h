#ifndef AFX_H
#define AFX_H

#include "protobuf/tvWall.pb.h"

typedef ::TvWall::Packet                                             PKT;
typedef ::TvWall::Packet::Session                                    PKT_SESSION;
typedef ::TvWall::Packet::Ctrl                                       PKT_CTRL;
typedef ::TvWall::Packet::Ctrl::ViewCtrl                             PKT_VIEW_CTRL;
typedef ::TvWall::Packet::Ctrl::ViewCtrl::ViewGeomotryCtrl           PKT_ViewGeoCtrl;

typedef ::TvWall::Packet::Notice::Stat                               PKT_STAT;
typedef ::TvWall::Packet::Notice::Stat::ViewStat                     PKT_VIEW_STAT;
typedef ::TvWall::Packet::Notice::Stat::ViewStat::ViewGeomotryStat   PKT_ViewGeoStat;
typedef ::TvWall::Packet::RequireSession                             PKT_ReqSession;
typedef ::TvWall::Packet::RequireSession::ClientRequire              PKT_ClientReq;

typedef ::TvWall::Packet::ElementMaster::MasterLayout                PKT_MasterLayout;
typedef ::TvWall::Packet::ElementMaster::MasterLayout::MasLayout     PKT_MasLayout;
typedef ::TvWall::Packet::Ctrl::CtrlMaster::CtrlMasterLyout          PKT_CtrlMasterLyout;
typedef ::TvWall::Packet::Session::LoginMasterAck::Server            PKT_Server;
typedef ::TvWall::Packet::Notice::NoticeMaster::NoticeMasterLyout    PKT_NoticeMasterLyout;
#endif // AFX_H
