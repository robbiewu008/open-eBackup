/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : protect.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/18
 * Version     : 1.0
 *
 * Description : ������������ݽṹ���壬������ֹͣ�������߳̽ӿ�����
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

/* ������״̬�����ڶ������ */
enum im_pg_state
{
    IM_PG_STATE_NORMAL = 1,
    IM_PG_STATE_CBT,
    IM_PG_STATE_VERIFY,
    IM_PG_STATE_ATOMIC,    /* ����CBT�������ط�*/
};

/* �������ڲ�״̬�����ڸ������̿��� */
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

    IM_PG_INTER_STATE_CONNECT_STAGE0,  /* ����socket���ӣ�����CONNECT_VRG���� */
    IM_PG_INTER_STATE_CONNECT_STAGE0_P,
    IM_PG_INTER_STATE_CONNECT_STAGE1,  /* ����IOMIRROR_START���� */
    IM_PG_INTER_STATE_CONNECT_STAGE1_P,

    IM_PG_INTER_STATE_WAIT_PENDING_VOLS,/* �ȴ���������ready */

    IM_PG_INTER_STATE_MAX,
};

/* �������߳��������� */
enum im_pg_start_type
{
    IM_PG_START_INIT = 0,    /* ��ʼͬ����ʽ�����ڴ�����������ʱ */
    IM_PG_START_NORMAL,      /* ������ʽ������OS����ʱ */
    IM_PG_START_VERIFY,      /* һ����У�鷽ʽ�����ڻ���ʱ */
#ifdef SUPPORT_BACKUP
    IM_PG_START_BACKUP,      /* ���ݷ�ʽ�����ڱ���״̬ */
#endif
};

#ifdef SUPPORT_BACKUP
/* ����ģʽ״̬ */
enum im_backup_state
{
    IM_NO_BACKUP_MODE = 0,
    IM_BACKUP_INIT,
    IM_BACKUP_CBT,
    IM_BACKUP_SNAPSHOT,
};
#endif

/* �������߳��˳����� */
enum im_pg_exit_type
{
    IM_PG_RUNNING = 0,
    IM_PG_EXIT_NORMAL,  /* �����˳����ػ��������� */
    IM_PG_EXIT_ABNORMAL,/* �쳣�˳����û��ֶ�ж��ģ�顢����pg�߳�ʧ�ܵ� */
    IM_PG_EXIT_STOP,    /* ɾ������ */
    IM_PG_EXIT_PLAN,    /* �ƻ��л� */
    IM_PG_EXIT_DR,      /* �����л� */
};

/* �������л����� */
enum im_pg_migrate_type
{
    IM_PG_MIGRATE_PLAN = 1, /* �ƻ��л� */
    IM_PG_MIGRATE_DISASER,  /* �����л� */
};

/* ������ack������� */
struct im_pg_pending_queue
{
    struct list_head  head;
    struct list_head *atomic;
    unsigned long     atomic_recv_time;
    uint64_t          count;
    uint64_t          page_cnt;
};

/* ������ack�Ŀ��Ʊ��Ķ��У����ڿ��Ʊ����ط� */
struct im_pg_cmd_pending_queue
{
    struct list_head  head;
    uint64_t          count;
    uint64_t          request_id;
    unsigned long     send_time;       /* �����б������һ���ط�ʱ�� */
};

/* �ⲿ�������� */
enum im_pg_ext_cmd_type
{
    IM_PG_EXT_CMD_ADD_VOL = 1,  /* ��̬��Ӿ� */
    IM_PG_EXT_CMD_DEL_VOL,      /* ��̬ɾ���� */
    IM_PG_EXT_CMD_VOL_READY,
    IM_PG_EXT_CMD_PAUSE,
    IM_PG_EXT_CMD_RESUME,
    IM_PG_EXT_CMD_MODIFY,
    IM_PG_EXT_CMD_MOD_VOL,      /* ��̬�޸ľ� */
    IM_PG_EXT_CMD_STOP_SEND_DATA,
    IM_PG_EXT_CMD_GET_KERNEL_ALARM,
    IM_PG_EXT_CMD_SET_TOKEN_ID,
};

enum im_pg_limit_kernel_alarm_report_rate {
    LIMIT_ALARM_BITMAP_NOT_READY = 0,          /* normal reboot but driver don't get bitmap from HA */
    LIMIT_ALARM_RECV_FAILED_ACK_FORM_OMA,      /* driver has been received failed ack from oma */
    LIMIT_ALARM_BUTTOM,
};

