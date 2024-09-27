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
#include "log/Log.h"
#include "system/System.hpp"
#include "LibnfsDirMetaWriter.h"
#include "LibnfsDeleteWriter.h"
#include "LibnfsServiceTask.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    constexpr auto MODULE_NAME = "LibnfsServiceTask";
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

LibnfsServiceTask::~LibnfsServiceTask()
{}

void LibnfsServiceTask::Exec()
{
    HCPTSP::getInstance().reset(m_params.backupParams.commonParams.reqID);

    switch (m_event) {
        case LibnfsEvent::WRITE_META: {
            HandleWriteMeta();
            break;
        }
        case LibnfsEvent::DELETE: {
            HandleDelete();
            break;
        }
        default:
            break;
    }
}

void LibnfsServiceTask::HandleWriteMeta()
{
    for (uint32_t i = 0; i < m_fileHandleList.size(); i++) {
        if (*(m_params.abort) || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
            break;
        }
        auto nfsfh = m_fileHandleCache->Get(m_fileHandleList[i].m_file->m_fileName);
        if (nfsfh == nullptr) {
            if (LookupRecursively(m_fileHandleList[i].m_file->m_fileName) != MP_SUCCESS) {
                DBGLOG("Lookup failed for: %s", m_fileHandleList[i].m_file->m_fileName.c_str());
                continue;
            }

            nfsfh = m_fileHandleCache->Get(m_fileHandleList[i].m_file->m_fileName);
            if (nfsfh == nullptr) {
                ERRLOG("Cannot obtain filehandle for: %s", m_fileHandleList[i].m_file->m_fileName.c_str());
                FSBackupUtils::RecordFailureDetail(m_failureRecorder, m_fileHandleList[i].m_file->m_fileName, ENOENT);
                m_controlInfo->m_noOfDirFailed++;
                continue;
            }
        }

        if (!(m_params.backupParams.commonParams.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE ||
            m_params.backupParams.commonParams.restoreReplacePolicy == RestoreReplacePolicy::NONE)) {
            // Dirmtime not to be set if it already exist as ReplacePolicy is ignore if older/Ignore if exist
            if (IsDirMetaSetRequired(m_fileHandleList[i]) == false) {
                continue;
            }
        }

        struct nfs_stat_64 allMetaData {};
        allMetaData.nfs_mode = m_fileHandleList[i].m_file->m_mode;
        allMetaData.nfs_uid = m_fileHandleList[i].m_file->m_uid;
        allMetaData.nfs_gid = m_fileHandleList[i].m_file->m_gid;
        allMetaData.nfs_atime = m_fileHandleList[i].m_file->m_atime;
        allMetaData.nfs_mtime = m_fileHandleList[i].m_file->m_mtime;
        int status = m_nfsCtx->NfsChmodChownUtimeLock(nfsfh, m_fileHandleList[i].m_file->m_fileName.c_str(),
            allMetaData);
        if (status != MP_SUCCESS) {
            HandleDirSetMetaFailure(status, m_fileHandleList[i]);
        } else {
            m_controlInfo->m_noOfDirCopied++;
            DBGLOG("DirMtimeSet success for: %s", m_fileHandleList[i].m_file->m_fileName.c_str());
        }
    }
    m_result = MP_SUCCESS;
    return;
}

