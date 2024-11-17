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
#ifndef FILTER_REMOTE_HOST_FILTER_H
#define FILTER_REMOTE_HOST_FILTER_H

#include "common/Types.h"
#include "taskmanager/externaljob/Job.h"

using namespace AppProtect;

namespace JobParamKey {
    const mp_string APP_INFO = "appInfo";
    const mp_string EXTEND_INFO = "extendInfo";
    const mp_string LAN_TYPE = "lanType";
    const mp_string PORT_TYPE = "portType";
    const mp_string COPIES = "copies";
    const mp_string REPOSITORIES = "repositories";
    const mp_string REMOTE_HOST = "remoteHost";
}

class RemoteHostFilter {
public:
    virtual ~RemoteHostFilter() {}
    virtual mp_int32 DoFilter(PluginJobData& jobData, mp_bool isInner) = 0;
};
#endif
