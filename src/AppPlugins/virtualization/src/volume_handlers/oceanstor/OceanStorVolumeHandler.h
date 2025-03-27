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
#ifndef OCEAN_STOR_VOLUME_HANDLER_H
#define OCEAN_STOR_VOLUME_HANDLER_H
#include "common/Macros.h"
#include "common/Constants.h"
#include "log/Log.h"
#include "volume_handlers/VolumeHandler.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/ControlDevice.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "ApiOperator.h"

namespace VirtPlugin {
class OceanStorVolumeHandler : public VolumeHandler {
public:
    OceanStorVolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId);
    virtual ~OceanStorVolumeHandler(){};
    int32_t InitializeVolumeInfo();
    int32_t GetDirtyRanges(const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot,
                           DirtyRanges &dirtyRanges, const uint64_t startOffset, uint64_t &endOffset);
    int32_t Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo);
    int32_t Open(const VolOpenMode &mode);
    int32_t ReadBlocks(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, std::shared_ptr<uint8_t[]> &buffer,
		std::shared_ptr<uint8_t[]> &calBuffer, std::shared_ptr<uint8_t[]> &readBuffer);
    int32_t WriteBlocks(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, std::shared_ptr<uint8_t[]> &buffer);
    int32_t Close();
    uint64_t GetVolumeSize()
    {
        return m_diskCapacityInBytes;
    }
    int32_t GetStorageInfoFromAppEnvAuth(const std::string &extendInfo, int32_t &erro);
    int32_t TestDeviceConnection(const std::string &authExtendInfo, int32_t &erroCode);
    int32_t CleanLeftovers()
    {
        return Module::SUCCESS;
    }
    int32_t Flush();

    std::shared_ptr<DiskDeviceFile> GetDiskDeviceFile() override
    {
        return m_spDeviceFile;
    }

    void SetOpService(bool isOpService)
    {
        m_isOpService = isOpService;
    }

    int32_t QueryStoragePoolUsedRate(double &usedCapacityRate) override;
private:
    int32_t GetStorageInfo(int32_t &erro);
    int32_t SetChangeInfo(const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot);
    int32_t ExecGetDirtyRanges(DirtyRanges& dirtyRanges, const uint64_t &startOffset, const uint64_t &endOffset);
    int32_t GetDirtyRangesFragmentForLun(const uint64_t &startOffset, const uint64_t &endOffset,
        DirtyRanges& dirtyRanges, std::string& errorDes);
    int32_t PrepareDirtyRangesFragmentForLun(DirtyRangesRequest& dirtyRangesRequest, std::string& errorDes);
    int32_t GetSnapshotCapacity(const NativeObjectInfo& snapshotObj, uint64_t& capacity, std::string& errDes);
    int32_t CheckExpansion(DirtyRangesRequest& dirtyRangesRequest);
    bool CheckDirtyRangesRequest(const DirtyRangesRequest& dirtyRangesRequest);
    int32_t GetDirtyRangesFragmentForLunImp(const DirtyRangesRequest& dirtyRangeRequest, DirtyRanges& dirtyRanges,
        std::string& errorDes);
    int32_t CalculateDirtyRanges(DirtyRangesCalculateInfo &calculateInfo, DirtyRanges &dirtyRanges,
        std::string &errorDes);
    int32_t ParseBitmap(std::string &bitmap, DirtyRanges &dirtyRanges, const uint64_t &startOffset,
        const uint64_t &size);
    bool ParseBitMapByByte(const unsigned char &ch, const uint64_t &chunkSize, const uint64_t &offSet,
        const uint64_t &totalSize, DirtyRanges &dirtyRanges);
    int32_t DoScanDisk(const std::string &objId, const std::string &objWwn, std::string &objPath);
    int32_t InitControlDeviceInfo(ControlDeviceInfo &deviceInfo, StorageInfo &storageInfo);
private:
    std::string m_devicePath;
    std::string m_diskWwn;
    std::string m_lunId;
    std::string m_snapshotId;
    NativeObjectInfo m_preChangeObj {};
    NativeObjectInfo m_curChangeObj {};
    bool m_isBackup = false;
    uint64_t m_chunkSize = 0;
    uint64_t m_diskCapacityInBytes = 0;
    std::shared_ptr<DiskDeviceFile> m_spDeviceFile = nullptr;
    std::shared_ptr<ApiOperator> m_spApiOperator = nullptr;
    VolumeDSExtendInfo m_dsExtend;
    AppProtect::BackupJobType m_backupType;
    bool m_isOpService {false};
};
}

#endif // OCEAN_STOR_VOLUME_HANDLER_H
