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
#include "LibnfsCommonMethods.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
    const uint16_t DEFAULT_RETRY_MAX_CNT = 3;
}

void Libnfscommonmethods::FreeNfsFh(struct nfsfh *&nfsFh)
{
    if (nfsFh != nullptr) {
        if (nfsFh->fh.val != nullptr) {
            free(nfsFh->fh.val);
            nfsFh->fh.len = 0;
            nfsFh->fh.val = nullptr;
        }
        if (nfsFh->pagecache.entries != nullptr) {
            free(nfsFh->pagecache.entries);
            nfsFh->pagecache.entries = nullptr;
        }
        free(nfsFh);
        nfsFh = nullptr;
    }
    return;
}

/* Check fh.len is 0. This denotes directory creation failed. Returns BACKUP_RET_FAILED in that case. */
int Libnfscommonmethods::IsValidNfsFh(struct nfsfh* const nfsFh)
{
    if (nfsFh->fh.len == 0) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int Libnfscommonmethods::NfsServerCheck(NasServerCheckParams &checkParams)
{
    if (checkParams.phase != BackupPhase::DELETE_STAGE && checkParams.phase != BackupPhase::DIR_STAGE) {
        /* If max no-space/no-access pending count is reached, abort the job */
        if (checkParams.pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_SPACE_ERR) >= DEFAULT_MAX_NOSPACE ||
            checkParams.pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_ACCESS_ERR) >= DEFAULT_MAX_NOACCESS) {
            if (checkParams.pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_SPACE_ERR) >= DEFAULT_MAX_NOSPACE) {
                ERRLOG("Threshold reached for DEFAULT_MAX_NOSPACE in %s", checkParams.direction.c_str());
                checkParams.failReason = BackupPhaseStatus::FAILED_NOSPACE;
            } else {
                ERRLOG("Threshold reached for DEFAULT_MAX_NOACCESS in %s", checkParams.direction.c_str());
                checkParams.failReason = BackupPhaseStatus::FAILED_NOACCESS;
            }
            checkParams.controlInfo->m_failed = true;
            return MP_FAILED;
        }
    }

    /* Nas Server Check for Source side */
    if (checkParams.pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR)
        >= checkParams.advParams->serverCheckMaxCount) {
        ERRLOG("Threshold reached calling %s servercheck", checkParams.direction.c_str());
        checkParams.suspend = true;
        shared_ptr<NfsContextWrapper> nfs = checkParams.nfsContextContainer->GetCurrContext();
        if (nfs == nullptr) {
            ERRLOG("nfs wrapper is null. NfsServerCheck failed in %s", checkParams.direction.c_str());
            FSBackupUtils::SetServerNotReachableErrorCode(checkParams.backupType, checkParams.failReason);
            checkParams.controlInfo->m_failed = true;
            checkParams.suspend = false;
            return MP_FAILED;
        }
        if (NasServerCheck(nfs, checkParams.advParams->serverCheckSleepTime,
            checkParams.advParams->serverCheckRetry) == MP_FAILED) {
            ERRLOG("Stop and Abort %s phase due to server inaccessible", checkParams.direction.c_str());
            FSBackupUtils::SetServerNotReachableErrorCode(checkParams.backupType, checkParams.failReason);
            checkParams.controlInfo->m_failed = true;
            checkParams.suspend = false;
            return MP_FAILED;
        } else {
            INFOLOG("Server reachable %s", checkParams.direction.c_str());
            checkParams.pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
        }
        RatelimitDecreaseMaxPendingRequestCount(checkParams.direction, checkParams.ratelimitTimer,
            checkParams.advParams);
        checkParams.suspend = false;
    }

    return MP_SUCCESS;
}

