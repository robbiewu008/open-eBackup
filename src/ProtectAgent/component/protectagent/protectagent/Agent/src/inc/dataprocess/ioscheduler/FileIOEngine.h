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
#ifndef __FILE_IO_ENGINE_H__
#define __FILE_IO_ENGINE_H__

#include <cstdint>
#include <functional>
#include "IOEngine.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "common/Defines.h"

class FileIOEngine : public IOEngine {
public:
    FileIOEngine(const vmware_volume_info& vol, mp_int32 protectType, mp_int32 snapType)
        : m_volInfo(vol), m_protectType(protectType), m_snapType(snapType), m_fp(NULL)
    {}
    ~FileIOEngine()
    {}
    // 任务开始时打开存储上对应的文件
    mp_int32 Open() override;
    // 任务结束后关闭存储上对应的文件
    mp_int32 Close() override;
    mp_int32 Read(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer) override;
    mp_int32 Write(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer) override;
    // 备份任务完成后生成磁盘描述文件
    mp_int32 PostBackup() override;
    mp_string GetFileName() override;
    mp_string GetFileNameForWrite() override;
private:
    mp_int32 OpenForRead();
    mp_int32 OpenForNFSRead();
    EXTER_ATTACK mp_int32 OpenForWrite();
    mp_int32 CreateDiskFile(const mp_string &diskFile, mp_int32 deeGroupId);
    mp_int32 DoRead(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer);
    mp_int32 DoWrite(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer);
    mp_int32 RetryOp(std::function<mp_int32()> internalOp);
    mp_void GenerateDescFileContent(mp_string& strContent);
    EXTER_ATTACK mp_int32 GenerateDiskDescFile();

private:
    const vmware_volume_info& m_volInfo;  // vmware_volume_info中vecDirtyRange数据量大，需以引用方式使用
    mp_int32 m_protectType;
    mp_int32 m_snapType;
    FILE* m_fp;
    mp_string m_ifIgnoreBadBlock;
};

#endif