void LibnfsServiceTask::HandleDelete()
{
    if (m_fileHandle.m_file->m_fileName.empty()) {
        m_result = MP_SUCCESS;
        return;
    }

    string filePath = m_fileHandle.m_file->m_fileName;
    string fPath {};
    int delStatIsDir = NUMBER_ZERO;
    struct stat st;
    if (filePath.at(0) == '.') {
        fPath = SEP + (filePath).substr(NUMBER_TWO, filePath.size() + NUMBER_ONE);
    } else {
        fPath = filePath;
    }
    string fullPath(m_params.dstRootPath);
    fullPath.append(fPath);

    int ret = lstat(fullPath.c_str(), &st);
    if (ret == 0) {
        if (HandleLstatStatus(fullPath, st, delStatIsDir) != MP_SUCCESS) {
            m_result = MP_SUCCESS;
            return;
        }
        DeleteFilesAndDirectory(m_fileHandle.m_file->m_fileName, delStatIsDir);
    } else if (errno == EIO) { // Retriable error
        HandleLstatRetry();
    } else if (errno == ENOENT) {
        DBGLOG("File/Dir: %s does not exist", fullPath.c_str());
    } else if (errno == BACKUP_ERR_NOTDIR) {
        WARNLOG("Dir : %s not a directory", fullPath.c_str());
    } else if (ret != MP_SUCCESS) {
        std::string errmsg = hcp_strerror(errno);
        ERRLOG("lstat failed for: %s errorno: %d and error string: %s", fullPath.c_str(), errno, errmsg.c_str());

        if (m_fileHandle.m_file->IsFlagSet(IS_DIR)) {
            m_controlInfo->m_noOfDirFailed++;
        } else {
            m_controlInfo->m_noOfFilesFailed++;
        }
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, fullPath, errno);
        ERRLOG("totalFailed: %llu, %llu", m_controlInfo->m_noOfDirFailed.load(),
            m_controlInfo->m_noOfFilesFailed.load());

        if (!m_params.backupParams.commonParams.skipFailure) {
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
        }
    }

    m_result = MP_SUCCESS;
    return;
}

