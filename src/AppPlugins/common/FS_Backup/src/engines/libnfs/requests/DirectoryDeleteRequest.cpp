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
#include "DirectoryDeleteRequest.h"

using namespace std;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsDirDeleteCbData* CreateDirDeleteCbData(FileHandle &fileHandle, NfsCommonData &commonData)
{
    auto cbData = new(nothrow) NfsDirDeleteCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;

    return cbData;
}

int SendDirDelete(FileHandle &fileHandle, NfsDirDeleteCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    NfsCommonData *commonData = cbData->writeCommonData;
    delete cbData;
    cbData = nullptr;
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return MP_FAILED;
    }

    if (LibNfsDeleteDirectorySync(fileHandle.m_file->m_fileName, commonData->syncNfsContextContainer) != MP_SUCCESS) {
        RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER, true, true);
        return MP_FAILED;
    }

    fileHandle.m_retryCnt = 0;

    fileHandle.m_file->SetDstState(FileDescState::LSTAT);
    if (commonData->writeQueue != nullptr) {
        commonData->writeQueue->Push(fileHandle);
    }

    return MP_SUCCESS;
}
