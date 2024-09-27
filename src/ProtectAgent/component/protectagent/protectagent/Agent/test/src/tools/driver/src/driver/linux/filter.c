/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : filter.c
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/18
 * Version     : 1.0
 *
 * Description : I/O分流，替换make_request_fn实现
 *
 */

#include <linux/bio.h>
#include <linux/delay.h>
#include <linux/namei.h>
#include <linux/version.h>

#include "cmd_define.h"
#include "util.h"
#include "filter.h"
#include "protect.h"

LIST_HEAD(im_vol_list_head);
DECLARE_RWSEM(im_vol_list_sem);

/*
 * Description : 获得块设备的原始make_request_fn
 *
 * Parameters  : bdev - 待获取mfn的块设备指针
 * Return      : 成功，返回获取到的mfn函数指针
 *               失败，返回NULL
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/26
 */
static inline make_request_fn *im_get_ori_mfn(struct block_device *bdev)
{
    dev_t  bd_dev = bdev->bd_contains->bd_dev; 
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;

    down_read(&im_vol_list_sem);
    list_for_each(ptr, &im_vol_list_head)
    {
        vol = list_entry(ptr, struct im_volume, list0);

        if (bd_dev == vol->bd_disk_dev)
        {
            up_read(&im_vol_list_sem);
            return vol->ori_mfn->f;
        }
    }
    up_read(&im_vol_list_sem);

    return NULL;
}


/**
 * Description : 判断所截获的bio是否需要保护
 *
 * Parameters  : bio - 待判断的bio指针
 * Return      : 被保护，返回bio所对应的im_volume指针
 *               不被保护，返回NULL
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/26
 */
static inline struct im_volume *im_is_protect(struct bio *bio)
{
    dev_t bd_dev    = bio->bi_bdev->bd_dev;
    sector_t start  = bio->bi_sector;
    sector_t end    = bio->bi_sector + bio_sectors(bio) - 1;
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;

    down_read(&im_vol_list_sem);
    list_for_each(ptr, &im_vol_list_head)
    {
        vol = list_entry(ptr, struct im_volume, list0);

        /**
         * 当bio所属磁盘与vol中磁盘一致，
         * 并且落在vol的起始范围内时才需要保护
         */
        if (bd_dev == vol->bd_disk_dev
            && start >= vol->start
            && end <= vol->end)
        {
            up_read(&im_vol_list_sem);
            return vol;
        }
    }
    up_read(&im_vol_list_sem);

    return NULL;
}


/**
 * Description : 将被保护bio包装成数据报文
 *
 * Parameters  : bio - 待处理的写请求bio指针
 *               vol - 待处理bio所属im_volume卷指针
 * Return      : 成功 - 申请并初始化完毕的cmd指针
 *               失败 - NULL
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/26
 */
static inline struct im_cmd *im_alloc_cmd(struct bio *bio,
                                               struct im_volume *vol)
{
    int i = 0;
    struct im_cmd *cmd = NULL;
    struct bio_vec *bv = NULL;
    struct page *page  = NULL;
    struct im_cmd_header *cmd_header = NULL;

    /* 为队列数据报文的报文头分配空间 */
    cmd = (struct im_cmd *)kzalloc(sizeof(struct im_cmd), GFP_ATOMIC);
    if (unlikely(NULL == cmd))
    {
        IOMIRROR_ERR("kzalloc for cmd failed.");
        goto fail;
    }
    cmd->private = vol;
    cmd->vcnt = bio->bi_vcnt;
    cmd->bvl = kzalloc(sizeof(struct bio_vec) * (cmd->vcnt), GFP_ATOMIC);
    cmd->buf = NULL;
    cmd->has_received = 0;

    if (unlikely(NULL == cmd->bvl))
    {
        IOMIRROR_ERR("kzalloc for cmd->bvl failed.");
        goto fail;
    }

    /* 为队列数据报文的报文体分配空间，并拷贝bio数据 */
    bio_for_each_segment(bv, bio, i)
    {
        page = alloc_page(GFP_ATOMIC);
        if (unlikely(NULL == page))
        {
            IOMIRROR_ERR("alloc_page failed.");
            goto fail;
        }
        cmd->bvl[i].bv_page   = page;
        cmd->bvl[i].bv_offset = 0;
        cmd->bvl[i].bv_len    = bv->bv_len;
        memcpy(page_address(page), page_address(bv->bv_page) + bv->bv_offset, bv->bv_len);
    }

