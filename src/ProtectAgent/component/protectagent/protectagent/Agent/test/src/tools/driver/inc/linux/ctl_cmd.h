/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : ctl_cmd.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/8
 * Version     : 1.0
 *
 * Description : ioctl�����ֶ��壬ioctl�����������Ͷ��壬
 *               ���û�̬�������ݽṹ���壬���ļ��ں�̬���û�̬����
 */

#ifndef _IOMIRROR_CTL_CMD_H_
#define _IOMIRROR_CTL_CMD_H_

#include "const.h"
#include "../share/ctl_define.h"
#include "../share/persist_data.h"

#define IM_CTL_MAGIC  'k'
#define IM_CTL_BITMAP_HEADER_MAGIC  "HWBITMAPHEADER"

/* �����ļ��б�������Ϣ��ʽ */
struct im_config_pg
{
    uint64_t    start_times;            /* �����鴴������������ */
    char        oma_id[VM_ID_LEN];
    char        host_id[VM_ID_LEN];
    uint32_t    vrg_ip;
    uint32_t    vrg_port;
    uint32_t    exp_rpo;
    uint8_t     bitmap_granularity;

    uint32_t    bitmap_dev_major;
    uint32_t    bitmap_dev_minor;
    uint64_t    bitmap_dev_offset;
    uint8_t     type;
    uint8_t     reboot_pause_state;

    uint8_t     vol_num;
} __attribute__((packed));

/* ����ʱ��ʱʹ�õı�����������������Ҫ��Ҫ����ʹ�� */
struct im_config_variable
{
    uint64_t    cbt_flush_times;        /* �����������������͵�dataset ID */
    uint64_t    cbt_flush_times_done;   /* ���������������յ������һ��dataset ID done*/
};

/* IOMirror״̬��������RD���� */
#define IM_CTL_IOMIRROR_STATE_BASE_SYNC     (2)
#define IM_CTL_IOMIRROR_STATE_CBT           (3)
#define IM_CTL_IOMIRROR_STATE_NORMAL        (4)
#define IM_CTL_IOMIRROR_STATE_VERIFY        (5)
#define IM_CTL_IOMIRROR_STATE_PAUSE         (7)
#define IM_CTL_IOMIRROR_STATE_LINK_DOWN     (8)
#define IM_CTL_IOMIRROR_STATE_ABNORMAL      (9)

/* IM_CTL_QUERY_IOMIRROR������Ϣ�ṹ */
struct im_ctl_query_iomirror
{
    int8_t      ret;
    uint8_t     state;
    uint64_t    secs_left;
};

struct im_ctl_bitmap
{
    uint8_t     vol_dev_id;
    char        vol_path[PER_DISK_NAME_LEN];
    char        vol_id[VOL_ID_LEN];
    uint64_t    bitmap_size;
    // bitmap��1������
    uint64_t    bitmap_count;
    uint64_t    *data;
};

struct im_ctl_bitmap_extent
{
    uint64_t    offset;
    uint32_t    length;
};
 
struct im_ctl_bitmap_extent_setting
{
    uint32_t    dev_major;
    uint32_t    dev_minor;
    uint32_t    extent_num;
    struct im_ctl_bitmap_extent *data;
};

struct im_ctl_notify_change
{
    uint32_t    length;
    uint64_t    *data;
};

#ifdef SUPPORT_BACKUP
struct im_ctl_backup_take_snapshot
{
	char        snap_id[IM_SNAP_ID_LEN];
    uint8_t     bitmap_granularity;
    uint8_t     max_vol_minor;
	uint32_t	vol_devno[128];
};
 
struct im_ctl_backup_remove_snapshot
{
	char        snap_id[IM_SNAP_ID_LEN];
    uint8_t     is_failed;
};

#define IM_CTL_BACKUP_SNAPREAD_FLAG	0x8000000000000000
#define IM_CTL_BACKUP_OFFSET_MASK	0x00FFFFFFFFFFFFFF
#define IM_CTL_BACKUP_VOLUME_BIT	59
#endif

#endif

