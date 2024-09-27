/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : const.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/25
 * Version     : 1.0
 *
 * Description : 常量定义
 */

#ifndef _IOMIRROR_CONST_H_
#define _IOMIRROR_CONST_H_

#define IM_VERSION             (0x1)
#define IM_MAGIC               (0xBC)

#define IM_SNAP_ID_LEN         VOL_ID_LEN         /* 快照id最大长度，与IM_VOL_ID_LEN保持一致 */

/**
 * 位表粒度IM_BITMAP_GRANULARITY
 * G    BlockSize   1TB Disk
 * 8    128KB       1MB
 * 9    256KB       512KB
 * 10   512KB       256KB
 * 11   1MB         128KB
 * 12   2MB         64KB
 * 13   4MB         32KB
 */
#define IM_BITMAP_GRANULARITY       (3)        /* 块大小为2^g个扇区 */
#define IM_MAX_BITMAP_GRANULARITY   (11)        /* 位图1bit代表的最大扇区数*/
#define IM_SECTOR_SIZE              (512)       /* 扇区大小 */
#define IM_MAX_VOL_NUM              (16)        /* 支持的最大卷个数 */
#define IM_MODULE_EXIT_DLAY         (1000)      /* 模块退出延迟时间，ms */
#define IM_VOLUME_DEL_DLAY          (3000)      /* 删卷时释放资源延时，ms */

#define IM_PG_STATISTICS_TIME_LEN   (HZ * 3)    /* 性能统计的时间长度*/
#define IM_PG_WAIT_TIMEOUT          (HZ)        /* pg线程等待超时值 */
#define IM_PG_HEARTBEAT_INTERVAL    (HZ * 5)    /* 心跳报文发送时间间隔 */
#define IM_PG_HEARTBEAT_TIMEOUT     (HZ * 90)   /* 心跳报文超时时间 */
#define IM_PG_VERIFY_CHECK_INTERVAL (HZ * 5)    /* verify check报文发送时间间隔 */
#define IM_PG_VERIFY_CHECK_TIMEOUT  (HZ * 90)   /* verify check报文超时时间 */
#define IM_PG_CMD_RESEND_INTERVAL   (HZ * 30)   /* 控制报文、atomic数据报文重发时间间隔 */
#define IM_PG_EXT_CMD_TIMEOUT       (HZ * 30)   /* ioctl等待pg执行外部命令超时时间 */
#define IM_PG_EXIT_RECV_DELAY       (200)       /* 退出时接收超时后休眠时间 */
#define IM_PG_IDLE_DELAY            (300)       /* pg线程空循环休眠时间，ms */
#define IM_PG_EXIT_TIME_OUT         (HZ * 60)   /* pg线程正常退出、计划切换退出超时值 */
#define IM_PG_RECONNECT_INTERVAL    (20)        /* 重连vrg时间间隔，IM_PG_IDLE_DELAY整数倍 */
#define IM_PG_PROCESS_CNT           (2)        /* 各状态下每轮循环最大处理报文数 */
#define IM_PG_EXIT_RECV_CNT         (300)       /* 退出时每轮尝试接收次数 */
#define IM_PG_FLUSH_SIZE            (16384)     /* requet&pending queue长度阈值，单位page(4KB) */
#define IM_PG_MAX_READ_SIZE         (32768)     /* 每个bio最多读取数据大小，1024*32 Bytes */

#define IM_SOCK_TIMEOUT_SEND        (HZ/2)      /* socket发送超时值 */
#define IM_SOCK_TIMEOUT_RECV        (HZ/100)    /* socket接收超时值 */
#define IM_SOCK_MAX_RETRY           (20)        /* socket超时最大重试次数 */

#define IM_CONFIG_PATH              ("/etc/huawei/im.conf") /* 配置文件路径 */
#define IM_DEFAULT_PG_ID            ("im_pg")           /* 默认pg id */
#define IM_CTL_DEV_NAME             ("im_ctldev")       /* ctl设备名 */

#define IM_DEFAULT_MAX_DATASET_SIZE (1ULL << 30)

#ifdef SUPPORT_BACKUP
#define IM_BACKUP_IO_TIMEOUT        (HZ * 300)   /* COW IO超时时间 */
#define IM_SNAPSHOT_DEV_NAME        "im_snapdev"        /* ctl设备名 */
#endif

#endif