    /* 初始化报文头 */
    cmd_header = &(cmd->header);
    cmd_header->version         = IM_VERSION;
    cmd_header->magic           = IM_MAGIC;
    cmd_header->cmd_type        = DPP_TYPE_DATA;
    cmd_header->data_offset     = bio->bi_sector - vol->start;
    cmd_header->data_size_bytes = bio->bi_size;
    memcpy(cmd_header->vol_id, vol->id, VOL_ID_LEN);

    return cmd;

fail:

    if (NULL != cmd)
    {
        if (NULL != cmd->bvl)
        {
            for (i = 0; i < cmd->vcnt; i++)
            {
                if (NULL != cmd->bvl[i].bv_page)
                {
                    __free_page(cmd->bvl[i].bv_page);
                }
                else
                {
                    break;
                }
            }
            kfree(cmd->bvl);
            cmd->bvl = NULL;
        }
    
        kfree(cmd);
        cmd = NULL;
    }

    return NULL;
}


/**
 * Description : IOMirror bio完成函数，用于im_filter_make_request_fn
 *
 * Parameters  : bio_clo   
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
static int im_bio_end_io(struct bio *bio_clo, unsigned int bytes_done, int error)
#else
static void im_bio_end_io(struct bio *bio_clo, int error)
#endif
{
    struct im_cmd_status* status = (struct im_cmd_status *)(bio_clo->bi_private);
    struct bio *bio_ori = status->bio_ori;

    /* bio写本地完成 */
    if (likely(0 == error))
    {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
        bio_endio(bio_ori, bio_ori->bi_size, error);
#else
        bio_endio(bio_ori, error);
#endif
        /* set related_local_bio_complete to 0 */
        if(status != NULL)
            status->related_local_bio_complete = LOCAL_BIO_SUCCESS;
    }
    else
    {
    // TODO: 本地写I/O出错，应作错误处理(一致性校验)，cmd/vol此时可能已释放
    // TODO: 当前I/O对应的cmd和 vol可以获得，本函数开头注释

        /* set related_local_bio_complete to 2 */
        if(status != NULL)
            status->related_local_bio_complete = LOCAL_BIO_FAILURE;

        IOMIRROR_ERR("write local bio failed, status is %u", status->related_local_bio_complete);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
        bio_io_error(bio_ori, bio_ori->bi_size);
#else
        bio_io_error(bio_ori);
#endif
    }

    if(status == NULL)
        goto out;

    if( 1 == atomic_cmpxchg(&(status->del_flag), 0, 1) )
    {
        kfree(status);
        status = NULL;
    }
    
out:
    bio_put(bio_clo);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    return 0;
#endif
}


/**
 * Description : IOMirror make_request_fn，
 *               用于替换原mfn，实现I/O分流
 *
 * Parameters  : q  
 *               bio
 * Return      :
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/25
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
void im_filter_make_request_fn(struct request_queue *q, struct bio *bio)
#else
int im_filter_make_request_fn(struct request_queue *q, struct bio *bio)
#endif
{
    int i = 0;
    int ret = 0;
    struct im_cmd *cmd = NULL;
    struct im_volume *vol = NULL;
    struct bio *bio_clo = NULL;
    make_request_fn *ori_mfn = NULL;
    struct im_cmd_status* cmd_status = NULL;

    /* 读I/O和不需要保护的写I/O直接透传 */
    if (READ == bio_data_dir(bio))
    {
        goto through;
    }

    vol = im_is_protect(bio);
    if (NULL == vol)
    {
        goto through;
    }

    /* 处理需要保护的写I/O */
    bio_clo = bio_clone(bio, GFP_NOIO);
    if (unlikely(NULL == bio_clo))
    {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
        bio_io_error(bio, bio->bi_size);
#else
        bio_io_error(bio);
#endif
        IOMIRROR_ERR("bio_clone failed.");
        goto out;
    }

    cmd = im_alloc_cmd(bio, vol);
    if (unlikely(NULL == cmd))
    {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
        bio_io_error(bio, bio->bi_size);
#else
        bio_io_error(bio);
#endif
        IOMIRROR_ERR("im_alloc_cmd failed.");
        goto out;
    }

    im_pg_count_rw_iops_and_wsize(cmd->header.data_size_bytes);

    cmd_status = (struct im_cmd_status *)kzalloc(sizeof(struct im_cmd_status), GFP_KERNEL);
    cmd_status->bio_ori = bio;
    cmd_status->related_local_bio_complete = LOCAL_BIO_ONGOING;

    bio_clo->bi_private = cmd_status;
    bio_clo->bi_end_io = im_bio_end_io;

    cmd->status = cmd_status;
    down(&(vol->rq->lock));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
    vol->ori_mfn->f(q, bio_clo);
    ret = 0;
