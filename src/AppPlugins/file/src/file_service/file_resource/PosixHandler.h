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
#ifndef APPPLUGIN_NAS_POSIXHANDLER_H
#define APPPLUGIN_NAS_POSIXHANDLER_H

#include "ApplicationManager.h"

namespace FilePlugin {

class PosixHandler : public ApplicationManager {
public:
    PosixHandler() = default;

    virtual ~PosixHandler() noexcept {}

    void ListNativeResource(
        FilePlugin::FileResourceInfo& resourceInfo,
        const FilePlugin::ListResourceParam& listResourceParam) override;
 
    void ListAggregateResource(
        FilePlugin::FileResourceInfo& resourceInfo,
        const FilePlugin::ListResourceParam& listResourceParam) override;
 
    void ListVolumeResource(
        FilePlugin::FileResourceInfo& resourceInfo,
        const FilePlugin::ListResourceParam& listResourceParam) override;

private:
    std::string NormalizeDirectoryPath(std::string path);

    void BrowseFolderByPage(
        FilePlugin::FileResourceInfo& resourceInfo,
        const std::string& parentPath,
        int pageNo,
        int pageSize);
};

}
#endif // APPPLUGIN_NAS_POSIXHANDLER_H
