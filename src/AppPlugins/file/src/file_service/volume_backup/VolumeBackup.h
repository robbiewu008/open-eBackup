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
#ifndef VOLUME_BACKUP_H
#define VOLUME_BACKUP_H

#include "constant/Defines.h"
#include "volume/VolumeCommonService.h"
#include "volume/VolumeCommonStruct.h"
#include "VolumeProtector.h"

namespace FilePlugin {
#define ENTER                                                                                                         \
    do {                                                                                                              \
        m_mainJobRequestId = GenerateHash(m_jobId);                                                                   \
        INFOLOG("Enter %s, jobId: %s, subJobId: %s", m_jobCtrlPhase.c_str(), m_jobId.c_str(), m_subJobId.c_str());    \
    } while (0)

#define EXIT                                                                                                          \
    do {                                                                                                              \
        INFOLOG("Exit %s, jobId: %s, subJobId: %s", m_jobCtrlPhase.c_str(), m_jobId.c_str(), m_subJobId.c_str());     \
    } while (0)

#ifdef WIN32
    const std::string SEP = "\\";
#else
    const std::string SEP = "/";
#endif
const std::string PREVIOUS = "previous";
const std::string LATEST = "latest";
const std::string SUBJOB_TYPE_VOLUME_JOBNAME = "VolumeBackup_CopyVolume";
const std::string RESIDUAL_SNAPSHORTS_INFO_FILE = "residual_snapshots.info";
const std::string ALARM_CODE_FAILED_DELETE_SNAPSHOT = "0x2064006F0001";
const std::string PLUGIN_CONFIG_KEY = "FilePluginConfig";
const std::string SNAPSHOT_PARENT_PATH_KEY  = "LinuxSnapshotParentPath";
const int SNAP_ERRNO_SPACELESS = 3;
const uint32_t BYTES_UNIT_RATE = 1024;
const std::string TRUE_STR = "true";
const std::string FALSE_STR = "false";
const std::string SHA_META = "meta.bin";

class VolumeBackup : public VolumeCommonService {
public:
    VolumeBackup();
    virtual ~VolumeBackup();
    EXTER_ATTACK int CheckBackupJobType();
    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;
protected:
    std::shared_ptr<AppProtect::BackupJob> m_backupJobPtr { nullptr };

    /* RequestId for mainJob and subJob */
    size_t m_mainJobRequestId { 0 };
    size_t m_subJobRequestId { 0 };

    int CheckBackupJobTypeInner();
    int PrerequisiteJobInner();
    int GenerateSubJobInner();
    int ExecuteSubJobInner();
    int PostJobInner();

    bool GetBackupJobInfo();
    void SetBackupJobInfo(const std::string jonPhase);
    bool InitJobInfoExecuteSubJob();
    virtual bool ScanVolumesToGenerateTask();
    virtual bool IsLimitedKernel();
    bool CreateBackupSubTask(VolumeBackupSubJob &backupSubJob, uint32_t stage);

    void PrintJobInfo() const;
    bool InitDataLayoutInfo();
    bool IsFullBackup() const;
    bool GenerateTearDownTask();

    virtual bool SaveVolumeMountEntriesJson();
    void FillBackupCopyInfo(VolumeBackupCopy& volumeBackupCopy);
    int ExecuteDataCopyVolume();
    bool RecordSnap(std::string& snapShotDev);
    int ExecuteTearDownVolume();
    bool PostReportCopyAdditionalInfo();
    virtual bool FillCopyInfo(std::string& extendInfo);
    bool StartBackup();
    virtual void FillBackupConfig(volumeprotect::task::VolumeBackupConfig &backupParams);
    bool SaveMetaFile(const std::string& metaFile, const std::string& volumeName);
    virtual bool PreProcess(std::string& snapShotDev);
    virtual void FillVolumeInfo(const VolumeBackupSubJob& backupSubJob);

    /* Protected HOST Share Information */
    VolumeDataLayoutExtend m_dataLayoutExt {};

    /* SubTask type: CopyVolume */

    time_t m_lastBackupTime { 0 };

    std::string m_backupCopyInfoFilePath;
    /* Used for reporting job progress log to agent */
    ActionResult m_logResult {};
    SubJobDetails m_logSubJobDetails {};
    LogDetail m_logDetail {};
    std::vector<LogDetail> m_logDetailList;
    std::string m_cacheFsParentPath;
    std::string m_backupPath;
    uint32_t m_blockSize;
    uint64_t m_sessionSize;

protected:
    bool IsFirstBackup();
    bool IsSameBackupConfig();
    bool IsSameVolumeSize();
    bool InitJobInfo();
    bool InitVolumeInfo();
    bool InitRepoPaths();
    bool InitShareResouce();
    virtual bool InitProtectVolume();
    bool PushVolumeToFile();
    bool ReadVolumeFromFile();
    bool SaveMeta();
    virtual void ClearResidualSnapshotsAndAlarm();
    void ClearSucceedDeletedSnapshotRecord(const std::vector<std::string>& snapshotNames);
    virtual bool CreateSnapshot(const std::string& volumeName, std::string& snapDevVol);
    virtual void DeleteSnapshot();
    void SendAlarmForResidualSnapshots(const std::vector<std::string>& snapshotNames);
    bool GetSnapshots(std::vector<std::string>& snapshotNames);
    bool RecordResidualSnapshots(const std::vector<std::string>& snapshotNames);
    void PostClean();
    void DeleteTmpFile();
    void KeepJobAlive();

    virtual bool GenerateSubJobHook();
    virtual bool TearDownVolumeHook();
    virtual bool RecordVolume();
    virtual bool MergeVolumeInfo();
    virtual uint32_t GetSysVolumeSize();
    virtual volumeprotect::CopyFormat CopyFormat();

    // Encapsulate volume backup api for mock
    bool IsBackupTerminated();
    volumeprotect::task::TaskStatus GetBackupGetStatus();
    void AbortBackup();
    volumeprotect::task::TaskStatistics GetBackupStatistics();

    virtual void GetOriVolumes(std::vector<VolumeInfo>& sourceVolumes);
    void GetMountPointFromFile(const std::string& volumeMountEntriesJson, std::string& mountPoints);
    bool AddDiskInfoToExtend(VolumeInfoSet &volumeInfoSet);

    std::string m_protectVolumeFile;
    std::vector<std::string> m_protectVolumeVec; /* 用户输入选择的逻辑卷 */
    VolumeBackupJobExtend m_advParms;

    std::string m_prevMetaPath;
    std::string m_currMetaPath;
    std::set <std::string> m_protectedVolumes;
    VolumeBackupCopy m_prevBackupCopyInfo {};

    std::string m_sysInfoPath;
    
    static uint32_t g_numberOfSubTask;
    std::atomic<bool> m_JobComplete {false};
};
}

#endif