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
#ifndef INSTANT_RESTORE_JOB_H
#define INSTANT_RESTORE_JOB_H

#include <unordered_map>
#include <vector>
#include <list>
#include "../restore/RestoreJob.h"

namespace VirtPlugin {
class InstantRestoreJob : public RestoreJob {
public:
    InstantRestoreJob() {};
    ~InstantRestoreJob() {};
    EXTER_ATTACK virtual int PrerequisiteJob() override;
    EXTER_ATTACK virtual int GenerateSubJob() override;
    EXTER_ATTACK virtual int ExecuteSubJob() override;
    EXTER_ATTACK virtual int PostJob() override;

private:
    int PreInstantJobInner();
    void PreInitInstantStateHandles();
    int CheckBeforeMount();
    void SetGenerateJobStateMachine();
    int GenerateSubJobPreHook();
    int GenerateJobInit();
    int CreateSubTasks();
    int PutSubTasksToFrame();
    int ExecuteSubJobInner();
    void SubJobStateInit();
    int SubTaskInitialize();
    int SubTaskExecute();
    int SubJobMigrateVolume();
    bool LoadMetaData() override;
    bool LoadLiveMetaData();
    int SaveVMInfo(const VMInfo &vmInfo);
    int PostJobInner();
    int PostJobStateInit();
    int DeleteLiveMachine();
    int PostClean();
    int PostJobPreHook();
    int PostJobPostHook();
    int SubJobPowerOnMachine();
    int PowerOnMachine();

private:
    VMInfo m_newVm;

    Livemount m_cacheInfo;
};
}
#endif