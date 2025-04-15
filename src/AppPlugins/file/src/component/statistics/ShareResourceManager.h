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
#ifndef APPPLUGIN_FILE_SHARERESOURCEMANAGER_H
#define APPPLUGIN_FILE_SHARERESOURCEMANAGER_H
#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>
#include <boost/filesystem.hpp>
#include "Semaphore.h"
#include "JsonFileTool.h"
#include "utils/PluginUtilities.h"

namespace {
    const int RETRY_TIMES = 3;
};
namespace FilePlugin {
enum class ShareResourceType {
    NONE = -1,
    BACKUP = 0,
    SCAN = 1,
    GENERAL = 2,
    SNAPSHOT = 3
};

class ShareResourceManager {
public:
    static ShareResourceManager& GetInstance();
    void SetResourcePath(const std::string &path, const std::string &jobId);
    void Wait(ShareResourceType resType, const std::string &jobId);
    void Signal(ShareResourceType resType, const std::string &jobId);
    bool DeleteResource(ShareResourceType resType, const std::string &jobId);
    /* 该函数仅用于删除在ShareResourceMap中保存的Semaphore和ResourcePath信息，不删除文件 */
    void RemoveResourceCache(ShareResourceType resType, const std::string &jobId);
    bool CanReportSpeedToPM(const std::string mainTaskId, const uint32_t& interval);
    bool CanReportStatToPM(const std::string& mainTaskId);
    bool CanPrintBackupStats(const std::string& mainTaskId);
    template <class T>
    bool UpdateResource(ShareResourceType resType, const std::string& jobId, T& resourceInfo)
    {
        std::string fileName = GetFileName(resType, jobId);
        int times = 0;
        while (times < RETRY_TIMES) {
            if (JsonFileTool::WriteToFile(resourceInfo, fileName)) {
                return true;
            }
            ++times;
        }
        return false;
    }

    template <class T>
    bool InitResource(ShareResourceType resType, const std::string &jobId, T& resourceInfo)
    {
        std::string resourcePath = GetJobResourcePath(jobId);
        PluginUtils::CreateDirectory(resourcePath);
        std::string fileName = GetFileName(resType, jobId);
        Wait(resType, jobId);
        bool ret = JsonFileTool::WriteToFile(resourceInfo, fileName);
        Signal(resType, jobId);
        return ret;
    }

    template <class T>
    bool QueryResource(ShareResourceType resType, const std::string &jobId, T& resourceInfo)
    {
        std::string fileName = GetFileName(resType, jobId);
        if (!PluginUtils::IsFileExist(fileName)) {
            ERRLOG("QueryResource failed, file: [ %s ] doesn't exist", fileName.c_str());
            return false;
        }
        bool ret = JsonFileTool::ReadFromFile(fileName, resourceInfo);
        return ret;
    }

    template <class T>
    bool QueryResource(const std::string& fileName, T& resourceInfo) const
    {
        if (!PluginUtils::IsFileExist(fileName)) {
            ERRLOG("QueryResource failed, file: [ %s ] doesn't exist", fileName.c_str());
            return false;
        }
        bool ret = JsonFileTool::ReadFromFile(fileName, resourceInfo);
        return ret;
    }

    void IncreaseRunningSubTasks(const std::string& mainTaskId);
    void DecreaseRunningSubTasks(const std::string& mainTaskId);
    uint8_t QueryRunningSubTasks(const std::string& mainTaskId);

    std::string GetFileName(ShareResourceType resType, const std::string &jobId);
    std::string GetJobResourcePath(const std::string &jobId);

private:
    ShareResourceManager();
    ~ShareResourceManager();
    std::string Calculatekey(ShareResourceType resType, const std::string &jobId) const;
    std::shared_ptr<std::mutex> GetMutex(const std::string& key);
    void RemoveMutex(const std::string& key);
    void RemoveJobResourcePath(const std::string &jobId);
private:
    std::unordered_map<std::string, std::shared_ptr<std::mutex>> m_ResMutexMap;
    std::mutex m_mutex;
    std::mutex m_taskMutex;
    std::mutex m_subTasksMutex;
    std::mutex m_cachePathMutex;
    std::unordered_map<std::string, time_t> m_TaskReportMap; // mainJobId : lastReportTime
    std::unordered_map<std::string, std::string> m_jobCachePathMap; // JobId : ResourcePath
    std::unordered_map<std::string, time_t> m_taskPrintBackupStatsMap; // mainJobId : lastPrintBackupStatsTime
    std::unordered_map<std::string, std::atomic<uint8_t>> m_noOfRuningSubTasks; // mainJobId : noOfRuningSubTasks
};
}

#endif // APPPLUGIN_FILE_SHARERESOURCEMANAGER_H