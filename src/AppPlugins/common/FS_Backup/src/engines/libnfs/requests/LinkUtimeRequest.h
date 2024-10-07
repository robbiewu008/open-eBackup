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
#ifndef LIBNFS_LINK_UTIME_REQUEST_H
#define LIBNFS_LINK_UTIME_REQUEST_H

#include "CopyCtrlParser.h"
#include "LibnfsCommonMethods.h"

class NfsLinkUtimeCbData {
public:
    FileHandle fileHandle {};
    NfsCommonData *writeCommonData;
};

NfsLinkUtimeCbData* CreateLinkUtimeCbData(FileHandle &fileHandle, NfsCommonData &commonData);
int SendLinkUtime(FileHandle &fileHandle, NfsLinkUtimeCbData *cbData);
void SendLinkUtimeCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void HandleLinkUtimeFailure(NfsCommonData *commonData, int status, FileHandle &fileHandle,
    struct nfs_context *nfs);
void LinkUtimeFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle);

#endif // LIBNFS_LINK_UTIME_REQUEST_H
