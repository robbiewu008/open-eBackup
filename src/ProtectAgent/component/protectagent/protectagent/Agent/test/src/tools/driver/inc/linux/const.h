/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : const.h
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/25
 * Version     : 1.0
 *
 * Description : ��������
 */

#ifndef _IOMIRROR_CONST_H_
#define _IOMIRROR_CONST_H_

#define IM_VERSION             (0x1)
#define IM_MAGIC               (0xBC)

#define IM_SNAP_ID_LEN         VOL_ID_LEN         /* ����id��󳤶ȣ���IM_VOL_ID_LEN����һ�� */

/**
 * λ������IM_BITMAP_GRANULARITY
 * G    BlockSize   1TB Disk
 * 8    128KB       1MB
 * 9    256KB       512KB
 * 10   512KB       256KB
 * 11   1MB         128KB
 * 12   2MB         64KB
 * 13   4MB         32KB
 */
#define IM_BITMAP_GRANULARITY       (3)        /* ���СΪ2^g������ */
#define IM_MAX_BITMAP_GRANULARITY   (11)        /* λͼ1bit��������������*/
#define IM_SECTOR_SIZE              (512)       /* ������С */
#define IM_MAX_VOL_NUM              (16)        /* ֧�ֵ�������� */
#define IM_MODULE_EXIT_DLAY         (1000)      /* ģ���˳��ӳ�ʱ�䣬ms */
#define IM_VOLUME_DEL_DLAY          (3000)      /* ɾ��ʱ�ͷ���Դ��ʱ��ms */

#define IM_PG_STATISTICS_TIME_LEN   (HZ * 3)    /* ����ͳ�Ƶ�ʱ�䳤��*/
#define IM_PG_WAIT_TIMEOUT          (HZ)        /* pg�̵߳ȴ���ʱֵ */
#define IM_PG_HEARTBEAT_INTERVAL    (HZ * 5)    /* �������ķ���ʱ���� */
#define IM_PG_HEARTBEAT_TIMEOUT     (HZ * 90)   /* �������ĳ�ʱʱ�� */
#define IM_PG_VERIFY_CHECK_INTERVAL (HZ * 5)    /* verify check���ķ���ʱ���� */
#define IM_PG_VERIFY_CHECK_TIMEOUT  (HZ * 90)   /* verify check���ĳ�ʱʱ�� */
#define IM_PG_CMD_RESEND_INTERVAL   (HZ * 30)   /* ���Ʊ��ġ�atomic���ݱ����ط�ʱ���� */
#define IM_PG_EXT_CMD_TIMEOUT       (HZ * 30)   /* ioctl�ȴ�pgִ���ⲿ���ʱʱ�� */
#define IM_PG_EXIT_RECV_DELAY       (200)       /* �˳�ʱ���ճ�ʱ������ʱ�� */
#define IM_PG_IDLE_DELAY            (300)       /* pg�߳̿�ѭ������ʱ�䣬ms */
#define IM_PG_EXIT_TIME_OUT         (HZ * 60)   /* pg�߳������˳����ƻ��л��˳���ʱֵ */
#define IM_PG_RECONNECT_INTERVAL    (20)        /* ����vrgʱ������IM_PG_IDLE_DELAY������ */
#define IM_PG_PROCESS_CNT           (2)        /* ��״̬��ÿ��ѭ����������� */
#define IM_PG_EXIT_RECV_CNT         (300)       /* �˳�ʱÿ�ֳ��Խ��մ��� */
#define IM_PG_FLUSH_SIZE            (16384)     /* requet&pending queue������ֵ����λpage(4KB) */
#define IM_PG_MAX_READ_SIZE         (32768)     /* ÿ��bio����ȡ���ݴ�С��1024*32 Bytes */

#define IM_SOCK_TIMEOUT_SEND        (HZ/2)      /* socket���ͳ�ʱֵ */
#define IM_SOCK_TIMEOUT_RECV        (HZ/100)    /* socket���ճ�ʱֵ */
#define IM_SOCK_MAX_RETRY           (20)        /* socket��ʱ������Դ��� */

#define IM_CONFIG_PATH              ("/etc/huawei/im.conf") /* �����ļ�·�� */
#define IM_DEFAULT_PG_ID            ("im_pg")           /* Ĭ��pg id */
#define IM_CTL_DEV_NAME             ("im_ctldev")       /* ctl�豸�� */

#define IM_DEFAULT_MAX_DATASET_SIZE (1ULL << 30)

#ifdef SUPPORT_BACKUP
#define IM_BACKUP_IO_TIMEOUT        (HZ * 300)   /* COW IO��ʱʱ�� */
#define IM_SNAPSHOT_DEV_NAME        "im_snapdev"        /* ctl�豸�� */
#endif

#endif

