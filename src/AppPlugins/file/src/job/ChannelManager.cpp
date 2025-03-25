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
#include "ChannelManager.h"
#include <iostream>
#include <sstream>
#include "log/Log.h"
using namespace std;
using namespace FilePlugin;
size_t DEFAULT_CHANNEL_NUM = 1;
namespace FilePlugin {

ChannelManager& ChannelManager::getInstance()
{
    static ChannelManager instance;
    return instance;
}

ChannelManager::ChannelManager() = default;

// add subJobId to jobId
bool ChannelManager::addSubJob(const std::string& jobId, const std::string& subJobId, const std::string& numOfChannels)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t limit;
    stringstream sstream(numOfChannels);
    sstream >> limit;

    auto& subJobs = m_subJobs[jobId];//  subjob set of jobId
    size_t currentSize = subJobs.size();

    // 检查是否已经存在
    if (subJobs.find(subJobId) != subJobs.end()) {
        INFOLOG("SubJob ID: %s already exists in Job ID: %s.", subJobId.c_str(), jobId.c_str());
        return true;// 已存在，不需要添加
    }

    if (currentSize < limit) {
        subJobs.insert(subJobId);
        return true;
    }
    return false;
}

// 移除subJobId
void ChannelManager::removeSubJob(const std::string& jobId, const std::string& subJobId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_subJobs.find(jobId);
    if (it != m_subJobs.end()) {
        size_t erasedCount = it->second.erase(subJobId);
        // 如果子任务集合为空，删除jobId
        if (it->second.empty()) {
            m_subJobs.erase(it);
            INFOLOG("Job ID: %s has no SubJob IDs, remove it.", jobId.c_str());
        }
    }
}

int ChannelManager::getSubJobCount(const std::string& jobId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_subJobs.find(jobId);
    if (it != m_subJobs.end()) {
        return it->second.size();
    }
    return 0; // jobId not found
}

void ChannelManager::printSubJobs(const std::string& jobId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_subJobs.find(jobId);
    if (it != m_subJobs.end()) {
        const auto& subJobs = it->second;
        if (!subJobs.empty()) {
            INFOLOG("Job ID: %s has the following SubJob IDs:", jobId.c_str());
            for (const auto& subJobId : subJobs) {
                INFOLOG(" subJobId - %s", subJobId.c_str());
            }
        } else {
            INFOLOG("Job ID: %s has no SubJob IDs.", jobId.c_str());
        }
    } else {
        INFOLOG("Job ID: %s not found.", jobId.c_str());
    }
}

}