int LibnfsServiceTask::HandleLstatStatus(const std::string &fullPath, struct stat &st, int &delStatIsDir)
{
    // For ReplacePolicy: 0 and 2, we ignore deleting
    // For ReplacePolicy: 1 and 22 (Default) we are deleting
    if (!(m_params.backupParams.commonParams.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE ||
        m_params.backupParams.commonParams.restoreReplacePolicy == RestoreReplacePolicy::NONE)) {
        INFOLOG("File/Dir not deleted : %s, restoreReplacePolicy is ignore", fullPath.c_str());
        return MP_FAILED;
    }
    DetermineDirectoryOrFile(st, delStatIsDir);
    auto fileCTime = static_cast<uint32_t>(st.st_ctime);
    if (fileCTime >= m_params.jobStartTime && fileCTime < m_params.deleteJobStartTime && st.st_nlink <= 1) {
        // fileCtime will be greater than startTime if it is newly created.
        // fileCtime can be changed during deletion phase in scenarios like deletion of
        // child files or directories. So fileCTime should be less than deleteJobStartTime.
        INFOLOG("File/Dir newly created: %s fileCTime: %d jobStartTime: %d", fullPath.c_str(), fileCTime,
            m_params.jobStartTime);
        return MP_FAILED;
    }
    if (CompareTypeOfDeleteEntryAndBackupCopy(delStatIsDir) != MP_SUCCESS) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int LibnfsServiceTask::CompareTypeOfDeleteEntryAndBackupCopy(int delStatIsDir) const
{
    if (m_fileHandle.m_file->IsFlagSet(IS_DIR)) {
        if (delStatIsDir == 0) {
            WARNLOG("File present. Already deleted and created as file: %s", m_fileHandle.m_file->m_fileName.c_str());
            return MP_FAILED;
        }
    } else {
        if (delStatIsDir != 0) {
            WARNLOG("Dir present. Already deleted and created as dir: %s", m_fileHandle.m_file->m_fileName.c_str());
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

void LibnfsServiceTask::DeleteFilesAndDirectory(std::string &fileName, int delStatIsDir)
{
    if (delStatIsDir == 1) {
        DeleteDirectory(fileName);
    } else {
        DeleteFile(fileName);
    }
}

int LibnfsServiceTask::DeleteDirectory(const std::string &filePath)
{
    uint32_t retryCnt = 0;
    int ret = 0;
    bool deletedFlag = true;
    errno = NUMBER_ZERO;
    /* Here append the filePath with BackupFsLocalPath */
    string fullPath = m_params.dstRootPath + filePath;
    DBGLOG("nftw called for: %s", fullPath.c_str());
    vector<string> output;
    vector<string> errOutput;
    do {
        retryCnt++;
        ret = nftw(fullPath.c_str(), UnlinkCb, NUMBER_EIGHT, FTW_DEPTH | FTW_PHYS);
        if (ret != NUMBER_ZERO) {
            if (errno != ENOENT) {
                ERRLOG("Deletion Failed for: %s ret: %d, errno: %d", fullPath.c_str(), ret, errno);
            }
        } else {
            DBGLOG("Deleted Dir/File: %s ret= %d", filePath.c_str(), ret);
        }
        string rmDirFile = "rm -rf '" + fullPath + "'";
        (void)runShellCmdWithOutput(INFO, MODULE_NAME, 0, rmDirFile, { }, output, errOutput);

        /* Check if the directory still exists. If exists retry delete */
        struct stat sb;
        ret = lstat(fullPath.c_str(), &sb);
        if (ret == MP_SUCCESS) {
            deletedFlag = false;
            /* This indicates the directory still exists. So retry delete after 10 seconds */
            ERRLOG("Directory still exists : %s", fullPath.c_str());
        } else if (errno == ENOENT) {
            DBGLOG("Directory is deleted : %s", fullPath.c_str());
            deletedFlag = true;
        } else {
            deletedFlag = false;
            std::this_thread::sleep_for(chrono::milliseconds(DEFAULT_MAX_RETRY_TIMEOUT));
            m_pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        }
    } while ((!deletedFlag) && retryCnt < DEFAULT_MAX_REQUEST_RETRY);
    if (ret != MP_SUCCESS) {
        DBGLOG("Remove succeeded for: %s", fullPath.c_str());
        m_controlInfo->m_noOfDirDeleted++;
        m_pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::FAILED);
    } else {
        ERRLOG("Remove failed for: %s after retries: %d", fullPath.c_str(), retryCnt);
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, fullPath, errno);
        m_controlInfo->m_noOfDirFailed++;
        if (!m_params.backupParams.commonParams.skipFailure) {
            m_controlInfo->m_failed = true;
        }
        m_pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::FAILED);
    }
    return ret;
}

int LibnfsServiceTask::DeleteFile(std::string &fPath)
{
    string filePath {};
    uint32_t retryCnt = 0;
    int rv = 0;
    int removeError = 0;
    if (fPath.at(0) == '.') {
        filePath = SEP + (fPath).substr(NUMBER_TWO, fPath.size() + NUMBER_ONE);
    } else {
        filePath = fPath;
    }
    string fullPath(m_params.dstRootPath);
    fullPath.append(filePath);
    DBGLOG("Remove called for: %s", fullPath.c_str());
    do {
        retryCnt++;
        rv = remove(fullPath.c_str());
        removeError = errno;
        if (removeError == EIO) {
            std::this_thread::sleep_for(chrono::milliseconds(DEFAULT_MAX_RETRY_TIMEOUT));
            m_pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        } /* EIO is the error set if mount path is not accessible due to network down */
    } while ((removeError == EIO) && retryCnt < DEFAULT_MAX_REQUEST_RETRY);
    if (rv != MP_SUCCESS) {
        if (removeError == ENOENT) {
            DBGLOG("File: %s does not exist", fullPath.c_str());
        } else {
            string removeErrorStr = hcp_strerror(errno);
            FSBackupUtils::RecordFailureDetail(m_failureRecorder, filePath, errno);
            m_controlInfo->m_noOfFilesFailed++;
            ERRLOG("Remove failed for: %s with error: %d and error string: %s after retries: %d, totalFailed: %llu",
                fullPath.c_str(), errno, removeErrorStr.c_str(), retryCnt, m_controlInfo->m_noOfFilesFailed.load());
            if (!m_params.backupParams.commonParams.skipFailure) {
                ERRLOG("set backup to failed!");
                m_controlInfo->m_failed = true;
            }
        }
    } else {
        DBGLOG("Remove succeeded for:  %s", fullPath.c_str());
        m_controlInfo->m_noOfFilesDeleted++;
    }
    return rv;
}

int LibnfsServiceTask::UnlinkCb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    DBGLOG("Unlinkb calld for: %s", fpath);
    int rv = remove(fpath);
    if (rv != MP_SUCCESS) {
        std::string errmsg = hcp_strerror(errno);
        DBGLOG("Remove failed for: %s with error: %d and error string: %s", fpath, errno, errmsg.c_str());
    }
    UNREFERENCE_PARAM(sb);
    UNREFERENCE_PARAM(typeflag);
    UNREFERENCE_PARAM(ftwbuf);
    return rv;
}

