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
#include "taskmanager/filter/RemoteHostFilterFactory.h"
#include "taskmanager/filter/DefaultRemoteHostFilter.h"

RemoteHostFilterFactory::RemoteHostFilterFactory()
{}

RemoteHostFilterFactory::~RemoteHostFilterFactory()
{}

std::shared_ptr<RemoteHostFilter> RemoteHostFilterFactory::CreateFilter(FilterType type)
{
    if (type == PORT_TYPE_FILTER) {
        return std::make_shared<DefaultRemoteHostFilter>();
    }
    return nullptr;
}

FilterAction RemoteHostFilterFactory::CreateFilterAction(FilterType type)
{
    FilterAction filterAction = [type](PluginJobData& jobData, mp_bool isInner) -> mp_int32 {
        return CreateFilter(type)->DoFilter(jobData, isInner);
    };
    return filterAction;
}