/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "afs/NtfsCommon.h"
#include <string.h>
#include <cstdlib>
#include "afs/LogMsg.h"
#include "afs/Afslibrary.h"
#include "afs/AfsError.h"

/**
 * @brief 检查当前用于存储ntfs_runlist_element结构体的数组空间是否足够，
 * 如果超出则重新分配空间
 *
 * @param runlist_count        当前已经存储的Item数
 * @param **data_runlist       runlist指针
 * @param &runlist_length      runlist空间长度
 *
 * @return int32_t  0   成功
 * AFS_ERR_API   扩充空间失败
 *
 */
int32_t ntfsCommon::checkRunListSpace(uint32_t runlist_count, struct ntfs_runlist_element **data_runlist,
    uint32_t &runlist_length)
{
    int32_t ret;
    if ((runlist_length - 1) == runlist_count) {
        struct ntfs_runlist_element *new_runlist = NULL;
        new_runlist = (struct ntfs_runlist_element *)calloc(1,
            sizeof(struct ntfs_runlist_element) * (runlist_length + NTFS_RUNLIST_REALLOC_LENGTH));
        if (NULL == new_runlist) {
            AFS_TRACE_OUT_ERROR("ReAllocation memory for the data_runlist is error. New size=%d",
                (runlist_length + NTFS_RUNLIST_REALLOC_LENGTH));
            return AFS_ERR_API;
        }

        // 将原数据拷贝到新的内存中
        ret =
            memcpy_s(new_runlist, sizeof(struct ntfs_runlist_element) * (runlist_length + NTFS_RUNLIST_REALLOC_LENGTH),
            *data_runlist, sizeof(struct ntfs_runlist_element) * runlist_length);

        runlist_length += NTFS_RUNLIST_REALLOC_LENGTH; // 更新Runlist长度

        // 释放源数据的空间
        free(*data_runlist);
        *data_runlist = new_runlist;
        if (ret != EOK) {
            AFS_TRACE_OUT_ERROR("call memcpy_s failed, errno is %d", ret);
            return ret;
        }
        AFS_TRACE_OUT_INFO("ReAllocation memory for the data_runlist successfully.");
    }

    return AFS_SUCCESS;
}

/**
 * @brief 从Runlist的Buffer中，读取一个簇流项
 *
 * @param *mft_pos                Runlist的Buffer空间
 * @param mapping_first_byte      簇流项的第一个字节值
 * @param runlist_count           当前已经存储的Item数
 * @param &runlist_one_length     当前簇流项的字节长度
 * @param *map_item               存储簇流项的数组
 * @param &runlist_high           第一个字节的高4位值(起始簇号占用字节数)
 * @param &runlist_low            第一个字节的低4位值（簇数占用的字节数）
 *
 * @return  int32_t   0   成功
 * AFS_ERR_INNER   分析失败
 *
 */
int32_t ntfsCommon::getOneRunlistItem(const uint8_t *mft_pos, uint8_t mapping_first_byte, uint32_t runlist_count,
    uint8_t &runlist_one_length, uint8_t *map_item, uint8_t &runlist_high, uint8_t &runlist_low)
{
    runlist_low = mapping_first_byte & 0x0F; // 读取低4位(数据长度)
    if (0 == runlist_low) {                  // 计算的数据检查
        AFS_TRACE_OUT_ERROR("Get runlist low bit error.");
        return AFS_ERR_INNER;
    }

    // 第二个簇流项起始簇号可以为0
    runlist_high = mapping_first_byte >> 4; // 读取高4位(lcn 起始簇号)
    if ((0 == runlist_high) && (0 == runlist_count)) {
        // 计算的数据检查
        AFS_TRACE_OUT_ERROR("Get runlis high bit error.");
        return AFS_ERR_INNER;
    }

    AFS_TRACE_OUT_DBG("Runlist high is %d...low is %d", runlist_high, runlist_low);
    runlist_one_length = runlist_high + runlist_low + 1; // 该簇流项长度

    CHECK_MEMCPY_S_OK(map_item, (size_t)runlist_one_length, (u8 *)mft_pos, (size_t)runlist_one_length);

    return AFS_SUCCESS;
}

/**
 * @brief 计算一个簇流项的起始簇号和占用簇数
 *
 * @param *map_item               簇流项数组
 * @param runlist_one_length      簇流项长度
 * @param runlist_high            第一个字节的高四位值
 * @param runlist_low             第一个字节的低四位值
 * @param runlist_count           当前簇流项的ID
 * @param &start_lcn              起始簇号
 * @param &item_len               簇数
 *
 * @return
 *
 */
