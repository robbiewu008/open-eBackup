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
#ifndef CNWARE_QUERY_TASK_RESPONSE_H
#define CNWARE_QUERY_TASK_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {

struct CNWareTaskResponse {
    std::string m_id;
    std::string m_name;
    std::string m_hostId;
    int32_t m_status = 0;
    std::string m_stepDesc;
    std::string m_targetName;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostId, hostId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_stepDesc, stepDesc);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_targetName, targetName);
    END_SERIAL_MEMEBER;
};

class QueryTaskResponse : public VirtPlugin::ResponseModel {
public:
    QueryTaskResponse() {}
    ~QueryTaskResponse() {}

    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_cwTaskInfo)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

    int32_t GetTaskStatus()
    {
        return m_cwTaskInfo.m_status;
    }

    std::string GetTaskStepDesc()
    {
        return m_cwTaskInfo.m_stepDesc;
    }

    CNWareTaskResponse GetInfo()
    {
        return m_cwTaskInfo;
    }

private:
    CNWareTaskResponse m_cwTaskInfo;
};
};

#endif