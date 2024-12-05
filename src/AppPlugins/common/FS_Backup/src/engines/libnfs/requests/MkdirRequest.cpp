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
#include "MkdirRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    constexpr int RETRY_WAIT_IN_SEC = 1;
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
    const int DIRECTORY_DIFAULT_MODE = 19949;
}

NfsMkdirCbData* CreateMkdirCbData(FileHandle &fileHandle, NfsCommonData &commonData, int retryCnt,
    shared_ptr<FileHandleCache> fileHandleCache, struct nfsfh* nfsfh)
{
    auto cbData = new(nothrow) NfsMkdirCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;
    cbData->retryCnt = retryCnt;
    cbData->fileHandleCache = fileHandleCache;
    cbData->nfsfh = nfsfh;

    return cbData;
}

int SendMkdir(FileHandle &fileHandle, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->syncNfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send mkdir req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        return MP_FAILED;
    }
    int ret = MP_FAILED;
    if (cbData->nfsfh == nullptr) {
        if (IsRootDir(fileHandle.m_file->m_dirName)) {
            ret = CreateDirWithPath(fileHandle, nfs, cbData);
        } else {
            ret = HandleParentDirNotPresent(fileHandle, cbData->retryCnt, nfs, cbData);
        }
    } else {
        ret = CreateDirWithFh(fileHandle, cbData->nfsfh, nfs, cbData);
    }

    ret = HandleMkdirSyncReqStatus(ret, fileHandle, cbData->retryCnt, nfs, cbData);
    return ret;
}

int CreateDirWithPath(FileHandle &fileHandle, shared_ptr<NfsContextWrapper> nfs, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::SENT);

    int ret = MP_FAILED;
    if (fileHandle.m_file->m_fileName == "." || fileHandle.m_file->m_fileName.empty()) {
        ret = nfs->NfsLookupGetFhLock(fileHandle.m_file->m_fileName.c_str(),
            &fileHandle.m_file->dstIOHandle.nfsFh);
    } else {
        ret = nfs->NfsMkdirGetFhLock(fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
            &fileHandle.m_file->dstIOHandle.nfsFh);
    }

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::RECVD);
    return ret;
}

int CreateDirWithFh(FileHandle &fileHandle, struct nfsfh* nfsfh, shared_ptr<NfsContextWrapper> nfs,
    NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::SENT);

    int ret = nfs->NfsMkdirGetFhWithParentFhLock(fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_file->m_onlyFileName.c_str(), fileHandle.m_file->m_mode,
        &fileHandle.m_file->dstIOHandle.nfsFh, nfsfh);

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::RECVD);
    return ret;
}

int HandleParentDirNotPresent(FileHandle &fileHandle, uint16_t retryCnt,
    shared_ptr<NfsContextWrapper> nfs, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    string targetFilePath = FSBackupUtils::GetParentDir(fileHandle.m_file->m_fileName);
    DBGLOG("targetFilePath: %s", targetFilePath.c_str());
    if (MakeDirRecursively(targetFilePath, nfs, cbData) != MP_SUCCESS) {
        ERRLOG("Directory creation failed: %s, retry: %d", fileHandle.m_file->m_fileName.c_str(), retryCnt);
        return MP_FAILED;
    }

    auto nfsfh = cbData->fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (nfsfh == nullptr) {
        return MP_FAILED;
    }

    int ret = CreateDirWithFh(fileHandle, nfsfh, nfs, cbData);

    return ret;
}

int HandleMkdirSyncReqStatus(int status, FileHandle &fileHandle, uint16_t retryCnt,
    shared_ptr<NfsContextWrapper> nfs, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    if (status == -BACKUP_ERR_EEXIST) {
        if (HandleDirExist(fileHandle, retryCnt, nfs, cbData) != MP_SUCCESS) {
            return MP_FAILED;
        }
        return MP_SUCCESS;
    }

    if (IsCriticalError(status)) {
        HandleDstNoSpaceAndNoAccessError(status, fileHandle, cbData);
        return MP_FAILED;
    }

    if (IS_LIBNFS_NEED_RETRY(status)) {
        DBGLOG("mkdir failed for: %s, Status: %d, %s, retryCnt: %d", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs->GetNfsContext()), retryCnt);
        return MP_FAILED;
    }
    if (status != MP_SUCCESS) {
        /* TO-DO: Handle retry and non-retriable errors differently */
        ERRLOG("mkdir failed for: %s, Status: %d, %s, retryCnt: %d", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs->GetNfsContext()), retryCnt);
        return MP_FAILED;
    }

    if (fileHandle.m_file->dstIOHandle.nfsFh != nullptr) {
        if (!(cbData->fileHandleCache->Push(fileHandle.m_file->m_fileName, fileHandle.m_file->dstIOHandle.nfsFh))) {
            WARNLOG("Freeing as fh present: %s", fileHandle.m_file->m_fileName.c_str());
            FreeDirFh(fileHandle);
            return MP_SUCCESS;
        }
    }

    cbData->writeCommonData->controlInfo->m_noOfDirCopied++;

    return MP_SUCCESS;
}

