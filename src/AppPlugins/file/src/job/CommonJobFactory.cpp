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
#include "CommonJobFactory.h"
#include "HostCommonService.h"
#include "HostBackup.h"
#include "HostRestore.h"
#include "HostLivemount.h"
#include "HostCancelLivemount.h"
#include "HostArchiveRestore.h"
#include "HostIndex.h"
#ifdef __linux__
#include "VolumeCommonService.h"
#include "VolumeBackup.h"
#include "LinuxVolumeBackup.h"
#include "LinuxVolumeRestore.h"
#include "VolumeIndex.h"
#include "VolumeLivemount.h"
#include "VolumeCancelLivemount.h"
#include "VolumeFileGranularRestore.h"
#include "LinuxVolumeLivemount.h"
#include "LinuxVolumeCancelLivemount.h"
#include "LinuxVolumeIndex.h"
#include "LinuxVolumeFileGranularRestore.h"
#endif
#ifdef WIN32
#include "WinVolumeBackup.h"
#include "WinVolumeRestore.h"
#include "WinVolumeLivemount.h"
#include "WinVolumeCancelLivemount.h"
#include "volume_index/WinVolumeIndex.h"
#include "volume_restore/WinVolumeFileGranularRestore.h"
#endif

namespace FilePlugin {
namespace {
constexpr auto MODULE = "CommonJobFactory";
const int NUM_INT_1 = 1;
const int NUM_INT_100 = 100;
const std::string LOCAL_RESTORE = "LocalRestore";

enum class AppType {
    APPTYPE_NONE = 0,
    APPTYPE_FILESET,
    APPTYPE_ARCHIVE_RESTORE,
    APPTYPE_VOLUME,
    APPTYPE_VOLUME_GRANULAR_RESTORE
};

const std::string FILESET_STR = "Fileset";
const std::string VOLUME_STR = "Volume";
}  // namespace

CommonJobFactory::CommonJobFactory()
{
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_FILESET), JobType::BACKUP)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<HostBackup>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_FILESET), JobType::RESTORE)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<HostRestore>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_FILESET), JobType::LIVEMOUNT)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<HostLivemount>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_FILESET), JobType::CANCELLIVEMOUNT)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<HostCancelLivemount>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_FILESET), JobType::INDEX)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<HostIndex>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_ARCHIVE_RESTORE), JobType::RESTORE)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<HostArchiveRestore>, this, std::placeholders::_1));
#ifdef __linux__
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::BACKUP)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<LinuxVolumeBackup>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::RESTORE)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<LinuxVolumeRestore>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME_GRANULAR_RESTORE), JobType::RESTORE)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<LinuxVolumeFileGranularRestore>,
        this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::LIVEMOUNT)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<LinuxVolumeLivemount>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::CANCELLIVEMOUNT)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<LinuxVolumeCancelLivemount>,
            this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::INDEX)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<LinuxVolumeIndex>, this, std::placeholders::_1));
#endif
#ifdef WIN32
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::BACKUP)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<WinVolumeBackup>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::RESTORE)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<WinVolumeRestore>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::LIVEMOUNT)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<WinVolumeLivemount>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::CANCELLIVEMOUNT)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<WinVolumeCancelLivemount>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME), JobType::INDEX)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<WinVolumeIndex>, this, std::placeholders::_1));
    m_commonJobMap[GetJobType(static_cast<int>(AppType::APPTYPE_VOLUME_GRANULAR_RESTORE), JobType::RESTORE)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<WinVolumeFileGranularRestore>,
            this, std::placeholders::_1));
#endif
}

CommonJobFactory::~CommonJobFactory()
{}

uint64_t CommonJobFactory::GetJobType(uint64_t appType, JobType jobType)
{
    return appType * NUM_INT_100 + static_cast<int>(jobType) * NUM_INT_1;
}

