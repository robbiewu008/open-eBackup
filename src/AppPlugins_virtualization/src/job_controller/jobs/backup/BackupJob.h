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
#ifndef BACKUP_JOB_H
#define BACKUP_JOB_H

#include <unordered_map>
#include <job_controller/jobs/VirtualizationBasicJob.h>
#include <common/Macros.h>
#include <common/Structs.h>
#include <common/DirtyRanges.h>
#include <common/checkpoint/Checkpoint.h>
#include <repository_handlers/RepositoryHandler.h>
#include <protect_engines/ProtectEngine.h>
#include <volume_handlers/VolumeHandler.h>
#include <job_controller/io_scheduler/TaskScheduler.h>
#include <job_controller/jobs/backup/BackupIoTask.h>

#ifndef EXTER_ATTACK
#define EXTER_ATTACK
#endif

#ifndef WIN32
class DataMoverLog;
class AsioDataMover;
#endif
namespace VirtPlugin {
/* BackupJob steps */
enum class BackupJobSteps {
    STATE_NONE = 0,
    /* prerequisite job steps */
    STEP_PRE_INIT,
    STEP_PRE_PREHOOK,
    STATE_PRE_CHECK_BEFORE_BACKUP,
    STEP_PRE_CREATE_SNAPSHOT,
    STEP_PRE_ACTIVE_SNAPSHOT_CONSISTENCY,
    STEP_PRE_GET_MACHINE_METADATA,
    STEP_PRE_GET_VOLUMES_METADATA,
    STEP_PRE_INIT_REPO_PATH,
    STEP_PRE_SAVE_METADATA,
    STEP_PRE_POSTHOOK,

    /* generate sub job steps */
    STEP_GENERATE_SUBJOB_INIT,
    STEP_GENERATE_PREHOOK,
    STEP_GENERATE_LOAD_PRE_SNAPSHOT_INFO,
    STEP_GENERATE_LOAD_VM_METADATA,
    STEP_GENERATE_DO_GENERATE_SUBJOB,
    STEP_GENERATE_POSTHOOK,

    /* exec sub job steps */
    STEP_EXEC_SUB_JOB_INIT,
    STEP_EXEC_PREHOOK,
    STEP_EXEC_REPORT_COPY,
    STEP_EXEC_GET_DIRTY_RANGES,
    STEP_EXEC_BACKUP_DIRTY_RANGES,
    STEP_EXEC_POSTHOOK,

    /* post job steps */
    STEP_POST_JOB_INIT,
    STEP_POST_PREHOOK,
    STEP_POST_UPDATE_SNAPSHOT_FILE,
    STEP_POST_CLEANUP_SNAPSHOT,
    STEP_POST_CLEANUP_CHECKPOINT,
    STEP_POST_POSTHOOK
};

class SnapToBeDeleted {
public:
    SnapshotInfo m_snapshotInfo;
    uint32_t m_tryDeleteCount { 0 };

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotInfo, snapshotInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tryDeleteCount, deleteCount)
    END_SERIAL_MEMEBER
};

class SnapListToBeDeleted {
public:
    std::vector<SnapToBeDeleted> m_snapList;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapList, volSnapList)
    END_SERIAL_MEMEBER
};

struct SnapResidualInfo {
    std::string m_jobId;
    std::string m_alarmId;
    std::string m_alarmParam;
    std::vector<VolSnapInfo> m_volSnapshots;
    uint32_t m_tryDeleteCount { 0 };

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_jobId, jobId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_alarmId, alarmId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_alarmParam, alarmParam)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volSnapshots, volSnapshots)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tryDeleteCount, deleteCount)
    END_SERIAL_MEMEBER
};

struct SnapResidualListSaveInfo {
    std::vector<SnapResidualInfo> m_snapResidualInfoList;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapResidualInfoList, snapResidualInfoList)
    END_SERIAL_MEMEBER
};

struct CopyExtendInfo {
    std::vector<VolInfo> m_volList;
    std::vector<BridgeInterfaceInfo> m_interfaceList;
    std::string m_copyVerifyFile;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volList, volList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_interfaceList, interfaceList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_copyVerifyFile, copyVerifyFile)
    END_SERIAL_MEMEBER
};

struct VerifyFileStates {
    std::string m_verifyFileState;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_verifyFileState, copyVerifyFile)
    END_SERIAL_MEMEBER
};

struct BackupCheckpointInfo {
    BackupCheckpointInfo() : m_completedSegmentSize(0), m_completedDataSize(0) {}

