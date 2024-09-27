/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : protect.c
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/18
 * Version     : 1.0
 *
 * Description : IOMirror保护组实现:
 *               1.保护组创建、删除
 *               2.增量队列
 *               3.增量CBT
 *               4.ATOMIC
 *               5.一致性校验
 *               6.网络重连、心跳
 *
 */

#include <linux/in.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/version.h>

#include "const.h"
#include "util.h"
#include "cmd_define.h"
#include "protect.h"
#include "ioctl.h"
#include "../share/ctl_define.h"

#define IM_PG_BIT_MASK(nr)        (1UL << ((nr) % BITS_PER_BYTE))
#define IM_PG_BIT_WORD(nr)        ((nr) / BITS_PER_BYTE)

static struct im_cmd* im_pg_check_one_dataset(struct im_cmd* dataset_head, unsigned short* p_dataset_status);
static struct im_cmd* im_pg_remove_one_dataset(struct im_pg_pending_queue* pq, struct im_cmd* pp_dataset_head);
static void im_pg_handle_dataset_remove_pending_queue(struct im_pg* pg, uint64_t current_request_id);
static void im_pg_get_vols_bitmap_size(struct im_pg *pg, uint64_t * cbt_size, uint64_t * verify_size);
static void im_pg_set_alarm_happen_time(struct im_pg * pg, int alarm_index);
static bool im_pg_if_alarm_report_time_arrive(struct im_pg * pg, int alarm_index);

const unsigned long MSEC_PER_SECONDS = 1000;
const int KB_UNIT = 1024;

LIST_HEAD(im_pg_list_head);

static inline void im_pg_free_cmd_status(struct im_cmd *cmd)
{
    if (cmd == NULL || cmd->status == NULL)
        return;

    if( 1 == atomic_cmpxchg(&(cmd->status->del_flag), 0, 1) )
    {
        kfree(cmd->status);
        cmd->status = NULL;
    }
}

/**
 * Description : 释放报文结构体
 *
 * Parameters  : cmd - 待释放的报文指针，调用者确保指针有效
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/27
 */
static inline void im_pg_cmd_free(struct im_cmd *cmd)
{
    int i = 0;

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

    if (NULL != cmd->buf)
    {
        vfree(cmd->buf);
        cmd->buf = NULL;
    }
    
    // free the im_cmd_status memory to avoid memory leap
    im_pg_free_cmd_status(cmd);

    kfree(cmd);
    cmd = NULL;
}

/**
 * Description : 清除控制报文pending queue，
 *               用于网络重连等场景使用
 *
 * Parameters  : pg - 待操作的cmd pending queue所在保护组指针
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/27
 */
void im_pg_clear_cmd_pending_queue(struct im_pg *pg)
{
    struct im_cmd *cmd = NULL;
    struct im_pg_cmd_pending_queue *cmd_pq = &(pg->cmd_pq);

    while (!list_empty(&(cmd_pq->head)))
    {
        cmd = list_first_entry(&(cmd_pq->head), struct im_cmd, list);
        list_del(&(cmd->list));
        cmd_pq->count--;

        im_pg_cmd_free(cmd);
        cmd = NULL;
    }
}

/**
 * Description : Only flush request queue to CBT bitmap
 *
 * Parameters  : pg - protect group; rq_page_count: the page counts of request queue in triggered time.
 * Return      : void
 *
 * Author      : z00455045
 * Date        : 2019.08.12
 */
void im_pg_flush_rq(struct im_pg *pg, uint64_t rq_page_count)
{
    struct im_cmd              *cmd = NULL;
    struct im_volume           *vol = NULL;
    struct im_reqeust_queue    *rq  = &(pg->rq);
    struct list_head           *ptr = NULL;
    uint64_t                   count = 0;

    /* 刷request queue至cbt位表 */
    while (count < rq_page_count && !list_empty(&(rq->head)))
    {
        down(&(rq->lock));
        cmd = list_first_entry(&(rq->head), struct im_cmd, list);
        list_del(&(cmd->list));
        rq->count--;
        rq->page_cnt -= cmd->vcnt;
        up(&(rq->lock));
        count ++;

        if (likely(DPP_TYPE_DATA == cmd->header.cmd_type))
        {
            vol = (struct im_volume *)(cmd->private);
            BitmapSetBit(vol->bitmap, cmd->header.data_offset,
                        cmd->header.data_size_bytes / IM_SECTOR_SIZE);
            // do not invoke im_pg_free_cmd_status() twice, otherwise it will crash
            // im_pg_free_cmd_status(cmd);
        }
        /* 丢弃一致性校验标签报文 */
        else if ( (DPP_TYPE_DATASET_START == cmd->header.cmd_type) || (DPP_TYPE_RESYNCSET_START == cmd->header.cmd_type) )
        {
            IOMIRROR_INFO("discard DPP_TYPE_DATASET_START cmd.");
        }
        else
        {
            IOMIRROR_INFO("error cmd in request queue, type=%d.", cmd->header.cmd_type);
        }

        im_pg_cmd_free(cmd);
        cmd = NULL;
    }

    /* 若当前为CBT状态，则刷新CBT数据发送位表bitmap_send */
    if (IM_PG_STATE_CBT == pg->state)
    {
        list_for_each(ptr, &(pg->vols))
        {
            vol = list_entry(ptr, struct im_volume, list1);
            BitmapItInit(vol->hbi_send, vol->bitmap, 0);
        }
    }

    // do not fresh pg->verify_bitmap_total_bytes here
    im_pg_get_vols_bitmap_size(pg, &pg->cbt_bitmap_total_bytes, &count);
    pg->cbt_bitmap_send_sum = 0;  // every re-enter CBT, synced and remain data count again 
}

/**
 * Description : 将request queue和pending queue中的请求刷至cbt位表
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/31
 */
void im_pg_flush_rq_and_pq(struct im_pg *pg)
{
    struct im_cmd              *cmd = NULL;
    struct im_volume           *vol = NULL;
    struct im_reqeust_queue    *rq  = &(pg->rq);
    struct im_pg_pending_queue *pq  = &(pg->pq);
    struct list_head           *ptr = NULL;

    /* 刷request queue至cbt位表 */
    while (!list_empty(&(rq->head)))
    {
        down(&(rq->lock));
        cmd = list_first_entry(&(rq->head), struct im_cmd, list);
        list_del(&(cmd->list));
        rq->count--;
        rq->page_cnt -= cmd->vcnt;
        up(&(rq->lock));

        if (likely(DPP_TYPE_DATA == cmd->header.cmd_type))
        {
            vol = (struct im_volume *)(cmd->private);
            BitmapSetBit(vol->bitmap, cmd->header.data_offset,
                        cmd->header.data_size_bytes / IM_SECTOR_SIZE);
            // im_pg_free_cmd_status will be invoked in im_pg_cmd_free(), do not invoke twice in work thread otherwise VM may be crash
            // im_pg_free_cmd_status(cmd);
        }
        /* 丢弃一致性校验标签报文 */
        else if ( (DPP_TYPE_DATASET_START == cmd->header.cmd_type) || (DPP_TYPE_RESYNCSET_START == cmd->header.cmd_type) )
        {
            IOMIRROR_INFO("discard DPP_TYPE_DATASET_START cmd.");
        }
        else
        {
            IOMIRROR_INFO("error cmd in request queue, type=%d.", cmd->header.cmd_type);
        }

        im_pg_cmd_free(cmd);
        cmd = NULL;
    }

    // under IM_PG_STATE_VERIFY, pending queue data shuold be flushed to verfy_bitmap in IM_PG_INTER_STATE_VERIFY_START
    // and if pg->exit_flag is not in running, should flush pending queue data into bitmap.
    if (pg->state == IM_PG_STATE_VERIFY && !pg->exit_flag) {
        return;
    }

    /* 刷pending queue至cbt位表 */
    while (!list_empty(&(pq->head)))
    {
        cmd = list_first_entry(&(pq->head), struct im_cmd, list);
        list_del(&(cmd->list));
        pq->count--;
        pq->page_cnt -= cmd->vcnt;

        if (likely(DPP_TYPE_DATA == cmd->header.cmd_type))
        {
            vol = (struct im_volume *)(cmd->private);
            BitmapSetBit(vol->bitmap, cmd->header.data_offset,
                        cmd->header.data_size_bytes / IM_SECTOR_SIZE);
            // do not invoke im_pg_free_cmd_status() twice, otherwise it will crash
            // im_pg_free_cmd_status(cmd);
        }
        /* 丢弃一致性校验标签报文 */
        else if ( (DPP_TYPE_DATASET_START == cmd->header.cmd_type) || (DPP_TYPE_RESYNCSET_START == cmd->header.cmd_type) )
        {
            IOMIRROR_INFO("discard pending DPP_TYPE_DATASET_START cmd, req=%llu.", cmd->header.request_id);
        }
        else
        {
            IOMIRROR_WARN("error cmd in pending queue, type=%d.", cmd->header.cmd_type);
        }

        im_pg_cmd_free(cmd);
        cmd = NULL;
    }

    /* 若当前为CBT状态，则刷新CBT数据发送位表bitmap_send */
    if (IM_PG_STATE_CBT == pg->state)
    {
        list_for_each(ptr, &(pg->vols))
        {
            vol = list_entry(ptr, struct im_volume, list1);
            BitmapItInit(vol->hbi_send, vol->bitmap, 0);
        }
    }
}

static inline void im_pg_reset_failed_ack_count(struct im_pg *pg)
{
    pg->failed_ack_count = 0; // 收到了返回正确flag的ack，不用延迟发送dataset了
    pg->last_failed_ack_count = 0;
}

static inline void im_pg_bvl_free(struct bio_vec *bvl, unsigned short vcnt)
{
    int i = 0;
    struct bio_vec *bvec = bvl;

    for (i = 0; i < vcnt; i++) {
        if (NULL != bvec->bv_page) {
            __free_page(bvec->bv_page);
            bvec->bv_page = NULL;
        }
        bvec++;
    }

    if (bvl) {
        kfree(bvl);
    }
    bvl = NULL;
}

inline void im_pg_count_rw_iops_and_wsize(uint64_t write_size)
{
    struct im_pg *pg;

    if (likely(!list_empty(&im_pg_list_head))) {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
    } else {
        return;
    }

    pg->real_time_write_io ++;
    pg->write_io_size += write_size;
}


/**
 * Description : statistics item -- real RPO time, Target: get the time of first cmd that has not receive ack
 *
 * Parameters  : pg -- driver protect group
 *              
 * Return      : void
 *
 * Author      : z00455045
 * Date        : 2019/10/17
 */
static inline void im_pg_get_first_no_ack_cmd_time(struct im_pg* pg)
{
    struct im_cmd *p_cmd;

    list_for_each_entry(p_cmd, &(pg->pq.head), list) {
        if (p_cmd->header.cmd_type == DPP_TYPE_DATA && p_cmd->has_received != DATASET_HAS_RECEIVED) {
            pg->statistics_jiffies.rpo_time = p_cmd->send_time;
            return; 
        }
    }

    pg->statistics_jiffies.rpo_time = jiffies;
}

/**
 * Description : DPP_TYPE_DATASET_DONE报文处理函数
 *
 * Parameters  : pg - 所属保护组
 *               cmd - 待处理报文
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/23
 */
static void im_pg_handle_dataset_done_cmd(
                                        struct im_pg *pg, struct im_cmd *cmd)
{
    int found_dataset = 0;
    struct im_cmd *p_cmd = NULL;
    struct im_pg_pending_queue *pq = &(pg->pq);

    IOMIRROR_INFO("got DPP_TYPE_DATASET_DONE cmd, "
                  "request_id=%llu, data_offset=%llu, data_size=%llu, "
                  "host_id=%llx, vol_id=%llx, ack_result=%d, handle it...",
                  cmd->header.request_id, cmd->header.data_offset,
                  cmd->header.data_size_bytes, *(long long *)cmd->header.host_id,
                  *(long long *)cmd->header.vol_id, cmd->header.ack_result);

    if (list_empty(&(pq->head)))
    {
        found_dataset = 1; // discard this DPP_TYPE_DATASET_DONE cmd directly
        goto not_found;
    }

    p_cmd = list_first_entry(&(pq->head), struct im_cmd, list);
    if (DPP_TYPE_DATASET_START != p_cmd->header.cmd_type)
    {
        IOMIRROR_INFO("cmd type %u is not DPP_TYPE_DATASET_START.", p_cmd->header.cmd_type);
        goto not_found;
    }

    // initial
    // 此处必须保证收到的dataset done的request id是未收到的dataset的ID
    // 如发了dataset start request=5\6\7,那么收到的必须是5，如果收到的是6
    // 则说明5是没有收到的，也就是远端不一定是落盘了，则进入重连状态
    // 2019-07-16 同步windows的逻辑，发了dataset start request=5\6\7,
    // 如果收到的是4,则说明driver已经做了flush，此种情况忽略4的包，直接成功

    if (p_cmd->header.request_id > cmd->header.request_id)
    {
        IOMIRROR_WARN("cmd req %llu is less than list req %llu, the cmd req have flushed, ignore this dataset.",
                    cmd->header.request_id, p_cmd->header.request_id);
        return;
    }

    pg->cbt_flush_times_done = cmd->header.request_id; // last received dataset_done
    found_dataset = 1;
    im_pg_handle_dataset_remove_pending_queue(pg, cmd->header.request_id);


not_found:
    if (0 == found_dataset)
    {
        IOMIRROR_INFO("no found DATA SET cmd, reconnecting.");
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_flush_rq_and_pq(pg);
/*
#if 0
        //刷pending queue至cbt位表
        while (!list_empty(&(pq->head)))
        {
            cmd = list_first_entry(&(pq->head), struct im_cmd, list);
            list_del(&(cmd->list));
            pq->count--;
            //pq->page_cnt -= cmd->vcnt;

            if (likely(DPP_TYPE_DATA == cmd->header.cmd_type))
            {
                vol = (struct im_volume *)(cmd->private);
                hbitmap_set(vol->bitmap, cmd->header.data_offset,
                            cmd->header.data_size_bytes / IM_SECTOR_SIZE);
            }
            //丢弃一致性校验标签报文
            else if ( (DPP_TYPE_DATASET_START == cmd->header.cmd_type) || (DPP_TYPE_RESYNCSET_START == cmd->header.cmd_type) )
            {
                IOMIRROR_INFO("discard pending DPP_TYPE_DATASET_START cmd.");
            }
            else
            {
                IOMIRROR_WARN("error cmd in pending queue, type=%d.", cmd->header.cmd_type);
            }

            im_pg_cmd_free(cmd);
            cmd = NULL;
        }

        //若当前状态为cbt或verify，则无需切换至cbt状态
        if (IM_PG_STATE_CBT != pg->state
            && IM_PG_STATE_VERIFY != pg->state)
        {
            pg->state = IM_PG_STATE_CBT;

            //建立连接过程中不切换内部状态
            if (IM_PG_INTER_STATE_CONNECT_STAGE0 != pg->inter_state
                && IM_PG_INTER_STATE_CONNECT_STAGE0_P != pg->inter_state
                && IM_PG_INTER_STATE_CONNECT_STAGE1 != pg->inter_state
                && IM_PG_INTER_STATE_CONNECT_STAGE1_P != pg->inter_state
                && IM_PG_INTER_STATE_PAUSE != pg->inter_state
                && IM_PG_INTER_STATE_WAIT_PENDING_VOLS != pg->inter_state)
            {
                pg->inter_state = IM_PG_INTER_STATE_CBT_START;
                im_pg_clear_cmd_pending_queue(pg);
            }

            //若为PAUSE状态，则设置临时状态
            if (IM_PG_INTER_STATE_PAUSE == pg->inter_state)
            {
                pg->temp_state = IM_PG_INTER_STATE_CBT_START;
                im_pg_clear_cmd_pending_queue(pg);
            }
        }
#endif
*/
    }
    else
    {
        // 刷新收到的dataset ID
        pg->cbt_flush_times_done = cmd->header.request_id;
    }

}


/**
 * Description  : check all bio status in the dataset
 *
 * Parameters   : dataset_head -- the head of dataset
 *                p_dataset_status -- status of dataset(if one bio is not success the dataset can not be deleted)
 *
 * Return       :  struct im_cmd* : pointer to dataset that will be deleted or next dataset is going to check bio status
 *
 * Author       : d00418490
 * Date         : 2019/07/16
 */
static struct im_cmd * im_pg_check_one_dataset(struct im_cmd * dataset_head, unsigned short* p_dataset_status)
{
    struct list_head* data_list;
    struct im_cmd* data_entry;
    uint64_t cur_request_id = dataset_head->header.request_id;

    *p_dataset_status = LOCAL_BIO_SUCCESS;
    data_list = &(dataset_head->list);
    list_for_each_entry(data_entry, data_list, list)
    {
        if(NULL == data_entry->status &&  data_entry->header.cmd_type != DPP_TYPE_DATASET_START)
        {
            continue; 
        }

        if(DPP_TYPE_DATASET_START == data_entry->header.cmd_type && data_entry->header.request_id > cur_request_id)
        {
            //find next dataset
            break;
        }

        if(LOCAL_BIO_ONGOING == data_entry->status->related_local_bio_complete)
        {
            //find one bio ongoing
            *p_dataset_status = LOCAL_BIO_ONGOING;
            continue; // may be the dataset has a bio failure already.
        }
        else if(LOCAL_BIO_FAILURE == data_entry->status->related_local_bio_complete)
        {
            //find one bio failed
            *p_dataset_status = LOCAL_BIO_FAILURE;
            break;
        }
        else
        {
            continue;
        }
    }

    if (*p_dataset_status == LOCAL_BIO_ONGOING)
    {
        IOMIRROR_INFO("found DATA SET: %llu, there are bio not finished, will be checked next time", dataset_head->header.request_id);
        return data_entry;// next dataset that will be checked  bio status.
    }

    return NULL; //LOCAL_BIO_FAILURE or LOCAL_BIO_SUCCESS may return this value

}

/**
 * Description  : remove the dataset from pending queue
 *
 * Parameters   : pq -- pending queue
 *                dataset_head -- the head of dataset
 * Return       : struct im_cmd* : pointer to the next dataset_start 
 *
 * Author       : d00418490
 * Date         : 2019/07/16
 */
