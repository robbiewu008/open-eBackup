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
#ifndef VOLUMEBACKUP_TASK_RESOURCE_MANAGER_HEADER
#define VOLUMEBACKUP_TASK_RESOURCE_MANAGER_HEADER

#include "VolumeProtector.h"

namespace volumeprotect {
namespace task {


/**
 * @brief Params struct used to build BackupTaskResourceManager
 */
struct BackupTaskResourceManagerParams {
    CopyFormat          copyFormat;
    BackupType          backupType;
    std::string         copyDataDirPath;
    std::string         copyName;
    std::string         shareName;
    uint64_t            volumeSize;
    uint64_t            maxSessionSize;     ///> only used to create fragment copy for CopyFormat::BIN
};

/**
 * @brief Params struct used to build RestoreTaskResourceManager
 */
struct RestoreTaskResourceManagerParams {
    CopyFormat                  copyFormat;
    std::string                 copyDataDirPath;
    std::string                 copyName;
    std::string                 shareName;
    std::vector<std::string>    copyDataFiles;
};

/**
 * @brief Base class for BackupTaskResourceManager and RestoreTaskResourceManager.
 * Provide TaskResourceManager builder and RAII resource management.
 * PrepareCopyResource() need to be invoked before backup/restore task start.
 * TaskResourceManager is used to prepare resource for Backup/Restore tasks, including:
 *  1. Create copy file on disk for backup
 *  2. Init virtual disk partition info for Backup
 *  3. Attach virtual disk for Backup & Restore
 *  4. Detach virtual disk for Backup & Restore (when destroyed)
 **/
class TaskResourceManager {
public:
    static std::unique_ptr<TaskResourceManager> BuildBackupTaskResourceManager(
        const BackupTaskResourceManagerParams& params);

    static std::unique_ptr<TaskResourceManager> BuildRestoreTaskResourceManager(
        const RestoreTaskResourceManagerParams& params);

    TaskResourceManager(
        CopyFormat copyFormat,
        const std::string& copyDataDirPath,
        const std::string& copyName,
        const std::string& shareName = "");

    virtual ~TaskResourceManager() = default;

    virtual bool PrepareCopyResource() = 0;
    virtual bool DetachCopyResource();

protected:
    virtual bool AttachCopyResource();

    virtual bool ResourceExists() = 0;

protected:
    CopyFormat          m_copyFormat;
    std::string         m_copyDataDirPath;
    std::string         m_copyName;
    std::string         m_shareName;
    // mutable
    std::string         m_physicalDrivePath;
};

// BackupTaskResourceManager is inited before backup task start
class BackupTaskResourceManager : public TaskResourceManager {
public:
    BackupTaskResourceManager(const BackupTaskResourceManagerParams& param);

    ~BackupTaskResourceManager();

    bool PrepareCopyResource() override;

private:
    // Create and Init operation is only need for backup
    bool CreateBackupCopyResource();

    bool InitBackupCopyResource();

    bool ResourceExists() override;

private:
    BackupType          m_backupType;
    uint64_t            m_volumeSize;
    uint64_t            m_maxSessionSize;     // only used to create fragment copy for CopyFormat::BIN
};

// RestoreTaskResourceManager is inited before restore task start
class RestoreTaskResourceManager : public TaskResourceManager {
public:
    RestoreTaskResourceManager(const RestoreTaskResourceManagerParams& param);

    ~RestoreTaskResourceManager();

    bool PrepareCopyResource() override;

protected:
    bool ResourceExists() override;

private:
    std::vector<std::string> m_copyDataFiles;
};

}
}

#endif