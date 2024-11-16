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
#ifndef NEWFRAMEWORKTEST_BASICJOB_H
#define NEWFRAMEWORKTEST_BASICJOB_H

#include <thread>
#include <memory>
#include <atomic>
#include <iostream>
#include "ApplicationProtectFramework_types.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "define/Types.h"
#include "JobCommonInfo.h"

#ifdef WIN32
#include "define/Defines.h"
#endif

using namespace AppProtect;
constexpr uint32_t MAX_RETRY_CNT = 3;
#define REPORT_LOG2AGENT(subJobDetails, returnValue, logDetailList, logDetail, curProcess, curSpeed, curJobstatus) do { \
    subJobDetails.__set_jobId(m_parentJobId);                                                                          \
    if (m_subJobInfo != nullptr && m_subJobInfo->subJobId != "") {                                                     \
        subJobDetails.__set_subJobId(m_subJobInfo->subJobId);                                                          \
    }                                                                                                                  \
    if (logDetail.description != "") {                                                                                 \
        HCP_Log(DEBUG, "BasicJob") << "Current label is : " << logDetail.description << HCPENDLOG;                     \
        logDetailList.push_back(logDetail);                                                                            \
    }                                                                                                                  \
    subJobDetails.__set_jobStatus(curJobstatus);                                                                       \
    if ((curSpeed) != 0) {                                                                                             \
        subJobDetails.__set_speed((curSpeed));                                                                         \
    }                                                                                                                  \
    if ((curProcess) != 0) {                                                                                           \
        subJobDetails.__set_progress((curProcess));                                                                    \
    }                                                                                                                  \
    subJobDetails.__set_logDetail((logDetailList));                                                                    \
    HCP_Log(DEBUG, "BasicJob") << "report job : " << (subJobDetails.jobId) << " , "                                    \
        << (subJobDetails.subJobId)  << " , " << (logDetail.description)                                               \
        << " , " << curJobstatus << HCPENDLOG;                                                                         \
    uint16_t retry = 0;                                                                                                \
    do {                                                                                                               \
        JobService::ReportJobDetails((returnValue), (subJobDetails));                                                  \
    } while (                                                                                                          \
        curJobstatus == SubJobStatus::COMPLETED && returnValue.code != Module::SUCCESS && retry++ < MAX_RETRY_CNT);    \
    logDetailList.clear();                                                                                             \
    logDetail.__set_description("");                                                                                   \
} while (0)

#define ABORT_ENDTASK(subJobDetails, result, logDetailList, logDetail, process, speed)                                 \
    if (IsAbortJob()) {                                                                                                \
        HCP_Log(INFO, "BasicJob") << "Receive abort req, End Task" << HCPENDLOG;                                       \
        REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, process, speed, SubJobStatus::ABORTED);      \
        return Module::SUCCESS;                                                                                        \
    }

#ifdef WIN32
class AGENT_API BasicJob {
#else
class BasicJob {
#endif
public:
    BasicJob();
    virtual ~BasicJob();
    virtual int PrerequisiteJob();
    virtual int GenerateSubJob();
    virtual int ExecuteSubJob();
    virtual int PostJob();
    virtual int AbortJob();

    virtual void EndJob(AppProtect::SubJobStatus::type jobStatus);
    // info
    void SetJobInfo(std::shared_ptr<JobCommonInfo> info);
    std::shared_ptr<JobCommonInfo> GetJobInfo();
    void SetJobId(const std::string& jobId);
    std::string GetJobId();
    void SetParentJobId(const std::string& parentJobId);
    std::string GetParentJobId();
    void SetSubJob(std::shared_ptr<SubJob> subJob);
    std::string GetSubJobId();

    // thread
    void SetJobThread(std::shared_ptr<std::thread> th);
    std::shared_ptr<std::thread> GetJobThread();
    int DetachJobThread();                               // for JobExecution to invoke detach
    // status
    void SetJobStatus(AppProtect::SubJobStatus::type jobStatus);
    AppProtect::SubJobStatus::type GetJobStatus();
    bool IsAbortJob() const;                                  // check job is abort or not
    virtual void SetJobAborted();                 // set job to abort, the operation thread will return smoothly
    void SetJobToFinish();                        // set job to finish, for JobMgr to erase
    bool IsJobFinish();                                 // check job is finish
    void SetJobToPause();
    bool IsJobPause() const;
    // progress
    void SetProgress(int p);
    // others
    void SetPostJobResultType(AppProtect::JobResult::type type);
    int RunStateMachine();
protected:
    std::string m_jobId {};                             // current jobId
    std::string m_parentJobId {};                       // subjob's parent jobId (main jobId) uesd by ExecuteSubJob
    std::atomic<int> m_progress {0};                    // progress
    std::atomic<bool> m_isAbort {false};                // job is aborted by agent
    std::atomic<bool> m_isFinish {false};               // job finish & need to erase by JobMgr
    std::atomic<bool> m_isPause {false};                // job is pause by agent when plugin not report for a long time
    std::shared_ptr<std::thread> m_jobThread {nullptr}; // operation thread
    std::shared_ptr<JobCommonInfo> m_jobCommonInfo {nullptr};  // container different backup or restore job info
    std::shared_ptr<SubJob> m_subJobInfo {nullptr};            // subjob info used by ExecuteSubJob operation
    // job result shows whether job finish success until post job
    AppProtect::JobResult::type m_jobResult {AppProtect::JobResult::type::SUCCESS};

    int m_nextState = 0;
    std::map<int, std::function<int(void)>> m_stateHandles;

    void AddInfoParam(const std::string &p, std::vector<std::string> &logInfoParam)
    {
        logInfoParam.push_back(p);
        return;
    }

    template<typename... Args>
    void AddLogDetail(AppProtect::LogDetail &logDetail, const std::string& label,
        const AppProtect::JobLogLevel::type &level, Args... args)
    {
        logDetail.__set_description(label);
        logDetail.__set_level(level);
        std::vector<std::string> logInfoParams;
        int v[2048] = { (AddInfoParam(args, logInfoParams), 0)... };
        logDetail.__set_params(logInfoParams);
        logDetail.__set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        return;
    }

    template<typename... Args>
    void AddErrCode(AppProtect::LogDetail &logDetail, const int64_t errCode, Args... args)
    {
        if (errCode == 0) {
            return;
        }
        logDetail.__set_errorCode(errCode);
        std::vector<std::string> errorParams;
        int v[2048] = { (AddInfoParam(args, errorParams), 0)... };
        logDetail.__set_errorParams(errorParams);
        return;
    }
};

#endif // NEWFRAMEWORKTEST_BASICJOB_H
