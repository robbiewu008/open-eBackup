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
#ifndef HOST_ARCHIVE_RESTORE_H
#define HOST_ARCHIVE_RESTORE_H

#include "Module/src/common/JsonHelper.h"
#include "Backup.h"
#include "Scanner.h"

#include "BasicJob.h"
#include "HostCommonService.h"
#include "ArchiveClient.h"
#include "ArchiveDownloadFile.h"


namespace FilePlugin {
enum class ArchiveJobState {
    NONE = -1,
    SUCCESS = 0,
    FAILED = 1,
    RUNNING = 2,
    ABORT = 3,
    NASSHARE_ERROR = 4,
    EMPTY_COPY = 5,
    PARTIAL_SUCCESS = 6,
    SKIP_PHASE = 7
};

class HostArchiveRestore : public HostCommonService {
public:
    HostArchiveRestore() {}
    virtual ~HostArchiveRestore() {}
    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;

private:
    int PrerequisiteJobInner();
    int GenerateSubJobInner();
    int ExecuteSubJobInner();
    int PostJobInner();
    inline bool IsAbort() const;
    bool SendJobReportForAliveness();
    // init
    bool InitJobInfo();
    bool InitRepoInfo();
    bool InitRestoreInfo();
    bool InitArchiveInfo();
    bool InitInfo();
    inline bool IsFineRestore() const;
    bool InitCacheRepoDir();
    bool GenerateSubJobList(std::vector<SubJob> &subJobList, std::vector<std::string> &ctrlFileList,
        const std::string &dstCtrlFileRelPath, const std::string &ctrlFileFullPath);
    bool GenerateControlFile();
    bool InitSubDir(const std::string& dir);
    bool InitArchiveClient();
    // resource stats
    bool InitMainResources();
    bool InitSubBackupJobResources();
    bool QueryMainScanResources();
    bool GetBackupCopyInfo(const std::string& backupCopyInfo);
    // gen
    bool DownloadMetaFile();
    bool StartScanner();
    inline bool IsDir(const std::string& name) const;
    void AddFilterRule(ScanConfig& scanConfig);
    void FillScanConfig(ScanConfig& scanConfig);
    bool UnzipMetafileZip(const std::string& downloadMetePath, const std::string& outputMetaPath);
    void MonitorScan();
    bool ReportScannerRunningStatus();
    bool CreateSubTasksFromCtrlFile(SubJobStatus::type &jobStatus, bool isFinal = false);
    void ReportGenerateSubJob();
    void SetArchiveJobState();
    bool GetSubJobInfoByFileName(int copyOrder, const std::string &fileName, std::string &subTaskName,
        int &subTaskType, int &subTaskPrio);
    void GenerateReportStartRestoreSubJob();

    // excute
    bool GetRestorePolicy();
    bool StartRestore();
    BackupParams FillRestoreConfig(const BackupSubJob& subJobInfo);
    SubJobStatus::type MonitorRestore();
    bool UpdateBackupStatistics();
    bool UpdateMainBackupStats();
    void PrintBackupStatistics(const BackupStatistic &backupStatistic);
    bool ReportBackupRunningStatus();

    // post
    void ReportPostJob();

private:
    // common info
    std::string m_jobCtrlPhase; // 4 job phase
    std::shared_ptr<AppProtect::RestoreJob> m_jobInfoPtr {nullptr};
    // job info
    std::string m_jobId;
    std::string m_subjobId;
    std::string m_copyId;
    std::string m_resourceId;
    bool m_isAggCopy {false};
    AggCopyExtendInfo m_aggCopyInfo;
    int64_t m_lastJobReportTime {0};
    // restore info
    std::string m_statsPath;
    std::vector<std::string> m_restorePathList;
    std::vector<std::string> m_fineRestoreObj;
    std::string m_cacheMdPath;
    std::string m_volumePath;
    std::string m_scanInputMetePath;
    std::string m_downloadMetePath;
    std::string m_mdLatestPath;
    std::string m_scanControlFilePath;
    std::string m_backupControlFilePath;
    std::string m_sourceContextMd; // source_policy_3e789c80dba046c7bce7d4fa14e29d61_Context_Global_MD
    std::string m_sourceContext; // source_policy_3e789c80dba046c7bce7d4fa14e29d61_Context
    std::string m_dataPath;
    RestoreReplacePolicy m_coveragePolicy {RestoreReplacePolicy::NONE};
    // repo info
    std::string m_cacheFs;
    std::string m_cacheFsPath;
    std::string m_cacheFsRemotePath;
    // archive info
    ArchiveServerInfo m_archiveServerInfo;
    std::string m_archiveFsId;
    // restore job context
    std::string m_jobStartTime;
    ArchiveJobState m_jobState {ArchiveJobState::NONE};
    // gen
    bool m_isPrepareMetaFinish = false;
    int64_t m_lastScannerReportTime {0};
    HostScanStatistics m_totalScanStats {}; // include all vol scan info
    // excute
    BackupStatistic m_backupStats {};
    BackupStatistic m_subBackupStats {};
    // other object
    std::shared_ptr<ArchiveClient> m_archiveClient {nullptr}; // TO-DO 后续使用对象池存放多个归档客户端

    std::unique_ptr<FS_Backup::Backup> m_backup {nullptr};
};
}
#endif // HOST_ARCHIVE_RESTORE_H