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
#ifndef LIBNFS_LSTAT_REQUEST_H
#define LIBNFS_LSTAT_REQUEST_H

#include "CopyCtrlParser.h"
#include "LibnfsCommonMethods.h"
#include "LibnfsInterface.h"

class NfsLstatCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *writeCommonData;
    RestoreReplacePolicy restoreReplacePolicy { RestoreReplacePolicy::NONE };
    struct nfsfh *nfsfh;
};

NfsLstatCbData* CreateLstatCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh,
    RestoreReplacePolicy restoreReplacePolicy);
int SendLstat(FileHandle &fileHandle, NfsLstatCbData *cbData);
void SendLstatCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void HandleLstatFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status, struct nfs_context *nfs);
bool CheckConditionsForBackupOrRestoreJob(NfsCommonData *commonData, FileHandle &fileHandle,
    RestoreReplacePolicy restoreReplacePolicy, struct nfs_stat_64 *st);
bool CheckConditionsForRestore(NfsCommonData *commonData, FileHandle &fileHandle,
    RestoreReplacePolicy restoreReplacePolicy, struct nfs_stat_64 *st, std::string targetPath);
void HandleOverWrite(NfsCommonData *commonData, FileHandle &fileHandle, struct nfs_stat_64 *st,
    RestoreReplacePolicy restoreReplacePolicy);
void LstatFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle);
void PushToWriteQueue(NfsCommonData *commonData, FileHandle &fileHandle);
int SendCreateWithTruncateFlag(FileHandle &fileHandle, NfsCommonData *commonData, struct nfsfh *nfsfh,
    RestoreReplacePolicy restoreReplacePolicy);
bool IsSpecialDeviceFile(mode_t mode);

#endif // LIBNFS_DIRECTORY_DELETE_REQUEST_H