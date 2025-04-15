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
#ifndef RESTORE_JOB_H
#define RESTORE_JOB_H
#include <unordered_map>
#include "job/BasicJob.h"
#include "common/Macros.h"
#include "common/Structs.h"
#include "common/Constants.h"
#include "RestoreIOTask.h"
#include "job_controller/io_scheduler/TaskPool.h"
#include "job_controller/io_scheduler/TaskScheduler.h"
#include "job_controller/jobs/VirtualizationBasicJob.h"
#include "protect_engines/ProtectEngine.h"
#include "volume_handlers/VolumeHandler.h"
#include "repository_handlers/RepositoryHandler.h"
#include "protect_engines/engine_factory/EngineFactory.h"
#include "repository_handlers/filesystem/FileSystemHandler.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "ArchiveStreamService.h"

namespace VirtPlugin {
enum class State {
    STATE_NONE = 0,
    /* 前置任务 */
    STATE_PRE_INIT,
    STATE_PRE_PREHOOK,
    STATE_PRE_CHECK_BEFORE_RECOVER,     // 恢复前检查
    STATE_PRE_GEN_INITIAL_VOL_LIST,     // 生成初始卷恢复列表
    STATE_PRE_GEN_FINAL_VOL_LIST,       // 生成最终的恢复卷列表(包括卷检查、创新卷等)
    STATE_PRE_POWER_OFF_MACHINE,        // 下电虚拟机
    STATE_PRE_DETACH_VOL,               // 卸载卷
    STATE_PRE_POSTHOOK,

    /* 分解任务 */
    STATE_GEN_PREHOOK,
    STATE_GEN_JOB_INIT,              // 分解_初始化
    STATE_GEN_GET_VOL_PAIR_LIST,     // 分解_获取源卷和目标卷的对应关系
    STATE_GEN_CREATE_EXEC_JOB,       // 分解_根据卷对应关系，生成执行子任务列表
    STATE_GEN_CREATE_POST_SUB_JOB,   // 分解_生成最后一个子任务，包括上电虚拟机、挂载卷等s
    STATE_GEN_PUT_JOB_TO_FRAME,      // 分解_将执行子任务列表放入框架
    STATE_GEN_POSTHOOK,

    /* 执行子任务 */
    STATE_EXECJOB_PREHOOK,
    STATE_EXECJOB_INITIALIZE,               // 执行子任务_初始化参数
    STATE_EXECJOB_LOAD_DIRTYRANGES,         // 执行子任务_加载dirtyrange
    STATE_EXECJOB_CHECK_BEFORE_MOUNT,
    STATE_EXECJOB_PREPARE_MOUNT,
    STATE_EXECJOB_RESTORE_VOLUME,
    STATE_EXECJOB_POWERON_MACHINE,          // 执行子任务_恢复卷
    STATE_EXECJOB_MIGRATE_VOLUME,
    STATE_EXECJOB_RENAME_MACHINE,
    STATE_EXECJOB_POSTHOOK,

    /* 最后的业务子任务 */
    STATE_EXEC_POST_SUBJOB_PREHOOK,    // 最后业务子任务PREHOOK
    STATE_EXEC_POST_CREATE_MACHINE,   // 创建虚拟机
    STATE_EXEC_POST_SUBJOB_ATTACH_VOLUME,
    STATE_EXEC_POST_SUBJOB_POWERON_MACHINE,
    STATE_EXEC_POST_SUBJOB_POSTHOOK,   // 最后业务子任务POSTHOOK

    /* 后置任务 */
    STATE_POST_PREHOOK,
    STATE_POST_DELETE_SNAPSHOT,     // 后置任务删除快照
    STATE_POST_POWERON_MACHINE,     // 后置任务上电新机
    STATE_POST_DELETE_MACHINE,      // 恢复任务失败后删除新创的虚拟机
    STATE_POST_DELETE_VOLUMES,      // 恢复任务失败后删除新创的卷列表
    STATE_POST_ATTACH_VOLUMES,      // 挂载磁盘到虚拟机
    STATE_POST_CLEAN_RESOURCE,      // 清除临时文件
    STATE_POST_POSTHOOK,

    STATE_ARCHIVE_INIT
};

class RestoreJob : public VirtualizationBasicJob {
public:
    RestoreJob() {};
    ~RestoreJob() {};
    EXTER_ATTACK virtual int PrerequisiteJob() override;
    EXTER_ATTACK virtual int GenerateSubJob() override;
    EXTER_ATTACK virtual int ExecuteSubJob() override;
    EXTER_ATTACK virtual int PostJob() override;

    /* 公共函数 */
    bool CommonInfoInit();
    virtual bool InitHandlers();
    bool InitRestoreParams();
    int PowerOnMachine();

    /* 前置任务 */
    int PrerequisiteJobInner();
    int CheckBeforeRecover();
    void PreInitStateHandles();
    int PrerequisiteJobInit();
    bool LoadVolMetaData();
    virtual bool LoadVmMetaData();
    virtual bool LoadMetaData();
    int GenInitialRestoreVolList();
    int GenFinalRestoreVolList();
    int CreateMachine();
    int PowerOffMachine();
    int DetachVolume();
    bool InitRepo() const;
    int CheckForciblyRestoreInvalidCopies(); // 强制恢复副本校验出的无效副本
    bool CheckMainTaskStatusFileExist();
    // 快照相关函数
    int CreateSnapshot();
    int DeleteSnapshot();