void LibnfsServiceTask::DetermineDirectoryOrFile(struct stat &st, int &delStatIsDir) const
{
    switch (st.st_mode & S_IFMT) {
        case S_IFDIR:
            delStatIsDir = 1;
            break;
        default:
            delStatIsDir = 0;
            break;
    }
}

bool LibnfsServiceTask::IsDirMetaSetRequired(FileHandle &fileHandle)
{
    string parentDirName = FSBackupUtils::GetParentDir(fileHandle.m_file->m_fileName);
    auto parentFh = m_fileHandleCache->Get(parentDirName);
    if (parentFh == nullptr && (parentDirName != "." && !parentDirName.empty())) {
        ERRLOG("Cannot obtain filehandle for: %s", fileHandle.m_file->m_fileName.c_str());
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, fileHandle.m_file->m_fileName, ENOENT);
        m_controlInfo->m_noOfDirFailed++;
        return false;
    }
    struct nfs_stat_64 st {};
    int ret = MP_FAILED;
    if (parentFh == nullptr) {
        ret = m_nfsCtx->NfsLstat64Lock(fileHandle.m_file->m_fileName.c_str(), &st);
    } else {
        ret = m_nfsCtx->NfsLstat64WithParentFhLock(fileHandle.m_file->m_onlyFileName.c_str(), parentFh, &st);
    }
    if (HandleDirLstatStatus(ret, fileHandle, st) != MP_SUCCESS) {
        return false;
    }

    return true;
}

int LibnfsServiceTask::HandleDirLstatStatus(int status, FileHandle &fileHandle, struct nfs_stat_64 st)
{
    if (status == -BACKUP_ERR_ENOENT) {
        // Directory not exist. FLR case. Can proceed with set dir meta and handle there
        return MP_SUCCESS;
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        HandleDirLstatRetry(fileHandle);
        return MP_FAILED;
    } else if (status != MP_SUCCESS) {
        ERRLOG("dir lstat failed for: %s, Status: %d", fileHandle.m_file->m_fileName.c_str(), status);
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, fileHandle.m_file->m_fileName, -status);
        m_controlInfo->m_noOfDirFailed++;

        if (!m_params.backupParams.commonParams.skipFailure) {
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
        }
        return MP_FAILED;
    }
    // Directory exists. Need check if newly created or not
    if (IsDirAlreadyExistedInTargetFS(fileHandle, st)) {
        // Already existed in target FS. So shouldn't apply dir meta
        return MP_FAILED;
    }

    // Directory didn't exist in target FS before the restore. Need set dir meta
    return MP_SUCCESS;
}

bool LibnfsServiceTask::IsDirAlreadyExistedInTargetFS(FileHandle &fileHandle, struct nfs_stat_64 st)
{
    auto fileCTime = static_cast<uint32_t>(st.nfs_ctime);
    if (fileCTime >= m_params.jobStartTime) {
        // directory Ctime will be greater than startTime if it is newly created.
        // So should apply dirmeta.
        return false;
    }
    DBGLOG("Dir: %s already exists. fileCTime: %d jobStartTime: %d", fileHandle.m_file->m_fileName.c_str(),
        fileCTime, m_params.jobStartTime);
    return true;
}

void LibnfsServiceTask::HandleDirLstatRetry(FileHandle &fileHandle)
{
    auto cpyWriter = (LibnfsDirMetaWriter *)m_params.writeObj;
    if (cpyWriter == nullptr) {
        ERRLOG("cpyWriter is nullptr");
        return;
    }

    fileHandle.m_retryCnt++;
    m_pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
    if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
        DBGLOG("Dir lstat failed after max retries for: %s", fileHandle.m_file->m_fileName.c_str());
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, fileHandle.m_file->m_fileName, EINVAL);
        m_controlInfo->m_noOfDirFailed++;
        if (!m_params.backupParams.commonParams.skipFailure) {
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
        }
    } else {
        m_pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::RETRIED);
        cpyWriter->m_timer.Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
    }
}