uint64_t CommonJobFactory::GetAppType(const std::shared_ptr<JobCommonInfo> &jobCommonInfo, JobType jobType)
{
    switch (jobType) {
        case JobType::BACKUP: {
            return GetBackupAppType(jobCommonInfo);
        }
        case JobType::RESTORE: {
            return GetRestoreAppType(jobCommonInfo);
        }
        case JobType::LIVEMOUNT: {
            return GetLivemountAppType(jobCommonInfo);
        }
        case JobType::CANCELLIVEMOUNT: {
            return GetCancelLivemountAppType(jobCommonInfo);
        }
        case JobType::INDEX: {
            return GetIndexAppType(jobCommonInfo);
        }
        default: {
            ERRLOG("jobType is wrong ,the appType is: %d", static_cast<int>(jobType));
            return static_cast<uint64_t>(AppType::APPTYPE_NONE);
        }
    }
    return static_cast<uint64_t>(AppType::APPTYPE_NONE);
}

uint64_t CommonJobFactory::GetBackupAppType(const std::shared_ptr<JobCommonInfo> &jobCommonInfo)
{
    std::shared_ptr<ThriftDataBase> tempPtr = jobCommonInfo->GetJobInfo();
    std::shared_ptr<AppProtect::BackupJob> jobInfo = std::dynamic_pointer_cast<AppProtect::BackupJob>(tempPtr);
    if (jobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "Pointer conversion failed., please check it! " << HCPENDLOG;
        return static_cast<uint64_t>(AppType::APPTYPE_NONE);
    }
    std::string appTypeString = jobInfo->protectObject.subType;
    if (appTypeString == FILESET_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_FILESET);
    } else if (appTypeString == VOLUME_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_VOLUME);
    }
    HCP_Log(ERR, MODULE) << "subJobType is wrong ,the appTypeString is: " << appTypeString << HCPENDLOG;
    return static_cast<uint64_t>(AppType::APPTYPE_NONE);
}

uint64_t CommonJobFactory::GetRestoreAppType(const std::shared_ptr<JobCommonInfo> &jobCommonInfo)
{
    std::shared_ptr<ThriftDataBase> tempPtr = jobCommonInfo->GetJobInfo();
    std::shared_ptr<AppProtect::RestoreJob> jobInfo = std::dynamic_pointer_cast<AppProtect::RestoreJob>(tempPtr);
    if (jobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "Pointer conversion failed., please check it! " << HCPENDLOG;
        return static_cast<uint64_t>(AppType::APPTYPE_NONE);
    }
    std::string appTypeString = jobInfo->targetObject.subType;
    for (const AppProtect::Copy &copy : jobInfo->copies) {
        DBGLOG("Copy type: %d", static_cast<int>(copy.dataType));
        if (copy.dataType == AppProtect::CopyDataType::CLOUD_STORAGE_COPY && appTypeString == FILESET_STR) {
            return static_cast<uint64_t>(AppType::APPTYPE_ARCHIVE_RESTORE);
        } else if (copy.dataType == AppProtect::CopyDataType::TAPE_STORAGE_COPY && appTypeString == FILESET_STR) {
            // 磁带归档副本既支持直接恢复，也支持下载恢复，如果是下载恢复，走原流程
            if (jobInfo->jobParam.restoreMode == LOCAL_RESTORE) {
                break;
            }
            return static_cast<uint64_t>(AppType::APPTYPE_ARCHIVE_RESTORE);
        }
    }
    bool isFineGrained = jobInfo->jobParam.restoreType == AppProtect::RestoreJobType::type::FINE_GRAINED_RESTORE;
    if (appTypeString == FILESET_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_FILESET);
    } else if (appTypeString == VOLUME_STR && isFineGrained) {
        return static_cast<uint64_t>(AppType::APPTYPE_VOLUME_GRANULAR_RESTORE);
    } else if (appTypeString == VOLUME_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_VOLUME);
    }

    HCP_Log(ERR, MODULE) << "subJobType is wrong ,the appTypeString is: " << appTypeString << HCPENDLOG;
    return static_cast<uint64_t>(AppType::APPTYPE_NONE);
}

uint64_t CommonJobFactory::GetLivemountAppType(const std::shared_ptr<JobCommonInfo> &jobCommonInfo)
{
    DBGLOG("Enter GetLivemountAppType!");
    std::shared_ptr<ThriftDataBase> tempPtr = jobCommonInfo->GetJobInfo();
    std::shared_ptr<AppProtect::LivemountJob> jobInfo = std::dynamic_pointer_cast<AppProtect::LivemountJob>(tempPtr);
    if (jobInfo == nullptr) {
        ERRLOG("Pointer conversion failed!");
        return static_cast<uint64_t>(AppType::APPTYPE_NONE);
    }
    std::string appTypeString = jobInfo->targetObject.subType;
    if (appTypeString == FILESET_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_FILESET);
    } else if (appTypeString == VOLUME_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_VOLUME);
    }
    HCP_Log(ERR, MODULE) << "subJobType is wrong ,the appTypeString is: " << appTypeString << HCPENDLOG;
    return static_cast<uint64_t>(AppType::APPTYPE_NONE);
}

