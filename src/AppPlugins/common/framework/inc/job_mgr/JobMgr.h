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
#ifndef NEWFRAMEWORKTEST_JOBMGR_H
#define NEWFRAMEWORKTEST_JOBMGR_H

#include <map>
#include <atomic>
#include <mutex>
#include "PluginTypes.h"
#include "BasicJob.h"

namespace common {
namespace jobmanager {
#ifdef WIN32
#ifdef TEST_DLL
#define OPENDLL_API __declspec(dllexport)
#else
#define OPENDLL_API __declspec(dllimport)
#endif
class OPENDLL_API JobMgr {
#else
class JobMgr {
#endif
public:
    JobMgr(const JobMgr&) = delete;
    JobMgr& operator=(const JobMgr&) = delete;

    static JobMgr& GetInstance();

    bool CheckJobIdExist(const std::string &jobId);
    int InsertJob(std::string jobId, std::shared_ptr<BasicJob> jobPtr);
    int AsyncAbortJob(const std::string& jobId, const std::string& subJobId);
    int PauseJob(const std::string& jobId, const std::string& subJobId);
    void PauseAllJob();
    void EraseFinishJob();
    void EndJobMonitor();
    int StartMonitorJob();
    void SetMonitorInterval(int i);

private:
    JobMgr();
    ~JobMgr();
    std::unique_ptr<std::thread> m_monitorJobMapThread {nullptr};
    int m_monitorInterval {0};
    std::atomic<bool> m_isMonitoring {true};
    std::map<std::string, std::shared_ptr<BasicJob>> m_jobIdMap {};
    std::mutex m_mtx;
    static JobMgr instance;
};
}
}
#endif  // NEWFRAMEWORKTEST_JOBMGR_H