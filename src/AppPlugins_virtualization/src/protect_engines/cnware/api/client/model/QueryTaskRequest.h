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
#ifndef CNWARE_QUERY_TASK_REQUEST_H
#define CNWARE_QUERY_TASK_REQUEST_H

#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
enum class CNwareTaskStatus {
    CNWARE_TASK_BEFORE_START = 0,
    CNWARE_TASK_STARTED      = 1,
    CNWARE_TASK_CANCELED     = 2,
    CNWARE_TASK_SUCCESS      = 3,
    CNWARE_TASK_FAILED       = 4,
};

class QueryTaskRequest : public CNwareRequest {
public:
    QueryTaskRequest() {}
    ~QueryTaskRequest() {}

    void SetTaskId(const std::string &taskId)
    {
        m_taskId = taskId;
    }
 
    std::string GetTaskId()
    {
        return m_taskId;
    }
 
private:
    std::string m_taskId;
};
};

#endif