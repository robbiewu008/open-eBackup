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
#ifndef LIBNFS_COMMON_METHODS_H
#define LIBNFS_COMMON_METHODS_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <locale>
#include <codecvt>
#include <securec.h>
#include <atomic>
#include <cstdint>
#include <fcntl.h>
#include "Backup.h"
#include "BackupStructs.h"
#include "NfsContextContainer.h"
#include "LibNfsConstants.h"
#include "LibnfsStructs.h"
#include "FileHandleCache.h"
#include "PacketStats.h"

constexpr uint8_t OFFSET_1  = 1;
constexpr uint8_t OFFSET_2  = 2;
constexpr uint8_t OFFSET_10  = 10;
constexpr uint8_t NUMBER1  = 1;
constexpr uint8_t NUMBER2  = 2;

#define IS_LIBNFS_NEED_RETRY(status) ((status) == -EINTR || (status) == -EAGAIN || (status) == -EIO)
#define VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData) \
    if ((cbData) == nullptr) \
        return; \
    NfsCommonData *commonData = (cbData)->writeCommonData; \
    if (commonData == nullptr) { \
        ERRLOG("commonData is nullptr"); \
        delete (cbData); \
        (cbData) = nullptr; \
        return; \
    }

#define VALIDATE_COMMON_DATA_PTR_RECEIVED(cbData) \
    if ((cbData) == nullptr) \
        return; \
    NfsCommonData *commonData = (cbData)->commonData; \
    if (commonData == nullptr) { \
        ERRLOG("commonData is nullptr"); \
        delete (cbData); \
        (cbData) = nullptr; \
        return; \
    }

namespace Libnfscommonmethods {
struct HardLinkMapRemoveParams {
    std::shared_ptr<HardLinkMap> hardLinkMap;
    FileHandle fileHandle {};
};

struct NasServerCheckParams {
    time_t throttleTimer { 0 };
    time_t ratelimitTimer { 0 };
    bool suspend { false };
    std::shared_ptr<PacketStats> pktStats { nullptr };
    std::shared_ptr<BackupControlInfo> controlInfo { nullptr };
    BackupPhaseStatus failReason {};
    Module::NfsContextContainer *nfsContextContainer { nullptr };
    std::shared_ptr<LibnfsBackupAdvanceParams> advParams { nullptr };
    BackupType backupType { BackupType::UNKNOWN_TYPE };
    BackupPhase phase { BackupPhase::UNKNOWN_STAGE };
    std::string direction {};
};

void FreeNfsFh(struct nfsfh *&nfsFh);
int IsValidNfsFh(struct nfsfh* const nfsFh);

int NfsServerCheck(NasServerCheckParams &checkParams);
int NasServerCheck(std::shared_ptr<Module::NfsContextWrapper> nfsContext, uint32_t serverCheckSleepTime,
    uint32_t serverCheckRetry);
void GetRWSizeFromLibnfs(std::shared_ptr<Module::NfsContextWrapper> rootNfs, BackupParams &backupParams);
bool IsRetryMount(int status);
bool FillNfsContextContainer(std::string rootPath, uint16_t contextCount,
    Module::NfsContextContainer &nfsContextContainer, BackupParams &backupParams, uint32_t serverCheckSleepTime);

int LibNfsDeleteDirectorySync(std::string dirName, Module::NfsContextContainer *&nfsContextContainer);
int DeleteDirectory(DeleteInfo deleteInfo, std::shared_ptr<Module::NfsContextWrapper> nfs);
int DeleteFileDirectoryLibNfsRecursively(std::string &filePath, int &isDir,
    std::shared_ptr<Module::NfsContextWrapper> dstNfs);
int DeleteAllFilesInsideRecursively(std::string &filePath, std::shared_ptr<Module::NfsContextWrapper> dstNfs);
int ProcessDirParentFh(std::string dirPath, std::string parentDirPath, struct nfsfh* parentNfsfh, uint16_t retryCnt);
int ProcessParentFh(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh);
bool IsAbort(NfsCommonData &commonData);
void HandleParentDirCreationFailure(FileHandle &fileHandle, NfsCommonData &commonData);
int RemoveHardLinkMapEntryIfFileCreationFailed(HardLinkMapRemoveParams &hardLinkMapRemoveParams);
void FreeDirFh(FileHandle &fileHandle);
void FillFileHandleCacheWithInvalidDirectoryFh(std::string dirPath, std::shared_ptr<FileHandleCache> fileHandleCache);
int ConstructReadBlock(FileHandle &fileHandle, uint64_t blockSize);
std::string FloatToString(const float &val, const uint8_t &precisson = NUMBER1);
std::string FormatCapacity(uint64_t capacity);
void RequestFailHandleAndCleanLinkMap(FileHandle &fileHandle, NfsCommonData *commonData, std::string direction,
    bool failIncOrgCount = false, bool removeHardlinkMapEntry = true);
void CheckForCriticalError(NfsCommonData *commonData, int status, PKT_TYPE packetType);
void ExpireRetryTimers(BackupTimer &timer);
void RatelimitIncreaseMaxPendingRequestCount(std::string direction, time_t &ratelimitTimer,
    std::shared_ptr<LibnfsBackupAdvanceParams> advParams);
void RatelimitDecreaseMaxPendingRequestCount(std::string direction, time_t &ratelimitTimer,
    std::shared_ptr<LibnfsBackupAdvanceParams> advParams);
}

#endif // LIBNFS_COMMON_METHODS_H
