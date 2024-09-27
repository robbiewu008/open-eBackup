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
#ifndef CANCEL_LIVEMOUNT_JOB_H
#define CANCEL_LIVEMOUNT_JOB_H

#include <unordered_map>
#include <common/Macros.h>
#include <common/Structs.h>
#include <common/DirtyRanges.h>
#include <common/checkpoint/Checkpoint.h>
#include <repository_handlers/RepositoryHandler.h>
#include <protect_engines/ProtectEngine.h>
#include <volume_handlers/VolumeHandler.h>
#include <job_controller/io_scheduler/TaskScheduler.h>
#include <job_controller/jobs/VirtualizationBasicJob.h>
#include <job_controller/jobs/backup/BackupIoTask.h>

namespace VirtPlugin {
/* BackupJob states */
enum class CancelLivemountJobStates {
    STATE_NONE = 0,
    /* prerequisite job states */
    STATE_PRE_INIT,
    STATE_PRE_PREHOOK,
    STATE_PRE_CHECK_BEFORE_UNMOUNT,
    STATE_PRE_POWER_OFF_MACHINE,        // 下电虚拟机
    STATE_PRE_POSTHOOK,

    /* generate sub job states */
    STATE_GEN_JOB_INIT,
    STATE_GENERATE_PREHOOK,
    STATE_GENERATE_DO_GENERATE_SUBJOB,
    STATE_GENERATE_POSTHOOK,

    /* exec sub job states */
    STATE_EXEC_SUB_JOB_INIT,
    STATE_EXEC_PREHOOK,
    STATE_EXEC_CHECK_BEFORE_UNMOUNT,
    STATE_EXEC_POWER_OFF_MACHINE,
    STATE_EXEC_DELETE_MACHINE,
    STATE_EXEC_CANCEL_MOUNT,
    STATE_EXEC_POSTHOOK,

    /* post job states */
    STATE_POST_JOB_INIT,
    STATE_POST_PREHOOK,
    STATE_POST_POSTHOOK
};

class CancelLivemountJob : public VirtualizationBasicJob {
public:
    CancelLivemountJob() {};
    virtual ~CancelLivemountJob() {};

    EXTER_ATTACK virtual int GenerateSubJob() override;
    EXTER_ATTACK virtual int ExecuteSubJob() override;

protected:
    /* Generate sub job */
    int GenerateJobInit();
    int DoGenerateSubJob();
    void SetGenerateJobStateMachine();
    int GenMainTaskStatusToFile();

    /* Execute sub job */
    int ExecuteSubJobInner();
    void SubJobStateInit();
    int SubTaskInitialize();
    int CheckBeforeUnmount();
    int PowerOffMachine();
    int DeleteVirtualMachine();
    int CancelNasMount();
    int SubTaskClean();

    /* Common */
    int LoadVMInfo(VMInfo &vmInfo);
    bool CommonInfoInit();
    bool InitHandlers();
    bool InitRepo() const;
    bool CheckMainTaskStatusFileExist();

    std::string GetTaskId() const
    {
        std::string output;
        if (m_cancelLivemountPara != nullptr) {
            output = "jobId=" + m_cancelLivemountPara->jobId;
        }
        if (m_subJobInfo != nullptr) {
            output = output + ", subJobId=" + m_subJobInfo->subJobId;
        }
        return output;
    }

    int GenerateSubJobPreHook();
    int GenerateSubJobPostHook();
    int ExecuteSubJobPreHook();
    int ExecuteSubJobPostHook();

protected:
    std::string m_metaRepoPath;
    std::string m_cacheRepoPath;

    VMInfo m_vmInfo;

    std::shared_ptr<AppProtect::CancelLivemountJob> m_cancelLivemountPara = nullptr;
    std::shared_ptr<VolumeHandler> m_volHandler = nullptr;
    std::shared_ptr<VolInfo> m_backupVolInfo = nullptr;
    std::unique_ptr<TaskScheduler> m_taskScheduler = nullptr;
    StorageRepository m_dataRepo {};
    StorageRepository m_metaRepo {};
    std::string m_liveMountVMMetaDataPath {};
    std::string m_dataRepoPath {};
    std::string m_xNNEsn;

protected:
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_dataRepoHandler = nullptr;
};
}

#endif
