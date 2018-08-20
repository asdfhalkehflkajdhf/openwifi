#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <errno.h>

#include "cloud_wlan_list.h"
#include "cloud_wlan_types.h"
#include "cloud_wlan_nl.h"
#include "cloud_wlan_ac_com.h"
#include "cloud_wlan_recv_info_branch.h"
#include "cloud_wlan_ap_info_list.h"

static pthread_attr_t attr;
static pthread_t g_ap_age_pt;

u32 g_ap_auth_mode = AP_OPEN_AUTH;
static pthread_mutex_t mutex;

void *cw_pthread_hear_beat_dispose(void *data)
{
	struct sockaddr client_info;
	ap_local_info_t ap_info;
	dcma_pthread_info_t *pthread_para = (dcma_pthread_info_t *)data;

	memcpy((void *)&client_info, (void *)pthread_para->client_info, sizeof(struct sockaddr));
	memcpy((void *)&ap_info, pthread_para->data, sizeof(ap_local_info_t));

	//����ap��Ϣ��û�����¼�һ��
	//�鿴�Ƿ��з�����Ϣ
	
	pthread_mutex_lock(&mutex);
	if(g_ap_auth_mode == AP_OPEN_AUTH)
	{
		cw_update_ap_info_list(&client_info, ap_info);
	}
	else
	{
		cw_admin_update_ap_info_list(&client_info, ap_info);
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}
void *cw_pthread_online_user_dispose(void *data)
{
	struct sockaddr client_info;
	online_user_info_t user_info;
	dcma_pthread_info_t *pthread_para = (dcma_pthread_info_t *)data;

	memcpy((void *)&client_info, (void *)pthread_para->client_info, sizeof(struct sockaddr));
	memcpy((void *)&user_info, pthread_para->data, sizeof(online_user_info_t));
	//���������û���Ϣ��ap�����ڣ�ʲô��������ap���ڣ�û������һ��
	
	pthread_mutex_lock(&mutex);
	if(g_ap_auth_mode == AP_OPEN_AUTH)
	{
		cw_update_user_info_list(&client_info, user_info);
	}
	else
	{
		cw_admin_update_user_info_list(&client_info, user_info);
	}
	
	pthread_mutex_unlock(&mutex);
	return NULL;
}

void *cw_pthread_web_set_cfg_dispose(void *data)
{
	struct sockaddr client_info;
	cwlan_cmd_sockt_t *cmd;
	dcma_pthread_info_t *pthread_para = (dcma_pthread_info_t *)data;
	memcpy((void *)&client_info, (void *)pthread_para->client_info, sizeof(struct sockaddr));

	cmd = (cwlan_cmd_sockt_t *)pthread_para->data;

	//��ap�������õ�buff

	pthread_mutex_lock(&mutex);
	if(g_ap_auth_mode == AP_OPEN_AUTH)
	{
		cw_update_web_set_cmd_list(&client_info, cmd);
	}
	else
	{
		cw_admin_update_web_set_cmd_list(&client_info, cmd);
	}

	pthread_mutex_unlock(&mutex);
	return NULL;
}
void *cw_pthread_web_get_cfg_dispose(void *data)
{
	//��ap��Ϣ����web
	
	struct sockaddr client_info;
	u8 apmac[6];
	
	dcma_pthread_info_t *pthread_para = (dcma_pthread_info_t *)data;
	memcpy((void *)&client_info, (void *)pthread_para->client_info, sizeof(struct sockaddr));
	memcpy((void *)&apmac, pthread_para->data, 6);

	//��ap�����͵�web
	
	pthread_mutex_lock(&mutex);
	if(g_ap_auth_mode == AP_OPEN_AUTH)
	{
		cw_web_get_ap_info_list(&client_info, apmac);
	}
	else
	{
		cw_admin_web_get_ap_info_list(&client_info, apmac);
	}
	
	pthread_mutex_unlock(&mutex);
	return NULL;
}
void *cw_pthread_web_add_ap_node_dispose(void *data)
{
	//����ap ���ظ��������ݿ�ȥ����
	
	struct sockaddr client_info;
	u8 apmac[6];
	
	dcma_pthread_info_t *pthread_para = (dcma_pthread_info_t *)data;
	memcpy((void *)&client_info, (void *)pthread_para->client_info, sizeof(struct sockaddr));
	memcpy((void *)&apmac, pthread_para->data, 6);

	pthread_mutex_lock(&mutex);
	cw_admin_web_add_ap_node(&client_info, apmac);
	pthread_mutex_unlock(&mutex);
	return NULL;
}
void *cw_pthread_web_del_ap_node_dispose(void *data)
{
	//ɾ��ap ���ظ��������ݿ�ȥ����
	
	struct sockaddr client_info;
	u8 apmac[6];
	
	dcma_pthread_info_t *pthread_para = (dcma_pthread_info_t *)data;
	memcpy((void *)&client_info, (void *)pthread_para->client_info, sizeof(struct sockaddr));
	memcpy((void *)&apmac, pthread_para->data, 6);

	pthread_mutex_lock(&mutex);
	cw_admin_web_del_ap_node(&client_info, apmac);
	pthread_mutex_unlock(&mutex);
	return NULL;
}
void *cw_pthread_online_ap_age_del_dispose(void *data)
{
	u32 key = 0;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	/*����ΪϵͳĬ�Ϸ�ʽ*/
	//pthread_setcanceltype(PTHREAD_CANCEL_DEFFERED, NULL);

	while(1)
	{
		key = key % CWLAN_AP_INFO_HASH_LEN_SECTION;
		pthread_testcancel();
		pthread_mutex_lock(&mutex);
		if(g_ap_auth_mode == AP_OPEN_AUTH)
		{
			cw_update_ap_info_list_age_del(key);
		}
		else
		{
			cw_admin_update_ap_info_list_age_del(key);
		}
		
		pthread_mutex_unlock(&mutex);
		key++;
		
		pthread_testcancel();
		sleep(2);
	}
	return NULL;
}
u32 cw_ac_auth_switch_to_admin(dcma_pthread_info_t *pthread_para)
{
	if(g_ap_auth_mode == AP_ADMIN_AUTH)
	{
		return CWLAN_OK;
	}
	
	/*��Ϊ��������תǿ������Ҫɾ�����н��
	��������ap*/
	pthread_mutex_lock(&mutex);
	g_ap_auth_mode = AP_ADMIN_AUTH;
	cw_web_del_all_ap_info_list_node();
	pthread_mutex_unlock(&mutex);
	
	cw_sendto_info(pthread_para->client_info, CW_NLMSG_RES_OK, sizeof(u32));
	return CWLAN_OK;
}
u32 cw_ac_auth_switch_to_open(dcma_pthread_info_t *pthread_para)
{
	if(g_ap_auth_mode == AP_OPEN_AUTH)
	{
		return CWLAN_OK;
	}
	/*��Ϊ��ǿ����ת����������Ҫɾ�����н��*/
	
	pthread_mutex_lock(&mutex);
	g_ap_auth_mode = AP_OPEN_AUTH;
	pthread_mutex_unlock(&mutex);
	
	cw_sendto_info(pthread_para->client_info, CW_NLMSG_RES_OK, sizeof(u32));
	return CWLAN_OK;
}

/*
���౨��:
1, ap��������
2,ap���߸���
3,web��ȡap info
4,web�·�����
*/

void cw_recv_info_branch(dcma_udp_skb_info_t *buff, u32 len, struct sockaddr *client_info)
{
	s32 ret = 0;
	pthread_t pt = 0;
	dcma_pthread_info_t pthread_para;
	pthread_para.client_info = client_info;
	pthread_para.data = buff->data;
	
	//printf("buff->type = %x, len = %d\n", buff->type, len);
	switch(buff->type)	
	{
		case CW_NLMSG_HEART_BEAT:
			ret = pthread_create(&pt, &attr, cw_pthread_hear_beat_dispose, (void *)&pthread_para);	 
			break;
		case CW_NLMSG_PUT_ONLINE_INFO_TO_AC:	
			ret = pthread_create(&pt, &attr, cw_pthread_online_user_dispose, (void *)&pthread_para);	 
			break; 
		case CW_NLMSG_WEB_SET_AP_CONFIG:
			ret = pthread_create(&pt, &attr, cw_pthread_web_set_cfg_dispose, (void *)&pthread_para);	 
			break;
		case CW_NLMSG_WEB_GET_AP_INFO: 
			ret = pthread_create(&pt, &attr, cw_pthread_web_get_cfg_dispose, (void *)&pthread_para);	 
			break;
		case CW_NLMSG_WEB_ADD_AP_NODE:
			ret = pthread_create(&pt, &attr, cw_pthread_web_add_ap_node_dispose, (void *)&pthread_para);	 
			break;
		case CW_NLMSG_WEB_DEL_AP_NODE:
			ret = pthread_create(&pt, &attr, cw_pthread_web_del_ap_node_dispose, (void *)&pthread_para);	 
			break;


		case CW_NLMSG_WEB_AP_OPEN_AUTH:
			cw_ac_auth_switch_to_open(&pthread_para);
			break;
		case CW_NLMSG_WEB_AP_ADMIN_AUTH:
			cw_ac_auth_switch_to_admin(&pthread_para);
			break;
		default:
			printf("[unkown cmd]:%d\n", buff->type);
			break;
	} 
	if(0 != ret  )
	{
		printf("pthread_create [%d] fail\n", buff->type);
	}
	return;  
}
u32 cw_recv_info_dispose_pthread_cfg_init()
{
	u32 ret = 0;
	//struct sched_param  params;
	
	ret += pthread_attr_init(&attr);
	ret += pthread_mutex_init(&mutex,NULL);
	//ret += pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	//�߳��˳�ֱ���ͷ���Դ
	ret += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	//ret += pthread_attr_setschedparam(&attr, &params);
	/*��������ap �ϻ��߳�*/
	ret += pthread_create(&g_ap_age_pt, NULL, cw_pthread_online_ap_age_del_dispose, NULL);	 


	return ret;
}
u32 cw_recv_info_dispose_pthread_cfg_exit()
{
	u32 ret = 0;
	//struct sched_param  params;
	/*kill ����ap �ϻ��߳�*/
	while( !pthread_cancel(g_ap_age_pt))
	{};
	printf("online_ap_age_del_dispose kill ok\n");


	/*ע����*/
	ret += pthread_mutex_destroy(&mutex);
	if( 0 != ret)
	{
		printf("pthread_mutex_destroy fail\n");
	}



	return ret;
}
