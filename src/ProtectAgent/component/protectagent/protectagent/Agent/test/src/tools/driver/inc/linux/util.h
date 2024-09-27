/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : util.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 * Version     : 1.0
 *
 * Description : socket�������շ�����������
 *               ��д�����ļ���������,
 *               ��־��ӡ�궨��
 *
 */
#ifndef _IOMIRROR_UTIL_H_
#define _IOMIRROR_UTIL_H_

#include <net/sock.h>
#include <linux/net.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include "cmd_define.h"
#include "const.h"
#include "../share/kernel_alarm.h"

#define DPP_ACK_ERROR  (1)
#define DPP_ACK_TOKEN_ID_ERROR (1)

// The 15th bit is error flag
#define DPP_ERROR_FLAG_OFFSET  (15)
#define GET_DPP_ACK_ERROR_FLAG(flags) ((flags >> DPP_ERROR_FLAG_OFFSET) & 1U)

// [6, 13] bits is error reason
#define DPP_ACK_ERROR_TYPE_OFFSET  (6)
#define GET_DPP_ACK_ERROR_TYPE(flags)  ((flags >> DPP_ACK_ERROR_TYPE_OFFSET) & (0x00ffU))

#define MIN(x,y)    (((x) < (y)) ? (x) : (y))
#define IP_STRING_LEN 16
#define IOMIRROR_MEMCMP(ptr1, ptr2, len)  memcmp(ptr1, ptr2, len)

int im_socket_create(int family, int type, int protocol, struct socket **sock);
void im_socket_close(struct socket *sock);
int im_socket_bind(struct socket *sock, struct sockaddr *address, int addrlen);
int im_socket_listen(struct socket *sock, int backlog);
int im_socket_accept(struct socket *sock, struct socket **newsock, int flags);
int im_socket_connect(struct socket *sock, struct sockaddr *address, int addrlen, int flags);
int im_getsockname(struct socket *sock, struct sockaddr *addr, int *addrlen);
int im_getpeername(struct socket *sock, struct sockaddr *addr, int *addrlen);
int im_cmd_send(struct socket *sock, struct im_cmd *cmd);
int im_cmd_recv(struct socket *sock, struct im_cmd *cmd);
struct im_config_pg *im_config_read(void);
void parse_ip_str(uint32_t ipInt, char *ipStr, uint32_t ipStrlen);
void change_to_dpp_data(struct im_cmd_header *cmd_header, DPP_HEADER *header, void *data);
int im_sock_xmit(struct socket *sock, int send, void *buf, size_t size, int flag, int retry);
uint32_t make_log_lower_bound(uint64_t value);
uint32_t make_log_upper_bound(uint64_t value);
uint64_t make_power_2_value(uint64_t x);
void im_init_alarm_item(PALARM_ITEM p_alarm_item);
void im_init_alarm_list(PALARM_LIST p_alarm_list);
int im_get_alarms(PALARM_LIST p_alarm_list, PALARM_ITEM p_item_array, int items_size);
void im_add_alarm_list(PALARM_LIST p_alarm_list, PALARM_ITEM alarm_item);
PALARM_ITEM im_create_and_init_alarm_item(void);
void im_pg_free_alarms(PALARM_LIST p_alarm_list);
bool im_is_token_valid(const char* token, const int token_len);

/**
 * KERN_EMERG   "<0>" �����¼���Ϣ��ϵͳ����֮ǰ��ʾ����ʾϵͳ������
 * KERN_ALERT   "<1>" ������Ϣ����ʾ����������ȡ��ʩ
 * KERN_CRIT    "<2>" �ٽ�������ͨ���漰���ص�Ӳ�����������ʧ��
 * KERN_ERR     "<3>" ��������������������KERN_ERR������Ӳ���Ĵ���
 * KERN_WARNING "<4>" �����������Կ��ܳ��������������о���
 * KERN_NOTICE  "<5>" ����������Ҫ���������������ѡ��������밲ȫ��ص���Ϣ
 * KERN_INFO    "<6>" ��ʾ��Ϣ����������������ʱ����ӡӲ����Ϣ
 * KERN_DEBUG   "<7>" ���Լ������Ϣ
 */
#define IOMIRROR_ALRT(f, arg...)  printk(KERN_ALERT   "[IOMirror][%-33s:%4d]" f "\n", __FILE__, __LINE__, ## arg)
#define IOMIRROR_CRIT(f, arg...)  printk(KERN_CRIT    "[IOMirror][%-33s:%4d]" f "\n", __FILE__, __LINE__, ## arg)
#define IOMIRROR_ERR(f, arg...)   printk(KERN_ERR     "[IOMirror][ERROR][%-33s:%4d]" f "\n", __FILE__, __LINE__, ## arg)
#define IOMIRROR_WARN(f, arg...)  printk(KERN_WARNING "[IOMirror][WARN ][%-33s:%4d]" f "\n", __FILE__, __LINE__, ## arg)
#define IOMIRROR_INFO(f, arg...)  printk(KERN_INFO    "[IOMirror][INFO ][%-33s:%4d]" f "\n", __FILE__, __LINE__, ## arg)
#define IOMIRROR_DBG(f, arg...)   printk(KERN_DEBUG   "[IOMirror][DEBUG][%-33s:%4d]" f "\n", __FILE__, __LINE__, ## arg)


//default: 5s printk at most 10 logs
#define IOMIRROR_INFO_RATE_LIMITED(f, arg...)                                                    \
{                                                                                                \
    if (printk_ratelimit())                                                                      \
    {                                                                                            \
         printk(KERN_INFO     "[IOMirror][INFO][%-33s:%4d]" f "\n", __FILE__, __LINE__, ## arg); \
    }                                                                                            \
}

#define IOMIRROR_RATELIMIT_INTERVAL (1 * HZ)
#define IOMIRROR_RATELIMIT_BURST    1

/**
 * Description : ���ٴ�ӡ��־��ֻ֧��3.0 ��֮����ں˰汾���٣�֮ǰ�İ汾�޷�����
 *  
 * Author      : z00455045
 * Date        : 2020/2/13
 */                                               
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))

#define iomirror_printk_ratelimited(fmt, ...)	\
({									            \
	static DEFINE_RATELIMIT_STATE(rs, IOMIRROR_RATELIMIT_INTERVAL, IOMIRROR_RATELIMIT_BURST);   \
	if (__ratelimit(&rs))						\
		printk(fmt, ##__VA_ARGS__);				\
})

#define IOMIRROR_INFO_PRINT_LIMITED(fmt, ...)  \
{ \
    iomirror_printk_ratelimited(KERN_INFO "[IOMirror][INFO ][%-33s:%4d]" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
}
#else                                              
#define IOMIRROR_INFO_PRINT_LIMITED(fmt, arg...) \
{\
    printk(KERN_INFO "[IOMirror][INFO ][%-33s:%4d]" fmt "\n", __FILE__, __LINE__, ## arg); \
}
#endif                                             

#endif

