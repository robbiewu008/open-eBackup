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
#include "afs/ImgReader.h"
#include "afs/LogMsg.h"
#include "afs/PartitionHandler.h"
#include "afs/AfsError.h"
#include "afs/Afslibmain.h"
#include "afs/FSCommon.h"

/**
* @brief 把imageinfo的内容拷贝到传入的空间中
*
* @param *imginfo  镜像信息的结构体指针
*
* @return AFS_SUCCESS 成功
*
*/
int32_t imgReader::GetReaderInfo(struct imageInfo *imginfo)
{
    CHECK_MEMCPY_S_OK(imginfo, sizeof(struct imageInfo), &(this->m_imageinfo), sizeof(this->m_imageinfo));
    return AFS_SUCCESS;
}

/**
 * @brief 初始化image reader
 *
 * @param *reader    reader指针
 * @param *ppart     分区信息
 * @param chunk_size LVM管理时使用，代表最小物理连续单元大小(扇区)
 *
 * @return int 0设置成功 -1设置失败
 *
 */
int32_t imgReader::initImgReader(afsObject *reader, struct partition *ppart, uint32_t chunk_size)
{
    struct imageInfo imginfo;
    CHECK_MEMSET_S_OK(&imginfo, sizeof(imginfo), 0, sizeof(imginfo));

    // 根据分区格式创建分区reader
    imgReader *imgreader = static_cast<imgReader *>(reader);

    int32_t ret = imgreader->GetReaderInfo(&imginfo);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("image reader get info failed, ret = %d", ret);
        return ret;
    }

    // 有效偏移检查
    if ((ppart->offset + ppart->length) > (unsigned long long)imginfo.length) {
        AFS_TRACE_OUT_ERROR("Offset over image.");
        return AFS_ERR_PARTITION;
    }

    this->m_lowreader = imgreader;
    this->m_imageinfo.formattype = imginfo.formattype;
    this->m_imageinfo.length = (int64_t)ppart->length;
    this->m_imageinfo.offset = (int64_t)ppart->offset;
    this->m_imageinfo.chunkSize = chunk_size;

    AFS_TRACE_OUT_INFO("m_imageinfo.length =%lld, m_imageinfo.offset=%lld, m_imageinfo.chunkSize=%d",
        m_imageinfo.length, m_imageinfo.offset, chunk_size);

    return AFS_SUCCESS;
}

/**
 * @brief 获取lowreader
 * @return
 * NULL
 * imgReader *
 */
imgReader *imgReader::getlowReader()
{
    if ((m_lowreader != NULL) && (m_lowreader->getObjType() == OBJ_TYPE_IMGREADER)) {
        return static_cast<imgReader *>(m_lowreader);
    } else {
        return NULL;
    }
}

/**
 * @brief 从镜像中读取指定偏移和长度的数据到buffer中
 *
 * @param *buf   读取到的数据buffer
 * @param offset 偏移
 * @param count  长度
 * @param is_annotated  是否开启调试信息
 * @return int 大于等于:读取成功    负数：读取失败
 *
 */
int64_t imgReader::read(void *buf, int64_t offset, int64_t count, int32_t is_annotated)
{
    int64_t newoffset = 0;
    imgReader *lowreader_tmp = getlowReader();

    if (lowreader_tmp != NULL) {
        newoffset = offset + m_imageinfo.offset * SECTOR_SIZE;
        return lowreader_tmp->read(buf, newoffset, count, 0);
    }

    return 0;
}

/**
 * @brief 从lvmreader获取该imgreader对象所属LV的stripe size
 *
 * @return -1/0  表示非跨磁盘的LV
 *
 */
uint32_t imgReader::getStripeSize()
{
    imgReader *lowreader_tmp = getlowReader();

    if (lowreader_tmp != NULL) {
        AFS_TRACE_OUT_DBG("imgReader::getStripeSize()");
        return lowreader_tmp->getStripeSize();
    }

    return 0;
}

/**
 * @brief 获取最大连续可写存储区的大小（针对跨磁盘的stripe LV和 Linear lV）
 *
 * @return  连续可写存储区的大小 单位扇区
 *
 */
uint32_t imgReader::getMaxStorageZone()
{
    uint32_t storage_size = 0;

    storage_size = getStripeSize();
    if (storage_size != 0) {
        return storage_size;
    }

    return getChunkSize();
}

/**
 * @brief 设置image的偏移
 *
 * @param offset   偏移位置
 * @param length   长度
 * @return AFS_SUCCESS
 *
 */
int32_t imgReader::setImgOffset(int64_t offset, int64_t length)
{
    struct imageInfo imginfo;

    int32_t ret = GetReaderInfo(&imginfo);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("image reader set offset and length of disk failed, ret = %d", ret);
        return ret;
    }

    this->m_imageinfo.offset = offset;
    this->m_imageinfo.formattype = imginfo.formattype;
    this->m_imageinfo.length = length;

    return AFS_SUCCESS;
}

/**
 * @brief 读取扇区数据
 * @param *buf     缓存
 * @param sect_off 数据起始位置偏移
 * @param count    读取长度(扇区)
 * @return
 * 实际读取的扇区数
 */
int64_t imgReader::readSector(void *buf, int64_t sect_off, int64_t count)
{
    int64_t ret_size = read(buf, sect_off * SECTOR_SIZE, count * SECTOR_SIZE, 1);
    return ret_size / SECTOR_SIZE;
}

/**
 * @brief 虚拟地址到物理地址转换
 * @param vaddr 虚地址
 * @return 返回物理地址(单位:扇区)
 */
int64_t imgReader::getVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
{
    if (NULL == m_lowreader) {
        disk_id = getImgInfoDiskID();
        AFS_TRACE_OUT_ERROR("Faile to get physical address .");
        return AFS_ERR_PARTITION;
    }

    // 如果非lvm，则逻辑地址就是物理地址，中间没有转换过程
    return ((imgReader *)m_lowreader)->getVaddrToPaddr(vaddr + m_imageinfo.offset * SECTOR_SIZE, disk_id);
}
