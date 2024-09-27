/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : ioctl.c
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/8
 * Version     : 1.0
 *
 * Description : 创建用于ioctl的字符设备、ioctl命令处理
 *
 */

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/version.h>

#include "const.h"
#include "util.h"
#include "ioctl.h"
#include "ctl_cmd.h"
#include "protect.h"
#include "../share/persist_data.h"
#include "../share/ctl_define.h"

extern struct list_head im_pg_list_head;

/* 用于ioctl的块设备 */
static struct im_ctldev
{
#ifdef SUPPORT_BACKUP
    uint32_t blkno;
#endif
    dev_t  devno;
    struct semaphore sem;
    struct cdev cdev;
    struct class *cls;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27))
    struct class_device *cd;
#else
    struct device *cd;
#endif
} ctldev;

/**
* Description : compute bitmap granularity according to protect_size and mem_threshold. The
*               last computed granularity must be [IM_BITMAP_GRANULARITY, IM_MAX_BITMAP_GRANULARITY]
*
* Parameters  : protect_size -- all protected disks size, uint is GB
*               mem_threshold -- allocated for save bitmap, uint is MB
*
* Return      : computed bitmap granularity
* 
* Author      : z00455045
* Date        : 2019/10/16
*/
static uint8_t compute_bitmap_granularity(unsigned int protect_size, unsigned int mem_threshold)
{
    const uint8_t bits_per_byte = 8;
    uint8_t bitmap_granularity;
    uint64_t protect_size_bytes = (1ULL << 30) * protect_size;
    uint64_t protect_size_sector = mem_threshold * (1ULL <<20) * IM_SECTOR_SIZE * bits_per_byte;

    if (mem_threshold == 0 || protect_size == 0) {
        bitmap_granularity = IM_BITMAP_GRANULARITY;
        IOMIRROR_INFO("bitmap_granularity is set default vaule %d.", IM_BITMAP_GRANULARITY);
        return bitmap_granularity;
    }
   
    if(protect_size_bytes % protect_size_sector == 0) {
        bitmap_granularity = (uint8_t)make_log_upper_bound(protect_size_bytes / protect_size_sector) - 1;
    } else {
        bitmap_granularity = (uint8_t)make_log_upper_bound(protect_size_bytes / protect_size_sector);
    }

    if (bitmap_granularity < IM_BITMAP_GRANULARITY) {
        IOMIRROR_INFO("Given memThreshold is enough big, use the minimal granularity.");
        bitmap_granularity = IM_BITMAP_GRANULARITY;
    } else if (bitmap_granularity > IM_MAX_BITMAP_GRANULARITY) {
        IOMIRROR_INFO("Given memThreshold is too small, " 
           "use the max granularity, driver may use more memory than memThreshold.");
        bitmap_granularity = IM_MAX_BITMAP_GRANULARITY;
    }
    
    IOMIRROR_INFO("Final bitmap granularity is %d.", bitmap_granularity);
    return bitmap_granularity;
}

/**
 * Description : 下发配置并启动IOmirror
 *
 * Parameters  : data - 用户态buf指针
 * Return      : 获取成功 - 0
 *               获取失败 - -1
 *
 * Author      : l00184171
 * Date        : 2014/5/17
 */
static int im_ctl_handle_start_iomirror_cmd(void* data, int mode)
{
    int ret;
    ProtectStrategy info;
    uint8_t bitmap_granularity;
    uint8_t send_now;
    //ProtectVol *parts = NULL;
    //uint8_t vol_num;

    IOMIRROR_INFO("Enter to im_ctl_handle_start_iomirror_cmd, type %d.", mode);

    if (!list_empty(&im_pg_list_head))
    {
        IOMIRROR_ERR("im_pg list is not empty.");
        //info.ret = -1;
        goto out;     
    }

    /**get hotsid,vrg info,speed,vol_num form user data.**/
    ret = copy_from_user(&info, data,sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("im_ctl_dr_config copy from user error.");
        //info.ret = -1;
        goto out;
    }

    if (info.send_now > 1) {
        IOMIRROR_ERR("Param sendNow (%u) is error.", info.send_now);
        goto out;
    } else {
        send_now = (info.send_now == 1 ? 0 : 1);
    }
    //vol_num = info.vol_num;
    //IOMIRROR_DBG("im config data is os_id=%s,vrg_ip=%u,vrg_port=%u,max_speed=%u,vol_num=%u", 
    //   info.host_id, info.vrg_ip, info.vrg_port, info.max_speed, vol_num);
    
    /**get vol(partition) id,path from user date.**/    
/*
    parts = kmalloc(sizeof(ProtectVol)*vol_num, GFP_KERNEL);
    if (unlikely(NULL == parts))
    {
        IOMIRROR_ERR("im_ctl_handle_start_iomirror_cmd malloc failed.");
        info.ret = -1;
        goto out;
    }
    ret = copy_from_user(parts, (char *)data + sizeof(ProtectStrategy), sizeof(ProtectVol) * vol_num);
    if (ret < 0)
    {
        IOMIRROR_ERR("im_ctl_dr_partiton copy from user error.");
        //info.ret = -1;
        goto out;
    }

    if (info.type == 0)
    {
*/
        /*nomal start*/

        info.exp_rpo = info.exp_rpo > 3 ? info.exp_rpo : 3;
        bitmap_granularity = compute_bitmap_granularity(info.protect_size, info.mem_threshold);
        ret = im_start_pg_thread(info.oma_id, info.token_id, info.vm_id, ntohl(info.oma_ip[0]), info.oma_port, info.exp_rpo,
                             bitmap_granularity, 0, NULL, mode, 0, send_now);
    //}
    //else if (info.type == 1)
    //{
       /*verify start*/
/*
       ret = im_start_pg_thread(IM_DEFAULT_PG_ID, info.host_id, ntohl(info.vrg_ip), info.vrg_port, info.max_speed,
                             IM_BITMAP_GRANULARITY, 0, NULL, IM_PG_START_VERIFY, 0);
    }
    else
    {
        IOMIRROR_ERR("Start im type error,type=%d",info.type);
        //info.ret = -1;
        goto out;
    }        
*/
    if (ret!=0)
    {
        IOMIRROR_ERR("im_start_pg_thread return failed %d.",ret);
        //info.ret=-1;
    }
    
out:
    ret = copy_to_user(data, &info, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("im_ctl_handle_start_iomirror_cmd copy_to_user failed.");
        ret = -1;
    }
/*
    if (parts != NULL)
    {
        kfree(parts);
    }
*/
    return ret;
}


/**
 * Description : 停止IOmirror
 *
 * Parameters  : void
 * Return      : 获取成功 - 0
 *               获取失败 - -1
 *
 * Author      : l00184171
 * Date        : 2014/5/17
 */
static int im_ctl_handle_stop_iomirror_cmd(void)
{
    IOMIRROR_INFO("enter im_ctl_handle_stop_iomirror_cmd.");

    im_stop_all_pg_threads(IM_PG_EXIT_STOP);

    return 0;
}

/**
 * Description : 修改IOMirror，配置
 *
 * Parameters  : data - 用户态buf指针
 * Return      : 获取成功 - 0
 *               获取失败 - -1
 *
 * Author      : l00184171
 * Date        : 2014/5/20
 */
static int im_ctl_handle_modify_iomirror_cmd(void* data)
{
    int ret;
    struct im_pg *pg = NULL;
    ProtectStrategy info;
    char ip_buf[IP_STRING_LEN] = {0};
    struct im_pg_external_cmd ext_cmd;

    IOMIRROR_INFO("Enter to modify iomirror.");

    ret = copy_from_user(&info, data, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("im_config_pg copy from user error.");
        goto out;
    }

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -1;
    }

    parse_ip_str(ntohl(info.oma_ip[0]), ip_buf, IP_STRING_LEN);
    IOMIRROR_INFO("modify iomiiror, protect info is vrg_ip=%s, vrg_port=%u, exp_rpo=%u, mem_threshold=%u, protect_size=%u, vm_id=%llx, oma_id=%llx.", 
                 ip_buf, info.oma_port, info.exp_rpo, info.mem_threshold, info.protect_size, *(long long*)(info.vm_id), *(long long*)(info.oma_id));

    ext_cmd.type = IM_PG_EXT_CMD_MODIFY;
    ext_cmd.info = &info;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (0 == ext_cmd.ret)
    {
        pg->ext_cmd = NULL;
        return 0;
    }
    else
    {
        pg->ext_cmd = NULL;
        IOMIRROR_ERR("modify iomirror failed.");
        return -1;
    }

out:
    ret = copy_to_user(data, &info, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("im_ctl_handle_modify_iomirror_cmd copy_to_user failed.");
        ret = -1;
    }
    return -1;
}


static int im_ctl_handle_stop_send_data(void)
{
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;
    
    IOMIRROR_INFO("enter im_ctl_handle_stop_send_data.");

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -1;
    }

    ext_cmd.type = IM_PG_EXT_CMD_STOP_SEND_DATA;
    ext_cmd.info = NULL;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (0 == ext_cmd.ret)
    {
        pg->ext_cmd = NULL;
        return 0;
    }
    else
    {
        pg->ext_cmd = NULL;
        IOMIRROR_ERR("stop send data failed.");
        return -1;
    } 
}

/**
 * Description : 暂停IOMirror数据复制，增加接口防重入的判断
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : z00455045
 * Date        : 2019/07/05
 */
static int im_ctl_handle_pause_iomirror_cmd(void* data)
{
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;
    WaitFlushQueue info;
    int ret;

    IOMIRROR_INFO("enter im_ctl_handle_pause_iomirror_cmd.");

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -1;
    }
    
    info.waitFlushQueueFlag = false;
    if (pg->state == IM_PG_STATE_NORMAL) {
        ret = copy_from_user(&info, data, sizeof(info));
        if (ret < 0) {
            IOMIRROR_ERR("im_config_pg copy from user error, ret = %d.", ret);
            return ret;
        }
        IOMIRROR_INFO("pause with parameter %d under normal state.", info.waitFlushQueueFlag);
    }

    if (pg->flow_control_pause_flag == 1 || pg->pause_pending == 1) { // 已经暂停或者正在暂停中
        IOMIRROR_ERR("Pause is ongoing, pause flag is %d, pause pending is %d.",
            pg->flow_control_pause_flag, pg->pause_pending);
        return 0; // 返回成功
    }

    pg->queuePageCnt = 0;

    ext_cmd.type = IM_PG_EXT_CMD_PAUSE;
    ext_cmd.info = &info;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (0 == ext_cmd.ret)
    {
        pg->ext_cmd = NULL;
        return 0;
    }
    else
    {
        pg->ext_cmd = NULL;
        IOMIRROR_ERR("pause iomirror failed.");
        return -1;
    }
}


/**
 * Description : 恢复IOMirror数据复制
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/29
 */
static int im_ctl_handle_resume_iomirror_cmd(void)
{
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;

    IOMIRROR_INFO("enter im_ctl_handle_resume_iomirror_cmd.");

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -1;
    }

    if (pg->flow_control_pause_flag == 0 && pg->pause_pending == 0)
    {
        IOMIRROR_ERR("Iomirror state no need to resume, return succ directly.");
        return 0;
    }

    ext_cmd.type = IM_PG_EXT_CMD_RESUME;
    ext_cmd.info = NULL;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    pg->resume_pending = 1;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (0 == ext_cmd.ret)
    {
        pg->ext_cmd = NULL;
        IOMIRROR_INFO("resume iomirror successful.");
        return 0;
    }
    else
    {
        pg->ext_cmd = NULL;
        IOMIRROR_ERR("resume iomirror failed.");
        return -1;
    }
}


/**
 * Description : 动态添加卷
 *
 * Parameters  : data - 用户态传入待添加的卷
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/7/4
 */
static int im_ctl_handle_add_volume_cmd(void* data)
{
    int ret = 0;
    ProtectVol vol;
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;
#ifdef SUPPORT_BACKUP
    struct im_volume *im_vol = NULL;
#endif

    IOMIRROR_INFO("enter im_ctl_handle_add_volume_cmd.");

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        ret = -1;
        goto out;
    }

    ret = copy_from_user(&vol, data, sizeof(vol));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy from user error.");
        ret = -1;
        goto out;
    }
    IOMIRROR_INFO("add_volume, vol_id=%llx, vol_path=%s.", *(long long *)vol.vol_id, vol.disk_path);