void ntfsCommon::getRunListItemInfo(uint8_t *map_item, uint8_t runlist_one_length, uint8_t runlist_high,
    uint8_t runlist_low, uint32_t runlist_count, int64_t &start_lcn, uint64_t &item_len)
{
    uint8_t loop = 0;    // loop为循环的临时变量
    int8_t map_sign = 0; // 存放簇流项长度字节

    // 读取一个簇流项的长度
    for (loop = runlist_low; loop >= 1; loop--) {
        item_len = item_len << NTFS_BIT_PER_BYTE;
        item_len |= (uint64_t)(map_item[loop]);
    }

    start_lcn = 0; // 从第二个簇流项开始，是相对于前一项的偏移(簇)
    if (runlist_high != 0) {
        // 读取一个簇流项的逻辑簇号lcn
        map_sign = (int8_t)map_item[runlist_one_length - 1];
        // 处理符号位,由于map_sign是有符号的，那么start_lcn最后结果是有符号的
        // 如：对于一个数0xFF,最终start_lcn为-1，而不是255
        start_lcn = (int64_t)((uint64_t)start_lcn | (uint64_t)map_sign);

        // 处理后面的数据，原理就是数据拼接。如0x1234,处理后为0x34*(2~8)+0x12
        for (loop = runlist_one_length - 2; loop > runlist_low; loop--) {
            start_lcn = (int64_t)((uint64_t)start_lcn << NTFS_BIT_PER_BYTE);
            start_lcn = (int64_t)((uint64_t)start_lcn | (uint64_t)(map_item[loop]));
        }
    }
}

/**
 * @brief 分区整个的Runlist Buffer，拆分出每个簇流项到ntfs_runlist_element数组中
 *
 * @param *mft_pos                簇流项在MFTBuffer中的起始位置
 * @param mapping_first_byte      第一个字节的值
 * @param runlist_count           当前簇流项的ID
 * @param &runlist_one_length     一个簇流项长度
 * @param runlist_count           当前簇流项的ID
 * @param **data_runlist          保存的簇流项数组
 * @param &runlist_length         簇流项数组长度
 *
 * @return  0  分析成功
 * 负数    分析失败
 */
int32_t ntfsCommon::analyzeRunList(const uint8_t *mft_pos, uint8_t mapping_first_byte, uint32_t runlist_count,
    uint8_t &runlist_one_length, struct ntfs_runlist_element **data_runlist, uint32_t &runlist_length)
{
    uint8_t runlist_low = 0;  // runlist_low为低字节数据
    uint8_t runlist_high = 0; // runlist_high为 高字节数据

    uint8_t map_item[NTFS_ONE_CLUSTER_FLOW] = {0}; // 存放读取簇流信息

    int32_t ret = 0;
    uint64_t run_item_len = 0; // 临时变量，记录簇流长度
    int64_t start_lcn = 0;     // 记录簇流中得逻辑簇号

    // 读取一个Runlist项
    ret = getOneRunlistItem(mft_pos, mapping_first_byte, runlist_count, runlist_one_length, map_item, runlist_high,
        runlist_low);
    if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Failed to get one runlist item length.");
        return ret;
    }

    // 读取一个簇流的LCN和长度
    getRunListItemInfo(map_item, runlist_one_length, runlist_high, runlist_low, runlist_count, start_lcn, run_item_len);

    // 检查Runlist分配空间
    ret = checkRunListSpace(runlist_count, data_runlist, runlist_length);
    if (ret != AFS_SUCCESS) {
        return ret;
    }

    // 为runlist赋值(一个簇流项的长度)
    (*data_runlist)[runlist_count].length = (s64)run_item_len;

    // 更新簇流数据
    if (0 == runlist_count) { // 第一条簇流信息
        (*data_runlist)[runlist_count].lcn = start_lcn;
        (*data_runlist)[runlist_count].vcn = 0;
    } else {                                                                                     // 不是第一条簇流信息
        (*data_runlist)[runlist_count].lcn = start_lcn + (*data_runlist)[runlist_count - 1].lcn; // lcn
        (*data_runlist)[runlist_count].vcn =
            (*data_runlist)[runlist_count - 1].vcn + (*data_runlist)[runlist_count - 1].length; // 虚拟簇号
    }

    return AFS_SUCCESS;
}

/**
 * @brief 计算一个MFT项的大小(一般为固定值1024字节)
 *
 * @param clusters_per_mft_record   引导扇区中描述的每个MFT项的簇数
 * @param cluster_size              簇大小
 *
 * @return uint32_t 返回MFT项的大小
 *
 */
uint32_t ntfsCommon::calculateMFTRecordSize(int8_t clusters_per_mft_record, uint32_t cluster_size)
{
    uint32_t mft_bytes = 0;
    if (0 > clusters_per_mft_record) {
        mft_bytes = (uint32_t)(1 << abs(clusters_per_mft_record));
    } else {
        mft_bytes = (uint32_t)(clusters_per_mft_record * cluster_size);
    }

    return mft_bytes;
}


/**
 * @brief 计算索引块大小(一般为固定值4096字节)
 *
 * @param clusters_per_index_block      引导扇区中描述的每个索引块的簇数
 * @param cluster_size                  簇大小
 *
 * @return uint32_t 返回索引块大小
 *
 */
uint32_t ntfsCommon::calculateIndexBlockSize(int8_t clusters_per_index_block, uint32_t cluster_size)
{
    uint32_t index_block_bytes = 0;
    if (0 > clusters_per_index_block) {
        index_block_bytes = (uint32_t)(1 << abs(clusters_per_index_block));
    } else {
        index_block_bytes = (uint32_t)(clusters_per_index_block * cluster_size);
    }

    return index_block_bytes;
}