#else
    ret = vol->ori_mfn->f(q, bio_clo);
#endif
    if (likely(0 == ret))
    {
        list_add_tail(&(cmd->list), &(vol->rq->head));
        vol->rq->count++;
        vol->rq->page_cnt += cmd->vcnt;
        up(&(vol->rq->lock));
        wake_up_interruptible(&(vol->rq->wq));
    }
    else
    {
        up(&(vol->rq->lock));
        for (i = 0; i < cmd->vcnt; i++)
        {
            __free_page(cmd->bvl[i].bv_page);
        }
        kfree(cmd->bvl);
        kfree(cmd);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
        bio_io_error(bio, bio->bi_size);
#else
        bio_io_error(bio);
#endif
        bio_put(bio_clo);
    }
    goto out;

through:
    ori_mfn = im_get_ori_mfn(bio->bi_bdev);
    if (unlikely(NULL == ori_mfn))
    {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
        bio_io_error(bio, bio->bi_size);
#else
        bio_io_error(bio);
#endif
        IOMIRROR_ERR("can NOT find original make request function, major=%u, minor=%u.",
                    MAJOR(bio->bi_bdev->bd_dev), MINOR(bio->bi_bdev->bd_dev));
        goto out;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
    ori_mfn(q, bio);    

out:
    return;
#else
    ret = ori_mfn(q, bio);    

out:
    return ret;
#endif
}


#ifdef SUPPORT_BACKUP
/**
 * Description : 释放报文结构体
 *
 * Parameters  : cmd - 待释放的报文指针，调用者确保指针有效
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/27
 */
void im_cmd_free(struct im_cmd *cmd)
{
    int i = 0;
    
    if (NULL == cmd)
        return;
 
    if (NULL != cmd->bvl)
    {
        for (i = 0; i < cmd->vcnt; i++)
        {
            if (NULL != cmd->bvl[i].bv_page)
            {
                __free_page(cmd->bvl[i].bv_page);
            }
            else
            {
                break;
            }
        }
        kfree(cmd->bvl);
    }
 
    if (NULL != cmd->buf)
    {
        vfree(cmd->buf);
    }
    kfree(cmd);
}
 
/**
 * Description : 将需要COW的bio包装成数据报文
 *
 * Parameters  : bio - 待处理的写请求bio指针
 *               vol - 待处理bio所属im_volume卷指针
 * Return      : 成功 - 申请并初始化完毕的cmd指针
 *               失败 - NULL
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
struct im_cmd *im_alloc_cmd4snapio(struct im_volume *vol, uint64_t offset)
{
    int i = 0, pages4block;
    struct im_cmd *cmd = NULL;
    struct page *page  = NULL;
    struct im_cmd_header *cmd_header = NULL;
 
    /* 为队列数据报文的报文头分配空间 */
    cmd = (struct im_cmd *)kzalloc(sizeof(struct im_cmd), GFP_ATOMIC);
    if (unlikely(NULL == cmd))
    {
        IOMIRROR_ERR("kzalloc for cmd failed.");
        goto fail;
    }
    cmd->private = vol;
 
    pages4block = (1U << vol->bitmap->granularity) * IM_SECTOR_SIZE / PAGE_SIZE;
    cmd->bvl = kzalloc(sizeof(struct bio_vec) * pages4block, GFP_ATOMIC);
    if (unlikely(NULL == cmd->bvl))
    {
        IOMIRROR_ERR("kzalloc for cmd->bvl failed.");
        goto fail;
    }
 
    /* 为队列数据报文的报文体分配空间 */
    for(i = 0; i < pages4block; i++)
    {
        page = alloc_page(GFP_ATOMIC);
        if (unlikely(NULL == page))
        {
            IOMIRROR_ERR("alloc_page failed.");
            goto fail;
        }
        cmd->bvl[i].bv_page   = page;
        cmd->bvl[i].bv_offset = 0;
        cmd->bvl[i].bv_len    = PAGE_SIZE;
        cmd->vcnt++;
    }
 
    /* 初始化报文头 */
    cmd_header = &(cmd->header);
    cmd_header->version         = IM_VERSION;
    cmd_header->magic           = IM_MAGIC;
    cmd_header->cmd_type        = DPP_TYPE_DATA;
    cmd_header->data_offset     = offset;
    cmd_header->data_size_bytes = 1U << vol->bitmap->granularity;
    cmd_header->data_size_bytes *= IM_SECTOR_SIZE;
    memcpy(cmd_header->vol_id, vol->id, VOL_ID_LEN);
 
    return cmd;
 