static struct im_cmd* im_pg_remove_one_dataset(struct im_pg_pending_queue* pq, struct im_cmd* pp_dataset_head)
{
    struct list_head* data_list = NULL;
    struct im_cmd* data_entry = NULL;
    struct im_cmd* tmp_entry = NULL;
    uint64_t cur_request_id = 0;

    data_list = &(pp_dataset_head->list);
    data_list = data_list->prev;
    cur_request_id = pp_dataset_head->header.request_id;

    list_for_each_entry_safe(data_entry, tmp_entry, data_list, list)
    {
        if(DPP_TYPE_DATASET_START == data_entry->header.cmd_type && data_entry->header.request_id > cur_request_id)
        {
            //find next dataset
            return data_entry;
        }

        list_del(&(data_entry->list));
        pq->count--;
        if(NULL != data_entry) {
            im_pg_cmd_free(data_entry);
        }
    }
  
    return NULL;
}

/**
 * Description  : every DPP_TYPE_DATASET_DONE triggers check and remove of all previous dataset
 *                if all bio of dataset is success, the dataset will be removed from pending queue
 *                if dataset contains bio in ongoing status, the dataset will be kept in pending queue
 *                if dataset contains bio in failed status, the dataset will be kept in pending queue and driver will transfer into CBT mode
 * Parameters   : pq -- pending queue
 *                current_request_id -- request id of DPP_TYPE_DATASET_DONE cmd
 * Return       : void
 *
 * Author       : d00418490
 * Date         : 2019/07/16
 */
static void im_pg_handle_dataset_remove_pending_queue(struct im_pg* pg, uint64_t current_request_id)
{
    bool dataset_io_status = true;
    uint32_t free_cmd_number = 0;
    uint32_t ongoing_cmd_number = 0;
    struct im_cmd* dataset_head = NULL;
    struct im_cmd* temp_dataset_head = NULL;
    struct im_pg_pending_queue* pq = &(pg->pq);
    struct list_head* total_list = &(pq->head);

    dataset_head = list_first_entry(total_list, struct im_cmd, list);
    while (!list_empty(total_list))
    {
        unsigned short dataset_status = 0;

        if (NULL == dataset_head)
        {
            IOMIRROR_ERR("pending queue first entry is null.");
            return;
        }

        if (dataset_head->header.cmd_type != DPP_TYPE_DATASET_START) {
            IOMIRROR_ERR("dataset %llu cmd type is %d.", dataset_head->header.request_id, dataset_head->header.cmd_type);
            return;
        }

        // current_request_id must has received already
        if (dataset_head->header.request_id == current_request_id) {
            dataset_head->has_received = DATASET_HAS_RECEIVED;
        }

        if (DATASET_HAS_RECEIVED != dataset_head->has_received && dataset_head->header.request_id < current_request_id)
        {
            IOMIRROR_INFO("Dataset %llu has not received, but now revice dataset %llu.", dataset_head->header.request_id, current_request_id);
            dataset_io_status = false;
            break;
        }

        if (DPP_TYPE_DATASET_START == dataset_head->header.cmd_type && dataset_head->header.request_id > current_request_id ) // dataset yet to be confirmed from remote point
        {
            IOMIRROR_INFO("found DATA SET: %llu, yet to be confirmed from remote point", dataset_head->header.request_id);
            break;
        }

        // check bio completion in dataset, if current dataset_head is LOCAL_BIO_ONGOING, it will return next dataset start cmd
        temp_dataset_head = im_pg_check_one_dataset(dataset_head, &dataset_status);
        if(LOCAL_BIO_SUCCESS == dataset_status)
        {
            free_cmd_number ++;
            IOMIRROR_INFO("delete data set, which id = %llu ", dataset_head->header.request_id);
            // im_pg_remove_one_dataset will delete a dataset, and return the dataset_head to the next dataset start cmd.
            temp_dataset_head = im_pg_remove_one_dataset(pq, dataset_head);
            dataset_head = temp_dataset_head;
        }
        else if(LOCAL_BIO_ONGOING == dataset_status)
        {
            ongoing_cmd_number ++;
            dataset_head = temp_dataset_head;
        }
        else
        {
            IOMIRROR_INFO("found DATA SET: %llu, there are bio failed, will enter CBT mode", dataset_head->header.request_id);
            dataset_io_status = false;
            break;
        }
    }

    IOMIRROR_INFO("total %u cmd data are free, ongoing cmds is %u.", free_cmd_number, ongoing_cmd_number);

    if(false == dataset_io_status)
    {
        //enter CBT mode
        IOMIRROR_INFO("enter CBT mode ");
        im_pg_flush_rq_and_pq(pg);
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    im_pg_reset_failed_ack_count(pg); // receive right ack
}

/**
 * Description : DPP_TYPE_RESYNCSET_DONE报文处理函数
 *
 * Parameters  : pg - 所属保护组
 *               cmd - 待处理报文
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/23
 */
static void im_pg_handle_resyncset_done_cmd(
                                        struct im_pg *pg, struct im_cmd *cmd)
{
    int i = 0, found_resyncset = 0;
    struct im_cmd *p_cmd = NULL;
    struct im_pg_pending_queue *pq = &(pg->pq);

    IOMIRROR_INFO("got DPP_TYPE_RESYNCSET_DONE cmd, "
                  "request_id=%llu, data_offset=%llu, data_size=%llu, "
                  "host_id=%llx, vol_id=%s, ack_result=%d, handle it...",
                  cmd->header.request_id, cmd->header.data_offset,
                  cmd->header.data_size_bytes, *(long long *)cmd->header.host_id,
                  cmd->header.vol_id, cmd->header.ack_result);

    if (list_empty(&(pq->head)))
    {
        pg->cbt_flush_times_done = cmd->header.request_id; // update received dataset_done
        return; // discard received DPP_TYPE_RESYNCSET_DONE
    }

    while (!list_empty(&(pq->head)))
    {
        p_cmd = list_first_entry(&(pq->head), struct im_cmd, list);
        if (found_resyncset)
        {
            if ( (DPP_TYPE_RESYNCSET_START == p_cmd->header.cmd_type) ||
                (DPP_TYPE_DATASET_START == p_cmd->header.cmd_type) ) // found next resyncset or dataset
            {
                IOMIRROR_INFO("found RESYNC SET: %llu, freed %d data cmd.", cmd->header.request_id, i);
                break;
            }

            list_del(&(p_cmd->list));
            pq->count--;
            i++;
            im_pg_cmd_free(p_cmd);
        }
        else
        {
            // if received DPP_TYPE_RESYNCSET_START is less than the first DPP_TYPE_RESYNCSET_START of pending queue, 
            // it should be success, not enter IM_PG_INTER_STATE_CONNECT_STAGE0.
            if ((p_cmd->header.request_id > cmd->header.request_id) &&
                    (p_cmd->header.cmd_type == DPP_TYPE_RESYNCSET_START) ) {
                IOMIRROR_INFO("cmd req %llu, list req %llu, type %u, should not enter IM_PG_INTER_STATE_CONNECT_STAGE0.",
                    cmd->header.request_id, p_cmd->header.request_id, p_cmd->header.cmd_type);
                return;
            }

            if ((DPP_TYPE_RESYNCSET_START != p_cmd->header.cmd_type) ||
                (p_cmd->header.request_id != cmd->header.request_id))
            {
                IOMIRROR_INFO("cmd req %llu, list req %llu, type %u.",
                    cmd->header.request_id, p_cmd->header.request_id, p_cmd->header.cmd_type);
                break;
            }

            pg->cbt_flush_times_done = cmd->header.request_id; // update received dataset_done
            list_del(&(p_cmd->list));
            pq->count--;
            i++;
            im_pg_cmd_free(p_cmd);
            found_resyncset = 1;
        }
    }

    if (0 == found_resyncset)
    {
        IOMIRROR_INFO("no found RESYNC SET cmd, reconnecting.");
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_flush_rq_and_pq(pg);
        im_pg_clear_cmd_pending_queue(pg);
        /*
#if 0
        // 刷pending queue至cbt位表
        while (!list_empty(&(pq->head)))
        {
            cmd = list_first_entry(&(pq->head), struct im_cmd, list);
            list_del(&(cmd->list));
            pq->count--;
            //pq->page_cnt -= cmd->vcnt;

            if (likely(DPP_TYPE_DATA == cmd->header.cmd_type))
            {
                vol = (struct im_volume *)(cmd->private);
                if (IM_PG_STATE_VERIFY == pg->state)
                    bitmap = vol->bitmap_verify;
                else
                    bitmap = vol->bitmap;

                if (bitmap)
                    hbitmap_set(bitmap, cmd->header.data_offset,
                            cmd->header.data_size_bytes / IM_SECTOR_SIZE);
            }
            // 丢弃一致性校验标签报文
            else if (DPP_TYPE_RESYNCSET_START == cmd->header.cmd_type)
            {
                if (pg->max_dataset_size != pg->sent_amount)
                {
                    pg->next_vol_offset = cmd->header.data_offset;
                    pg->sent_amount = pg->max_dataset_size; // set sent_amount so to send very beginning dataset
                }
            }
            else if (DPP_TYPE_DATASET_START == cmd->header.cmd_type)
            {
                IOMIRROR_INFO("discard pending DPP_TYPE_DATASET_START cmd.");
            }
            else
            {
                IOMIRROR_WARN("error cmd in pending queue, type=%d.", cmd->header.cmd_type);
            }

            im_pg_cmd_free(cmd);
            cmd = NULL;
        }

        if (IM_PG_STATE_VERIFY == pg->state)
        {
            // 建立连接过程中不切换内部状态
            if (IM_PG_INTER_STATE_CONNECT_STAGE0 != pg->inter_state
                && IM_PG_INTER_STATE_CONNECT_STAGE0_P != pg->inter_state
                && IM_PG_INTER_STATE_CONNECT_STAGE1 != pg->inter_state
                && IM_PG_INTER_STATE_CONNECT_STAGE1_P != pg->inter_state
                && IM_PG_INTER_STATE_PAUSE != pg->inter_state
                && IM_PG_INTER_STATE_WAIT_PENDING_VOLS != pg->inter_state)
            {
                pg->inter_state = IM_PG_INTER_STATE_VERIFY_START;
                im_pg_clear_cmd_pending_queue(pg);
            }

            // 若为PAUSE状态，则设置临时状态
            if (IM_PG_INTER_STATE_PAUSE == pg->inter_state)
            {
                pg->temp_state = IM_PG_INTER_STATE_VERIFY_START;
                im_pg_clear_cmd_pending_queue(pg);
            }
        }
#endif
*/
    }else {
        im_pg_reset_failed_ack_count(pg); // receive a right resync ACK
    }
}


/**
 * Description : 返回字节中的一个为1的位
 *
 * Parameters  : c - 待检查的字符
 * Return      : 输入字节中下一个为1位的位置，全零返回-1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/30
 */
static inline int im_pg_get_next_bit(unsigned char c)
{
    if (0 == c)
    {
        return -1;
    }
    if (c & 0x01)
    {
        return 0;
    }
    if (c & 0x02)
    {
        return 1;
    }
    if (c & 0x04)
    {
        return 2;
    }
    if (c & 0x08)
    {
        return 3;
    }
    if (c & 0x10)
    {
        return 4;
    }
    if (c & 0x20)
    {
        return 5;
    }
    if (c & 0x40)
    {
        return 6;
    }
    if (c & 0x80)
    {
        return 7;
    }
    return -1;
}


/**
 * Description : 设置字节中某一位为1
 *
 * Parameters  : nr - 待设置的bit位置
 *               addr - 待设置的字节地址
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/30
 */
static inline void im_pg_reset_bit(int nr, unsigned char* addr)
{
    uint8_t* p = addr;
    uint8_t mask = ~(1U << nr);
    *p &= mask;
}


/**
 * Description : 将收到的位表与数据卷当前位表合并
 *
 * Parameters  : granularity - hbitmap粒度
 *               cmd - 收到的bitmap报文
 *               vol - 待合并位表的卷
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/30
 */
bool im_pg_merge_bitmap(struct im_volume *vol, unsigned char *bitmap, unsigned int len, uint64_t bit_count)
{
    bool ret = MergeBitmapByBuffer(vol->bitmap, bitmap, len);

    IOMIRROR_INFO("vol bitmap count=%llu, bit=%llu, len=%u.", GetBitmapCount(vol->bitmap), bit_count, len);
#ifdef DBG_OM_BITMAP
    if (GetBitmapCount(vol->bitmap) != bit_count)
    {
        RaiseException();
    }
#else
    bit_count = 0;
#endif

    return ret;

    /*
    int bit = 0;
    unsigned char *cur  = bitmap;
    unsigned int pos    = 0;
    uint64_t sector_num = 0;
    uint64_t nb_sectors = 1U << granularity;

    IOMIRROR_INFO("enter im_pg_merge_bitmap.");

    //对于page中的每一个字节循环一次
    while (len > 0)
    {
        while (1)
        {
            //找到当前字节中的下一个为1的位，设置cbt位表
            bit = im_pg_get_next_bit(*cur);
            if (-1 == bit)
            {
                break;
            }

            sector_num = nb_sectors * (8 * pos + bit);
            if (vol->sectors > sector_num + nb_sectors)
            {
                BitmapSetBit(vol->bitmap, sector_num, nb_sectors);
            }
            else
            {
                BitmapSetBit(vol->bitmap, sector_num, vol->sectors - sector_num);
                break;
            }

            im_pg_reset_bit(bit, cur);
        }
        pos++;
        cur++;
        len--;
    }
    */
}


/**
 * Description : DPP_TYPE_SESSION_LOGIN_ACK报文处理函数
 *
 * Parameters  : pg - 所属保护组
 *               cmd - 待处理报文
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static void im_pg_handle_session_login_ack_cmd(
                                        struct im_pg *pg, struct im_cmd *cmd)
{
    struct im_cmd *p_cmd = NULL;
    struct list_head *ptr = NULL;
    struct im_pg_cmd_pending_queue *cmd_pq = &(pg->cmd_pq);
    uint64_t dataset_sent = 0, dataset_done = 0;
    PDPP_SESSION_LOGIN_ACK_AUX p_session_login_ack_aux = NULL;

    IOMIRROR_INFO("got DPP_TYPE_SESSION_LOGIN_ACK cmd, "
                  "request_id=%llu, data_offset=%llu, data_size=%llu, "
                  "host_id=%llx, vol_id=%s, ack_result=%d, handle it...",
                  cmd->header.request_id, cmd->header.data_offset,
                  cmd->header.data_size_bytes, *(long long *)cmd->header.host_id,
                  cmd->header.vol_id, cmd->header.ack_result);

    if (IM_PG_INTER_STATE_CONNECT_STAGE0_P != pg->inter_state)
    {
        IOMIRROR_WARN("current internal state=%d, "
                "but received DPP_TYPE_SESSION_LOGIN_ACK cmd.", pg->inter_state);
        return;
    }

    if (NULL == cmd->buf)
    {
        IOMIRROR_ERR("cmd->buf is null.");
        return;
    }

    p_session_login_ack_aux = (PDPP_SESSION_LOGIN_ACK_AUX)cmd->buf;
    dataset_sent = be64_to_cpu(p_session_login_ack_aux->dataset_id_sent);
    dataset_done = be64_to_cpu(p_session_login_ack_aux->dataset_id_done);

    // 判断pg的dataset状态
    if (pg->cbt_flush_times_done > dataset_done)
    {
        IOMIRROR_ERR("Dataset_id_done %llu is higher than received one %llu, change to verify mode.", pg->cbt_flush_times_done, dataset_done);

        // 进入verify状态，重新开始同步
        pg->state = IM_PG_STATE_VERIFY;
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    // linux在发送的时候使用+1处理，判断是需要使用小于，不能小于等于
    if (pg->cbt_flush_times < dataset_sent)
    {
        IOMIRROR_ERR("Dataset_id %llu is less than received one %llu, change to verify mode", pg->cbt_flush_times, dataset_sent);

        // 进入verify状态，重新开始同步，使用login ack中的dataset id进行初始化
        pg->cbt_flush_times = dataset_sent + 1;
        pg->cbt_flush_times_done = dataset_done;
        pg->state = IM_PG_STATE_VERIFY;
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    if (list_empty(&(cmd_pq->head)))
    {
        IOMIRROR_WARN("empty cmd pendding queue, but received DPP_TYPE_SESSION_LOGIN_ACK cmd, "
                      "request_id=%llu.", cmd->header.request_id);
        return;
    }

    pg->next_vol_offset = 0;
    pg->total_credit = cmd->header.data_size_bytes;
    pg->max_dataset_size = cmd->header.data_offset; // use data_offset to pass max_dataset_size
    pg->sent_amount = pg->max_dataset_size / 2; // set sent_amount so to send very beginning dataset

    /* 在cmd pending queue中寻找对应的报文 */
    list_for_each(ptr, &(cmd_pq->head))
    {
        p_cmd = list_entry(ptr, struct im_cmd, list);
        break;

        // 20190729 当前只有登录cmd放到cmd_pq中，cmd_pq在iomirror中的目的是
        // 登录或者其他命令放到queue中
        // 1.超时可以使用cmd_pq去重复登录包
        // 2.怀疑iomirror中为了支持一个host支持多个目的端的场景，当前Mobility不支持，注释此代码
        /*
        if (p_cmd->header.request_id == cmd->header.request_id
            && 0 == memcmp(p_cmd->header.vol_id, cmd->header.vol_id, VOL_ID_LEN))
        {
            break;
        }
        else
        {
            p_cmd = NULL;
        }
        */
    }

    if (NULL != p_cmd)
    {
        list_del(&(p_cmd->list));
        cmd_pq->count--;
        kfree(p_cmd);
        p_cmd = NULL;
        /* 收到所有CONNECT_VRG_ACK则切换至下一状态 */
        if (list_empty(&(cmd_pq->head)))
        {
            pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE1;
        }
        else
        {
            list_for_each(ptr, &(cmd_pq->head))
            {
                p_cmd = list_entry(ptr, struct im_cmd, list);
                IOMIRROR_WARN("cmd req %llu, cmd %u.", p_cmd->header.request_id, p_cmd->header.cmd_type);
            }

        }
    }
    else
    {
        IOMIRROR_WARN("can NOT find the corresponding cmd in cmd pendding queue,"
                  " but received DPP_TYPE_SESSION_LOGIN_ACK cmd, request_id=%llu.",
                  cmd->header.request_id);
    }
}


