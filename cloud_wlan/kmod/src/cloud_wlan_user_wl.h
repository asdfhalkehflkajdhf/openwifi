#ifndef CLOUD_WLAN_UWER_WL_H_
#define CLOUD_WLAN_UWER_WL_H_

#include "cloud_wlan_nl_in_pub.h"

typedef struct user_white_list
{
	struct list_head list;
	u8 mac[6];
}user_white_list_t;



extern user_white_list_t g_user_white_list;


extern u32 cloud_wlan_user_white_list_init(void);
extern u32 cloud_wlan_user_wl_add(user_wl_buf_t *buff);
extern u32 cloud_wlan_user_wl_del(user_wl_buf_t *buff);
extern u32 cloud_wlan_user_wl_update(user_wl_buf_t *buff);
extern u32 cloud_wlan_user_white_list_find(u8 *mac);
extern u32 cloud_wlan_user_white_list_exit(void);


#endif

