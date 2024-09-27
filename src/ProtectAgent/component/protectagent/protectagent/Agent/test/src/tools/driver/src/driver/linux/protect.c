/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : protect.c
 * Author      : WANG Chao/w00238084
 * Date        : 2014/2/18
 * Version     : 1.0
 *
 * Description : IOMirror������ʵ��:
 *               1.�����鴴����ɾ��
 *               2.��������
 *               3.����CBT
 *               4.ATOMIC
 *               5.һ����У��
 *               6.��������������
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
 * Description : �ͷű��Ľṹ��
 *
 * Parameters  : cmd - ���ͷŵı���ָ�룬������ȷ��ָ����Ч
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
 * Description : ������Ʊ���pending queue��
 *               �������������ȳ���ʹ��
 *
 * Parameters  : pg - ��������cmd pending queue���ڱ�����ָ��
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

    /* ˢrequest queue��cbtλ�� */
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
        /* ����һ����У���ǩ���� */
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

    /* ����ǰΪCBT״̬����ˢ��CBT���ݷ���λ��bitmap_send */
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
 * Description : ��request queue��pending queue�е�����ˢ��cbtλ��
 *
 * Parameters  : pg - ����������
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

    /* ˢrequest queue��cbtλ�� */
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
        /* ����һ����У���ǩ���� */
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

    /* ˢpending queue��cbtλ�� */
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
        /* ����һ����У���ǩ���� */
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

    /* ����ǰΪCBT״̬����ˢ��CBT���ݷ���λ��bitmap_send */
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
    pg->failed_ack_count = 0; // �յ��˷�����ȷflag��ack�������ӳٷ���dataset��
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
 * Description : DPP_TYPE_DATASET_DONE���Ĵ�����
 *
 * Parameters  : pg - ����������
 *               cmd - ��������
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
    // �˴����뱣֤�յ���dataset done��request id��δ�յ���dataset��ID
    // �緢��dataset start request=5\6\7,��ô�յ��ı�����5������յ�����6
    // ��˵��5��û���յ��ģ�Ҳ����Զ�˲�һ���������ˣ����������״̬
    // 2019-07-16 ͬ��windows���߼�������dataset start request=5\6\7,
    // ����յ�����4,��˵��driver�Ѿ�����flush�������������4�İ���ֱ�ӳɹ�

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
        //ˢpending queue��cbtλ��
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
            //����һ����У���ǩ����
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

        //����ǰ״̬Ϊcbt��verify���������л���cbt״̬
        if (IM_PG_STATE_CBT != pg->state
            && IM_PG_STATE_VERIFY != pg->state)
        {
            pg->state = IM_PG_STATE_CBT;

            //�������ӹ����в��л��ڲ�״̬
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

            //��ΪPAUSE״̬����������ʱ״̬
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
        // ˢ���յ���dataset ID
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
 * Description : DPP_TYPE_RESYNCSET_DONE���Ĵ�����
 *
 * Parameters  : pg - ����������
 *               cmd - ��������
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
        // ˢpending queue��cbtλ��
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
            // ����һ����У���ǩ����
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
            // �������ӹ����в��л��ڲ�״̬
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

            // ��ΪPAUSE״̬����������ʱ״̬
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
 * Description : �����ֽ��е�һ��Ϊ1��λ
 *
 * Parameters  : c - �������ַ�
 * Return      : �����ֽ�����һ��Ϊ1λ��λ�ã�ȫ�㷵��-1
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
 * Description : �����ֽ���ĳһλΪ1
 *
 * Parameters  : nr - �����õ�bitλ��
 *               addr - �����õ��ֽڵ�ַ
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
 * Description : ���յ���λ�������ݾ�ǰλ��ϲ�
 *
 * Parameters  : granularity - hbitmap����
 *               cmd - �յ���bitmap����
 *               vol - ���ϲ�λ��ľ�
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

    //����page�е�ÿһ���ֽ�ѭ��һ��
    while (len > 0)
    {
        while (1)
        {
            //�ҵ���ǰ�ֽ��е���һ��Ϊ1��λ������cbtλ��
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
 * Description : DPP_TYPE_SESSION_LOGIN_ACK���Ĵ�����
 *
 * Parameters  : pg - ����������
 *               cmd - ��������
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

    // �ж�pg��dataset״̬
    if (pg->cbt_flush_times_done > dataset_done)
    {
        IOMIRROR_ERR("Dataset_id_done %llu is higher than received one %llu, change to verify mode.", pg->cbt_flush_times_done, dataset_done);

        // ����verify״̬�����¿�ʼͬ��
        pg->state = IM_PG_STATE_VERIFY;
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    // linux�ڷ��͵�ʱ��ʹ��+1�����ж�����Ҫʹ��С�ڣ�����С�ڵ���
    if (pg->cbt_flush_times < dataset_sent)
    {
        IOMIRROR_ERR("Dataset_id %llu is less than received one %llu, change to verify mode", pg->cbt_flush_times, dataset_sent);

        // ����verify״̬�����¿�ʼͬ����ʹ��login ack�е�dataset id���г�ʼ��
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

    /* ��cmd pending queue��Ѱ�Ҷ�Ӧ�ı��� */
    list_for_each(ptr, &(cmd_pq->head))
    {
        p_cmd = list_entry(ptr, struct im_cmd, list);
        break;

        // 20190729 ��ǰֻ�е�¼cmd�ŵ�cmd_pq�У�cmd_pq��iomirror�е�Ŀ����
        // ��¼������������ŵ�queue��
        // 1.��ʱ����ʹ��cmd_pqȥ�ظ���¼��
        // 2.����iomirror��Ϊ��֧��һ��host֧�ֶ��Ŀ�Ķ˵ĳ�������ǰMobility��֧�֣�ע�ʹ˴���
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
        /* �յ�����CONNECT_VRG_ACK���л�����һ״̬ */
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
 * Description : IM_CMD_PAUSE���Ĵ�����
 *
 * Parameters  : pg - ����������
 *               cmd - ��������
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

    /* ��������ʱ������ñ��ģ�ֱ�Ӷ��� */
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
            /* �л���pause״̬����ͣ�������� */
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
 * Description : DPP_TYPE_HEARTBEAT_ACK���Ĵ�����
 *
 * Parameters  : pg - ����������
                 cmd - ��������
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
 * Description : ���ա��ַ����ģ����Ĵ��������
 *
 * Parameters  : pg - ����������
 * Return      : �ɹ� - 0
 *               ʧ�� - -1�����ձ���ʧ��
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

    /* pg�����󣬵�һ����vrg��������ǰsockΪNULL */
    if (unlikely(NULL == pg->sock || IM_PG_INTER_STATE_CONNECT_STAGE0 == pg->inter_state))
    {
        IOMIRROR_INFO_PRINT_LIMITED("im_pg_recv_and_handle_cmd break, state %d, wait to send login cmd or "
            "wait to receive a ACK.", pg->inter_state);
        ret = -1;
        goto out;
    }

    /**
     * ����һ������:
     * ���ճ�ʱֱ���˳�������
     * �����쳣��������״̬���˳���
     * �յ�Magic�����ģ���������״̬���˳���
     */
    ret = im_cmd_recv(pg->sock, &cmd);
    if (unlikely(ret < 0))
    {       
        if (ret == -2) { // -2��ʾ���յ�ʧ�ܵ�ack
            pg->failed_ack_count += 1;
            if (im_pg_if_alarm_report_time_arrive(pg, LIMIT_ALARM_RECV_FAILED_ACK_FORM_OMA)) {
                p_alarm_item = im_create_and_init_alarm_item();
                if (likely(p_alarm_item != NULL)) {
                    snprintf(p_alarm_item->info, KERNEL_ALARM_INFO_DESC_LEN,
                        "Driver has received continuous %llu failed ack, driver state : %d.",
                        pg->failed_ack_count, pg->state);
                    p_alarm_item->error_code = ERROR_CODE_BITMAP_RECV_FAILED_ACK;
                    im_add_alarm_list(&pg->alarm_list, p_alarm_item);  // �澯��Ϣ����list
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
    /* ���ݱ�������ѡ��handle���������� */
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
 * Description : ��ʼ�����Ʊ���
 *
 * Parameters  : cmd - �ѷ���ÿռ�Ĵ���������ָ��
 *               type - ��������
 *               pg - ����������
 *               vol_id - ��id
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
 * Description : ����pause pending״̬������DPP_TYPE_FLUSH�󣬽�����ͣ״̬
 *
 * Parameters  : pg - ����������
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
 * Description : ���㱣����ʣ��δ�������������ֽڣ���CBTλͼ��verifyλͼ�ֱ����
 *
 * Parameters  : pg
 *              cbt_size  CBTλͼδ�������ݴ�С
 *              verify_size  verifyλͼδ�������ݴ�С
 * Return      : ��
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
 * Description : ����DPP_ATTENTION��acitivity���ͱ��ģ�����CBT��resyncģʽ�·���
 *
 * Parameters  : pg - ����������
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

    //��������״̬�������� DPP_ATTENTION_OPERATION_ACTIVITY
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
 * Description : ����һ�����Ʊ���
 *
 * Parameters  : pg - ����������
 *               type - ����Ҫ���Ϳ��Ʊ��ĵ�����
 *               state - ���ͳɹ��������л������ڲ�״̬���粻�л����ǰ״̬
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
    /* �������Ʊ��ģ�������cmd pending queue */
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
 * Description : �жϵ�ǰrequest queue��pending queue�ܴ�С�Ƿ񳬹���ֵ��
 *               �糬������������ˢ��λ���л���cbt״̬
 *
 * Parameters  : pg - ������ı�����ָ��
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

    /* ���������ﵽ��ֵ��ˢ��cbtλ�� */
    IOMIRROR_INFO("Total %llu pages, request queue is full, will flush...", count);
    im_pg_flush_rq(pg, count);

    /* ����ǰ״̬Ϊcbt��verify���������л���cbt״̬ */
    if (IM_PG_STATE_CBT != pg->state
        && IM_PG_STATE_VERIFY != pg->state)
    {
        IOMIRROR_INFO("iomirror change state form %d to %d.", pg->state, IM_PG_STATE_CBT);
        pg->state = IM_PG_STATE_CBT;

        /* �������ӹ����в��л��ڲ�״̬ */
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

        /* ��ΪPAUSE״̬����������ʱ״̬ */
        if (IM_PG_INTER_STATE_PAUSE == pg->inter_state)
        {
            pg->temp_state = IM_PG_INTER_STATE_CBT_START;
            im_pg_clear_cmd_pending_queue(pg);
        }
    }
}


/**
 * Description : ����̬��Ӿ��ⲿ����
 *
 * Parameters  : pg - ������ָ��
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

    /* ��filter�����Ӿ� */
    vol = im_add_volume(info->vol_id, info->disk_path,
                        0, pg->bitmap_granularity, &(pg->rq));
    if (NULL == vol)
    {
        IOMIRROR_ERR("im_add_volume failed, vol_path=%s.", info->disk_path);
        pg->ext_cmd->ret = -1;
        goto out;
    }

    /* ���뱾pg����� */
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

    /* ����ǰ״̬Ϊcbt��verify���������л���cbt״̬ */
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
 * Description : ����̬ɾ�����ⲿ����
 *
 * Parameters  : pg - ������ָ��
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

    /* �ڱ�����ľ�����в��Ҵ�ɾ���� */
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

    /* ��filter��ɾ���� */
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
 * Description : ����̬�޸ľ��ⲿ����
 *
 * Parameters  : pg - ������ָ��
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

    /* �ڱ�����ľ�����в��Ҵ��޸ľ� */
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

    /* ��filter���޸ľ� */
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
 * Description : ����������׼�������ⲿ����
 *
 * Parameters  : pg - ������ָ��
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

    /* �жϴ�����Ƿ�Ϊpending volume */
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

    /* ��filter�����Ӿ� */
    vol = im_add_volume(pending_vol->id, pending_vol->path,
                    pending_vol->sectors, pg->bitmap_granularity, &(pg->rq));
    if (NULL == vol)
    {
        IOMIRROR_ERR("im_add_volume failed, vol_path=%s.", pending_vol->path);
        pg->ext_cmd->ret = -1;
        goto out;
    }

    /* ���뱾pg����У���pending volumes������ɾ�� */
    list_add_tail(&(vol->list1), &(pg->vols));
    pg->vol_num++;
    list_del_init(&(pending_vol->list));

    if ( (pg->is_init) && (IM_PG_STATE_VERIFY != pg->state) )
    {
        BitmapSetBit(vol->bitmap, 0, vol->sectors);
    }

    /* pending volume����Ϊ�գ����д��������Ѽ���filter����ʼ���Ӹ������� */
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
 * Description : ������ͣ�ⲿ����
 *
 * Parameters  : pg - ������ָ��
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
    pg->queuePageCnt = (info->waitFlushQueueFlag ?  pg->rq.page_cnt : 0);; // ��ȡ��ʱ�������bio��ռpage����

    complete_all(&(pg->ext_cmd->comp));
}

/**
 * Description : ���ں�̬�澯list�л�ȡalarm info, ����ں�̬��ʱû�и澯�������򷵻سɹ�
 *               ���ں�̬û�в����㹻�ĸ澯��ʵ�ʷ����û�̬�ĸ澯�������ܻ�С���û�̬�������õ��ĸ�����
 *               ���Ƿ��ص����澯���������ں� MAX_ALARM_NUM_ONCE_REPORT
 * Parameters  : pg - ������ָ��
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
        info->return_alarm_num = 0; // �û�̬�ж���Ҫ
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
 * Description : �޸�driver��������
 *
 * Parameters  : pg - ������ָ��
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


        /* ׼����������״̬ */
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
 * Description : ����ָ������ⲿ����
 *
 * Parameters  : pg - ������ָ��
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
         * �˴�ˢ��verify_recv_time��
         * ���pauseǰ��һ����У��״̬��
         * ��ȷ��resume�����У��������·���
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
    /* ����DATA SET���ģ�����pending queue */
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
    /* ����RESYNC SET���ģ�����pending queue */
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
    // ����dataset�����ֵ��ȡ��ǰ���Է��͵����ֵ
    cmd_send->header.data_size_bytes = (pg->next_vol_offset + pg->max_dataset_size / 2 > vol_size)?
        vol_size - pg->next_vol_offset : pg->max_dataset_size / 2;
    // ʹ��pg��vol��¼��ƫ����
    cmd_send->header.data_offset = pg->next_vol_offset;
    // ����pg�е���һ��ƫ�������Թ��´η���ʹ��
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
 * Description : �ⲿDPP_ATTENTION(activity)��RPO����dataset�����Ʊ����ط����
 *
 * Parameters  : pg - ����������
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

    /* ��normal״̬�£�RPO time ��1/3ʱ����dataset*/
    if ((IM_PG_INTER_STATE_NORMAL == pg->inter_state) && ((cur_time - pg->rpo_send_time) > (pg->exp_rpo / 3)))
    {
        im_pg_send_dataset(pg);
        pg->rpo_send_time = cur_time;
        pg->sent_amount = 0;
    }

    /* ������Ʊ����ط�ʱ�䣬�ط����Ʊ��� */
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
 * Description : �ⲿ������������
 *
 * Parameters  : pg - ����������
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

    /* �ж��Ƿ����ⲿ������Ҫ���� */
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

    /* ��ʱʱ����δ�յ�����ack���ģ��������� */
    if (unlikely((cur_time - pg->hb_recv_time) > IM_PG_HEARTBEAT_TIMEOUT))
    {
        IOMIRROR_ERR("heartbeat timeout, reconnect.");
        pg->inter_state = IM_PG_INTER_STATE_CONNECT_STAGE0;
        im_pg_clear_cmd_pending_queue(pg);
        return;
    }

    /* �����������ķ���ʱ������������������ */
    if ((cur_time - pg->hb_send_time) > IM_PG_HEARTBEAT_INTERVAL)
    {
        im_pg_setup_control_cmd(&cmd, DPP_TYPE_HEARTBEAT, pg, "");
        ret = im_cmd_send(pg->sock, &cmd);
        if (unlikely(ret < 0))
        {
            IOMIRROR_ERR("im_cmd_send heartbeat cmd failed.");
        }

        /* ��CBT��resync״̬�·���DPP_ATTENTION���� */
        // ����λͼ��ʣ�������������͸�OMA
        im_pg_send_activity_cmd(pg);

        pg->hb_send_time = cur_time;
    }
}


/**
 * Description : ��vrg�������ӵ�һ�׶Σ�
 *               ����socket������vrg��������DPP_TYPE_SESSION_LOGIN����
 *
 * Parameters  : pg - ����������
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

    /* ����������CONNECT_VRG���� */
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
 * Description : ��vrg�������ӵڶ��׶Σ�����IM_CMD_IOMIRROR_START����
 *
 * Parameters  : pg - ����������
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static void im_pg_connect_stage1(struct im_pg *pg)
{
    /**
     * �����������跢��IM_CMD_IOMIRROR_START���ģ�
     * ���������ɹ������ݵ�ǰ״̬ˢ���ڲ�״̬
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
// ����Normal״̬��pause����pg->queuePageCnt��ֵ��ÿ�δ�request queueȡ��vcnt��pages��Ҫ���ñ�����
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
    const int max_failed_ack_count = 7; // 2^(7-1) = 64s dataset֮���������ӳ�64��
    
    // ��Ҫ�ӳٷ��͵�����
    if (pg->delay_time_count == 0 && pg->failed_ack_count != 0 && pg->last_failed_ack_count != pg->failed_ack_count) {
        limit_failed_ack_count = (pg->failed_ack_count > max_failed_ack_count ? max_failed_ack_count
            : pg->failed_ack_count);
        pg->delay_time_count = make_power_2_value(limit_failed_ack_count - 1); // �˴���λ����
        IOMIRROR_INFO("Receive failed data ack, driver will delay %llu seconds to send data.", pg->delay_time_count);
        pg->last_failed_ack_count = pg->failed_ack_count;
        pg->end_delay_time = jiffies + pg->delay_time_count * HZ; // ��¼�ӳٵ��ڷ��͵�ʱ��
        // ÿ���ӳ�200���룬��ֹ�ӳ�ʱ������������������޷���������
        pg->delay_time_count *= (ms_per_second / delay_time_uint); // 200ms������ȥ�ӳ�
    }
    // �����ʼ�ӳٵ�ʱ�䣬�͵�ǰʱ��Ĳ��Ѿ�������Ҫ�ӳٵ�ʱ�䣬��ôֱ������������ӳ٣�
    // ����������ܳ����� �ڵȴ��ӳٷ���ʱ�������д���дIO, driver��ˢλͼ�����������ķѽϳ�ʱ��
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
 * Description : �������д�������
 *               ����������DPP_TYPE_DATA���ݱ���
 *
 * Parameters  : pg - ����������
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
            /* ��ȫhost id��pg id���Ͳ�����pending queue */
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
        /* �л���pause״̬����ͣ�������� */
        pg->temp_state  = pg->inter_state;
        pg->inter_state = IM_PG_INTER_STATE_PAUSE;
        pg->work_state = WORK_STATE_NO_BUFFER;
        IOMIRROR_INFO_PRINT_LIMITED("have no size to send in normal state, convert to pause.");
    }
}

/**
 * Description : bio��ɺ���������ͬ����ȡ���ݿ�
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
 * Description : cbt��ʼ״̬������������IM_CMD_CBT_START����
 *
 * Parameters  : pg - ����������
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
    pg->cbt_bitmap_send_sum = 0; // ÿ�ν�CBT��Ҫ���¼���CBT bitmap�Ĵ�С������cbtģʽ���ѷ��͵�������
    pg->queuePageCnt = 0; // resync��cbt״̬���õȴ�flush request queue data��pause
    pg->work_mode = (pg->work_mode != WORK_MODE_INITIALIZE ? WORK_MODE_SYNCING : pg->work_mode); // includes cbt �� normal state

    if (set_pause_state(pg) == 1) {
        return; // stop send any disk data
    }

    // ��Ҫ�ӳ�һ��ʱ�䷢�͵����
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
        // ˢpending queue��cbtλ��
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
            //����һ����У���ǩ����
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

    /* Ϊpg�����еľ��ʼ������CBT����λ��bitmap_send */
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
 * Description : �ͷ�bio
 *
 * Parameters  : bio - ���ͷŵ�bio
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
 * Description : ����bio
 *
 * Parameters  : bdev - bio���账��Ŀ��豸
 *               sector - ��������ʼ������
 *               size - ����С
 *               event - ����¼������ڻص�
 * Return      : �ɹ��������뵽��bio
 *               ʧ�ܷ���NULL
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

    /* ����page���������bio */
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
 * Description : ��ȡһ���������
 *
 * Parameters  : bdev - ����ȡ�Ŀ��豸
 *               sector - ��������ʵ������
 *               size - ����С
 *               bvl - ���ض�ȡ��������
 * Return      : >0:ִ�гɹ�,���ض�ȡ����vcnt��
 *               -1:ִ��ʧ��
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

    /* ����bvl�����뷵������ */
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
    /* �豸���ƣ�ÿ������ȡIM_PG_MAX_READ_SIZE�����ζ� */
    while (0 != size)
    {
        len = MIN(IM_PG_MAX_READ_SIZE, size);
        bio = im_pg_bio_alloc(bdev, sector, len, &event);
        if (NULL == bio)
        {
            IOMIRROR_ERR("im_pg_bio_alloc failed.");
            goto fail;
        }

        /* �ύ���ȴ�bio������� */
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
 * Description : cbt״̬������
 *
 * Parameters  : pg - ����������
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

    /* Ѱ��cbtλ��Ϊ�յľ� */
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

    /* ���о��bitmap��Ϊ�գ�cbt���� */
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
    /* ÿ��ѭ����෢��IM_PG_PROCESS_CNT��cbt���ݱ��� */
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
        /* �ҵ���һ�������õ�λ��������λ��β�������ʼ��hbi��λ��ͷ */
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

        /* ��ȡһ��������ݣ������� */
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

        /* ��λ���ж�Ӧλ���� */
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
        /* �л���pause״̬����ͣ�������� */
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
 * Description : atomic��ʼ������������IM_CMD_ATOMIC_START����
 *
 * Parameters  : pg - ������ı�����
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

    /* CBT���������bitmap_send */
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
 * Description : verify��ʼ������������IM_CMD_VERIFY_START����
 *
 * Parameters  : pg - ������ı�����
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
    // ��Ҫ�ӳ�һ��ʱ�䷢�͵����
    if (unlikely(im_pg_delay_send_dataset(pg) == 1)) {
        return;
    }

    /* ˢpending queue��verifyλ�� */
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
        /* ����һ����У���ǩ���� */
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

    /* Ϊpg�����еľ���һ����У������λ�� */
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
 * Description : �������߳��˳�������
 *
 * Parameters  : pg - ����������
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
     * ��⵽�ػ��¼�����IM_PG_EXIT_NORMAL�˳�,
     * ��ʱ��Ҫ�жϹػ��Ƿ��������л�������
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

    /* ��տ��Ʊ��Ķ��� */
    im_pg_clear_cmd_pending_queue(pg);

    /* �����˳�����ѡ���Ӧ�Ĵ����� */
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
     * rmmod�˳�ʱ����ɾ��ȫ����֮ǰ�������������������rq��
     * �ػ��˳�������ָ����.
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
 * Description : �������߳���������
 *               ��ͬ����״̬�����
 *
 * Parameters  : data - ������im_pg�ṹָ��
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
        /* �������Ƿ�ﵽ��ֵ�����͡�������� */
        im_pg_check_and_flush(pg);
        im_pg_check_heartbeat_and_ext_cmd(pg);

        if (pg->flow_control_pause_flag == 1 || pg->stop_send_data == 1) //pause state
        {
            msleep(IM_PG_IDLE_DELAY); //300ms
            pg->work_state = WORK_STATE_PAUSE;
            goto HANDLE_RECEIVE_PACKET;
        }

        im_pg_check_packet_send(pg);

        /* ���ݵ�ǰ�ڲ�״̬ѡ���Ӧ�Ĵ����� */
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
                        if (bitmap_not_set_count < BITMAP_NOT_SET_COUNT) { // ��ֹ��������ʱHA����λͼ��driver�ӳٹ������µ���
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
        /* ���ղ������� */
        for (i = 0; i < IM_PG_PROCESS_CNT; i++)
        {
            ret = im_pg_recv_and_handle_cmd(pg);
            if (unlikely(ret < 0))
            {
                break;
            }
        }

        /* Normal״̬�����޴�����д����sleep�ȴ����� */
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
 * Description : ��ʼͬ��������ȫ1 cbt λ��
 *
 * Parameters  : pg - ����pg
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
 * Description : ����im_pgֹͣ�������߳�
 *
 * Parameters  : pg - ����������
                 exit_type - �˳�����
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

        // ���ݳ����²�û�������̣߳�������Ҫ����������savebitmap
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
 * Description : ���ݱ�����idֹͣ�������߳�
 *
 * Parameters  : oma_id - ���˳�������id
 *               exit_type - �˳�����
 * Return      : �ɹ� - 0
 *               ʧ�� - -1��δ���ҵ�id��Ӧ������
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
 * Description : ֹͣ���б������߳�
 *
 * Parameters  : exit_type - �˳�����
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
 * Description : ��������δ���������pending_vols����
 *
 * Parameters  : pg - ����pg
 *               vol - ����ӵ�δ����volume
 * Return      : 0 - �ɹ�
 *               -1 - ʧ��
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
 * Description : pg����ʱ���������������filter
 *
 * Parameters  : pg - ����pg
 *               vols - ����������Ϣ
 *               vol_num - �����������
 * Return      : 0 - �ɹ�
 *               -1 - ʧ��
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
             * ��Ϊ�·���������������
             * im_add_volumeʧ����ֱ�ӷ���ʧ�ܣ�
             * ��Ϊ��OS�������������ڱ�������δ��ɳ�ʼ����
             * �򽫸þ����pending���в�������
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
            /* ���뱾pg����� */
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
 * Description : �Կ���Ƶ������ĸ澯������Ƶ��ÿ LIMIT_ALARM_REPORT_FREQUENCY �ϱ�һ��
 *
 * Parameters  : limit_alarm_array[] �� ������������Ƶ�ĸ澯���һ�η�����ʱ��
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
 * Description : �澯��Ƶ��Ҫ��ÿ����ӵ��澯list�󣬸��¸ø澯���͵����һ�η���ʱ��
 *
 * Parameters  : pg ���ṹ�����limit_alarm_report_rate������������Ƶ�ĸ澯���һ�εķ�����ʱ��
 *               alarm_index �� enumֵ�� �澯����
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
        pg->limit_alarm_report_rate[alarm_index] = 1; // = 0���������壬ֻ���ڳ�ʼ����ʱ�����0
    }
}

/**
 * Description : �жϸ澯���ͣ�alarm_indexָ�����������һ�η���ʱ���Ƿ�ﵽ��ƵҪ��
 *
 * Parameters  : pg ���ṹ�����limit_alarm_report_rate������������Ƶ�ĸ澯���һ�εķ�����ʱ��
 *               alarm_index �� enumֵ�� �澯����
 * Return      : true : �����͸澯�����ٴμ��뵽list
 *
 * Author      : z00455045
 * Date        : 2019/12/21
 */
static inline bool im_pg_if_alarm_report_time_arrive(struct im_pg * pg, int alarm_index)
{
    if (unlikely(alarm_index < 0 || alarm_index >= LIMIT_ALARM_BUTTOM)) {
        return false;
    }

    // ����澯���һ�η�����ʱ��Ϊ0��ֻ���ڳ�ʼ��ʱΪ0������ֱ�ӷ���true
    if (unlikely(pg->limit_alarm_report_rate[alarm_index] == 0)) {
        return true;
    }

    // time_after(x, y) returns true if the time x is after time y
    return (time_after(jiffies, pg->limit_alarm_report_rate[alarm_index] + LIMIT_ALARM_REPORT_FREQUENCY));
}
/**
 * Description : �����������߳�
 *
 * Parameters  : oma_id - �������������id
 *               token_id - IM_PG_START_INIT �� IM_PG_START_VERIFYģʽ�¿���������Ҫ����token (16bytes)
 *               host_id - ����id
 *               vrg_ip - VRG IP
 *               vrg_port - VRG Port
 *               exp_rpo - RPO����
 *               bitmap_granularity - ���ݿ��С��512*2^g
 *               vol_num - �����������
 *               vols - ����������Ϣ
 *               start_type - ��������
 *               start_times - �����鴴������������
 *               pause_state - 1 : driver will not send data 
 * Return      : �ɹ� - 0
 *               ʧ�� - -1
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

    /* ��������ʼ��pg */
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
            case IM_PG_START_NORMAL:  // os reboot mode = IM_PG_START_NORMAL ���û��token_id
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

    /* ��������״̬����pg״̬���ڲ�״̬ */
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

    /* ��filter�������Ҫ�����ľ� */
    if (im_pg_add_volumes_for_start(pg, vols, vol_num) < 0)
    {
        goto fail;
    }

#ifdef SUPPORT_BACKUP
    if (IM_PG_START_BACKUP != start_type)   /* ����ģʽ����Ҫ���������鹤���߳� */
    {
#endif
        /* ׼�����������������鹤���߳� */
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

        /* ��ʱ�п����ѽػ����ݣ�ȫ������ͷ� */
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

