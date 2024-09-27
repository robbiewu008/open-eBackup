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
#ifndef LIBNFS_MKDIR_REQUEST_H
#define LIBNFS_MKDIR_REQUEST_H

#include "LibnfsCommonMethods.h"

class NfsMkdirCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *writeCommonData;
    std::shared_ptr<FileHandleCache> fileHandleCache;
    struct nfsfh *nfsfh;
    int retryCnt = 0;
};

NfsMkdirCbData* CreateMkdirCbData(FileHandle &fileHandle, NfsCommonData &commonData, int retryCnt,
    std::shared_ptr<FileHandleCache> fileHandleCache, struct nfsfh* nfsfh);
int SendMkdir(FileHandle &fileHandle, NfsMkdirCbData *cbData);
int CreateDirWithPath(FileHandle &fileHandle, std::shared_ptr<Module::NfsContextWrapper> nfs, NfsMkdirCbData *cbData);
int CreateDirWithFh(FileHandle &fileHandle, struct nfsfh* nfsfh, std::shared_ptr<Module::NfsContextWrapper> nfs,
    NfsMkdirCbData *cbData);
int HandleParentDirNotPresent(FileHandle &fileHandle, uint16_t retryCnt,
    std::shared_ptr<Module::NfsContextWrapper> nfs, NfsMkdirCbData *cbData);

int HandleMkdirSyncReqStatus(int status, FileHandle &fileHandle, uint16_t retryCnt,
    std::shared_ptr<Module::NfsContextWrapper> nfs, NfsMkdirCbData *cbData);

int HandleDirExist(FileHandle &fileHandle, uint16_t retryCnt, std::shared_ptr<Module::NfsContextWrapper> nfs,
    NfsMkdirCbData *cbData);
int HandleFileExistWithSameNameAsDirectory(FileHandle &fileHandle, uint16_t retryCnt,
    std::shared_ptr<Module::NfsContextWrapper> nfs, NfsMkdirCbData *cbData);
void HandleDstNoSpaceAndNoAccessError(int status, FileHandle &fileHandle, NfsMkdirCbData *cbData);

int MakeDirRecursively(std::string targetFilePath, std::shared_ptr<Module::NfsContextWrapper> nfs,
    NfsMkdirCbData *cbData);
int MakeDirWithRetry(std::string dirPath, std::string &parentDirPath, bool &makeDirFailed,
    std::shared_ptr<Module::NfsContextWrapper> nfs, NfsMkdirCbData *cbData);
int MakeDirSync(std::string dirPath, std::string parentDirPath, uint16_t retryCnt,
    std::shared_ptr<Module::NfsContextWrapper> nfs, NfsMkdirCbData *cbData);

bool IsRootDir(std::string path);
bool IsCriticalError(int status);

#endif // LIBNFS_MKDIR_REQUEST_H