fail:
    if (NULL != cmd)
    {
        im_cmd_free(cmd);
    }
 
    return NULL;
}
 
/**
 * Description : 将需要COW的bio包装成数据报文
 *
 * Parameters  : bio - 待处理的写请求bio指针
 *               vol - 待处理bio所属im_volume卷指针
 * Return      : 成功 - 申请并初始化完毕的cmd指针
 *               失败 - NULL
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
struct im_cmd *im_alloc_cmd4diskio(struct im_volume *vol, uint64_t offset, uint8_t is_snapread)
{
    struct im_cmd *cmd = NULL;
    struct im_cmd_header *cmd_header = NULL;
 
    /* 为队列数据报文的报文头分配空间 */
    cmd = (struct im_cmd *)kzalloc(sizeof(struct im_cmd), GFP_ATOMIC);
    if (unlikely(NULL == cmd))
    {
        IOMIRROR_ERR("kzalloc for cmd failed.");
        goto fail;
    }
    cmd->private = vol;
    cmd->is_snapread = is_snapread;
 
    /* 初始化报文头 */
    cmd_header = &(cmd->header);
    cmd_header->version         = IM_VERSION;
    cmd_header->magic           = IM_MAGIC;
    cmd_header->cmd_type        = DPP_TYPE_DATA;
    cmd_header->data_offset     = offset;
    cmd_header->data_size_bytes = (vol->sectors - offset < (1U << vol->bitmap->granularity))? vol->sectors - offset : (1U << vol->bitmap->granularity);
    cmd_header->data_size_bytes *= IM_SECTOR_SIZE;
    memcpy(cmd_header->vol_id, vol->id, VOL_ID_LEN);

    return cmd;
 
fail:
    if (NULL != cmd)
    {
        im_cmd_free(cmd);
    }
 
    return NULL;
}
 