/**
 * Description : IM_CMD_PAUSE报文处理函数
 *
 * Parameters  : pg - 所属保护组
 *               cmd - 待处理报文
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/29
 */
static void im_pg_handle_credit_cmd(
                                        struct im_pg *pg, struct im_cmd *cmd)
{
    IOMIRROR_INFO("got DPP_TYPE_CREDIT cmd, "
                  "request_id=%llu, data_offset=%llu, data_size=%llu, "
                  "host_id=%llx, vol_id=%s, ack_result=%d, handle it...",
                  cmd->header.request_id, cmd->header.data_offset,
                  cmd->header.data_size_bytes, *(long long *)cmd->header.host_id,
                  cmd->header.vol_id, cmd->header.ack_result);

    /* 网络重连时不处理该报文，直接丢弃 */
    if (IM_PG_INTER_STATE_CONNECT_STAGE0 == pg->inter_state
        || IM_PG_INTER_STATE_CONNECT_STAGE0_P == pg->inter_state
        || IM_PG_INTER_STATE_CONNECT_STAGE1 == pg->inter_state
        || IM_PG_INTER_STATE_CONNECT_STAGE1_P == pg->inter_state)
    {
        IOMIRROR_WARN("current internal state is CONNECT %d, "
                      "ignore DPP_TYPE_CREDIT cmd.", pg->inter_state);
    }
    else
    {
        //if(cmd->header.data_size_bytes == 0)
        //    pg->total_credit = 0;
        //else

        pg->total_credit += cmd->header.data_size_bytes;
        IOMIRROR_INFO("Iomirror total credit buffer is %llu bytes.", pg->total_credit);

        if ((0 == pg->total_credit) && (IM_PG_INTER_STATE_PAUSE != pg->inter_state))
        {
            /* 切换至pause状态，暂停发送数据 */
            pg->temp_state  = pg->inter_state;
            pg->inter_state = IM_PG_INTER_STATE_PAUSE;
            IOMIRROR_WARN("current internal convert to PAUSE state.");
        }
        else if ((0 != pg->total_credit) && (IM_PG_INTER_STATE_PAUSE == pg->inter_state))
        {
            pg->inter_state = pg->temp_state;
            pg->temp_state  = IM_PG_INTER_STATE_MAX;
            IOMIRROR_INFO("current internal state is CONNECT %d, temp_state %d.", pg->inter_state, pg->temp_state);
        }
    }
}


/**
 * Description : DPP_TYPE_HEARTBEAT_ACK报文处理函数
 *
 * Parameters  : pg - 所属保护组
                 cmd - 待处理报文
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/29
 */
static void im_pg_handle_heartbeat_ack_cmd(
                                        struct im_pg *pg, struct im_cmd *cmd)
{
    pg->hb_recv_time = jiffies;
}


/**
 * Description : 接收、分发报文，报文处理函数入口
 *
 * Parameters  : pg - 所属保护组
 * Return      : 成功 - 0
 *               失败 - -1，接收报文失败
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static inline int im_pg_recv_and_handle_cmd(struct im_pg *pg)
{
    int i = 0;
    int ret = 0;
    struct im_cmd cmd;
    PALARM_ITEM p_alarm_item = NULL;

    cmd.bvl = NULL;

    /* pg启动后，第一次与vrg建立连接前sock为NULL */
    if (unlikely(NULL == pg->sock || IM_PG_INTER_STATE_CONNECT_STAGE0 == pg->inter_state))
    {
        IOMIRROR_INFO_PRINT_LIMITED("im_pg_recv_and_handle_cmd break, state %d, wait to send login cmd or "
            "wait to receive a ACK.", pg->inter_state);
        ret = -1;
        goto out;
    }

    /**
     * 接收一个报文:
     * 接收超时直接退出函数；
     * 接收异常设置重连状态后退出；
     * 收到Magic错误报文，设置重联状态后退出。
     */
    ret = im_cmd_recv(pg->sock, &cmd);
    if (unlikely(ret < 0))
    {       
        if (ret == -2) { // -2表示接收到失败的ack
            pg->failed_ack_count += 1;
            if (im_pg_if_alarm_report_time_arrive(pg, LIMIT_ALARM_RECV_FAILED_ACK_FORM_OMA)) {
                p_alarm_item = im_create_and_init_alarm_item();
                if (likely(p_alarm_item != NULL)) {
                    snprintf(p_alarm_item->info, KERNEL_ALARM_INFO_DESC_LEN,
                        "Driver has received continuous %llu failed ack, driver state : %d.",
                        pg->failed_ack_count, pg->state);
                    p_alarm_item->error_code = ERROR_CODE_BITMAP_RECV_FAILED_ACK;
                    im_add_alarm_list(&pg->alarm_list, p_alarm_item);  // 告警信息加入list
                    im_pg_set_alarm_happen_time(pg, LIMIT_ALARM_RECV_FAILED_ACK_FORM_OMA);
                }
            }
        }

        if (cmd.header.ack_result == DPP_ACK_TOKEN_ID_ERROR) {
            pg->token.isValid = 0; // token id in driver memory is old or error, should transfer new token id from Agent
        }

        if (-1 == ret || ret == -2) {
            IOMIRROR_ERR("im_cmd_recv failed.");
            pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
            im_pg_flush_rq_and_pq(pg);
            im_pg_clear_cmd_pending_queue(pg);
        }
        goto out;
    }
    if (unlikely(IM_MAGIC != cmd.header.magic))
    {
        IOMIRROR_ERR("unknown magic=%u, reconnect.", cmd.header.magic);
        IOMIRROR_ERR("cmd_type=%u, request_id=%llu, data_offset=%llu, "
                     "data_size=%llu, host_id=%llx, vol_id=%s, ack_result=%d.",
                     cmd.header.cmd_type, cmd.header.request_id,
                     cmd.header.data_offset, cmd.header.data_size_bytes,
                     *(long long *)cmd.header.host_id, cmd.header.vol_id,
                     cmd.header.ack_result);
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_flush_rq_and_pq(pg);
        im_pg_clear_cmd_pending_queue(pg);
        ret = -1;
        goto out;
    }
    /* 根据报文类型选择handle函数处理报文 */
    switch (cmd.header.cmd_type)
    {
        case DPP_TYPE_DATASET_DONE:
            im_pg_handle_dataset_done_cmd(pg, &cmd);
            break;
        case DPP_TYPE_RESYNCSET_DONE:
            im_pg_handle_resyncset_done_cmd(pg, &cmd);
            break;
        case DPP_TYPE_SESSION_LOGIN_ACK:
            im_pg_handle_session_login_ack_cmd(pg, &cmd);
            break;
        case DPP_TYPE_CREDIT:
            im_pg_handle_credit_cmd(pg, &cmd);
            break;
        case DPP_TYPE_HEARTBEAT_ACK:
            im_pg_handle_heartbeat_ack_cmd(pg, &cmd);
            break;
        default:
            IOMIRROR_WARN("unknown cmd_type=%d.", cmd.header.cmd_type);
            break;
    }

out:

    if (unlikely(NULL != cmd.bvl))
    {
        for (i = 0; i < cmd.vcnt; i++)
        {
            if (NULL != cmd.bvl[i].bv_page)
            {
                __free_page(cmd.bvl[i].bv_page);
            }
        }
        kfree(cmd.bvl);
        cmd.bvl = NULL;
    }

    return ret;
}


/**
 * Description : 初始化控制报文
 *
 * Parameters  : cmd - 已分配好空间的待创建报文指针
 *               type - 报文类型
 *               pg - 所属保护组
 *               vol_id - 卷id
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static inline void im_pg_setup_control_cmd(struct im_cmd *cmd,
                uint32_t type, struct im_pg *pg, const char *vol_id)
{
    cmd->vcnt                   = 0;
    cmd->has_received           = 0;
    cmd->bvl                    = NULL;
    cmd->buf                    = NULL;
    cmd->private                = NULL;
    cmd->header.version         = IM_VERSION;
    cmd->header.magic           = IM_MAGIC;
    cmd->header.cmd_type        = type;
    cmd->header.data_size_bytes = 0;
    cmd->header.data_offset     = 0;
    cmd->header.request_id      = pg->cmd_pq.request_id;
    cmd->header.ack_result      = 0;
    cmd->send_time              =  jiffies;
    pg->cmd_pq.request_id++;

    memcpy(cmd->header.host_id, pg->host_id, VM_ID_LEN);
}

/**
 * Description : 处理pause pending状态，发送DPP_TYPE_FLUSH后，进入暂停状态
 *
 * Parameters  : pg - 所属保护组
 *
 * Return      : void
 *
 * Author      :
 * Date        : 2019-07-05
 */
static void im_pg_handle_pause_pending(struct im_pg *pg)
{
    int ret = 0;
    struct im_cmd cmd;

    if (pg->pause_pending == 0)
    {
        return;
    }

    IOMIRROR_INFO("Process pause cmd under external state %d.", pg->state);

    im_pg_setup_control_cmd(&cmd, DPP_TYPE_FLUSH, pg, "");
    ret = im_cmd_send(pg->sock, &cmd);
    if (unlikely(ret < 0))
    {
        IOMIRROR_ERR("im_cmd_send flush cmd failed.");
    }

    pg->flow_control_pause_flag = 1;
    pg->pause_pending = 0;

    IOMIRROR_INFO("Enter pause state successfully.");
}

/**
 * Description : 计算保护卷剩余未传输数据量（字节），CBT位图和verify位图分别计算
 *
 * Parameters  : pg
 *              cbt_size  CBT位图未传输数据大小
 *              verify_size  verify位图未传输数据大小
 * Return      : 无
 *
 * Author      : z00455045
 * Date        : 2019/07/09
 */
static void im_pg_get_vols_bitmap_size(struct im_pg *pg, uint64_t * cbt_size, uint64_t * verify_size)
{
    struct im_volume *vol = NULL;
    struct list_head *ptr = NULL;

    *cbt_size = 0;
    *verify_size = 0;

    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        *cbt_size += GetBitmapCount(vol->bitmap);

        if (vol->bitmap_verify != NULL)
        {
            *verify_size += GetBitmapCount(vol->bitmap_verify);
        }
    }

    *cbt_size *= IM_SECTOR_SIZE;
    *verify_size *= IM_SECTOR_SIZE;
}

/**
 * Description : 发送DPP_ATTENTION的acitivity类型报文，仅在CBT和resync模式下发送
 *
 * Parameters  : pg - 所属保护组
 *
 * Return      : void
 *
 * Author      :
 * Date        : 2019/07/09
 */
static void im_pg_send_activity_cmd(struct im_pg *pg)
{
    struct im_cmd cmd;
    int ret = 0;

    if (pg->state != IM_PG_STATE_CBT && pg->state != IM_PG_STATE_VERIFY)
    {
        return;
    }

    //网络重连状态，不发送 DPP_ATTENTION_OPERATION_ACTIVITY
    if (IM_PG_INTER_STATE_CONNECT_STAGE0 == pg->inter_state
        || IM_PG_INTER_STATE_CONNECT_STAGE0_P == pg->inter_state
        || IM_PG_INTER_STATE_CONNECT_STAGE1 == pg->inter_state
        || IM_PG_INTER_STATE_CONNECT_STAGE1_P == pg->inter_state
        || IM_PG_INTER_STATE_WAIT_PENDING_VOLS == pg->inter_state)
    {
        return;
    }

    im_pg_setup_control_cmd(&cmd, DPP_TYPE_ATTENTION, pg, "");

    im_pg_get_vols_bitmap_size(pg, &(cmd.header.data_size_bytes), &(cmd.header.data_offset));
    cmd.header.ack_result = DPP_ATTENTION_OPERATION_ACTIVITY;
    IOMIRROR_INFO("DPP_TYPE_ATTENTION type %d, cbt_backlog %llu, resync_remaining %llu.", cmd.header.ack_result, cmd.header.data_size_bytes, cmd.header.data_offset);

    ret = im_cmd_send(pg->sock, &cmd);
    if (unlikely(ret < 0))
    {
        IOMIRROR_ERR("send attention cmd failed.");
    }
}

/**
 * Description : 发送一个控制报文
 *
 * Parameters  : pg - 所属保护组
 *               type - 所需要发送控制报文的类型
 *               state - 发送成功后所需切换至的内部状态，如不切换则填当前状态
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/21
 */
static inline void im_pg_send_control_cmd(struct im_pg *pg,
      uint32_t type, enum im_pg_inter_state state, uint64_t request_id)
{
    int ret = 0;
    struct im_cmd cmd;

    memset(&cmd, 0, sizeof(struct im_cmd));
    /* 创建控制报文，并插入cmd pending queue */
    im_pg_setup_control_cmd(&cmd, type, pg, "");
    if (request_id > 0)
    {
        cmd.header.request_id = request_id;
    }

    ret = im_cmd_send(pg->sock, &cmd);
    if (unlikely(ret < 0))
    {
        IOMIRROR_ERR("im_cmd_send failed.");
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    pg->inter_state = state;

    IOMIRROR_INFO("Send data set cmd %u, request id %llu, state %d.",
        type, cmd.header.request_id, pg->inter_state);
}


/**
 * Description : 判断当前request queue和pending queue总大小是否超过阈值，
 *               如超过则将所有请求刷至位表，切换至cbt状态
 *
 * Parameters  : pg - 待处理的保护组指针
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/27
 */
static inline void im_pg_check_and_flush(struct im_pg *pg)
{
    uint64_t count = pg->rq.page_cnt + pg->pq.page_cnt;

    if (likely(count < IM_PG_FLUSH_SIZE))
    {
        return;
    }

    /* 总请求数达到阈值，刷至cbt位表 */
    IOMIRROR_INFO("Total %llu pages, request queue is full, will flush...", count);
    im_pg_flush_rq(pg, count);

    /* 若当前状态为cbt或verify，则无需切换至cbt状态 */
    if (IM_PG_STATE_CBT != pg->state
        && IM_PG_STATE_VERIFY != pg->state)
    {
        IOMIRROR_INFO("iomirror change state form %d to %d.", pg->state, IM_PG_STATE_CBT);
        pg->state = IM_PG_STATE_CBT;

        /* 建立连接过程中不切换内部状态 */
        if (IM_PG_INTER_STATE_CONNECT_STAGE0 != pg->inter_state
            && IM_PG_INTER_STATE_CONNECT_STAGE0_P != pg->inter_state
            && IM_PG_INTER_STATE_CONNECT_STAGE1 != pg->inter_state
            && IM_PG_INTER_STATE_CONNECT_STAGE1_P != pg->inter_state
            && IM_PG_INTER_STATE_PAUSE != pg->inter_state
            && IM_PG_INTER_STATE_WAIT_PENDING_VOLS != pg->inter_state)
        {
            pg->inter_state = IM_PG_INTER_STATE_CBT_START;
            im_pg_clear_cmd_pending_queue(pg);
        }

        /* 若为PAUSE状态，则设置临时状态 */
        if (IM_PG_INTER_STATE_PAUSE == pg->inter_state)
        {
            pg->temp_state = IM_PG_INTER_STATE_CBT_START;
            im_pg_clear_cmd_pending_queue(pg);
        }
    }
}


/**
 * Description : 处理动态添加卷外部命令
 *
 * Parameters  : pg - 保护组指针
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/7/4
 */
static inline void im_pg_handle_ext_cmd_add_volume(struct im_pg *pg)
{
    ProtectVol *info  = NULL;
    struct im_volume *vol       = NULL;

    info = (ProtectVol *)(pg->ext_cmd->info);

    IOMIRROR_INFO("handle IM_PG_EXT_CMD_ADD_VOL cmd, id=%llx-%llx, path=%s.",
                 *(unsigned long long *)(info->vol_id),*(unsigned long long *)(info->vol_id+8), info->disk_path);

    /* 向filter中增加卷 */
    vol = im_add_volume(info->vol_id, info->disk_path,
                        0, pg->bitmap_granularity, &(pg->rq));
    if (NULL == vol)
    {
        IOMIRROR_ERR("im_add_volume failed, vol_path=%s.", info->disk_path);
        pg->ext_cmd->ret = -1;
        goto out;
    }

    /* 加入本pg卷队列 */
    list_add_tail(&(vol->list1), &(pg->vols));
    pg->vol_num++;

    if ( (pg->is_init) && (IM_PG_STATE_VERIFY != pg->state) )
    {
        BitmapSetBit(vol->bitmap, 0, vol->sectors);
    }

    BitmapItInit(vol->hbi_send, vol->bitmap, 0);
    //pg->cbt_flush_times++;

    if (IM_PG_STATE_VERIFY == pg->state && vol->bitmap_verify == NULL) {

        vol->bitmap_verify = BitmapAlloc(vol->sectors, pg->bitmap_granularity, BitmapAllocFunc, BitmapFreeFunc);
        if (NULL == vol->bitmap_verify)
        {
            IOMIRROR_WARN("BitmapAlloc for vol->bitmap_verify failed.");
            pg->ext_cmd->ret = -1;
            goto out;
        }

        BitmapSetBit(vol->bitmap_verify, 0, vol->sectors);
        BitmapItInit(vol->hbi_send, vol->bitmap_verify, 0);
        IOMIRROR_INFO("vol %llx bitmap iterator pos is %llu, bitmap count is %llu.", *(unsigned long long *)(vol->id), vol->hbi_send->pos, vol->bitmap_verify->count);
    }

    /* 若当前状态为cbt或verify，则无需切换至cbt状态 */
    if (IM_PG_STATE_CBT != pg->state
        && IM_PG_STATE_VERIFY != pg->state)
    {
        pg->state = IM_PG_STATE_CBT;
        if (IM_PG_INTER_STATE_CONNECT_STAGE0 != pg->inter_state
            && IM_PG_INTER_STATE_CONNECT_STAGE0_P != pg->inter_state
            && IM_PG_INTER_STATE_CONNECT_STAGE1 != pg->inter_state
            && IM_PG_INTER_STATE_CONNECT_STAGE1_P != pg->inter_state
            && IM_PG_INTER_STATE_PAUSE != pg->inter_state
            && IM_PG_INTER_STATE_WAIT_PENDING_VOLS != pg->inter_state)
        {
            pg->inter_state = IM_PG_INTER_STATE_CBT_START;
            im_pg_clear_cmd_pending_queue(pg);
        }
    }
    
    im_pg_get_vols_bitmap_size(pg, &pg->cbt_bitmap_total_bytes, &pg->verify_bitmap_total_bytes);
    IOMIRROR_INFO("add new volume: %s, bitmaps in cbt mode contains total %llu bytes.", 
        info->disk_path, pg->cbt_bitmap_total_bytes);  

    pg->ext_cmd->ret = 0;

out:
    complete_all(&(pg->ext_cmd->comp));
}

/**
 * Description : 处理动态删除卷外部命令
 *
 * Parameters  : pg - 保护组指针
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/7/4
 */
static inline void im_pg_handle_ext_cmd_del_volume(struct im_pg *pg)
{
    ProtectVol *info  = NULL;
    struct im_volume *vol       = NULL;
    struct list_head *ptr       = NULL;

    info = (ProtectVol *)(pg->ext_cmd->info);

    IOMIRROR_INFO("handle IM_PG_EXT_CMD_DEL_VOL cmd, id=%s, path=%s.",
                  info->vol_id, info->disk_path);

    /* 在保护组的卷队列中查找待删除卷 */
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);

        if (0 == memcmp(vol->id, info->vol_id, VOL_ID_LEN))
        {
            break;
        }
        else
        {
            vol = NULL;
        }
    }

    /* 从filter中删除卷 */
    if (likely(NULL != vol))
    {
        list_del_init(&(vol->list1));
        im_del_volume(vol);
        vol = NULL;
        pg->vol_num--;
        pg->ext_cmd->ret = 0;
    }
    else
    {
        IOMIRROR_ERR("can NOT find the corresponding volume, volume_id=%s.",
                      info->vol_id);
        pg->ext_cmd->ret = -1;
        goto out;
    }

