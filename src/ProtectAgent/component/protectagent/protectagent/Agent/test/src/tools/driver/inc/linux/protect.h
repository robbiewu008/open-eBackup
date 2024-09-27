/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : protect.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/18
 * Version     : 1.0
 *
 * Description : 保护组相关数据结构定义，启动、停止保护组线程接口声明
 *
 */
#ifndef _IOMIRROR_PROTECT_H
#define _IOMIRROR_PROTECT_H

#include <linux/types.h>
#include <linux/jiffies.h>
#include "filter.h"
#include "ctl_cmd.h"
#include "../share/kernel_alarm.h"

#define MAX_COMPARE(x,y)               ((x) > (y) ? (x) : (y))
#define CMD_HEADER_BODY_LEN            (sizeof(DPP_HEADER) + MAX_COMPARE(sizeof(DPP_DATASET_START), sizeof(DPP_DATA)))
#define MAX_NORMAL_SEND_BUFFER         (2*1024*1024 + CMD_HEADER_BODY_LEN)      //2M+ buffer
#define BITMAP_NOT_SET_COUNT 3

#define SAFE_TWO_JIFFIES_SUB(jiffies_new, jiffies_old, sub_jiffies)  {       \
    const uint32_t MAX_UINT32_VALUE = 0xffffffff;                            \
    if (time_after(jiffies_new, jiffies_old)) {                              \
        sub_jiffies = jiffies_new - jiffies_old;                             \
    } else {                                                                 \
        sub_jiffies = MAX_UINT32_VALUE - jiffies_old + jiffies_new;          \
    }                                                                        \
}

/* 保护组状态，用于对外呈现 */
enum im_pg_state
{
    IM_PG_STATE_NORMAL = 1,
    IM_PG_STATE_CBT,
    IM_PG_STATE_VERIFY,
    IM_PG_STATE_ATOMIC,    /* 用于CBT后、数据重发*/
};

/* 保护组内部状态，用于复制流程控制 */
enum im_pg_inter_state
{
    IM_PG_INTER_STATE_NORMAL = 0,

    IM_PG_INTER_STATE_PAUSE,

    IM_PG_INTER_STATE_CBT_START,
    IM_PG_INTER_STATE_CBT_START_P,
    IM_PG_INTER_STATE_CBT_END,
    IM_PG_INTER_STATE_CBT_END_P,
    IM_PG_INTER_STATE_CBT_DATA,

    IM_PG_INTER_STATE_ATOMIC_START,
    IM_PG_INTER_STATE_ATOMIC_START_P,
    IM_PG_INTER_STATE_ATOMIC_END,
    IM_PG_INTER_STATE_ATOMIC_END_P,
    IM_PG_INTER_STATE_ATOMIC_DATA,
    IM_PG_INTER_STATE_ATOMIC_DATA_P,

    IM_PG_INTER_STATE_VERIFY_START,
    IM_PG_INTER_STATE_VERIFY_START_P,
    IM_PG_INTER_STATE_VERIFY_END,
    IM_PG_INTER_STATE_VERIFY_END_P,
    IM_PG_INTER_STATE_VERIFY_DATA_END,
    IM_PG_INTER_STATE_VERIFY_DATA_END_P,
    IM_PG_INTER_STATE_VERIFY_BITMAP_P,
    IM_PG_INTER_STATE_VERIFY_DATA,

    IM_PG_INTER_STATE_CONNECT_STAGE0,  /* 建立socket连接，发送CONNECT_VRG报文 */
    IM_PG_INTER_STATE_CONNECT_STAGE0_P,
    IM_PG_INTER_STATE_CONNECT_STAGE1,  /* 发送IOMIRROR_START报文 */
    IM_PG_INTER_STATE_CONNECT_STAGE1_P,

    IM_PG_INTER_STATE_WAIT_PENDING_VOLS,/* 等待被保护卷ready */

    IM_PG_INTER_STATE_MAX,
};

/* 保护组线程启动类型 */
enum im_pg_start_type
{
    IM_PG_START_INIT = 0,    /* 初始同步方式，用于创建保护策略时 */
    IM_PG_START_NORMAL,      /* 正常方式，用于OS重启时 */
    IM_PG_START_VERIFY,      /* 一致性校验方式，用于回切时 */
#ifdef SUPPORT_BACKUP
    IM_PG_START_BACKUP,      /* 备份方式，用于备份状态 */
#endif
};

#ifdef SUPPORT_BACKUP
/* 备份模式状态 */
enum im_backup_state
{
    IM_NO_BACKUP_MODE = 0,
    IM_BACKUP_INIT,
    IM_BACKUP_CBT,
    IM_BACKUP_SNAPSHOT,
};
#endif

