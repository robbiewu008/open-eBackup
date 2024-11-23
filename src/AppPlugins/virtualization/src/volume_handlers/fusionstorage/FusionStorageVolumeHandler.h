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
#ifndef FUSIONSTORAGE_VOLUME_HANDLER_H
#define FUSIONSTORAGE_VOLUME_HANDLER_H
#include <boost/function.hpp>
#include <memory>
#include "FusionStorageApi.h"
#include "FusionStorageRestApiOperator.h"
#include "FusionStorageBitmapHandle.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/VolumeHandler.h"
#include "DiskDataPersistence.h"
#include "common/DirtyRanges.h"

namespace VirtPlugin {
class FusionStorageVolumeHandler : public VolumeHandler {
public:
    FusionStorageVolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId);

    virtual ~FusionStorageVolumeHandler();

    int32_t InitializeVolumeInfo();  // 初始化卷信息

    int32_t GetDirtyRanges(const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot,
        DirtyRanges &dirtyRanges, const uint64_t startOffset, uint64_t &endOffset);

    int32_t Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo);  // for backup

    int32_t Open(const VolOpenMode &mode);  // for restore

    int32_t Close();

    int32_t ReadBlocks(const uint64_t &offsetInBytes, uint64_t &bufSizeInBytes, std::shared_ptr<uint8_t[]> &buf,
        std::shared_ptr<uint8_t[]> &calBuffer, std::shared_ptr<uint8_t[]> &readBuffer);

    int32_t WriteBlocks(const uint64_t &offsetInBytes, uint64_t &bufSizeInBytes, std::shared_ptr<uint8_t[]> &buf);

    uint64_t GetVolumeSize();
    int32_t TestDeviceConnection(const std::string &authExtendInfo, int32_t &erroCode);
    bool CheckHealthStatus(std::string &errDesc, int32_t &erroCode);
    void DetachVolume(Json::Value &itemJson, const std::string &volumeName, std::string &attachedVolumes,
        int32_t &result);

    int32_t CleanLeftovers();
    int32_t RegKeyForVolume(const std::string &diskDevicePath);
    int32_t UnRegKeyForVolume(const std::string &diskDevicePath);
    void ReleaseBufferCache(const uint64_t &offsetInBytes, const uint64_t &readLen);
    int32_t Flush();
    std::shared_ptr<JobHandle> GetJobHandleInTask();
    int32_t QueryStoragePoolUsedRate(double &usedCapacityRate) override;
    std::shared_ptr<DiskDeviceFile> GetDiskDeviceFile() override
    {
        return m_spDeviceFile;
    }

    std::shared_ptr<FusionStorageBitmapHandle> m_spBitmapHandle;

private:
    FusionStorageVolumeHandler(const FusionStorageVolumeHandler &src);
    FusionStorageVolumeHandler &operator=(const FusionStorageVolumeHandler &);

    int32_t InitComponents();

    int32_t RetryOp(boost::function<int()> fun, const std::string &opName);

    int32_t ParseBackupParams();
    int32_t ParseRestoreParams();

    int32_t CleanLeftoversForBackup(std::vector<std::string> &itemVecStr);
    int32_t CleanLeftoversForRestore(std::vector<std::string> &itemVecStr);

    bool CheckAgentRunning();

    void DeleteBitmapVolume(
        Json::Value &itemJson, std::string &createdBitmapVolume, std::string &createVolumes, int32_t &result);

    void StrSplit(std::vector<std::string> &vecTokens, const std::string &strText, char cSep);
    int32_t CheckMountPointIsExpire(const std::string &strItem);
    int32_t AddBitmapInfoToSnapShot(const std::string &bitmapName, const std::string &snapshotId);
    void ReportDeleteBitmap(const std::string &bitmapName);
    bool QuerySnapshot(const std::string &snapshotName);
    void SetDirtyRangesParams(const uint64_t &diskSize, const uint64_t &startOffset,
        DirtyRangesParams &params);

private:
    std::string m_fusionStorMgrIp;
    std::string m_poolID;
    std::string m_changeID;  // backup, 当前快照ID
    std::string m_preChangeID;
    std::string m_volumeID;  // restore
    uint64_t m_volSizeInBytes;  // 卷大小
    std::shared_ptr<FusionStorageApi> m_spDSWareApi;
    std::shared_ptr<FusionStorageRestApiOperator> m_spDSWareRestApi;
    std::shared_ptr<DiskDeviceFile> m_spDeviceFile;
    std::shared_ptr<DiskDataPersistence> m_dataPersistence;
    std::shared_ptr<FusionStorageCleanFile> m_cleanFile;

    AppProtect::BackupJobType m_backupType;
    std::string m_apiMode;
    bool m_isBackup;
};
}  // namespace VirtPlugin

#endif  // FUSIONSTORAGE_VOLUME_HANDLER_H