#ifdef SUPPORT_BACKUP
    if (IM_NO_BACKUP_MODE != pg->backup_status)
    {
        im_vol = im_add_volume(vol.vol_id, vol.disk_path,
                            0, pg->bitmap_granularity, &(pg->rq));
        if (NULL == im_vol)
        {
            IOMIRROR_ERR("im_add_volume failed, vol_path=%s.", vol.disk_path);
            return -1;
        }

        /* 加入本pg卷队列 */
        list_add_tail(&(im_vol->list1), &(pg->vols));
        pg->vol_num++;
        return 0;
    }
#endif

    ext_cmd.type = IM_PG_EXT_CMD_ADD_VOL;
    ext_cmd.info = &vol;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (0 == ext_cmd.ret)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
        IOMIRROR_ERR("pg add volume failed, vol_id=%llx, vol_path=%s.", *(long long *)vol.vol_id, vol.disk_path);
    }

    pg->ext_cmd = NULL;

out:
    return ret;
}


/**
 * Description : 动态删除卷
 *
 * Parameters  : data - 用户态传入待删除的卷
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/7/4
 */
static int im_ctl_handle_del_volume_cmd(void* data)
{
    int ret = 0;
    ProtectVol vol;
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;
#ifdef SUPPORT_BACKUP
    struct im_volume *im_vol = NULL;
    struct list_head *ptr = NULL;
#endif

    IOMIRROR_INFO("enter im_ctl_handle_del_volume_cmd.");

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        ret = -1;
        goto out;
    }

    ret = copy_from_user(&vol, data, sizeof(vol));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy from user error.");
        ret = -1;
        goto out;
    }
    IOMIRROR_INFO("del_volume, vol_id=%llx, vol_path=%s.", *(long long *)vol.vol_id, vol.disk_path);

#ifdef SUPPORT_BACKUP
    if (IM_NO_BACKUP_MODE != pg->backup_status)
    {
        /* 在保护组的卷队列中查找待删除卷 */
        list_for_each(ptr, &(pg->vols))
        {
            im_vol = list_entry(ptr, struct im_volume, list1);

            if (0 == memcmp(im_vol->id, vol.vol_id, VOL_ID_LEN))
            {
                break;
            }
            else
            {
                im_vol = NULL;
            }
        }

        /* 从filter中删除卷 */
        if (likely(NULL != im_vol))
        {
            list_del_init(&(im_vol->list1));
            im_del_volume(im_vol);
            im_vol = NULL;
            pg->vol_num--;
            return 0;
        }

        IOMIRROR_ERR("can NOT find the corresponding volume, volume_id=%llx.",
                      *(long long *)vol.vol_id);
        return -1;
    }
#endif

    ext_cmd.type = IM_PG_EXT_CMD_DEL_VOL;
    ext_cmd.info = &vol;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (0 == ext_cmd.ret)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
        IOMIRROR_ERR("pg del volume failed, vol_id=%llx, vol_path=%s.", *(long long *)vol.vol_id, vol.disk_path);
    }

    pg->ext_cmd = NULL;

out:
    return ret;
}

/**
 * Description : 修改卷信息
 *
 * Parameters  : data - 用户态传入待修改的卷
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : wangguitao
 * Date        : 2019-08-13
 */
static int im_ctl_handle_mod_volume_cmd(void* data)
{
    int ret = 0;
    ProtectVol vol;
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;

    IOMIRROR_INFO("enter im_ctl_handle_mod_volume_cmd.");

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        ret = -1;
        goto out;
    }

    ret = copy_from_user(&vol, data, sizeof(vol));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy from user error.");
        ret = -1;
        goto out;
    }
    IOMIRROR_INFO("mod_volume, old_vol_id=%llx, new_vol_id=%llx, vol_path=%s.", *(long long *)vol.old_vol_id, *(long long *)vol.vol_id, vol.disk_path);

    ext_cmd.type = IM_PG_EXT_CMD_MOD_VOL;
    ext_cmd.info = &vol;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (0 == ext_cmd.ret)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
        IOMIRROR_ERR("pg modify volume failed, old_vol_id=%llx, new_vol_id=%llx, vol_path=%s.", *(long long *)vol.old_vol_id, *(long long *)vol.vol_id, 
            vol.disk_path);
    }

    pg->ext_cmd = NULL;

out:
    return ret;
}


/**
 * Description : 被保护卷准备就绪，通知pg线程加卷
 *
 * Parameters  : data - 用户态传入待添加的卷
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/7/25
 */
static int im_ctl_handle_volume_ready_cmd(void* data)
{
    int ret = 0;
    ProtectVol vol;
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;
#ifdef SUPPORT_BACKUP
    struct im_pg_pending_vol *pending_vol = NULL;
    struct im_volume *im_vol = NULL;
    struct list_head *ptr = NULL;
#endif

    IOMIRROR_INFO("enter im_ctl_handle_volume_ready_cmd.");

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        ret = -1;
        goto out;
    }

    ret = copy_from_user(&vol, data, sizeof(vol));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy from user error.");
        ret = -1;
        goto out;
    }
    IOMIRROR_INFO("volume_ready, vol_id=%llx, vol_path=%s.", *(long long *)vol.vol_id, vol.disk_path);

#ifdef SUPPORT_BACKUP
    if (IM_NO_BACKUP_MODE != pg->backup_status)
    {
        /* 判断传入卷是否为pending volume */
        list_for_each(ptr, &(pg->pending_vols))
        {
            pending_vol = list_entry(ptr, struct im_pg_pending_vol, list);

            if (0 == strncmp(pending_vol->path, vol.disk_path, DISK_PATH_LEN))
            {
                break;
            }
            else
            {
                pending_vol = NULL;
            }
        }
        if (NULL == pending_vol)
        {
            IOMIRROR_ERR("can not find pending volume, vol_path=%s.", vol.disk_path);
            return -1;
        }

        /* 向filter中增加卷 */
        im_vol = im_add_volume(pending_vol->id, pending_vol->path,
                        pending_vol->sectors, pg->bitmap_granularity, &(pg->rq));
        if (NULL == im_vol)
        {
            IOMIRROR_ERR("im_add_volume failed, vol_path=%s.", pending_vol->path);
            return -1;
        }

        /* 加入本pg卷队列，从pending volumes队列中删除 */
        list_add_tail(&(im_vol->list1), &(pg->vols));
        pg->vol_num++;
        list_del_init(&(pending_vol->list));
        return 0;
    }
#endif

    ext_cmd.type = IM_PG_EXT_CMD_VOL_READY;
    ext_cmd.info = &vol;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (0 == ext_cmd.ret)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
        IOMIRROR_ERR("pg handle volume ready failed, vol_id=%llx, vol_path=%s.",
                      *(long long *)vol.vol_id, vol.disk_path);
    }

    pg->ext_cmd = NULL;

out:
    return ret;
}


#ifdef SUPPORT_BACKUP
static int cmd2bio(struct bio *dest_bio, struct im_cmd *cmd, int len, unsigned int *margin)
{
    int i, j, len_tmp, left, len_src = 0, len_dest = 0, *skip;
    unsigned char *src_data = NULL, *dest_data = NULL, *dest_base = NULL;
 
    skip = (int *)&dest_bio->bi_private;
    left = *skip;
 
    if(len < *margin)
        return -1;
    len -= *margin;
 
    for(i = 0; i < cmd->vcnt; i++) {
        if(*margin >= cmd->bvl[i].bv_len) {
            *margin -= cmd->bvl[i].bv_len;
            continue;
        }
 
        src_data = page_address(cmd->bvl[i].bv_page);
        len_src = cmd->bvl[i].bv_len - *margin;
        break;
    }
 
    for(j = 0; j < dest_bio->bi_vcnt; j++) {
        if(left > dest_bio->bi_io_vec[j].bv_len) {
            left -= dest_bio->bi_io_vec[j].bv_len;
            continue;
        }
 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
        dest_base = dest_data = kmap_atomic(dest_bio->bi_io_vec[j].bv_page);
#else
        dest_base = dest_data = kmap_atomic(dest_bio->bi_io_vec[j].bv_page, KM_USER0);
#endif
        //dest_base = dest_data = page_address(dest_bio->bi_io_vec[j].bv_page);
        len_dest = dest_bio->bi_io_vec[j].bv_len - left;
        break;
    }
 
    while( (i < cmd->vcnt) && (j < dest_bio->bi_vcnt) ) {
        len_tmp = (len_dest > len_src)? len_src : len_dest;
        len_tmp = (len > len_tmp)? len_tmp : len;
        memcpy(dest_data + left + dest_bio->bi_io_vec[j].bv_offset, src_data + *margin + cmd->bvl[i].bv_offset, len_tmp);
 
        *skip += len_tmp;
        src_data += len_tmp;
        dest_data += len_tmp;
 
        len -= len_tmp;
        len_src -= len_tmp;
        len_dest -= len_tmp;
 
        if(len_src == 0) {
            *margin = 0;
 
            i += 1;
            if(i == cmd->vcnt)
                break;
 
            src_data = page_address(cmd->bvl[i].bv_page);
            len_src = cmd->bvl[i].bv_len;
        }
 
        if(len == 0) {
            break;
        }
 
        if(len_dest == 0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)        
            kunmap_atomic(dest_base);
#else
            kunmap_atomic(dest_base, KM_USER0);
#endif
            left = 0;
 
            j += 1;
            if(j == dest_bio->bi_vcnt)
                break;
 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
            dest_base = dest_data = kmap_atomic(dest_bio->bi_io_vec[j].bv_page);
#else
            dest_base = dest_data = kmap_atomic(dest_bio->bi_io_vec[j].bv_page, KM_USER0);
#endif
            //dest_base = dest_data = page_address(dest_bio->bi_io_vec[j].bv_page);
            len_dest = dest_bio->bi_io_vec[j].bv_len;
        }
    }
 
    if(j < dest_bio->bi_vcnt)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)        
        kunmap_atomic(dest_base);
#else
        kunmap_atomic(dest_base, KM_USER0);
#endif
 
    return 0;
}
 
static struct im_cmd *cowdata_read(struct im_volume *vol, uint64_t offset)
{
    struct im_cmd *cmd;
    struct completion completion;
 
    cmd = im_alloc_cmd4snapio(vol, offset);
    if (NULL == cmd)
    {
        return NULL;
    }
 
    init_completion(&completion);
    cmd->completion = &completion;
    down(&(vol->rq->lock));
    list_add_tail(&(cmd->list), &(vol->rq->head));
    vol->rq->count++;
    up(&(vol->rq->lock));
 
    wake_up_interruptible(&(vol->rq->wq));
    if (wait_for_completion_timeout(&completion, IM_BACKUP_IO_TIMEOUT) == 0)
    {
        /* 失败的情况不需要释放cmd */
        return NULL;
    }
 
    return cmd;
}
 
static struct im_cmd *diskdata_read(struct im_volume *vol, uint64_t offset)
{
    struct im_cmd *cmd;
    struct completion completion;
    
    cmd = im_alloc_cmd4diskio(vol, offset, 1);
    if (NULL == cmd)
    {
        return NULL;
    }
 
    init_completion(&completion);
    cmd->completion = &completion;
    down(&(vol->rq->lock));
    list_add_tail(&(cmd->list), &(vol->rq->head));
    vol->rq->count++;
    up(&(vol->rq->lock));
 
    wake_up_interruptible(&(vol->rq->wq));
    if (wait_for_completion_timeout(&completion, IM_BACKUP_IO_TIMEOUT) == 0)
    {
        im_cmd_free(cmd);
        return NULL;
    }
 
    return cmd;
}
 
