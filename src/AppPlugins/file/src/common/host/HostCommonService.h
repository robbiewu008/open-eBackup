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
#ifndef HOST_COMMONSERVICE_H
#define HOST_COMMONSERVICE_H
 
#include <unordered_map>
#include "BasicJob.h"
#include "client/ClientInvoke.h"
#include "system/System.hpp"
#include "utils/PluginUtilities.h"
#include "constant/PluginConstants.h"
#include "constant/ErrorCode.h"
#include "statistics/ShareResourceManager.h"
#include "HostCommonStruct.h"
#include "Backup.h"
#include "Module/src/common/Utils.h"
#include "Module/src/common/File.h"
#include "Module/src/common/Snowflake.h"
#include "MtimeCtrlParser.h"
#include "BackupMtimeCtrl.h"

/* Scanner "HostCommonService" */
#include "Scanner.h"
#include "ScanConfig.h"

/* Backup "HostCommonService" */
#include "BackupMgr.h"

namespace FilePlugin {
class HostCommonService : public BasicJob {
public:
    HostCommonService();
    virtual ~HostCommonService() {};
    virtual int PrerequisiteJob() = 0;
    virtual int GenerateSubJob() = 0;
    virtual int ExecuteSubJob() = 0;
    virtual int PostJob() = 0;
    enum class MONITOR_BACKUP_RES_TYPE {
        MONITOR_BACKUP_RES_TYPE_SUCCESS          = 0,
        MONITOR_BACKUP_RES_TYPE_FAILED           = 1,
        MONITOR_BACKUP_RES_TYPE_ABORTED          = 2,
        MONITOR_BACKUP_RES_TYPE_NEEDRETRY        = 3
    };
protected:
    template<typename... Args>
    void ReportJobDetailsWithLabelAndErrcode(
        const std::tuple<JobLogLevel::type, SubJobStatus::type, const int>& reportInfo,
        std::string logLabel, int64_t errCode, Args... logArgs)
    {
        SubJobDetails subJobDetails;
        LogDetail logDetail {};
        std::vector<LogDetail> logDetailList;
        ActionResult result;
        SubJobStatus::type jobStatus = std::get<1>(reportInfo);
        if (IsAbortJob() && jobStatus == SubJobStatus::RUNNING) {
            INFOLOG("Job is aborted, force change jobStatus to aborting");
            jobStatus = SubJobStatus::ABORTING;
            logLabel = "";
        }
        AddLogDetail(logDetail, logLabel, std::get<0>(reportInfo), logArgs...);
        AddErrCode(logDetail, errCode);
        logDetail.__set_additionalDesc(std::vector<std::string>{m_errMsg});
        if (m_dataSize != 0) {
            subJobDetails.__set_dataSize(m_dataSize);
        }
        INFOLOG("Enter ReportJobDetails: jobId: %s, subJobId: %s, jobStatus: %d, logLabel: %s, errCode: %d, errMsg: %s,"
            " speed: %d", m_jobId.c_str(), m_subJobId.c_str(), static_cast<int>(jobStatus), logLabel.c_str(),
            errCode, m_errMsg.c_str(), m_jobSpeed);
        constexpr uint8_t index2  = 2;
        REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail,
            std::get<index2>(reportInfo), m_jobSpeed, jobStatus);
        if (result.code != Module::SUCCESS) {
            ERRLOG("ReportJobDetails failed, jobId: %s, subJobId: %s, errCode: %d",
                m_jobId.c_str(), m_subJobId.c_str(), result.code);
        }
        INFOLOG("Exit ReportJobDetails: jobId: %s, subJobId: %s, jobStatus: %d, logLabel: %s, errCode: %d",
            m_jobId.c_str(), m_subJobId.c_str(), static_cast<int>(jobStatus), logLabel.c_str(), errCode);
        return;
    }

    template<typename... Args>
    void ReportJobDetails(
        const std::tuple<const JobLogLevel::type, SubJobStatus::type, const int>& reportInfo, Args... logArgs)
    {
        ReportJobDetailsWithLabelAndErrcode(reportInfo, "", INITIAL_ERROR_CODE, logArgs...);
    }