void LibnfsServiceTask::HandleDirSetMetaFailure(int status, FileHandle &fileHandle)
{
    if (status == -BACKUP_ERR_ENOENT) {
        DBGLOG("Directory not exist: %s", fileHandle.m_file->m_fileName.c_str());
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        HandleDirMetaRetry(fileHandle);
    } else {
        ERRLOG("mtime set failed for:  %s mode: %d uid: %d gid: %d atime: %d mtime: %d, Status: %d",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
            fileHandle.m_file->m_uid, fileHandle.m_file->m_gid,
            fileHandle.m_file->m_atime, fileHandle.m_file->m_mtime, status);
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, fileHandle.m_file->m_fileName, -status);
        m_controlInfo->m_noOfDirFailed++;

        if (!m_params.backupParams.commonParams.skipFailure) {
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
        }
    }
}

int LibnfsServiceTask::LookupRecursively(std::string targetFilePath)
{
    string dirPathStr {};
    string parentDir {};

    istringstream stringStream{targetFilePath};
    string token {};

    bool lookupFailed = false;
    while (getline(stringStream, token, '/')) {
        if (*(m_params.abort) || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
            break;
        }
        if (!token.empty() && token != ".") {
            dirPathStr += "/" + token;
            LookupDir(dirPathStr, parentDir, lookupFailed);
        }
    }

    if (lookupFailed) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int LibnfsServiceTask::LookupDir(std::string dirPath, std::string &parentDirPath, bool &lookupFailed)
{
    if (lookupFailed) {
        DBGLOG("Lookup failed for : %s", dirPath.c_str());
        FillFileHandleCacheWithInvalidDirectoryFh(dirPath);
        return MP_FAILED;
    }

    uint16_t retryCnt = 0;
    int ret = MP_FAILED;
    do {
        if (*(m_params.abort) || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
            break;
        }
        ret = LookupSync(dirPath, parentDirPath, retryCnt);
        if (ret != MP_SUCCESS) {
            std::this_thread::sleep_for(chrono::milliseconds(DEFAULT_MAX_RETRY_TIMEOUT));
        }
        retryCnt++;
    } while ((ret != MP_SUCCESS) && (retryCnt <= DEFAULT_MAX_REQUEST_RETRY));

    if (ret != MP_SUCCESS) {
        lookupFailed = true;
        DBGLOG("Lookup failed for : %s", dirPath.c_str());
        FillFileHandleCacheWithInvalidDirectoryFh(dirPath);
        return MP_FAILED;
    }

    parentDirPath = dirPath;
    return MP_SUCCESS;
}

int LibnfsServiceTask::LookupSync(std::string dirPath, std::string parentDirPath, uint16_t retryCnt)
{
    string onlyDirPath = dirPath.substr(parentDirPath.length() + 1,
        dirPath.length() - parentDirPath.length() - 1);

    /* Dir is already present in fileHandleCache. No need to do lookup again */
    auto nfsfhStat = m_fileHandleCache->Get(dirPath);
    if (nfsfhStat != nullptr) {
        return MP_SUCCESS;
    }

    auto nfsfhParent = m_fileHandleCache->Get(parentDirPath);
    if (parentDirPath != "." && nfsfhParent != nullptr) {
        if (IsValidNfsFh(nfsfhParent) != MP_SUCCESS) {
            ERRLOG("Parent directory lookup failed for dir: %s. Parent directory: %s retry: %d",
                dirPath.c_str(), parentDirPath.c_str(), retryCnt);
            return MP_FAILED;
        }
    }

    int ret = MP_FAILED;
    struct nfsfh *nfsfh = nullptr;
    if (nfsfhParent == nullptr) {
        ret = m_nfsCtx->NfsLookupGetFhLock(dirPath.c_str(), &nfsfh);
    } else {
        ret = m_nfsCtx->NfsLookupGetFhWithParentFhLock(dirPath.c_str(), onlyDirPath.c_str(), &nfsfh, nfsfhParent);
    }

    return HandleLookupSyncReqStatus(ret, retryCnt, nfsfh, dirPath, m_nfsCtx);
}

int LibnfsServiceTask::HandleLookupSyncReqStatus(int status, uint16_t retryCnt, struct nfsfh *nfsfh,
    std::string fileName, shared_ptr<NfsContextWrapper> nfs)
{
    if (IS_LIBNFS_NEED_RETRY(status)) {
        DBGLOG("Lookup failed: %s Status: %d %s retryCnt: %d", fileName.c_str(), status,
            nfs_get_error(nfs->GetNfsContext()), retryCnt);
        return MP_FAILED;
    }

    if (status < MP_SUCCESS) {
        ERRLOG("Lookup failed: %s Status: %d %s retryCnt: %d", fileName.c_str(), status,
            nfs_get_error(nfs->GetNfsContext()), retryCnt);
        return MP_FAILED;
    }

    if (nfsfh == nullptr) {
        ERRLOG("Lookup failed to return filehandle for directory: %s", fileName.c_str());
        return MP_FAILED;
    }

    if (!(m_fileHandleCache->Push(fileName, nfsfh))) {
        DBGLOG("Filehandle already present for directory: %s", fileName.c_str());
        FreeNfsFh(nfsfh);
    }

    return MP_SUCCESS;
}

void LibnfsServiceTask::FillFileHandleCacheWithInvalidDirectoryFh(std::string dirPath)
{
    auto nfsfh = m_fileHandleCache->Get(dirPath);
    if (nfsfh == nullptr) {
        struct nfsfh *nfsFhTmp = (struct nfsfh *) malloc(sizeof(struct nfsfh));
        if (nfsFhTmp == nullptr) {
            ERRLOG("Failed to allocate Memory for nfsFhTmp");
            return;
        }

        memset_s(nfsFhTmp, sizeof(struct nfsfh), 0, sizeof(struct nfsfh));
        nfsFhTmp->fh.len = 0;
        if (!(m_fileHandleCache->Push(dirPath, nfsFhTmp))) {
            WARNLOG("Freeing as fh already present: %s", dirPath.c_str());
            FreeNfsFh(nfsFhTmp);
        }
    }
}

void LibnfsServiceTask::HandleDirMetaRetry(FileHandle &fileHandle)
{
    auto cpyWriter = (LibnfsDirMetaWriter *)m_params.writeObj;
    if (cpyWriter == nullptr) {
        ERRLOG("cpyWriter is nullptr");
        return;
    }

    fileHandle.m_retryCnt++;
    m_pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
    if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
        DBGLOG("MetaSet failed after max retries for:  %s mode: %d uid: %d gid: %d atime: %d mtime: %d",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
            fileHandle.m_file->m_uid, fileHandle.m_file->m_gid, fileHandle.m_file->m_atime,
            fileHandle.m_file->m_mtime);
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, fileHandle.m_file->m_fileName, EINVAL);
        m_controlInfo->m_noOfDirFailed++;
        if (!m_params.backupParams.commonParams.skipFailure) {
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
        }
    } else {
        m_pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::RETRIED);
        cpyWriter->m_timer.Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
    }
}

void LibnfsServiceTask::HandleLstatRetry()
{
    auto cpyWriter = (LibnfsDeleteWriter *)m_params.writeObj;
    if (cpyWriter == nullptr) {
        ERRLOG("cpyWriter is nullptr");
        return;
    }

    m_fileHandle.m_retryCnt++;
    m_pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
    if (m_fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
        DBGLOG("LStat failed after max retries for:  %s", m_fileHandle.m_file->m_fileName.c_str());
        if (m_fileHandle.m_file->IsFlagSet(IS_DIR)) {
            m_controlInfo->m_noOfDirFailed++;
        } else {
            m_controlInfo->m_noOfFilesFailed++;
        }
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, m_fileHandle.m_file->m_fileName, EINVAL);
        ERRLOG("failed fileName: %s, totalFailed: %llu", m_fileHandle.m_file->m_fileName.c_str(),
            m_controlInfo->m_noOfFilesFailed.load());
        if (!m_params.backupParams.commonParams.skipFailure) {
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
        }
    } else {
        m_pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::RETRIED);
        cpyWriter->m_timer.Insert(m_fileHandle, m_fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
    }
}