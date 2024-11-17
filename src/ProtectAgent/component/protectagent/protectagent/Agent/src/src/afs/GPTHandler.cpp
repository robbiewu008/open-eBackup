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
#include "afs/GPTHandler.h"
#include "afs/RawReader.h"
#include "afs/AfsLVM.h"
#include "afs/LogMsg.h"

/**
 * @brief 解析lvm
 *
 * @param real_part_num  实际分区数
 * @param part_num       分区号
 *
 * @return
 * AFS_SUCCESS：分析成功
 * AFS_ERR_LVM_PART：分析失败
 */
int32_t GPTHandler::parseSingleEntryDoLVM(int32_t real_part_num, int32_t part_num)
{
    int32_t lv_cnt = 0;

    // LVM模式
    // LVM处理,LVM会对分区进行管理,重新设置分区偏移
    vector<struct partition> &partVect = getPartSpaceVect();
    afsLVM lvm(this, partVect, part_num - 1);
    lv_cnt = lvm.parseLVMFormat(real_part_num + 1);
    if (0 <= lv_cnt) {
        setPartnumValue(part_num + lv_cnt);
    } else if (-1 == lv_cnt || AFS_VOLUME_GROUP_EXIST == lv_cnt || AFS_PV_NO_VG_METADAT == lv_cnt) {
        return AFS_SUCCESS;
    } else if (NOT_LVM_FORMAT != lv_cnt) {
        AFS_TRACE_OUT_ERROR("Parse LVM format is failed");
        return lv_cnt;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 解析entry
 *
 * @param *part_table_data   分区表数据
 * @param part_table_num     分区号
 *
 * @return
 * AFS_SUCCESS：分析成功
 * AFS_ERR_LVM_PART：分析失败
 */
int32_t GPTHandler::parseSingleEntry(uint8_t *part_table_data, uint32_t part_table_num)
{
    int32_t ret = AFS_SUCCESS;
    struct gpt_entry *gpt_entry_start = (struct gpt_entry *)part_table_data;
    struct gpt_entry *gpt_ent = NULL;
    int32_t part_num = 0;
    struct partitionOpt part;

    // /分析每一个entry
    for (uint32_t i = 0; i < part_table_num; i++) {
        gpt_ent = &gpt_entry_start[i];

        if (gpt_ent->starting_lba == 0) {
            // 最后一个分区
            continue;
        }

        part_num = getPartnum();
        ret = setPartnum(++part_num);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("GPT set partitions num failed, ret = %d", ret);
            return ret;
        }

        part.setPartInfo(PARTITION_GPT, getPartnum() - 1, false, gpt_ent->starting_lba,
            gpt_ent->ending_lba - gpt_ent->starting_lba + 1, AFS_FILESYSTEM_NULL);

        ret = setPartition(part_num - 1, (struct partition *)(&part));
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Set partition failed. partNum=%d", part_num);
            return ret;
        }

        void *part_tmp = getPartitionPointer(part_num - 1);
        if (NULL == part_tmp) {
            AFS_TRACE_OUT_ERROR("Failed to get partition information.");
            ret = AFS_ERR_PARTITION;
            return ret;
        }

        ret = parseSingleEntryDoLVM(i, part_num);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to parse lvm.");
            return ret;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief  分析GPT分区
 *
 * @return   0： 成功
 * 负值：失败
 */

int32_t GPTHandler::parseAllOfPart()
{
    int32_t ret = AFS_SUCCESS;
    uint32_t part_table_num = 0;
    uint32_t part_entry_size = 0;
    uint32_t part_table_size = 0;
    uint8_t *part_table_data = NULL;
    struct gpt_header gpt_head;
    CHECK_MEMSET_S_OK(&gpt_head, sizeof(gpt_head), 0, sizeof(gpt_head));

    uint64_t read_size = 0;

    imgReader *reader_tmp = this->getImgReader();
    if (NULL == reader_tmp) {
        AFS_TRACE_OUT_ERROR("Cann't get reader handler.");
        return AFS_ERR_INNER;
    }

    // 读取gpt头部(位于第二个扇区，长度为92字节，按照1个扇区读取)
    int64_t stSize = sizeof(gpt_header);
    bool result = ReadBySectorsBuff(reader_tmp, reinterpret_cast<void *>(&gpt_head), SECTOR_SIZE, stSize, 1);
    if (!result) {
        AFS_TRACE_OUT_ERROR("Failed to read gpt_header data.");
        free(part_table_data);
        part_table_data = NULL;
        return AFS_ERR_IMAGE_READ;
    }

    // 处理gpt header
    part_table_num = gpt_head.num_partition_entries;
    part_entry_size = gpt_head.sizeof_partition_entry;
    part_table_size = part_table_num * part_entry_size;

    if (0 == part_table_size) {
        AFS_TRACE_OUT_ERROR("The new space of specify is zero.");
        return AFS_ERR_API;
    }

    part_table_data = (uint8_t *)calloc(1, part_table_size);
    if (NULL == part_table_data) {
        AFS_TRACE_OUT_ERROR("Failed to allocate GPT partition table memory.");
        return AFS_ERR_API;
    }

    // 读取partition entry
    result = ReadBySectorsBuff(reader_tmp, reinterpret_cast<void *>(part_table_data), 2 * SECTOR_SIZE, part_table_size, 1);
    if (!result) {
        AFS_TRACE_OUT_ERROR("Failed to read gpt_header data.");
        free(part_table_data);
        return AFS_ERR_IMAGE_READ;
    }

    ret = parseSingleEntry(part_table_data, part_table_num);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Failed to parse entry.");
        free(part_table_data);
        return ret;
    }

    free(part_table_data);
    part_table_data = NULL;

    return ret;
}

/**
 * @brief 获取GPT分区的BitMap
 *
 * @param  &bitmap  镜像的Bitmap对象
 *
 * @return 0 成功
 *
 */
int32_t GPTHandler::getBitmap(BitMap &bitmap)
{
    int32_t ret = 0;
    uint64_t index = 0;
    struct gpt_header gpt_head;
    CHECK_MEMSET_S_OK(&gpt_head, sizeof(gpt_head), 0, sizeof(gpt_head));

    AFS_TRACE_OUT_DBG("Enter GPTHandler::getBitmap");

    size_t read_len = 0;
    imgReader *reader_tmp = this->getImgReader();
    if (NULL == reader_tmp) {
        AFS_TRACE_OUT_ERROR("Cann't get reader handler.");
        return AFS_ERR_INNER;
    }

    // 偏移跳过一个扇区的MBR保留结构，获取主分区表头
    int64_t stSize = sizeof(gpt_header);
    bool result = ReadBySectorsBuff(reader_tmp, reinterpret_cast<void *>(&gpt_head), SECTOR_SIZE, stSize, 1);
    if (!result) {
        AFS_TRACE_OUT_ERROR("Failed to read GPT head data.");
        return AFS_ERR_IMAGE_READ;
    }

    // 设置主分区表的bitmap(LBA从0开始，直到第一个可用的LBA，中间保存着分区表信息)
    for (index = 0; index < gpt_head.first_usable_lba; index++) {
        ret = bitmap.bitmapSet(index);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to set bitmap.");
            return AFS_ERR_INNER;
        }
    }

    // 设置备份分区表的bitmap(LBA从最后一个可用LBA的下一个LBA开始，直到备份分区表头所在LBA，中间保存着备份分区表信息)
    for (index = gpt_head.last_usable_lba + 1; index <= gpt_head.alternate_lba; index++) {
        ret = bitmap.bitmapSet(index);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Failed to set bitmap.");
            return AFS_ERR_INNER;
        }
    }
    // 主分区表(多了一个扇区的MBR保留结构)与备份分区表内容一致，备份分区表倒着存放主分区表的信息
    return AFS_SUCCESS;
}
