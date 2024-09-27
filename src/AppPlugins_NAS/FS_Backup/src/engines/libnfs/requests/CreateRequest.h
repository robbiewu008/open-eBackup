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
#ifndef LIBNFS_CREATE_REQUEST_H
#define LIBNFS_CREATE_REQUEST_H

#include "LibnfsCommonMethods.h"
#include "LibnfsInterface.h"

class NfsCreateCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *writeCommonData;
    struct nfsfh *nfsfh;
    uint32_t openFlag = O_WRONLY | O_CREAT;
    RestoreReplacePolicy restoreReplacePolicy { RestoreReplacePolicy::NONE };
};

NfsCreateCbData* CreateCreateCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh,
    uint32_t openFlag, RestoreReplacePolicy restoreReplacePolicy);
int SendCreate(FileHandle &fileHandle, NfsCreateCbData *cbData);
void SendCreateCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void HandleCreateFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status, struct nfs_context *nfs);
void CreateFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle);
int SendLstatFromCreate(FileHandle &fileHandle, NfsCommonData *commonData, struct nfsfh *nfsfh,
    RestoreReplacePolicy restoreReplacePolicy);

#endif // LIBNFS_CREATE_REQUEST_H
