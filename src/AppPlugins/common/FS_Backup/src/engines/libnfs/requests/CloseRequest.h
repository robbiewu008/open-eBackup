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
#ifndef LIBNFS_CLOSE_REQUEST_H
#define LIBNFS_CLOSE_REQUEST_H

#include "LibnfsCommonMethods.h"

class NfsCloseCbData {
public:
    FileHandle fileHandle {};
    BACKUP_DIRECTION direction {};
    NfsCommonData *commonData;
    struct nfsfh* nfsFh;
};

NfsCloseCbData* CreateCloseCbData(FileHandle &fileHandle, NfsCommonData &commonData, BACKUP_DIRECTION direction);
int SendClose(FileHandle &fileHandle, NfsCloseCbData *cbData);
void SendCloseCb(int status, struct nfs_context *nfs, void *data, void *privateData);
void ProcessCloseCompletion(FileHandle &fileHandle, NfsCommonData *commonData);
int SendCloseSync(FileHandle &fileHandle, NfsCloseCbData *cbData);

#endif // LIBNFS_CLOSE_REQUEST_H