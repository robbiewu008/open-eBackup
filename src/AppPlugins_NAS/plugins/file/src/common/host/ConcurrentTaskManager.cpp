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
#include "log/Log.h"
#include "config_reader/ConfigIniReader.h"
#include "ConcurrentTaskManager.h"

using namespace std;
using namespace FilePlugin;

namespace {
    const std::string FILE_PLUGIN_CONFIG_KEY = "FilePluginConfig";
}

ConcurrentTaskManager& ConcurrentTaskManager::GetConcurrentTaskManager()
{
    static ConcurrentTaskManager instance;
    return instance;
}

bool ConcurrentTaskManager::AccquireLock(const std::string& taskID)
{
    std::lock_guard<std::mutex> lock(m_lock);
    if (std::find(m_runningTasks.begin(), m_runningTasks.end(), taskID) != m_runningTasks.end()) {
        INFOLOG("%s is already guaranteed to running!", taskID.c_str());
        return true;
    }
    if (m_idleCount > 0) {
        m_idleCount--;
        m_runningTasks.push_back(taskID);
        INFOLOG("%s take an idle slot! %d/%d remain", taskID.c_str(), m_idleCount, m_maxConcurrent);
        if (std::find(m_pendingTasks.begin(), m_pendingTasks.end(), taskID) != m_pendingTasks.end()) {
            m_pendingTasks.erase(std::find(m_pendingTasks.begin(), m_pendingTasks.end(), taskID));
        }
        PrintTaskList();
        return true;
    }
    DBGLOG("%s try accqure lock failed, 0/%d remain", taskID.c_str(), m_maxConcurrent);
    if (std::find(m_pendingTasks.begin(), m_pendingTasks.end(), taskID) == m_pendingTasks.end()) {
        m_pendingTasks.push_back(taskID);
    }
    return false;
}

bool ConcurrentTaskManager::ReleaseLock(const std::string& taskID)
{
    std::lock_guard<std::mutex> lock(m_lock);
    if (std::find(m_pendingTasks.begin(), m_pendingTasks.end(), taskID) != m_pendingTasks.end()) {
        m_pendingTasks.erase(std::find(m_pendingTasks.begin(), m_pendingTasks.end(), taskID));
    }
    if (std::find(m_runningTasks.begin(), m_runningTasks.end(), taskID) == m_runningTasks.end()) {
        ERRLOG("erase %s, but is not running!", taskID.c_str());
        return false;
    }
    m_runningTasks.erase(std::find(m_runningTasks.begin(), m_runningTasks.end(), taskID));
    m_idleCount++;
    INFOLOG("%s release an idle slot! %d/%d remain", taskID.c_str(), m_idleCount, m_maxConcurrent);
    PrintTaskList();
    return true;
}

void ConcurrentTaskManager::PrintTaskList() const
{
    std::vector<std::string> m_runningTasks;
    std::string message = "running task: ";
    for (const auto& taskID : m_runningTasks) {
        message += taskID + ",";
    }
    message += "  pending task: ";
    for (const auto& taskID : m_pendingTasks) {
        message += taskID + ",";
    }
    INFOLOG("%s", message.c_str());
}

ConcurrentTaskManager::ConcurrentTaskManager()
{
    m_maxConcurrent = Module::ConfigReader::getInt(FILE_PLUGIN_CONFIG_KEY, "SCAN_CONCURRENT_COUNT");
    INFOLOG("init ConcurrentTaskManager, maxConcurrent = %d", m_maxConcurrent);
    m_idleCount = m_maxConcurrent;
}
