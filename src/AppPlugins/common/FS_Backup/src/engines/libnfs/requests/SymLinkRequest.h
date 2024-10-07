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
#ifndef LIBNFS_SYMLINK_REQUEST_H
#define LIBNFS_SYMLINK_REQUEST_H

#include "LibnfsCopyWriter.h"
#include "BlockBufferMap.h"

class NfsSymLinkCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *writeCommonData;
    struct nfsfh *nfsfh;
    std::shared_ptr<BlockBufferMap> blockBufferMap;
};

NfsSymLinkCbData* CreateSymLinkCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh,
    std::shared_ptr<BlockBufferMap> blockBufferMap);
int SendSymLink(FileHandle &fileHandle, NfsSymLinkCbData *cbData);
void SendSymLinkCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void HandleSymLinkFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status,
    struct nfs_context *nfs);
void SymlinkFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle);

#endif // LIBNFS_SYMLINK_REQUEST_H