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
#ifndef HOST_BACKUP_H
#define HOST_BACKUP_H

#include <unordered_map>
#include "host/HostCommonService.h"
#include "snapshot_provider/SnapshotProvider.h"
#include "constant/Defines.h"
#include "FsDevice.h"
#include "DeviceMount.h"
#include "Backup.h"
#include "parser/MetaParser.h"
#include "parser/ParserStructs.h"

namespace FilePlugin {
class HostBackup : public HostCommonService {
public:
    HostBackup();
    ~HostBackup();
    EXTER_ATTACK int CheckBackupJobType();
    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;
private:
    int CheckBackupJobTypeInner();
    int PrerequisiteJobInner();
    int GenerateSubJobInner();
    int ExecuteSubJobInner();
    int ExecuteCopyMetaSubJobInner();
    int ExecuteTeardownSubJobInner();
    int ExecuteCheckSubJobInner();
    int ExecuteBackupSubJobInner(BackupSubJob backupSubJob);
    int HandleAggregatedDirPhase(const std::string& controlFile);
    int PostJobInner();
    void ScanPrimalSnapshotVol1(std::string& prefix, std::string srcVolId, ScanConfig& scanConfig,
		std::map<std::string, std::string> driverLetterMapper, std::vector<std::string>& scanSrcPaths);
    bool CreateDirAndMountforSubVolumes(const std::string& subSnapMntPath, const std::string& path,
        const std::string subSnapVolDevice, std::shared_ptr<FilePlugin::SnapshotProvider> shotProviderPtr);
    bool GetBackupJobInfo();
    bool CreateCtrlDirectory(const std::string &path);

    /* Init job info */
    bool InitJobInfo();
    bool InitFilesetInfo();
    void FilesetPathDeduplication();
    bool InitDataLayoutInfo();
    bool InitMetaDataCacheBackupFs();
    void InitRepoPaths();
    bool SetupCacheFsForBackupJob() const;
    void UpdateJobStartTime();
    void PrintJobInfo() const;
    bool GetPrevBackupCopyInfo();
    void CloseAggregateSwitchByBackupType();
    bool FilterProtectPaths();
    void GenerateCopyOsFlagRecord() const;
    bool WriteScannSuccess(std::set<std::string>& jobInfoSet);
    bool CheckScanRedo(std::set<std::string>& jobInfoSet);
    void HoldGenerateSubTaskAndKeepLive();
    void RemoveSnapDir(const std::string& jobId);

    /* Snapshort */
    std::shared_ptr<SnapshotProvider> BuildSnapshotProvider() const;
    bool CreateSnapshot();
    void DeleteSnapshot();
    void ReportSnapshotResult(
        const std::set<std::string>& spaceless, const std::string& snapNames, const std::string& notSupports);
    bool MountSnapshot(
        const std::string& path,
        std::shared_ptr<FilePlugin::SnapshotProvider> shotProviderPtr,
        const SnapshotResult& snapshotResult,
        std::string& snapshotsId,
        std::set<std::string>& mountedPaths);
        
    bool MountSnapshotForSubVolumes(
        const std::string& path,
        std::shared_ptr<FilePlugin::SnapshotProvider> shotProviderPtr,
        const SnapshotResult& snapshotResult,
        std::string& snapshotNames,
        std::set<std::string>& mountedPaths);

    bool WriteSnapInfoToFile(const std::set<std::string>& snapshotInfo, const std::string& infoFilePath);
    void ReadSnapInfoFromFile(std::set<std::string>& snapshotInfo, const std::string& infoFilePath);

    /* Scanner */
    bool ScanPrimalSourceVol(HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet);
    bool ScanPrimalSourceVolForNfs(HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet);
    bool ScanPrimalSnapshotVol(HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet);
    bool ScanSubVolume(HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet);
    bool ScanFailedVolume(HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet);
    bool EnqueueSubjobInfo(std::set<std::string>& jobInfoSet, const std::string& prefix = "");
    std::string GetVolumeMountPath(const std::string& path) const;
    void GetBackupSubVolumesPath();
    void FillScanConfig(ScanConfig &scanConfig);
    void FillScanConfigMetaPath(ScanConfig &scanConfig, std::string pathId);
    void FilterAllSubVol(ScanConfig &scanConfig);
    void FilterSubVol(ScanConfig &scanConfig, std::string path, const std::string& prefix = "");
    void FillScanAclConfig(ScanConfig &scanConfig, const std::string& path) const;
    bool StartScanner(
        const ScanConfig& scanConfig, const std::vector<std::string>& paths, const std::string& prefix = "");

