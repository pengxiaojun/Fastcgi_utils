#ifndef _TVW_TYPES
#define _TVW_TYPES


#include <sysinc.hpp>

#ifdef __cplusplus
extern "C" {
#endif


#define E_SUCCESS       0
#define E_CONNDOWN      1
#define E_LINKDOWN      2
#define E_NOSESSION     3
#define E_NOLOGIN       4
#define E_NOVIEW        5
#define E_POSTDATA      6


typedef struct http_ret_t{
    int code;
    int action;
    std::string message;
}http_ret_t;

typedef enum {
    ra_unknow,
    ra_login,
    ra_login_ack,
    ra_query_data,

    //start data acknowledge
    ra_basic_ack,
    ra_camera_ack,
    ra_localmovie_ack,
    ra_flash_ack,
    ra_layout_ack,
    ra_hotpoint_ack,
    ra_map_ack,
    ra_layout_loop_ack,
    ra_screen_ack,
    ra_view_ack,
    ra_nvr_ack,
    ra_network_ack,
    //end data acknowledge

    //logout
    ra_logout,
    //query network
    ra_network,
    //query fmp ack
    ra_fmp_ack,

    //start notice
    ra_screen_stat = 0x20,
    ra_view_geo_stat,
    ra_view_video_stat,
    ra_view_fhlm_stat,
    ra_layout_stat,
    ra_layout_loop_stat,
    ra_camera_stat,
    ra_hotpoint_stat,
    ra_network_stat,
    ra_record_stat,
    //end notice

    //start control
    ra_ctl_screen_split = 0x40,
    ra_ctl_view_geo,
    ra_ctl_view_video,
    ra_ctl_view_record,
    ra_ctl_view_flash,
    ra_ctl_view_html,
    ra_ctl_view_text,
    ra_ctl_view_localmovie,
    ra_ctl_view_map,
    ra_ctl_camera,
    ra_ctl_hotpoint,
    ra_ctl_layout,
    ra_ctl_layout_loop,
    ra_ctl_machine,
    ra_ctl_key,
    ra_ctl_keycmd,
    ra_ctl_alarm,
    ra_ctl_master,
    //end control
}req_action_t;

typedef enum{
    rt_unknow,
    rt_nvr,
    rt_tvw,
    rt_oth,
}req_target_t;

typedef enum{
    rm_unsupported,
    rm_get,
    rm_post,
    rm_put,
    rm_head,
}req_method_t;

typedef enum{
    rc_unsupported,
    rc_json,
    rc_html,
    rc_plain,
    rc_stream,
}resp_content_type_t;


#ifdef __cplusplus
}
#endif

#endif
