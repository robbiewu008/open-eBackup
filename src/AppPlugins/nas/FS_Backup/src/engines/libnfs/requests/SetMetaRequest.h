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
#ifndef LIBNFS_SET_META_REQUEST_H
#define LIBNFS_SET_META_REQUEST_H

#include "CopyCtrlParser.h"
#include "LibnfsCommonMethods.h"
#include "LibnfsInterface.h"

class NfsSetMetaCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *writeCommonData;
};

NfsSetMetaCbData* CreateSetMetaCbData(FileHandle &fileHandle, NfsCommonData &commonData);
int SendSetMeta(FileHandle &fileHandle, NfsSetMetaCbData *cbData);
void SendSetMetaCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void MetaModifiedCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void HandleSetMetaFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status, struct nfs_context *nfs);
void SetMetaFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle);
int SendDstCloseFile(FileHandle &fileHandle, NfsCommonData *commonData);

#endif // LIBNFS_SET_META_REQUEST_H