    template<typename... Args>
    void ReportJobLabel(const JobLogLevel::type logLevel, std::string logLabel, Args... logArgs)
    {
        ReportJobDetailsWithLabelAndErrcode(std::make_tuple(logLevel, SubJobStatus::RUNNING, PROGRESS0),
            logLabel, INITIAL_ERROR_CODE, logArgs...);
    }

    void SetMainJobId(const std::string& jobId);
    void SetSubJobId();
    void SetJobCtrlPhase(const std::string& jobCtrlPhase);

    /* Backup Statistic */
    BackupStatistic GetIncBackupStats(const BackupStats& currStats, BackupStatistic prevStats);
    void SerializeBackupStats(const BackupStats& backupStats, BackupStatistic& backupStatistic);

    /* Scanner */
    bool CheckFilePathAndGetSrcFileList(std::string srcDir, std::string dstDir,
                                               std::vector<std::string> &srcFileList);
    bool IsValidCtrlFile(const std::string ctrlFileFullPath) const;

    bool InitSubTask(SubJob &subJob, const std::string& ctrlFile, bool ignoFail = false,
        const std::string& prefix = "", const std::string& fsId = "", const std::string& parentDir = "");
    void GetSubTaskInfoByFileName(const std::string &fileName, std::string &subTaskName,
                                        uint32_t &subTaskType, uint32_t &subTaskPrio);
    uint32_t GetSubJobTypeByFileName(const std::string &fileName) const;

    /* API to update statistics */
    void UpdateScannerStats(const HostScanStatistics& preStats = {});
    void PrintScannerStats() const;
    void PrintBackupStats(const BackupStatistic &backupStats, const std::string& jobId, bool completed = false) const;

    /**
     * API to create a sub-task
     */
    void PrintSubJobInfo(std::shared_ptr<SubJob> &m_subJobInfo);
    bool CreateSubTask(std::vector<SubJob> &subJobList, std::vector<std::string> &ctrlFileList);
    bool CreateSubTask(const SubJob &job);

    /**
     * API to Init general resource
     */
    bool InitGenerateResource(const std::string& path);

    std::string CheckMetaFileVersion(const std::string& metaPath) const;
    int ReadDirCountForMtimeStats(const std::string& control, const std::string& metaPath) const;
    bool InitIdGenerator();
    void MergeBackupFailureRecords();
    bool IsBackupStatusInprogress(SubJobStatus::type &jobStatus) const;
    uint64_t CalculateSizeInKB(uint64_t bytes) const;
    bool SetNumOfChannels(const std::string& channelCntStr) const;

    BackupStats m_backupStats;
    uint64_t m_curProcess = 0;
    std::string m_subJobId;
    std::atomic<int> m_jobProgress {0};
    /* Data size copied to destination in KB for subjob */
    std::atomic<double> m_dataSize {0.0};
    uint32_t m_jobSpeed = 0;
    std::string m_jobCtrlPhase;

    /* Scanner instance */
    std::unique_ptr<Scanner> m_scanner { nullptr };

    /* Subtask completion status of scan task */
    SCANNER_STATUS m_scanStatus {};
    /* Subtask completion status of backup task */
    BackupPhaseStatus m_backupStatus {BackupPhaseStatus::INPROGRESS};
    /* Scanner statistics - saved in SharedResource */
    HostScanStatistics m_scanStats {};
    /* General info about backup job - saved in SharedResource */
    NativeGeneral m_generalInfo {};
    /* Generate UniqueId */
    std::shared_ptr<Module::Snowflake> m_idGenerator { nullptr };
    /* Root Path Of Backup Failure Recorder Output */
    std::string m_failureRecordRoot {};
    uint64_t m_maxFailureRecordsNum = 0;
    /* 插件配置文件读写锁 */
    static std::mutex m_pluginAtrributeJsonFileMutex;
    int m_errCode = 0;
    std::string m_errMsg {};
};
}
#endif