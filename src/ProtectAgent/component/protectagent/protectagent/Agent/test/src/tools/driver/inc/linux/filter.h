/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : filter.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/18
 * Version     : 1.0
 *
 * Description : ��������im_volume�ṹ����
 *
 */

#ifndef _IOMIRROR_FILTER_H
#define _IOMIRROR_FILTER_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>

#include "const.h"
#include "bitmap_alloc.h"
#include "ctl_cmd.h"

#ifdef SUPPORT_BACKUP
struct im_pg;
#endif

/* д������� */
struct im_reqeust_queue
{
    struct list_head  head;         /* д�����������ͷ */
    uint64_t          count;        /* ������������� */
    uint64_t          page_cnt;     /* ����������ռ�ÿռ��С����λpage */
    wait_queue_head_t wq;           /* �ȴ����У������뱣����ͬ�� */
    struct semaphore  lock;         /* �����������ȼ��� */
#ifdef SUPPORT_BACKUP
    struct im_pg      *pg;
#endif
};

/* ԭʼmake request fn */
struct im_disk_ori_mfn
{
    make_request_fn *f;
    uint8_t cnt;
};

/* ���������һ����У��״̬ */
enum im_vol_verify_state
{
    IM_VOL_VERIFY_NOT_START = 0,
    IM_VOL_VERIFY_SEND_DATA,
    IM_VOL_VERIFY_DATA_END_ACK_P,
    IM_VOL_VERIFY_BITMAP_P,
    IM_VOL_VERIFY_FINISH,
};

/* ����������������� */
struct im_volume
{
    char        id[VOL_ID_LEN];
    char        path[DISK_PATH_LEN];

    dev_t       bd_dev;                 /* ����������豸�� */
    dev_t       bd_disk_dev;            /* ���������������̵��豸�� */
    bool        is_part;                /* �Ƿ�Ϊ���� */
    sector_t    start;                  /* ���ڴ����е���ʼ���� */
    sector_t    end;                    /* ���ڴ����еĽ������� */
    sector_t    sectors;                /* ���������С����λΪ���� */

    struct block_device     *bdev;      /* ��������Ŀ��豸�ṹ�� */
    struct im_reqeust_queue *rq;        /* ��������������д������� */
    struct im_disk_ori_mfn  *ori_mfn;   /* ��������ԭʼд������ָ�� */

    OM_BITMAP     *bitmap;                /* CBTλ�� */
    OM_BITMAP     *bitmap_verify;         /* һ����У��λ�� */

#ifdef SUPPORT_BACKUP
    uint8_t volume_minor;
    struct request_queue *snap_queue;
    struct gendisk *snap_disk;
#endif
    OM_BITMAP     *bitmap_original;       /* ԭʼCBTλ�� */
#ifdef SUPPORT_BACKUP
    OM_BITMAP     *bitmap_temp;           /* ��ʱCBTλ�� */
    OM_BITMAP     *bitmap_filter;         /* CBT����λ�� */
    OM_BITMAP     *bitmap_snapshot;       /* COWλ�� */
#endif

    OM_BITMAP_IT *hbi_send;
    enum im_vol_verify_state verify_state;

    struct list_head list0;
    struct list_head list1;

};


struct im_volume *im_add_volume(const char *id,
                        const char *path, uint64_t sectors,
                        int bitmap_granularity, struct im_reqeust_queue *rq);
void im_del_volume(struct im_volume *vol);
#ifdef SUPPORT_BACKUP
void im_cmd_free(struct im_cmd *cmd);
struct im_cmd *im_alloc_cmd4snapio(struct im_volume *vol, uint64_t offset);
struct im_cmd *im_alloc_cmd4diskio(struct im_volume *vol, uint64_t offset, uint8_t is_snapread);
#endif

#endif

