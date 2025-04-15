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
#ifndef FILTER_DEFAULT_REMOTE_HOST_FILTER_H
#define FILTER_DEFAULT_REMOTE_HOST_FILTER_H

#include <map>
#include <vector>
#include "taskmanager/filter/RemoteHostFilter.h"
#include "common/JsonHelper.h"
#include "common/Log.h"

class DefaultRemoteHostFilter : public RemoteHostFilter {
public:
    DefaultRemoteHostFilter();
    ~DefaultRemoteHostFilter();
    mp_int32 DoFilter(PluginJobData& jobData, mp_bool isInner) override;
private:
    mp_int32 GetLANType(Json::Value& jobParam, mp_int32& lanType);
    mp_void Transfer2ByPortType(const std::vector<mp_int32>& portTypes,
                                std::vector<Json::Value>& remoteHost,
                                std::vector<Json::Value>& filteredRemoteHost);
    mp_void UpdateReposParam(const std::vector<mp_int32>& portTypes,
                             Json::Value& jobParam, std::map<Json::ArrayIndex,
                             std::vector<Json::Value>> jsonReposMap);
};
#endif
