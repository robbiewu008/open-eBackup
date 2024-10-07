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
#ifndef HOST_MUTUAL_TASK_MANAGER_H
#define HOST_MUTUAL_TASK_MANAGER_H

#include <string>
#include <mutex>
#include <vector>
#include "define/Defines.h"

namespace FilePlugin {

const int DEFAULT_MAX_CONCURRENT_TASK = 1;

class ConcurrentTaskManager {
public:
    static ConcurrentTaskManager& GetConcurrentTaskManager();

    bool AccquireLock(const std::string& taskID);

    bool ReleaseLock(const std::string& taskID);
    
    void PrintTaskList() const;
    
private:
    ConcurrentTaskManager();

private:
    std::mutex m_lock {};

    int m_maxConcurrent { DEFAULT_MAX_CONCURRENT_TASK }; // immutable

    int m_idleCount { DEFAULT_MAX_CONCURRENT_TASK };

    std::vector<std::string> m_runningTasks;

    std::vector<std::string> m_pendingTasks;
};

}

#endif