/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file thinlvSegment.cpp
 *
 * @brief AFS - LVM Thin Pool logical volume segment class
 *
 */

#include "afs/ThinlvSegment.h"
#include "afs/ThinPoolSegment.h"

/**
 * @brief 构造体
 */
thinlvSegment::thinlvSegment() : segment()
{
    device_id = 0;
    pool_lv = NULL;
    m_thin_lv_disk_id = 0;
}

/**
 * @brief 析构函数
 */
thinlvSegment::~thinlvSegment()
{
    pool_lv = NULL;
}

/**
 * @brief 寻找pool卷
 *
 * @return 返回pool卷
 * NULL
 */
logicalVolume *thinlvSegment::findPoolLv()
{
    if (NULL == m_this_vg) {
        AFS_TRACE_OUT_ERROR("The space is NULL");
        return NULL;
    }

    vector<logicalVolume *>::iterator lv_iter;
    int32_t ret = 0;

    // 1.vg中匹配pool
    for (lv_iter = m_this_vg->m_lvolumes.begin(); lv_iter != m_this_vg->m_lvolumes.end(); ++lv_iter) {
        // 匹配卷名
        ret = sthin_pool.compare((*lv_iter)->m_volname);
        if (0 == ret) {
            return (*lv_iter);
        }
    }

    return NULL;
}

/**
 * @brief 初始化segment，找到匹配的pool卷
 *
 * @param info 传入参数
 *
 * @return：AFS_ERR_LVM_PART ：错误的lvm-part;
 * AFS_SUCCESS      ：成功
 */
int32_t thinlvSegment::initSegment(segment_init_info *info)
{
    pool_lv = findPoolLv();
    if (NULL == pool_lv) {
        AFS_TRACE_OUT_ERROR("Cannt find pool lv");
        return AFS_ERR_LVM_PART;
    }

    m_this_lv->m_chunk_size = pool_lv->m_chunk_size;

    return AFS_SUCCESS;
}

/**
 * @brief 寻找块号
 *
 * @param reader 句柄
 * @param start_sectno 开始扇区号
 * @param buf 缓存
 * @param count_sector 需要扇区的大小
 *
 * @return
 * ：AFS_ERR_LVM_PART：lvm错误
 * ：读到的大小
 */
int64_t thinlvSegment::findBlock(imgReader *reader, uint64_t start_sectno, char *buf, uint64_t count_sector,
    int32_t is_annotated)
{
    if (NULL == pool_lv) {
        AFS_TRACE_OUT_ERROR("The segment is NULL");
        return AFS_ERR_LVM_PART;
    }

    SEG_TYPE_ENU type;
    int64_t read_ret = 0;
    uint64_t read_size = 0;
    thinpoolSegment *pSegPool = NULL;
    list<segment *>::iterator seg_it = pool_lv->m_segments.begin();

    // 轮询找到segment类型是thin-pool
    for (; seg_it != pool_lv->m_segments.end(); ++seg_it) {
        if (NULL == (*seg_it)) {
            AFS_TRACE_OUT_ERROR("The segment is NULL");
            return AFS_ERR_LVM_PART;
        }

        type = (*seg_it)->getType();
        if (SEG_THIN_POOL == type) {
            pSegPool = dynamic_cast<thinpoolSegment *>(*seg_it);
            if (NULL == pSegPool) {
                AFS_TRACE_OUT_ERROR("The Segment space is NULL.");
                return AFS_ERR_LVM_PART;
            }

            pSegPool->m_thinlv_device_id = device_id;
            // 1.pool中data卷,利用phy_blk,找到数据
            read_ret = pSegPool->findBlock(reader, start_sectno, buf, count_sector, is_annotated);
            if (read_ret < 0) {
                AFS_TRACE_OUT_ERROR("Failed to read disk. ret = %lld", (long long)read_ret);
                return 0; // 返回read size 0
            }

            read_size += read_ret;
            // 不在当前segment 或者 未完全读完
            if ((read_ret == 0) || (read_size < count_sector)) {
                continue;
            } else {
                // 读完结束
                break;
            }
        }
    }

    return read_size;
}

/**
 * @brief 虚拟到物理地址之间的映射
 *
 * @param vaddr 虚地址（扇区）
 *
 * @return：物理地址(扇区)
 * -1
 * AFS_ERR_LVM_PART：错误lvm分区
 */
int64_t thinlvSegment::mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
{
    if (NULL == pool_lv) {
        AFS_TRACE_OUT_ERROR("The segment is NULL");
        return AFS_ERR_LVM_PART;
    }

    SEG_TYPE_ENU type;
    list<segment *>::iterator seg_it = pool_lv->m_segments.begin();
    thinpoolSegment *pSegPool = NULL;

    int64_t ret = 0;

    // 轮询找到segment类型是thin-pool
    for (; seg_it != pool_lv->m_segments.end(); ++seg_it) {
        if (NULL == (*seg_it)) {
            AFS_TRACE_OUT_ERROR("The segment is NULL");
            return -1;
        }

        type = (*seg_it)->getType();
        if (SEG_THIN_POOL == type) {
            pSegPool = dynamic_cast<thinpoolSegment *>(*seg_it);
            if (NULL == pSegPool) {
                AFS_TRACE_OUT_ERROR("The Segment space is NULL.");
                return AFS_ERR_LVM_PART;
            }

            pSegPool->m_thinlv_device_id = device_id;
            // 1.pool中data卷,利用phy_blk,找到数据
            ret = pSegPool->mapVaddrToPaddr(vaddr, disk_id);
            if (ret == -1) {
                AFS_TRACE_OUT_ERROR("Failed to map_vaddr_to_paddr. ret = %lld", (long long)ret);
                return -1;
            }

            break;
        }
    }

    return ret;
}

/**
 * @brief 获取lv的首地址
 *
 * @return：返回首地址
 */
int64_t thinlvSegment::getPartitionFirstAddr()
{
    return mapVaddrToPaddr(0, m_thin_lv_disk_id);
}

/**
 * @brief 设置属性
 *
 * @param plvm afslvm类
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t thinlvSegment::setSegProp(afsLVM *plvm)
{
    sthin_pool = plvm->parseSegThinPool();
    if (sthin_pool.empty()) {
        AFS_TRACE_OUT_ERROR("Can'nt parse thin pool");
        return AFS_ERR_LVM_PART;
    }

    device_id = plvm->parseSegDevId();
    if (0 > device_id) {
        AFS_TRACE_OUT_ERROR("Can'nt parse device id");
        return AFS_ERR_LVM_PART;
    }

    return AFS_SUCCESS;
}