/* 保护组线程退出类型 */
enum im_pg_exit_type
{
    IM_PG_RUNNING = 0,
    IM_PG_EXIT_NORMAL,  /* 正常退出，关机、重启等 */
    IM_PG_EXIT_ABNORMAL,/* 异常退出，用户手动卸载模块、启动pg线程失败等 */
    IM_PG_EXIT_STOP,    /* 删除保护 */
    IM_PG_EXIT_PLAN,    /* 计划切换 */
    IM_PG_EXIT_DR,      /* 故障切换 */
};

/* 保护组切换类型 */
enum im_pg_migrate_type
{
    IM_PG_MIGRATE_PLAN = 1, /* 计划切换 */
    IM_PG_MIGRATE_DISASER,  /* 灾难切换 */
};

/* 待接收ack请求队列 */
struct im_pg_pending_queue
{
    struct list_head  head;
    struct list_head *atomic;
    unsigned long     atomic_recv_time;
    uint64_t          count;
    uint64_t          page_cnt;
};

/* 待接收ack的控制报文队列，用于控制报文重发 */
struct im_pg_cmd_pending_queue
{
    struct list_head  head;
    uint64_t          count;
    uint64_t          request_id;
    unsigned long     send_time;       /* 队列中报文最后一次重发时间 */
};

/* 外部命令类型 */
enum im_pg_ext_cmd_type
{
    IM_PG_EXT_CMD_ADD_VOL = 1,  /* 动态添加卷 */
    IM_PG_EXT_CMD_DEL_VOL,      /* 动态删除卷 */
    IM_PG_EXT_CMD_VOL_READY,
    IM_PG_EXT_CMD_PAUSE,
    IM_PG_EXT_CMD_RESUME,
    IM_PG_EXT_CMD_MODIFY,
    IM_PG_EXT_CMD_MOD_VOL,      /* 动态修改卷 */
    IM_PG_EXT_CMD_STOP_SEND_DATA,
    IM_PG_EXT_CMD_GET_KERNEL_ALARM,
    IM_PG_EXT_CMD_SET_TOKEN_ID,
};

enum im_pg_limit_kernel_alarm_report_rate {
    LIMIT_ALARM_BITMAP_NOT_READY = 0,          /* normal reboot but driver don't get bitmap from HA */
    LIMIT_ALARM_RECV_FAILED_ACK_FORM_OMA,      /* driver has been received failed ack from oma */
    LIMIT_ALARM_BUTTOM,
};

/* 保护组线程异步处理的外部命令 */
struct im_pg_external_cmd
{
    enum im_pg_ext_cmd_type type;
    struct completion comp;
    void *info;
    int8_t ret;
};

/* 待准备就绪的被保护卷 */
struct im_pg_pending_vol
{
    char        id[VOL_ID_LEN];
    char        path[DISK_PATH_LEN];
    uint64_t    sectors;
    struct list_head list;
};

struct im_pg_jiffies {
    volatile unsigned long rpo_time;  
    unsigned long iops_time;
    unsigned long data_trans_time;
    unsigned long sync_bitmap_data_time;
};

/* 保护组结构体 */
struct im_pg
{
    char        id[VM_ID_LEN];
    char        host_id[VM_ID_LEN];
    uint32_t    vrg_ip;                 /* VRG IP地址 */
    uint32_t    vrg_port;               /* VRG 端口 */
    uint32_t    exp_rpo;                /* RPO，单位秒 */
    uint8_t     bitmap_granularity;     /* 位表粒度，块大小=512*2^g */

    volatile uint8_t     is_init;       /* 是否完成初始同步 */
    uint64_t    total_credit;           /* 用于流控，单位字节 */
    uint64_t    sent_amount;            /* 用于流控，单位字节 */
    uint64_t    max_dataset_size;       /* 用于流控，单位字节 */
    uint64_t    next_vol_offset;        /* 用于流控，单位字节 */

    volatile uint8_t flow_control_pause_flag;/* 用于流控，暂停发送IO数据到OMA*/ 
    volatile uint8_t pause_pending;          /* pause接口临时状态，需要发送一个dataset之后才能进入pause状态*/
    volatile uint8_t resume_pending;         
    volatile uint8_t stop_send_data;        /*when normal shutdown, agent send ioctl to stop driver send data to soma*/
    uint8_t  reboot_pause_state;        /* After reboot, driver may enter pause state*/

    uint64_t    cbt_flush_times;        /* 本次启动以来，发送的dataset ID */
    uint64_t    cbt_flush_times_done;   /* 本次启动以来，收到的最后一个dataset ID done*/
    uint64_t    start_times;            /* 保护组创建后启动次数 */
    uint64_t    verify_times;           /* 本次启动以来，发起一致性校验次数 */
    uint32_t    connect_times;          /* 与VRG成功重连次数 */
    uint8_t     vol_num;
    uint64_t    request_id;             /* 当前normal/atomic报文的request id */
    uint64_t    queuePageCnt;           /* normal 状态下支持发送完64M数据再pause*/