/**
 * Description : 更新CBT位图，并分配一组cmd，
 *
 * Parameters  : bio  
 *               vol
 * Return      : not NULL - 需要做COW
 *               NULL - 不需要做COW
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
static int im_alloc_cmds(struct bio *bio, struct im_volume *vol, struct completion *completion)
{
    int margin, blocksize, ret = -1;
    uint64_t pos, start, size;
    struct im_cmd *cmd = NULL, *cmd_old = NULL;
 
    if ( (vol->rq->pg->backup_status != IM_BACKUP_CBT) && (vol->rq->pg->backup_status != IM_BACKUP_SNAPSHOT) )
    {
        return -1;
    }
 
    if (NULL == vol->bitmap)    /* 无CBT位图，错误分支 */
    {
        //vol->rq->pg->backup_status = IM_BACKUP_INIT;
        return -1;
    }
 
    blocksize = 1U << vol->bitmap->granularity;
    start = bio->bi_sector - vol->start;
    margin = do_div(start, blocksize) * IM_SECTOR_SIZE;
    start *= blocksize; // align start
    size = bio->bi_size + margin;
    size = (size + IM_SECTOR_SIZE - 1) / IM_SECTOR_SIZE;
 
    for (pos = start; pos < start + size; pos += blocksize)
    {
        if (!IsBitmapBitSet(vol->bitmap, pos))
            break;
    }
 
    if (pos >= start + size)
        return -1;  /* 快速检测，对应CBT的所有相关位已经被设置，无需继续 */
 
    if (IM_BACKUP_CBT == vol->rq->pg->backup_status)
    {
        goto end;
    }
    
    if (NULL == vol->bitmap_snapshot) /* 无快照位图，错误分支 */
    {
        vol->rq->pg->backup_status = IM_BACKUP_CBT;
        goto end;
    }
 
    for (pos = start; pos < start + size; pos += blocksize)
    {
        // TODO: 如支持可写虚拟快照，需要继续检测snapshot位图
        if ( (!IsBitmapBitSet(vol->bitmap, pos)) && 
            ( (NULL == vol->bitmap_filter) || (IsBitmapBitSet(vol->bitmap_filter, pos)) ) )
        {
            cmd_old = cmd;
            cmd = im_alloc_cmd4diskio(vol, pos, 0);
            if (unlikely(NULL == cmd))
            {
                IOMIRROR_ERR("im_alloc_cmd4diskio failed.");
                vol->rq->pg->backup_status = IM_BACKUP_CBT;
                break;
            }
        }
 
        if (NULL != cmd_old)
        {
            down(&(vol->rq->lock));
            list_add_tail(&(cmd_old->list), &(vol->rq->head));
            vol->rq->count++;
            up(&(vol->rq->lock));
            cmd_old = NULL;
        }
    }
 
    if (NULL == cmd)
    {
        goto end;
    }
    
    cmd->completion = completion; /* 在最后一个cmd上设置completion */
    down(&(vol->rq->lock));
    list_add_tail(&(cmd->list), &(vol->rq->head));
    vol->rq->count++;
    up(&(vol->rq->lock));
    ret = 0;
 
end:;
    BitmapSetBit(vol->bitmap, start, size);
    return ret;
}
 
/**
 * Description : IOMirror make_request_fn for backup，
 *               用于替换原mfn，实现I/O分流
 *
 * Parameters  : q  
 *               bio
 * Return      :
 *
 * Author      : Hou Jie/h00434839
 * Date        : 2018/8/16
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
void im_backup_make_request_fn(struct request_queue *q, struct bio *bio)
#else
int im_backup_make_request_fn(struct request_queue *q, struct bio *bio)
#endif
{
    struct im_volume *vol = NULL;
    make_request_fn *ori_mfn = NULL;
    struct completion completion;
 
    ori_mfn = im_get_ori_mfn(bio->bi_bdev);
    if (unlikely(NULL == ori_mfn))
    {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
        bio_io_error(bio, bio->bi_size);
#else
        bio_io_error(bio);
#endif
        IOMIRROR_ERR("can NOT find original make request function, major=%u, minor=%u.",
                    MAJOR(bio->bi_bdev->bd_dev), MINOR(bio->bi_bdev->bd_dev));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
        return;
#else
        return -EIO;
#endif
    }
 
    /* 读I/O和不需要保护的写I/O直接透传 */
    if (READ == bio_data_dir(bio))
    {
        goto through;
    }
 
    vol = im_is_protect(bio);
    if (NULL == vol)
    {
        goto through;
    }
 
    /* 处理需要保护的写I/O */
    init_completion(&completion);
    if (im_alloc_cmds(bio, vol, &completion) < 0)
    {
        goto through;
    }
 
    wake_up_interruptible(&(vol->rq->wq));
    if (wait_for_completion_timeout(&completion, IM_BACKUP_IO_TIMEOUT) == 0)
    {
        vol->rq->pg->backup_status = IM_BACKUP_CBT;
        IOMIRROR_ERR("COW timeout.");
        goto through;
    }
 
through:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
    ori_mfn(q, bio);    
    return;
#else
    return ori_mfn(q, bio);
#endif
}
#endif //SUPPORT_BACKUP


/**
 * Description : 替换make_request_fn函数
 *
 * Parameters  : vol - 待替换make_request_fn函数的im_volume卷指针
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/25
 */
