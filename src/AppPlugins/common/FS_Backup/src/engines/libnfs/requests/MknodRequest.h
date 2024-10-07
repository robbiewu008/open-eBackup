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
#ifndef LIBNFS_MKNOD_REQUEST_H
#define LIBNFS_MKNOD_REQUEST_H

#include "LibnfsCopyWriter.h"

class NfsMknodCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *writeCommonData;
    struct nfsfh *nfsfh;
};

NfsMknodCbData* CreateMknodCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh);
int SendMknod(FileHandle &fileHandle, NfsMknodCbData *cbData);
void SendMknodCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void HandleMknodSuccess(NfsCommonData *commonData, FileHandle &fileHandle, void *data, struct nfs_context *nfs);
void HandleMknodFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status, struct nfs_context *nfs);
void MknodFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle);

#endif // LIBNFS_MKNOD_REQUEST_H