int HandleDirExist(FileHandle &fileHandle, uint16_t retryCnt, shared_ptr<NfsContextWrapper> nfs,
    NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    struct nfs_stat_64 dirStatBuff {};
    int ret = MP_FAILED;

    auto nfsfh = cbData->fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (ProcessDirParentFh(fileHandle.m_file->m_fileName, fileHandle.m_file->m_dirName, nfsfh,
        retryCnt) != MP_SUCCESS) {
        return MP_FAILED;
    }

    if (nfsfh == nullptr) {
        ret = nfs->NfsLstat64Lock(fileHandle.m_file->m_fileName.c_str(), &dirStatBuff);
    } else {
        ret = nfs->NfsLstat64WithParentFhLock(fileHandle.m_file->m_onlyFileName.c_str(), nfsfh, &dirStatBuff);
    }

    if (ret == MP_SUCCESS) {
        if (!S_ISDIR(dirStatBuff.nfs_mode)) {
            // We got a directory creation request with same name as the file present in BackupFS.
            // delete file as delete entry will come later. Without deleting we cannot create dir.
            if (HandleFileExistWithSameNameAsDirectory(fileHandle, retryCnt, nfs, cbData) != MP_SUCCESS) {
                return MP_FAILED;
            }
        }
    }

    if (fileHandle.m_file->dstIOHandle.nfsFh != nullptr) {
        if (!(cbData->fileHandleCache->Push(fileHandle.m_file->m_fileName, fileHandle.m_file->dstIOHandle.nfsFh))) {
            WARNLOG("Freeing as fh present: %s", fileHandle.m_file->m_fileName.c_str());
            FreeDirFh(fileHandle);
        }
    }

    return MP_SUCCESS;
}

int HandleFileExistWithSameNameAsDirectory(FileHandle &fileHandle, uint16_t retryCnt,
    shared_ptr<NfsContextWrapper> nfs, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::SENT);
    int status = nfs->NfsUnlinkLock(fileHandle.m_file->m_fileName.c_str());
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::RECVD);

    if (status != MP_SUCCESS) {
        ERRLOG("unlink failed for: %s, Status: %d, %s, retryCnt: %d", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs->GetNfsContext()), retryCnt);

        return MP_FAILED;
    }

    auto nfsfh = cbData->fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (ProcessDirParentFh(fileHandle.m_file->m_fileName, fileHandle.m_file->m_dirName, nfsfh,
        retryCnt) != MP_SUCCESS) {
        return MP_FAILED;
    }
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::SENT);
    int ret = MP_FAILED;
    if (nfsfh == nullptr) {
        ret = nfs->NfsMkdirGetFhLock(fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
            &fileHandle.m_file->dstIOHandle.nfsFh);
    } else {
        ret = nfs->NfsMkdirGetFhWithParentFhLock(fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_file->m_onlyFileName.c_str(), fileHandle.m_file->m_mode,
            &fileHandle.m_file->dstIOHandle.nfsFh, nfsfh);
    }

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::RECVD);

    if (IsCriticalError(ret)) {
        HandleDstNoSpaceAndNoAccessError(ret, fileHandle, cbData);
    }
    if (ret != MP_SUCCESS) {
        ERRLOG("mkdir failed for: %s, Status: %d, %s, retryCnt: %d", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs->GetNfsContext()), retryCnt);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void HandleDstNoSpaceAndNoAccessError(int status, FileHandle &fileHandle, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return;
    }

    if (fileHandle.m_file->m_dirName.empty()) {
        if (IS_NFSSHARE_ACCESS_ERROR(status)) {
            cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::FAILED, DEFAULT_MAX_NOACCESS,
                PKT_ERROR::NO_ACCESS_ERR);
        } else {
            cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::FAILED, DEFAULT_MAX_NOSPACE,
                PKT_ERROR::NO_SPACE_ERR);
        }
    }
    if (IS_NFSSHARE_ACCESS_ERROR(status)) {
        cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::FAILED, OFFSET_10,
            PKT_ERROR::NO_ACCESS_ERR);
    } else {
        cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::FAILED, OFFSET_10,
            PKT_ERROR::NO_SPACE_ERR);
    }
    return;
}

