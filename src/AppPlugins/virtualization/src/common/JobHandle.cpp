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
#include <log/Log.h>
#include <common/Structs.h>

VIRT_PLUGIN_NAMESPACE_BEGIN

AppProtect::ApplicationEnvironment JobHandle::GetAppEnv() const
{
    AppProtect::ApplicationEnvironment appEnv;
    switch (m_jobType) {
        case JobType::BACKUP:
            appEnv = GetProtectAppEnv<AppProtect::BackupJob>();
            break;
        case JobType::RESTORE:
        case JobType::INSTANT_RESTORE:
            appEnv = GetTargetAppEnv<AppProtect::RestoreJob>();
            break;
        case JobType::LIVEMOUNT:
            appEnv = GetTargetAppEnv<AppProtect::LivemountJob>();
            break;
        case JobType::CANCELLIVEMOUNT:
            appEnv = GetTargetAppEnv<AppProtect::CancelLivemountJob>();
            break;
        case JobType::DELCOPY:
            appEnv = GetProtectAppEnv<AppProtect::DelCopyJob>();
            break;
        default:
            break;
    }
    return appEnv;
}

template <class T>
AppProtect::ApplicationEnvironment JobHandle::GetProtectAppEnv() const
{
    AppProtect::ApplicationEnvironment appEnv;
    std::shared_ptr<T> jobParam = std::dynamic_pointer_cast<T>(m_jobInfo->GetJobInfo());
    if (jobParam == nullptr) {
        ERRLOG("Job para is null.");
        return appEnv;
    }
    appEnv = jobParam->protectEnv;
    return appEnv;
}

template <class T>
AppProtect::ApplicationEnvironment JobHandle::GetTargetAppEnv() const
{
    AppProtect::ApplicationEnvironment appEnv;
    std::shared_ptr<T> jobParam = std::dynamic_pointer_cast<T>(m_jobInfo->GetJobInfo());
    if (jobParam == nullptr) {
        ERRLOG("Job para is null.");
        return appEnv;
    }
    appEnv = jobParam->targetEnv;
    return appEnv;
}

AppProtect::Application JobHandle::GetApp() const
{
    AppProtect::Application app;
    std::shared_ptr<AppProtect::BackupJob> backupParam = nullptr;
    std::shared_ptr<AppProtect::RestoreJob> restoreParam = nullptr;
    std::shared_ptr<AppProtect::LivemountJob> livemountParam = nullptr;
    std::shared_ptr<AppProtect::CancelLivemountJob> cancelLivemountParam = nullptr;
    switch (m_jobType) {
        case JobType::BACKUP:
            backupParam = std::dynamic_pointer_cast<AppProtect::BackupJob>(m_jobInfo->GetJobInfo());
            if (backupParam == nullptr) {
                ERRLOG("Backup job para is null.");
                return app;
            }
            app = backupParam->protectObject;
            break;
        case JobType::RESTORE:
        case JobType::INSTANT_RESTORE:
            restoreParam = std::dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobInfo->GetJobInfo());
            if (restoreParam == nullptr) {
                ERRLOG("Restore job para is null.");
                return app;
            }
            app = restoreParam->targetObject;
            break;
        case JobType::LIVEMOUNT:
            livemountParam = std::dynamic_pointer_cast<AppProtect::LivemountJob>(m_jobInfo->GetJobInfo());
            if (livemountParam == nullptr) {
                ERRLOG("LivemountJob job para is null.");
                return app;
            }
            app = livemountParam->targetObject;
            break;
        case JobType::CANCELLIVEMOUNT:
            cancelLivemountParam = std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(m_jobInfo->GetJobInfo());
            if (cancelLivemountParam == nullptr) {
                ERRLOG("CancelLivemountJob job para is null.");
                return app;
            }
            app = cancelLivemountParam->targetObject;
            break;
        default:
            break;
    }
    return app;
}

std::string JobHandle::GetTaskId() const
{
    std::shared_ptr<AppProtect::BackupJob> backupParam = nullptr;
    std::shared_ptr<AppProtect::RestoreJob> restoreParam = nullptr;
    std::shared_ptr<AppProtect::LivemountJob> livemountParam = nullptr;
    std::shared_ptr<AppProtect::CancelLivemountJob> cancelLivemountParam = nullptr;
    switch (m_jobType) {
        case JobType::BACKUP:
            backupParam = std::dynamic_pointer_cast<AppProtect::BackupJob>(m_jobInfo->GetJobInfo());
            return backupParam->requestId;
        case JobType::RESTORE:
        case JobType::INSTANT_RESTORE:
            restoreParam = std::dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobInfo->GetJobInfo());
            return restoreParam->requestId;
        case JobType::LIVEMOUNT:
            livemountParam = std::dynamic_pointer_cast<AppProtect::LivemountJob>(m_jobInfo->GetJobInfo());
            if (livemountParam == nullptr) {
                ERRLOG("Restore job para is null.");
                return std::string();
            }
            return livemountParam->requestId;
        case JobType::CANCELLIVEMOUNT:
            cancelLivemountParam = std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(m_jobInfo->GetJobInfo());
            return cancelLivemountParam->requestId;
        default:
            break;
    }
    return std::string();
}

