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
#ifndef LIVEMOUNT_JOB_H
#define LIVEMOUNT_JOB_H

#include <unordered_map>
#include "job/BasicJob.h"
#include "common/Macros.h"
#include "common/Structs.h"
#include "common/Constants.h"
#include "job_controller/io_scheduler/TaskPool.h"
#include "job_controller/io_scheduler/TaskScheduler.h"
#include "job_controller/jobs/VirtualizationBasicJob.h"
#include "protect_engines/ProtectEngine.h"
#include "repository_handlers/RepositoryHandler.h"
#include "protect_engines/engine_factory/EngineFactory.h"
#include "repository_handlers/filesystem/FileSystemHandler.h"
#include "repository_handlers/factory/RepositoryFactory.h"

namespace VirtPlugin {
class LivemountJob : public VirtualizationBasicJob {
public:
    LivemountJob() {};
    ~LivemountJob() {};
    EXTER_ATTACK virtual int GenerateSubJob() override;
    EXTER_ATTACK virtual int ExecuteSubJob() override;

    /* 公共函数 */
    bool CommonInfoInit();
    bool InitRepo() const;
    bool InitHandlers();
    bool LoadVmMetaData();
    bool LoadMetaData();
    int GenMainTaskStatusToFile();
    bool CheckMainTaskStatusFileExist();
    int SaveVMInfo(const VMInfo &vmInfo);
    int PowerOnMachine();
    int SubJobRenameMachine();
    int SubJobPowerOnMachine();
    int ReportCopy();
    int FormatBackupCopy(Copy &copy);

    /* 前置任务 */
    int CheckBeforeMount();

    // 执行子任务接口
    int ExecuteSubJobInner();
    int SubTaskInitialize();
    int InitTaskScheduler();
    void SubJobStateInit();
    int SubTaskExecute();
    int SubTaskClean();
    void SetGenerateJobStateMachine();
    int GenerateJobInit();
    int CreateSubTasks();
    int PutSubTasksToFrame();

    /* 后置任务 */
    int PostClean();
    int DeleteLiveMachine();

    /* hook函数 */
    int PrerequisitePreHook();
    int PrerequisitePostHook();
    int GenerateSubJobPreHook();
    int GenerateSubJobPostHook();
    int ExecuteSubJobPreHook();
    int ExecuteSubJobPostHook();

    std::string GetTaskId() const
    {
        std::string output;
        if (m_liveMountPara != nullptr) {
            output = "jobId=" + m_liveMountPara->jobId;
        }
        if (m_subJobInfo != nullptr) {
            output = output + ",subJobId=" + m_subJobInfo->subJobId;
        }
        return output;
    }

public:
    std::shared_ptr<AppProtect::LivemountJob> m_liveMountPara = nullptr;

    std::string m_newVMMetaDataPath {};
    std::vector<SubJob> m_execSubs;
    
    VMInfo m_copyVm;
    VMInfo m_newVm;
    std::vector<VolInfo> m_backupedVolList;

    std::string m_dataRepoPath {};
    std::string m_cacheRepoPath {};
    std::string m_metaRepoPath {};
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_dataRepoHandler = nullptr;
    LivemountType m_jobType = LivemountType::UNKNOWN;

    bool m_poweron {false};
};
}

#endif