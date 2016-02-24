#include "session_parser.h"
#include "request.h"
#include "logger.h"
#include <3rd/json.h>


SessionDataParser::SessionDataParser()
{
}

SessionDataParser::~SessionDataParser()
{
}

http_ret_t SessionDataParser::ParseQuery(HttpRequest *request, int* type)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_query_data, ""};

    len = request->GetContentLength();
    if (len == 0)
        return r;

    json = (json_char*)request->GetPostData();
    if (json == NULL)
        return r;

    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse query data error";
        return r;
    }
    /*
     * {
     *	"type" : "user",
     * }
     */
    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            *type = attr->u.integer;
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseLogin(HttpRequest* request, PKT_SESSION::Login& login)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_login, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse login data error";
        return r;
    }
    /*
     * {
     *	"username" : "user",
     *	"password" : "password",
     *	"peer" : "192.169.1.1:4555",
     * }
     */
    len = value->u.object.length;
    if (len < 3)
    {
        r.code = -1;
        r.message = "Login data at least contain 4 elements";
        return r;
    }
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "username") &&
                attr->type == json_string &&
                attr->u.string.ptr != NULL)
        {
            login.set_username(attr->u.string.ptr);
        }

        if (!strcmp(name, "password") &&
                attr->type == json_string &&
                attr->u.string.ptr != NULL)
        {
            login.set_passwd(attr->u.string.ptr);
        }

        if (!strcmp(name, "peer") &&
                attr->type == json_string &&
                attr->u.string.ptr != NULL)
        {
            login.set_version(attr->u.string.ptr);  //XXX: store peer to version field
        }
    }
    if (login.username().empty() ||
        login.passwd().empty())
    {
        r.code = E_POSTDATA;
        r.message = "Invalid user or password";
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseScreenSplit(HttpRequest *request, PKT_CTRL::ScreenSplitCtrl& ssc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_screen_split, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse screen split data error";
        return r;
    }
    /*
     * {
     *	"screenindex" : 0,
     *	"row" : 2,
     *	"col" : 2,
     * }
     */
    len = value->u.object.length;
    if (len < 3)
    {
        r.code = E_POSTDATA;
        r.message = "Screensplit data at least contain 3 elements";
        return r;
    }
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "screenindex") &&
                attr->type == json_integer)
        {
            ssc.set_screenindex(attr->u.integer);
        }

        if (!strcmp(name, "row") &&
                attr->type == json_integer)
        {
            ssc.set_row(attr->u.integer);
        }

        if (!strcmp(name, "col") &&
                attr->type == json_integer)
        {
            ssc.set_col(attr->u.integer);
        }
    }
    if (ssc.screenindex() < 0 ||
            ssc.row() < 0 ||
            ssc.col() < 0)
    {
        r.code = -1;
        r.message = "screenindex, col and row data must greater than 0";
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseViewGeometry(HttpRequest *request, PKT_ViewGeoCtrl& vgc)
{
    json_char *json;
    json_value *value, *attr, *subattr;
    int i, j, len, sublen;
    char *name, *subname;

    http_ret_t r = {E_SUCCESS, ra_ctl_view_geo, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view geometry data error";
        return r;
    }


    /*
     * {
     *	"winid" : 1,
     *	"action" : 2,  //create=0; window_control=1; remove=2; set_border_width=3; show_title=4
     *	"width" : 2, //effected whe type is set border width
     *	"showtitle" : true,
     * }
     */
    PKT::ElementDat::View *v = vgc.mutable_view();

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "winid") && attr->type == json_integer)
        {
            v->set_winid(attr->u.integer);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            vgc.set_type((PKT_ViewGeoCtrl::ViewGeomotryCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "borderwidth") && attr->type == json_integer)
        {
            v->set_borderwidth(attr->u.integer);
        }

        if (!strcmp(name, "showtitle") && attr->type == json_boolean)
        {
            v->set_showtitle(attr->u.integer);
        }

        if (!strcmp(name, "showfullscreen") && attr->type == json_boolean)
        {
            v->set_showfullscreen(attr->u.integer);
        }

        if (!strcmp(name, "showmaxfullscreen") && attr->type == json_boolean)
        {
            v->set_showfullscreen(attr->u.integer);
            v->set_showmaxfullscreen(attr->u.integer);
            v->clear_fullscreenrect(); 
        }

        if (!strcmp(name, "showminfullscreen") && attr->type == json_boolean)
        {
            v->set_showminfullscreen(attr->u.integer);
        }

        if (!strcmp(name, "windowstack") && attr->type == json_integer)
        {
            v->set_windowstack(attr->u.integer);
        }

        if (!strcmp(name, "url") && attr->type == json_string)
        {
            v->set_url(attr->u.string.ptr);
        }

        if (!strcmp(name, "viewtype") && attr->type == json_integer)
        {
            v->set_viewtype((PKT::ElementDat::View::ViewType)attr->u.integer);
        }

        if (!strcmp(name, "cameraids") && attr->type == json_array)
        {
            sublen = attr->u.array.length;
            for (j = 0; j<sublen; ++j)
            {
                subattr = attr->u.array.values[j];
                if (subattr->type == json_string)
                {
                    TvWall::Guid *camid = v->add_cameraid();
                    camid->set_data(subattr->u.string.ptr);
                }
            }
        }

        if (!strcmp(name, "starttime") && attr->type == json_integer)
        {
            v->set_starttime(attr->u.integer);
        }

        if (!strcmp(name, "rect") && attr->type == json_object)
        {
            sublen = attr->u.object.length;
            TvWall::Rect *rect = v->mutable_rect();

            for (j = 0; j<sublen; ++j)
            {
                subname = attr->u.object.values[j].name;
                subattr = attr->u.object.values[j].value;

                if (!strcmp(subname, "x") && subattr->type == json_integer)
                    rect->set_x(subattr->u.integer);
                if (!strcmp(subname, "y") && subattr->type == json_integer)
                    rect->set_y(subattr->u.integer);
                if (!strcmp(subname, "w") && subattr->type == json_integer)
                    rect->set_w(subattr->u.integer);
                if (!strcmp(subname, "h") && subattr->type == json_integer)
                    rect->set_h(subattr->u.integer);
            }
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseViewVideo(HttpRequest *request, PKT_VIEW_CTRL::VideoCtrl& vc)
{
    json_char *json;
    json_value *value, *attr, *subattr;
    int i, j, len, sublen;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_view_video, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view video data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"cameraids" : [],
     *	"pollinterval" : 2, 
     *	"keepstretch" : true,
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "winid") && attr->type == json_integer)
        {
            vc.set_winid(attr->u.integer);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            vc.set_type((PKT_VIEW_CTRL::VideoCtrl::VideoCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "pollinterval") && attr->type == json_integer)
        {
            vc.set_pollinterval(attr->u.integer);
        }

        if (!strcmp(name, "keepstretch") && attr->type == json_boolean)
        {
            vc.set_keepstretch(attr->u.integer);
        }

        if (!strcmp(name, "cameraids") && attr->type == json_array)
        {
            sublen = attr->u.array.length;
            for (j = 0; j<sublen; ++j)
            {
                subattr = attr->u.array.values[j];
                if (subattr->type == json_string)
                {
                    TvWall::Guid *camid = vc.add_cameraid();
                    camid->set_data(subattr->u.string.ptr);
                }
            }
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseViewRecord(HttpRequest *request, PKT_VIEW_CTRL::RecordCtrl& rc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_view_record, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view record data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"cameraids" : [],
     *	"starttime" : 2, 
     *	"rate" : 2, 
     *	"keepstretch" : true,
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "winid") && attr->type == json_integer)
        {
            rc.set_winid(attr->u.integer);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            rc.set_type((PKT_VIEW_CTRL::RecordCtrl::RecordCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "rate") && attr->type == json_integer)
        {
            rc.set_rate(attr->u.integer);
        }

        if (!strcmp(name, "starttime") && attr->type == json_integer)
        {
            rc.set_starttime(attr->u.integer*1000); //convert to millie seconds
        }

        if (!strcmp(name, "keepstretch") && attr->type == json_boolean)
        {
            rc.set_keepstretch(attr->u.integer);
        }

        if (!strcmp(name, "cameraid") && attr->type == json_string)
        {
            rc.mutable_cameraid()->set_data(attr->u.string.ptr);
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseViewFlash(HttpRequest *request, PKT_VIEW_CTRL::FlashCtrl& fc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_view_flash, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view flash data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"url" : "",
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "winid") && attr->type == json_integer)
        {
            fc.set_winid(attr->u.integer);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            fc.set_type((PKT_VIEW_CTRL::FlashCtrl::FlashCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "url") && attr->type == json_string)
        {
            fc.set_url(attr->u.string.ptr);
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseViewHtml(HttpRequest *request, PKT_VIEW_CTRL::HTMLCtrl& hc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_view_html, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view html data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"url" : "",
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "winid") && attr->type == json_integer)
        {
            hc.set_winid(attr->u.integer);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            hc.set_type((PKT_VIEW_CTRL::HTMLCtrl::HTMLCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "url") && attr->type == json_string)
        {
            hc.set_url(attr->u.string.ptr);
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseViewText(HttpRequest *request, PKT_VIEW_CTRL::TextCtrl& tc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_view_text, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view text data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"url" : "",
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "winid") && attr->type == json_integer)
        {
            tc.set_winid(attr->u.integer);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            tc.set_type((PKT_VIEW_CTRL::TextCtrl::TextCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "text") && attr->type == json_string)
        {
            tc.set_text(attr->u.string.ptr);
        }
    }
    json_value_free(value);
    return r;

}

http_ret_t SessionDataParser::ParseViewLocalMovie(HttpRequest *request, PKT_VIEW_CTRL::LocalMovieCtrl& lmc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_view_map, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view map data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"url" : "",
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "winid") && attr->type == json_integer)
        {
            lmc.set_winid(attr->u.integer);
        }

        if (!strcmp(name, "volume") && attr->type == json_integer)
        {
            lmc.set_volume(attr->u.integer);
        }

        if (!strcmp(name, "timestamp") && attr->type == json_integer)
        {
            lmc.set_timestamp(attr->u.integer);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            lmc.set_type((PKT_VIEW_CTRL::LocalMovieCtrl::LocalMovieType)attr->u.integer);
        }

        if (!strcmp(name, "url") && attr->type == json_string)
        {
            lmc.set_url(attr->u.string.ptr);
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseViewMap(HttpRequest *request, PKT_VIEW_CTRL::MapCtrl& mc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_view_map, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view map data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"url" : "",
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "winid") && attr->type == json_integer)
        {
            mc.set_winid(attr->u.integer);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            mc.set_type((PKT_VIEW_CTRL::MapCtrl::MapCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "url") && attr->type == json_string)
        {
            mc.set_url(attr->u.string.ptr);
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseCameraCtrl(HttpRequest *request, PKT_CTRL::CameraCtrl& cc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_camera, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse camera data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"url" : "",
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            cc.set_type((PKT_CTRL::CameraCtrl::PTZ_TYPE)attr->u.integer);
        }

        if (!strcmp(name, "cameraid") && attr->type == json_string)
        {
            cc.mutable_cameraid()->set_data(attr->u.string.ptr);
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseLayoutCtrl(HttpRequest *request, PKT_CTRL::LayoutCtrl& lc)
{
    json_char *json;
    json_value *value, *attr;
    int i, len;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_layout, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse view map data error";
        return r;
    }


    /*
     * {
     *  "type" : 1,
     *	"winid" : 1,
     *	"url" : "",
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "name") && attr->type == json_string)
        {
            lc.set_lyoutname(attr->u.string.ptr);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            lc.set_type((PKT_CTRL::LayoutCtrl::LayoutCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "isalarm") && attr->type == json_boolean)
        {
            lc.set_isalarm(attr->u.integer);
        }

        if (!strcmp(name, "alarminterval") && attr->type == json_integer)
        {
            lc.set_alarminterval(attr->u.integer);
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseLayoutLoopCtrl(HttpRequest *request, PKT_CTRL::LayoutLoopCtrl& llc)
{
    json_char *json;
    json_value *value, *attr, *subattr;
    int i, j, len, sublen;
    char *name;

    http_ret_t r = {E_SUCCESS, ra_ctl_layout_loop, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse layout loop data error";
        return r;
    }

    /*
     * {
     *  "layoutloopname" : 1,
     *	"layoutname" : 1,
     *	"loop" : "",
     *	"type" : 0 
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "layoutloopname") && attr->type == json_string)
        {
            llc.set_lyoutloopname(attr->u.string.ptr);
        }

        if (!strcmp(name, "layoutname") && attr->type == json_array)
        {
            sublen = attr->u.array.length;
            for (j = 0; j<sublen; ++j)
            {
                subattr = attr->u.array.values[j];
                if (subattr->type == json_string)
                {
                    llc.add_layoutname(subattr->u.string.ptr);
                }
            }
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            llc.set_type((PKT_CTRL::LayoutLoopCtrl::LayoutLoopCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "loop") && attr->type == json_integer)
        {
            llc.set_loop(attr->u.integer);
        }
    }
    json_value_free(value);
    return r;
}

http_ret_t SessionDataParser::ParseMachineCtrl(HttpRequest *request, PKT_CTRL::MachineCtrl& mc)
{
    json_char *json;
    json_value *value, *attr, *subattr;
    int i, j, len, sublen;
    char *name, *subname;

    http_ret_t r = {E_SUCCESS, ra_ctl_machine, ""};

    len = request->GetContentLength();
    json = (json_char*)request->GetPostData();
    value = json_parse(json, len);

    if (value == NULL)
    {
        r.code = E_POSTDATA;
        r.message = "Parse control machine data error";
        return r;
    }

    /*
     * {
     *  "type" : 1,
     *	"screenmode" : "",
     * }
     */

    len = value->u.object.length;
    for (i = 0; i<len; i++)
    {
        name = value->u.object.values[i].name;
        attr = value->u.object.values[i].value;

        if (!strcmp(name, "screenmode") && attr->type == json_string)
        {
            mc.set_screenmode(attr->u.string.ptr);
        }

        if (!strcmp(name, "type") && attr->type == json_integer)
        {
            mc.set_type((PKT_CTRL::MachineCtrl::MachineCtrlType)attr->u.integer);
        }

        if (!strcmp(name, "interface") && attr->type == json_object)
        {
            sublen = attr->u.object.length;
            TvWall::NetInterface *it = mc.mutable_netinterface();
            for (j = 0; j<sublen; ++j)
            {
                subname = attr->u.object.values[j].name;
                subattr = attr->u.object.values[j].value;

                if (!strcmp(subname, "name") && subattr->type == json_string)
                    it->set_ifname(subattr->u.string.ptr);

                if (!strcmp(subname, "ip") && subattr->type == json_string)
                    it->set_ip(subattr->u.string.ptr);

                if (!strcmp(subname, "mask") && subattr->type == json_string)
                    it->set_netmask(subattr->u.string.ptr);

                if (!strcmp(subname, "speed") && subattr->type == json_integer)
                    it->set_speed(subattr->u.integer);

                if (!strcmp(subname, "dhcp") && subattr->type == json_boolean)
                    it->set_dhcp(subattr->u.integer);
            }
        }

        if (!strcmp(name, "host") && attr->type == json_object)
        {
            sublen = attr->u.object.length;
            TvWall::Network *net = mc.mutable_network();
            for (j = 0; j<sublen; ++j)
            {
                subname = attr->u.object.values[j].name;
                subattr = attr->u.object.values[j].value;

                if (!strcmp(subname, "name") && subattr->type == json_string)
                    net->set_hostname(subattr->u.string.ptr);

                if (!strcmp(subname, "nat") && subattr->type == json_string)
                    net->set_nat(subattr->u.string.ptr);

                if (!strcmp(subname, "dns") && subattr->type == json_string)
                    net->set_dns(subattr->u.string.ptr);
            }
        }

        if (!strcmp(name, "gateway") && attr->type == json_object)
        {
            sublen = attr->u.object.length;
            TvWall::Network *net = mc.mutable_network();
            for (j = 0; j<sublen; ++j)
            {
                subname = attr->u.object.values[j].name;
                subattr = attr->u.object.values[j].value;

                if (!strcmp(subname, "name") && subattr->type == json_string)
                    net->set_hostname(subattr->u.string.ptr);

                if (!strcmp(subname, "gateway") && subattr->type == json_string)
                    net->set_gateway(subattr->u.string.ptr);
                
                if (!strcmp(subname, "gateway6") && subattr->type == json_string)
                    net->set_gateway6(subattr->u.string.ptr);

                if (!strcmp(subname, "gwdev") && subattr->type == json_string)
                    net->set_gwdev(subattr->u.string.ptr);
            }
        }

        if (!strcmp(name, "nvr") && attr->type == json_object)
        {
            sublen = attr->u.object.length;
            TvWall::NVR *nvr = mc.mutable_nvr();
            for (j = 0; j<sublen; ++j)
            {
                subname = attr->u.object.values[j].name;
                subattr = attr->u.object.values[j].value;

                if (!strcmp(subname, "ip") && subattr->type == json_string)
                    nvr->set_ip(subattr->u.string.ptr);
                
                if (!strcmp(subname, "username") && subattr->type == json_string)
                    nvr->set_username(subattr->u.string.ptr);

                if (!strcmp(subname, "password") && subattr->type == json_string)
                    nvr->set_password(subattr->u.string.ptr);
            }
        }
        if (!strcmp(name, "fmp") && attr->type == json_object)
        {
            sublen = attr->u.object.length;
            TvWall::FMP *fmp = mc.mutable_fmp();
            for (j = 0; j<sublen; ++j)
            {
                subname = attr->u.object.values[j].name;
                subattr = attr->u.object.values[j].value;

                if (!strcmp(subname, "ip") && subattr->type == json_string)
                    fmp->set_ip(subattr->u.string.ptr);
                
                if (!strcmp(subname, "username") && subattr->type == json_string)
                    fmp->set_username(subattr->u.string.ptr);

                if (!strcmp(subname, "password") && subattr->type == json_string)
                    fmp->set_password(subattr->u.string.ptr);
            }
        }
        if (!strcmp(name, "tvwallindex") && attr->type == json_integer)
        {
            mc.set_tvwallindex(attr->u.integer);
        }
    }
    json_value_free(value);
    return r;
}
