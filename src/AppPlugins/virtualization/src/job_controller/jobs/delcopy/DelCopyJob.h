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
#ifndef DELCOPY_JOB_H
#define DELCOPY_JOB_H
#include <job_controller/jobs/VirtualizationBasicJob.h>
namespace VirtPlugin {
enum class DelCopyJobSteps {
    STATE_NONE = 0,
    /* generate sub job steps */
    STEP_GENERATE_DO_GENERATE_SUBJOB,
    /* exec sub job steps */
    STEP_EXEC_SUB_JOB_INIT,
    STATE_EXECJOB_PREHOOK,
    STEP_EXEC_DELETE_SNAPSHOT,
    STATE_EXECJOB_POSTHOOK
};

class DelCopyJob : public VirtualizationBasicJob {
public:
    DelCopyJob() = default;
    virtual ~DelCopyJob() = default;
    EXTER_ATTACK virtual int32_t GenerateSubJob() override;
    EXTER_ATTACK virtual int32_t ExecuteSubJob() override;
    /* common*/
    int32_t ParseJobInfo();
    /* generate subjob*/
    int32_t GenerateSubJobInner();
    int32_t CreateSubTask();
    /* execute subjob*/
    int32_t ExecuteSubJobInner();
    int32_t ExecuteSubJobPreHook();
    void InitExecStateMachine();
    int32_t InitExecInfo();
    int32_t DeleteSnapshot();
    std::string GetSnapshotsLogDetails(const std::vector<VolSnapInfo>& volSnapshots);
    int32_t ExecuteSubJobPostHook();
private:
    std::shared_ptr<AppProtect::DelCopyJob> m_delCopyPara = nullptr;
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::string m_metaRepoPath;
    SnapshotInfo m_snapshotInfo;
};
}
#endif