out:
    complete_all(&(pg->ext_cmd->comp));
}

/**
 * Description : 处理动态修改卷外部命令
 *
 * Parameters  : pg - 保护组指针
 * Return      : void
 *
 * Author      : wangguitao
 * Date        : 2019/8/13
 */
static inline void im_pg_handle_ext_cmd_mod_volume(struct im_pg *pg)
{
    ProtectVol *info  = NULL;
    struct im_volume *vol       = NULL;
    struct list_head *ptr       = NULL;

    info = (ProtectVol *)(pg->ext_cmd->info);

    IOMIRROR_INFO("handle IM_PG_EXT_CMD_MOD_VOL cmd, old_vol_id=%llx, vol_id=%llx, path=%s.",
                  *(long long *)info->old_vol_id, *(long long *)info->vol_id, info->disk_path);

    /* 在保护组的卷队列中查找待修改卷 */
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        
        if (0 == memcmp(vol->id, info->old_vol_id, VOL_ID_LEN))
        {
            IOMIRROR_INFO("get vol_id=%llx, old_vol_id=%llx.", *(long long *)vol->id, *(long long *)info->old_vol_id);
            break;
        }
        else
        {
            vol = NULL;
        }
    }

    /* 从filter中修改卷 */
    if (likely(NULL != vol))
    {
        memcpy(vol->id, info->vol_id, VOL_ID_LEN);
        memcpy(vol->path, info->disk_path, VOL_ID_LEN);
        pg->ext_cmd->ret = 0;
    }
    else
    {
        IOMIRROR_ERR("can not find the corresponding volume, volume_id=%llx.",
                      *(long long *)info->old_vol_id);
        pg->ext_cmd->ret = -1;
        goto out;
    }

out:
    complete_all(&(pg->ext_cmd->comp));
}

/**
 * Description : 处理被保护卷准备就绪外部命令
 *
 * Parameters  : pg - 保护组指针
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/7/25
 */
static inline void im_pg_handle_ext_cmd_volume_ready(struct im_pg *pg)
{
    struct list_head         *ptr           = NULL;
    ProtectVol     *info          = NULL;
    struct im_pg_pending_vol *pending_vol   = NULL;
    struct im_volume         *vol           = NULL;

    info = (ProtectVol *)(pg->ext_cmd->info);

    IOMIRROR_INFO("handle IM_PG_EXT_CMD_VOL_READY cmd, id=%s, path=%s.",
                  info->vol_id, info->disk_path);

    /* 判断传入卷是否为pending volume */
    list_for_each(ptr, &(pg->pending_vols))
    {
        pending_vol = list_entry(ptr, struct im_pg_pending_vol, list);

        if (0 == strncmp(pending_vol->path, info->disk_path, DISK_PATH_LEN))
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
        IOMIRROR_ERR("can not find pending volume, vol_path=%s.", info->disk_path);
        pg->ext_cmd->ret = -1;
        goto out;
    }

    /* 向filter中增加卷 */
    vol = im_add_volume(pending_vol->id, pending_vol->path,
                    pending_vol->sectors, pg->bitmap_granularity, &(pg->rq));
    if (NULL == vol)
    {
        IOMIRROR_ERR("im_add_volume failed, vol_path=%s.", pending_vol->path);
        pg->ext_cmd->ret = -1;
        goto out;
    }

    /* 加入本pg卷队列，从pending volumes队列中删除 */
    list_add_tail(&(vol->list1), &(pg->vols));
    pg->vol_num++;
    list_del_init(&(pending_vol->list));

    if ( (pg->is_init) && (IM_PG_STATE_VERIFY != pg->state) )
    {
        BitmapSetBit(vol->bitmap, 0, vol->sectors);
    }

    /* pending volume队列为空，所有待保护卷都已加入filter，开始连接复制数据 */
    if (list_empty(&(pg->pending_vols)))
    {
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
    }

    pg->ext_cmd->ret = 0;

out:
    complete_all(&(pg->ext_cmd->comp));
}

static inline void im_pg_handle_ext_cmd_stop_send_data(struct im_pg *pg)
{
    IOMIRROR_INFO("handle IM_PG_EXT_CMD_STOP_SEND_DATA.");
    pg->stop_send_data = 1;
    pg->ext_cmd->ret = 0;
    complete_all(&(pg->ext_cmd->comp));
}

static inline void im_pg_handle_ext_cmd_set_token_id(struct im_pg *pg)
{
    SetTokenID* pInfo;
    IOMIRROR_INFO("handle IM_PG_EXT_CMD_SET_TOKEN_ID.");

    pInfo = (SetTokenID*)(pg->ext_cmd->info);
    pg->token.isValid = pInfo->isValid;
    memcpy(pg->token.token_id, pInfo->token_id, TOKEN_ID_LEN);
    // avoid Agent requests token again when driver has received this token succcess
    if (pg->token.isValid) {
        pg->link_state = (pg->link_state == LINK_STATE_BREAK ? LINK_STATE_BREAK_WITH_VALID_TOKEN_ID : pg->link_state);
        pg->ext_cmd->ret = 0;
    } else {
        IOMIRROR_ERR("receive a tokenid but isValid flag is invalid.");
        pg->ext_cmd->ret = -2;
    }

    complete_all(&(pg->ext_cmd->comp));
}

/**
 * Description : 处理暂停外部命令
 *
 * Parameters  : pg - 保护组指针
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/9/22
 */
static inline void im_pg_handle_ext_cmd_pause(struct im_pg *pg)
{
    WaitFlushQueue* info;
    IOMIRROR_INFO("handle IM_PG_EXT_CMD_PAUSE ext cmd, wait to send dataset.");

    pg->pause_pending = 1; //wait to send a dataset start cmd
    pg->ext_cmd->ret = 0;

    info = (WaitFlushQueue *)(pg->ext_cmd->info);
    pg->queuePageCnt = (info->waitFlushQueueFlag ?  pg->rq.page_cnt : 0);; // 获取此时队列里的bio所占page个数

    complete_all(&(pg->ext_cmd->comp));
}

/**
 * Description : 从内核态告警list中获取alarm info, 如果内核态暂时没有告警产生，则返回成功
 *               当内核态没有产生足够的告警，实际返回用户态的告警个数可能会小于用户态的期望得到的个数，
 *               但是返回的最大告警个数不大于宏 MAX_ALARM_NUM_ONCE_REPORT
 * Parameters  : pg - 保护组指针
 * Return      : void
 *
 * Author      : z00455045
 * Date        : 2019/12/20
 */
static inline void im_pg_handle_ext_cmd_get_alarm(struct im_pg *pg)
{
    GetAlarm* info = NULL;

    IOMIRROR_INFO("handle im_pg_handle_ext_cmd_get_alarm ext cmd.");
    info = (GetAlarm *)(pg->ext_cmd->info);
    if (info->pData == NULL) {
        IOMIRROR_ERR("im_pg_handle_ext_cmd_get_alarm parameters error.");
        pg->ext_cmd->ret = -1;
        goto out;
    }

    if (pg->alarm_list.alarm_total == 0) {
        IOMIRROR_INFO("There are no kernel alarm info to report.");
        pg->ext_cmd->ret = 0;
        info->return_alarm_num = 0; // 用户态判断需要
        goto out;
    }

    info->return_alarm_num = im_get_alarms(&pg->alarm_list, info->pData, MAX_ALARM_NUM_ONCE_REPORT);
    if (info->return_alarm_num <= 0) {
        IOMIRROR_ERR("im_pg_handle_ext_cmd_get_alarm get kernel alarm failed %d.", info->return_alarm_num);
        pg->ext_cmd->ret = -2;
        goto out;
    }

    pg->ext_cmd->ret = 0;
out:
    complete_all(&(pg->ext_cmd->comp));
}
/**
 * Description : 修改driver配置命令
 *
 * Parameters  : pg - 保护组指针
 * Return      : void
 *
 * Author      : z00455045
 * Date        : 2019/7/24
 */
static inline void im_pg_handle_ext_cmd_modify(struct im_pg *pg)
{
    ProtectStrategy* info = NULL;

    IOMIRROR_INFO("handle IM_PG_EXT_CMD_MODIFY ext cmd.");
    info = (ProtectStrategy *)(pg->ext_cmd->info);
    pg->exp_rpo = info->exp_rpo * HZ;

    if (IOMIRROR_MEMCMP(pg->id, info->oma_id, VM_ID_LEN) !=0 ||
        IOMIRROR_MEMCMP(pg->host_id, info->vm_id, VM_ID_LEN) !=0 ||
        pg->vrg_ip != ntohl(info->oma_ip[0]) || pg->vrg_port != info->oma_port)
    {
        pg->vrg_ip = ntohl(info->oma_ip[0]);
        pg->vrg_port = info->oma_port;
        memcpy(pg->id, info->oma_id, VM_ID_LEN);
        memcpy(pg->host_id, info->vm_id, VM_ID_LEN);


        /* 准备进入重连状态 */
        if (pg->inter_state != IM_PG_INTER_STATE_WAIT_PENDING_VOLS && pg->inter_state != IM_PG_INTER_STATE_CONNECT_STAGE0)
        {
            IOMIRROR_INFO("pg->inter_state is %d.", pg->inter_state);
            im_pg_flush_rq_and_pq(pg);
            pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
            pg->temp_state  = IM_PG_INTER_STATE_MAX;
        }

        IOMIRROR_INFO("Iomirror will reconnect to OMA...");
        im_pg_clear_cmd_pending_queue(pg);//clear sent cmd before reconnect, otherwise cmds will re-send in IM_PG_CMD_RESEND_INTERVAL seconds
    }

    pg->ext_cmd->ret = 0;
    complete_all(&(pg->ext_cmd->comp));
}
/**
 * Description : 处理恢复保护外部命令
 *
 * Parameters  : pg - 保护组指针
 * Return      : void
 *
 * Author      : z00455045
 * Date        : 2019/07/05
 */
static inline void im_pg_handle_ext_cmd_resume(struct im_pg *pg)
{
    IOMIRROR_INFO("handle IM_PG_EXT_CMD_RESUME ext cmd.");

    if (pg->resume_pending == 1 || pg->reboot_pause_state == 1)
    {
        pg->resume_pending = 0;
        pg->flow_control_pause_flag = 0;
        pg->pause_pending = 0;
        pg->reboot_pause_state = 0;
        pg->ext_cmd->ret = 0;
        IOMIRROR_INFO("Resume iomirror successful.");
    }
    else
    {
        pg->ext_cmd->ret = -1;
        IOMIRROR_WARN("the resume state is wrong, ignore IM_PG_EXT_CMD_RESUME ext cmd.");
    }

    /*if (IM_PG_INTER_STATE_PAUSE != pg->inter_state)
    {
        IOMIRROR_WARN("current internal state=%d, "
                      "ignore IM_PG_EXT_CMD_RESUME ext cmd.", pg->inter_state);
        pg->ext_cmd->ret = -1;
    }
    else
    {
        pg->inter_state  = pg->temp_state;
        pg->temp_state   = IM_PG_INTER_STATE_MAX;
        pg->ext_cmd->ret = 0;*/

        /***
         * 此处刷新verify_recv_time，
         * 如果pause前是一致性校验状态，
         * 可确保resume后继续校验而非重新发起
         */
       /* pg->verify_recv_time = jiffies;
    }*/

    complete_all(&(pg->ext_cmd->comp));
}


static void im_pg_send_dataset(struct im_pg *pg)
{
    struct im_cmd *cmd_send  = NULL;
    struct im_pg_pending_queue *pq = &(pg->pq);
    int dataset_type = (pg->inter_state != IM_PG_INTER_STATE_CBT_DATA ? DPP_DATASET_TYPE_NORMAL : DPP_DATASET_TYPE_CBT);

    IOMIRROR_INFO("send dataset cmd, req id %llu, bytes %llu, type %d.", pg->cbt_flush_times + 1, pg->sent_amount, dataset_type);
    /* 构造DATA SET报文，插入pending queue */
    cmd_send = (struct im_cmd *)kzalloc(sizeof(struct im_cmd), GFP_KERNEL);
    if (unlikely(NULL == cmd_send))
    {
        IOMIRROR_ERR("kzalloc for cmd_send failed.");
        return;
    }
    im_pg_setup_control_cmd(cmd_send, DPP_TYPE_DATASET_START, pg, "");
    pg->cbt_flush_times++;
    cmd_send->header.request_id = pg->cbt_flush_times;
    cmd_send->send_time = jiffies;

    list_add_tail(&(cmd_send->list), &(pq->head));
    pq->count++;
    //pq->page_cnt += cmd_send->vcnt;

    if (pg->inter_state == IM_PG_INTER_STATE_CBT_DATA)
    {
        im_pg_send_control_cmd(pg, (DPP_TYPE_DATASET_START | DPP_CBT_DATASET_FLAG),
                           IM_PG_INTER_STATE_CBT_DATA, pg->cbt_flush_times);
    }
    else
    {
        im_pg_send_control_cmd(pg, DPP_TYPE_DATASET_START,
                           IM_PG_INTER_STATE_NORMAL, pg->cbt_flush_times);
    }
}


static void im_pg_send_resyncset(struct im_pg *pg, char *vol_id, uint64_t vol_size)
{
    int ret = 0;
    struct im_cmd *cmd_send  = NULL;
    struct im_pg_pending_queue *pq = &(pg->pq);

    IOMIRROR_INFO("send resync set cmd, req=%llu, bytes %llu.", pg->cbt_flush_times + 1, pg->sent_amount);
    /* 构造RESYNC SET报文，插入pending queue */
    cmd_send = (struct im_cmd *)kzalloc(sizeof(struct im_cmd), GFP_KERNEL);
    if (unlikely(NULL == cmd_send))
    {
        IOMIRROR_ERR("kzalloc for cmd_send failed.");
        return;
    }
    im_pg_setup_control_cmd(cmd_send, DPP_TYPE_RESYNCSET_START, pg, "");
    pg->cbt_flush_times++;

    memcpy(cmd_send->header.vol_id, vol_id, VOL_ID_LEN);
    cmd_send->header.request_id = pg->cbt_flush_times;
    // 根据dataset的最大值获取当前可以发送的最大值
    cmd_send->header.data_size_bytes = (pg->next_vol_offset + pg->max_dataset_size / 2 > vol_size)?
        vol_size - pg->next_vol_offset : pg->max_dataset_size / 2;
    // 使用pg中vol记录的偏移量
    cmd_send->header.data_offset = pg->next_vol_offset;
    // 更新pg中的下一个偏移量，以供下次发送使用
    pg->next_vol_offset += cmd_send->header.data_size_bytes;
    if (pg->next_vol_offset >= vol_size) {
        // judge whether the sending dataset_start is the last one in this volume
        IOMIRROR_INFO("The volume which id is %llx-%llx last dataset size is %llu.",
            *(unsigned long long *)vol_id, *(unsigned long long *)(vol_id+8), cmd_send->header.data_size_bytes);
        pg->next_vol_offset = 0;
    }

    ret = im_cmd_send(pg->sock, cmd_send);
    if (unlikely(ret < 0))
    {
        IOMIRROR_ERR("im_cmd_send failed.");
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        kfree(cmd_send);
        return;
    }

    cmd_send->send_time = jiffies;
    list_add_tail(&(cmd_send->list), &(pq->head));
    pq->count++;
    //pq->page_cnt += cmd_send->vcnt;

    pg->inter_state = IM_PG_INTER_STATE_VERIFY_DATA;
}

/**
 * Description : 外部DPP_ATTENTION(activity)、RPO发送dataset、控制报文重发检查
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : z00455045
 * Date        : 2019/7/18
 */
