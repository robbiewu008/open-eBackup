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
#ifndef LIBNFS_READ_LINK_REQUEST_H
#define LIBNFS_READ_LINK_REQUEST_H

#include "LibnfsCommonMethods.h"
#include "BlockBufferMap.h"

class NfsReadlinkCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *commonData;
    std::shared_ptr<BlockBufferMap> blockBufferMap;
};

NfsReadlinkCbData* CreateReadlinkCbData(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap);
int SendReadlink(FileHandle &fileHandle, NfsReadlinkCbData *cbData);
void SendReadlinkCb(int status, struct nfs_context *nfs, void *data, void *privateData);
int HandleReadlinkSuccess(NfsReadlinkCbData *cbData, FileHandle &fileHandle, void *data);
void HandleReadlinkFailure(NfsCommonData *commonData, int status, FileHandle &fileHandle);

#endif // LIBNFS_READ_LINK_REQUEST_H