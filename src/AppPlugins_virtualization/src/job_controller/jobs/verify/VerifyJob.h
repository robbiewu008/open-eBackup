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
#ifndef VERIFY_JOB_H
#define VERIFY_JOB_H

#include <map>
#include <job_controller/jobs/VirtualizationBasicJob.h>
#include <common/Macros.h>
#include <common/Structs.h>
#include <common/DirtyRanges.h>
#include <repository_handlers/RepositoryHandler.h>
#include <job_controller/io_scheduler/TaskScheduler.h>
#include <job_controller/jobs/backup/BackupIoTask.h>

#ifndef EXTER_ATTACK
#define EXTER_ATTACK
#endif

namespace VirtPlugin {
/* VerifyJob steps */
enum class VerifyJobSteps {
    STATE_NONE = 0,
    /* generate copy verify sub job steps */
    STEP_GENERATE_SUBJOB_INIT,
    STEP_GENERATE_DO_GENERATE_SUBJOB,

    /* exec sub job steps */
    STEP_EXEC_SUB_JOB_INIT,
    STEP_EXEC_LOAD_CHECKSUM_FILE,
    STEP_EXEC_CHECK_BLOCKS,
};

struct VerifySubJobInfo {
    std::string m_volUuid;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volUuid, volUuid)
    END_SERIAL_MEMEBER
};

struct VerifyFileInfo {
    std::string m_verifyFileDmage;
    
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_verifyFileDmage, isDamaged)
    END_SERIAL_MEMEBER
};

class VerifyJob : public VirtualizationBasicJob {
public:
    VerifyJob();
    virtual ~VerifyJob() = default;

    EXTER_ATTACK virtual int PrerequisiteJob() override;
    EXTER_ATTACK virtual int GenerateSubJob() override;
    EXTER_ATTACK virtual int ExecuteSubJob() override;
    EXTER_ATTACK virtual int PostJob() override;

protected:
    /**
     *  @brief 初始化校验业务子任务的执行信息
     *
     *  @return 0 成功，非0 失败
     */
    int InitExecInfo();

    /**
     *  @brief 分段加载SHA256 checksum文件，每个4M块占用32字节的SHA256值，每60G为一段
     *
     *  @return 0 成功，非0 失败
     */
    int LoadCheckSumFile();

    /**
     *  @brief 计算数据文件中的每个块的SHA256值与备份时记录的SHA256值进行对比，如果SHA256文件为空洞文件，则跳过全零值
     *
     *  @return 0 成功，非0 失败
     */
    int CheckBlocks();

private:
    void InitExecDetails();
    /* Common */
    int ParseJobInfo(const bool parseSubJob = true);

    /* GenerateSubJob */
    int32_t GenerateSubJobInner();
    int32_t GenerateJobInit();
    int32_t CreateSubTaskByVolumeInfo();

    /* ExecuteSubJob */
    void InitExecStateMachine();
    int ExecuteSubJobInner();
    int ParseCheckSumFileInfo();
    int CheckBlocksInner(TaskScheduler &ts);
    void ReportVerifyState(const bool jobRunning, const int execRet = SUCCESS);
    int TaskExecResult(TaskScheduler &ts, int &nTaskRunning);
    int TaskExecEnd(TaskScheduler &ts, int &nTaskRunning);

    int GetTaskResult(const int taskExecRet, const int endExecRet);
    void CheckTaskExecResult(const int taskRet, int& damagedRet, int& failedRet);

protected:
    uint64_t m_totalSize = 0;      // checksum文件总大小
    uint64_t m_offset = 0;         // 已处理checksum数据的偏移
    uint64_t m_ckSumBufSize = 0;   // 分段存放的SHA256校验文件的缓冲区大小
    std::shared_ptr<uint8_t[]> m_ckSumBuf = nullptr; // 分段存放的SHA256校验文件的缓冲区
    std::string m_checkSumFile;    // SHA256 checksum文件名
    std::string m_dataImgFile;     // 数据镜像文件名
    std::string m_metaRepoPath;
    std::string m_dataRepoPath;
    VerifySubJobInfo m_verifySubJob {};
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_dataRepoHandler = nullptr;
    std::map<int, ReportJobDetailsParam> m_mapExecRetDetails;
};
}
#endif