static inline void im_pg_check_packet_send(struct im_pg *pg)
{
    int ret = 0;
    struct im_cmd *p_cmd = NULL;
    struct list_head *ptr = NULL;
    unsigned long cur_time = jiffies;
    struct im_pg_cmd_pending_queue *cmd_pq = &(pg->cmd_pq);

    if (unlikely(IM_PG_INTER_STATE_CONNECT_STAGE0 == pg->inter_state
                 || IM_PG_INTER_STATE_WAIT_PENDING_VOLS == pg->inter_state))
    {
        return;
    }

    /* 在normal状态下，RPO time 的1/3时插入dataset*/
    if ((IM_PG_INTER_STATE_NORMAL == pg->inter_state) && ((cur_time - pg->rpo_send_time) > (pg->exp_rpo / 3)))
    {
        im_pg_send_dataset(pg);
        pg->rpo_send_time = cur_time;
        pg->sent_amount = 0;
    }

    /* 到达控制报文重发时间，重发控制报文 */
    if ((!list_empty(&(cmd_pq->head)))
        && ((cur_time - cmd_pq->send_time) > IM_PG_CMD_RESEND_INTERVAL))
    {
        IOMIRROR_INFO("control cmd resend time is up, state %d, resend cmd now...", pg->inter_state);
        list_for_each(ptr, &(cmd_pq->head))
        {
            p_cmd = list_entry(ptr, struct im_cmd, list);
            IOMIRROR_INFO("resend control cmd, cmd_type=%d, request_id=%llu, "
                          "data_offset=%llu, data_size=%llu, host_id=%llx, "
                          "vol_id=%s, ack_result=%d.",
                          p_cmd->header.cmd_type, p_cmd->header.request_id,
                          p_cmd->header.data_offset, p_cmd->header.data_size_bytes,
                          *(long long *)p_cmd->header.host_id,
                          p_cmd->header.vol_id, p_cmd->header.ack_result);
            ret = im_cmd_send(pg->sock, p_cmd);
            if (unlikely(ret < 0))
            {
                IOMIRROR_ERR("im_cmd_send resend control cmd failed.");
                pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
                im_pg_clear_cmd_pending_queue(pg);
                break;
            }
        }
        cmd_pq->send_time = cur_time;
    }
}

static inline void im_pg_count_iops_and_throughout(struct im_pg *pg)
{
    unsigned long sub_jiffies;
    unsigned int  msec;

    // count jiffies sub
    SAFE_TWO_JIFFIES_SUB(jiffies, pg->statistics_jiffies.iops_time, sub_jiffies);
    msec = jiffies_to_msecs(sub_jiffies);
    if (msec == 0) {
        IOMIRROR_WARN("the time interval of count read and write iops is 0 ticks.");
        goto func_out;
    }

    pg->write_iops =  (pg->real_time_write_io * MSEC_PER_SECONDS)  / msec;
    // unit : KB/s
    pg->write_throughout = (pg->write_io_size * MSEC_PER_SECONDS)  / msec / KB_UNIT; 

func_out:
    pg->real_time_write_io = 0;
    pg->statistics_jiffies.iops_time = jiffies; 
    pg->write_io_size = 0; 
}

static inline void im_pg_count_data_speed(struct im_pg *pg)
{
    unsigned long sub_jiffies; 
    unsigned int  msec;

    // count jiffies sub
    SAFE_TWO_JIFFIES_SUB(jiffies, pg->statistics_jiffies.data_trans_time, sub_jiffies);
    msec = jiffies_to_msecs(sub_jiffies);
    if (msec == 0) {
        IOMIRROR_WARN("the time interval of count read and write iops is 0 ticks.");
        goto data_speed_out;
    }
    // unit : KB/s 
    pg->data_send_speed = (pg->data_send_size * MSEC_PER_SECONDS) / msec / KB_UNIT;

data_speed_out:
    pg->statistics_jiffies.data_trans_time = jiffies;
    pg->data_send_size = 0;
}

static void im_pg_get_statistics_info(struct im_pg *pg)
{
    if (jiffies - pg->statistics_jiffies.iops_time > IM_PG_STATISTICS_TIME_LEN) {
        im_pg_count_iops_and_throughout(pg); 
    }

    if (jiffies - pg->statistics_jiffies.data_trans_time > IM_PG_STATISTICS_TIME_LEN) {
        im_pg_count_data_speed(pg);
    }
}

/**
 * Description : 外部命令处理、心跳检查
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/29
 */
static inline void im_pg_check_heartbeat_and_ext_cmd(struct im_pg *pg)
{
    int ret = 0;
    struct im_cmd cmd;
    unsigned long cur_time = jiffies;

    /* 判断是否有外部命令需要处理 */
    if (unlikely(NULL != pg->ext_cmd))
    {
        IOMIRROR_INFO("got external cmd, type=%d, handle it.", pg->ext_cmd->type);
        switch (pg->ext_cmd->type)
        {
            case IM_PG_EXT_CMD_ADD_VOL:
                im_pg_handle_ext_cmd_add_volume(pg);
                break;
            case IM_PG_EXT_CMD_DEL_VOL:
                im_pg_handle_ext_cmd_del_volume(pg);
                break;
            case IM_PG_EXT_CMD_MOD_VOL:
                im_pg_handle_ext_cmd_mod_volume(pg);
                break;
            case IM_PG_EXT_CMD_VOL_READY:
                im_pg_handle_ext_cmd_volume_ready(pg);
                break;
            case IM_PG_EXT_CMD_PAUSE:
                im_pg_handle_ext_cmd_pause(pg);
                break;
            case IM_PG_EXT_CMD_RESUME:
                im_pg_handle_ext_cmd_resume(pg);
                break;
            case IM_PG_EXT_CMD_MODIFY:
                im_pg_handle_ext_cmd_modify(pg);
                break;
            case IM_PG_EXT_CMD_STOP_SEND_DATA:
                im_pg_handle_ext_cmd_stop_send_data(pg);
                break;
            case IM_PG_EXT_CMD_GET_KERNEL_ALARM:
                im_pg_handle_ext_cmd_get_alarm(pg);
                break;
            case IM_PG_EXT_CMD_SET_TOKEN_ID:
                im_pg_handle_ext_cmd_set_token_id(pg);
                break;
            default:
                IOMIRROR_ERR("unknown external cmd type %d.", pg->ext_cmd->type);
                pg->ext_cmd->ret = -1;
                complete_all(&(pg->ext_cmd->comp));
                break;
        }
        pg->ext_cmd = NULL;
    }

    im_pg_get_first_no_ack_cmd_time(pg);
    // should invoke here because iops has be count in im_pg_get_statistics_info()
    im_pg_get_statistics_info(pg);

    if (unlikely(IM_PG_INTER_STATE_CONNECT_STAGE0 == pg->inter_state
                 || IM_PG_INTER_STATE_WAIT_PENDING_VOLS == pg->inter_state))
    {
        return;
    }

    /* 超时时间内未收到心跳ack报文，启动重连 */
    if (unlikely((cur_time - pg->hb_recv_time) > IM_PG_HEARTBEAT_TIMEOUT))
    {
        IOMIRROR_ERR("heartbeat timeout, reconnect.");
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    /* 到达心跳报文发送时间间隔，发送心跳报文 */
    if ((cur_time - pg->hb_send_time) > IM_PG_HEARTBEAT_INTERVAL)
    {
        im_pg_setup_control_cmd(&cmd, DPP_TYPE_HEARTBEAT, pg, "");
        ret = im_cmd_send(pg->sock, &cmd);
        if (unlikely(ret < 0))
        {
            IOMIRROR_ERR("im_cmd_send heartbeat cmd failed.");
        }

        /* 在CBT和resync状态下发送DPP_ATTENTION报文 */
        // 计算位图中剩余数据量，发送给OMA
        im_pg_send_activity_cmd(pg);

        pg->hb_send_time = cur_time;
    }
}


/**
 * Description : 与vrg建立连接第一阶段，
 *               创建socket、连接vrg、并发送DPP_TYPE_SESSION_LOGIN报文
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static void im_pg_connect_stage0(struct im_pg *pg, int *conn_delay)
{
    int ret = 0;
    struct im_cmd *cmd = NULL;
    struct sockaddr_in vrg_addr;
	char ip_buf[IP_STRING_LEN] = {0};

    if (*conn_delay > 0)
    {
        msleep(IM_PG_IDLE_DELAY);
        *conn_delay = *conn_delay - 1;
        return;
    }

    if (NULL != pg->sock)
    {
        im_socket_close(pg->sock);
        pg->sock = NULL;
    }

    // token_id is invalid, stop creating socket
    if (!pg->token.isValid) {
        IOMIRROR_ERR("That is not necessary to create socket with peer VM because token id is not valid.");
        goto fail;
    }

    vrg_addr.sin_family      = AF_INET;
    vrg_addr.sin_port        = htons(pg->vrg_port);
    vrg_addr.sin_addr.s_addr = htonl(pg->vrg_ip);
    ret = im_socket_create(AF_INET, SOCK_STREAM, 0, &(pg->sock));
    if (ret < 0)
    {
        IOMIRROR_ERR("im_socket_create failed.");
        goto fail;
    }

    ret = im_socket_connect(pg->sock,
                      (struct sockaddr*)&vrg_addr, sizeof(struct sockaddr), 0);
    if (ret < 0)
    {
        IOMIRROR_WARN("im_socket_connect failed.");
        goto fail;
    }

    /* 创建并发送CONNECT_VRG报文 */
    cmd = (struct im_cmd *)kzalloc(sizeof(struct im_cmd), GFP_KERNEL);
    if (unlikely(NULL == cmd))
    {
        IOMIRROR_ERR("kzalloc for cmd failed.");
        goto fail;
    }

    im_pg_setup_control_cmd(cmd, DPP_TYPE_SESSION_LOGIN, pg, "");
    // use cmd->header.request_id to deliver dataset_id_done in DPP_TYPE_SESSION_LOGIN
    cmd->header.request_id = pg->cbt_flush_times;
    cmd->header.data_offset = IM_DEFAULT_MAX_DATASET_SIZE;
    // use cmd->header.data_size_bytes to deliver dataset_id_done in DPP_TYPE_SESSION_LOGIN
    cmd->header.data_size_bytes = pg->cbt_flush_times_done;

    // use the cmd->header.oma_id to deliver tokenID
    memcpy(cmd->header.oma_id, pg->token.token_id, VM_ID_LEN);

    list_add_tail(&(cmd->list), &(pg->cmd_pq.head));
    pg->cmd_pq.count++;
    pg->cmd_pq.send_time = jiffies;

    ret = im_cmd_send(pg->sock, cmd);
    if (unlikely(ret < 0))
    {
    	IOMIRROR_ERR("im_cmd_send failed.");
    	goto fail;
    }

    parse_ip_str(pg->vrg_ip, ip_buf, IP_STRING_LEN);
    IOMIRROR_INFO("connect VRG ip(%s) port(%u), and send DPP_TYPE_SESSION_LOGIN cmd.", ip_buf, pg->vrg_port);

    pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0_P;
    pg->hb_recv_time = jiffies;

    *conn_delay = IM_PG_RECONNECT_INTERVAL;
    return;

fail:

    if (NULL != pg->sock)
    {
        im_socket_close(pg->sock);
        pg->sock = NULL;
    }
    im_pg_clear_cmd_pending_queue(pg);
    pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
    *conn_delay = IM_PG_RECONNECT_INTERVAL;
}


/**
 * Description : 与vrg建立连接第二阶段，发送IM_CMD_IOMIRROR_START报文
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static void im_pg_connect_stage1(struct im_pg *pg)
{
    /**
     * 断线重连无需发送IM_CMD_IOMIRROR_START报文，
     * 至此重连成功，根据当前状态刷新内部状态
     */
    switch (pg->state)
    {
        case IM_PG_STATE_NORMAL:
        case IM_PG_STATE_ATOMIC:
        case IM_PG_STATE_CBT:
            pg->state = IM_PG_STATE_CBT;
            pg->inter_state = IM_PG_INTER_STATE_CBT_START;
            break;

        case IM_PG_STATE_VERIFY:
            pg->inter_state = IM_PG_INTER_STATE_VERIFY_START;
            break;

        default:
            IOMIRROR_ERR("unknown pg state.");
            break;
    }
    pg->connect_times++;
    pg->link_state = LINK_STATE_NORMAL;
    pg->token.isValid = 0; // driver has connected to peer, thus reset the old tokenid to invalid state
    memset(pg->token.token_id, 0, TOKEN_ID_LEN);
    IOMIRROR_INFO("connected!!! switch inter_state to %d.", pg->inter_state);
    return;
}
// 更新Normal状态下pause条件pg->queuePageCnt的值，每次从request queue取出vcnt个pages，要调用本函数
static inline void im_pg_fresh_queue_page_count(struct im_pg *pg, unsigned short vcnt)
{
    if (likely(pg->queuePageCnt == 0)) {
        return;
    }

    pg->queuePageCnt = (pg->queuePageCnt - vcnt > 0 ? pg->queuePageCnt - vcnt : 0);
}

static inline int im_pg_delay_send_dataset(struct im_pg *pg)
{
    uint64_t limit_failed_ack_count = 0;
    const int delay_time_uint = 200; // 200ms
    const int ms_per_second = 1000;
    const int need_delay = 1; // return the value indicates that driver should not send dataset to soma now
    const int max_failed_ack_count = 7; // 2^(7-1) = 64s dataset之间最大可以延迟64秒
    
    // 需要延迟发送的条件
    if (pg->delay_time_count == 0 && pg->failed_ack_count != 0 && pg->last_failed_ack_count != pg->failed_ack_count) {
        limit_failed_ack_count = (pg->failed_ack_count > max_failed_ack_count ? max_failed_ack_count
            : pg->failed_ack_count);
        pg->delay_time_count = make_power_2_value(limit_failed_ack_count - 1); // 此处单位是秒
        IOMIRROR_INFO("Receive failed data ack, driver will delay %llu seconds to send data.", pg->delay_time_count);
        pg->last_failed_ack_count = pg->failed_ack_count;
        pg->end_delay_time = jiffies + pg->delay_time_count * HZ; // 记录延迟到期发送的时间
        // 每次延迟200毫秒，防止延迟时间过长，导致心跳等无法正常发送
        pg->delay_time_count *= (ms_per_second / delay_time_uint); // 200ms的粒度去延迟
    }
    // 如果开始延迟的时间，和当前时间的差已经大于需要延迟的时间，那么直接跳过下面的延迟，
    // 这种情况可能出现在 在等待延迟发送时，主机有大量写IO, driver会刷位图，这个动作会耗费较长时间
    if (unlikely(pg->delay_time_count != 0) && time_after(jiffies, pg->end_delay_time)) {
        goto cancel_delay;
    }

    if (pg->delay_time_count != 0) {
        msleep(delay_time_uint);
        pg->delay_time_count--;
        return need_delay;
    }
    
cancel_delay:
    pg->delay_time_count = 0;
    pg->end_delay_time = 0;
    return 0;
}
/**
 * Description : 增量队列处理函数，
 *               创建并发送DPP_TYPE_DATA数据报文
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static inline void im_pg_normal(struct im_pg *pg, unsigned long *send_time)
{
    int i = 0;
    int ret = 0;
    struct im_cmd *cmd = NULL;
    struct im_reqeust_queue *rq = &(pg->rq);
    struct im_pg_pending_queue *pq = &(pg->pq);
    uint32_t normal_send_size = 0;
    uint32_t body_len = 0;
    DPP_HEADER header;
    char data[256] = {0};
    uint32_t left_buffer_size = 0;
    struct bio_vec *bvec = NULL;
    uint8_t is_buffer_enough = 0;
    uint32_t cmd_header_body_size = 0;
    bool buffer_compare_flag = false;

    *send_time = jiffies;

    pg->work_state = WORK_STATE_NORMAL;
    //if pg->pause_pending == 1, send a dataset start, and switch to pending state.
    if(pg->sent_amount >= (pg->max_dataset_size / 2) || pg->pause_pending == 1)
    {
        if (IM_PG_INTER_STATE_NORMAL == pg->inter_state || pg->inter_state == IM_PG_INTER_STATE_ATOMIC_DATA)
        {
            im_pg_send_dataset(pg);
        }

        pg->sent_amount = 0;

        if (pg->pause_pending == 1 && pg->queuePageCnt == 0)
        {
            im_pg_handle_pause_pending(pg);
            return; // normal state should stop sending data after sending a flush commands 
        }
    }

    normal_send_size = 0;
    cmd_header_body_size = sizeof(DPP_HEADER) + MAX_COMPARE(sizeof(DPP_DATASET_START), sizeof(DPP_DATA));
    while (normal_send_size < MAX_NORMAL_SEND_BUFFER)
    {
        // buffer_compare_flag = false indicates credit buffer is less
        buffer_compare_flag = (pg->total_credit > (MAX_NORMAL_SEND_BUFFER - normal_send_size));
        left_buffer_size = (buffer_compare_flag ? (MAX_NORMAL_SEND_BUFFER - normal_send_size) : pg->total_credit);
        
        down(&(rq->lock));
        if (unlikely(rq->count <= 0)) {
            up(&(rq->lock));
            break;
        }

        cmd = list_first_entry(&(rq->head), struct im_cmd, list); 
        if (cmd->header.data_size_bytes + cmd_header_body_size <= left_buffer_size) {
            list_del(&(cmd->list));
            rq->count--;
            rq->page_cnt -= cmd->vcnt;
        } else {
            if (buffer_compare_flag == false) { // pg->total_credit is not enough
                pg->work_state = WORK_STATE_NO_BUFFER;
            }
            is_buffer_enough = 1;
        }
        up(&(rq->lock));

        // left buffer is not enough to pick up cmd from request queue
        if (is_buffer_enough == 1) {
            is_buffer_enough = 0;
            //IOMIRROR_INFO("left buffer is too small, cmd data size = %llu, left buffer = %u.", 
                //cmd->header.data_size_bytes, left_buffer_size);
            break;
        }

        if (DPP_TYPE_DATASET_START == cmd->header.cmd_type)
        {
            pg->inter_state = IM_PG_INTER_STATE_NORMAL;
            IOMIRROR_INFO("pg->sent_amount = %llu.", pg->sent_amount);
            IOMIRROR_INFO("iomirror change state from %d to %d.", pg->state, IM_PG_STATE_NORMAL);
            pg->state = IM_PG_STATE_NORMAL;
            body_len = sizeof(DPP_DATASET_START);
            // set the atomic dataset_start request id
            pg->cbt_flush_times++;
            cmd->header.request_id = pg->cbt_flush_times;
            pg->rpo_send_time = jiffies;
        }
        else
        {
            /* 补全host id和pg id后发送并插入pending queue */
            memcpy(cmd->header.host_id, pg->host_id, VM_ID_LEN);
            memcpy(cmd->header.oma_id, pg->id, VM_ID_LEN);
            cmd->header.request_id = pg->request_id++;
            body_len = sizeof(DPP_DATA);
        }

        change_to_dpp_data(&(cmd->header), &header, &data);
       
        memcpy(pg->normal_send_buffer + normal_send_size, &header, sizeof(DPP_HEADER));
        normal_send_size += sizeof(DPP_HEADER);
        memcpy(pg->normal_send_buffer + normal_send_size, data, body_len);
        normal_send_size += body_len;

        bvec = NULL;
        if (cmd->header.cmd_type != DPP_TYPE_DATASET_START) {
            bvec = cmd->bvl;
            for (i = 0; i < cmd->vcnt && bvec != NULL; i++) {
                memcpy(pg->normal_send_buffer + normal_send_size, page_address(bvec->bv_page) + bvec->bv_offset, bvec->bv_len);
                normal_send_size += bvec->bv_len;
                bvec ++;
            }
        }

        im_pg_bvl_free(cmd->bvl, cmd->vcnt);
        cmd->bvl     = NULL;
        im_pg_fresh_queue_page_count(pg, cmd->vcnt);
        cmd->vcnt    = 0;
        cmd->send_time = jiffies;
        list_add_tail(&(cmd->list), &(pq->head));
        pq->count++;

        pg->data_send_size += cmd->header.data_size_bytes;
        pg->sent_amount += cmd->header.data_size_bytes;
        pg->total_credit = (pg->total_credit > cmd->header.data_size_bytes) ? pg->total_credit - cmd->header.data_size_bytes : 0;
    }
    
    if (normal_send_size != 0) {
        ret = im_sock_xmit(pg->sock, 1, pg->normal_send_buffer, normal_send_size, 0, IM_SOCK_MAX_RETRY);
    }

    if (unlikely(ret < 0))
    {
        IOMIRROR_ERR("normal send data failed, enter re-connect, ret = %d.", ret);
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_flush_rq_and_pq(pg);
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    if ((0 == pg->total_credit) && (IM_PG_INTER_STATE_PAUSE != pg->inter_state))
    {
        /* 切换至pause状态，暂停发送数据 */
        pg->temp_state  = pg->inter_state;
        pg->inter_state = IM_PG_INTER_STATE_PAUSE;
        pg->work_state = WORK_STATE_NO_BUFFER;
        IOMIRROR_INFO_PRINT_LIMITED("have no size to send in normal state, convert to pause.");
    }
}

