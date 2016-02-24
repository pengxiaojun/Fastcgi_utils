#ifndef _TVW_PARSER
#define _TVW_PARSER

#include <sysinc.hpp>
#include "types.h"
#include "afx.h"

class HttpRequest;


class SessionDataParser
{
public:
    SessionDataParser();
    ~SessionDataParser();

    http_ret_t ParseQuery(HttpRequest *request, int* type);
    http_ret_t ParseLogin(HttpRequest *request, PKT_SESSION::Login& login);
    http_ret_t ParseScreenSplit(HttpRequest *request, PKT_CTRL::ScreenSplitCtrl& ssc);
    http_ret_t ParseViewGeometry(HttpRequest *request, PKT_ViewGeoCtrl& vgc);
    http_ret_t ParseViewVideo(HttpRequest *request, PKT_VIEW_CTRL::VideoCtrl& vc);
    http_ret_t ParseViewRecord(HttpRequest *request, PKT_VIEW_CTRL::RecordCtrl& rc);
    http_ret_t ParseViewFlash(HttpRequest *request, PKT_VIEW_CTRL::FlashCtrl& fc);
    http_ret_t ParseViewHtml(HttpRequest *request, PKT_VIEW_CTRL::HTMLCtrl& hc);
    http_ret_t ParseViewText(HttpRequest *request, PKT_VIEW_CTRL::TextCtrl& tc);
    http_ret_t ParseViewLocalMovie(HttpRequest *request, PKT_VIEW_CTRL::LocalMovieCtrl& lmc);
    http_ret_t ParseViewMap(HttpRequest *request, PKT_VIEW_CTRL::MapCtrl& mc);
    http_ret_t ParseCameraCtrl(HttpRequest *request, PKT_CTRL::CameraCtrl& lc);
    http_ret_t ParseLayoutCtrl(HttpRequest *request, PKT_CTRL::LayoutCtrl& lc);
    http_ret_t ParseLayoutLoopCtrl(HttpRequest *request, PKT_CTRL::LayoutLoopCtrl& llc);
    http_ret_t ParseMachineCtrl(HttpRequest *request, PKT_CTRL::MachineCtrl& mc);
};

#endif
