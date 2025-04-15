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
#include "afs/RawReader.h"
#include <stdio.h>
#include "afs/LVMReader.h"
#include "afs/LogMsg.h"

void rawReader::initImgReader(struct imgInfo *pImageInfo, int64_t offset, int64_t length)
{
    AFS_TRACE_OUT_DBG("rawReader class initialization");
    m_imageinfo.offset = offset;
    m_imageinfo.length = length;
    m_read_call_back_func = pImageInfo->read_call_back_func;
    m_handle = pImageInfo->Handle;
}

/**
 * @brief rawreader的读取镜像函数
 *
 * @param buf void* 传入的buf用于存放读取的内容
 * @param offset int64_t 偏移
 * @param count int64_t 字节数
 * @return int64_t 成功读取的字节数
 *
 */
int64_t rawReader::readDisk(void *buf, int64_t offset, int64_t count)
{
    imgReader *lowreader = getlowReader();
    int64_t newoffset = offset + m_imageinfo.offset;

    if (lowreader != NULL) {
        // 需要设置分区reader
        return lowreader->read(buf, newoffset, count, 1);
    }

    int64_t align_offset = newoffset / AFS_SECTOR_SEZE;
    int64_t remain_offset = newoffset % AFS_SECTOR_SEZE;
    int64_t real_size = count;

    if (0 != remain_offset) {
        // 多读一个扇区
        real_size += AFS_SECTOR_SEZE;
        AFS_TRACE_OUT_DBG("remain_offset != 0");
    }

    if (real_size <= 0) {
        AFS_TRACE_OUT_ERROR("Invalid read sectors size.");
        return 0;
    }

    // 多读一个扇区
    char *real_buf = new char[real_size]();
    if (NULL == real_buf) {
        AFS_TRACE_OUT_ERROR("Cann't allocate memory space for read data.");
        return 0; // 读数据失败
    }

    int64_t ret_size =
        m_read_call_back_func(m_handle, (char *)real_buf, align_offset * AFS_SECTOR_SEZE, (int32)real_size);
    if (real_size == ret_size) {
        // 读成功
        real_size = memcpy_s(buf, count, &real_buf[remain_offset], count);
        if (real_size != EOK) {
            AFS_TRACE_OUT_ERROR("call memcpy_s() failed, ret = %d", real_size);
            delete[] real_buf;
            return 0; // 复制数据失败
        }
    } else {
        AFS_TRACE_OUT_ERROR("Failed to read data by call back function.. result=%llu", (long long)ret_size);
        delete[] real_buf;
        return 0; // 读数据失败
    }

    delete[] real_buf;

    return count;
}

/**
 * @brief rawreader的读取镜像函数
 *
 * @param buf void* 传入的buf用于存放读取的内容
 * @param offset int64_t 偏移
 * @param count int64_t 字节数
 * @return int64_t 成功读取的字节数
 *
 */
int64_t rawReader::read(void *buf, int64_t offset, int64_t count, int32_t is_annotated)
{
    return readDisk(buf, offset, count);
}