/**
 * Description : bio完成函数，用于同步读取数据块
 *
 * Parameters  : bio
 *               bytes_done
 *               bio
 *               error
 * Return      :
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/21
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
static int im_sync_bio_end_io(struct bio *bio, unsigned int bytes_done, int error)
#else
static void im_sync_bio_end_io(struct bio *bio, int error)
#endif
{
    complete((struct completion *) bio->bi_private);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    return 0;
#endif
}

// return 1 : indicates driver stop sending data 
inline static int set_pause_state(struct im_pg *pg)
{
    // when reboot, driver just connect to oma without sending any data, then handle reboot pause state
    pg->flow_control_pause_flag = pg->reboot_pause_state;  
    return pg->flow_control_pause_flag;
}
/**
 * Description : cbt开始状态处理函数，发送IM_CMD_CBT_START报文
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/21
 */
static void im_pg_cbt_start(struct im_pg *pg)
{
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;

    // should invoke before pg->flow_control_pause_flag
    im_pg_get_vols_bitmap_size(pg, &pg->cbt_bitmap_total_bytes, &pg->verify_bitmap_total_bytes);
    pg->verify_bitmap_total_bytes = 0; // verify bitmap is all zero
    pg->verify_bitmap_send_sum = 0; // verify bitmap data has finish sending 
    pg->cbt_bitmap_send_sum = 0; // 每次进CBT都要重新计算CBT bitmap的大小，重置cbt模式下已发送的数据量
    pg->queuePageCnt = 0; // resync和cbt状态不用等待flush request queue data再pause
    pg->work_mode = (pg->work_mode != WORK_MODE_INITIALIZE ? WORK_MODE_SYNCING : pg->work_mode); // includes cbt 和 normal state

    if (set_pause_state(pg) == 1) {
        return; // stop send any disk data
    }

    // 需要延迟一段时间发送的情况
    if (unlikely(im_pg_delay_send_dataset(pg) == 1)) {
        return;
    }

    if (pg->state == IM_PG_STATE_VERIFY) {
        IOMIRROR_INFO("iomirror change state from VERIFY to CBT.");
        pg->state = IM_PG_STATE_CBT;
    }

    // enter IM_PG_INTER_STATE_CBT_START in CBT mode, should first send a DPP_TYPE_DATASET_START.
    if (IM_PG_STATE_CBT == pg->state)
    {
        IOMIRROR_INFO("im_pg_cbt_start send dataset start in state %d.", pg->state);
        im_pg_send_dataset(pg);
        pg->sent_amount = 0;
    }
    else
    {
/*
#if 0
        // 刷pending queue至cbt位表
        while (!list_empty(&(pq->head)))
        {
            cmd = list_first_entry(&(pq->head), struct im_cmd, list);
            list_del(&(cmd->list));
            pq->count--;
            //pq->page_cnt -= cmd->vcnt;

            if (likely(DPP_TYPE_DATA == cmd->header.cmd_type))
            {
                vol = (struct im_volume *)(cmd->private);
                hbitmap_set(vol->bitmap, cmd->header.data_offset,
                            cmd->header.data_size_bytes / IM_SECTOR_SIZE);
            }
            //丢弃一致性校验标签报文
            else if ( (DPP_TYPE_DATASET_START == cmd->header.cmd_type) || (DPP_TYPE_RESYNCSET_START == cmd->header.cmd_type) )
            {
                IOMIRROR_INFO("discard pending DPP_TYPE_DATASET_START cmd.");
            }
            else
            {
                IOMIRROR_WARN("error cmd in pending queue, type=%d.", cmd->header.cmd_type);
            }

            im_pg_cmd_free(cmd);
            cmd = NULL;
        }
#endif
*/
    }

    /* 为pg中所有的卷初始化发送CBT数据位表bitmap_send */
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        BitmapItInit(vol->hbi_send, vol->bitmap, 0);
    }

    if (IM_PG_INTER_STATE_PAUSE == pg->inter_state)
    {
        pg->temp_state = IM_PG_INTER_STATE_CBT_DATA;
    }
    else
    {
        pg->inter_state = IM_PG_INTER_STATE_CBT_DATA;
    }
}


/**
 * Description : 释放bio
 *
 * Parameters  : bio - 待释放的bio
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/22
 */
static inline void im_pg_bio_free(struct bio *bio)
{
    int i = 0;
    struct bio_vec *bvec = NULL;

    if (NULL != bio)
    {
        bio_for_each_segment(bvec, bio, i)
        {
            if (NULL != bvec->bv_page)
            {
                __free_page(bvec->bv_page);
            }
        }
        bio_put(bio);
    }
}


/**
 * Description : 申请bio
 *
 * Parameters  : bdev - bio所需处理的块设备
 *               sector - 读操作起始扇区号
 *               size - 读大小
 *               event - 完成事件，用于回调
 * Return      : 成功返回申请到的bio
 *               失败返回NULL
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/22
 */
static inline struct bio *im_pg_bio_alloc(struct block_device *bdev,
                 sector_t sector, unsigned int size, struct completion *event)
{
    int i = 0;
    int ret = 0;
    int page_cnt = 0;
    unsigned int len  = 0;
    struct bio *bio   = NULL;
    struct page *page = NULL;

    page_cnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    bio = bio_alloc(GFP_KERNEL, page_cnt + 1);
    if (NULL == bio)
    {
        IOMIRROR_ERR("bio_alloc failed, page_cnt=%d.", page_cnt);
        goto fail;
    }

    bio->bi_private = event;
    bio->bi_sector  = sector;
    bio->bi_bdev    = bdev;
    bio->bi_end_io  = im_sync_bio_end_io;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
    bio->bi_rw      = READ | (1UL << REQ_SYNC);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
    bio->bi_rw      = READ | (1UL << BIO_RW_SYNC);
#else
    bio->bi_rw      = READ | (1UL << BIO_RW_SYNCIO);
#endif

    /* 申请page，并添加至bio */
    len  = PAGE_SIZE;
    for (i = 0; i < page_cnt; i++)
    {
        page = alloc_page(GFP_NOIO);
        if (NULL == page)
        {
            IOMIRROR_ERR("alloc_page failed.");
            goto fail;
        }

        len = (len > size) ? size : PAGE_SIZE;

        ret = bio_add_page(bio, page, len, 0);
        if (len != ret)
        {
            IOMIRROR_ERR("bio_add_page failed, ret=%d.", ret);
            goto fail;
        }
        size -= len;
    }

    return bio;

fail:

    im_pg_bio_free(bio);
    return NULL;
}


/**
 * Description : 读取一个块的数据
 *
 * Parameters  : bdev - 待读取的块设备
 *               sector - 读操作其实扇区号
 *               size - 读大小
 *               bvl - 返回读取到的数据
 * Return      : >0:执行成功,返回读取到的vcnt数
 *               -1:执行失败
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/28
 */
static inline int im_pg_read_block(struct block_device *bdev,
                    sector_t sector, unsigned int size, struct bio_vec **bvl)
{
    int i = 0;
    int vcnt = 0;
    unsigned int len     = 0;
    struct bio *bio      = NULL;
    struct bio_vec *bvec = NULL;
    struct completion event;

    /* 申请bvl，用与返回数据 */
    vcnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    bvec = kzalloc((sizeof(struct bio_vec) * vcnt), GFP_KERNEL);
    if (NULL == bvec)
    {
        IOMIRROR_ERR("kzalloc for bvec failed.");
        goto fail;
    }
    *bvl = bvec;

    vcnt = 0;
    init_completion(&event);
    /* 设备限制，每次最多读取IM_PG_MAX_READ_SIZE，需多次读 */
    while (0 != size)
    {
        len = MIN(IM_PG_MAX_READ_SIZE, size);
        bio = im_pg_bio_alloc(bdev, sector, len, &event);
        if (NULL == bio)
        {
            IOMIRROR_ERR("im_pg_bio_alloc failed.");
            goto fail;
        }

        /* 提交并等待bio处理完成 */
        submit_bio(bio_data_dir(bio), bio);
        wait_for_completion(&event);
        if (0 != bio->bi_size)
        {
            IOMIRROR_ERR("sync read block failed.");
            im_pg_bio_free(bio);
            goto fail;
        }

        vcnt += bio->bi_vcnt;
        for (i = 0; i < bio->bi_vcnt; i++)
        {
            bvec->bv_page   = bio->bi_io_vec[i].bv_page;
            bvec->bv_len    = bio->bi_io_vec[i].bv_len;
            bvec->bv_offset = bio->bi_io_vec[i].bv_offset;
            bio->bi_io_vec[i].bv_page = NULL;
            bvec++;
        }

        im_pg_bio_free(bio);
        size -= len;
        sector += len / IM_SECTOR_SIZE;
    }

    return vcnt;

fail:

    if (NULL != *bvl)
    {
        im_pg_bvl_free(*bvl, vcnt);
        *bvl = NULL;
    }

    return -1;
}


int readdata4cmd(struct im_cmd *cmd)
{
    int ret;
    struct im_volume *vol = cmd->private;

    ret = im_pg_read_block(vol->bdev, cmd->header.data_offset, cmd->header.data_size_bytes, &(cmd->bvl));
    if (ret <= 0)
    {
        IOMIRROR_ERR("im_pg_read_block failed.");
        return -1;
    }

    cmd->vcnt = ret;
    return 0;
}


int readbitmap_from_disk(void *targetdev, unsigned char *dest_data, unsigned long long start)
{
    int ret = -1;
    struct completion completion;
    struct bio *bio = NULL;
    struct page *page = NULL;
    unsigned char *bio_data = NULL;

    bio = bio_alloc(GFP_NOIO, 1);
    if(bio == NULL)
        return -1;

    page = alloc_page(GFP_NOIO);
    if(page == NULL) {
        bio_put(bio);
        return -1;
    }

    bio->bi_bdev = targetdev;
    bio->bi_sector = start;
    if(unlikely(!bio_add_page(bio, page, IM_SECTOR_SIZE, 0))) {
        goto err;
    }

    init_completion(&completion);
    bio->bi_private = &completion;
    bio->bi_end_io = im_sync_bio_end_io;
    submit_bio(READ, bio);
    wait_for_completion(&completion);

    bio_data = page_address(page);
    memcpy(dest_data, bio_data, IM_SECTOR_SIZE);
    ret = 0;
err:;
    page_cache_release(page);
    bio_put(bio);
    return ret;
}


int writebitmap_to_disk(void *targetdev, unsigned char *src_data, unsigned long long start)
{
    int ret = -1;
    struct completion completion;
    struct bio *bio = NULL;
    struct page *page = NULL;
    unsigned char *bio_data = NULL;

    bio = bio_alloc(GFP_NOIO, 1);
    if(bio == NULL)
        return -1;

    page = alloc_page(GFP_NOIO);
    if(page == NULL) {
        bio_put(bio);
        return -1;
    }

    bio_data = page_address(page);
    memcpy(bio_data, src_data, IM_SECTOR_SIZE);

    bio->bi_bdev = targetdev;
    bio->bi_sector = start;
    if(unlikely(!bio_add_page(bio, page, IM_SECTOR_SIZE, 0))) {
        goto err;
    }

    init_completion(&completion);
    bio->bi_private = &completion;
    bio->bi_end_io = im_sync_bio_end_io;
    submit_bio(WRITE_SYNC, bio);
    wait_for_completion(&completion);
    if(unlikely(!test_bit(BIO_UPTODATE, &bio->bi_flags))) {
        goto err;
    }

    ret = 0;
err:;
    page_cache_release(page);
    bio_put(bio);
    return ret;
}


static inline void im_pg_count_sended_data(struct im_pg *pg, uint64_t data_size)
{
    if (pg->state == IM_PG_STATE_CBT) {
        pg->cbt_bitmap_send_sum += data_size;
    } else {
        pg->verify_bitmap_send_sum += data_size;
    }
}

/**
 * Description : cbt状态处理函数
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/21
 */
static void im_pg_cbt_data(struct im_pg *pg, unsigned long *send_time)
{
    int i                    = 0;
    int ret                  = 0;
    int64_t sector           = 0;
    uint64_t nb_sectors      = 0;
    uint64_t size            = 0;
    uint64_t left            = 0;
    uint64_t max_require     = 0;
    struct im_volume *vol    = NULL;
    struct list_head *ptr    = NULL;
    struct im_pg_pending_queue *pq = &(pg->pq);
    struct im_cmd *p_cmd       = NULL;
    OM_BITMAP *bitmap;
    struct im_cmd cmd;

    /* 寻找cbt位表不为空的卷 */
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);

        if (IM_PG_INTER_STATE_CBT_DATA == pg->inter_state)
            bitmap = vol->bitmap;
        else
            bitmap = vol->bitmap_verify;

        if (IsBitmapEmpty(bitmap))
        {
            vol = NULL;
            continue;
        }
        else
        {
            break;
        }
    }

    /* 所有卷的bitmap均为空，cbt结束 */
    if (NULL == vol)
    {
        IOMIRROR_INFO("pg->inter_state is %d, all bitmaps are empty.", pg->inter_state);
        if (IM_PG_INTER_STATE_CBT_DATA == pg->inter_state)
        {
            pg->inter_state = IM_PG_INTER_STATE_ATOMIC_START;
        }
        else
        {
            pg->inter_state = IM_PG_INTER_STATE_CBT_START;
        }
        
        pg->work_mode = WORK_MODE_SYNCING;
        pg->work_state = WORK_STATE_NORMAL;
        goto out;
    }
    *send_time = jiffies;

    memset(&cmd, 0, sizeof(struct im_cmd));
    cmd.header.version         = IM_VERSION;
    cmd.header.magic           = IM_MAGIC;
    cmd.header.cmd_type        = DPP_TYPE_DATA;
    cmd.header.request_id      = pg->cbt_flush_times;
    cmd.header.ack_result      = 0;
    memcpy(cmd.header.host_id, pg->host_id, VM_ID_LEN);
    memcpy(cmd.header.oma_id, pg->id, VM_ID_LEN);
    memcpy(cmd.header.vol_id, vol->id, VOL_ID_LEN);
    cmd.bvl     = NULL;
    cmd.vcnt    = 0;
    cmd.buf     = NULL;
    cmd.private = vol;

    nb_sectors = (uint64_t)1U << pg->bitmap_granularity;
    /* 每轮循环最多发送IM_PG_PROCESS_CNT个cbt数据报文 */
    for (i = 0; i < IM_PG_PROCESS_CNT; i++)
    {
        size = 0;
        sector = 0;
        max_require = ((uint64_t)1U << IM_MAX_BITMAP_GRANULARITY) * IM_SECTOR_SIZE;
        if (max_require > pg->total_credit) {
            IOMIRROR_INFO_RATE_LIMITED("Credit restrain, credit buffer %llu, required %llu.",  pg->total_credit, max_require);
            max_require = pg->total_credit;
        }

        max_require /= IM_SECTOR_SIZE;
        max_require = (max_require / nb_sectors) * nb_sectors;
        if (max_require == 0) {
            pg->work_state = WORK_STATE_NO_BUFFER; // The restrain size is too small for one bit
            //IOMIRROR_INFO("The restrain size is too small for one bit.");
            goto out;
        }

        pg->work_state = WORK_STATE_NORMAL;

        sector = BitmapItNextSuccessive(vol->hbi_send, max_require, &size);
        /* 找到下一个被设置的位，若到达位表尾部，则初始化hbi至位表头 */
        if (sector < 0 || sector >= vol->sectors)
        {
            IOMIRROR_INFO("bitmap_send reached end, next_sector=%lld, vol_id=%llx-%llx, "
                          "vol_path=%s, reinit...", sector, *(unsigned long long *)vol->id, *(unsigned long long *)(vol->id+8), vol->path);
            //pg->sent_amount = pg->max_dataset_size / 2;
            BitmapItInit(vol->hbi_send, bitmap, 0);
            IOMIRROR_INFO("bitmap->count is %llu.", bitmap->count);
            msleep(IM_PG_IDLE_DELAY);
            goto out;
        }

        if (unlikely(vol->sectors <= sector + size))
        {
            size = IM_SECTOR_SIZE * (vol->sectors - sector);
        }
        else
        {
            size *= IM_SECTOR_SIZE;
        }

        if(pg->sent_amount >= (pg->max_dataset_size / 2) || (pg->inter_state == IM_PG_INTER_STATE_VERIFY_DATA && pg->last_vol != vol))// must read pg->max_dataset_size / 2 bytes when bitmap has enough data
        {
            if (IM_PG_INTER_STATE_CBT_DATA == pg->inter_state)
            {
                im_pg_send_dataset(pg);
            }
            else
            {
                if (pg->last_vol != vol) {
                    IOMIRROR_INFO("volume change, vol is %s, pg->sent_amount is %llu.", vol->path, pg->sent_amount);
                    pg->last_vol = vol;
                }
                im_pg_send_resyncset(pg, vol->id , IM_SECTOR_SIZE * vol->sectors);
            }

            pg->sent_amount = 0;

            if (pg->pause_pending == 1)
            {
                im_pg_handle_pause_pending(pg);
                return; // not send any data when im_pg_handle_pause_pending success.
            }
        }

        /* 读取一个块的数据，并发送 */
        p_cmd = (struct im_cmd *)kzalloc(sizeof(struct im_cmd), GFP_KERNEL);
        if (unlikely(NULL == p_cmd))
        {
            IOMIRROR_ERR("kzalloc for cmd failed.");
            goto out;
        }
        memcpy(p_cmd, &cmd, sizeof(cmd));

        ret = im_pg_read_block(vol->bdev, sector, size, &(p_cmd->bvl));
        if (ret <= 0)
        {
            IOMIRROR_ERR("im_pg_read_block failed.");
            goto out;
        }

        p_cmd->vcnt = ret;
        p_cmd->header.data_size_bytes = size;
        p_cmd->header.data_offset     = sector;
        
        ret = im_cmd_send(pg->sock, p_cmd);
        if (unlikely(ret < 0))
        {
            IOMIRROR_ERR("im_cmd_send failed.");
            pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
            im_pg_clear_cmd_pending_queue(pg);
            goto out;
        }

        pg->data_send_size += size;
        im_pg_count_sended_data(pg, size);

        pg->sent_amount += p_cmd->header.data_size_bytes;
        pg->total_credit = (pg->total_credit > p_cmd->header.data_size_bytes)? pg->total_credit - p_cmd->header.data_size_bytes : 0;

        /* 将位表中对应位清零 */
        left = vol->sectors - p_cmd->header.data_offset;
        BitmapResetBit(bitmap, p_cmd->header.data_offset, ((size / IM_SECTOR_SIZE) < left) ? (size / IM_SECTOR_SIZE) : left);

        im_pg_bvl_free(p_cmd->bvl, p_cmd->vcnt);
        p_cmd->bvl     = NULL;
        p_cmd->vcnt    = 0;
        p_cmd->send_time = jiffies;

        list_add_tail(&(p_cmd->list), &(pq->head));
        pq->count++;
        //pq->page_cnt += cmd->vcnt;
        p_cmd = NULL;
    }

    if ((0 == pg->total_credit) && (IM_PG_INTER_STATE_PAUSE != pg->inter_state))
    {
        /* 切换至pause状态，暂停发送数据 */
        pg->temp_state  = pg->inter_state;
        pg->inter_state = IM_PG_INTER_STATE_PAUSE;
        pg->work_state = WORK_STATE_NO_BUFFER;
        IOMIRROR_INFO_PRINT_LIMITED("have no size to send, convert to pause.");
    }