    void skipSubVolumeDir(const std::string& prefix, const std::string& path, ScanConfig& scanConfig);
    void FillScanFilterConfig(ScanConfig &scanConfig);
    void FillCrossFilterConfig(ScanConfig &scanConfig);
    void FillInvailMountPoints(ScanConfig &scanConfig);
    bool IsProtectedSubPath(const std::string& subPath);
    void FillScanFilterType(const std::string& mode, FILTER_TYPE& filterType);
    int ZipSubMetaFileToMetaRepo(const std::string& metaDirName);
    int ZipFinalMeta() const;
    int ZipMetaThread(const std::vector<std::string>& subVolMetaDirName);
    ScanStatistics MonitorScanner(HostScanStatistics &scanStats, SubJobStatus::type &jobStatus,
        std::string &jobLogLabel);
    void FillMonitorScannerVarDetails(SCANNER_TASK_STATUS &scanTaskStatus, SubJobStatus::type &jobStatus,
        std::string &jobLogStr);
    bool HandleScanCompletion(const std::set<std::string>& jobInfoSet);

    /* Scanner CB */
    static void GeneratedCopyCtrlFileCb(const void *usrData, std::string ctrlFile);
    static void GeneratedHardLinkCtrlFileCb(const void *usrData, std::string ctrlFile);

    /* Scanner runing status report */
    bool ReportScannerRunningStats(const HostScanStatistics &preScanStats);
    void ReportScannerCompleteStatus();
    bool SendJobReportForAliveness();

    /* *
     * API to create a backup sub job
     */
    bool CreateSubTasksFromCtrlFile(const std::string& jobInfoStr, SubJobStatus::type& jobStatus);
    bool CreateBackupSubJobTask(const uint32_t &subTaskType);
    bool GenerateSubJobList(std::vector<SubJob> &subJobList, std::vector<std::string> &ctrlFileList,
        const std::string &scanCtrlFile, const std::string &backupCtrlFileInCacheFsPath, const std::string& prefix);
    bool UpdateCopyPhaseStartTimeInGenRsc();

    bool IsFullBackup() const;
    bool IsSubTaskStatsFileExists() const;
    bool NeedChangeIncToFull();

    /* Backup */
    void FillBackupConfig(BackupParams &backupParams, BackupSubJob &backupSubJob);
#ifdef WIN32
    void FillBackupConfigWin32(BackupParams &backupParams, BackupSubJob &backupSubJob);
#else
    void FillBackupConfigPosix(BackupParams &backupParams, BackupSubJob &backupSubJob);
#endif
    void FillCommonParams(CommonParams& commonParams);
    void FillFailureRecorderParams(CommonParams &commonParams);
    void FillBackupConfigPhase(BackupParams &backupParams, BackupSubJob &backupSubJob);
    void PrintBackupConfig(const BackupParams& backupParams);
    bool StartBackup(BackupSubJob backupSubJob);
    HostCommonService::MONITOR_BACKUP_RES_TYPE MonitorBackup(SubJobStatus::type &jobStatus);
    bool UpdateSubBackupStats(bool forceComplete);
    void HandleMonitorStuck(SubJobStatus::type &jobStatus);
    bool UpdateMainBackupStats(BackupStatistic& mainStats);
    bool ReportBackupRunningStats(const BackupStatistic& backupStats);
    bool ReportBackupCompletionStatus();
    bool HandleCacheDirectories() const;
    bool SaveScannerMeta() const;
    void FillBackupCopyInfo(HostBackupCopy &backupCopy);

    bool IsAggregate() const;
    bool PostReportCopyAdditionalInfo();
    bool GetAggCopyExtendInfo(std::string& jsonString);
    void FillAggregateFileSet(std::vector<std::string>& aggregateFileSet);

    /* Report job progress */
    int ReportJobProgress(SubJobStatus::type &jobStatus);
    void KeepJobAlive();

    /* Init Shared Resources */
    bool InitSubBackupJobResources();
    void DeleteSharedResources() const;

    /* Handle advance params */
    void GetExcludeSubPath(const std::string& path, std::set<std::string>& excludePathList);
    bool IsBackupDevice(std::shared_ptr<FsDevice> fsDevicePtr);
    bool IsBackupPath(std::string path);

    /* Aggregate backup skips phases */
    bool IsAggregateSkipeSubJob(uint32_t subTaskType);

    bool RecordResidualSnapshots(const std::set<std::string>& snapshotInfos);
    bool UpdateResidualSnapshots(const std::set<std::string>& snapshotInfos);
    void SendAlarmForResidualSnapshots(const std::set<std::string>& snapshotInfos);
    void ClearResidualSnapshotsAndAlarm();
    bool SetNumOfChannels() const;

