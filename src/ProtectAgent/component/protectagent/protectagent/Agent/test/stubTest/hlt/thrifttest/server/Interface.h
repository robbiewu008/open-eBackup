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
#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "TaskService.h"
#include "MountService.h"

using namespace thrifttest;

class TaskServiceHandler : public TaskServiceIf {
public:
    TaskServiceHandler()
    {
        m_tasks.emplace(1, "Task1");
        m_tasks.emplace(2, "Task2");
        m_tasks.emplace(3, "Task3");
    }

    virtual void GetTask(Task& task, int64_t id) {
        auto it = m_tasks.find(id);
        if (it != m_tasks.end()) {
            task.jobID = it->first;
            task.info = it->second;
        }
    }
private:
    std::map<int64_t, std::string> m_tasks;
};

class MountServiceHandler : public MountServiceIf {
public:
    virtual int64_t Mount(const Config& config, const std::string& path)
    {
        if (m_paths.find(path) == m_paths.end()) {
            m_paths.emplace(path);
            return 1;
        }
        return 0;
    }
  virtual int64_t Unmount(const std::string& path) {
        auto it = m_paths.find(path);
        if (it != m_paths.end()) {
            m_paths.erase(it);
            return 1;
        }
        return 0;
  }
private:
    std::set<std::string> m_paths;
};
#endif