int MakeDirRecursively(string targetFilePath, shared_ptr<NfsContextWrapper> nfs, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    if (cbData->writeCommonData == nullptr) {
        ERRLOG("writeCommonData is nullptr");
        return MP_FAILED;
    }

    string dirPathStr {};
    string parentDir {};
    bool makeDirFailed = false;

    istringstream stringStream{targetFilePath};
    string token {};

    while (getline(stringStream, token, '/')) {
        if (*(cbData->writeCommonData->abort)) {
            return MP_FAILED;
        }
        if (!token.empty() && token != ".") {
            dirPathStr += "/" + token;
            DBGLOG("MakeDirWithRetry: %s", dirPathStr.c_str());
            MakeDirWithRetry(dirPathStr, parentDir, makeDirFailed, nfs, cbData);
        }
    }

    if (makeDirFailed) {
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

int MakeDirWithRetry(string dirPath, string &parentDirPath, bool &makeDirFailed,
    shared_ptr<NfsContextWrapper> nfs, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    if (cbData->writeCommonData == nullptr) {
        ERRLOG("writeCommonData is nullptr");
        return MP_FAILED;
    }

    if (makeDirFailed) {
        ERRLOG("Create failed for : %s", dirPath.c_str());
        FillFileHandleCacheWithInvalidDirectoryFh(dirPath, cbData->fileHandleCache);
        return MP_FAILED;
    }

    uint16_t retryCnt = 0;
    int ret = MP_FAILED;
    do {
        if (*(cbData->writeCommonData->abort)) {
            ERRLOG("abort");
            return MP_FAILED;
        }
        ret = MakeDirSync(dirPath, parentDirPath, retryCnt, nfs, cbData);
        if (ret != MP_SUCCESS) {
            sleep(RETRY_WAIT_IN_SEC);
        }
        retryCnt++;
    } while ((ret != MP_SUCCESS) && (retryCnt <= DEFAULT_MAX_REQUEST_RETRY));

    if (ret != MP_SUCCESS) {
        makeDirFailed = true;
        ERRLOG("Create failed for : %s", dirPath.c_str());
        FillFileHandleCacheWithInvalidDirectoryFh(dirPath, cbData->fileHandleCache);
        return MP_FAILED;
    }

    parentDirPath = dirPath;
    return MP_SUCCESS;
}

int MakeDirSync(string dirPath, string parentDirPath, uint16_t retryCnt,
    shared_ptr<NfsContextWrapper> nfs, NfsMkdirCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    string onlyDirPath = dirPath.substr(parentDirPath.length() + 1,
        dirPath.length() - parentDirPath.length() - 1);

    /* If dir is already created. No need to create again */
    auto nfsfhStat = cbData->fileHandleCache->Get(dirPath);
    if (nfsfhStat != nullptr) {
        DBGLOG("nfsfh stat is nullptr");
        return MP_SUCCESS;
    }

    auto nfsfhParent = cbData->fileHandleCache->Get(parentDirPath);
    if (ProcessDirParentFh(dirPath, parentDirPath, nfsfhParent, retryCnt) != MP_SUCCESS) {
        return MP_FAILED;
    }

    FileHandle fileHandle {};
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = dirPath;
    fileHandle.m_file->m_dirName = parentDirPath;
    fileHandle.m_file->m_onlyFileName = onlyDirPath;
    fileHandle.m_file->m_size = DIRECTORY_SIZE;

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::SENT);

    int ret = MP_FAILED;
    if (nfsfhParent == nullptr) {
        DBGLOG("Create parent directory: %s", dirPath.c_str());
        ret = nfs->NfsMkdirGetFhLock(dirPath.c_str(), DIRECTORY_DIFAULT_MODE, &fileHandle.m_file->dstIOHandle.nfsFh);
    } else {
        DBGLOG("Create parent directory: %s", dirPath.c_str());
        ret = nfs->NfsMkdirGetFhWithParentFhLock(dirPath.c_str(), onlyDirPath.c_str(), DIRECTORY_DIFAULT_MODE,
            &fileHandle.m_file->dstIOHandle.nfsFh, nfsfhParent);
    }

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::RECVD);

    return HandleMkdirSyncReqStatus(ret, fileHandle, retryCnt, nfs, cbData);
}

bool IsRootDir(string path)
{
    if (path == "." || path.empty()) {
        return true;
    }
    return false;
}

bool IsCriticalError(int status)
{
    if (status == -ENOSPC || status == -ERANGE || IS_NFSSHARE_ACCESS_ERROR(status)) {
        return true;
    }
    return false;
}
