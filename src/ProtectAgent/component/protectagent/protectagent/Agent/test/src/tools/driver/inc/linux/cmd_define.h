/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : cmd.h
 * Author      : WANG Chao/w00238084
 * Date        : 2013/5/29
 * Version     : 1.0
 *
 * Description : 报文格式、报文类型定义
 *
 */

#ifndef _IOMIRROR_CMD_H
#define _IOMIRROR_CMD_H

#include <linux/types.h>
#include "const.h"
#include "../share/cmd.h"

enum local_bio_status
{
    LOCAL_BIO_SUCCESS = 0,
    LOCAL_BIO_ONGOING = 1,
    LOCAL_BIO_FAILURE = 2
};

enum del_bio_status
{
    STATUS_DEL_UNSET = 0,
    STATUS_DEL_IN_FLUSHED = 1,
    STATUS_DEL_IN_BIO_END_IO = 2
};

#define DATASET_HAS_RECEIVED 1

struct im_cmd_header
{
    uint8_t     version;
    uint8_t     magic;
    uint16_t    cmd_type;
    uint64_t    data_size_bytes;
    uint64_t    data_offset;              /* 单位:扇区 */
    uint64_t    request_id;
    char        host_id[VM_ID_LEN];  /* 主机ID(PM_ID/VM_ID)，对应主机层VM_ID */
    char        oma_id[VM_ID_LEN];      /* OMA ID，对应主机层VM_SUFFIX，当前只对外呈现一个保护组 */
    char        vol_id[VOL_ID_LEN];
    char        ack_result;
    char        reserved[15];
} __attribute__((packed));

struct im_cmd
{
    struct im_cmd_header header;
    struct bio_vec *bvl;
    unsigned short vcnt;
    void *buf;
    void *private;
#ifdef SUPPORT_BACKUP
    uint8_t is_snapread;
    struct completion *completion;        /* 等待COW完成 */
#endif
   
    uint32_t send_time;                   /* the time that the cmd sended to oma */ 
    struct im_cmd_status *status; 
    struct list_head list;
    uint8_t has_received; // flag : 0 --- dataset has not received, 1 --- dataset has received but related_local_bio_complete is ongoing
};

struct im_cmd_status
{
    unsigned short related_local_bio_complete; /* flag of local IO completion: 0---success, 1---ongoing, 2---fail */
    unsigned short cmd_flushed; /* flag --- who is responsible to delete im_cmd_status 0 --- im_pg_flush_rq_and_pq, 1 --- im_bio_end_io */
    struct bio* bio_ori; /* related local origin bio */
    atomic_t del_flag;
};

#define DPP_CBT_DATASET_FLAG    0x8000
//#define DPP_SET_DATA_THRESHOLD				((uint64_t) (20) << 30)

#endif

