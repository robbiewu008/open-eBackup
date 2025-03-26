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
#ifndef VOLUME_COMMONSERVICE_H
#define VOLUME_COMMONSERVICE_H

/* Plugin include */
#include "BasicJob.h"
#include "ClientInvoke.h"
#include "ErrorCode.h"
#include "PluginConstants.h"
#include "VolumeCommonStruct.h"
#include "ShareResourceManager.h"
#include "PluginTypes.h"
/* Module include path */
#include "File.h"
#include "Snowflake.h"
#include "Utils.h"
#include "system/System.hpp"

/* Backup included path */
#include "BackupMgr.h"
#include "ScanMgr.h"
#include "VolumeProtector.h"

namespace FilePlugin {

const std::string VOLUME_MOUNT_ENTRIES_JSON_NAME = "volume_mount_entries.json";
#ifdef _WIN32
const std::string VOLUME_LIVEMOUNT_PATH_ROOT = R"(C:\databackup\volumelivemount)";
const std::string VOLUME_GRANULAR_RESTORE_PATH_ROOT = R"(C:\databackup\volumegranular)";
#else
const std::string VOLUME_LIVEMOUNT_PATH_ROOT = "/mnt/databackup/volumelivemount";
const std::string VOLUME_GRANULAR_RESTORE_PATH_ROOT = "/mnt/databackup/volumegranular";
#endif

const std::string MOUNT_RECORD_JSON_EXTENSION = ".record.json";
const std::string DEFAULT_COPY_NAME = "volumeprotect";

struct VolumeSetMountDrive {
    std::string mountDirve;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountDirve, MountDirve)
    END_SERIAL_MEMEBER
};

class VolumeCommonService : public BasicJob {
public:
    VolumeCommonService();
    virtual ~VolumeCommonService() {};
    virtual int PrerequisiteJob() = 0;
    virtual int GenerateSubJob() = 0;
    virtual int ExecuteSubJob() = 0;
    virtual int PostJob() = 0;

protected:
    template<typename... Args>
    void ReportJobDetails(const PluginReportInfo& info = {}, Args... logArgs)
    {
        JobLogLevel::type logLevel = info.logLevel;
        SubJobStatus::type jobStatus = info.jobStatus;
        int jobProgress = info.jobProgress;
        std::string logLabel = info.logLabel;
        int64_t errCode = info.errCode;

        if (IsAbortJob() && jobStatus == SubJobStatus::RUNNING) {
            INFOLOG("Job is aborted, force change jobStatus to aborting");
            jobStatus = SubJobStatus::ABORTING;
            logLabel = "";
        }

        LogDetail logDetail {};
        AddLogDetail(logDetail, logLabel, logLevel, logArgs...);
        AddErrCode(logDetail, errCode);

        SubJobDetails subJobDetails;
        if (m_dataSize != 0) {
            DBGLOG("subJob:%llu, report dataSize:%llu", m_subJobId, m_dataSize);
            subJobDetails.__set_dataSize(m_dataSize);
        }

        INFOLOG("Enter ReportJobDetails, jobId:%s, subJobId:%s, jobStatus:%d, logLabel:%s, errCode:%d, speed:%d",
            m_jobId.c_str(), m_subJobId.c_str(), static_cast<int>(jobStatus), logLabel.c_str(), errCode, m_jobSpeed);
        ActionResult result;
        std::vector<LogDetail> logDetailList;
        REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, jobProgress, m_jobSpeed, jobStatus);
        if (result.code != Module::SUCCESS) {
            ERRLOG("ReportJobDetails failed, jobId: %s, subJobId: %s, errCode: %d",
                m_jobId.c_str(), m_subJobId.c_str(), result.code);
        }
        INFOLOG("Exit ReportJobDetails, jobId:%s, subJobId:%s, jobStatus:%d, logLabel:%s, errCode:%d",
            m_jobId.c_str(), m_subJobId.c_str(), static_cast<int>(jobStatus), logLabel.c_str(), errCode);
        return;
    }

    template<typename... Args>
    void ReportJobLabel(const JobLogLevel::type logLevel, std::string logLabel, Args... logArgs)
    {
        ReportJobDetails({
            logLevel,
            SubJobStatus::RUNNING,
            PROGRESS0,
            logLabel,
            INITIAL_ERROR_CODE},
            logArgs...);
    }

    std::string GetMountDevicetype(const std::string& mountType, const std::string& volumePath, const std::string moduleType) const;
    void SetMainJobId(const std::string& jobId);
    void SetSubJobId();
    bool UpdateSubRestoreStats();
    void SetJobCtrlPhase(const std::string& jobCtrlPhase);
    bool InitIdGenerator();
    bool InitVolumeInfo();
    bool InitGeneralResource(const std::string& path);
    void SerializeVolumeStats(const volumeprotect::task::TaskStatistics& backupStats, uint64_t volumeCount,
        VolumeReportStatistic& backupStatistic);
    bool IsRestoreStatusInProgress(SubJobStatus::type& jobStatus);
    bool UpdateMainBackupStats(VolumeReportStatistic& mainStats, bool forceReport = false);
    bool MonitorRestore(SubJobStatus::type& jobStatus);
    bool ReportBackupRunningStats(VolumeReportStatistic mainStats, bool forceReport);
    bool UpdateCopyPhaseStartTime();
    int ReportJobProgress(SubJobStatus::type &jobStatus);
    bool ReportBackupCompletionStatus();

    bool CreateSubTask(const SubJob &subJob);
    size_t SetRequestId(const std::string& jobId, const std::string& subJobId);
    void CalcuSumStructBackupStatistic(VolumeReportStatistic& mainStats, VolumeReportStatistic& subStats);
    bool InitSubBackupJobResource();
    uint64_t CalculateSizeInKB(const uint64_t bytes) const;
    /* used for indexing, granular restore, livemount */
    bool MountNasShare(
        const AppProtect::StorageRepository& repository,
        const std::string& targetPath,
        const std::string& extendInfo,
        std::string& driveInfo) const;

    bool UmountNasShare(
        const AppProtect::StorageRepository& repository,
        const std::string& targetPath,
        const std::string& extendInfo) const;

    void ForceUmountNasShare(const std::string& nasShareMountTarget) const;

    bool MountSingleVolumeReadWrite(
        const std::string& volumeName,
        const std::string& volumeDirPath,
        const std::string& volumeMountTarget,
        const std::string& outputRecordDirPath,
        std::string& mountRecordPath) const;

    bool MountSingleVolumeReadOnly(
        const std::string& volumeName,
        const std::string& volumeDirPath,
        const std::string& volumeMountTarget,
        const std::string& outputRecordDirPath,
        std::string& mountRecordPath) const;

    bool UmountSingleVolume(
        const std::string& mountRecordJsonPath) const;

    std::vector<std::string> GetAllvolumeNameList(const std::string& dataFsPath) const;

    void MergeBackupFailureRecords() const;