static int readsnapdata(struct im_volume *vol, struct bio *bio)
{
    struct im_cmd *cmd;
    uint64_t pos, start, size;
    int ret, margin, blocksize = 1U << vol->bitmap->granularity;
    void *private = bio->bi_private;
 
    start = bio->bi_sector;
    margin = do_div(start, blocksize) * IM_SECTOR_SIZE;
    start *= blocksize; // align start
    ret = size = bio->bi_size + margin;
    size = (size + IM_SECTOR_SIZE - 1) / IM_SECTOR_SIZE;
 
    bio->bi_private = NULL;
    for (pos = start; pos < start + size; pos += blocksize)
    {
        if (IsBitmapBitSet(vol->bitmap_snapshot, pos))
        {
            cmd = cowdata_read(vol, pos);
        }
        else
        {
            cmd = diskdata_read(vol, pos);
            
            if (IM_BACKUP_SNAPSHOT != vol->rq->pg->backup_status)
            {
                im_cmd_free(cmd);
                ret = -EPERM;
                break;
            }
            
            if (IsBitmapBitSet(vol->bitmap_snapshot, pos))
            {
                im_cmd_free(cmd);
                cmd = cowdata_read(vol, pos);
            }
        }
 
        if (cmd == NULL)
        {
            vol->rq->pg->backup_status = IM_BACKUP_CBT;
            ret = -ENOMEM;
            break;
        }
        
        if (IM_BACKUP_SNAPSHOT != vol->rq->pg->backup_status)
        {
            im_cmd_free(cmd);
            ret = -EPERM;
            break;
        }
        
        if (cmd2bio(bio, cmd, (ret > blocksize * IM_SECTOR_SIZE)? blocksize * IM_SECTOR_SIZE : ret, &margin) != 0)
        {
            im_cmd_free(cmd);
            ret = -EPERM;
            break;
        }
 
        im_cmd_free(cmd);
        if(ret > blocksize * IM_SECTOR_SIZE) {
            ret -= blocksize * IM_SECTOR_SIZE;
        } else {
            ret = 0;
        }
    }
 
    bio->bi_private = private;
    if(ret < 0)
        return ret;
    return bio->bi_size - ret;
}
 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
#define im_type void
#define im_ret
#define im_err
#else
#define im_type int
#define im_ret 0
#define im_err -1
#endif
 
static im_type snapdev_request(struct request_queue* prq, struct bio *bio)
{
    int ret = 0;
    struct im_volume *vol;
 
    vol = (struct im_volume *)(bio->bi_bdev->bd_disk->private_data);
    if( (NULL == vol) || (IM_BACKUP_SNAPSHOT != vol->rq->pg->backup_status) || 
        (WRITE == bio_data_dir(bio)) || (readsnapdata(vol, bio) != bio->bi_size) )
    {
        ret = -EIO;
        vol->rq->pg->backup_status = IM_BACKUP_CBT;
    }
 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
    bio_endio(bio, bio->bi_size, ret);
#else
    bio_endio(bio, ret);
#endif
    return im_ret;
}
 
static int snapdev_open_blk(struct block_device *bdev, fmode_t mode)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32))
    int ret;
#endif
    struct im_volume *vol;
 
    if( (NULL == bdev) || (NULL == bdev->bd_disk) )
        return -1;
 
        
    vol = bdev->bd_disk->private_data;
    if( (vol == NULL) || (vol->rq->pg->backup_status != IM_BACKUP_SNAPSHOT) ) {
        return -1;
    }
    
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    bdev->bd_inode_backing_dev_info = NULL;
#endif
 
    set_device_ro(bdev, 1);
    if(bdev->bd_disk->queue) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32))
        ret = blk_get_queue(bdev->bd_disk->queue);
#else
        if (likely(!test_bit(QUEUE_FLAG_DEAD, &bdev->bd_disk->queue->queue_flags))) {
            kobject_get(&bdev->bd_disk->queue->kobj);
        }
#endif
    }
 
    return 0;
}
 
static im_type snapdev_release_blk(struct gendisk *disk, fmode_t mode)
{
    struct im_volume *vol;
 
    if( disk == NULL )
        return im_err;
 
    vol = disk->private_data;
    if(vol == NULL) {
        return im_err;
    }
 
    if(disk->queue) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32))
        blk_put_queue(disk->queue);
#else
        if (likely(!test_bit(QUEUE_FLAG_DEAD, &disk->queue->queue_flags))) {
            kobject_put(&disk->queue->kobj);
        }
#endif
    }
 
    return im_ret;
}
 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
static int snapdev_open(struct inode *inode, struct file *file)
{
    struct block_device *bdev;
    bdev = inode->i_bdev;
    return snapdev_open_blk(bdev, 1);
}
 
static int snapdev_release(struct inode *inode, struct file *file)
{
    struct gendisk *disk;
    disk = inode->i_bdev->bd_disk;
    return snapdev_release_blk(disk, 1);
}
#else
#define snapdev_open snapdev_open_blk
#define snapdev_release snapdev_release_blk
#endif
 
static struct block_device_operations snapdev_ops = {
    .owner =    THIS_MODULE,
    .open =     snapdev_open,
    //.ioctl =  snapdev_ioctl,
    .release =  snapdev_release,
};
 
static int set_snapdev(struct im_volume *vol)
{
    struct gendisk *disk;
    struct request_queue *queue = blk_alloc_queue(GFP_KERNEL);
 
    if(queue == NULL) {
        return -1;
    }
 
    disk = alloc_disk(1);
    if(disk == NULL) {
        blk_cleanup_queue(queue);
        return -1;
    }
 
    disk->major = ctldev.blkno;
    disk->first_minor = vol->volume_minor;
    sprintf(disk->disk_name, IM_SNAPSHOT_DEV_NAME"%d", vol->volume_minor);
    disk->private_data = vol;
 
    disk->fops = &snapdev_ops;
    disk->queue = queue;
    set_capacity(disk, vol->sectors);
 
    if(vol->snap_disk) {
        del_gendisk(vol->snap_disk);
        put_disk(vol->snap_disk);
    }
    vol->snap_disk = disk;
 
    if(vol->snap_queue) {
        blk_cleanup_queue(vol->snap_queue);
    }
    vol->snap_queue = queue;
 
    blk_queue_make_request(queue, snapdev_request);
 
    add_disk(disk);
    return 0;
}
 
static void reset_snapdev(struct im_volume *vol)
{
    if(vol->snap_disk) {
        del_gendisk(vol->snap_disk);
        put_disk(vol->snap_disk);
        vol->snap_disk = NULL;
    }
 
    if(vol->snap_queue) {
        blk_cleanup_queue(vol->snap_queue);
        vol->snap_queue = NULL;
    }
}
 
/**
 * Description : 解除备份一致性快照
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
static int im_ctl_handle_remove_snapshot_cmd(void* data)
{
    int            ret     = 0;
    //char          *snap_id = NULL;
    //HBitmap        *bitmap_swap = NULL;
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;
    struct im_pg  *pg      = NULL;
    //struct im_cmd *cmd     = NULL;
    struct im_ctl_backup_remove_snapshot info;
 
    IOMIRROR_INFO("enter im_ctl_handle_remove_snapshot_cmd.");
 
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }
 
    if (IM_NO_BACKUP_MODE == pg->backup_status)
    {
        IOMIRROR_ERR("not in backup mode.");
        return -ENOENT;
    }
/*
    if (IM_BACKUP_INIT == pg->backup_status)
    {
        IOMIRROR_ERR("not have snapshot.");
        return -ENOENT;
    }
*/
    ret = copy_from_user(&info, data, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
    IOMIRROR_INFO("got snap_id from userspace, snap_id=%s.", info.snap_id);
 
    if (IM_BACKUP_SNAPSHOT != pg->backup_status)
    {
        if (info.is_failed == 0)
        {
            IOMIRROR_ERR("snapshot failed already.");
            return -EINVAL;
        }
        
        if (*info.snap_id)
        {
            // check snap_id for normal shutdown case
            if(strncmp(pg->snap_id, info.snap_id, IM_SNAP_ID_LEN))
                return -EFAULT;
            return 0;
        }
    }
    else
    {       
        if (info.is_failed == 0)
        {
            if ( (info.snap_id == 0) || (strncmp(info.snap_id, pg->snap_id, IM_SNAP_ID_LEN) == 0) )
            {
                IOMIRROR_ERR("wrong snap id.");
                return -EINVAL;
            }
            
            strncpy(pg->snap_id, info.snap_id, IM_SNAP_ID_LEN);
        }
 
        pg->backup_status = IM_BACKUP_CBT; /* 切换至CBT状态 */
    }
 
    if (down_trylock(&(ctldev.sem))) /* sub count which added during taking snapshot */
    {
        return -EAGAIN;
    }
 
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        IOMIRROR_INFO("reset snapdev, vol path=%s.", vol->path);
        reset_snapdev(vol);
 
        if (NULL != vol->bitmap_original)
        {
            if (*info.snap_id) // 设置了snap_id，表示备份成功，清除bitmap_original
            {           
                BitmapFree(vol->bitmap_original, BitmapFreeFunc);
                vol->bitmap_original = NULL;
            }
        }
        
        if (NULL != vol->bitmap_snapshot)
        {
            BitmapFree(vol->bitmap_snapshot, BitmapFreeFunc);
            vol->bitmap_snapshot = NULL;
        }
 
        if (NULL != vol->bitmap_temp)
        {
            BitmapFree(vol->bitmap_temp, BitmapFreeFunc);
            vol->bitmap_temp = NULL;
        }
    }
    
    return 0;
}
 
/**
 * Description : 为备份创建一致性快照
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
static int im_ctl_handle_take_snapshot_cmd(void* data)
{
    int            ret     = 0;
    //char          *snap_id = NULL;
    OM_BITMAP        *bitmap_swap = NULL;
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;
    struct im_pg  *pg      = NULL;
    //struct im_cmd *cmd     = NULL;
    struct im_ctl_backup_take_snapshot info;
 
    IOMIRROR_INFO("enter im_ctl_handle_take_snapshot_cmd.");
 
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }
 
    if (IM_NO_BACKUP_MODE == pg->backup_status)
    {
        IOMIRROR_ERR("not in backup mode.");
        return -ENOENT;
    }
 
    if (IM_BACKUP_SNAPSHOT == pg->backup_status)
    {
        memset(&info, 0, sizeof(info));
        strncpy(info.snap_id, pg->snap_id, IM_SNAP_ID_LEN);
        info.bitmap_granularity = pg->bitmap_granularity;
        info.max_vol_minor = pg->cur_volume_minor;
 
        list_for_each(ptr, &(pg->vols))
        {
            vol = list_entry(ptr, struct im_volume, list1);
            info.vol_devno[vol->volume_minor] = vol->bd_dev;
        }
 
        ret = copy_to_user(data, &info, sizeof(info));
        if (ret < 0)
        {
            IOMIRROR_ERR("copy_from_user failed.");
            return ret;
        }
        return -EEXIST;
    }
 
    ret = copy_from_user(&info, data, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
    IOMIRROR_INFO("got snap_id from userspace, snap_id=%s.", info.snap_id);
 
    if ( (*info.snap_id) && (strncmp(info.snap_id, pg->snap_id, IM_SNAP_ID_LEN)) )
    {
        IOMIRROR_ERR("wrong snapshot id.");
        return -EINVAL;
    }
 
    /* 为位图切换预分配空间 */
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        reset_snapdev(vol);
/*
        if (NULL != vol->bitmap_original)
        {
            hbitmap_merge(vol->bitmap, vol->bitmap_original);
            BitmapFree(vol->bitmap_original);
            vol->bitmap_original = NULL;
        }
*/
        if (NULL != vol->bitmap_snapshot)
        {
            BitmapFree(vol->bitmap_snapshot, BitmapFreeFunc);
            vol->bitmap_snapshot = NULL;
        }
 
        if (NULL != vol->bitmap_temp)
        {
            BitmapFree(vol->bitmap_temp, BitmapFreeFunc);
            vol->bitmap_temp = NULL;
        }
 
        vol->bitmap_snapshot = BitmapAlloc(vol->sectors, pg->bitmap_granularity, BitmapAllocFunc, BitmapFreeFunc);
        if (NULL == vol->bitmap_snapshot)
        {
            IOMIRROR_ERR("BitmapAlloc for bitmap_snapshot failed.");
            return -ENOMEM;
        }
 
        vol->bitmap_temp = BitmapAlloc(vol->sectors, pg->bitmap_granularity, BitmapAllocFunc, BitmapFreeFunc);
        if (NULL == vol->bitmap_temp)
        {
            IOMIRROR_ERR("BitmapAlloc for bitmap_temp failed.");
            return -ENOMEM;
        }
    }
 
    /* 生成快照虚拟块设备 */
    info.bitmap_granularity = pg->bitmap_granularity;
    info.max_vol_minor = pg->cur_volume_minor;
    memset(info.vol_devno, 0, sizeof(info.vol_devno));
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        info.vol_devno[vol->volume_minor] = vol->bd_dev;
        
        if (set_snapdev(vol) < 0)
        {
            IOMIRROR_ERR("set_snapdev failed.");
            return ret;
        }
    }
 
    ret = copy_to_user(data, &info, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
 
    /* 正式切换位图 */
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
 
        bitmap_swap = vol->bitmap;
        vol->bitmap = vol->bitmap_temp;
        vol->bitmap_temp = bitmap_swap;
    }
 
    //if (IM_BACKUP_INIT == pg->backup_status)
    //{
        up(&(ctldev.sem)); // add more one process to call ioctl for remove snapshot
    //}
    pg->backup_status = IM_BACKUP_SNAPSHOT; /* 切换至快照状态 */
 
    list_for_each(ptr, &(pg->vols))
    {
        if (NULL != vol->bitmap_original)
        {
            MergeBitmap(vol->bitmap_original, vol->bitmap_temp, BitmapAllocFunc, BitmapFreeFunc);
            BitmapFree(vol->bitmap_temp, BitmapFreeFunc);
        }
        else
        {
            vol->bitmap_original = vol->bitmap_temp;
        }
        vol->bitmap_temp = NULL;
    }
 
    return 0;
}
#endif


