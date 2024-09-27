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
#ifndef JOBDETAIL_H
#define JOBDETAIL_H

#include <thread>
#include <chrono>
#include <atomic>
#include "LocalCmdExector.h"
#include "define/Types.h"
#include "BasicJob.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "trjsontostruct.h"
#include "JobControl.h"

namespace GeneralDB {
enum class DetailTimerStatus {
    INITIALIZATION = 0,
    STOP_TIMER,
    QUERY_DETAIL_NOW
};

class JobDetail {
public:
    JobDetail();
    ~JobDetail();

    mp_void QueryJobDetails(const GeneralDB::Param &prm);
    mp_void AbortJobOrPauseJob(const GeneralDB::Param &prm, std::shared_ptr<BasicJob> jobInfo = nullptr);
    mp_void NotifyJobDetailTimer(const DetailTimerStatus &status);
    mp_void StopAbortOrPauseTimer();

private:
    mp_void ReportAbortStatus(const mp_string &jobId, const mp_string &subJobId, const mp_int32 &status);
private:
    std::atomic<mp_int32> m_jobDetails;
    std::atomic<bool> m_stopAbortOrPause;
};
}
#endif