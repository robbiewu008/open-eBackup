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
#ifndef CHANNELMANAGER_H
#define CHANNELMANAGER_H

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <string>
#include <cstddef>
namespace FilePlugin {
class ChannelManager {
public:
    static ChannelManager& getInstance();
    ChannelManager(const ChannelManager&) = delete;
    ChannelManager& operator=(const ChannelManager&) = delete;

    // 添加subJobId到指定的jobId
    // 返回true表示成功，false表示达到上限（busy）
    bool addSubJob(const std::string& jobId, const std::string& subJobId, const std::string& numOfChannels);

    // 移除subJobId
    void removeSubJob(const std::string& jobId, const std::string& subJobId);

    void printSubJobs(const std::string& jobId);

    int getSubJobCount(const std::string& jobId);

private:
    ChannelManager();

    // 存储每个jobId对应的subJobId集合
    std::unordered_map<std::string, std::unordered_set<std::string>> m_subJobs;

    std::mutex m_mutex;
};
}
#endif // CHANNELMANAGER_H