int Libnfscommonmethods::NasServerCheck(std::shared_ptr<Module::NfsContextWrapper> nfsContext,
    uint32_t serverCheckSleepTime, uint32_t serverCheckRetry)
{
    uint16_t retryCnt = 0;
    int ret = MP_FAILED;
    uint32_t retryMulti = 0 ;
    uint32_t serverRetryInterval = 0;

    std::this_thread::sleep_for(chrono::milliseconds(serverCheckSleepTime));
    do {
        INFOLOG("ServerPath: %s", nfsContext->m_url.c_str());
        ++retryMulti;
        ret = nfsContext->NfsNullSyncLock();
        if (ret != MP_SUCCESS) {
            if (retryMulti <= NUMBER_THREE) {
                serverRetryInterval = serverCheckSleepTime * retryMulti;
            }
            std::this_thread::sleep_for(chrono::milliseconds(serverRetryInterval));
        }
        retryCnt++;
    } while ((ret != MP_SUCCESS) && (retryCnt <= serverCheckRetry));

    if (ret == MP_SUCCESS) {
        INFOLOG("Server check succeeded for: %s", nfsContext->m_url.c_str());
        return MP_SUCCESS;
    } else {
        ERRLOG("Server check failed for: %s with error %d", nfsContext->m_url.c_str(), ret);
        return MP_FAILED;
    }
}

void Libnfscommonmethods::GetRWSizeFromLibnfs(shared_ptr<NfsContextWrapper> rootNfs, BackupParams &backupParams)
{
    uint64_t readMaxSize = rootNfs->GetNfsReadMaxSizeLock();
    uint64_t writeMaxSize = rootNfs->GetNfsWriteMaxSizeLock();

    if (readMaxSize < backupParams.commonParams.blockSize) {
        backupParams.commonParams.blockSize = readMaxSize;
    }
    if (writeMaxSize < backupParams.commonParams.blockSize) {
        backupParams.commonParams.blockSize = writeMaxSize;
    }
}

bool Libnfscommonmethods::IsRetryMount(int status)
{
    if (IS_LIBNFS_NEED_RETRY(status) || status == -EPERM || status == -EFAULT || status == -EACCES) {
        if (status == -EPERM || status == -EFAULT || status == -EACCES) {
            WARNLOG("Nfs Mount failed with EPERM/EFAULT/EACCES Error. Status: %d", status);
        }
        return true;
    }
    return false;
}

bool Libnfscommonmethods::FillNfsContextContainer(string rootPath, uint16_t contextCount,
    NfsContextContainer &nfsContextContainer, BackupParams &backupParams, uint32_t serverCheckSleepTime)
{
    string nfsMntArgs = "auto-traverse-mounts=0";
    uint32_t retryMulti = 0 ;
    uint32_t serverRetryInterval = 0;
    std::shared_ptr<LibnfsBackupAdvanceParams> advParams =
        dynamic_pointer_cast<LibnfsBackupAdvanceParams>(backupParams.srcAdvParams);
    uint16_t maxRetryCnt = advParams == nullptr ? DEFAULT_RETRY_MAX_CNT : advParams->serverCheckRetry;

    for (uint16_t i = 0; i < contextCount; i++) {
        shared_ptr<NfsContextWrapper> rootNfs = make_shared<NfsContextWrapper>(rootPath, nfsMntArgs);
        uint16_t retryCnt = 0;
        int ret = MP_FAILED;
        do {
            ret = rootNfs->NfsMount(true);
            retryCnt++;
            retryMulti++;
            // this if logic keeping outside just to over come the Function nesting depth is 5
            if (retryMulti <= NUMBER_THREE) {
                serverRetryInterval = serverCheckSleepTime * retryMulti;
            }
            if (IsRetryMount(ret)) {
                WARNLOG("Retry mount for url: %s, with error: %d, retryCnt: %d, sleepTime: %d",
                    rootPath.c_str(), ret, retryCnt, serverRetryInterval);
                std::this_thread::sleep_for(chrono::milliseconds(serverRetryInterval));
            } else if (ret != MP_SUCCESS) {
                ERRLOG("Mount for url: %s failed with non-retriable error : %d", rootPath.c_str(), ret);
                break;
            }
        } while ((ret != MP_SUCCESS) && (retryCnt <= maxRetryCnt));

        if (ret != MP_SUCCESS) {
            ERRLOG("Failed to mount url: %s, error: %d", rootPath.c_str(), ret);
            if (IS_LIBNFS_NEED_RETRY(ret)) {
                continue;
            } else {
                // For non retriable errors, no need to try mount again for other contexts
                break;
            }
        }
        GetRWSizeFromLibnfs(rootNfs, backupParams);

        nfsContextContainer.Insert(i, rootNfs);
    }

    if (nfsContextContainer.Size() == 0) {
        ERRLOG("Create Nfs Context container failed, url:  %s", rootPath.c_str());
        return false;
    }
    if (nfsContextContainer.Size() < contextCount) {
        ERRLOG("URL: %s, CreatedNfsContext: %d, RequestedNfsContext: %d", rootPath.c_str(),
            nfsContextContainer.Size(), contextCount);
    }

    return true;
}

