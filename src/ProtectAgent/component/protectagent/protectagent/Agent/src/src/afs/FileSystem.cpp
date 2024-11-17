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
#include "afs/FileSystem.h"
#include "afs/Ext4FS.h"
#include "afs/NtfsFS.h"
#include "afs/XfsFS.h"
#include "afs/LogMsg.h"
#include "afs/AfsError.h"
#include "afs/FSCommon.h"

/**
 * @brief filesystemHandler类构造函数
 *
 */
filesystemHandler::filesystemHandler()
{
    AFS_TRACE_OUT_DBG("ENTER filesystemHandler::filesystemHandler()");
    setObjType(OBJ_TYPE_FILESYSTEM);
    setMagic("unknow");
    m_fs_type = (int32_t)AFS_FILESYSTEM_NULL;
    m_reader = NULL;
}

/**
 * @brief filesystemHandler类析构函数
 *
 */
filesystemHandler::~filesystemHandler()
{
    m_reader = NULL;
}

/**
 * @brief filesystemHandler类虚函数
 *
 */
int32_t filesystemHandler::initRealHandler()
{
    return 0;
}

/**
 * @brief 设置文件系统类型
 *
 */
void filesystemHandler::setType(int type)
{
    this->m_fs_type = type;
}

/**
 * @brief 读取文件系统类型
 *
 */
int32_t filesystemHandler::getType()
{
    return this->m_fs_type;
}

/**
 * @brief 设置镜像读取的Reader
 *
 */
void filesystemHandler::setImageReader(imgReader *reader)
{
    this->m_reader = reader;
}

bool filesystemHandler::AllocBySectors(std::unique_ptr<char[]> &buffer, int64_t length, int64_t &allocSize)
{
    allocSize = length;
    if (length % VMDK_SECTOR_SIZE != 0) {
        allocSize = ((length / VMDK_SECTOR_SIZE) + 1) * VMDK_SECTOR_SIZE;
    }

    buffer = std::make_unique<char[]>(allocSize);
    if (buffer == nullptr) {
        return false;
    }
    CHECK_MEMSET_S_OK(buffer.get(), allocSize, 0, allocSize);
    return true;
}

bool filesystemHandler::ReadBySectorsBuff(imgReader *reader, void *buff, int64_t offset, int64_t length, int32_t annotated)
{
    int64_t needSize = length;
    std::unique_ptr<char[]> bufferPtr;
    if (!AllocBySectors(bufferPtr, length, needSize)) {
        AFS_TRACE_OUT_ERROR("Failed to calloc buffer.");
        return false;
    }
    int64_t readLen = reader->read(bufferPtr.get(), offset, needSize, annotated);
    if (needSize != readLen) {
        AFS_TRACE_OUT_ERROR("Failed to read data.");
        return false;
    }

    CHECK_MEMCPY_S_OK(buff, length, bufferPtr.get(), length);
    return true;
}

/**
 * @brief 获得文件系统类型和UUID
 *
 * @param *fs_type 获取的文件系统类型
 * @param *fs_uuid 获取的文件系统uuid
 *
 * @return int32_t 0 成功
 * 负数  失败
 *
 */
int32_t filesystemHandler::getFSTypeUUID(int32_t *fs_type, char *fs_uuid, size_t fs_uuid_len)
{
    unsigned char *buf = NULL;
    int32_t ret = 0;
    uint64_t read_size = 0;
    uint64_t read_cnt = 8 * SECTOR_SIZE;

    buf = (unsigned char *)calloc(1, read_cnt);
    if (NULL == buf) {
        AFS_TRACE_OUT_ERROR("Allocate memory space failed.");
        return AFS_ERR_API;
    }
    read_size = m_reader->read(buf, 0, read_cnt, 1); // 这里需要分析
    if (read_cnt != read_size) {
        AFS_TRACE_OUT_ERROR("Read data from image failed. ReadSize=%llu", (long long)read_size);
        free(buf);
        return AFS_ERR_IMAGE_READ;
    }

    ret = AFS_SUCCESS;
    // 幻数判断，检查是否是交换分区
    if (!strncmp(((struct swsusp_header *)buf)->sig, SWAP_MAGIC, strlen(SWAP_MAGIC))) {
        m_fs_type = AFS_FILESYSTEM_SWAP;
        AFS_TRACE_OUT_INFO("Current volume is SWAP.");
    } else {
        ret = getFSTypeUUID_1(buf, fs_uuid, fs_uuid_len);
    }

    free(buf);
    buf = NULL;
    *fs_type = m_fs_type;

    return ret;
}

/**
 * @brief 分析系统类型和返回文件系统的UUID
 *
 * @param *buf        超级块数据
 * @param *fs_uuid    不为空时(内存不小于AFS_FS_UUID_MAX_LEN+1)获取文件系统的UUID,
 *
 * @return int32_t 0 成功
 * 负数  失败
 *
 */
