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
#include "RepositoryFactory.h"
#include <repository_handlers/filesystem/FileSystemHandler.h>
#ifdef WIN32
#include <repository_handlers/win32filesystem/Win32FileSystemHandler.h>
#endif

namespace VirtPlugin {
std::shared_ptr<RepositoryHandler> RepositoryFactory::CreateRepositoryHandler(
    const AppProtect::StorageRepository &storageRepo)
{
#ifdef WIN32
    return std::make_shared<Win32FileSystemHandler>();
#else
    switch (storageRepo.protocol) {
        case RepositoryProtocolType::CIFS:
        case RepositoryProtocolType::NFS:
            return std::make_shared<FileSystemHandler>();
        default:
            return nullptr;
    }
#endif
}
}