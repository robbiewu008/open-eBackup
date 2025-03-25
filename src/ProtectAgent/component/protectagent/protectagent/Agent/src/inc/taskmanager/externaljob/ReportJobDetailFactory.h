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
#ifndef REPORT_JOB_DETAIL_FACTORY_H_
#define REPORT_JOB_DETAIL_FACTORY_H_
#include <memory>
#include "common/Types.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"
#include "taskmanager/externaljob/Job.h"
#include "taskmanager/externaljob/PluginMainJob.h"
#include "taskmanager/externaljob/PluginSubJob.h"

namespace AppProtect {
enum class DmeTaskStatus {
    INITIALIZING = 0,
    RUNNING = 1,
    ABORTING = 2,
    COMPLETED = 3,
    ABORTED = 4,
    ABORTED_FAILED = 5,
    FAILED = 6,
    FAILED_NOTRY = 7,
    PARTIAL_COMPLETED = 13,
};

enum class DmeTaskStage {
    // when job failed, no stage can be report,set status failed,stage empty
    NULL_STAGE = -1,
    DME_PREPARING = 0,
    DME_PREPARE_COMPLETED = 1,
    AGENT_PREPARING = 2,
    PRE_TASK_COMPLETE = 3,
    GENERATE_TASK_COMPLETE = 4,
    DME_CLEANING = 5,
    WAITING_DME_POST_PROCESS = 6,
    DME_PROCESS_END = 7,
    SWITCH_INCR_TO_FULL = 8,
    CHECK_FAILED = 9,
    SWITCH_INCR_TO_DIFF = 10
};

class ReportJobDetailFactory {
public:
    static std::shared_ptr<ReportJobDetailFactory> GetInstance();
    AppProtect::SubJobDetails GeneratorMainJobDetail(PluginMainJob::ActionEvent event,
        PluginMainJob::EventResult result, mp_int32 resultCode, const PluginJobData& data);
    AppProtect::SubJobDetails GeneratorSubJobDetail(PluginSubJob::ActionEvent event,
        PluginSubJob::EventResult result, mp_int32 resultCode, const PluginJobData& data);
    
    mp_int32 SendDetailToDme(const AppProtect::SubJobDetails& detail, mp_int32 main_stage);
    mp_int32 SendAndhandleRequest(const AppProtect::SubJobDetails& jobInfo, const Json::Value& detail);
    mp_void TransferJobStageToJson(mp_int32 jobMainStage, Json::Value& value);
    mp_int32 ReportMainAndPostJobInformation(const Json::Value& mainJob);
private:
    mp_bool IsReportMainJobDetail(const AppProtect::SubJobDetails& jobInfo, Json::Value &detail);
    mp_void JobDetailAddNodeId(const AppProtect::SubJobDetails& jobInfo, Json::Value &detail);
    mp_void StopReportMainJobDetail(const mp_string& jobId, const Json::Value& detail);
};
}
#endif