static int write_bitmap_data(void *targetdev, struct im_ctl_bitmap_extent *bitmap_extent, OM_BITMAP *bitmap_increment, 
    unsigned int bitmap_len, int skip)
{
    unsigned char *p, *bitmap, buf[IM_SECTOR_SIZE];
    unsigned long long offset = 0;
    int i = 0, ret = -1, length = 0, pos = 0;
    
    bitmap = vmalloc(bitmap_len);
    if(bitmap == NULL) {
        IOMIRROR_ERR("vmalloc bitmap failed.\n");
        return -1;
    }
    memcpy(bitmap, bitmap_increment->buffer, bitmap_len);
    
    // skip是为了调过header占有的位置以及前面bitmap占的位置
    // 此处为了计算后面写数据的开始偏移量，因为length可能还是有可能达不到skip的长度
    // 优先保证数据的写入，防止不够写的场景，但实际上还是可能存在不够写的场景，
    // 如果后面不够，则直接会写失败
    while(skip >= 0) {
        if(bitmap_extent[i].length == -1) {
            IOMIRROR_ERR("block file format error.\n");
            goto err;
        }
        
        if(bitmap_extent[i].length > skip)
            break;
        
        skip -= bitmap_extent[i].length;
        i += 1;
    }

    // 根据skip调整bitmap写入的位置
    offset = bitmap_extent[i].offset + skip;
    length = bitmap_extent[i].length - skip;

    // 按照sector维度依次写入bitmap到磁盘
    while(pos < bitmap_len) {
        if(bitmap_extent[i].length == -1) {
            IOMIRROR_ERR("bitmap range format error.\n");
            goto err;
        }
        
        p = bitmap + pos;
        if((bitmap_len - pos) < IM_SECTOR_SIZE) {
            memset(buf, 0, IM_SECTOR_SIZE);
            memcpy(buf, p, bitmap_len - pos);
            p = buf;
        }

        IOMIRROR_INFO("pos=%u, offset=%llu, p=%02X, start=%llu.", pos, offset, *p, offset / IM_SECTOR_SIZE);
        if(writebitmap_to_disk(targetdev, p, offset / IM_SECTOR_SIZE) != 0) {
            IOMIRROR_ERR("writebitmap_to_disk error.\n");
            goto err;
        }
 
        pos += IM_SECTOR_SIZE;
        length -= IM_SECTOR_SIZE;
        if(length == 0) {
            i += 1;
            offset = bitmap_extent[i].offset;
            length = bitmap_extent[i].length;
        } else {
            offset += IM_SECTOR_SIZE;
        }
    }
 
    ret = 0;
err:;
    vfree(bitmap);
    return ret;
}


int save_bitmap(void)
{
    int bitmap_len, ret = -1, pos = 0;
    unsigned char * header = NULL, *desc = NULL;
    struct block_device *targetdev;
    struct im_ctl_bitmap_extent *bitmap_extent;
    struct im_pg *pg = NULL;
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;
    PPERS_DATA_HEADER data_header = NULL;
    PPERS_VOL_BITMAP_DESC desc_data = NULL;
    OM_BITMAP temp_bitmap;
    unsigned long vol_count = 0;
    unsigned long bitmap_offset =0;
 
    IOMIRROR_INFO("enter save_bitmap.");
    
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -1;
    }
 
    if(pg->dev_savebitmap == 0) {
        IOMIRROR_ERR("dev_savebitmap is null.");	
        return 0;
    }
 
    if(pg->bitmap_extent == NULL) {
        IOMIRROR_ERR("bitmap_extent is null.");
        return 0;
    }
    bitmap_extent = (struct im_ctl_bitmap_extent *)pg->bitmap_extent;
 
    if(bitmap_extent[0].length <= IM_SECTOR_SIZE) {
        IOMIRROR_ERR("bitmap range format is wrong %d.\n", bitmap_extent[0].length);
        return -1;
    }
 
    while(bitmap_extent[pos].length != -1) {
        if(bitmap_extent[pos].offset != (bitmap_extent[pos].offset / IM_SECTOR_SIZE * IM_SECTOR_SIZE)) {
            IOMIRROR_ERR("bitmap range format is wrong %lld.\n", bitmap_extent[pos].offset);
            return -1;
        }
 
        if(bitmap_extent[pos].length != (bitmap_extent[pos].length / IM_SECTOR_SIZE * IM_SECTOR_SIZE)) {
            IOMIRROR_ERR("bitmap range format is wrong %d.\n", bitmap_extent[pos].length);
            return -1;
        }
        pos += 1;
    } 

    // get hwsnap_bitmap device
    targetdev = bdget(pg->dev_savebitmap);
    if(targetdev == NULL)
    {
        IOMIRROR_ERR("bdget bitmap device failed.");
        return -1;
    }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28) || LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 38))
    ret = blkdev_get(targetdev, FMODE_READ, 0);
#else
    ret = blkdev_get(targetdev, FMODE_READ);
#endif
    if(ret < 0)
    {
        IOMIRROR_ERR("blkdev_get bitmap device failed.");
        return -1;
    }

    header = vmalloc(PER_HEADER_SECTION_SIZE);
    if (!header)
    {
        IOMIRROR_ERR("vmalloc failed.");
        ret = -1;
        goto err;
    }
    memset(header, 0 , PER_HEADER_SECTION_SIZE);
    
    desc = vmalloc(PER_DESCRIPTOR_SECTION_SIZE);
    if (!desc)
    {
        IOMIRROR_ERR("vmalloc failed.");
        ret = -1;
        goto err;
    }
    memset(desc, 0, PER_DESCRIPTOR_SECTION_SIZE);

    data_header = (PPERS_DATA_HEADER)header; 
    memcpy(data_header->magic, PER_DATA_MAGIC, sizeof(data_header->magic));
    data_header->start_safe_flag = 1;
    data_header->desc_offset = PER_HEADER_SECTION_SIZE;
    data_header->bitmap_offset = PER_HEADER_SECTION_SIZE + PER_DESCRIPTOR_SECTION_SIZE;

    desc_data = (PPERS_VOL_BITMAP_DESC)desc;
    desc_data->disk_count = pg->vol_num;
    desc_data->granularity = pg->bitmap_granularity;
 
    IOMIRROR_INFO("save_bitmap, offset=%llu, len=%u, major=%d, minor=%d.", bitmap_extent[0].offset, bitmap_extent[0].length, 
                   MAJOR(pg->dev_savebitmap), MINOR(pg->dev_savebitmap));
    
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        bitmap_len = (vol->sectors + (1ULL << pg->bitmap_granularity) - 1) >> pg->bitmap_granularity;
        bitmap_len = (bitmap_len + BITS_PER_LONG - 1) >> BITS_PER_LEVEL;

        memset(desc_data->bitmap_info[vol_count].vol_info.disk_name, 0, PER_DISK_NAME_LEN);
        memcpy(desc_data->bitmap_info[vol_count].vol_info.disk_name, vol->path, PER_DISK_NAME_LEN);
        memcpy(desc_data->bitmap_info[vol_count].vol_info.vol_id, vol->id, VOL_ID_LEN);
        
        desc_data->bitmap_info[vol_count].bitmap_size = bitmap_len * sizeof(bitmap_operator_t);
        desc_data->bitmap_info[vol_count].logical_offset = bitmap_offset;
        bitmap_offset += (desc_data->bitmap_info[vol_count].bitmap_size + IM_SECTOR_SIZE - 1) / IM_SECTOR_SIZE * IM_SECTOR_SIZE;

        if (vol->bitmap)
        {
            IOMIRROR_INFO("vol bits = %llu.", GetBitmapCount(vol->bitmap));
            if (vol->bitmap_original) {
                // im_pg_merge_bitmap中bit_count为bitmap中bit为1的数量，需要转换扇区的数量
                im_pg_merge_bitmap(vol, (unsigned char *)vol->bitmap_original->buffer, bitmap_len * sizeof(bitmap_operator_t), 
                    vol->bitmap_original->count >> pg->bitmap_granularity);
            }

            desc_data->bitmap_info[vol_count].bitmap_count = GetBitmapCount(vol->bitmap);

            if (write_bitmap_data(targetdev, bitmap_extent, vol->bitmap, desc_data->bitmap_info[vol_count].bitmap_size, data_header->bitmap_offset + desc_data->bitmap_info[vol_count].logical_offset) < 0)
            {
                IOMIRROR_ERR("writebitmap_to_disk error.\n");
                goto err;
            }
        }

        vol_count ++;
    }

    // write bitmap descriptor struct 
    temp_bitmap.buffer = (bitmap_operator_t *)desc;
    if(write_bitmap_data(targetdev, bitmap_extent, &temp_bitmap, PER_DESCRIPTOR_SECTION_SIZE, data_header->desc_offset) != 0) {
        IOMIRROR_ERR("writebitmap_to_disk of descriptor error.\n");
        goto err;
    }

    // write bitmap header struct
    temp_bitmap.buffer= (bitmap_operator_t *)header;
    if(write_bitmap_data(targetdev, bitmap_extent, &temp_bitmap, PER_HEADER_SECTION_SIZE, 0) != 0) {
        IOMIRROR_ERR("writebitmap_to_disk of headererror.\n");
        goto err;
    }
 
    ret = 0;

    IOMIRROR_INFO("save bitmap succ.");
err:
    blkdev_put(targetdev, FMODE_READ);
    if (desc)
    {
        vfree(desc);
        desc = NULL;
    }
    if (header)
    {
        vfree(header);
        header = NULL;
    }
    return ret;
}

// 写配置文件到磁盘，暂时不会用到
int read_data_from_disk(struct block_device *targetdev, unsigned char *buffer, int conf_size, 
        struct im_ctl_bitmap_extent *configfile_extent)
{
    int total_num = 0, extent_num = 0, i = 0;
    unsigned char buf[IM_SECTOR_SIZE];
    unsigned long long sec_num = 0;

    IOMIRROR_INFO("enter read_config_data_from_disk, conf_size %d, offset %llu, length %u, major %X.", 
        conf_size, configfile_extent[0].offset, configfile_extent[0].length, targetdev->bd_dev);
    
