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
#pragma once
#ifndef HYPERV_VOLUME_HANDLER_H
#define HYPERV_VOLUME_HANDLER_H
#ifdef WIN32
#include <windows.h>
#include <virtdisk.h>
#include <fstream>
#include "common/Constants.h"
#include "common/MpString.h"
#include "common/utils/Utils.h"
#include "log/Log.h"
#include "volume_handlers/VolumeHandler.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "common/sha256/Sha256.h"
#include "common/utils/Win32Utils.h"
#include <repository_handlers/win32filesystem/Win32FileSystemHandler.h>

using namespace VirtPlugin;

namespace HyperVPlugin {

class HyperVVolumeHandler : public VolumeHandler {
public:
    HyperVVolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId)
        : VolumeHandler(jobHandle, volInfo, jobId, subJobId)
    { ifmarkBlockValidData = false; }
    virtual ~HyperVVolumeHandler(){};
    int32_t InitializeVolumeInfo();
    int32_t GetDirtyRanges(const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot,
                           DirtyRanges &dirtyRanges, const uint64_t startOffset, uint64_t &endOffset) override;
    int32_t Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo) override;
    int32_t Open(const VolOpenMode &mode) override;
    int32_t ReadBlocks(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, std::shared_ptr<uint8_t[]> &buffer,
        std::shared_ptr<uint8_t[]> &calBuffer, std::shared_ptr<uint8_t[]> &readBuffer) override;
    int32_t Close() override;
    int32_t WriteBlocks(const uint64_t &offsetInBytes, uint64_t &bufSizeInBytes,
        std::shared_ptr<uint8_t[]> &buf) override;
    uint64_t GetVolumeSize() override;
    int32_t TestDeviceConnection(const std::string &authExtendInfo, int32_t &erroCode) override;
    int32_t CleanLeftovers() override;
    int32_t Flush() override;
    std::shared_ptr<DiskDeviceFile> GetDiskDeviceFile() override
    {
        return m_spDeviceFile;
    }

private:
    int32_t GetDirtyRangesFromVirtDisk(const std::string &rctIdStr, uint64_t startOffset,
        uint64_t endOffset, DirtyRanges &dirtyRanges);
    bool AddDirtyRanges(DirtyRanges &dirtyRanges, ULONG rangeCount);
    bool OpenDisk(const std::string &diskPath, HANDLE &volHandler);
    bool AttachDisk(HANDLE &volHandler, ATTACH_VIRTUAL_DISK_FLAG  attachFlags);

private:
    std::shared_ptr<DiskDeviceFile> m_spDeviceFile = nullptr;
    HANDLE m_readVhdHandle = INVALID_HANDLE_VALUE;
    HANDLE m_writeVhdHandle = INVALID_HANDLE_VALUE;
    std::unique_ptr<QUERY_CHANGES_VIRTUAL_DISK_RANGE[]> m_ranges = nullptr;
    std::fstream m_RestoreTargetFileDescriptor;
    bool m_IfRestore = false;
    std::mutex m_repoMutex;
};
}
#endif
#endif
