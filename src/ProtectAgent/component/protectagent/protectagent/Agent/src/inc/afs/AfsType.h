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
#ifndef INCLUDE_AFS_TYPE_H_
#define INCLUDE_AFS_TYPE_H_

#include <stdint.h>

const uint32_t AFS_SECTOR_SEZE = 512;

typedef int32_t int32;
typedef int64_t int64;
typedef void *AFS_HANDLE;
// return值为实际读到的数据长度，-1为失败
typedef int64 (*AFS_READ_CALLBACK_FUNC_t)(AFS_HANDLE handle, char *buf, int64 offset, int32 len);

/**
 * @brief 文件系统类型
 */
typedef enum AFS_FSTYPE {
    AFS_FILESYSTEM_NULL = -1, //  不支持的文件系统
    AFS_FILESYSTEM_SWAP,      //  Linux交换分区
    AFS_FILESYSTEM_EXT4,      //  EXT4文件系统
    AFS_FILESYSTEM_NTFS,      //  NTFS文件系统
    AFS_FILESYSTEM_XFS        //  XFS文件系统
} AFS_FSTYPE_t;

#define AFS_MAX_LV_NAME 256
#define AFS_PV_UUID_LEN 32
#define AFS_FS_UUID_MAX_LEN 16
#define AFS_FS_UUID_STRING_LEN 36

/**
 * @brief lvm分区信息
 */
struct LVM_parttion_info {
    uint64_t lv_offset; // LV相对分区偏移
    uint64_t lv_length; // LV分区长度
    char lv_name[AFS_MAX_LV_NAME];
    char lv_pvUUID[AFS_PV_UUID_LEN + 1];
    char lv_update;
};

/**
 * @brief 分区信息
 */
struct partition {
    unsigned char part_type; // 分区类型
    bool is_lvm;             // 是否是LVM管理
    bool is_pv_part;         // 是否是PV分区 0:单独的物理分区, 1: 物理分区作PV分区
    AFS_FSTYPE_t fstype;     // 分区文件系统类型

    int32_t part_id; // 分区ID(第一个分区ID为0)
    int32_t disk_id; // 分区在所给disk iamge对象中的编号

    // 分区信息
    uint64_t offset; // 分区偏移
    uint64_t length; // 分区长度(512倍数)

    // LVM分区
    // 物理分区可以划分多个LV提供给文件系统
    struct LVM_parttion_info lvm_info;
};

/**
 * @brief 镜像相关信息
 */
struct imgInfo {
    AFS_HANDLE Handle;
    AFS_READ_CALLBACK_FUNC_t read_call_back_func;

    int64_t imageSize; // img文件大小(单位:字节)
};

#endif /* INCLUDE_AFS_TYPE_H_ */