    // 保证配置文件内容读取完成，因为文件的磁盘还有是分段，需要从分段读取文件所有的内容
    // total_read_num : 已经从磁盘读取的字节数量
    // conf_offset : 配置结构的读取偏移
    // extent_read_num: 当前读取的字节数量
    while(conf_size > total_num)
    {
        // 已经用完了最后的磁盘，还没有读完conf，则退出
        if (configfile_extent[i].offset == -1)
        {
            IOMIRROR_ERR("read conf failed, config file extent parameter is invalid.");
            return -1;
        }
        
        sec_num = (configfile_extent[i].offset + extent_num) / IM_SECTOR_SIZE;
        IOMIRROR_INFO("sec_num=%llu", sec_num);
        // 从磁盘读取文件内容
        if(readbitmap_from_disk(targetdev, buf, sec_num) != 0) 
        {
            IOMIRROR_ERR("read config file from disk error, i=%d.\n", i);
            return -1;
        }
        
        total_num += IM_SECTOR_SIZE;
        extent_num += IM_SECTOR_SIZE;

        // 如果单个extent达到最大长度，则开始使用后面的磁盘扇区
        if (extent_num >= configfile_extent[i].length)
        {
            i++;
            extent_num = 0;
        }

        // 往结构体中复制数据， 如果不够一个扇区，只复制部分数据
        if (conf_size < total_num)
        {
            memcpy(buffer, buf, conf_size);
        }
        else if (conf_size - total_num >= IM_SECTOR_SIZE)
        {
            memcpy(buffer, buf, IM_SECTOR_SIZE);
        }
        else
        {
            memcpy(buffer, buf, conf_size - total_num);
        }

        buffer += IM_SECTOR_SIZE;
    }
    
    return 0;
}

int write_data_from_disk(struct block_device *targetdev, unsigned char *buffer, int conf_size, 
        struct im_ctl_bitmap_extent *configfile_extent)
{
    int total_num = 0, extent_num = 0, i = 0;
    unsigned char buf[IM_SECTOR_SIZE] = {0};
    unsigned long long sec_num = 0;
    
    IOMIRROR_INFO("enter write_config_data_from_disk, conf_size %d.", conf_size);

    while (conf_size > total_num)
    {
        // 已经用完了最后的磁盘扇区，还没有写完conf，则退出
        if (configfile_extent[i].offset == -1)
        {
            IOMIRROR_ERR("read conf failed, config file extent parameter is invalid.");
            return -1;
        }

        memset(buf, 0, sizeof(buf));
        if (conf_size - total_num > IM_SECTOR_SIZE)
        {
            memcpy(buf, buffer, IM_SECTOR_SIZE);
        }
        else
        {
            memcpy(buf, buffer, conf_size - total_num);
        }

        sec_num = (configfile_extent[i].offset + extent_num) / IM_SECTOR_SIZE;
        IOMIRROR_INFO("buf=%x, start=%llu.", *buf, sec_num);
        // 写配置文件内容到磁盘中
        if(writebitmap_to_disk(targetdev, buf, sec_num) != 0) 
        {
            IOMIRROR_ERR("write config file to disk error, i=%d.\n", i);
            return -1;
        }

        total_num += IM_SECTOR_SIZE;
        buffer += IM_SECTOR_SIZE;
        extent_num += IM_SECTOR_SIZE;
        
        // 如果单个extent达到最大长度，则开始使用后面的磁盘扇区
        if (extent_num >= configfile_extent[i].length)
        {
            i++;
            extent_num = 0;
        }
    }

    return 0;
}

int save_configfile_data(void)
{
    int ret = -1, pos = 0;
    struct block_device *targetdev;
    struct im_config_variable *conf = NULL;
    struct im_config_variable *conf1 = NULL;
    struct im_pg *pg = NULL;
    size_t size;
    struct im_ctl_bitmap_extent *configfile_extent;
 
    IOMIRROR_INFO("enter save_configfile_data.");
    
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -1;
    }

    if (pg->dev_saveconfigfile == 0) {
        IOMIRROR_ERR("dev_saveconfigfile is null.");
        return 0;
    }
 
    if (pg->configfile_extent == NULL) {
        IOMIRROR_ERR("configfile_extent is null.");
        return 0;
    }

    // check extent parameter valid
    configfile_extent = (struct im_ctl_bitmap_extent *)pg->configfile_extent;
     if(configfile_extent[0].length <= IM_SECTOR_SIZE) {
        IOMIRROR_ERR("config file extent range format is wrong %d.\n", configfile_extent[0].length);
        return -1;
    }

    // check if offset and length is aligned
    while(configfile_extent[pos].length != -1) {
        if(configfile_extent[pos].offset != (configfile_extent[pos].offset / IM_SECTOR_SIZE * IM_SECTOR_SIZE)) {
            IOMIRROR_ERR("config file exten range format is wrong %lld.\n", configfile_extent[pos].offset);
            return -1;
        }
 
        if(configfile_extent[pos].length != (configfile_extent[pos].length / IM_SECTOR_SIZE * IM_SECTOR_SIZE)) {
            IOMIRROR_ERR("config file exten range format is wrong %d.\n", configfile_extent[pos].length);
            return -1;
        }
        pos += 1;
    }

    targetdev = bdget(pg->dev_saveconfigfile);
    if(targetdev == NULL)
    {
        IOMIRROR_ERR("bdget config file device failed.");
        return -1;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28) || LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 38))
    ret = blkdev_get(targetdev, FMODE_READ, 0);
#else
    ret = blkdev_get(targetdev, FMODE_READ);
#endif
    if(ret < 0)
    {
        IOMIRROR_ERR("blkdev_get config file device failed.");
        return -1;
    }

    size = sizeof(struct im_config_variable);
    conf = (struct im_config_variable *)kzalloc(size, GFP_KERNEL);
    if (NULL == conf)
    {
        IOMIRROR_ERR("kzalloc for conf variable failed.");
        goto err;
    }

    // 设置最新的flush time值
    conf->cbt_flush_times = pg->cbt_flush_times;
    conf->cbt_flush_times_done = pg->cbt_flush_times_done;

    // 将conf的内容重新保存到磁盘中
    ret = write_data_from_disk(targetdev, (unsigned char *)conf, size, configfile_extent);
    if (0 != ret)
    {
        IOMIRROR_ERR("write config to disk failed.");
        goto err;
    }

    conf1 = (struct im_config_variable *)kzalloc(size, GFP_KERNEL);
    if (NULL == conf1)
    {
        IOMIRROR_ERR("kzalloc for conf1 failed.");
        goto err;
    }
    // 从配置文件中读取数据
    ret = read_data_from_disk(targetdev, (unsigned char *)conf1, size, configfile_extent);
    if (0 != ret)
    {
        IOMIRROR_ERR("read config from disk failed.");
        kfree(conf1);
        goto err;
    }

    IOMIRROR_INFO("flushtime %llu, flushdone %llu.", conf1->cbt_flush_times, conf1->cbt_flush_times_done);
    kfree(conf1);
    
    ret = 0;
err:;
    blkdev_put(targetdev, FMODE_READ);
    if (conf)
    {
        kfree(conf);
    }
    return ret;
}

int reset_bitmap_header(dev_t dev_savebitmap, uint64_t offset)
{
    int ret = -1;
    unsigned char header[IM_SECTOR_SIZE];
    struct block_device *targetdev;
 
    IOMIRROR_INFO("enter reset_bitmap_header, offset=%lld.", offset);
 
    targetdev = bdget(dev_savebitmap);
    if(targetdev == NULL)
    {
        IOMIRROR_ERR("bdget failed.");
        return -1;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28) || LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 38))
    ret = blkdev_get(targetdev, FMODE_READ, 0);
#else
    ret = blkdev_get(targetdev, FMODE_READ);
#endif
    if(ret < 0)
    {
        IOMIRROR_ERR("blkdev_get failed.");
        return -1;
    }

    ret = -1;
    // 靠靠?"%s:%d,%d,%llu\n", vol->path, skip, bitmap_len, hbitmap_count(vol->bitmap))
    if(readbitmap_from_disk(targetdev, header, offset / IM_SECTOR_SIZE) != 0) 
    {
        IOMIRROR_ERR("writebitmap_to_disk error.\n");
        goto err;
    }

    if (strncmp(header, PER_DATA_MAGIC, strlen(PER_DATA_MAGIC)) != 0)
    {
        IOMIRROR_ERR("no found reset_bitmap_header, %llx.", *(uint64_t *)header);
        goto err;
    }
        
    if (header[sizeof(PER_DATA_MAGIC)-1] == 0) {
        IOMIRROR_ERR("the bitmap header[%u] is %u, invalid bitmap.", (unsigned int)sizeof(PER_DATA_MAGIC) - 1, header[sizeof(PER_DATA_MAGIC)-1]);
        goto err;
    }

    IOMIRROR_INFO("found bitmap header, the header[%u] is %u.", (unsigned int)sizeof(PER_DATA_MAGIC) - 1, header[sizeof(PER_DATA_MAGIC)-1]);

    /*
    // 靠tagPERS_DATA_HEADER靠start_safe_flag靠縝itmap靠靠靠
    memset(header + strlen(PER_DATA_MAGIC), 0, 1);
    if(writebitmap_to_disk(targetdev, header, offset / IM_SECTOR_SIZE) != 0)
    {
        IOMIRROR_ERR("writebitmap_to_disk error.\n");
        goto err;
    }
    IOMIRROR_INFO("write reset_bitmap_header.");
    */

    ret = 0;
err:;
    blkdev_put(targetdev, FMODE_READ);
    return ret;
}

static int im_ctl_bitmap_set_bitmap_extent(void* data)
{
    int ret = 0, i = 0;
    struct im_pg  *pg      = NULL;
    struct im_ctl_bitmap_extent_setting setting;
    struct im_ctl_bitmap_extent *bitmap_extent, *temp;
 
    IOMIRROR_INFO("enter im_ctl_bitmap_set_bitmap_extent.");
 
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }

    if (pg->state == IM_PG_STATE_VERIFY) {
        IOMIRROR_ERR("IN_PG_STATE_VERIFIY mode don't save bitmap.");
        return -EPERM;    
    } 

    ret = copy_from_user(&setting, data, sizeof(setting));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
 
    if ( (0 == setting.extent_num) || (0 == setting.dev_major) )
    {
        IOMIRROR_ERR("parameter is wrong.");
        return -EINVAL;;
    }

    // 为什么申请内存要多申请一块内存,最后一块设置为-1，后面可以进行判断数组结束
    bitmap_extent = vmalloc(sizeof(struct im_ctl_bitmap_extent) * (setting.extent_num + 1));
    if (NULL == bitmap_extent)
    {
        IOMIRROR_ERR("vmalloc for bitmap_extent failed.");
        return -ENOMEM;
    }
    memset(bitmap_extent, 0, sizeof(struct im_ctl_bitmap_extent) * (setting.extent_num + 1));
    
    bitmap_extent[setting.extent_num].offset = -1;
    bitmap_extent[setting.extent_num].length = -1;
    ret = copy_from_user(bitmap_extent, setting.data, sizeof(struct im_ctl_bitmap_extent) * setting.extent_num);
    if (ret < 0)
    {
        vfree(bitmap_extent);
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
    
    temp = pg->bitmap_extent;
    pg->bitmap_extent = bitmap_extent;
    pg->dev_savebitmap = MKDEV(setting.dev_major, setting.dev_minor);

    IOMIRROR_INFO("boot major %u, monir %u, extent_num %u.", setting.dev_major, setting.dev_minor, setting.extent_num);
    for (i = 0; i < setting.extent_num; ++i)
    {
        IOMIRROR_INFO("offset %llu, length %u.", pg->bitmap_extent[i].offset, pg->bitmap_extent[i].length);
    }
 
    if (temp)
        vfree(temp);
    return 0;
}

/**
 * Description : 设置/boot/huawei/im.conf配置文件的位置
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : wangguitao/w00510599
 * Date        : 2019/7/19
 */
