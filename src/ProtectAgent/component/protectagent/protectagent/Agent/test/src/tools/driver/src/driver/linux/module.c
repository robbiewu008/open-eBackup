/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : module.c
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/25
 * Version     : 1.0
 *
 * Description : IOMirror内核模块，模块加载、卸载实现
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/reboot.h>

#include "util.h"
#include "module.h"
#include "protect.h"
#include "ioctl.h"
#include "ctl_cmd.h"

 
/**
 * Description : 系统重启通知函数
 *
 * Parameters  : this
 *               code
 *               x   
 * Return      : NOTIFY_DONE
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/31
 */
static int im_notify_reboot(struct notifier_block *this, unsigned long code, void *x)
{
    if ((code == SYS_DOWN)
        || (code == SYS_HALT)
        || (code == SYS_POWER_OFF))
    {
        IOMIRROR_INFO("IOMirror will exit, flush all PGs.");

        im_stop_all_pg_threads(IM_PG_EXIT_NORMAL);
    }

    return NOTIFY_OK;
}


static struct notifier_block im_notifier =
{
    .notifier_call  = im_notify_reboot,
    .next           = NULL,
    .priority       = INT_MAX,
};


/**
 * Description : IOMirror Module加载函数
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/15
 */
static int __init iomirror_init(void)
{
    int mode, ret = 0;
    struct im_config_pg *conf = NULL;

    IOMIRROR_INFO("IOMirror init.");

    /**
     * 只要reboot_notifier注册成功，
     * 且ctl控制设备创建成功，模块即加载成功
     */
    ret = register_reboot_notifier(&im_notifier);
    if (ret < 0)
    {
        IOMIRROR_ERR("IOMirror register_reboot_notifier failed.");
        goto out;
    }

    ret = im_ctldev_create();
    if (ret < 0)
    {
        IOMIRROR_ERR("IOMirror im_ctldev_create failed.");
        goto fail;
    }

    /* 配置文件读取失败、启动pg线程失败不会导致模块加载失败 */
    conf = im_config_read();
    if (NULL == conf)
    {
        IOMIRROR_ERR("IOMirror im_config_read failed.");
    }
    else
    {
        IOMIRROR_INFO("dev major is %u, dev minor is %u.", conf->bitmap_dev_major, conf->bitmap_dev_minor);
        if (conf->vol_num > 0)
        {
            mode = IM_PG_START_VERIFY;
#ifdef SUPPORT_BACKUP
            if (2 == conf->type)
                mode = IM_PG_START_BACKUP;
            else if ((conf->bitmap_dev_major) &&
#else
            if ((conf->bitmap_dev_major) &&
#endif
                (reset_bitmap_header(MKDEV(conf->bitmap_dev_major, conf->bitmap_dev_minor), conf->bitmap_dev_offset) == 0))
                mode = IM_PG_START_NORMAL;
            im_start_pg_thread(conf->oma_id, NULL, conf->host_id,
                               ntohl(conf->vrg_ip), conf->vrg_port, conf->exp_rpo,
                               conf->bitmap_granularity, conf->vol_num,
                               (ProtectVol *)(conf + 1),
                               mode, conf->start_times + 1, 
                               conf->reboot_pause_state);
        }
        kfree(conf);
        conf = NULL;
    }
    goto out;

fail:
    im_ctldev_remove();
    unregister_reboot_notifier(&im_notifier);

out:
    return ret;
}


/**
 * Description : IOMirror Module卸载函数，
 *               除卸IOMirror外，此函数不应被调用，
 *               即用户不应手动卸载IOMirror模块。
 *
 * Parameters  : void
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/15
 */
static void __exit iomirror_exit(void)
{
    int ret = 0;

    IOMIRROR_INFO("IOMirror exit ABNORMAL.");

    im_ctldev_remove();

    ret = unregister_reboot_notifier(&im_notifier);
    if (ret < 0)
    {
        IOMIRROR_ERR("IOMirror unregister_reboot_notifier failed.");
    }

    im_stop_all_pg_threads(IM_PG_EXIT_ABNORMAL);

    msleep(IM_MODULE_EXIT_DLAY);
}


module_init(iomirror_init);
module_exit(iomirror_exit);

MODULE_AUTHOR("Huawei Inc.");
MODULE_DESCRIPTION("IOMirror");
MODULE_LICENSE("Huawei");

