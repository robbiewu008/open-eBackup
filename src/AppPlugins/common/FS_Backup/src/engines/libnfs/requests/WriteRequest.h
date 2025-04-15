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
#ifndef LIBNFS_WRITE_REQUEST_H
#define LIBNFS_WRITE_REQUEST_H

#include "BlockBufferMap.h"
#include "LibnfsCommonMethods.h"
#include "LibnfsInterface.h"

class NfsWriteCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *writeCommonData;
    std::shared_ptr<BlockBufferMap> blockBufferMap;
};

NfsWriteCbData* CreateWriteCbData(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap);
int SendWrite(FileHandle &fileHandle, NfsWriteCbData *cbData);
void SendWriteCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void HandlePartlySuccess(NfsWriteCbData *cbData, FileHandle &fileHandle, int status, struct nfs_context *nfs);
void HandleWriteFailure(NfsWriteCbData *cbData, FileHandle &fileHandle, int status, struct nfs_context *nfs);
void WriteFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle);
void SendFileSyncCb(int status, struct nfs_context *nfs, void *data, void *privateData);
int SendSetMeta(FileHandle &fileHandle, NfsCommonData *commonData);
int SendCloseForAggregateFile(FileHandle &fileHandle, NfsCommonData *commonData);

#endif // LIBNFS_WRITE_REQUEST_H