static int im_ctl_handel_set_configfile_extent(void* data)
{
    int ret = 0, i = 0;
    struct im_pg  *pg      = NULL;
    struct im_ctl_bitmap_extent_setting config_setting;
    struct im_ctl_bitmap_extent *configfile_extent, *temp;
 
    IOMIRROR_INFO("enter im_ctl_handel_set_configfile_extent.");
 
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }
 
    ret = copy_from_user(&config_setting, data, sizeof(config_setting));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
 
    if ((0 == config_setting.extent_num) || (0 == config_setting.dev_major) )
    {
        IOMIRROR_ERR("config file parameter is wrong.");
        return -EINVAL;;
    }

    // 为什么申请内存要多申请一块内存,最后一块设置为-1，后面可以进行判断数组结束
    configfile_extent = vmalloc(sizeof(struct im_ctl_bitmap_extent) * (config_setting.extent_num + 1));
    if (NULL == configfile_extent)
    {
        IOMIRROR_ERR("vmalloc for configfile_extent failed.");
        return -ENOMEM;
    }
    memset(configfile_extent, 0, sizeof(struct im_ctl_bitmap_extent) * (config_setting.extent_num + 1));
    
    configfile_extent[config_setting.extent_num].offset = -1;
    configfile_extent[config_setting.extent_num].length = -1;
    ret = copy_from_user(configfile_extent, config_setting.data, sizeof(struct im_ctl_bitmap_extent) * config_setting.extent_num);
    if (ret < 0)
    {
        vfree(configfile_extent);
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
    
    temp = pg->configfile_extent;
    pg->configfile_extent = configfile_extent;
    pg->dev_saveconfigfile = MKDEV(config_setting.dev_major, config_setting.dev_minor);

    IOMIRROR_INFO("config file major %u, minor %u, extent_num %u.", config_setting.dev_major, config_setting.dev_minor, config_setting.extent_num);
    for (i = 0; i < config_setting.extent_num; ++i)
    {
        IOMIRROR_INFO("offset %llu, length %u.", pg->configfile_extent[i].offset, pg->configfile_extent[i].length);
    }

    if (temp)
        vfree(temp);

    return 0;
}


/**
 * Description : get protected volumes size when executing normal shutdown, thus to allocate bitmap memory according volume size;
 *
 * Parameters  : void * arg
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : z00455045
 * Date        : 2019/8/2
 */
static int im_ctl_handle_get_volumes_size(void* arg)
{
    int ret = 0;
    struct im_pg *pg = NULL;
    ProtectVolSize protectVolSize;
    struct list_head *ptr = NULL;    
    struct im_volume * vol = NULL;
    uint32_t bitmap_len = 0;
    uint64_t bitmap_size = 0;

    IOMIRROR_INFO("enter im_ctl_handle_get_volumes_size.");
    
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }

    protectVolSize.disk_num = pg->vol_num;
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        bitmap_len = 0;
        bitmap_len = (vol->sectors + (1ULL << pg->bitmap_granularity) - 1) >> pg->bitmap_granularity;
        bitmap_len = (bitmap_len + BITS_PER_LONG - 1) >> BITS_PER_LEVEL;

        //get this volume's bitmap size, unit is byte.
        bitmap_len = bitmap_len * sizeof(bitmap_operator_t);

        //get integer multiple of IM_SECTOR_SIZE 
        bitmap_len = ((bitmap_len + IM_SECTOR_SIZE - 1) / IM_SECTOR_SIZE * IM_SECTOR_SIZE);
 
        bitmap_size += bitmap_len;
    }
    
    bitmap_size += PER_HEADER_SECTION_SIZE;
    bitmap_size += PER_DESCRIPTOR_SECTION_SIZE;
    protectVolSize.bitmap_size = bitmap_size;

    ret = copy_to_user(arg, &protectVolSize, sizeof(protectVolSize));
    if (ret < 0) {
        IOMIRROR_ERR("copy_to_user failed, ret = %d.", ret);
        return ret;
    }

    IOMIRROR_INFO("get volume size succ, disk number is %u, bitmap size %llu bytes.", protectVolSize.disk_num, protectVolSize.bitmap_size); 

    return 0;
}


/**
 * Description : 设置/boot/huawei/im.conf配置文件的内容，调用时需要放到im_ctl_handle_set_bitmap_cmd前执行
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : wangguitao/w00510599
 * Date        : 2019/7/19
 */
static int im_ctl_handel_set_configfile_content(void* data)
{
    int ret = 0;
    struct im_pg  *pg      = NULL;
    struct im_config_variable config_content;
 
    IOMIRROR_INFO("enter im_ctl_handel_set_configfile_content.");
     
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }

    // 需要比set bitmap前执行，否则flushtime会被使用
     if (1 == pg->is_init)
    {
        IOMIRROR_ERR("pg has been inited already, need set confile before setting bitmap.");
        return -EEXIST;
    }

    ret = copy_from_user(&config_content, data, sizeof(struct im_config_variable));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user struct im_config_variable failed.");
        return ret;
    }

    pg->cbt_flush_times = config_content.cbt_flush_times;
    pg->cbt_flush_times_done = config_content.cbt_flush_times_done;

    IOMIRROR_INFO("flushtimes %llu, flush_times_done %llu.", pg->cbt_flush_times, pg->cbt_flush_times_done);
    return 0;
}

// pg should not be NULL
static void get_bitmap_data_info(struct im_pg *pg, StatisticsInfo *info)
{
    const int percentage = 100;
    const int UNIT_KB = 1024;
    const int stop_send = 0;

    info->data_send_speed = pg->data_send_speed; // KB/s

    if (pg->state == IM_PG_STATE_CBT &&  pg->cbt_bitmap_total_bytes != 0) {
        info->remain_sync_data = (pg->cbt_bitmap_total_bytes >= pg->cbt_bitmap_send_sum ? 
                pg->cbt_bitmap_total_bytes - pg->cbt_bitmap_send_sum : 0); 
        info->synced_data = (pg->cbt_bitmap_send_sum >= pg->cbt_bitmap_total_bytes ? 
            pg->cbt_bitmap_total_bytes : pg->cbt_bitmap_send_sum);
        // The synced_data_rate max value is 99% under CBT and VERIFY mode, only in normal mode or is 100%
        info->synced_data_rate = (percentage * info->synced_data) / (pg->cbt_bitmap_total_bytes + 1); 
    } else if (pg->state == IM_PG_STATE_VERIFY && pg->verify_bitmap_total_bytes != 0) {
        info->remain_sync_data = (pg->verify_bitmap_total_bytes >= pg->verify_bitmap_send_sum ?
                pg->verify_bitmap_total_bytes - pg->verify_bitmap_send_sum : 0);
        info->synced_data = (pg->verify_bitmap_send_sum >= pg->verify_bitmap_total_bytes ?
                pg->verify_bitmap_total_bytes : pg->verify_bitmap_send_sum);
        info->synced_data_rate = (percentage * info->synced_data) / (pg->verify_bitmap_total_bytes + 1);
    } else {
        info->remain_sync_data = 0;
        info->synced_data = 0;
        // only in normal mode synced data rate is 100%
        info->synced_data_rate = (pg->state == IM_PG_STATE_NORMAL ? percentage : percentage - 1);
    }
 
    info->data_send_speed = ((pg->link_state == LINK_STATE_NORMAL && pg->work_state == WORK_STATE_NORMAL) ?
        info->data_send_speed : 0);

    info->expected_time = (info->data_send_speed != 0 ? 
        (info->remain_sync_data / UNIT_KB / info->data_send_speed) : stop_send);

    info->driver_pair_state.data_state = (pg->state == IM_PG_STATE_NORMAL ? DATA_STATE_NORMAL : DATA_STATE_CBT);
    info->driver_pair_state.link_state = pg->link_state;
    info->driver_pair_state.work_mode = pg->work_mode;
    info->driver_pair_state.work_state = pg->work_state;
}

static int im_ctl_handle_statistics_info(void* data)
{
    StatisticsInfo info;
    struct im_pg  *pg;
    int ret;
    unsigned long temp_rpo_time;
    const int msecs_per_second = 1000;

    IOMIRROR_INFO("enter im_ctl_handle_statistics_info.");

    if (!list_empty(&im_pg_list_head)) {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    } else {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }

    get_bitmap_data_info(pg, &info);

    temp_rpo_time = pg->statistics_jiffies.rpo_time;
    if (time_after(jiffies, temp_rpo_time)) { // mutiple thread handle jiffies
        info.driver_rpo_time = jiffies_to_msecs(jiffies + HZ / 2 - temp_rpo_time) / msecs_per_second; 
    } else {
        info.driver_rpo_time = 0;
    }

    // only show rpo time in normal state
    info.driver_rpo_time = (pg->state == IM_PG_STATE_NORMAL ? info.driver_rpo_time : 0);

    info.write_iops = pg->write_iops;
    info.write_throughout = pg->write_throughout; 

    ret = copy_to_user(data, &info, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_to_user statistics info failed.");
        return ret;
    }

    IOMIRROR_INFO("remain_sync_data : %llu, synced_data : %llu, synced_data_rate : %u, expected_time : %u, "
        "write_iops : %u, write_throughout : %llu, data_send_speed : %llu, rpo_time : %u, data_state : %d, "
        "link_state : %d, work_mode : %d, work_state : %d.", info.remain_sync_data, info.synced_data, 
        info.synced_data_rate, info.expected_time, info.write_iops, info.write_throughout, info.data_send_speed,
        info.driver_rpo_time, info.driver_pair_state.data_state, info.driver_pair_state.link_state,
        info.driver_pair_state.work_mode, info.driver_pair_state.work_state);

    return 0;
}

static int im_ctl_handle_get_kernel_alarm(void* data)
{
    GetAlarm* info = NULL;
    int ret = 0;
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;
    
    IOMIRROR_INFO("enter im_ctl_handle_kernel_alarm.");

    if (!list_empty(&im_pg_list_head)) {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    } else {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }

    info = (GetAlarm*)kzalloc(sizeof(GetAlarm), GFP_ATOMIC);
    if (!info) {
        IOMIRROR_ERR("vmalloc for GetAlarm failed.");
        return -ENOMEM;
    }

    ext_cmd.type = IM_PG_EXT_CMD_GET_KERNEL_ALARM;
    ext_cmd.info = info;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (ext_cmd.ret != 0) {     
        IOMIRROR_ERR("get kernel alarm info failed %d.", ext_cmd.ret);
        ret = ext_cmd.ret;
        pg->ext_cmd = NULL;
        goto out;
    }

    pg->ext_cmd = NULL;
    ret = copy_to_user(data, info, sizeof(GetAlarm));
    if (ret < 0) {
        IOMIRROR_ERR("copy_to_user kernel alarm info failed %d.", ret);
        goto out;
    }

    ret = 0;
out:
    if (info) {
        kfree(info);
        info = NULL;
    }
    return ret;
}

static int im_ctl_handle_set_tokenID(void* data)
{
    SetTokenID info;
    int ret = 0;
    struct im_pg *pg = NULL;
    struct im_pg_external_cmd ext_cmd;
    
    IOMIRROR_INFO("enter im_ctl_handle_set_tokenID.");

    if (!list_empty(&im_pg_list_head)) {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    } else {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }

    ret = copy_from_user(&info, data, sizeof(info));
    if (ret < 0) {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }

    if (info.isValid != 0 && info.isValid != 1) {
        IOMIRROR_ERR("token id flag is not 0 or 1.");
        return -1;
    }

    ext_cmd.type = IM_PG_EXT_CMD_SET_TOKEN_ID;
    ext_cmd.info = &info;
    ext_cmd.ret = -1;
    init_completion(&(ext_cmd.comp));
    pg->ext_cmd = &ext_cmd;

    wait_for_completion_interruptible_timeout(&(ext_cmd.comp), IM_PG_EXT_CMD_TIMEOUT);
    if (ext_cmd.ret != 0) {     
        IOMIRROR_ERR("set tokenid failed %d.", ext_cmd.ret);
        ret = ext_cmd.ret;
        pg->ext_cmd = NULL;
        goto out;
    }

    ret = 0;
    pg->ext_cmd = NULL;
out:
    return ret;
}

