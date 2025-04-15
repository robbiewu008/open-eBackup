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
#ifndef VOLUMEBACKUP_BACKUPTASK_HEADER
#define VOLUMEBACKUP_BACKUPTASK_HEADER

#include "VolumeProtector.h"
#include "VolumeProtectTaskContext.h"
#include "native/TaskResourceManager.h"
#include "common/VolumeUtils.h"

namespace volumeprotect {
namespace task {

/**
 * @brief Control control volume backup procedure
 */
class VolumeBackupTask
    : public VolumeProtectTask, public TaskStatisticTrait, public VolumeTaskCheckpointTrait {
public:
    using SessionQueue = std::queue<VolumeTaskSession>;

    bool            Start() override;
    TaskStatistics  GetStatistics() const override;

    VolumeBackupTask(const VolumeBackupConfig& backupConfig, uint64_t volumeSize);

    ~VolumeBackupTask() override;

protected:
    bool Prepare(); // split session and save meta

    void ThreadFunc();

    bool StartBackupSession(std::shared_ptr<VolumeTaskSession> session) const;

    virtual bool InitBackupSessionContext(std::shared_ptr<VolumeTaskSession> session) const;

    virtual bool InitBackupSessionTaskExecutor(std::shared_ptr<VolumeTaskSession> session) const;

    bool IsIncrementBackup() const;

    void SaveSessionWriterBitmap(std::shared_ptr<VolumeTaskSession> session);

    VolumeTaskSession NewVolumeTaskSession(uint64_t sessionOffset, uint64_t sessionSize, int sessionIndex) const;

    bool InitHashingContext(std::shared_ptr<VolumeTaskSession> session) const;

    virtual bool LoadSessionPreviousCopyChecksum(std::shared_ptr<VolumeTaskSession> session) const;

    virtual bool SaveVolumeCopyMeta(
        const std::string& copyMetaDirPath,
        const std::string& copyName,
        const VolumeCopyMeta& volumeCopyMeta) const;

    virtual bool ValidateIncrementBackup() const;

    void DetachCopyResource();

protected:
    uint64_t                                m_volumeSize;
    std::shared_ptr<VolumeBackupConfig>     m_backupConfig;

    std::thread         m_thread;
    SessionQueue        m_sessionQueue;
    std::shared_ptr<TaskResourceManager> m_resourceManager;
};

}
}

#endif