    std::string LoadSnapshotParentPath() const;
    std::vector<std::string> LoadExcludeFileSystemList() const;
    void LoadExcludePathList();

    void FillScanConfigMapFunc(ScanConfig& scanConfig, std::string& orginalMntPoint);
    void ExcludePathsInConfig(ScanConfig &scanConfig, const std::string& pathFix = "");
    void UpdateTaskInfo();
    void HandleFailedRecord(const std::vector<std::string>& subVolMetaDirName);
    void HandleUpdateMeta(const std::string& failedRecordPath, const std::string& subVolDir);
    void DoRealUpdateMeta(std::unordered_map<uint64_t, Module::FileMeta>& tmpRecordMap,
        const std::unique_ptr<Module::MetaParser>& metaParser);
    bool DoRealBackup(const BackupSubJob& backupSubJob, SubJobStatus::type& jobStatus,
        HostCommonService::MONITOR_BACKUP_RES_TYPE& ret, int retryCnt);
    bool DoRealScanFailedVolume(const std::vector<std::string>& failedRecords,
        HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet);
    bool IsErrNeedSkip(const uint32_t errNUm);
    bool ProcessFailedRecordLine(const std::string& line, FailedRecordItem& item);

private:
    std::shared_ptr<AppProtect::BackupJob> m_backupJobPtr { nullptr };

    /* RequestId for mainJob and subJob */
    size_t m_mainJobRequestId { 0 };
    size_t m_subJobRequestId { 0 };

    /* Secondary Storage dataFS Info */
    StorageRepository m_dataFs;
    std::string m_dataFsPath;
    std::string m_dataFsLocalMountPath;
    std::string m_localStorageIps;

    /* Secondary Storage metaFS Info */
    StorageRepository m_metaFs;
    std::string m_metaFsPath;
    std::string m_incControlDir;  // 用于存放增量生成的控制文件，给增量恢复用

    /* Secondary Storage cacheFs Info */
    StorageRepository m_cacheFs;
    std::string m_cacheFsPath;
    std::string m_statisticsPath;
    std::string m_scanMetaPath;
    std::string m_scanControlPath;
    std::string m_backupControlPath;
    std::string m_scanStatusPath;
    bool m_scanRedo {false};

    /* Protected HOST Share Information */
    ProtectedFileset m_fileset {};

    /* Data layout config information */
    DataLayOutExtend m_dataLayoutExt {};

    /* Pre backup copy info genereted in last job */
    HostBackupCopy m_prevBackupCopyInfo {};

    time_t m_lastBackupTime { 0 };

    /* key: mountpoint, value: device number */
    std::map<std::string, uint64_t> m_subVolInfo;

    /* Is scanner restarted */
    bool m_isScannerRestarted { false };

    /* Backup instance */
    std::unique_ptr<FS_Backup::Backup> m_backup { nullptr };

    /* Backup statistics - saved in SharedResource */
    BackupStatistic m_subBackupStats;

    std::atomic<bool> m_isCopying { false };
    std::atomic<bool> m_isCopySuccessFlag { true };
    std::atomic<bool> m_isBackupInProgress {false};
    std::atomic<bool> m_JobComplete {false};
    int64_t m_lastJobReportTime { 0 };  // record the reporting time

    /* SubTask type: CopyPhase/DelPhase/HardLinkPhase/DirMTimePhase */
    uint32_t m_subTaskType { 0 };

    /* JSON file (/m_metaFs/backup-copy-meta.json) where the meta info about the backup copy is stored */
    std::string m_backupCopyInfoFilePath;

    /* Cross file system backup */
    std::shared_ptr<FilePlugin::DeviceMount> m_deviceMountPtr {nullptr};

    std::set<std::string> m_subVolPathWithSnap;
    std::set<std::string> m_filesetPathsWithSnap;
    std::set<std::string> m_allSnapVolumeInfos;
    std::string m_subVolSnapInfoFilePath;
    std::string m_filesetSnapInfoFilePath;
    std::string m_filesetSnapIdFilePath;
    HostSnapResidualInfoList  m_snapResidualInfos;

    /* paths of Nfs3 in primiayVolume */
    std::vector<std::string> m_protectedPathsForNfs;

    /* Used for reporting job progress log to agent */
    ActionResult m_logResult {};
    SubJobDetails m_logSubJobDetails {};
    LogDetail m_logDetail {};
    std::vector<LogDetail> m_logDetailList;

    static uint32_t m_numberOfSubTask;
    std::vector<std::string> m_excludePathList;
    std::string m_volumeName;
    uint64_t m_numberOfFailedFilesScaned {0};
};
}
#endif