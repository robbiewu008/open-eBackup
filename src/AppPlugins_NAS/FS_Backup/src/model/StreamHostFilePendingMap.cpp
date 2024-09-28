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
#include "StreamHostFilePendingMap.h"
#include "log/Log.h"

inline std::string StreamPath2HostFilePath(const std::string& streampath)
{
    std::string filepath;
    auto pos = streampath.rfind(":");
    if (pos == std::string::npos) {
        filepath = streampath;
    } else {
        filepath = streampath.substr(0, pos);
    }
    return filepath;
}

void StreamHostFilePendingMap::MarkHostWriteComplete(const std::string& filepath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    DBGLOG("MarkHostWriteComplete %s", filepath.c_str());
    auto it = m_map.find(filepath);
    if (it == m_map.end()) {
        m_map[filepath] = std::make_pair<bool, uint64_t>(true, 0);
    }
    it->second.first = true;
}

void StreamHostFilePendingMap::MarkHostWriteFailed(const std::string& filepath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    DBGLOG("MarkHostWriteFailed %s", filepath.c_str());
    m_failedAdsHost.insert(filepath);
}

void StreamHostFilePendingMap::IncStreamPending(const std::string& streampath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string filepath = StreamPath2HostFilePath(streampath);
    DBGLOG("IncStreamPending %s", filepath.c_str());
    auto it = m_map.find(filepath);
    if (it == m_map.end()) {
        m_map[filepath] = std::make_pair<bool, uint64_t>(false, 1);
        return;
    }
    ++(it->second.second);
}

void StreamHostFilePendingMap::DecStreamPending(const std::string& streampath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string filepath = StreamPath2HostFilePath(streampath);
    DBGLOG("DecStreamPending %s", filepath.c_str());
    auto it = m_map.find(filepath);
    if (it == m_map.end()) {
        ERRLOG("illegal operation (host not exists)");
        return; // illegal operation
    }
    if (!it->second.first) {
        ERRLOG("illegal operation (host not backuped)");
        return;
    }
    --(it->second.second);
    if (it->second.second == 0) {
        DBGLOG("%s has no pending stream, remove from map", filepath.c_str());
        m_map.erase(it);
    }
}

bool StreamHostFilePendingMap::IsHostWriteFailed(const std::string& streampath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string filepath = StreamPath2HostFilePath(streampath);
    auto it = m_failedAdsHost.find(filepath);
    if (it == m_failedAdsHost.end()) {
        DBGLOG("host file %s not failed backup", filepath.c_str());
        return false;
    }
    DBGLOG("host file %s has failed backup", filepath.c_str());
    return true;
}

bool StreamHostFilePendingMap::IsHostWriteComplete(const std::string& streampath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string filepath = StreamPath2HostFilePath(streampath);
    auto it = m_map.find(filepath);
    if (it == m_map.end() || !it->second.first) {
        DBGLOG("host file %s not completed backup yet", filepath.c_str());
        return false;
    }
    DBGLOG("host file %s has completed backup", filepath.c_str());
    return true;
}

uint64_t StreamHostFilePendingMap::PendingStreamNum(const std::string& streampath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string filepath = StreamPath2HostFilePath(streampath);
    auto it = m_map.find(filepath);
    if (it == m_map.end()) {
        return 0;
    }
    return it->second.second;
}
