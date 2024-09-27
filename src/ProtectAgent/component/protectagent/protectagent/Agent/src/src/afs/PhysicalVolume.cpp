/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file physicalVolume.cpp
 *
 * @brief AFS - LVM physical volume.
 *
 */

#include "afs/PhysicalVolume.h"

/* * @brief 构造函数
 *
 */
physicalVolume::physicalVolume()
{
    pv_num = -1;
    uuid = "";
    dev_size = 0;
    pe_start = 0;
    pe_count = 0;
    offset = 0;
    part_id = 0;
    disk_id = 0;
}

/* * @brief 带参构造函数
 *
 * @param sdev uuid
 * @param part_num 分区索引
 * @param id uuid
 * @param devsize 设备大小
 * @param start 开的pe
 * @param count pe的计数
 * @param dsk_offset 区分偏移
 *
 */
physicalVolume::physicalVolume(string &sdev, int32_t part_num, string &id, uint64_t devsize, uint32_t start,
    uint32_t count, int32_t diskId, uint64_t dsk_offset)
{
    pv_num = -1;
    uuid = id;
    dev_size = devsize;
    pe_start = start;
    pe_count = count;
    offset = dsk_offset;
    device = sdev;
    part_id = part_num;
    disk_id = diskId;
}

/* * @brief 传入无连字符'-'格式的UUID
 *
 * @param sdev uuid
 *
 * 返回值: AFS_SUCEESS 成功，其他: 失败
 */
int32_t physicalVolume::setPVUUID(char *temp_uuid)
{
    CHECK_MEMSET_S_OK(pv_uuid2, UUID_LEN + 1, 0, UUID_LEN + 1);
    CHECK_MEMCPY_S_OK(pv_uuid2, UUID_LEN + 1, temp_uuid, UUID_LEN + 1);
    return AFS_SUCCESS;
}

/* * @brief 析构函数
 *
 */
physicalVolume::~physicalVolume() {}