    uint64_t m_completedSegmentSize;
    uint64_t m_completedDataSize;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_completedSegmentSize, completedSegmentSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_completedDataSize, completedDataSize)
    END_SERIAL_MEMEBER
};

const int INITIAL_STATE = -1;
const int SAVE_BLOCK_BIT_MAP = 0;
const int NOT_SAVE_BLOCK_BIT_MAP = 1;

class BackupJob : public VirtualizationBasicJob {
public:
    BackupJob() = default;
    virtual ~BackupJob() = default;

    EXTER_ATTACK virtual int PrerequisiteJob() override;
    EXTER_ATTACK virtual int GenerateSubJob() override;
    EXTER_ATTACK virtual int ExecuteSubJob() override;
    EXTER_ATTACK virtual int PostJob() override;

private:
    /* Prerequisite */
    int PrerequisiteJobInner();
    void InitPreStateMachine();
    int PrerequisiteInit();
    int CheckBeforeBackup();
    int CreateSnapshot();
    int GetMachineMetadata();
    int GetVolumesMetadata();
    int InitRepoPath();
    int SaveMetadata();
    int SaveSnapshotInfo();
    int SaveVolumesMetadata();
    int SaveVMInfo();
    int SaveJobContext();
    std::string GetJobContextPath(const std::string &fileName);
    int GetProtectObjectInfo();
    int ActiveSnapShotConsistency();
    bool CheckMainTaskStatusFileExist();

    /* Generate sub job */
    int GenerateSubJobInner();
    void InitGenerateSubJobStateMachine();
    int GenerateSubJobInit();
    int LoadPreSnapshotInfo();
    int LoadVMMetadata();
    int DoGenerateSubJob();
    bool FillUpBackupSubJob(const VolInfo &vol, BackupSubJobInfo &subJobInfo);
    int WriteGenerateSha256State(const int32_t execState);
    int GenMainTaskStatusToFile();

    /* Execute sub job */
    int ExecuteSubJobInit();
    int GetDirtyRanges();
    int BackupDirtyRanges();
    int32_t SaveBlockDataBitMap(uint64_t confSegSize);
    void SetBlockDataFlag(uint64_t offset, uint64_t confSegSize);
    bool InitCurSegBlockDataBitMap();
    int BlockBitMapFileInit();
    bool InitBlockDataBitMapFile(const std::string &blockBitMapFile);
    std::string VolValidDataBitMapFile();
    bool IfSaveValidDataBitMap();

    /* PostJob */
    int PostJobInner();
    void InitPostJobStateMachine();
    int PostJobInit();
    int UpdateSnapshotFile();
    int CleanupSnapshot();
    int CleanCheckpoint();
    void GetSha256States(std::string& strInfo);
    int ReportCopy();
    bool CheckIfNeedDeleteSnapShot(const VolSnapInfo &volSnap, SnapshotInfo &snapInfo);

    /* Common */
    int CommonInit();
    bool InitHandlers();
    int GetJobInfoBody();
    void GetProtectSubObjects();
    void VolumeFilter();

    int ExecuteSubJobInner();
    void InitExecStateMachine();
    int BackupParamInit();
    int ExecJobHandlerInit();
    int BlockBackupTaskInit();
    int OpenRWHandler();
    int CloseRWHandler();
    int CleanLeftovers();
    bool ExecBlockTaskStart(const std::shared_ptr<BackupIoTask> &task, uint32_t &tasksCount, int32_t &tasksExecRet,
        uint64_t &completedBlocksNum, bool& sha256Success);
    void ExecBlockTaskEnd(uint32_t &tasksCount, int32_t &tasksExecRet, uint64_t &completedBlocksNum,
        bool& sha256Success);
    void CheckNextSegment();
    std::string GetTaskId() const
    {
        std::string output;
        if (m_backupPara != nullptr) {
            output = "jobId=" + m_backupPara->jobId;
        }
        if (m_subJobInfo != nullptr) {
            output = output + ", subJobId=" + m_subJobInfo->subJobId;
        }
        return output;
    }
    int FormatBackupCopy(Copy &copy);
    int GetSnapshotToBeDelete();
    void TryDeleteSnapshots();
    int SaveDeleteList();
    std::string GetSnapshotsLogDetails(const std::vector<VolSnapInfo>& volSnapshots);
    int SendAlarmAndRecordResidualSnapshots(const std::vector<SnapshotInfo>& reSnapshotInfos);
    void TryClearResidualSnapshotsAndAlarm();
    int DeleteVmSnapshot(SnapshotInfo& snap);
    int SaveResidualSnapshots();
    bool IsSnapInVolSnapshots(const VolSnapInfo& volSnap, const std::vector<VolSnapInfo>& volSnaps);
    bool IsSnapInToResidualSnapshots(const VolSnapInfo& volSnapInfo);