std::vector<AppProtect::ApplicationResource> JobHandle::GetVolumes() const
{
    std::vector<AppProtect::ApplicationResource> volList;
    std::shared_ptr<AppProtect::BackupJob> backupParam = nullptr;
    std::shared_ptr<AppProtect::RestoreJob> restoreParam = nullptr;
    std::shared_ptr<AppProtect::LivemountJob> livemountParam = nullptr;
    std::shared_ptr<AppProtect::CancelLivemountJob> cancelLivemountParam = nullptr;
    switch (m_jobType) {
        case JobType::BACKUP:
            backupParam = std::dynamic_pointer_cast<AppProtect::BackupJob>(m_jobInfo->GetJobInfo());
            if (backupParam == nullptr) {
                ERRLOG("Backup job para is null");
                return volList;
            }
            return backupParam->protectSubObject;
        case JobType::RESTORE:
        case JobType::INSTANT_RESTORE:
            restoreParam = std::dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobInfo->GetJobInfo());
            if (restoreParam == nullptr) {
                ERRLOG("Restore job para is null");
                return volList;
            }
            return restoreParam->restoreSubObjects;
        case JobType::LIVEMOUNT:
            livemountParam = std::dynamic_pointer_cast<AppProtect::LivemountJob>(m_jobInfo->GetJobInfo());
            if (livemountParam == nullptr) {
                ERRLOG("Livemount job para is null.");
                return volList;
            }
            return livemountParam->targetSubObjects;
        case JobType::CANCELLIVEMOUNT:
            cancelLivemountParam = std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(m_jobInfo->GetJobInfo());
            if (cancelLivemountParam == nullptr) {
                ERRLOG("Livemount job para is null.");
                return volList;
            }
            return cancelLivemountParam->targetSubObjects;
        default:
            break;
    }
    return volList;
}

std::vector<AppProtect::StorageRepository> JobHandle::GetStorageRepos() const
{
    std::vector<AppProtect::StorageRepository> storageRepos;
    std::shared_ptr<AppProtect::BackupJob> backupParam = nullptr;
    std::shared_ptr<AppProtect::RestoreJob> restoreParam = nullptr;
    std::shared_ptr<AppProtect::LivemountJob> livemountParam = nullptr;
    std::shared_ptr<AppProtect::CancelLivemountJob> cancelLivemountParam = nullptr;
    switch (m_jobType) {
        case JobType::BACKUP:
            backupParam = std::dynamic_pointer_cast<AppProtect::BackupJob>(m_jobInfo->GetJobInfo());
            if (backupParam == nullptr) {
                ERRLOG("Backup job para is null");
                return storageRepos;
            }
            return backupParam->repositories;
        case JobType::RESTORE:
        case JobType::INSTANT_RESTORE:
            restoreParam = std::dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobInfo->GetJobInfo());
            if (restoreParam == nullptr) {
                ERRLOG("Restore job para is null");
                return storageRepos;
            }
            return restoreParam->copies[0].repositories;
        case JobType::LIVEMOUNT:
            livemountParam = std::dynamic_pointer_cast<AppProtect::LivemountJob>(m_jobInfo->GetJobInfo());
            if (livemountParam == nullptr) {
                ERRLOG("Livemount job para is null.");
                return storageRepos;
            }
            return livemountParam->copy.repositories;
        case JobType::CANCELLIVEMOUNT:
            cancelLivemountParam = std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(m_jobInfo->GetJobInfo());
            if (cancelLivemountParam == nullptr) {
                ERRLOG("Livemount job para is null.");
                return storageRepos;
            }
            return cancelLivemountParam->copy.repositories;
        default:
            break;
    }
    return storageRepos;
}

AppProtect::BackupJobType JobHandle::GetBackupType() const
{
    std::shared_ptr<AppProtect::BackupJob> backupParam = nullptr;
    switch (m_jobType) {
        case JobType::BACKUP:
            backupParam = std::dynamic_pointer_cast<AppProtect::BackupJob>(m_jobInfo->GetJobInfo());
            if (backupParam == nullptr) {
                ERRLOG("Backup job para is null, return full backup.");
                return AppProtect::BackupJobType::FULL_BACKUP;
            }
            return backupParam->jobParam.backupType;
        default:
            break;
    }
    return AppProtect::BackupJobType::FULL_BACKUP;
}
VIRT_PLUGIN_NAMESPACE_END