/* !!!! CAUTION: This involves sync calls which uses a lock. NEVER use from async callbacks, as it will lead to hang */
int Libnfscommonmethods::LibNfsDeleteDirectorySync(string dirName, NfsContextContainer *&nfsContextContainer)
{
    DeleteInfo deleteInfo {};
    deleteInfo.m_fileName = dirName;
    deleteInfo.m_isDir = 1;

    shared_ptr<NfsContextWrapper> nfs = nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send dirdelete req failed for: %s", dirName.c_str());
        return MP_FAILED;
    }
    return DeleteDirectory(deleteInfo, nfs);
}

int Libnfscommonmethods::DeleteDirectory(DeleteInfo deleteInfo, shared_ptr<NfsContextWrapper> nfs)
{
    if (!deleteInfo.m_fileName.empty()) {
        int delStatIsDir = NUMBER_ZERO;
        struct nfs_stat_64 st;
        /* todo: need write libnfs API which uses parentfh */
        if (nfs->NfsLstat64Lock(deleteInfo.m_fileName.c_str(), &st) == 0) {
            switch (st.nfs_mode & S_IFMT) {
                case S_IFDIR:
                    delStatIsDir = 1;
                    break;
                default:
                    delStatIsDir = 0;
                    break;
            }
            return DeleteFileDirectoryLibNfsRecursively(deleteInfo.m_fileName, delStatIsDir, nfs);
        }
    }
    return MP_SUCCESS;
}

int Libnfscommonmethods::DeleteFileDirectoryLibNfsRecursively(string &filePath, int &isDir,
    shared_ptr<NfsContextWrapper> dstNfs)
{
    int ret = MP_SUCCESS;
    if (isDir != 0) {
        return DeleteAllFilesInsideRecursively(filePath, dstNfs);
    } else {
        /* todo: need write libnfs API which uses parentfh */
        ret = dstNfs->NfsUnlinkLock(filePath.c_str());
        if (ret == MP_SUCCESS) {
            DBGLOG("Delete file success: %s", filePath.c_str());
            return MP_SUCCESS;
        } else if (ret == -BACKUP_ERR_ENOENT) {
            ERRLOG("File: %s  already deleted", filePath.c_str());
            return MP_SUCCESS;
        } else {
            ERRLOG("Delete file failed: %s Status: %d ERR: %s", filePath.c_str(), ret,
                nfs_get_error(dstNfs->GetNfsContext()));
            return MP_FAILED;
        }
    }
}

