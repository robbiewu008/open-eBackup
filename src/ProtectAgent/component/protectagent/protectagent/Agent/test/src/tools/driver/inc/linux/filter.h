/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : filter.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/18
 * Version     : 1.0
 *
 * Description : 被保护卷im_volume结构定义
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

/* 写请求队列 */
struct im_reqeust_queue
{
    struct list_head  head;         /* 写请求队列链表头 */
    uint64_t          count;        /* 队列中请求个数 */
    uint64_t          page_cnt;     /* 队列中请求占用空间大小，单位page */
    wait_queue_head_t wq;           /* 等待队列，用于与保护组同步 */
    struct semaphore  lock;         /* 操作队列需先加锁 */
#ifdef SUPPORT_BACKUP
    struct im_pg      *pg;
#endif
};

/* 原始make request fn */
struct im_disk_ori_mfn
{
    make_request_fn *f;
    uint8_t cnt;
};

/* 被保护卷的一致性校验状态 */
enum im_vol_verify_state
{
    IM_VOL_VERIFY_NOT_START = 0,
    IM_VOL_VERIFY_SEND_DATA,
    IM_VOL_VERIFY_DATA_END_ACK_P,
    IM_VOL_VERIFY_BITMAP_P,
    IM_VOL_VERIFY_FINISH,
};

/* 被保护卷，分区或磁盘 */
struct im_volume
{
    char        id[VOL_ID_LEN];
    char        path[DISK_PATH_LEN];

    dev_t       bd_dev;                 /* 被保护卷的设备号 */
    dev_t       bd_disk_dev;            /* 被保护卷所属磁盘的设备号 */
    bool        is_part;                /* 是否为分区 */
    sector_t    start;                  /* 所在磁盘中的起始扇区 */
    sector_t    end;                    /* 所在磁盘中的结束扇区 */
    sector_t    sectors;                /* 被保护卷大小，单位为扇区 */

    struct block_device     *bdev;      /* 被保护卷的块设备结构体 */
    struct im_reqeust_queue *rq;        /* 被保护卷所属的写请求队列 */
    struct im_disk_ori_mfn  *ori_mfn;   /* 被保护卷原始写请求函数指针 */

    OM_BITMAP     *bitmap;                /* CBT位表 */
    OM_BITMAP     *bitmap_verify;         /* 一致性校验位表 */

#ifdef SUPPORT_BACKUP
    uint8_t volume_minor;
    struct request_queue *snap_queue;
    struct gendisk *snap_disk;
#endif
    OM_BITMAP     *bitmap_original;       /* 原始CBT位表 */
#ifdef SUPPORT_BACKUP
    OM_BITMAP     *bitmap_temp;           /* 临时CBT位表 */
    OM_BITMAP     *bitmap_filter;         /* CBT过滤位表 */
    OM_BITMAP     *bitmap_snapshot;       /* COW位表 */
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

