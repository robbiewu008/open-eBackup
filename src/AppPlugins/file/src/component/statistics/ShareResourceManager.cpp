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
#include "ShareResourceManager.h"
#include <fstream>
#include "Module/src/common/JsonHelper.h"
#include "constant/PluginConstants.h"
#include "utils/PluginUtilities.h"

using namespace std;
namespace FilePlugin {
namespace {
    constexpr auto MODULE = "ShareResourceManager";
    constexpr auto BACKUP_KEY_SUFFIX = "_backup_stats";
    constexpr auto SCAN_KEY_SUFFIX = "_scan_stats";
    constexpr auto GENERAL_KEY_SUFFIX = "_general_stats";
    const string SNAPSHOT_KEY_SUFFIX = "_snapshot_stats";
}
ShareResourceManager::ShareResourceManager()
{}

ShareResourceManager::~ShareResourceManager()
{
}

void ShareResourceManager::SetResourcePath(const std::string &path, const std::string &jobId)
{
    lock_guard<std::mutex> lk(m_cachePathMutex);
    auto it = m_jobCachePathMap.begin();
    it = m_jobCachePathMap.find(jobId);
    if (it != m_jobCachePathMap.end()) {
        return;
    }
    m_jobCachePathMap.emplace(jobId, path);
}

std::string ShareResourceManager::Calculatekey(ShareResourceType resType, const std::string &jobId) const
{
    if (resType == ShareResourceType::BACKUP) {
        return jobId + BACKUP_KEY_SUFFIX;
    } else if (resType == ShareResourceType::SCAN) {
        return jobId + SCAN_KEY_SUFFIX;
    } else if (resType == ShareResourceType::SNAPSHOT) {
        return jobId + SNAPSHOT_KEY_SUFFIX;
    }
    return "";
}

std::string ShareResourceManager::GetFileName(ShareResourceType resType, const std::string &jobId)
{
    string path = PluginUtils::PathJoin(GetJobResourcePath(jobId), jobId);
    if (resType == ShareResourceType::BACKUP) {
        path.append(BACKUP_KEY_SUFFIX).append(".json");
    } else if (resType == ShareResourceType::SCAN) {
        path.append(SCAN_KEY_SUFFIX).append(".json");
    } else if (resType == ShareResourceType::GENERAL) {
        path.append(GENERAL_KEY_SUFFIX).append(".json");
    } else if (resType == ShareResourceType::SNAPSHOT) {
        path.append(SNAPSHOT_KEY_SUFFIX).append(".json");
    }
    path = PluginUtils::ReverseSlash(path);
    DBGLOG("After reverse: %s", path.c_str());
    return path;
}

void ShareResourceManager::Wait(ShareResourceType resType, const std::string &jobId)
{
    string mutexKey = Calculatekey(resType, jobId);
    shared_ptr<std::mutex> mutexPtr = GetMutex(mutexKey);
    mutexPtr->lock();
}

void ShareResourceManager::Signal(ShareResourceType resType, const std::string &jobId)
{
    string mutexKey = Calculatekey(resType, jobId);
    shared_ptr<std::mutex> mutexPtr = GetMutex(mutexKey);
    mutexPtr->unlock();
}

bool ShareResourceManager::DeleteResource(ShareResourceType resType, const std::string &jobId)
{
    string fileName = GetFileName(resType, jobId);
    RemoveResourceCache(resType, jobId);
    bool result = PluginUtils::Remove(fileName);
    if (!result) {
        HCP_Log(ERR, MODULE) << "DeleteResource, delete file failed, path:"<< fileName << HCPENDLOG;
    }
    return result;
}

void ShareResourceManager::RemoveResourceCache(ShareResourceType resType, const std::string &jobId)
{
    string key = Calculatekey(resType, jobId);
    RemoveMutex(key);
    RemoveJobResourcePath(jobId);
}

bool ShareResourceManager::CanReportStatToPM(const std::string& mainTaskId)
{
    lock_guard<std::mutex> lk(m_taskMutex);
    time_t lastUpdateTime = 0;
    if (m_TaskReportMap.count(mainTaskId)) {
        lastUpdateTime = m_TaskReportMap[mainTaskId];
    } else {
        m_TaskReportMap[mainTaskId] = PluginUtils::GetCurrentTimeInSeconds();
        return false;
    }
    if ((PluginUtils::GetCurrentTimeInSeconds() - lastUpdateTime) < BACKUP_REPORT_CIRCLE_TIME_IN_SEC) {
        return false;
    }
    m_TaskReportMap[mainTaskId] = PluginUtils::GetCurrentTimeInSeconds();
    return true;
}

bool ShareResourceManager::CanPrintBackupStats(const std::string& mainTaskId)
{
    lock_guard<std::mutex> lk(m_subTasksMutex);
    time_t lastUpdateTime = 0;
    if (m_taskPrintBackupStatsMap.count(mainTaskId)) {
        lastUpdateTime = m_taskPrintBackupStatsMap[mainTaskId];
    } else {
        m_taskPrintBackupStatsMap[mainTaskId] = PluginUtils::GetCurrentTimeInSeconds();
        return false;
    }
    if ((PluginUtils::GetCurrentTimeInSeconds() - lastUpdateTime) < BACKUP_PRINT_CIRCLE_TIME_IN_SEC) {
        return false;
    }
    m_taskPrintBackupStatsMap[mainTaskId] = PluginUtils::GetCurrentTimeInSeconds();
    return true;
}

bool ShareResourceManager::CanReportSpeedToPM(const std::string mainTaskId, const uint32_t& interval)
{
    lock_guard<std::mutex> lk(m_taskMutex);
    time_t lastUpdateTime = 0;
    if (m_TaskReportMap.count(mainTaskId)) {
        lastUpdateTime = m_TaskReportMap[mainTaskId];
    }
    if ((PluginUtils::GetCurrentTimeInSeconds() - lastUpdateTime) < interval) {
        return false;
    }
    m_TaskReportMap[mainTaskId] = PluginUtils::GetCurrentTimeInSeconds();
    return true;
}

shared_ptr<std::mutex> ShareResourceManager::GetMutex(const std::string& key)
{
    lock_guard<std::mutex> lk(m_mutex);
    auto it = m_ResMutexMap.begin();
    it = m_ResMutexMap.find(key);
    if (it != m_ResMutexMap.end()) {
        return it->second;
    }
    shared_ptr<std::mutex> mutexPtr = make_shared<std::mutex>();
    m_ResMutexMap.emplace(key, mutexPtr);
    return mutexPtr;
}
void ShareResourceManager::RemoveMutex(const std::string& key)
{
    lock_guard<std::mutex> lk(m_mutex);
    auto it = m_ResMutexMap.begin();
    it = m_ResMutexMap.find(key);
    if (it != m_ResMutexMap.end()) {
        m_ResMutexMap.erase(it);
    }
}

void ShareResourceManager::RemoveJobResourcePath(const std::string& jobId)
{
    lock_guard<std::mutex> lk(m_cachePathMutex);
    auto it = m_jobCachePathMap.begin();
    it = m_jobCachePathMap.find(jobId);
    if (it != m_jobCachePathMap.end()) {
        m_jobCachePathMap.erase(it);
    }
}

std::string ShareResourceManager::GetJobResourcePath(const std::string &jobId)
{
    lock_guard<std::mutex> lk(m_cachePathMutex);
    auto it = m_jobCachePathMap.begin();
    it = m_jobCachePathMap.find(jobId);
    if (it != m_jobCachePathMap.end()) {
        return it->second;
    }
    WARNLOG("job resource path is empty,jobId:%s", jobId.c_str());
    return "";
}
void ShareResourceManager::IncreaseRunningSubTasks(const std::string& mainTaskId)
{
    if (m_noOfRuningSubTasks.count(mainTaskId)) {
        ++m_noOfRuningSubTasks[mainTaskId];
    } else {
        m_noOfRuningSubTasks[mainTaskId] = 1;
    }
}
void ShareResourceManager::DecreaseRunningSubTasks(const std::string& mainTaskId)
{
    if (m_noOfRuningSubTasks.count(mainTaskId) && m_noOfRuningSubTasks[mainTaskId] > 0) {
        --m_noOfRuningSubTasks[mainTaskId];
    } else {
        m_noOfRuningSubTasks[mainTaskId] = 0;
    }
}
uint8_t ShareResourceManager::QueryRunningSubTasks(const std::string& mainTaskId)
{
    if (m_noOfRuningSubTasks.count(mainTaskId)) {
        return m_noOfRuningSubTasks[mainTaskId].load();
    }
    return 0;
}
ShareResourceManager& ShareResourceManager::GetInstance()
{
    static ShareResourceManager instance;
    return instance;
}
}