    unsigned long hb_send_time; /* 最近一次发送心跳报文时间 */
    unsigned long hb_recv_time; /* 最近一次收到心跳报文时间 */

    unsigned long verify_send_time; /* 最近一次发送verify check报文时间 */
    unsigned long verify_recv_time; /* 最近一次收到verify check报文时间 */

    unsigned long rpo_send_time; /* 最近一次dataset报文时间 */
    
    char              snap_id[IM_SNAP_ID_LEN];  /* 备份GUID，用于验证备份链有效性 */
    volatile uint8_t  backup_status;      		/* 备份模式状态，0：非备份模式，1：备份初始状态，2：备份CBT状态，3：备份快照状态 */
    uint8_t           cur_volume_minor;			/* 当前最大的volume minor号 */
    dev_t             dev_savebitmap;           /* 保存persistent bitmap的磁盘设备信息*/
    struct im_ctl_bitmap_extent *bitmap_extent; /* 保存persistent bitmap的磁盘偏移数据*/

    dev_t             dev_saveconfigfile;           /* 保存config file的磁盘设备信息*/
    struct im_ctl_bitmap_extent *configfile_extent; /* 保存config file的磁盘偏移数据*/
       
    // statistics info variable 
    struct im_pg_jiffies statistics_jiffies;
    volatile uint64_t write_io_size;
    volatile uint64_t real_time_write_io;
    uint64_t write_iops;         
    uint64_t write_throughout;   // unit : KB/s
    uint32_t data_send_speed;    // unit : KB/s
    uint64_t data_send_size;     // unit : Bytes
    uint64_t verify_bitmap_total_bytes;
    uint64_t verify_bitmap_send_sum;
    uint64_t cbt_bitmap_total_bytes;
    uint64_t cbt_bitmap_send_sum;

    int work_state;
    int work_mode;
    int link_state;

    SetTokenID token;

    ALARM_LIST  alarm_list;
    unsigned long limit_alarm_report_rate[LIMIT_ALARM_BUTTOM];
    // pause some time send data when receiving failed ACK
    volatile uint64_t failed_ack_count;
    // delay send data variables
    unsigned long end_delay_time;
    uint64_t delay_time_count;
    uint64_t last_failed_ack_count; 

    struct list_head vols;
    struct list_head pending_vols;
    struct socket *sock;
    struct im_reqeust_queue rq;    /* request queue, 等待发送 */
    struct im_pg_pending_queue pq; /* pending queue, 已发送，等待接收ack */
    struct im_pg_cmd_pending_queue cmd_pq;  /* 待重发控制报文队列 */
    struct im_pg_external_cmd *ext_cmd;     /* 等待处理的外部命令 */
    enum im_pg_state state;
    enum im_pg_inter_state inter_state;
    enum im_pg_inter_state temp_state;      /* 用于在流控PAUSE等场景暂存状态 */
    enum im_pg_exit_type exit_flag;
    enum im_pg_migrate_type migrate_type;   /* 切换类型，0为不切换 */

    struct im_volume *last_vol;   /*resync时切换发送卷使用，切换卷需要清空next_vol_offset*/
    struct task_struct *task;
    struct completion event;
    uint8_t *normal_send_buffer;     /* 2M buffer to send multiple im_cmd in request queue*/
    struct list_head list;
};

int im_start_pg_thread(const char *oma_id, const char *token_id,
                             const char *host_id, uint32_t vrg_ip,
                             uint32_t vrg_port, uint32_t exp_rpo,
                             uint8_t bitmap_granularity, int vol_num,
                             ProtectVol *vols,
                             enum im_pg_start_type start_type,
                             uint64_t start_times,
                             uint8_t pause_state);
int im_stop_pg_thread(const char *oma_id, enum im_pg_exit_type exit_type);
void im_stop_all_pg_threads(enum im_pg_exit_type exit_type);
void im_pg_clear_cmd_pending_queue(struct im_pg *pg);
bool im_pg_merge_bitmap(struct im_volume *vol, unsigned char *bitmap, unsigned int len, uint64_t bit_count);
int readdata4cmd(struct im_cmd *cmd);
int readbitmap_from_disk(void *targetdev, unsigned char *dest_data, unsigned long long start);
int writebitmap_to_disk(void *targetdev, unsigned char *src_data, unsigned long long start);
void im_pg_flush_rq_and_pq(struct im_pg *pg);
void im_pg_count_rw_iops_and_wsize(uint64_t write_size);
#endif