protected:
    /* sub job id */
    std::string m_subJobId;
    /* Data size copied to destination in KB for subjob */
    uint64_t m_dataSize {0};
    /* current sub job backup speed (KB/S) */
    uint32_t m_jobSpeed = 0;
    /* current job phase: { PrerequisiteJob, GenerateSubJob, ExecuteSubJob, PostJob } */
    std::string m_jobCtrlPhase;

    /* Generate unique Id by using snowflake algorithm to avoid suspension caused by insufficient system entropy */
    std::shared_ptr<Module::Snowflake> m_idGenerator { nullptr };
    /* Root Path Of Backup Failure Recorder Output */
    std::string m_failureRecordRoot;
    /* Maximum number of records in the failure record file */
    uint64_t m_maxFailureRecordsNum = 0;
    /* Number of running subtasks of the current task */
    static uint32_t m_numberOfSubTask;
    /* Scanner statistics - saved in SharedResource */
    VolumeScanStatistics m_scanStats {};

    std::string m_cacheFsPath {};
    std::string m_metaFsPath {};
    std::string m_dataFsPath {};
    std::string m_metaFilePath {}; // for linux , metaFilePath is dataFsPath, for win , metaFilePath is metaFsPath
    std::string m_statisticsPath {};
    StorageRepository m_dataFs;
    StorageRepository m_metaFs;
    StorageRepository m_cacheFs;

    uint32_t m_subTaskType { 0 };
    VolumeNativeGeneral m_generalInfo;
    int64_t m_errorCode = 0;
    VolumeReportStatistic m_subBackupStats;
    JobType m_jobType = JobType::RESTORE;
    std::shared_ptr<volumeprotect::task::VolumeProtectTask> m_backup;

    volumeprotect::task::TaskStatus m_backupStatus {volumeprotect::task::TaskStatus::INIT};
    ErrCodeType m_volumeBackupErrorCode { volumeprotect::VOLUMEPROTECT_ERR_SUCCESS };
};
}
#endif // VOLUME_COMMONSERVICE_H