    // 分解子任务
    void SetGenerateJobStateMachine();
    int GenerateJobInit();
    int GetVolMatchPairInfo();
    int CreateSubTasksByVolPairInfo();
    int CreatePostSubJob();
    int PutSubTasksToFrame();
    int GenMainTaskStatusToFile();

    // 执行子任务接口
    int ExecuteSubJobInner();
    void SubJobStateInit();
    int SubTaskInitialize();
    int SubTaskGetDirtyRanges();
    int SubTaskExecute();
    int SubTaskClean();
    int InitExecJobParams();
    int CreateIOHandler();
    int CreateReadHandler();
    int CreateWriteHandler();
    bool IOTaskExecResult(int &nTaskExecuting);
    void ExecIOTaskEnd(int &nTaskExecuting, int &taskExecRet);
    bool GetSnapshotInfo(VolSnapInfo &snapInfo);
    int LoadCurSegBlockBitMap();
    bool ExecBlockTaskStart(const std::shared_ptr<RestoreIOTask> &task, int &tasksCount, int &tasksExecRet);
    bool CheckCurBlockHasData(uint64_t offset) const;
    bool IfCheckDataBitMap() const;
    bool IfCheckDataBitMapArchive() const;
    int InitTaskScheduler();
    void ArchiveTaskInitial(DirtyRanges::iterator& it, std::shared_ptr<RestoreIOTask>& task);

    // 最后一个子任务
    int ExecBusinessLastSubJob();
    void PostSubJobStateInit();
    int PostSubJobPreHook();
    int SubJobAttachVolume();
    int SubJobPowerOnMachine();
    int PostSubJobPostHook();

    /* 后置任务 */
    int PostJobInner();
    int PostJobStateInit();
    int DeleteMachine();
    int DeleteVolumes();
    
    int PostJobPowerOnMachine();
    int PostJobAttachVolume();
    int PostClean();

    /* hook函数 */
    int PrerequisitePreHook();
    int PrerequisitePostHook();
    int GenerateSubJobPreHook();
    int GenerateSubJobPostHook();
    int ExecuteSubJobPreHook();
    int ExecuteSubJobPostHook();
    int PostJobPreHook();
    int PostJobPostHook();
    std::string GetTaskId() const
    {
        std::string output;
        if (m_restorePara != nullptr) {
            output = "jobId=" + m_restorePara->jobId;
        }
        if (m_subJobInfo != nullptr) {
            output = output + ",subJobId=" + m_subJobInfo->subJobId;
        }
        return output;
    }

public:
    std::shared_ptr<AppProtect::RestoreJob> m_restorePara = nullptr;

    bool m_poweron;                                // 恢复后是否上电虚拟机
    VMInfo m_restoreVm;                            // 待恢复的虚拟机实例
    VMInfo m_copyVm;                               // 保存副本中的虚拟机实例
    RestoreLevel m_RestoreLevel;                     // 虚拟机恢复或者卷恢复
    RestorePlace m_restorePlace;                   // 恢复到原位置新机/新位置/指定虚拟机

    std::string m_dataRepoPath {};
    std::string m_cacheRepoPath {};
    std::string m_metaRepoPath {};

    std::string m_newVMMetaDataPath {};                // 新创虚拟机元数据保存到缓存仓的路径
    std::string m_volPairPath {};                      // 卷匹配对存储路径

    std::vector<VolInfo> m_backupedVolList;        // 副本数据中的卷列表，key=volume uuid, value=卷元数据
    std::unordered_map<std::string, VolInfo> m_initialRestoreVolsMap;  // 初始待恢复卷列表 first-uuid, second-volumetadata
    VolMatchPairInfo m_finalRestoreVolPair;        // 最终的卷恢复列表

    // 执行子任务
    SnapshotInfo m_snapshot {};
    SubJobExtendInfo m_subJobExtendInfo {};
    DirtyRanges m_dirtyRanges {};
    std::shared_ptr<VolumeHandler> m_targetVolHandler = nullptr;
    std::atomic<std::uint64_t> m_completedBlocks; // 记录任务完成的数据块数量
    uint64_t m_maxIOThreadNumConf = 0;  // 最大io任务个数，从配置文件中获取
    uint64_t m_segmentSizeConf = 0; // 分段数据量，从配置文件中获取
    uint64_t m_completedSegmentSize = 0;    // 分段完成的数据量
    uint64_t m_curSegmentSize = 0;
    bool m_needSegment = false; // 是否需要分段
    VolInfo m_originVolInfo {}; // 原卷信息
    VolInfo m_targetVolInfo {}; // 目标卷信息
    std::shared_ptr<uint8_t[]> m_blockDataBitMap = nullptr;

    // 分解子任务
    std::string m_jobId;
    std::vector<SubJob> m_execSubs;
    bool m_isArchiveRestore {false};
    bool m_forceRecoverIgnoreBadBlock {false};

    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_dataRepoHandler = nullptr;

    std::unique_ptr<TaskScheduler> m_taskScheduler = nullptr;

    std::shared_ptr<ArchiveStreamService> m_clientHandler {nullptr};
    ArchiveStreamGetFileReq m_getFileReq;
};
}

#endif