int Libnfscommonmethods::DeleteAllFilesInsideRecursively(string &filePath,
    shared_ptr<NfsContextWrapper> dstNfs)
{
    int ret = MP_SUCCESS;
    int retDirectoryDelete = MP_SUCCESS;
    int fileDeleteFailed = 0;
    struct nfsdir *nfsdir = nullptr;
    struct nfsdirent *nfsdirent = nullptr;
    /* todo: need write libnfs API which uses parentfh */
    ret = dstNfs->NfsOpendirLock(filePath.c_str(), &nfsdir);
    if (ret != MP_SUCCESS) {
        ERRLOG("nfs_opendir failed for: %s Status: %d ERR: %s", filePath.c_str(), ret,
            nfs_get_error(dstNfs->GetNfsContext()));
        return MP_FAILED;
    }
    while ((nfsdirent = dstNfs->NfsReadDirLock(nfsdir)) != nullptr) {
        if (!strcmp(nfsdirent->name, ".") || !strcmp(nfsdirent->name, ".."))
            continue;
        string fullPath(filePath);
        fullPath.append("/");
        fullPath.append(nfsdirent->name);
        if (S_ISDIR(nfsdirent->mode)) {
            DeleteInfo deleteInfo {};
            deleteInfo.m_fileName = fullPath;
            deleteInfo.m_isDir = 1;
            retDirectoryDelete = DeleteDirectory(deleteInfo, dstNfs);
        } else {
            /* todo: need write libnfs API which uses parentfh */
            ret = dstNfs->NfsUnlinkLock(fullPath.c_str());
            if (ret == MP_SUCCESS) {
                DBGLOG("Delete subfile succ: %s", fullPath.c_str());
            } else if (ret == -BACKUP_ERR_ENOENT) {
                ERRLOG("Sub file  %s already deleted", filePath.c_str());
            } else {
                ERRLOG("Failed to delete sub file: %s Status: %d ERR: %s", filePath.c_str(), ret,
                    nfs_get_error(dstNfs->GetNfsContext()));
                fileDeleteFailed++;
            }
        }
    }
    dstNfs->NfsCloseDirLock(nfsdir);
    // if no files/dir left in this directory, then remove it
    if (fileDeleteFailed == 0) {
        /* todo: need write libnfs API which uses parentfh */
        ret = dstNfs->NfsRmdirLock(filePath.c_str());
        if (ret != MP_SUCCESS) {
            ERRLOG("Failed to delete directory : %s Status: %d ERR: %s", filePath.c_str(), ret,
                nfs_get_error(dstNfs->GetNfsContext()));
            return MP_FAILED;
        }
        DBGLOG("Directory Delete Success: %s", filePath.c_str());
    }
    return retDirectoryDelete;
}