static int im_replace_make_request_fn(struct im_volume *vol)
{
    struct list_head *ptr = NULL;
    struct im_volume *p_vol = NULL;
    struct block_device *bdev = NULL;

    bdev = vol->bdev->bd_contains;

    /**
     * if条件成立: 当前vol所属磁盘的make_request_fn未被替换过，执行替换操作；
     * if条件不成立: 当前vol所属磁盘的make_request_fn已被替换，增加引用计数。
     */
#ifdef SUPPORT_BACKUP
    if ( (im_filter_make_request_fn != bdev->bd_disk->queue->make_request_fn) &&
         (im_backup_make_request_fn != bdev->bd_disk->queue->make_request_fn) )
#else
    if (im_filter_make_request_fn != bdev->bd_disk->queue->make_request_fn)
#endif
    {
        vol->ori_mfn= kzalloc(sizeof(struct im_disk_ori_mfn), GFP_KERNEL);
        if (NULL == vol->ori_mfn)
        {
            IOMIRROR_ERR("kzalloc for vol->disk failed.");
            return -1;
        }
        vol->ori_mfn->f = bdev->bd_disk->queue->make_request_fn;
        vol->ori_mfn->cnt = 1;
#ifdef SUPPORT_BACKUP
        bdev->bd_disk->queue->make_request_fn = (vol->rq->pg->backup_status == 0)? im_filter_make_request_fn : im_backup_make_request_fn;
#else
        bdev->bd_disk->queue->make_request_fn = im_filter_make_request_fn;
#endif
    }
    else
    {
        list_for_each(ptr, &im_vol_list_head)
        {
            p_vol = list_entry(ptr, struct im_volume, list0);

            if (vol->bd_disk_dev == p_vol->bd_disk_dev)
            {
                break;
            }
        }

        if (NULL != p_vol)
        {
            vol->ori_mfn = p_vol->ori_mfn;
            vol->ori_mfn->cnt++;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}


/**
 * Description : 还原make_request_fn函数
 *
 * Parameters  : vol - 待还原make_request_fn函数的im_volume卷指针
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/26
 */
static void im_restore_make_request_fn(struct im_volume *vol)
{
    struct block_device *bdev = NULL;

    vol->ori_mfn->cnt--;
    if (0 == vol->ori_mfn->cnt)
    {
        bdev = vol->bdev->bd_contains;
        bdev->bd_disk->queue->make_request_fn = vol->ori_mfn->f;
        kfree(vol->ori_mfn);
        vol->ori_mfn= NULL;
    }
}


/**
 * Description : 将被保护卷插入IOMirror被保护卷队列
 *
 * Parameters  : vol - 待插入的被保护im_volume卷指针
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/26
 */
static inline int im_insert_vol_to_list(struct im_volume *vol)
{
    int ret = 0;
    down_write(&im_vol_list_sem);
    ret = im_replace_make_request_fn(vol);
    if (0 == ret)
    {
        /* 为磁盘的卷头插入，提高透传请求处理效率 */
        if (vol->is_part)
        {
            list_add_tail(&(vol->list0), &im_vol_list_head);
        }
        else
        {
            list_add(&(vol->list0), &im_vol_list_head);
        }
    }
    up_write(&im_vol_list_sem);
    return ret;
}


/**
 * Description : 根据路径获得块设备的设备号，
 *               内核版本小于2.6.27时使用
 *
 * Parameters  : path - 待获取设备号的块设备文件路径
 *               dev - 返回所获得的设备号
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/9
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27))
static int im_lookup_device(const char *path, dev_t *dev)
{
    int r;
    struct nameidata nd;
    struct inode *inode;

    if ((r = path_lookup(path, LOOKUP_FOLLOW, &nd)))
    {
        return r;
    }

    inode = nd.dentry->d_inode;
    if (!inode)
    {
        r = -ENOENT;
        goto out;
    }

    if (!S_ISBLK(inode->i_mode))
    {
        r = -ENOTBLK;
        goto out;
    }

    *dev = inode->i_rdev;

out:
    path_release(&nd);
    return r;
}
#endif


/**
 * Description : 新添加被保护卷
 *
 * Parameters  : id  - 待添加卷的卷id
 *               path - 待添加卷的块设备文件路径
 *               sectors - 卷逻辑大小，单位扇区
 *               bitmap_granularity - 位表粒度
 *               rq - 待添加卷所属保护组的写请求队列指针
 * Return      : 成功，返回为新卷创建的im_volume结构指针
 *               失败，返回NULL
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/26
 */
struct im_volume *im_add_volume(const char *id,
                            const char *path, uint64_t sectors,
                            int bitmap_granularity, struct im_reqeust_queue *rq)
{
    int ret = 0;
    struct im_volume *volume  = NULL;
    struct block_device *bdev = NULL;

    volume = kzalloc(sizeof(struct im_volume), GFP_KERNEL);
    if (unlikely(NULL == volume))
    {
        IOMIRROR_ERR("kzalloc for volume failed, path=%s.", path);
        goto fail;
    }

    /* 根据内核版本，使用不同方法得到path所指向块设备文件的block_device结构 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27))
    ret = im_lookup_device(path, &(volume->bd_dev));
    if (ret < 0)
    {
        IOMIRROR_ERR("im_lookup_device failed, path=%s.", path);
        goto fail;
    }

    bdev = bdget(volume->bd_dev);
    if (NULL == bdev)
    {
        IOMIRROR_ERR("open volume(bdget) failed, path=%s.", path);
        goto fail;
    }
#else
    bdev = lookup_bdev(path);
    if (IS_ERR(bdev))
    {
        ret = PTR_ERR(bdev);
        IOMIRROR_ERR("open volume(lookup_bdev) failed, path=%s.", path);
        goto fail;
    }
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28) || LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 38))
    ret = blkdev_get(bdev, FMODE_READ, 0);
#else
    ret = blkdev_get(bdev, FMODE_READ);
#endif
    if (ret < 0)
    {
        IOMIRROR_ERR("blkdev_get failed, path=%s.", path);
        goto fail;
    }

    /* 初始化volume数据结构 */
    memcpy(volume->id, id, VOL_ID_LEN);
    strncpy(volume->path, path, DISK_PATH_LEN);
    volume->path[DISK_PATH_LEN - 1] = '\0';
    volume->bd_dev      = bdev->bd_dev;
    volume->bd_disk_dev = bdev->bd_contains->bd_dev;
    volume->bdev        = bdev;
    volume->ori_mfn     = NULL;
    volume->rq          = rq;
    if (bdev == bdev->bd_contains)
    {
        volume->is_part = false;
        volume->start   = 0;
        volume->end     = get_capacity(bdev->bd_disk) - 1;
        volume->sectors = get_capacity(bdev->bd_disk);
    }
    else
    {
        volume->is_part = true;
        volume->start   = bdev->bd_part->start_sect;
        volume->end     = volume->start + bdev->bd_part->nr_sects - 1;
        volume->sectors = bdev->bd_part->nr_sects;
    }
    IOMIRROR_INFO("volume path=%s, size=%llu, phy_size=%llu.",
                  path, sectors, (uint64_t)volume->sectors);
    if (sectors > 0)
    {
        volume->sectors = sectors;
        volume->end     = volume->start + volume->sectors - 1;
    }

#ifdef SUPPORT_BACKUP
    if (IM_NO_BACKUP_MODE == volume->rq->pg->backup_status)     /* 非备份模式 */
    {
#endif
        /* 初始化bitmap */
        volume->bitmap = BitmapAlloc(volume->sectors, bitmap_granularity, BitmapAllocFunc, BitmapFreeFunc);
        if (NULL == volume->bitmap)
        {
            IOMIRROR_ERR("BitmapAlloc for cbt bitmap failed.");
            goto fail_put;
        }

        volume->hbi_send = (OM_BITMAP_IT *)kzalloc(sizeof(OM_BITMAP_IT), GFP_KERNEL);
        if (NULL == volume->hbi_send)
        {
            IOMIRROR_ERR("kzalloc for hbi_send failed.");
            goto fail_put;
        }

        BitmapItInit(volume->hbi_send, volume->bitmap, 0);
#ifdef SUPPORT_BACKUP
    }
    else
    {
        volume->rq->pg->cur_volume_minor++;
        if (volume->rq->pg->cur_volume_minor > 127)
        {
            IOMIRROR_ERR("too many volumes.");
            goto fail_put;
        }
        volume->volume_minor = volume->rq->pg->cur_volume_minor;
    }
#endif

    /* 将创建好的volume插入全局被保护卷队列 */
    ret = im_insert_vol_to_list(volume);
    if (ret < 0)
    {
        IOMIRROR_ERR("im_insert_vol_to_list failed, path=%s.", path);
        goto fail_put;
    }

    //IOMIRROR_DBG("volume id=%s.", volume->id);
    //IOMIRROR_DBG("volume path=%s.", volume->path);
    //IOMIRROR_DBG("max_sectors=%u.", bdev->bd_disk->queue->max_sectors);
    //IOMIRROR_DBG("max_hw_sectors=%u.", bdev->bd_disk->queue->max_hw_sectors);
    //IOMIRROR_DBG("max_phys_segments=%u.", bdev->bd_disk->queue->max_phys_segments);
    //IOMIRROR_DBG("max_hw_segments=%u.", bdev->bd_disk->queue->max_hw_segments);
    //IOMIRROR_DBG("hardsect_size=%u.", bdev->bd_disk->queue->hardsect_size);
    //IOMIRROR_DBG("max_segment_size=%u.", bdev->bd_disk->queue->max_segment_size);

    return volume;

fail_put:

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28))
    blkdev_put(bdev);
#else
    blkdev_put(bdev, FMODE_READ);
#endif

fail:

    if (NULL != volume)
    {
        if (NULL != volume->hbi_send)
        {
            kfree(volume->hbi_send);
            volume->hbi_send = NULL;
        }

        if (NULL != volume->bitmap)
        {
            BitmapFree(volume->bitmap, BitmapFreeFunc);
            volume->bitmap = NULL;
        }

        kfree(volume);
        volume = NULL;
    }
    return NULL;
}


