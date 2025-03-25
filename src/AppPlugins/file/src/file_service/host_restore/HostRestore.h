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
#ifndef HOST_RESTORE_H
#define HOST_RESTORE_H
#include <memory>
#include <mutex>
#include "host/HostCommonService.h"
#include "Backup.h"
#include "ScanConfig.h"
#include "Scanner.h"
#include "constant/Defines.h"
#ifdef __linux__
#include "OsRestore.h"
#endif

namespace FilePlugin {
struct SubJobInfo {
    uint32_t copyOrder = 0;
    std::string controlFileName;
    uint32_t subJobType;
    std::string metaFilePath;
    std::string dcacheAndFcachePath;
    std::string dataCachePath;
    std::string extendInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyOrder, copyOrder)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(controlFileName, controlFileName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(subJobType, subJobType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaFilePath, metaFilePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dcacheAndFcachePath, dcacheAndFcachePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dataCachePath, dataCachePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(extendInfo, extendInfo)
    END_SERIAL_MEMEBER
};

struct TapeCopyExtendInfo {
    AggCopyExtendInfo extendInfo;
    std::string metaPath;
    std::string metaPathFSID;
    std::string multiFileSystem;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(extendInfo, extendInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaPath, metaPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaPathFSID, metaPathFSID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(multiFileSystem, multiFileSystem)
    END_SERIAL_MEMEBER
};

struct HostTaskInfo {
    time_t startTime = 0;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(startTime, startTime)
    END_SERIAL_MEMEBER
};

struct CopyControlFilePath {
    std::string scanContrlFilePath;
    std::string restoreContrlFilePath;
    std::string dcaheAndFcachePath;
};

class HostRestore : public HostCommonService {
public:
    HostRestore() {};
    ~HostRestore() {};
    int PrerequisiteJob() override;
    int GenerateSubJob() override;
    int ExecuteSubJob() override;
    int PostJob() override;

private:
    int PrerequisiteJobInner();
    int GenerateSubJobInner();
    int ExecuteSubJobInner();
    int PostJobInner();

    void KeepPluginAlive();
    int GenerateAggregateSubJobInner();
    int InitAggregateGenerateJobInfo();
    int GetRepoInfoForAggregate();
    int GetCopyExtendInfo(const int& copyOrder, AggCopyExtendInfo& aggCopyExtendInfo);
    int RecordControlFilePathForAggregate();
    int CreateSrcDirForAggregate() const;
    int UntarDcacheAndFcachePackageForAggregate();
    int GenerateRestoreExecuteSubJobsForAggregate();
    int GeneralSubJobByLastControl(int volumeOrder);
    int StartToGenerateControlFileForAggregate(ScanConfig& scanConfig);
    int GetMetaZipDirListForAggregate();
    int HandleSingleCopyForAggregate();

    int GenerateRestoreExecuteSubJobsForAggregateFineGrained();
    int HandleCopyForAggregateFineGrained();
    int HandleThePenultimateCopy();
    int HandleTheLastCopy();

    void GetRestoreType();
    int InitGenerateJobInfo();
    int GetRepoInfo();
    int UntarWholePackage(std::string zipPath, std::string targetPath) const;
    int UntarDcachefiles(int volumeOrder) const;
    int AddFilterRule(ScanConfig& scanConfig);
    int StartToScan(ScanConfig& scanConfig);
    int CreateSrcDir() const;
    bool CreateRestoreSubJob();
    bool WriteScannSuccess();
    int InitialRestoreLocalProgressInfo(std::string jobId) const;
    int GenerateRestoreExecuteSubJob(std::vector<std::string> contrlFileList, bool isDelControlFile = true);
    int GenerateSubJobByDcacheAndFcache(int volumeOrder);
    void FillScanConfig(ScanConfig& scanConfig, int volumeOrder);
    void SpecialHandelScanConfig(ScanConfig& scanConfig, int volumeOrder);
    bool ExistPreviousVolume(int volumeOrder, int& previousVolumeOrder) const;
    int GetMetaZipDirList(std::string targetPath);
    static void GeneratedCopyCtrlFileCb(void *usrData, std::string ctrlFile);
    static void GeneratedHardLinkCtrlFileCb(void *usrData, std::string ctrlFile);
    int GetControlFileList(std::string controlFilePath, std::vector<std::string> &contrlFileList);
    int InitSubJobInfo(SubJob &subJob, const std::string controlFileFullPath, const std::string& metaPath);
    void GetSubJobInfoByFileName(
        const std::string &fileName, std::string &subTaskName, int &subTaskType, int &subTaskPrio);
    int GenerateTearDownSubJob();
    int MonitorScannerProgress(int volumeOrder);
    void PrintImportInfo() const;
    void CalcuScannerStatistic();
    int UpdateRestoreScanStatistic(std::unique_ptr<Scanner> scanner);
    int ExecuteCheckSubJobInner();
    int GenerateCheckSubJob();
    int GetExecuteSubJobType();
    int FinalReportStatictisInfo();
    int ReportRestoreCompletionStatus();
    int InitExecuteJobInfo();
    int GetRestoreCoverPolicy();
    int ReportSubJobToAgent(std::vector<SubJob> &subJobList, std::vector<std::string> &ctrlFileList,
        bool isDelControlFile = true);
    int StartToRestore();
    int GetSpecialConfigForRestore();
    void UpdateTaskInfo();
    inline bool GetIgnoreFailed();
    void FillCommonParams(CommonParams &commonParams, size_t subJobRequestId);

    /* fill restore config for according to different platform */
    void FillRestoreConfig(BackupParams& backupParams);
    void FillRestoreConfigForPosix(
        BackupParams&       backupParams,
        const std::string&  resourcePath,
        const std::string&  destinationPath);
#ifdef WIN32
    void FillRestoreConfigForWin32(
        BackupParams&       backupParams,
        const std::string&  resourcePath,
        const std::string&  destinationPath);
    bool ReadBackupCopyInfo(HostBackupCopy& hostBackupCopy);
#endif

    void ReportCopyPhaseStart();
    int SpecialDealRestoreConfig(BackupParams& backupParams);
    template<typename... Args>
    bool ReportJobMoreDetails(const JobLogLevel::type &logLevel, SubJobStatus::type jobStatus,
                              int32_t jobProgress, std::string logLabel,  Args... logArgs);
    HostCommonService::MONITOR_BACKUP_RES_TYPE MonitorRestoreJobStatus();
    bool UpdateSubBackupStats(bool forceComplete);
    void HandleMonitorStuck(SubJobStatus::type &jobStatus);
    void UpdateSpeedAndReport();
    bool CalcuMainBackupStats(BackupStatistic& mainBackupStats) const;
    bool ReportBackupRunningStatus(BackupStatistic& mainBackupStats);
    void CheckScanRedo();
    int GetCopyOrder(const std::string& metaPath);
    int InitialReportSpeedInfo() const;
    std::string GetDiffDir(std::string& path);
    int GetCacheRepositoryPath();
    int DeleteSrcDirForRestore() const;
    int DeleteReportSpeedInfo() const;
    void CalcuSpeed(BackupStatistic& mainBackupStats, time_t startTime);
    std::string GetLastCtrlPath();

#ifdef __linux__
    bool OSConfigRestore();
    bool CheckBMRCompatible();
    bool InitDiskMapInfo();
#endif

private:
    StorageRepository m_cacheFs {};
    std::string m_cacheFsPath;
    std::string m_metaOriPath;
    StorageRepository m_dataFs {};
    std::string m_dataFsPath;

    StorageRepository m_metaFs {};
    std::string m_metaFsPath;
    std::string m_scanStatusPath;
    std::string m_backupCopyInfoFilePath;

    std::string m_sysInfoPath;
    std::string m_lvmInfoPath;
#ifdef __linux__
    std::vector<DiskMapInfo> m_diskMapInfoSet;
#endif

    bool m_scanRedo {false};

    bool m_fineGrainedRestore {false};
    bool m_aggregateRestore {false};
    bool m_incrementalRestore {false};
    bool m_tapeCopy {false};
    std::atomic<bool> m_generateSubjobFinish { false };
    std::atomic<bool> m_isRestoreInProgress { false };

    std::shared_ptr<AppProtect::RestoreJob> m_restoreJobInfo {nullptr}; // 记录恢复任务里所有的参数信息

    std::shared_ptr<Scanner> m_scanner;
    SCANNER_STATUS m_scannerStatus;
    ScanStatistics m_scannerStatistic;

    std::unique_ptr<FS_Backup::Backup> m_backup {};
    BackupStatistic m_addedBackupStatistic;
    BackupPhase m_backupPhase;
    /* subBackup statistics - saved in SharedResource */
    BackupStatistic m_subBackupStats;

    time_t m_lastReportTime = 0;
    time_t m_startTime = 0;

    bool m_finalReportStatistic {false}; // 恢复任务执行完后上报总的统计信息
    bool m_checkSubJob {false}; // 恢复任务执行完，检查是否存在文件/目录恢复失败，如果恢复失败，则部分成功
    std::string m_scanContrlFilePath; // 临时存放扫描生成的控制文件
    std::string m_restoreContrlFilePath; // 控制文件最终的存放路径，用于恢复的执行子任务
    std::vector<std::string> m_currentMetaZipDirFullPathList; // 记录当前副本各个卷的metafile.zip所在目录全路径;
    std::vector<std::string> m_currentMetaZipDirNameList; // 记录当前副本各个卷的metafile.zip所在目录名称;
    std::vector<std::string> m_previousMetaZipDirList; // 记录上一个副本各个卷的metafile.zip所在目录的全路径;
    uint32_t m_singleCopyVolumeNumber; // 记录单个副本下卷的总数
    SubJobInfo m_subJobPathsInfo; // 记录子任务所需的信息
    std::string m_restorePath;  // 恢复的目标路径
    RestoreReplacePolicy m_coveragePolicy; // 覆盖策略

    std::string m_restoreMetaPath; // 非聚合下用于存放扫描生成的metefile文件以及解压生成的dcache、fcache、metefile

    std::vector<std::string> m_dataFsPathList; // 聚合格式下，存储所有副本的data仓路径
    std::vector<std::string> m_metaFsPathList; // 聚合格式下，存储所有副本的meta仓路径
    std::vector<std::string> m_dcacheAndFcachePathForCopies; // 聚合格式下，用于存放解压生成的dcache、fcache文件
    uint32_t m_orderNumberForCopies {0}; // 聚合格式下，代表副本顺序
    uint32_t m_numberCopies {0}; // 聚合格式下，记录副本的总数

    uint32_t m_maxSizeAfterAggregate; // 聚合格式下， 聚合打包生成的文件的最大规格
    uint32_t m_maxSizeToAggregate; // 聚合格式下，超过此规格大小的文件不支持聚合
    static uint32_t m_numberOfSubTask;
};
}
#endif