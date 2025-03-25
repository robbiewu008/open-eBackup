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
#ifndef LIB_SCANNER_H
#define LIB_SCANNER_H

#include <iostream>
#include <mutex>
#include <unordered_map>
#include "define/Defines.h"

class ScanTaskInfo {
public:
    static ScanTaskInfo& GetInstance()
    {
        static ScanTaskInfo inst;
        return inst;
    }

    void Insert(std::string jobId, std::string path)
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        std::pair<std::string, std::string> confInfo {jobId, path};
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

#ifdef __cplusplus
extern "C" {
AGENT_API typedef struct ScanConf_S {
    char *jobId;                    /* Job id */
    char *metaPath;                 /* Metadata path for cache files of nas share */
    char *metaPathForCtrlFiles;     /* Metadata path for control files of nas share */
    bool enableProduce = true;      /* won't produce meta and control file once disabled */
    uint64_t writeQueueSize = 1000; /* max write queue size */
} ScanConf;

AGENT_API typedef struct ScanStats_S {
    uint64_t openDirRequestCnt;
    uint64_t openDirTime;
    uint64_t totalDirs;
    uint64_t totalFiles;
    uint64_t totalSize;
    uint64_t totalFailedDirs;
    uint64_t totalFailedFiles;
    uint64_t totalControlFiles;
    uint64_t totalSizeToBackup;
    uint64_t totalDirsToBackup;
    uint64_t totalFilesToBackup;
} ScanStats;

/* use to init scanner log */
AGENT_API extern void InitLog(const char* fullLogPath, int logLevel);

/* use to create a scanner instance */
AGENT_API extern void* CreateScannerInst(ScanConf scanConf);

/* use to start a scanner task */
AGENT_API extern bool StartScanner(void *scannerHandle, const char *dirPath);

#ifdef WIN32
/* use to start a scanner task with no SplitCompoundPrefix*/
AGENT_API extern bool StartScannerWithoutSplitCompoundPrefix(void *scannerHandle, const char *dirPath);
#endif

/* use to start a scanner task with multiple files */
AGENT_API extern bool StartScannerBatch(void *scannerHandle, const char *dirPathList);

/* use to monitor a scanner task */
AGENT_API extern bool MonitorScanner(void *scannerHandle, const char *jobId);

/* use to get statistics of a scanner task */
AGENT_API extern ScanStats GetStatistics(void *scannerHandle);

/* use to destroy a scanner instance */
AGENT_API extern void DestroyScannerInst(void *scannerHandle);
};
#endif
#endif
