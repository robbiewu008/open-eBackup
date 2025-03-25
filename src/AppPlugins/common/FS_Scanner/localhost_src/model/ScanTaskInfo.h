/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * @date 11/25/2022
 * @author s30021416
 * @brief
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