/* �������߳��첽������ⲿ���� */
struct im_pg_external_cmd
{
    enum im_pg_ext_cmd_type type;
    struct completion comp;
    void *info;
    int8_t ret;
};

/* ��׼�������ı������� */
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

/* ������ṹ�� */
struct im_pg
{
    char        id[VM_ID_LEN];
    char        host_id[VM_ID_LEN];
    uint32_t    vrg_ip;                 /* VRG IP��ַ */
    uint32_t    vrg_port;               /* VRG �˿� */
    uint32_t    exp_rpo;                /* RPO����λ�� */
    uint8_t     bitmap_granularity;     /* λ�����ȣ����С=512*2^g */

    volatile uint8_t     is_init;       /* �Ƿ���ɳ�ʼͬ�� */
    uint64_t    total_credit;           /* �������أ���λ�ֽ� */
    uint64_t    sent_amount;            /* �������أ���λ�ֽ� */
    uint64_t    max_dataset_size;       /* �������أ���λ�ֽ� */
    uint64_t    next_vol_offset;        /* �������أ���λ�ֽ� */

    volatile uint8_t flow_control_pause_flag;/* �������أ���ͣ����IO���ݵ�OMA*/ 
    volatile uint8_t pause_pending;          /* pause�ӿ���ʱ״̬����Ҫ����һ��dataset֮����ܽ���pause״̬*/
    volatile uint8_t resume_pending;         
    volatile uint8_t stop_send_data;        /*when normal shutdown, agent send ioctl to stop driver send data to soma*/
    uint8_t  reboot_pause_state;        /* After reboot, driver may enter pause state*/

    uint64_t    cbt_flush_times;        /* �����������������͵�dataset ID */
    uint64_t    cbt_flush_times_done;   /* ���������������յ������һ��dataset ID done*/
    uint64_t    start_times;            /* �����鴴������������ */
    uint64_t    verify_times;           /* ������������������һ����У����� */
    uint32_t    connect_times;          /* ��VRG�ɹ��������� */
    uint8_t     vol_num;
    uint64_t    request_id;             /* ��ǰnormal/atomic���ĵ�request id */
    uint64_t    queuePageCnt;           /* normal ״̬��֧�ַ�����64M������pause*/

    unsigned long hb_send_time; /* ���һ�η�����������ʱ�� */
    unsigned long hb_recv_time; /* ���һ���յ���������ʱ�� */

    unsigned long verify_send_time; /* ���һ�η���verify check����ʱ�� */
    unsigned long verify_recv_time; /* ���һ���յ�verify check����ʱ�� */

    unsigned long rpo_send_time; /* ���һ��dataset����ʱ�� */
    
    char              snap_id[IM_SNAP_ID_LEN];  /* ����GUID��������֤��������Ч�� */
    volatile uint8_t  backup_status;      		/* ����ģʽ״̬��0���Ǳ���ģʽ��1�����ݳ�ʼ״̬��2������CBT״̬��3�����ݿ���״̬ */
    uint8_t           cur_volume_minor;			/* ��ǰ����volume minor�� */
    dev_t             dev_savebitmap;           /* ����persistent bitmap�Ĵ����豸��Ϣ*/
    struct im_ctl_bitmap_extent *bitmap_extent; /* ����persistent bitmap�Ĵ���ƫ������*/

    dev_t             dev_saveconfigfile;           /* ����config file�Ĵ����豸��Ϣ*/
    struct im_ctl_bitmap_extent *configfile_extent; /* ����config file�Ĵ���ƫ������*/
       
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
    struct im_reqeust_queue rq;    /* request queue, �ȴ����� */
    struct im_pg_pending_queue pq; /* pending queue, �ѷ��ͣ��ȴ�����ack */
    struct im_pg_cmd_pending_queue cmd_pq;  /* ���ط����Ʊ��Ķ��� */
    struct im_pg_external_cmd *ext_cmd;     /* �ȴ�������ⲿ���� */
    enum im_pg_state state;
    enum im_pg_inter_state inter_state;
    enum im_pg_inter_state temp_state;      /* ����������PAUSE�ȳ����ݴ�״̬ */
    enum im_pg_exit_type exit_flag;
    enum im_pg_migrate_type migrate_type;   /* �л����ͣ�0Ϊ���л� */

    struct im_volume *last_vol;   /*resyncʱ�л����;�ʹ�ã��л�����Ҫ���next_vol_offset*/
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

