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
#include "ApplicationManager.h"
#include "PluginConstants.h"
using namespace AppProtect;
namespace FilePlugin {
void ApplicationManager::ListApplicationResource(ResourceResultByPage& returnValue,
                                                 const ListResourceParam& listResourceParam)
{
    std::string fileType = listResourceParam.resourceExtendInfo.fileType;
    FileResourceInfo resourceInfo;
    if (fileType == RESOURCE_NATIVE_FILE_TYPE) {
        ListNativeResource(resourceInfo, listResourceParam);
    } else if (fileType == RESOURCE_VOLUME_TYPE) {
        ListVolumeResource(resourceInfo, listResourceParam);
    } else {
        ListAggregateResource(resourceInfo, listResourceParam);
    }
    returnValue.pageNo = listResourceParam.pageNo;
    returnValue.pageSize = listResourceParam.pageSize;
    TransformResult(returnValue, resourceInfo);
}

void ApplicationManager::TransformResult(ResourceResultByPage& returnValue,
                                         const FileResourceInfo& resourceInfo)
{
    returnValue.total = resourceInfo.totalCount;
    for (auto resourceInfo : resourceInfo.resourceDetailVec) {
        ApplicationResource resource;
        resource.__set_id(std::to_string(returnValue.total)); // ?????
        std::string extendInfo;
        if (!Module::JsonHelper::StructToJsonString(resourceInfo, extendInfo)) {
            continue;
        }
        resource.__set_extendInfo(extendInfo);
        returnValue.items.push_back(resource);
    }
}
}
