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
#ifndef FS_SCANNER_TASK_INFO_STRUCTS_H
#define FS_SCANNER_TASK_INFO_STRUCTS_H
#include <iostream>
#include <string>
#include <mutex>
#include <unordered_map>

class ScanTaskInfo {
public:
    static ScanTaskInfo& GetInstance()
    {
        static ScanTaskInfo inst;
        return inst;
    }
    void Insert(std::string jobId, std::string Path)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        std::pair<std::string, std::string> confInfo {jobId, Path};
        if (m_confInfoMap.find(jobId) == m_confInfoMap.end()) {
            m_confInfoMap.insert(confInfo);
        }
    }
    std::string Query(std::string jobId)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        if (m_confInfoMap.find(jobId) == m_confInfoMap.end()) {
            return std::string("");
        }
        return m_confInfoMap[jobId];
    }
    void Delete(std::string jobId)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        auto iter = m_confInfoMap.find(jobId);
        if (iter != m_confInfoMap.end()) {
            m_confInfoMap.erase(iter);
        }
    }

private:
    ScanTaskInfo() {}
    ~ScanTaskInfo() {}
    std::mutex m_mutex;
    std::unordered_map<std::string, std::string> m_confInfoMap;
};
#endif