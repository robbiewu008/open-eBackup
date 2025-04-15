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
#ifndef VOLUMEBACKUP_RESTORE_TASK_HEADER
#define VOLUMEBACKUP_RESTORE_TASK_HEADER

#include "VolumeProtector.h"
#include "VolumeProtectTaskContext.h"
#include "native/TaskResourceManager.h"
#include "common/VolumeUtils.h"

namespace volumeprotect {
namespace task {

/**
 * @brief Control control volume restore procedure
 */
class VolumeRestoreTask
    : public VolumeProtectTask, public TaskStatisticTrait, public VolumeTaskCheckpointTrait {
public:
    using SessionQueue = std::queue<VolumeTaskSession>;

    bool            Start() override;

    TaskStatistics  GetStatistics() const override;

    VolumeRestoreTask(const VolumeRestoreConfig& restoreConfig, const VolumeCopyMeta& volumeCopyMeta);

    ~VolumeRestoreTask() override;

protected:
    bool Prepare(); // split session and save meta

    void ThreadFunc();

    void DetachCopyResource();

    bool StartRestoreSession(std::shared_ptr<VolumeTaskSession> session);

    virtual bool InitRestoreSessionContext(std::shared_ptr<VolumeTaskSession> session);

    virtual bool InitRestoreSessionTaskExecutor(std::shared_ptr<VolumeTaskSession> session);

protected:
    uint64_t                                m_volumeSize;
    std::shared_ptr<VolumeRestoreConfig>    m_restoreConfig;
    std::shared_ptr<VolumeCopyMeta>         m_volumeCopyMeta;

    std::thread     m_thread;
    SessionQueue    m_sessionQueue;
    std::shared_ptr<TaskResourceManager> m_resourceManager;
};

}
}

#endif