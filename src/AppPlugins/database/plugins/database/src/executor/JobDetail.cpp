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
#include <vector>
#include <exception>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log/Log.h"
#include "trjsontostruct.h"
#include "client/ClientInvoke.h"
#include "DBPluginPath.h"
#include "UniqueId.h"
#include "JobDetail.h"

using namespace GeneralDB;

namespace {
const mp_int32  PERIODICITY_TIME = 30;

const mp_int32  TIMEOUT_NUMS = 10;
const mp_int32  INITIAL_VALUE = 0;
}

JobDetail::JobDetail()
{
    m_jobDetails.store(static_cast<mp_int32>(DetailTimerStatus::INITIALIZATION));
    m_stopAbortOrPause.store(MP_FALSE);
}

JobDetail::~JobDetail()
{
}

mp_void JobDetail::QueryJobDetails(const Param& prm)
{
    INFOLOG("Report detail thread start, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
    Param detailParam = prm;
    mp_int32 execFailedNums = 0;
    mp_int32 queryDetailsInterval = 0;
    detailParam.isAsyncInterface = MP_FALSE;
    detailParam.isProgessScript = MP_TRUE;
    while (true) {
        mp_int32 reportTimerStatus = m_jobDetails.load();
        if (reportTimerStatus == static_cast<mp_int32>(DetailTimerStatus::STOP_TIMER)) {
            INFOLOG("Stop report job details timer, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
            break;
        }
        if (reportTimerStatus != static_cast<mp_int32>(DetailTimerStatus::QUERY_DETAIL_NOW)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (queryDetailsInterval < PERIODICITY_TIME) {
                queryDetailsInterval++;
                continue;
            }
            queryDetailsInterval = INITIAL_VALUE;
        }
        Json::Value retValue;
        if (LocalCmdExector::GetInstance().Exec(detailParam, retValue, nullptr) != MP_SUCCESS) {
            ERRLOG("Execute progress scipt failed, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
 
            execFailedNums++;
            if (execFailedNums > TIMEOUT_NUMS) {
                ERRLOG("Failed to exec progress scipt for more than %d times, jobId=%s, subJobId=%s.",
                    TIMEOUT_NUMS, prm.jobId.c_str(), prm.subJobId.c_str());
                break;
            }
            continue;
        }
        execFailedNums = INITIAL_VALUE;
        AppProtect::SubJobDetails jobDetails;
        JsonToStruct(retValue, jobDetails);
        AppProtect::ActionResult rtnValue;
        JobService::ReportJobDetails(rtnValue, jobDetails);
        if (rtnValue.code != MP_SUCCESS) {
            WARNLOG("Report job details failed, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
        }
        if (jobDetails.jobStatus != AppProtect::SubJobStatus::RUNNING) {
            INFOLOG("Report detail thread end, jobId=%s, subJobId=%s, jobStatus=%d.",
                prm.jobId.c_str(), prm.subJobId.c_str(), jobDetails.jobStatus);
            break;
        }
    }
    DBGLOG("Report timer thread exited, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
}

mp_void JobDetail::AbortJobOrPauseJob(const Param& prm, std::shared_ptr<BasicJob> jobInfo)
{
    LOGGUARD("");
    if (jobInfo == nullptr) {
        ERRLOG("Abort and pause job info is nullptr, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
        return;
    }
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        mp_bool ret = m_stopAbortOrPause.load();
        if (ret) {
            DBGLOG("Stop abort and pause timer, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
            break;
        }
        if (jobInfo->IsAbortJob()) {
            INFOLOG("Job get abort signal jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
            m_jobDetails.store(static_cast<mp_int32>(DetailTimerStatus::STOP_TIMER));
            AppProtect::ActionResult rtnValue;
            JobControl::AbortJob(rtnValue, prm);
            ReportAbortStatus(prm.jobId, prm.subJobId, rtnValue.code);
            if (rtnValue.code == 0) {
                jobInfo->SetJobToFinish();
            }
            INFOLOG("Job abort completed jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
            return;
        }
        if (jobInfo->IsJobPause()) {
            INFOLOG("Job get pause signal jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
            m_jobDetails.store(static_cast<mp_int32>(DetailTimerStatus::STOP_TIMER));
            AppProtect::ActionResult rtnValue;
            JobControl::PauseJob(rtnValue, prm);
            ReportAbortStatus(prm.jobId, prm.subJobId, rtnValue.code);
            if (rtnValue.code == 0) {
                jobInfo->SetJobToFinish();
            }
            INFOLOG("Job pause completed jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
            return;
        }
    }
}

mp_void JobDetail::NotifyJobDetailTimer(const DetailTimerStatus &status)
{
    m_jobDetails.store(static_cast<mp_int32>(status));
}

mp_void JobDetail::StopAbortOrPauseTimer()
{
    m_stopAbortOrPause.store(MP_TRUE);
}

mp_void JobDetail::ReportAbortStatus(const mp_string &jobId, const mp_string &subJobId, const mp_int32 &status)
{
    AppProtect::SubJobDetails jobInfo;
    AppProtect::ActionResult rtnValue;
    jobInfo.__set_jobId(jobId);
    jobInfo.__set_subJobId(subJobId);
    jobInfo.__set_jobStatus(status != 0 ? SubJobStatus::ABORTED_FAILED : SubJobStatus::ABORTED);
    JobService::ReportJobDetails(rtnValue, jobInfo);
    if (rtnValue.code != MP_SUCCESS) {
        WARNLOG("Report abort job detail failed, jobId=%s, subJobId=%s.", jobId.c_str(), subJobId.c_str());
    }
}