int Libnfscommonmethods::ProcessDirParentFh(string dirPath, string parentDirPath, struct nfsfh* parentNfsfh,
    uint16_t retryCnt)
{
    if (parentDirPath != "." && parentNfsfh != nullptr) {
        if (IsValidNfsFh(parentNfsfh) != MP_SUCCESS) {
            ERRLOG("Parent directory creation failed for file: %s. Parent directory: %s, retry: %d", dirPath.c_str(),
                parentDirPath.c_str(), retryCnt);

            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

int Libnfscommonmethods::ProcessParentFh(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh)
{
    if (fileHandle.m_file->m_dirName != "." && !fileHandle.m_file->IsFlagSet(IS_DIR)) {
        // if file is in root path "." we will not get the fh of "."
        if (nfsfh == nullptr && commonData.hardlinkMap == nullptr) {
            commonData.writeWaitQueue->Push(fileHandle);
            return MP_FAILED;
        }

        if (nfsfh != nullptr) {
            if (IsValidNfsFh(nfsfh) != MP_SUCCESS) {
                HandleParentDirCreationFailure(fileHandle, commonData);
                return MP_FAILED;
            }
        }
    }

    return MP_SUCCESS;
}

bool Libnfscommonmethods::IsAbort(NfsCommonData &commonData)
{
    if (*(commonData.abort) || commonData.controlInfo->m_failed || commonData.controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            *(commonData.abort), commonData.controlInfo->m_failed.load(),
            commonData.controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

void Libnfscommonmethods::HandleParentDirCreationFailure(FileHandle &fileHandle, NfsCommonData &commonData)
{
    if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
        return;
    }
    ERRLOG("ParentDir creation failed for file: %s. Parent directory: %s", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_file->m_dirName.c_str());

    if (IS_INCREMENT_FAIL_COUNT(fileHandle)) {
        FSBackupUtils::RecordFailureDetail(commonData.failureRecorder, fileHandle.m_file->m_fileName, ENOENT);
        commonData.controlInfo->m_noOfFilesFailed += fileHandle.m_file->m_originalFileCount;
        ERRLOG("Write file failed: %s, totalFailed: %llu", fileHandle.m_file->m_fileName.c_str(),
            commonData.controlInfo->m_noOfFilesFailed.load());
    }

    if (!commonData.skipFailure) {
        commonData.controlInfo->m_failed = true;
    }

    if (fileHandle.m_file->GetSrcState() == FileDescState::LINK) {
        return;
    }

    HardLinkMapRemoveParams hardLinkMapRemoveParams {};
    hardLinkMapRemoveParams.hardLinkMap = commonData.hardlinkMap;
    hardLinkMapRemoveParams.fileHandle = fileHandle;
    RemoveHardLinkMapEntryIfFileCreationFailed(hardLinkMapRemoveParams);

    fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
    // This is to free the blockBufferMap value
    commonData.writeWaitQueue->Push(fileHandle);
    return;
}

int Libnfscommonmethods::RemoveHardLinkMapEntryIfFileCreationFailed(HardLinkMapRemoveParams &hardLinkMapRemoveParams)
{
    if (hardLinkMapRemoveParams.hardLinkMap != nullptr) {
        if (hardLinkMapRemoveParams.hardLinkMap->Exist(hardLinkMapRemoveParams.fileHandle.m_file->m_inode)) {
            string targetPath{};
            hardLinkMapRemoveParams.hardLinkMap->GetTargetPath(hardLinkMapRemoveParams.fileHandle.m_file->m_inode,
                targetPath);
            if (strcmp(hardLinkMapRemoveParams.fileHandle.m_file->m_fileName.c_str(), targetPath.c_str()) == 0) {
                DBGLOG("Removing Failed File Entry from hardlink map with inode: %lu, target: %s",
                    hardLinkMapRemoveParams.fileHandle.m_file->m_inode, targetPath.c_str());
                hardLinkMapRemoveParams.hardLinkMap->RemoveElement(hardLinkMapRemoveParams.fileHandle.m_file->m_inode);
            }
        }
    }

    return MP_SUCCESS;
}

void Libnfscommonmethods::FreeDirFh(FileHandle &fileHandle)
{
    struct nfsfh *nfsFh = fileHandle.m_file->dstIOHandle.nfsFh;
    if (nfsFh == nullptr) {
        return;
    }

    if (nfsFh->fh.val != nullptr) {
        free(nfsFh->fh.val);
    }
    free(nfsFh);
    fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
}

/* For every failed directory, a entry is made in fileHandleCache with value as this nfsfh with
 * fh.len as zero. Later in SendCreateReq, SendLstatReq and SendMknodReq, we can compare and see
 * the fh.len is zero and know parent directory creation failed. Then ignore that entry. */
void Libnfscommonmethods::FillFileHandleCacheWithInvalidDirectoryFh(string dirPath,
    shared_ptr<FileHandleCache> fileHandleCache)
{
    auto nfsfh = fileHandleCache->Get(dirPath);
    if (nfsfh == nullptr) {
        FileHandle fileHandleTmp {};
        fileHandleTmp.m_file = make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
        struct nfsfh *nfsFhTmp = (struct nfsfh *) malloc(sizeof(struct nfsfh));
        if (nfsFhTmp == nullptr) {
            ERRLOG("Failed to allocate Memory for nfsFhTmp");
            return;
        }

        memset_s(nfsFhTmp, sizeof(struct nfsfh), 0, sizeof(struct nfsfh));
        nfsFhTmp->fh.len = 0;
        fileHandleTmp.m_file->dstIOHandle.nfsFh = nfsFhTmp;
        if (fileHandleTmp.m_file->dstIOHandle.nfsFh != nullptr) {
            if (!(fileHandleCache->Push(dirPath, fileHandleTmp.m_file->dstIOHandle.nfsFh))) {
                WARNLOG("Freeing as fh already present: %s", dirPath.c_str());
                FreeDirFh(fileHandleTmp);
            }
        }
    }
}

int Libnfscommonmethods::ConstructReadBlock(FileHandle &fileHandle, uint64_t blockSize)
{
    uint64_t fileSize = fileHandle.m_file->m_size;
    uint64_t totalOffsetSendToRead = fileHandle.m_file->m_blockStats.m_readReqCnt * blockSize;
    fileHandle.m_block.m_size = ((fileSize - totalOffsetSendToRead) < blockSize)
        ? (fileSize - totalOffsetSendToRead) : blockSize;
    fileHandle.m_block.m_offset = totalOffsetSendToRead;
    fileHandle.m_block.m_seq = fileHandle.m_file->m_blockStats.m_readReqCnt;
    fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_block.m_size];
    if (fileHandle.m_block.m_buffer == nullptr) {
        ERRLOG("memalloc failed for Filename: %s, BlockSize: %lu",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_size);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

string Libnfscommonmethods::FloatToString(const float &val, const uint8_t &precisson)
{
    stringstream sstream;
    sstream << setiosflags(ios::fixed) << setprecision(precisson) << val;
    return sstream.str();
}

string Libnfscommonmethods::FormatCapacity(uint64_t capacity)
{
    float formatCapacity;

    if ((capacity >= MB_SIZE) && (capacity < GB_SIZE)) {
        formatCapacity = static_cast<float>(capacity) / MB_SIZE;
        return FloatToString(formatCapacity) + "MB";
    } else if ((capacity >= GB_SIZE) && (capacity < TB_SIZE)) {
        formatCapacity = static_cast<float>(capacity) / GB_SIZE;
        return FloatToString(formatCapacity) + "GB";
    } else if ((capacity >= TB_SIZE) && (capacity < PB_SIZE)) {
        formatCapacity = static_cast<float>(capacity) / TB_SIZE;
        return FloatToString(formatCapacity, NUMBER2) + "TB";
    } else if ((capacity >= KB_SIZE) && (capacity < MB_SIZE)) {
        formatCapacity = static_cast<float>(capacity) / KB_SIZE;
        return FloatToString(formatCapacity) + "KB";
    } else if (capacity >= PB_SIZE) {
        formatCapacity = static_cast<float>(capacity) / PB_SIZE;
        return FloatToString(formatCapacity) + "PB";
    } else {
        return to_string(capacity) + "B";
    }
}

void Libnfscommonmethods::RequestFailHandleAndCleanLinkMap(FileHandle &fileHandle, NfsCommonData *commonData,
    string direction, bool failIncOrgCount, bool removeHardlinkMapEntry)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (IS_INCREMENT_FAIL_COUNT(fileHandle)) {
        if (!failIncOrgCount) {
            commonData->controlInfo->m_noOfFilesFailed++;
        } else {
            commonData->controlInfo->m_noOfFilesFailed += fileHandle.m_file->m_originalFileCount;
        }
        FSBackupUtils::RecordFailureDetail(commonData->failureRecorder, fileHandle.m_file->m_fileName, EINVAL);
        ERRLOG("request file failed: %s, totalFailed: %llu", fileHandle.m_file->m_fileName.c_str(),
            commonData->controlInfo->m_noOfFilesFailed.load());
    }

    if (!commonData->skipFailure) {
        commonData->controlInfo->m_failed = true;
    }

    if (direction == LIBNFS_READER) {
        if (IS_INCREMENT_READ_FAIL_COUNT(fileHandle)) {
            commonData->controlInfo->m_noOfFilesReadFailed++;
        }
        fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
    } else {
        fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
    }

    if (removeHardlinkMapEntry) {
        /* The target file copy failed. The current logic of non target hardlink file creation will
         * start only once the target file is copied. As the target file cannot be copied,
         * the non target file entries will remain unprocessed in the queues and will lead to a hang.
         * So we need to delete the delete the inode from hardlink map. Further in writer,
         * if we encounter any LINK entries not present in hardlink map, mark them too as failed. */
        HardLinkMapRemoveParams hardLinkMapRemoveParams {};
        hardLinkMapRemoveParams.hardLinkMap = commonData->hardlinkMap;
        hardLinkMapRemoveParams.fileHandle = fileHandle;
        RemoveHardLinkMapEntryIfFileCreationFailed(hardLinkMapRemoveParams);
    }
}

void Libnfscommonmethods::CheckForCriticalError(NfsCommonData *commonData, int status, PKT_TYPE packetType)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (status == -ENOSPC || status == -ERANGE) {
        commonData->pktStats->Increment(packetType, PKT_COUNTER::FAILED, 1, PKT_ERROR::NO_SPACE_ERR);
    } else if (IS_NFSSHARE_ACCESS_ERROR(status)) {
        commonData->pktStats->Increment(packetType, PKT_COUNTER::FAILED, 1, PKT_ERROR::NO_ACCESS_ERR);
    } else {
        commonData->pktStats->Increment(packetType, PKT_COUNTER::FAILED);
    }
}

void Libnfscommonmethods::ExpireRetryTimers(BackupTimer &timer)
{
    vector<FileHandle> fileHandles {};
    timer.ClearAllEvents(fileHandles);
    for (FileHandle fileHandle : fileHandles) {
        if (fileHandle.m_file->srcIOHandle.nfsFh != nullptr) {
            FreeNfsFh(fileHandle.m_file->srcIOHandle.nfsFh);
        }
        if (fileHandle.m_file->dstIOHandle.nfsFh != nullptr) {
            FreeNfsFh(fileHandle.m_file->dstIOHandle.nfsFh);
        }
    }
}

void Libnfscommonmethods::RatelimitIncreaseMaxPendingRequestCount(string direction, time_t &ratelimitTimer,
    std::shared_ptr<LibnfsBackupAdvanceParams> advParams)
{
    if ((FSBackupUtils::GetCurrentTime() - ratelimitTimer) < RATELIMIT_TIMER_INTERVAL) {
        // Time elapsed not greater than RATELIMIT_TIMER_INTERVAL
        return;
    }

    // Time elapsed is greater than RATELIMIT_TIMER_INTERVAL. Start rate limit increase
    // Reset the timer
    ratelimitTimer = FSBackupUtils::GetCurrentTime();

    if (advParams->maxPendingAsyncReqCnt >= RATELIMIT_MAX_PENDING_REQ_CNT) {
        // Already pending req count is set to maximum. No need to increase further
        return;
    }

    uint32_t newMaxPendingRequestCount = advParams->maxPendingAsyncReqCnt * NUMBER2;
    uint32_t newMinPendingRequestCount = advParams->minPendingAsyncReqCnt * NUMBER2;
    if (newMaxPendingRequestCount >= RATELIMIT_MAX_PENDING_REQ_CNT) {
        advParams->maxPendingAsyncReqCnt = RATELIMIT_MAX_PENDING_REQ_CNT;
        advParams->minPendingAsyncReqCnt = RATELIMIT_MAX_PENDING_REQ_CNT_75_PERCENT;
    } else {
        advParams->maxPendingAsyncReqCnt = newMaxPendingRequestCount;
        advParams->minPendingAsyncReqCnt = newMinPendingRequestCount;
    }
    INFOLOG("Throttling up %s to %d & %d", direction.c_str(), advParams->maxPendingAsyncReqCnt,
        advParams->minPendingAsyncReqCnt);
}

void Libnfscommonmethods::RatelimitDecreaseMaxPendingRequestCount(string direction, time_t &ratelimitTimer,
    std::shared_ptr<LibnfsBackupAdvanceParams> advParams)
{
    // Reset the timer
    ratelimitTimer = FSBackupUtils::GetCurrentTime();

    if (advParams->maxPendingAsyncReqCnt <= RATELIMIT_MIN_PENDING_REQ_CNT) {
        // Already pending req count is set to minimum. No need to decrease further
        return;
    }

    uint32_t newMaxPendingRequestCount = advParams->maxPendingAsyncReqCnt / NUMBER2;
    uint32_t newMinPendingRequestCount = advParams->minPendingAsyncReqCnt / NUMBER2;
    if (newMaxPendingRequestCount <= RATELIMIT_MIN_PENDING_REQ_CNT) {
        advParams->maxPendingAsyncReqCnt = RATELIMIT_MIN_PENDING_REQ_CNT;
        advParams->minPendingAsyncReqCnt = RATELIMIT_MIN_PENDING_REQ_CNT_75_PERCENT;
    } else {
        advParams->maxPendingAsyncReqCnt = newMaxPendingRequestCount;
        advParams->minPendingAsyncReqCnt = newMinPendingRequestCount;
    }
    INFOLOG("Throttling for %s to %d & %d",  direction.c_str(), advParams->maxPendingAsyncReqCnt,
        advParams->minPendingAsyncReqCnt);
}