out:
    if (NULL != p_cmd)
    {
        kfree(p_cmd);
        p_cmd = NULL;
    }
    return;
}


/**
 * Description : atomic开始处理函数，发送IM_CMD_ATOMIC_START报文
 *
 * Parameters  : pg - 待处理的保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/21
 */
static void im_pg_atomic_start(struct im_pg *pg)
{
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;
    struct im_cmd *cmd  = NULL;
    enum im_pg_inter_state temp_inter_state;

    /* CBT结束，清空bitmap_send */
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        BitmapItInit(vol->hbi_send, vol->bitmap, 0);
    }

    if (IM_PG_INTER_STATE_PAUSE == pg->inter_state)
    {
        pg->temp_state = IM_PG_INTER_STATE_ATOMIC_DATA;
    }
    else
    {
        pg->inter_state = IM_PG_INTER_STATE_ATOMIC_DATA;
    }
    
    // to send left cbt data with DPP_DATASET_TYPE_CBT type
    temp_inter_state = pg->inter_state;
    pg->inter_state = IM_PG_INTER_STATE_CBT_DATA;
    im_pg_send_dataset(pg);
    pg->sent_amount = 0;
    IOMIRROR_INFO("finish sending all cbt data.");
    pg->inter_state = temp_inter_state;

    pg->cbt_bitmap_send_sum = 0; // cbt bitmap data has finish sending
    pg->cbt_bitmap_total_bytes = 0; // cbt bitmap is all zero

    IOMIRROR_INFO("rq count is %llu.", pg->rq.count);
    IOMIRROR_INFO("iomirror change state from %d to %d.", pg->state, IM_PG_STATE_ATOMIC);
    pg->state = IM_PG_STATE_ATOMIC;

    // add first consist dataset into request queue;
    cmd = (struct im_cmd *)kzalloc(sizeof(struct im_cmd), GFP_KERNEL);
    if (unlikely(NULL == cmd))
    {
        IOMIRROR_ERR("kzalloc for cmd failed.");
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    // insert a DPP_TYPE_DATASET_START into request pending, but do not set request_id now. 
    im_pg_setup_control_cmd(cmd, DPP_TYPE_DATASET_START, pg, "");
    cmd->header.request_id = 0;

    down(&(pg->rq.lock));
    list_add_tail(&(cmd->list), &(pg->rq.head));
    pg->rq.count++;
    up(&(pg->rq.lock));
}

/**
 * Description : verify开始处理函数，发送IM_CMD_VERIFY_START报文
 *
 * Parameters  : pg - 待处理的保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/29
 */
static void im_pg_verify_start(struct im_pg *pg)
{
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;
    struct im_cmd *cmd = NULL;
    struct im_pg_pending_queue *pq = &(pg->pq);

    // should invoke before pg->flow_control_pause_flag
    im_pg_get_vols_bitmap_size(pg, &pg->cbt_bitmap_total_bytes, &pg->verify_bitmap_total_bytes);
    pg->verify_bitmap_send_sum = 0;
    pg->cbt_bitmap_send_sum = 0;
    pg->work_mode = (pg->work_mode != WORK_MODE_INITIALIZE ? WORK_MODE_RESYNC : pg->work_mode);

    if (set_pause_state(pg) == 1) {
        return;
    }
    // 需要延迟一段时间发送的情况
    if (unlikely(im_pg_delay_send_dataset(pg) == 1)) {
        return;
    }

    /* 刷pending queue至verify位表 */
    while (!list_empty(&(pq->head)))
    {
        cmd = list_first_entry(&(pq->head), struct im_cmd, list);
        list_del(&(cmd->list));
        pq->count--;
        //pq->page_cnt -= cmd->vcnt;

        if (likely(DPP_TYPE_DATA == cmd->header.cmd_type))
        {
            vol = (struct im_volume *)(cmd->private);
            if (vol->bitmap_verify)
                BitmapSetBit(vol->bitmap_verify, cmd->header.data_offset,
                        cmd->header.data_size_bytes / IM_SECTOR_SIZE);
        }
        /* 丢弃一致性校验标签报文 */
        else if (DPP_TYPE_RESYNCSET_START == cmd->header.cmd_type)
        {
            if (pg->max_dataset_size != pg->sent_amount)
            {
                pg->next_vol_offset = cmd->header.data_offset;
                pg->sent_amount = pg->max_dataset_size; // set sent_amount so to send very beginning dataset
            }
        }
        else if (DPP_TYPE_DATASET_START == cmd->header.cmd_type)
        {
            IOMIRROR_INFO("discard pending DPP_TYPE_DATASET_START cmd.");
        }
        else
        {
            IOMIRROR_WARN("error cmd in pending queue, type=%d.", cmd->header.cmd_type);
        }

        im_pg_cmd_free(cmd);
        cmd = NULL;
    }

    /* 为pg中所有的卷创建一致性校验所需位表 */
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);

        if (NULL == vol->bitmap_verify)
        {
            vol->bitmap_verify = BitmapAlloc(vol->sectors, pg->bitmap_granularity, BitmapAllocFunc, BitmapFreeFunc);
            if (NULL == vol->bitmap_verify)
            {
                IOMIRROR_WARN("BitmapAlloc for vol->bitmap_verify failed.");
                goto fail;
            }
            BitmapSetBit(vol->bitmap_verify, 0, vol->sectors);
        }
        BitmapItInit(vol->hbi_send, vol->bitmap_verify, 0);
        IOMIRROR_INFO("vol %llx bitmap iterator pos is %llu, bitmap count is %llu.",
             *(unsigned long long *)(vol->id), vol->hbi_send->pos, vol->bitmap_verify->count);
    }

    if (IM_PG_INTER_STATE_PAUSE == pg->inter_state)
    {
        pg->temp_state = IM_PG_INTER_STATE_VERIFY_DATA;
    }
    else
    {
        pg->inter_state = IM_PG_INTER_STATE_VERIFY_DATA;
    }
    return;

fail:
    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);

        if (NULL != vol->bitmap_verify)
        {
            BitmapFree(vol->bitmap_verify, BitmapFreeFunc);
            vol->bitmap_verify = NULL;
        }
        //vol->verify_state = IM_VOL_VERIFY_NOT_START;
    }
    im_pg_clear_cmd_pending_queue(pg);
}


/**
 * Description : 保护组线程退出处理函数
 *
 * Parameters  : pg - 所属保护组
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/31
 */
static void im_pg_thread_cleanup(struct im_pg *pg)
{
    struct im_volume *vol = NULL;
    struct im_cmd    *cmd = NULL;
    struct im_pg_pending_vol *pending_vol = NULL;

    IOMIRROR_INFO("enter im_pg_thread_cleanup");
    IOMIRROR_INFO("migrate type=%d.", pg->migrate_type);
    IOMIRROR_INFO("exit type=%d.", pg->exit_flag);

    /**
     * 检测到关机事件触发IM_PG_EXIT_NORMAL退出,
     * 此时需要判断关机是否由容灾切换触发。
     */
    if (IM_PG_MIGRATE_PLAN == pg->migrate_type
        && IM_PG_EXIT_NORMAL == pg->exit_flag)
    {
        pg->exit_flag = IM_PG_EXIT_PLAN;
        if (IM_PG_STATE_NORMAL != pg->state)
        {
            IOMIRROR_WARN("current state=%d, can NOT exec plan migrate, "
                          "change exit type to IM_PG_EXIT_NORMAL.", pg->state);
            pg->exit_flag = IM_PG_EXIT_NORMAL;
        }
    }
    else if (IM_PG_MIGRATE_DISASER == pg->migrate_type
             && IM_PG_EXIT_NORMAL == pg->exit_flag)
    {
        pg->exit_flag = IM_PG_EXIT_DR;
    }
    else if (IM_PG_STATE_VERIFY == pg->state)
    {
        IOMIRROR_WARN("current state is IM_PG_STATE_VERIFY, "
                      "change exit type to IM_PG_EXIT_ABNORMAL.");
        pg->exit_flag = IM_PG_EXIT_ABNORMAL;
    }

    IOMIRROR_INFO("final exit type=%d.", pg->exit_flag);

    /* 清空控制报文队列 */
    im_pg_clear_cmd_pending_queue(pg);

    /* 根据退出类型选择对应的处理函数 */
    switch (pg->exit_flag)
    {
        case IM_PG_EXIT_NORMAL:
            im_pg_flush_rq_and_pq(pg);
            save_bitmap();
            save_configfile_data(); 
            break;
        case IM_PG_EXIT_ABNORMAL:
        case IM_PG_EXIT_STOP:
        case IM_PG_EXIT_DR:
            im_pg_flush_rq_and_pq(pg);
            break;
        default:
            im_pg_flush_rq_and_pq(pg);
            IOMIRROR_ERR("unknown exit_flag %d.", pg->exit_flag);
            break;
    }

    while (!list_empty(&(pg->vols)))
    {
        vol = list_first_entry(&(pg->vols), struct im_volume, list1);
        list_del_init(&(vol->list1));
        im_del_volume(vol);
        vol = NULL;
    }

    if (pg->normal_send_buffer)
    {
        vfree(pg->normal_send_buffer);
        pg->normal_send_buffer = NULL;
    }

    im_pg_free_alarms(&pg->alarm_list);

    /**
     * rmmod退出时，在删除全部卷之前，还可能有新请求插入rq，
     * 关机退出不会出现该情况.
     */
    while (!list_empty(&(pg->rq.head)))
    {
        cmd = list_first_entry(&(pg->rq.head), struct im_cmd, list);
        list_del(&(cmd->list));
        im_pg_cmd_free(cmd);
    }

    while (!list_empty(&(pg->pending_vols)))
    {
        pending_vol = list_first_entry(&(pg->pending_vols), struct im_pg_pending_vol, list);
        list_del_init(&(pending_vol->list));
        kfree(pending_vol);
        pending_vol = NULL;
    }

    if (NULL != pg->sock)
    {
        im_socket_close(pg->sock);
        pg->sock = NULL;
    }
}


/**
 * Description : 保护组线程主函数，
 *               不同复制状态间调度
 *
 * Parameters  : data - 保护组im_pg结构指针
 * Return      : 0
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static int im_pg_thread(void *data)
{
    int           i          = 0;
    int           ret        = 0;
    int           conn_delay = 0;
    int bitmap_not_set_count = 0;
    unsigned long send_time  = jiffies;
    struct im_pg  *pg        = (struct im_pg *)data;
    PALARM_ITEM   p_alarm   = NULL;

    IOMIRROR_INFO("enter im_pg_thread");

    while (!(pg->exit_flag))
    {
        /* 检测队列是否达到阈值，发送、检测心跳 */
        im_pg_check_and_flush(pg);
        im_pg_check_heartbeat_and_ext_cmd(pg);

        if (pg->flow_control_pause_flag == 1 || pg->stop_send_data == 1) //pause state
        {
            msleep(IM_PG_IDLE_DELAY); //300ms
            pg->work_state = WORK_STATE_PAUSE;
            goto HANDLE_RECEIVE_PACKET;
        }

        im_pg_check_packet_send(pg);

        /* 根据当前内部状态选择对应的处理函数 */
        /* pause state not handle the inter state */
        switch (pg->inter_state)
        {
            case IM_PG_INTER_STATE_NORMAL:
                im_pg_normal(pg, &send_time);
                break;
            case IM_PG_INTER_STATE_CBT_START:
                im_pg_cbt_start(pg);
                break;
            case IM_PG_INTER_STATE_CBT_DATA:
                im_pg_cbt_data(pg, &send_time);
                break;
            case IM_PG_INTER_STATE_ATOMIC_START:
                im_pg_atomic_start(pg);
                break;
            case IM_PG_INTER_STATE_ATOMIC_DATA:
                im_pg_normal(pg, &send_time);
                break;
            case IM_PG_INTER_STATE_VERIFY_START:
                im_pg_verify_start(pg);
                break;
            case IM_PG_INTER_STATE_VERIFY_DATA:
                im_pg_cbt_data(pg, &send_time);
                break;
            case IM_PG_INTER_STATE_CONNECT_STAGE0:
                pg->link_state = (pg->token.isValid ? LINK_STATE_BREAK_WITH_VALID_TOKEN_ID : LINK_STATE_BREAK);
                // to resolve the questione: driver send login cmd with cbt_flush_times = 0 while saved cbt_flush_times has not been 
                // delivered to driver, this will cause driver enter IM_PG_STATE_VERIFY due to less than dataset_id_sent received from login ack
                if (0 == pg->is_init)
                {
                    IOMIRROR_INFO("Bitmap is not setup, state is %d.", pg->inter_state);
                    if (im_pg_if_alarm_report_time_arrive(pg, LIMIT_ALARM_BITMAP_NOT_READY)) {
                        if (bitmap_not_set_count < BITMAP_NOT_SET_COUNT) { // 防止正常重启时HA发送位图到driver延迟过长导致的误报
                            bitmap_not_set_count++;
                            im_pg_set_alarm_happen_time(pg, LIMIT_ALARM_BITMAP_NOT_READY);
                            continue;
                        }
                        p_alarm = im_create_and_init_alarm_item();
                        if (likely(p_alarm)) {
                            snprintf(p_alarm->info, KERNEL_ALARM_INFO_DESC_LEN, "Bitmap is not ready after normal reboot, "
                                "driver state is %d.", pg->state);
                            p_alarm->error_code = ERROR_CODE_BITMAP_NOT_SET;
                            im_add_alarm_list(&pg->alarm_list, p_alarm);
                            im_pg_set_alarm_happen_time(pg, LIMIT_ALARM_BITMAP_NOT_READY);
                        }
                    }
                    msleep(IM_PG_IDLE_DELAY);
                    continue;
                }
                im_pg_connect_stage0(pg, &conn_delay);
                continue;
                break;
            case IM_PG_INTER_STATE_CONNECT_STAGE1:
                im_pg_connect_stage1(pg);
                break;
            case IM_PG_INTER_STATE_PAUSE:
            case IM_PG_INTER_STATE_CBT_START_P:
            case IM_PG_INTER_STATE_CBT_END_P:
            case IM_PG_INTER_STATE_ATOMIC_START_P:
            case IM_PG_INTER_STATE_ATOMIC_END_P:
            case IM_PG_INTER_STATE_VERIFY_START_P:
            case IM_PG_INTER_STATE_CONNECT_STAGE0_P:
            case IM_PG_INTER_STATE_CONNECT_STAGE1_P:
                msleep(IM_PG_IDLE_DELAY);
                break;

            case IM_PG_INTER_STATE_WAIT_PENDING_VOLS:
                IOMIRROR_INFO("state is %d.", pg->inter_state);
                msleep(IM_PG_IDLE_DELAY);
                continue;
                break;

            default:
                IOMIRROR_ERR("unknown internal state %d.", pg->inter_state);
                break;
        }