/**
 * Description : 设置CBT位图，系统启动时使用
 *    实现原理：重启之后，Agent从磁盘读取位图信息，每次构造成单个卷/磁盘的位图信息发送
 *    给driver的im_ctl_handle_set_bitmap_cmd处理，把该卷/磁盘的位图暂存于vol->bitmap_original，
 *    当每个卷/磁盘的位图都构造完成后，最后会在调用一次该函数的IOCTL，driver接收到的数据满足
 *    (0 == *info.vol_path) && (NULL == info.data)为真条件，然后执行im_pg_merge_bitmap，将关机之前
 *    的位图以及开机之后新产生的CBT位图merge到vol->bitmap中，不会丢失开机后这段时间的位图信息。   
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
static int im_ctl_handle_set_bitmap_cmd(void* data)
{
    int ret = 0;
    uint64_t size;
    OM_BITMAP *bitmap_temp;
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;
    struct im_pg  *pg      = NULL;
    struct im_ctl_bitmap info;

    IOMIRROR_INFO("enter im_ctl_handle_set_bitmap_cmd.");

    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }

    if (1 == pg->is_init)
    {
        IOMIRROR_ERR("pg has been inited already.");
        return -EEXIST;
    }

    ret = copy_from_user(&info, data, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }

    if ((0 == *info.vol_path) && (NULL == info.data))
    {
        list_for_each(ptr, &(pg->vols))
        {
            vol = list_entry(ptr, struct im_volume, list1);
            if (vol->bitmap_original)
            {
                size = (vol->sectors + (1ULL << pg->bitmap_granularity) - 1) >> pg->bitmap_granularity;
                size = (size + BITS_PER_LONG - 1) >> BITS_PER_LEVEL;
                // 复用count字段，需要再转换bit为1的数量为扇区数量
                im_pg_merge_bitmap(vol, (unsigned char *)vol->bitmap_original->buffer, size * sizeof(bitmap_operator_t), 
                    vol->bitmap_original->count >> pg->bitmap_granularity);
                BitmapFree(vol->bitmap_original, BitmapFreeFunc);
                vol->bitmap_original = NULL;
                IOMIRROR_INFO("after merge, vol bitmap count=%llu.", GetBitmapCount(vol->bitmap));
            }
            else
            {
                BitmapSetBit(vol->bitmap, 0, vol->sectors);
                IOMIRROR_INFO("vol bitmap count=%llu.", GetBitmapCount(vol->bitmap));
            }
        }

        IOMIRROR_INFO("im_ctl_handle_set_bitmap_cmd exec success.");
        pg->is_init = 1;
        return 0;
    }

    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        IOMIRROR_INFO("vol->path = %s, bitmap vol path is %s.", vol->path, info.vol_path);
        if (strncmp(vol->path, info.vol_path, sizeof(info.vol_path)) == 0)
            break;
        vol = NULL;
    }

    if (NULL == vol)
    {
        IOMIRROR_ERR("no found volume.");
        return -EINVAL;
    }

    size = (vol->sectors + (1ULL << pg->bitmap_granularity) - 1) >> pg->bitmap_granularity;
    size = (size + BITS_PER_LONG - 1) >> BITS_PER_LEVEL;
    if (info.bitmap_size == 0)
    {
        info.bitmap_size = size;
        strncpy(info.vol_path, vol->path, sizeof(info.vol_path));
        ret = copy_to_user(data, &info, sizeof(info));
        if (ret < 0)
        {
            IOMIRROR_ERR("copy_from_user failed.");
            return ret;
        }

        IOMIRROR_ERR("copy_from_user is EAGAIN.");
        return -EAGAIN;
    }

    IOMIRROR_INFO("vol %s size is %llu, sizeof(bitmap_operator_t) is %u.", vol->path, size, (unsigned int)(sizeof(bitmap_operator_t)));
    if (info.bitmap_size != (size * sizeof(bitmap_operator_t)))
    {
        IOMIRROR_ERR("wrong volume size, info.bitmap_size is %llu, size is %llu.", info.bitmap_size,  (size * sizeof(bitmap_operator_t)));
        return -EFAULT;
    }

    if (vol->bitmap_original)
    {
        IOMIRROR_ERR("original bitmap existed.");
        return -EEXIST;
    }

    bitmap_temp = BitmapAlloc(vol->sectors, pg->bitmap_granularity, BitmapAllocFunc, BitmapFreeFunc);
    if (NULL == bitmap_temp)
    {
        IOMIRROR_ERR("BitmapAlloc for bitmap_snapshot failed.");
        return -ENOMEM;
    }

    ret = copy_from_user(bitmap_temp->buffer, info.data, size * sizeof(bitmap_operator_t));
    if (ret < 0)
    {
        BitmapFree(bitmap_temp, BitmapFreeFunc);
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }

    vol->bitmap_original = bitmap_temp;
    // 复用bitmap中bitmap_count的字段，使用bitmap文件中扇区的数量转换为bit为1的数量
    vol->bitmap_original->count = info.bitmap_count << pg->bitmap_granularity;
    IOMIRROR_INFO("im_ctl_handle_set_bitmap_cmd exec success, bitmap file count=%llu, granularity=%u, flush time %llu.",
        info.bitmap_count, pg->bitmap_granularity, pg->cbt_flush_times);
    return 0;
}


#ifdef SUPPORT_BACKUP
/**
 * Description : 设置过滤位图
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
static int im_ctl_handle_set_filterbitmap_cmd(void* data)
{
    int ret = 0;
    uint64_t size;
    OM_BITMAP *bitmap_temp;
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;
    struct im_pg  *pg      = NULL;
    struct im_ctl_bitmap info;
 
    IOMIRROR_INFO("enter im_ctl_handle_set_filterbitmap_cmd.");
 
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }
 
    if (IM_NO_BACKUP_MODE == pg->backup_status)
    {
        IOMIRROR_ERR("not in backup mode.");
        return -ENOENT;
    }
 
    ret = copy_from_user(&info, data, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
 
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        if ( (vol->volume_minor == info.vol_dev_id) || (strncmp(vol->path, info.vol_path, sizeof(info.vol_path)) == 0) )
            break;
        vol = NULL;
    }
 
    if (NULL == vol)
    {
        IOMIRROR_ERR("no found volume.");
        return -EINVAL;
    }
 
    size = (vol->sectors + (1ULL << pg->bitmap_granularity) - 1) >> pg->bitmap_granularity;
    size = (size + BITS_PER_LONG - 1) >> BITS_PER_LEVEL;
    if (info.bitmap_size == 0)
    {
        info.bitmap_size = size;
        info.vol_dev_id = vol->volume_minor;
        strncpy(info.vol_path, vol->path, sizeof(info.vol_path));
        ret = copy_to_user(data, &info, sizeof(info));
        if (ret < 0)
        {
            IOMIRROR_ERR("copy_from_user failed.");
            return ret;
        }
        
        return -EAGAIN;
    }
 
    if (info.bitmap_size != size)
    {
        IOMIRROR_ERR("wrong volume size.");
        return -EFAULT;
    }
    
    if (NULL == vol->bitmap_filter)
    {
        bitmap_temp = BitmapAlloc(vol->sectors, pg->bitmap_granularity, BitmapAllocFunc, BitmapFreeFunc);
        if (NULL == bitmap_temp)
        {
            IOMIRROR_ERR("BitmapAlloc for bitmap_snapshot failed.");
            return -ENOMEM;
        }
 
        ret = copy_from_user(bitmap_temp->buffer, info.data, size * sizeof(bitmap_operator_t));
        if (ret < 0)
        {
            BitmapFree(bitmap_temp, BitmapFreeFunc);
            IOMIRROR_ERR("copy_from_user failed.");
            return ret;
        }
        
        vol->bitmap_filter = bitmap_temp;
    }
    else
    {
        ret = copy_from_user(vol->bitmap_filter->buffer, info.data, size * sizeof(bitmap_operator_t));
        if (ret < 0)
        {
            IOMIRROR_ERR("copy_from_user failed.");
            return ret;
        }
    }
 
    return 0;
}


/**
 * Description : 获取CBT位图
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
static int im_ctl_handle_get_bitmap_cmd(void* data)
{
    int ret = 0;
    uint64_t size;
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;
    struct im_pg  *pg      = NULL;
    struct im_ctl_bitmap info;
 
    IOMIRROR_INFO("enter im_ctl_handle_get_bitmap_cmd.");
 
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }
 
    if (IM_BACKUP_SNAPSHOT != pg->backup_status)
    {
        IOMIRROR_ERR("not in snapshot mode.");
        return -ENOENT;
    }
 
    ret = copy_from_user(&info, data, sizeof(info));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        return ret;
    }
 
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        if ( (vol->volume_minor == info.vol_dev_id) || (strncmp(vol->path, info.vol_path, sizeof(info.vol_path)) == 0) )
            break;
        vol = NULL;
    }
 
    if (NULL == vol)
    {
        IOMIRROR_ERR("no found volume.");
        return -EINVAL;
    }
 
    size = (vol->sectors + (1ULL << pg->bitmap_granularity) - 1) >> pg->bitmap_granularity;
    size = (size + BITS_PER_LONG - 1) >> BITS_PER_LEVEL;
    if (info.bitmap_size == 0)
    {
        if (vol->bitmap_original)
            info.bitmap_size = size;
        info.vol_dev_id = vol->volume_minor;
        strncpy(info.vol_path, vol->path, sizeof(info.vol_path));
        ret = copy_to_user(data, &info, sizeof(info));
        if (ret < 0)
        {
            IOMIRROR_ERR("copy_from_user failed.");
            return ret;
        }
        
        return -EAGAIN;
    }
 
    if (info.bitmap_size != size)
    {
        IOMIRROR_ERR("wrong volume size.");
        return -EFAULT;
    }
 
    ret = copy_to_user(info.data, vol->bitmap_original->buffer, size * sizeof(bitmap_operator_t));
    if (ret < 0)
    {
        IOMIRROR_ERR("copy_to_user failed.");
        return ret;
    }
 
    return 0;
}


static unsigned int copy_from_user_to_cmd(struct im_cmd *cmd, const void *from)
{
    unsigned int i, ret = 0;
    unsigned char *to = NULL;
 
    for(i = 0; i < cmd->vcnt; i++) {
        to = page_address(cmd->bvl[i].bv_page);
        ret = copy_from_user(to, from + i * PAGE_SIZE, PAGE_SIZE);
        if(ret != 0)
            break;
        cmd->bvl[i].bv_len = PAGE_SIZE;
        cmd->bvl[i].bv_offset = 0;
    }
 
    return ret;
}


static uint32_t copy_to_user_from_cmd(void *to, struct im_cmd *cmd, uint32_t len)
{
    uint32_t i, offset = 0, ret = 0;
    char *from = NULL;
 
    for(i = 0; i < cmd->vcnt; i++) {
        from = page_address(cmd->bvl[i].bv_page);
        if(len == 0) {
            ret = copy_to_user(to + i * PAGE_SIZE, from, PAGE_SIZE);
        } else {
            ret = copy_to_user(to + offset, from + cmd->bvl[i].bv_offset, cmd->bvl[i].bv_len);
            offset += cmd->bvl[i].bv_len;
        }
        if(ret != 0)
            return ret;
    }
 
    if(len != offset) {
        return -EPERM;
    }
    return 0;
}


/**
 * Description : 字符设备read函数，将COW数据获取到用户态
 *
 * Parameters  : flip
 *               buf
 *               count
 *               ppos
 * Return      : 0
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
static ssize_t im_ctldev_read(struct file *flip, char __user *buf, size_t count, loff_t *ppos)
{
    uint64_t offset;
    ssize_t ret = 0;
    struct im_volume *vol;
    struct im_cmd *cmd = NULL;
    struct im_pg *pg = NULL;
 
    flip->private_data = NULL;
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        return -ENODEV;
    }
 
    if (IM_BACKUP_SNAPSHOT != pg->backup_status)
    {
        IOMIRROR_ERR("no snapshot currently.");
        return -ENOENT;
    }
 
    wait_event_interruptible_timeout(pg->rq.wq, pg->rq.count > 0, IM_PG_WAIT_TIMEOUT);
    if (signal_pending(current))
    {
        return -EINTR;
    }
 
    if (pg->rq.count <= 0)
    {
        return -EAGAIN;
    }
 
    if (list_empty(&(pg->rq.head)))
    {
        IOMIRROR_ERR("queue is empty, but count is not 0, bug?");
        pg->rq.count = 0;
        return -EAGAIN;
    }
    
    down(&(pg->rq.lock));
    cmd = list_first_entry(&(pg->rq.head), struct im_cmd, list);
    list_del(&(cmd->list));
    pg->rq.count--;
    up(&(pg->rq.lock));
 
    vol = cmd->private;
    offset = vol->volume_minor;
    offset <<= IM_CTL_BACKUP_VOLUME_BIT;
    offset += cmd->header.data_offset * IM_SECTOR_SIZE;
    if (cmd->bvl)
    {
        offset |= IM_CTL_BACKUP_SNAPREAD_FLAG;
        ret = copy_to_user(buf, &offset, sizeof(uint64_t));
        if (0 != ret)
        {
            pg->backup_status = IM_BACKUP_CBT;
            IOMIRROR_ERR("copy_to_user failed.");
            goto end;
        }
        
        flip->private_data = cmd;
        return 0;
    }
    
    if (readdata4cmd(cmd) < 0)
    {
        pg->backup_status = IM_BACKUP_CBT;
        ret = -EPERM;
        goto end;
    }
    
    if (cmd->is_snapread)
    {
		if (cmd->completion)
		{
			complete(cmd->completion);
		}
 
        return -EAGAIN;
    }
 
    ret = copy_to_user(buf, &offset, sizeof(uint64_t));
    if (0 != ret)
    {
        pg->backup_status = IM_BACKUP_CBT;
        IOMIRROR_ERR("copy_to_user failed.");
        goto end;
    }
    
    ret = copy_to_user_from_cmd(buf + sizeof(uint64_t), cmd, 0);
    if (0 != ret)
    {
        pg->backup_status = IM_BACKUP_CBT;
        IOMIRROR_ERR("copy_to_user_from_cmd failed.");
        goto end;
    }
 
    BitmapSetBit(vol->bitmap_snapshot, cmd->header.data_offset, 1);
end:;
    if (NULL != cmd)
    {
        if (cmd->completion)
        {
            complete(cmd->completion);  /* 放行写IO */
        }
 
        im_cmd_free(cmd);
    }
    return ret;
}