/**
 * Description : 删除被保护卷
 *
 * Parameters  : vol - 待删除卷的im_volume结构指针
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/26
 */
void im_del_volume(struct im_volume *vol)
{
    down_write(&im_vol_list_sem);
    im_restore_make_request_fn(vol);
    list_del_init(&(vol->list0));
    up_write(&im_vol_list_sem);

    /* 确保使用volume结构写请求完成，延时3s释放资源 */
    IOMIRROR_INFO("deleting volume path=%s...", vol->path);
    msleep(IM_VOLUME_DEL_DLAY);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28))
    blkdev_put(vol->bdev);
#else
    blkdev_put(vol->bdev, FMODE_READ);
#endif

    if (NULL != vol->hbi_send)
    {
        kfree(vol->hbi_send);
        vol->hbi_send = NULL;
    }
/*
    if (NULL != vol->bitmap_temp)
    {
        BitmapFree(vol->bitmap_temp);
        vol->bitmap_temp = NULL;
    }
*/
    if (NULL != vol->bitmap_original)
    {
        BitmapFree(vol->bitmap_original, BitmapFreeFunc);
        vol->bitmap_original = NULL;
    }
/*
    if (NULL != vol->bitmap_filter)
    {
        BitmapFree(vol->bitmap_filter);
        vol->bitmap_filter = NULL;
    }
    if (NULL != vol->bitmap_snapshot)
    {
        BitmapFree(vol->bitmap_snapshot);
        vol->bitmap_snapshot = NULL;
    }
*/
    if (NULL != vol->bitmap)
    {
        BitmapFree(vol->bitmap, BitmapFreeFunc);
        vol->bitmap = NULL;
    }

    if (NULL != vol->bitmap_verify)
    {
        BitmapFree(vol->bitmap_verify, BitmapFreeFunc);
        vol->bitmap_verify = NULL;
    }

#ifdef SUPPORT_BACKUP
    if (NULL != vol->bitmap_temp)
    {
        BitmapFree(vol->bitmap_temp, BitmapFreeFunc);
        vol->bitmap_temp = NULL;
    }
    if (NULL != vol->bitmap_original)
    {
        BitmapFree(vol->bitmap_original, BitmapFreeFunc);
        vol->bitmap_original = NULL;
    }
    if (NULL != vol->bitmap_filter)
    {
        BitmapFree(vol->bitmap_filter, BitmapFreeFunc);
        vol->bitmap_filter = NULL;
    }
    if (NULL != vol->bitmap_snapshot)
    {
        BitmapFree(vol->bitmap_snapshot, BitmapFreeFunc);
        vol->bitmap_snapshot = NULL;
    }
#endif
}

