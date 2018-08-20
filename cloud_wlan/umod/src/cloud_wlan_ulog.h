#ifndef CLOUD_WLAN_ULOG_H_
#define CLOUD_WLAN_ULOG_H_

#define CW_AP_ULOG_FILE "/etc/config/cw_ap_run.log"
#define CW_AP_ULOG_MAX 1024*1024

#define CW_AP_KLOG_URL_FILE "/etc/config/cw_ap_url.log"
#define CW_AP_KLOG_USER_FILE "/etc/config/cw_ap_user.log"
#define CW_AP_KLOG_MAX 1024*1024

extern u32 g_cw_debug;

extern u32 cw_local_log_writeto_file(s8 *str, ...);

#endif

