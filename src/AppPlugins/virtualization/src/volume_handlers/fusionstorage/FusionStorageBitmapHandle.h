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
#ifndef FUSIONSTORAGE_BITMAP_HANDLE_H
#define FUSIONSTORAGE_BITMAP_HANDLE_H

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <memory>
#include "common/Structs.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "common/DirtyRanges.h"
#include "DiskDataPersistence.h"
#include "volume_handlers/oceanstor/DiskScannerHandler.h"
#include "FusionStorageApi.h"
#include "FusionStorageRestApiOperator.h"

namespace VirtPlugin {
struct DirtyRangesParams {
    std::string volumeID;
    std::string parentVolumeID;
    uint64_t offset;
    uint64_t diskSize;
    DirtyRangesParams() : offset(0), diskSize(0)
    {}

    void SetDiskSize(uint64_t disksize)
    {
        diskSize = disksize;
    }

    void SetOffset(uint64_t offsetnum)
    {
        offset = offsetnum;
    }

    void SetParentVolumeID(std::string parentvolumeid)
    {
        parentVolumeID = parentvolumeid;
    }

    void SetVolumeID(std::string volumeid)
    {
        volumeID = volumeid;
    }
};

class FusionStorageBitmapHandle {
public:
    FusionStorageBitmapHandle(const std::string &fusionStorMgrIp, const std::string &poolID,
        const bool isBackup, const std::string &volumeID, std::shared_ptr<DiskDataPersistence> diskDataPersistence,
        std::shared_ptr<RepositoryHandler> cacheRepoHandler, const std::string cacheRepoPath);

    ~FusionStorageBitmapHandle();

    int32_t GetDirtyRanges(const DirtyRangesParams &params, DirtyRanges &dirtyRanges, std::string &errMsg,
        BitmapVolumeInfo &bitmapVolumeInfo);

    void InitializeFusionStorageApi(std::string &apiMode,
         std::shared_ptr<FusionStorageRestApiOperator> fusionStorageRestApiOperator,
         std::shared_ptr<FusionStorageApi> fusionStorageRestApi);
    int32_t DeleteAndDetachapVolume(const std::string &volumeName, std::string &errMsg);
    void SetBitmapName(const std::string &snapshotId, const VolSnapInfo &snapshotInfo);

private:
    FusionStorageBitmapHandle(const FusionStorageBitmapHandle &);

    const FusionStorageBitmapHandle &operator=(const FusionStorageBitmapHandle &);

    int32_t CreateAndAttachBitmapVolume(BitmapVolumeInfo &bitmapVolumeInfo, std::string &errMsg);

    int32_t RetryOp(boost::function<int()> fun, const std::string &opName);

    int32_t CalculateDirtyRange(const uint64_t startOffset, DirtyRanges &dirtyRanges);

    std::string GetBitmapVolName();

    bool AddDitryRange(uint64_t endAddr, uint64_t curAddr, unsigned int c, DirtyRanges &dirtyRanges);

    int32_t CreateBitmapVolume(BitmapVolumeInfo &bitmapVolumeInfo, std::string &errMsg);

    int32_t RetryToQueryBitmap(BitmapVolumeInfo &info);

private:
    std::string m_fusionStorMgrIp;
    uint64_t m_trunkSize;
    uint64_t m_diskSize;
    uint64_t m_bitmapVolumeSizeInBytes;
    std::string m_snapshotUUID;
    std::shared_ptr<DiskDeviceFile> m_spDeviceFile;
    std::shared_ptr<FusionStorageApi> m_spDSWareApi = nullptr;
    std::shared_ptr<FusionStorageRestApiOperator> m_spDSWareRestApi = nullptr;
    int32_t m_diskNum;
    std::string m_bitmapVolumeDiskPath;
    bool m_isBackup;
    std::string m_strVolumeID;
    std::shared_ptr<DiskDataPersistence> m_diskDataPersistence;
    std::string m_apiMode;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::string m_cacheRepoPath;
    std::string m_bitmapName;
};
}  // namespace VirtPlugin

#endif  // FUSIONSTORAGE_BITMAP_HANDLE_H