HANDLE_RECEIVE_PACKET:
        /* 接收并处理报文 */
        for (i = 0; i < IM_PG_PROCESS_CNT; i++)
        {
            ret = im_pg_recv_and_handle_cmd(pg);
            if (unlikely(ret < 0))
            {
                break;
            }
        }

        /* Normal状态，且无待处理写请求，sleep等待唤醒 */
        wait_event_interruptible_timeout(pg->rq.wq,
                            pg->rq.count > 0 || pg->pq.count > 0
                            || IM_PG_INTER_STATE_NORMAL != pg->inter_state,
                            IM_PG_WAIT_TIMEOUT);
    }

    im_pg_thread_cleanup(pg);
    complete_all(&(pg->event));

    IOMIRROR_INFO("leave im_pg_thread.");
    return 0;
}


/**
 * Description : 初始同步，设置全1 cbt 位表
 *
 * Parameters  : pg - 所属pg
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/23
 */
static void im_pg_setup_bitmaps_for_init(struct im_pg *pg)
{
    struct list_head *ptr = NULL;
    struct im_volume *vol = NULL;

    list_for_each(ptr, &(pg->vols))
    {
        vol = list_entry(ptr, struct im_volume, list1);
        BitmapSetBit(vol->bitmap, 0, vol->sectors);
    }
}


/**
 * Description : 根据im_pg停止保护组线程
 *
 * Parameters  : pg - 所属保护组
                 exit_type - 退出类型
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static inline void __im_stop_pg_thread(struct im_pg *pg,
                                              enum im_pg_exit_type exit_type)
{
#ifdef SUPPORT_BACKUP
    struct im_volume *vol = NULL;
#endif

    pg->exit_flag = exit_type;

#ifdef SUPPORT_BACKUP
    if (IM_NO_BACKUP_MODE != pg->backup_status)
    {
        pg->backup_status = IM_BACKUP_INIT;

        while (!list_empty(&(pg->vols)))
        {
            vol = list_first_entry(&(pg->vols), struct im_volume, list1);
            list_del_init(&(vol->list1));
            im_del_volume(vol);
            vol = NULL;
        }

        // 备份场景下并没有启动线程，所以需要单独调用下savebitmap
        save_bitmap();
        im_pg_free_alarms(&pg->alarm_list);
    }
    else
#endif
        wait_for_completion(&(pg->event));
    list_del(&(pg->list));

    kfree(pg);
    pg = NULL;
}


/**
 * Description : 根据保护组id停止保护组线程
 *
 * Parameters  : oma_id - 待退出保护组id
 *               exit_type - 退出类型
 * Return      : 成功 - 0
 *               失败 - -1，未查找到id对应保护组
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
int im_stop_pg_thread(const char *oma_id, enum im_pg_exit_type exit_type)
{
    struct im_pg *pg = NULL;
    struct list_head *ptr = NULL;

    list_for_each(ptr, &im_pg_list_head)
    {
        pg = list_entry(ptr, struct im_pg, list);
        if (0 == strncmp(pg->id, oma_id, VOL_ID_LEN))
        {
            break;
        }
        else
        {
            pg = NULL;
        }
    }

    if (NULL == pg)
    {
        return -1;
    }
    else
    {
        __im_stop_pg_thread(pg, exit_type);
        return 0;
    }
}


/**
 * Description : 停止所有保护组线程
 *
 * Parameters  : exit_type - 退出类型
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
void im_stop_all_pg_threads(enum im_pg_exit_type exit_type)
{
    struct im_pg *pg = NULL;

    while (!list_empty(&im_pg_list_head))
    {
        pg = list_first_entry(&im_pg_list_head, struct im_pg, list);
        __im_stop_pg_thread(pg, exit_type);
    }
}


/**
 * Description : 将待保护未就绪卷加入pending_vols队列
 *
 * Parameters  : pg - 所属pg
 *               vol - 待添加的未就绪volume
 * Return      : 0 - 成功
 *               -1 - 失败
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/7/25
 */
static inline int im_add_pending_volume(struct im_pg *pg,
                                                  ProtectVol *vol)
{
    struct im_pg_pending_vol *vol_add = NULL;

    vol_add = kzalloc(sizeof(ProtectVol), GFP_KERNEL);
    if (NULL == vol_add)
    {
        IOMIRROR_ERR("kzalloc for vol_add failed.");
        return -1;
    }

    memcpy(vol_add->id, vol->vol_id, VOL_ID_LEN);
    strncpy(vol_add->path, vol->disk_path, DISK_PATH_LEN);
    vol_add->sectors = 0;
    list_add_tail(&(vol_add->list), &(pg->pending_vols));

    return 0;
}


/**
 * Description : pg启动时，将被保护卷加入filter
 *
 * Parameters  : pg - 所属pg
 *               vols - 被保护卷信息
 *               vol_num - 被保护卷个数
 * Return      : 0 - 成功
 *               -1 - 失败
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/7/25
 */
static inline int im_pg_add_volumes_for_start(struct im_pg *pg,
                                    ProtectVol *vols, int vol_num)
{
    int i = 0;
    struct im_volume *vol = NULL;

    IOMIRROR_INFO("now adding volumes...");
    for (i = 0; i < vol_num; i++)
    {
        IOMIRROR_INFO("volume-%d: vol_id=%llx-%llx, vol_path=%s.",
                      i, *(unsigned long long *)((vols + i)->vol_id), *(unsigned long long *)((vols + i)->vol_id+8),  (vols + i)->disk_path);

        vol = im_add_volume((vols + i)->vol_id, (vols + i)->disk_path,
                    0, pg->bitmap_granularity, &(pg->rq));
        if (NULL == vol)
        {
            /**
             * 若为下发容灾配置启动，
             * im_add_volume失败则直接返回失败；
             * 若为随OS启动，可能由于被保护卷未完成初始化，
             * 则将该卷加入pending队列并继续。
             */
            if (pg->start_times > 0
                && 0 == im_add_pending_volume(pg, vols + i))
            {
                IOMIRROR_WARN("im_add_volume failed, maybe not ready, vol_path=%s.",
                               (vols + i)->disk_path);
                pg->inter_state = IM_PG_INTER_STATE_WAIT_PENDING_VOLS;
                continue;
            }
            else
            {
                IOMIRROR_WARN("im_add_volume failed, vol_path=%s.", (vols + i)->disk_path);
                return -1;
            }
        }
        else
        {
            /* 加入本pg卷队列 */
            list_add_tail(&(vol->list1), &(pg->vols));
        }
    }
    pg->vol_num = vol_num;

    if ( (pg->is_init) && (IM_PG_STATE_VERIFY != pg->state) )
    {
        im_pg_setup_bitmaps_for_init(pg);
    }

    return 0;
}
/**
 * Description : 对可能频繁报错的告警进行限频，每 LIMIT_ALARM_REPORT_FREQUENCY 上报一次
 *
 * Parameters  : limit_alarm_array[] ： 保存所有需限频的告警最近一次发生的时间
 * Return      : 
 *
 * Author      : z00455045
 * Date        : 2019/12/21
 */
static inline void im_pg_init_limit_alarm_report(unsigned long limit_alarm_array[])
{
    limit_alarm_array[LIMIT_ALARM_BITMAP_NOT_READY] = 0;
    limit_alarm_array[LIMIT_ALARM_RECV_FAILED_ACK_FORM_OMA] = 0;
}

/**
 * Description : 告警限频需要，每次添加到告警list后，更新该告警类型的最后一次发生时间
 *
 * Parameters  : pg ：结构体变量limit_alarm_report_rate保存所有需限频的告警最近一次的发生的时间
 *               alarm_index ： enum值， 告警类型
 * Return      : 
 *
 * Author      : z00455045
 * Date        : 2019/12/21
 */
static inline void im_pg_set_alarm_happen_time(struct im_pg * pg, int alarm_index)
{
    if (unlikely(alarm_index < 0 || alarm_index >= LIMIT_ALARM_BUTTOM)) {
        return;
    }

    pg->limit_alarm_report_rate[alarm_index] = jiffies;
    if (unlikely(pg->limit_alarm_report_rate[alarm_index] == 0)) {
        pg->limit_alarm_report_rate[alarm_index] = 1; // = 0有特殊意义，只能在初始化的时候等于0
    }
}

/**
 * Description : 判断告警类型（alarm_index指定）距离最后一次发生时间是否达到限频要求
 *
 * Parameters  : pg ：结构体变量limit_alarm_report_rate保存所有需限频的告警最近一次的发生的时间
 *               alarm_index ： enum值， 告警类型
 * Return      : true : 该类型告警可以再次加入到list
 *
 * Author      : z00455045
 * Date        : 2019/12/21
 */
static inline bool im_pg_if_alarm_report_time_arrive(struct im_pg * pg, int alarm_index)
{
    if (unlikely(alarm_index < 0 || alarm_index >= LIMIT_ALARM_BUTTOM)) {
        return false;
    }

    // 如果告警最近一次发生的时间为0（只能在初始化时为0），则直接返回true
    if (unlikely(pg->limit_alarm_report_rate[alarm_index] == 0)) {
        return true;
    }

    // time_after(x, y) returns true if the time x is after time y
    return (time_after(jiffies, pg->limit_alarm_report_rate[alarm_index] + LIMIT_ALARM_REPORT_FREQUENCY));
}
/**
 * Description : 启动保护组线程
 *
 * Parameters  : oma_id - 待创建保护组的id
 *               token_id - IM_PG_START_INIT 和 IM_PG_START_VERIFY模式下开启保护需要传递token (16bytes)
 *               host_id - 主机id
 *               vrg_ip - VRG IP
 *               vrg_port - VRG Port
 *               exp_rpo - RPO，秒
 *               bitmap_granularity - 数据块大小，512*2^g
 *               vol_num - 被保护卷个数
 *               vols - 被保护卷信息
 *               start_type - 启动类型
 *               start_times - 保护组创建后启动次数
 *               pause_state - 1 : driver will not send data 
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/4/29
 */
int im_start_pg_thread(const char *oma_id, const char *token_id, 
                             const char *host_id, uint32_t vrg_ip,
                             uint32_t vrg_port, uint32_t exp_rpo,
                             uint8_t bitmap_granularity, int vol_num,
                             ProtectVol *vols,
                             enum im_pg_start_type start_type,
                             uint64_t start_times,
                             uint8_t pause_state)
{
    struct im_pg *pg = NULL;
    struct im_volume *vol = NULL;
    struct im_cmd *cmd = NULL;
    struct im_pg_pending_vol *pending_vol = NULL;

    IOMIRROR_INFO("start pg thread now, oma_id=%llx, host_id=%llx, vrg_ip=%u, "
                  "vrg_port=%u, exp_rpo=%u, granularity=%u, vol_num=%d, "
                  "start_type=%d, star_times=%llu, pause_state=%u.",
                  *(long long *)oma_id, *(long long *)host_id, vrg_ip, vrg_port, exp_rpo, bitmap_granularity,
                  vol_num, start_type, start_times, pause_state);

    /* 创建并初始化pg */
    pg = (struct im_pg *)kzalloc(sizeof(*pg), GFP_KERNEL);
    if (unlikely(NULL == pg))
    {
        IOMIRROR_ERR("kzalloc for pg failed.");
        goto fail;
    }

    memcpy(pg->id, oma_id, VM_ID_LEN);
    memcpy(pg->host_id, host_id, VM_ID_LEN);
    pg->vrg_ip               = vrg_ip;
    pg->vrg_port             = vrg_port;
    pg->exp_rpo              = HZ * exp_rpo;
    pg->bitmap_granularity   = bitmap_granularity;
    pg->cbt_flush_times      = 0;
    pg->start_times          = start_times;
    pg->verify_times         = 0;
    pg->connect_times        = 0;
    pg->vol_num              = 0;
    pg->flow_control_pause_flag = 0;
    pg->reboot_pause_state   = pause_state;
    pg->pause_pending        = 0;
    pg->resume_pending       = 0;
    pg->hb_send_time         = jiffies;
    pg->hb_recv_time         = jiffies;
    pg->last_vol             = NULL;
    pg->normal_send_buffer   = NULL;
    pg->stop_send_data       = 0;
    pg->queuePageCnt         = 0;

    // statistics info
    pg->rpo_send_time        = jiffies;
    pg->write_io_size        = 0;
    pg->write_throughout     = 0;
    pg->write_iops           = 0;
    pg->real_time_write_io   = 0;
    pg->data_send_speed      = 0;
    pg->data_send_size       = 0;
    pg->cbt_bitmap_total_bytes     = 0;
    pg->verify_bitmap_total_bytes  = 0;
    pg->cbt_bitmap_send_sum        = 0;
    pg->verify_bitmap_send_sum     = 0;
    pg->statistics_jiffies.iops_time = jiffies;
    pg->statistics_jiffies.rpo_time  = jiffies;
    pg->statistics_jiffies.data_trans_time = jiffies;
    pg->work_state = WORK_STATE_NORMAL;
    pg->work_mode = WORK_MODE_INITIALIZE;
    memset(pg->token.token_id, 0, TOKEN_ID_LEN);
    pg->token.isValid = false;
    pg->link_state = LINK_STATE_BREAK;

    if (im_is_token_valid(token_id, TOKEN_ID_LEN)) {
        switch(start_type) {
            case IM_PG_START_NORMAL:  // os reboot mode = IM_PG_START_NORMAL 情况没有token_id
                break;
            case IM_PG_START_INIT:
            case IM_PG_START_VERIFY:
                pg->token.isValid = true;
                memcpy(pg->token.token_id, token_id, TOKEN_ID_LEN);
                pg->link_state = LINK_STATE_BREAK_WITH_VALID_TOKEN_ID;
                break;
            default:
                IOMIRROR_ERR("type %d is not defined.", start_type);
                break;
        }
    }

    im_init_alarm_list(&pg->alarm_list);
    im_pg_init_limit_alarm_report(pg->limit_alarm_report_rate);
    pg->failed_ack_count = 0;
    pg->end_delay_time = 0;
    pg->delay_time_count = 0;
    pg->last_failed_ack_count = 0; 

    pg->normal_send_buffer = (uint8_t *)vmalloc(MAX_NORMAL_SEND_BUFFER);
    if (pg->normal_send_buffer == NULL)
    {
        IOMIRROR_ERR("vmalloc buffer failed.");
        goto fail;
    }

    memset(pg->normal_send_buffer, 0, MAX_NORMAL_SEND_BUFFER);
    INIT_LIST_HEAD(&(pg->vols));
    INIT_LIST_HEAD(&(pg->pending_vols));
    pg->sock                 = NULL;
#ifdef SUPPORT_BACKUP
    pg->rq.pg = pg;
#endif
    INIT_LIST_HEAD(&(pg->rq.head));
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
    sema_init(&(pg->rq.lock), 1);
#else
    init_MUTEX(&(pg->rq.lock));
#endif
    init_waitqueue_head(&(pg->rq.wq));
    INIT_LIST_HEAD(&(pg->pq.head));
    INIT_LIST_HEAD(&(pg->cmd_pq.head));

    /* 根据启动状态设置pg状态及内部状态 */
    switch (start_type)
    {
        case IM_PG_START_INIT:
            pg->state   = IM_PG_STATE_CBT;
            pg->is_init = 1;
            break;

        case IM_PG_START_NORMAL:
            pg->state   = IM_PG_STATE_CBT;
            pg->is_init = 0;
            break;

        case IM_PG_START_VERIFY:
            pg->state   = IM_PG_STATE_VERIFY;
            pg->is_init = 1;
            break;

#ifdef SUPPORT_BACKUP
        case IM_PG_START_BACKUP:
            pg->state   = IM_PG_STATE_NORMAL;
            pg->is_init = 0;
            pg->backup_status = IM_BACKUP_INIT;
            break;
#endif

        default:
            IOMIRROR_ERR("unknown start type.");
            goto fail;
            break;
    }

    pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
    pg->temp_state  = IM_PG_INTER_STATE_MAX;
    pg->exit_flag   = IM_PG_RUNNING;
    pg->task        = NULL;
    init_completion(&(pg->event));

    /* 向filter中添加需要保护的卷 */
    if (im_pg_add_volumes_for_start(pg, vols, vol_num) < 0)
    {
        goto fail;
    }

#ifdef SUPPORT_BACKUP
    if (IM_PG_START_BACKUP != start_type)   /* 备份模式不需要启动保护组工作线程 */
    {
#endif
        /* 准备就绪，启动保护组工作线程 */
        pg->task = kthread_run(im_pg_thread, pg, pg->id);
        if (IS_ERR(pg->task))
        {
            IOMIRROR_ERR("kthread_run failed.");
            pg->task = NULL;
            goto fail;
        }
#ifdef SUPPORT_BACKUP
    }
#endif
    list_add(&(pg->list), &im_pg_list_head);

    return 0;

fail:

    if (NULL != pg)
    {
        while (!list_empty(&(pg->vols)))
        {
            vol = list_first_entry(&(pg->vols), struct im_volume, list1);
            list_del_init(&(vol->list1));
            im_del_volume(vol);
            vol = NULL;
        }
        while (!list_empty(&(pg->pending_vols)))
        {
            pending_vol = list_first_entry(&(pg->pending_vols), struct im_pg_pending_vol, list);
            list_del_init(&(pending_vol->list));
            kfree(pending_vol);
            pending_vol = NULL;
        }

        /* 此时有可能已截获数据，全部清空释放 */
        while (!list_empty(&(pg->rq.head)))
        {
            cmd = list_first_entry(&(pg->rq.head), struct im_cmd, list);
            list_del(&(cmd->list));
            im_pg_cmd_free(cmd);
            cmd = NULL;
        }
        
        if (pg->normal_send_buffer)
        {
            vfree(pg->normal_send_buffer);
            pg->normal_send_buffer = NULL;
        }

        kfree(pg);
        pg = NULL;
    }

    return -1;
}