int32_t filesystemHandler::getFSTypeUUID_1(unsigned char *buf, char *fs_uuid, size_t fs_uuid_len)
{
    int32_t ret = 0;
    unsigned char *tmp_buffer = buf;
    char temp_uuid[AFS_FS_UUID_MAX_LEN + 1];
    CHECK_MEMSET_S_OK(temp_uuid, AFS_FS_UUID_MAX_LEN + 1, 0, AFS_FS_UUID_MAX_LEN + 1);

    ext4_super_block *ext4Sp = (struct ext4_super_block *)(tmp_buffer + 1024);

    if (((struct ext4_super_block *)(tmp_buffer + 1024))->s_magic == static_cast<le16>(EXT_MAGIC)) {
        // 检查是否是Ext3或者Ext4
        AFS_TRACE_OUT_INFO("s_magic is %02x", ((struct ext4_super_block *)(tmp_buffer + 1024))->s_magic);
        if (((struct ext4_super_block *)(tmp_buffer + 1024))->s_feature_compat ==
            static_cast<le32>(EXT3_4_FEATURE_COMPAT)) {
            m_fs_type = AFS_FILESYSTEM_EXT4;
            CHECK_MEMCPY_S_OK(temp_uuid, AFS_FS_UUID_MAX_LEN + 1,
                ((struct ext4_super_block *)(tmp_buffer + 1024))->s_uuid, AFS_FS_UUID_MAX_LEN);
            AFS_TRACE_OUT_INFO("Current file system is EXT4 or EXT3.");
        } else {
            ret = AFS_ERR_FS_VERSION;
            m_fs_type = AFS_FILESYSTEM_NULL;
            AFS_TRACE_OUT_ERROR("Current file system is EXT2.");
        }
    } else if (convert_bswap_32(((struct xfs_dsb *)tmp_buffer)->sb_magicnum) == static_cast<__be32>(XFS_MAGIC)) {
        m_fs_type = AFS_FILESYSTEM_XFS;
        CHECK_MEMCPY_S_OK(temp_uuid, AFS_FS_UUID_MAX_LEN + 1, ((struct xfs_dsb *)tmp_buffer)->sb_uuid.__u_bits,
            AFS_FS_UUID_MAX_LEN);
        AFS_TRACE_OUT_INFO("Current file system is XFS.");
    } else if (((ntfs_boot_sector *)(tmp_buffer))->oem_id == const_cpu_to_le64(static_cast<le64>(NTFS_MAGIC))) {
        m_fs_type = AFS_FILESYSTEM_NTFS;
        CHECK_VSNPRINTF_S_OK(temp_uuid, AFS_FS_UUID_MAX_LEN + 1, AFS_FS_UUID_MAX_LEN, "%016llX",
            ((ntfs_boot_sector *)(tmp_buffer))->volume_serial_number);
        AFS_TRACE_OUT_INFO("Current file system is NTFS, and its volume serial number is %s", temp_uuid);
    } else {
        AFS_TRACE_OUT_ERROR("ext4_super_block s_magic is %02x",
            ((struct ext4_super_block *)(tmp_buffer + 1024))->s_magic);
        AFS_TRACE_OUT_ERROR("XFS sb_magicnum is %02x", ((struct xfs_dsb *)tmp_buffer)->sb_magicnum);
        AFS_TRACE_OUT_ERROR("Unknown type.");
        m_fs_type = AFS_FILESYSTEM_NULL;
        ret = AFS_ERR_FS_SUPPORT;
    }

    if (m_fs_type != AFS_FILESYSTEM_NULL && fs_uuid != NULL) {
        CHECK_MEMCPY_S_OK(fs_uuid, AFS_FS_UUID_MAX_LEN + 1, temp_uuid, AFS_FS_UUID_MAX_LEN + 1);
    }

    return ret;
}

/**
 * @brief 设置空闲块Bitmap(由继承的相关文件系统函数实现)
 *
 * @param &bitmap  待设置的镜像Bitmap
 *
 * @return int32_t 0 成功
 * 负数  失败
 *
 */
int32_t filesystemHandler::getBitmap(vector<BitMap *> &bitmap_vect)
{
    AFS_TRACE_OUT_DBG("ENTER filesystemHandler::getBitmap()");
    if (NULL == m_reader) {
        return AFS_ERR_INNER;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 设置过滤文件的Bitmap(由继承的相关文件系统函数实现)
 *
 * @param *file_path   文件路径
 * @param &bitmap      待设置的镜像Bitmap
 *
 * @return int32_t 0：成功
 * 负数：失败
 *
 */
int32_t filesystemHandler::getFile(const char *file_path, vector<BitMap *> &bitmap_vect)
{
    AFS_TRACE_OUT_DBG("ENTER filesystemHandler::getFile()");
    if (m_fs_type == AFS_FILESYSTEM_NULL) {
        return AFS_ERR_INNER;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 4字节大小端转换
 *
 * @param x 大端表示值
 *
 * @return int32_t 0 成功
 * 负数  失败
 *
 */
uint32_t filesystemHandler::convert_bswap_32(uint32_t x)
{
    x = (((uint32_t)(x << 8)) & 0xFF00FF00) | (((uint32_t)(x >> 8)) & 0x00FF00FF);

    return ((uint32_t)(x << 16)) | ((uint32_t)(x >> 16));
}