/**
 * Description : 字符设备write函数，将COW数据写回核心态
 *
 * Parameters  : flip
 *               buf
 *               count
 *               ppos
 * Return      : 0
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
static ssize_t im_ctldev_write(struct file *flip, const char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t ret = 0;
    uint64_t offset;
    struct im_cmd *cmd = NULL;
    struct im_pg *pg = NULL;
 
    IOMIRROR_INFO("enter im_ctldev_write.");
 
    if (NULL == flip->private_data)
        return -ENOTBLK;
 
    cmd = flip->private_data;
    flip->private_data = NULL;
    if (NULL == cmd->completion)
    {
        IOMIRROR_ERR("wrong completion on cmd saved.");
        ret = -EACCES;
        goto fail;
    }
 
    ret = copy_from_user(&offset, buf, sizeof(uint64_t));
    if (0 != ret)
    {
        IOMIRROR_ERR("copy_from_user failed.");
        goto fail;
    }
    
    if ((cmd->header.data_offset * IM_SECTOR_SIZE) != (offset & IM_CTL_BACKUP_OFFSET_MASK))
    {
        IOMIRROR_ERR("wrong offset on cmd saved.");
        ret = -EINVAL;
        goto fail;
    }
    
    if (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    }
    else
    {
        IOMIRROR_ERR("can NOT find protect group.");
        ret = -ENODEV;
        goto fail;
    }
 
    if (IM_BACKUP_SNAPSHOT != pg->backup_status)
    {
        IOMIRROR_ERR("wrong cmd saved.");
        ret = -ENOENT;
        goto fail;
    }
 
    ret = copy_from_user_to_cmd(cmd, buf + sizeof(uint64_t));
    if (0 != ret)
    {
        pg->backup_status = IM_BACKUP_CBT;
        IOMIRROR_ERR("copy_from_user_to_cmd failed.");
        goto fail;
    }
 
    complete(cmd->completion);
    return 0;
 
fail:
    im_cmd_free(cmd);
    return ret;
}
#endif


/**
 * Description : 字符设备open函数，
 *               通过获取信号量，确保仅能被一个进程打开
 *
 * Parameters  : inode
 *               flip 
 * Return      : 成功 - 0
 *               失败 - -EBUSY，已被open，设备忙
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static int im_ctldev_open(struct inode *inode, struct file *flip)
{
    if (down_trylock(&(ctldev.sem)))
    {
        return -EBUSY;
    }
    else
    {
        return 0;
    }
}


/**
 * Description : 字符设备release函数，释放用于互斥的信号量
 *
 * Parameters  : inode
 *               flip 
 * Return      : 0
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static int im_ctldev_release(struct inode *inode, struct file *flip)
{
    up(&(ctldev.sem));
    return 0;
}


/**
 * Description : ioctl命令处理入口
 *
 * Parameters  : inode
 *               flip 
 *               cmd - 命令字
 *               arg - 命令参数指针
 * Return      : 命令处理函数返回值
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static long im_ctldev_unlocked_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd)
    {
        case IM_CTL_START_IOMIRROR:
            ret = im_ctl_handle_start_iomirror_cmd((void *)arg, IM_PG_START_INIT);
            break;
        case IM_CTL_START_IOMIRROR_WITH_VERIFY:
            ret = im_ctl_handle_start_iomirror_cmd((void *)arg, IM_PG_START_VERIFY);
            break;
        case IM_CTL_START_IOMIRROR_FOR_BACKUP:
            ret = im_ctl_handle_start_iomirror_cmd((void *)arg, IM_PG_START_BACKUP);
            break;
        case IM_CTL_STOP_IOMIRROR:
            ret = im_ctl_handle_stop_iomirror_cmd();
            break;
        case IM_CTL_MODIFY_IOMIRROR:
            ret = im_ctl_handle_modify_iomirror_cmd((void*)arg);
            break;
        case IM_CTL_PAUSE_IOMIRROR: 
            ret = im_ctl_handle_pause_iomirror_cmd((void*)arg);
            break;
        case IM_CTL_RESUME_IOMIRROR: 
            ret = im_ctl_handle_resume_iomirror_cmd();
            break;
        case IM_CTL_ADD_VOLUME: 
            ret = im_ctl_handle_add_volume_cmd((void*)arg);
            break;
        case IM_CTL_DEL_VOLUME: 
            ret = im_ctl_handle_del_volume_cmd((void*)arg);
            break;
        case IM_CTL_MOD_VOLUME: 
            ret = im_ctl_handle_mod_volume_cmd((void*)arg);
            break;
        case IM_CTL_VOLUME_READY: 
            ret = im_ctl_handle_volume_ready_cmd((void*)arg);
            break;
        // 用于接收OS正常关机时下发的bitmap文件的数据块,
        // 关机前把两个bitmap数据写入文件中
        case IM_CTL_SETBITMAPEXTENT:
            ret = im_ctl_bitmap_set_bitmap_extent((void*)arg);
            break;
        // 用于接收OS启动后agent从bitmap文件中读取bitmap内容,
        // 接收bitmap后可以正常启动保护
        case IM_CTL_SETBITMAP:
            ret = im_ctl_handle_set_bitmap_cmd((void*)arg);
            break;
        case IM_CTL_SETCONFIG_EXTENT:
            ret = im_ctl_handel_set_configfile_extent((void*)arg);
            break;
        case IM_CTL_SETCONFIG_CONTENT:
            ret = im_ctl_handel_set_configfile_content((void*)arg);
            break;
        // get protected volumes size during normal shutdown to allocate bitmap file automatic 
        case IM_CTL_GET_VOLUMES_SIZE:
            ret = im_ctl_handle_get_volumes_size((void*)arg);
            break;
        case IM_CTL_STOP_SEND_DATA:
            ret = im_ctl_handle_stop_send_data();
            break;
        case IM_CTL_GET_STATISTICS_INFO:
            ret = im_ctl_handle_statistics_info((void*)arg);
            break;
        case IM_CTL_GET_KERNEL_ALARM:
            ret = im_ctl_handle_get_kernel_alarm((void*)arg);
            break;
        case IM_CTL_SET_TOKEN_ID:
            ret = im_ctl_handle_set_tokenID((void*)arg);
            break;
#ifdef SUPPORT_BACKUP
        case IM_CTL_BACKUP_TAKESNAPSHOT: 
            ret = im_ctl_handle_take_snapshot_cmd((void*)arg);
            break;
        case IM_CTL_BACKUP_REMOVESNAPSHOT: 
            ret = im_ctl_handle_remove_snapshot_cmd((void*)arg);
            break;
        case IM_CTL_BACKUP_SETFILTERBITMAP:
            ret = im_ctl_handle_set_filterbitmap_cmd((void*)arg);
            break;
        case IM_CTL_BACKUP_GETBITMAP:
            ret = im_ctl_handle_get_bitmap_cmd((void*)arg);
            break;
#endif
        default:
            ret = -ENOTTY;
            break;
    }


    return ret;
}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35))
static int im_ctldev_ioctl(struct inode *inode,
                    struct file *flip, unsigned int cmd, unsigned long arg)
{
    return (int)im_ctldev_unlocked_ioctl(flip, cmd, arg);
}
#endif
static const struct file_operations im_ctldev_fops =
{
    .owner    = THIS_MODULE,
    .open     = im_ctldev_open,
    .release  = im_ctldev_release,
#ifdef SUPPORT_BACKUP
    .read     = im_ctldev_read,
    .write    = im_ctldev_write,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl    = im_ctldev_unlocked_ioctl
#else
    .ioctl    = im_ctldev_ioctl
#endif
};


/**
 * Description : 创建用于ioctl的字符设备
 *
 * Parameters  : void
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
int im_ctldev_create(void)
{
    int ret = 0;

    IOMIRROR_DBG("enter im_ctldev_create.");

    memset(&ctldev, 0, sizeof(ctldev));
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
    sema_init(&(ctldev.sem), 1);
#else
    init_MUTEX(&(ctldev.sem));
#endif
    cdev_init(&(ctldev.cdev), &im_ctldev_fops);
    ctldev.cdev.owner = THIS_MODULE;

    ret = alloc_chrdev_region(&(ctldev.devno), 0, 1, IM_CTL_DEV_NAME);
    if (ret < 0)
    {
        IOMIRROR_ERR("alloc_chrdev_region failed, errno=%d.", ret);
        ret = -1;
        goto fail1;
    }

    ret = cdev_add(&(ctldev.cdev), ctldev.devno, 1);
    if (ret)
    {
        IOMIRROR_ERR("cdev_add failed, errno=%d", ret);
        ret = -1;
        goto fail2;
    }

#ifdef SUPPORT_BACKUP
    ctldev.blkno = register_blkdev(0, IM_SNAPSHOT_DEV_NAME);
    if(ctldev.blkno < 0) {
        IOMIRROR_ERR("register_blkdev failed, errno=%d", ctldev.blkno);
        ret = -1;
        goto fail2;
    }
#endif
/* GPL-only symbol，改由用户态创建控制设备文件 */
//    ctldev.cls = class_create(THIS_MODULE, "im_ctldev_class");
//    if (IS_ERR(ctldev.cls))
//    {
//        IOMIRROR_ERR("class_create failed.");
//        ret = -1;
//        goto fail3;
//    }
//
//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27))
//    ctldev.cd = class_device_create(ctldev.cls, NULL,
//                                    ctldev.devno, NULL, IM_CTL_DEV_NAME);
//#else
//    ctldev.cd = device_create(ctldev.cls, NULL, ctldev.devno, NULL, IM_CTL_DEV_NAME);
//#endif
//    if (IS_ERR(ctldev.cd))
//    {
//        IOMIRROR_ERR("class_device_create failed.");
//        ret = -1;
//        goto fail4;
//    }

    return 0;


//fail4:
//    class_destroy(ctldev.cls);
//fail3:
//    cdev_del(&(ctldev.cdev));
fail2:
    unregister_chrdev_region(ctldev.devno, 1);
fail1:
    return ret;
}


/**
 * Description : 删除用于ioctl的字符设备
 *
 * Parameters  : void
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
void im_ctldev_remove(void)
{
/* GPL-only symbol，改由用户态创建控制设备文件 */
//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27))
//    if (NULL != ctldev.cls && 0 != ctldev.devno)
//    {
//        class_device_destroy(ctldev.cls, ctldev.devno);
//    }
//#else
//    if (NULL != ctldev.cls && 0 != ctldev.devno)
//    {
//        device_destroy(ctldev.cls, ctldev.devno);
//    }
//#endif
//
//    if (NULL != ctldev.cls)
//    {
//        class_destroy(ctldev.cls);
//    }
#ifdef SUPPORT_BACKUP
    unregister_blkdev(ctldev.blkno, IM_SNAPSHOT_DEV_NAME);
#endif

    if (0 != ctldev.devno)
    {
        cdev_del(&(ctldev.cdev));
        unregister_chrdev_region(ctldev.devno, 1);
    }
}