uint64_t CommonJobFactory::GetCancelLivemountAppType(const std::shared_ptr<JobCommonInfo> &jobCommonInfo)
{
    DBGLOG("Enter GetLivemountAppType!");
    std::shared_ptr<ThriftDataBase> tempPtr = jobCommonInfo->GetJobInfo();
    std::shared_ptr<AppProtect::CancelLivemountJob> jobInfo =
        std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(tempPtr);
    if (jobInfo == nullptr) {
        ERRLOG("Pointer conversion failed!");
        return static_cast<uint64_t>(AppType::APPTYPE_NONE);
    }
    std::string appTypeString = jobInfo->targetObject.subType;
    if (appTypeString == FILESET_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_FILESET);
    } else if (appTypeString == VOLUME_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_VOLUME);
    }
    HCP_Log(ERR, MODULE) << "subJobType is wrong ,the appTypeString is: " << appTypeString << HCPENDLOG;
    return static_cast<uint64_t>(AppType::APPTYPE_NONE);
}

uint64_t CommonJobFactory::GetIndexAppType(const std::shared_ptr<JobCommonInfo> &jobCommonInfo)
{
    DBGLOG("Enter GetIndexAppTypeP!");
    std::shared_ptr<ThriftDataBase> tempPtr = jobCommonInfo->GetJobInfo();
    std::shared_ptr<AppProtect::BuildIndexJob> jobInfo = std::dynamic_pointer_cast<AppProtect::BuildIndexJob>(tempPtr);
    if (jobInfo == nullptr) {
        ERRLOG("Pointer conversion failed!");
        return static_cast<uint64_t>(AppType::APPTYPE_NONE);
    }
    std::string appTypeString = jobInfo->indexProtectObject.subType;
    if (appTypeString == FILESET_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_FILESET);
    } else if (appTypeString == VOLUME_STR) {
        return static_cast<uint64_t>(AppType::APPTYPE_VOLUME);
    }
    HCP_Log(ERR, MODULE) << "subJobType is wrong ,the appTypeString is: " << appTypeString << HCPENDLOG;
    return static_cast<uint64_t>(AppType::APPTYPE_NONE);
}

std::shared_ptr<BasicJob> CommonJobFactory::CreateJob(
    const std::shared_ptr<JobCommonInfo> &jobCommonInfo, JobType jobType)
{
    DBGLOG("Enter Create Job , %d", static_cast<int>(jobType));
    uint64_t appType = GetAppType(jobCommonInfo, jobType);
    uint64_t mapKey = GetJobType(appType, jobType);
    if (m_commonJobMap.find(mapKey) == m_commonJobMap.end()) {
        HCP_Log(ERR, MODULE) << "no such operation in map, create job failed" << HCPENDLOG;
        HCP_Log(ERR, MODULE) << "appType is: " << appType << "\tcommandType is: " << static_cast<int>(jobType)
                             << HCPENDLOG;
        return nullptr;
    }
    DBGLOG("Create job appType: %d, mapKet: %d", appType, mapKey);
    auto func = m_commonJobMap[mapKey];
    DBGLOG("Func: %s", typeid(func).name());
    auto jobPtr = func(jobCommonInfo, appType);
    if (jobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "create job failed" << HCPENDLOG;
        return nullptr;
    }
    HCP_Log(DEBUG, MODULE) << "Exit create job." << HCPENDLOG;
    return jobPtr;
}

template <typename T>
std::shared_ptr<BasicJob> CommonJobFactory::CreateFactoryJob(std::shared_ptr<JobCommonInfo> jobCommonInfo)
{
    HCP_Log(DEBUG, MODULE) << "Enter create factory job." << HCPENDLOG;
    auto job = std::make_shared<T>();
    job->SetJobInfo(jobCommonInfo);
    return job;
}
}  // namespace FilePlugin