    int PrerequisitePreHook();
    int PrerequisitePostHook();
    int GenerateSubJobPreHook();
    int GenerateSubJobPostHook();
    int ExecuteSubJobPreHook();
    int ExecuteSubJobPostHook();
    int PostJobPreHook();
    int PostJobPostHook();
    void ReportBackupSpeed(const uint64_t &dataSizeInByte);
    void LoadSnapshotToBeDeleted(const std::string &file);
    void LoadSnapshotsOfVMToBeDeleted(const std::vector<VolSnapInfo>& exVolSnaps);
    void ShowSnapshot(const SnapshotInfo &snap);
    int FormatSubJobItems(const VolInfo &vol, std::vector<SubJob> &subJobs, const int index);
    int InitCheckpointFolder();
    int GenShaInterruptFile();
    int InitCheckpoint();
    int PostJobInitCheckpoint();
    int SaveCheckpointInfo();
    int UpdateInfoByCheckpoint();

    bool CreateVerifyFile();
    bool CreateVerifyFailedFile();
    int32_t InitSha256File();
    int32_t CalculateSha256Operator(std::shared_ptr<BlockTask>& res, const int32_t tasksExecRet, bool& sha256Success);
    int32_t SaveBlockSha256Value(const std::shared_ptr<unsigned char[]>& shaBuf, const uint64_t& startAddr);
    bool WriteShaValueToFile();
    int32_t ExecWriteShaFile();
    void InitAndRegTracePoint();
    bool IfDiskExpand(uint64_t &preVolBlockCount);
    int SavePreVolInfoToCache();
    int InitBitmap();
    bool PrepareForAioBackup();
#ifndef WIN32
    int BackupDirtyRangesAio();
    void HandleAioBackupLoop(AsioDataMover& temp);
#endif
private:
    std::string m_metaRepoPath;
    std::string m_cacheRepoPath;

    VMInfo m_vmInfo;
    SnapshotInfo m_snapshotInfo;
    SnapshotInfo m_preSnapshotInfo;

    std::shared_ptr<AppProtect::BackupJob> m_backupPara = nullptr;
    std::shared_ptr<VolumeHandler> m_volHandler = nullptr;
    std::shared_ptr<VolInfo> m_backupVolInfo = nullptr;
    std::unique_ptr<TaskScheduler> m_taskScheduler = nullptr;
    BackupSubJobInfo m_backupSubJob {};
    StorageRepository m_dataRepo {};
    StorageRepository m_metaRepo {};
    std::string m_dataRepoPath {};
    SnapshotInfo m_curSnapshot {};
    DirtyRanges m_dirtyRanges {};
    uint64_t m_totalVolumeSize = 0;
    uint64_t m_totalBlockCount = 0;
    uint64_t m_curSegmentSize = 0;
    uint64_t m_totalSegmentNum = 0;
    uint64_t m_curSegmentIndex = 0;
    uint32_t m_maxTaskThreadNum = 0;
    uint64_t m_completedSegmentSize = 0;
    int m_ifSaveValidDataBitMap = INITIAL_STATE; // 初始值-1表示未做判断
    SnapListToBeDeleted m_snapListToBeDeleted;
    SnapResidualListSaveInfo m_snapResidualListSaveInfo;
    std::unordered_map<std::string, AppProtect::ApplicationResource> m_volToBeBackupMap;
    Checkpoint<BackupCheckpointInfo> m_checkpoint;
    BackupCheckpointInfo m_backupCheckpointInfo;
    std::string m_xNNEsn;

protected:
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_dataRepoHandler = nullptr;
    bool m_isCopyVerify = false;
    bool m_sha256Result = true;
    // std::string m_mainTaskEndFlag = ;

    std::vector<BlockShaData> m_blockShaData;  // 分段存放 本段写入备份存储4M块的SHA256值

    std::shared_ptr<uint8_t[]> m_blockDataBitMap = nullptr;
    std::vector<VirtPlugin::DirtyRange> m_dirtyRangesForAio;
    std::shared_ptr<int> m_writerFD = nullptr;
    std::shared_ptr<int> m_readFD = nullptr;
#ifndef WIN32
    std::shared_ptr<DataMoverLog> m_aioLogger = nullptr;
#endif
};
}

#endif
