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
#include "BackupJob.h"
#include <iostream>
#include <ClientInvoke.h>
#include <common/uuid/Uuid.h>
#include <common/utils/Utils.h>
#include <common/Macros.h>
#include <config_reader/ConfigIniReader.h>
#include <memory>
#include <string>
#include <repository_handlers/factory/RepositoryFactory.h>
#include <protect_engines/engine_factory/EngineFactory.h>
#include "job_controller/factory/VirtualizationJobFactory.h"
#include "common/sha256/Sha256.h"
#ifndef WIN32
#include "AsioDataMover.h"
#endif

namespace {
const std::string MODULE_NAME = "BackupJob";
const std::string SHA256_FILE_FAILED_STATE = "Sha256Failed.info";
const std::string VIRT_PLUGIN_SNAPSHOT_PREFIX = "VIRTUAL_PLUGIN_";
const std::string VOLUME_DATA_PROCESS = "VOLUME_DATA_PROCESS";
const std::string MAX_BACKUP_THREADS = "MAX_BACKUP_THREADS";
constexpr uint64_t DEFAULT_BACKUP_THREADS = 4;
constexpr uint64_t DEFAULT_BLOCK_TIMEOUT_MS = 25;
constexpr uint32_t DELETE_SNAPSHOT_RETRY_MAX = 5;
constexpr uint32_t REPORT_SPEED_BLOCK_NUM = 1024;
constexpr uint32_t KB_SIZE = 1024;
constexpr uint32_t GB_SIZE = 1024 * 1024 * 1024;
constexpr uint32_t MAX_EXEC_COUNT = 3;
constexpr uint32_t RETRY_INTERVAL_SECOND = 5;
 
// 	错误场景：执行调用生产端接口操作时，由于调用生产端接口失败，操作失败。
constexpr int64_t INVOKE_API_FAILED_GENERAL_CODE = 1577213460;
 
const std::string ALARM_CODE_FAILED_DELETE_SNAPSHOT = "0x2064006F0001";
}
#ifndef WIN32
class VirtualFrameworkDataMoverLog : public DataMoverLog {
public:
    void Info(const std::string& content) override
    {
        INFOLOG("%s", content.c_str());
    }
    void Debug(const std::string& content) override
    {
        DBGLOG("%s", content.c_str());
    }
    void Warn(const std::string& content) override
    {
        WARNLOG("%s", content.c_str());
    }
    void Error(const std::string& content) override
    {
        ERRLOG("%s", content.c_str());
    }
};
#endif

VIRT_PLUGIN_NAMESPACE_BEGIN
EXTER_ATTACK int BackupJob::PrerequisiteJob()
{
    LOGGUARD("");
    int ret = PrerequisiteJobInner();
    ReportJobResult(ret, "PrerequisiteJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int BackupJob::PrerequisiteJobInner()
{
    InitAndRegTracePoint();
    DBGLOG("Begin to exeute backup pre-requisite job, %s", GetTaskId().c_str());
    InitPreStateMachine();
    return RunStateMachine();
}

EXTER_ATTACK int BackupJob::GenerateSubJob()
{
    LOGGUARD("");
    int ret = GenerateSubJobInner();
    ReportJobResult(ret, "GenerateSubJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int BackupJob::ExecuteSubJob()
{
    LOGGUARD("");
    int ret = ExecuteSubJobInner();
    WriteGenerateSha256State(ret);
    ReportJobResultPara reportJobResultPara;
    if (ret != SUCCESS) {
        std::vector<std::string> args = { m_subJobInfo->subJobId };
        ReportJobDetailsParam param = {
            "plugin_backup_subjob_fail_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0 };
        reportJobResultPara.m_jobDetailsParam = param;
        reportJobResultPara.m_args = args;
    }
    ReportJobResult(ret, "ExecuteSubJob finish.", m_completedDataSize, reportJobResultPara);
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    std::string nodeBalance = Module::ConfigReader::getString(GENERAL_CONF, "DataRepoPathBalance");
    if (nodeBalance == "true" && m_subJobInfo->jobName.find(BUSINESS_SUB_JOB_NAME_PREFIX) != std::string::npos) {
        LoadBalancer::GetInstance()->RemoveNodePath(m_dataRepoPath);
    }
    SetJobToFinish();
    return ret;
}

int BackupJob::ExecuteSubJobInner()
{
    DBGLOG("Begin to exeute backup business sub job, %s", GetTaskId().c_str());
    InitExecStateMachine();
    int iRet = RunStateMachine();
    if (iRet != SUCCESS) {
        ERRLOG("RunStateMachine failed, ret %d, %s", iRet, GetTaskId().c_str());
    }
    CloseRWHandler();
    CleanLeftovers();
    return iRet;
}

void BackupJob::InitExecStateMachine()
{
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_EXEC_SUB_JOB_INIT)]
        = std::bind(&BackupJob::ExecuteSubJobInit, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_EXEC_PREHOOK)]
        = std::bind(&BackupJob::ExecuteSubJobPreHook, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_EXEC_REPORT_COPY)]
        = std::bind(&BackupJob::ReportCopy, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_EXEC_GET_DIRTY_RANGES)]
        = std::bind(&BackupJob::GetDirtyRanges, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_EXEC_BACKUP_DIRTY_RANGES)]
        = std::bind(&BackupJob::BackupDirtyRanges, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_EXEC_POSTHOOK)]
        = std::bind(&BackupJob::ExecuteSubJobPostHook, this);
    m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_SUB_JOB_INIT);
}

/**
 *  @brief 执行子任务_初始化
 *
 *  @return 0 成功，非0 失败
 */
int BackupJob::ExecuteSubJobInit()
{
    INFOLOG("Start sub task initialize, %s", GetTaskId().c_str());
    // 获取参数信息
    if (BackupParamInit() != SUCCESS) {
        ERRLOG("Get job info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 副本上报子任务
    if (m_subJobInfo->jobName == REPORT_COPY_SUB_JOB) {
        m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_PREHOOK);
        INFOLOG("Start to exec report copy sub job, %s", m_taskInfo.c_str());
        return SUCCESS;
    }
    // 创建Handler
    if (ExecJobHandlerInit() != SUCCESS) {
        ERRLOG("Failed to create IO handler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 块任务初始化
    if (BlockBackupTaskInit() != SUCCESS) {
        ERRLOG("Failed to create IO handler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 打开读写端io Handler
    if (OpenRWHandler() != SUCCESS) {
        ERRLOG("Failed to open io handler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 初始化续作点
    if (InitCheckpoint() != SUCCESS) {
        ERRLOG("Checkpoint initialization failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_PREHOOK);
    return SUCCESS;
}

/**
 *  @brief 执行子任务_获取备份参数信息
 *
 *  @return 0 成功，非0 失败
 */
int BackupJob::BackupParamInit()
{
    INFOLOG("Start sub task initialize, %s", GetTaskId().c_str());
    // 获取备份任务参数
    if (CommonInit() != SUCCESS) {
        /* Log error inside. */
        return FAILED;
    }
    // 获取子任务中卷信息
    if (m_subJobInfo == nullptr || !Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, m_backupSubJob)) {
        ERRLOG("Get backup subjob info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_subJobInfo->jobName == REPORT_COPY_SUB_JOB) {
        return SUCCESS;
    }
    std::vector<std::string> args = { m_backupSubJob.m_volInfo.m_uuid };
    ReportJobDetailsParam param = {
        "virtual_plugin_backup_job_execute_subjob_start_label",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING, 0, 0 };
    ReportJobDetailWithLabel(param, args);

    return SUCCESS;
}

/**
 *  @brief 获取读写端Handler
 *
 *  @return 0 成功，非0 失败
 */
int BackupJob::ExecJobHandlerInit()
{
    // 保护对象->卷Handler
    int32_t ret = m_protectEngine->GetVolumeHandler(m_backupSubJob.m_volInfo, m_volHandler);
    if (ret != SUCCESS) {
        ERRLOG("Failed to create volume handler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    auto handler = std::bind(&BackupJob::ReportApplicationLabels, this, std::placeholders::_1);
    m_volHandler->SetReportJobDetailHandler(handler);
    return SUCCESS;
}

/**
 *  @brief 块备份任务初始化
 *
 *  @return 0 成功，非0 失败
 */
int BackupJob::BlockBackupTaskInit()
{
    // IO 调度初始化
    TaskPool *tp = TaskPool::GetInstance();
    m_taskScheduler = std::make_unique<TaskScheduler>(*tp);
    if (m_taskScheduler == nullptr) {
        ERRLOG("Failed to create task scheduler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 单个卷备份的最大并行IO数
    m_maxTaskThreadNum = Module::ConfigReader::getUint(VOLUME_DATA_PROCESS, MAX_BACKUP_THREADS);
    if (m_maxTaskThreadNum == 0) {
        m_maxTaskThreadNum = DEFAULT_BACKUP_THREADS;
    }
    DBGLOG("Cur task thread num: %d, %s", m_maxTaskThreadNum, m_taskInfo.c_str());
    // 当前卷大小
    m_totalVolumeSize = m_backupSubJob.m_volInfo.m_volSizeInBytes;
    if (m_totalVolumeSize == 0) {
        ERRLOG("Failed to get volume size, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 段大小 byte，默认60G
    m_curSegmentSize = GetSegementSizeFromConf();
    DBGLOG("Cur seg size %llu, %s", m_curSegmentSize, m_taskInfo.c_str());
    if (m_totalVolumeSize < m_curSegmentSize) {
        m_curSegmentSize = m_totalVolumeSize;
    }
    // 当前卷的分段数量
    if (m_curSegmentSize == 0) {
        ERRLOG("Cur segment size error, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (InitBitmap() != SUCCESS) {
        ERRLOG("Init bitmap failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_totalSegmentNum = (m_totalVolumeSize + m_curSegmentSize - 1) / m_curSegmentSize;
    m_curSegmentIndex = 0;
    m_completedSegmentSize = 0;
    m_completedDataSize = 0;
    DBGLOG("Block backup task init success, total size:%ld , cur seg size: %ld ,total segment num:%ld , %s",
        m_totalVolumeSize, m_curSegmentSize, m_totalSegmentNum, m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::InitBitmap()
{
    if (m_backupPara->jobParam.backupType == AppProtect::BackupJobType::FULL_BACKUP) {
        INFOLOG("Full backup dont have pre bitmap.");
        return SUCCESS;
    }
    uint64_t preVolBlockCount;
    if (!IfDiskExpand(preVolBlockCount)) {
        INFOLOG("No need add data to bitmap.");
        return SUCCESS;
    }
    uint64_t curVolBlockCount = (m_totalVolumeSize + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
    int64_t resCount = curVolBlockCount - preVolBlockCount;
    if (!m_metaRepoHandler->Exists(VolValidDataBitMapFile())) {
        WARNLOG("No previous volume bitmap, dont add data to it.");
        return SUCCESS;
    }
    if (m_metaRepoHandler->Open(VolValidDataBitMapFile(), "a+") != SUCCESS) {
        ERRLOG("Open bitmap failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    Utils::Defer _(nullptr, [&](...) {
        m_metaRepoHandler->Close();
    });
    std::shared_ptr<uint8_t[]> buf;
    int64_t confSegSize = GetSegementSizeFromConf();
    int64_t perSegBlockCount = (confSegSize + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
    while (resCount > 0) {
        int64_t curCount = resCount - perSegBlockCount >= 0 ? perSegBlockCount : resCount;
        resCount -= perSegBlockCount;
        buf.reset(new uint8_t[curCount], std::default_delete<uint8_t[]>());
        memset_s(buf.get(), curCount, '0', curCount);
        if (m_metaRepoHandler->Write(buf, curCount) != curCount) {
            ERRLOG("Write data to bitmap failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }
    INFOLOG("Write data to bitmap success.");
    return SUCCESS;
}

/**
 *  @brief 打开读写端io Handler
 *
 *  @return 0 成功，非0 失败
 */
int BackupJob::OpenRWHandler()
{
    std::string curSnapFile = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, curSnapFile, m_curSnapshot) != SUCCESS) {
        ERRLOG("Load context failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_volHandler == nullptr ||
        m_volHandler->Open(VolOpenMode::READ_ONLY, m_backupSubJob) != SUCCESS) {
        ERRLOG("Failed to open vol handler, %s", m_taskInfo.c_str());
        VolHandlerReportTaskLabel(m_volHandler);
        return FAILED;
    }
    std::string rawFileName = m_dataRepoPath + Module::PATH_SEPARATOR + m_backupSubJob.m_volInfo.m_uuid + ".raw";
    if (m_dataRepoHandler->Open(rawFileName, "a+") != SUCCESS) {
        ERRLOG("Failed to open repo handler, %s", m_taskInfo.c_str());
        return FAILED;
    }

    if (m_dataRepoHandler->Truncate(m_totalVolumeSize) != SUCCESS) {
        ERRLOG("Truncate failed, file name: %s, %s", rawFileName.c_str(), m_taskInfo.c_str());
        return FAILED;
    }

    /* close to make ftruncate takes affect */
    if (m_dataRepoHandler->Close() != SUCCESS) {
        ERRLOG("Close failed, file name: %s, %s", rawFileName.c_str(), m_taskInfo.c_str());
        return FAILED;
    }

    /* open for writting */
    if (m_dataRepoHandler->Open(rawFileName, "r+") != SUCCESS) {
        ERRLOG("Open for writting failed, file name: %s, %s", rawFileName.c_str(), m_taskInfo.c_str());
        return FAILED;
    }

    return SUCCESS;
}

/**
 *  @brief 关闭读写端io Handler
 *
 *  @return 0 成功，非0 失败
 */
int BackupJob::CloseRWHandler()
{
    if (m_volHandler != nullptr) {
        if (m_volHandler->Close() != SUCCESS) {
            ERRLOG("Close volume handler failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }
    if (m_dataRepoHandler != nullptr) {
        if (m_dataRepoHandler->Close() != SUCCESS) {
            ERRLOG("Close data repository handler failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }

    INFOLOG("Close io handler success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

/**
 *  @brief 清理代理主机挂载点资源
 *
 *  @return 0 成功，非0 失败
 */
int BackupJob::CleanLeftovers()
{
    if (m_volHandler != nullptr) {
        if (m_volHandler->CleanLeftovers() != SUCCESS) {
            ERRLOG("Clean mount point failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }

    return SUCCESS;
}

bool BackupJob::IfSaveValidDataBitMap()
{
    // 全量备份第一段差量位图为全F, 则该卷不记有效数据位图
    if (m_backupPara->jobParam.backupType == AppProtect::BackupJobType::FULL_BACKUP &&
        !m_volHandler->GetIfMarkBlockValidData()) {
        return false;
    }
    // 增量备份根据是否存在有效数据位图信息来判断
    if (m_backupPara->jobParam.backupType == AppProtect::BackupJobType::INCREMENT_BACKUP &&
        !m_metaRepoHandler->Exists(VolValidDataBitMapFile())) {
        return false;
    }
    return true;
}

/**
 *  @brief 执行子任务_获取当前分段的 DirtyRanges
 *
 *  @return 0 成功，非0 失败
 */
int BackupJob::GetDirtyRanges()
{
    DBGLOG("Begin BackupJob::GetDirtyRanges, %s", m_taskInfo.c_str());
    if (UpdateInfoByCheckpoint() != SUCCESS) {
        ERRLOG("Update info by checkpoint failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 卷完成备份后进程异常, 会重新走子任务，当前判断备份完成则不再重新备份
    if (m_backupCheckpointInfo.m_completedSegmentSize == m_totalVolumeSize) {
        INFOLOG("Vol(%s) backup finisih.", m_backupSubJob.m_volInfo.m_uuid.c_str());
        m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_POSTHOOK);
        return SUCCESS;
    }
    uint64_t startOffset = m_completedSegmentSize;
    if (m_totalVolumeSize < (startOffset + m_curSegmentSize)) {
        m_curSegmentSize = m_totalVolumeSize - startOffset;
    }
    // 开始备份本次DirtyRange之前，将m_blockShaData中存放的数据清空，用来存放本次dirtyRange的校验数据
    m_blockShaData.clear();
    uint64_t endOffset = startOffset + m_curSegmentSize;
    m_dirtyRanges.clear();
    int32_t ret = m_volHandler->GetDirtyRanges(m_backupSubJob.m_preSnapshotInfo, m_backupSubJob.m_curSnapshotInfo,
                                               m_dirtyRanges, startOffset, endOffset);
    if (ret != SUCCESS) {
        ERRLOG("Failed to get dirty ranges, start:[%llu], end:[%llu], %s", startOffset, endOffset, m_taskInfo.c_str());
        return FAILED;
    }
    m_dirtyRanges.Initialize(m_cacheRepoPath, m_backupSubJob.m_volInfo.m_vmMoRef, m_cacheRepoHandler);
    if (!m_dirtyRanges.FlushToStorage()) {
        ERRLOG("Save dirty range file failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_ifSaveValidDataBitMap == INITIAL_STATE) {
        m_ifSaveValidDataBitMap = IfSaveValidDataBitMap() ? SAVE_BLOCK_BIT_MAP : NOT_SAVE_BLOCK_BIT_MAP;
    }
    // 初始化块位图文件及指针
    if (m_ifSaveValidDataBitMap == SAVE_BLOCK_BIT_MAP && !InitCurSegBlockDataBitMap()) {
        ERRLOG("Init block bit map failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 下一个状态：执行DirtyRanges备份
    m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_BACKUP_DIRTY_RANGES);
    return SUCCESS;
}

std::string BackupJob::VolValidDataBitMapFile()
{
    return m_metaRepoPath + VIRT_PLUGIN_VOLUMES_BLOCK_BITMAP + m_volHandler->GetVolumeUUID() + "_block_bitmap.info";
}

bool BackupJob::InitBlockDataBitMapFile(const std::string &blockBitMapFile)
{
    if (m_metaRepoHandler->Exists(blockBitMapFile)) {
        return true;
    }
    if (m_metaRepoHandler->Open(blockBitMapFile, "w") != SUCCESS) { // 全量备份初始化磁盘bitmap文件
        ERRLOG("Create file %s failed, %s", blockBitMapFile.c_str(), m_taskInfo.c_str());
        return false;
    }
    if (m_metaRepoHandler->Close() != SUCCESS) {
        ERRLOG("Close file %s failed, %s", blockBitMapFile.c_str(), m_taskInfo.c_str());
        return false;
    }
    return true;
}

/**
 * @brief 全量备份则初始化块比特位，增量备份从文件中读取当前段块比特位图
 *
 * @return true
 * @return false
 */
bool BackupJob::InitCurSegBlockDataBitMap()
{
    if (m_blockDataBitMap == nullptr) {
        uint64_t curSegBlockCount = (m_curSegmentSize + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
        m_blockDataBitMap.reset(new uint8_t[curSegBlockCount], std::default_delete<uint8_t[]>());
        DBGLOG("Block data bit map size %ld, %s", curSegBlockCount, m_taskInfo.c_str());
    }
    uint64_t curSegBlockCount = (m_curSegmentSize + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
    if (m_backupPara->jobParam.backupType == AppProtect::BackupJobType::FULL_BACKUP) {
        memset_s(m_blockDataBitMap.get(), curSegBlockCount, '0', curSegBlockCount);
        return InitBlockDataBitMapFile(VolValidDataBitMapFile());
    }
    uint64_t confSegSize = GetSegementSizeFromConf();
    // 增量备份从bitmap表中读取出当前段的比特位图
    uint64_t perSegBlockCount = (confSegSize + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
    uint64_t curSegBlockStartIndex = m_curSegmentIndex * perSegBlockCount;
    if (m_metaRepoHandler->Open(VolValidDataBitMapFile(), "r") != SUCCESS) {
        ERRLOG("Open file %s failed, %s", VolValidDataBitMapFile().c_str(), m_taskInfo.c_str());
        return false;
    }
    Utils::Defer _(nullptr, [&](...) {
        m_metaRepoHandler->Close();
    });
    int retrytimes = 3;
    while (retrytimes) {
        if (m_metaRepoHandler->Seek(curSegBlockStartIndex) != SUCCESS) {
            ERRLOG("Seek dirty range index %llu failed, %s", curSegBlockStartIndex, m_taskInfo.c_str());
            Utils::SleepSeconds(RETRY_INTERVAL_SECOND);
            retrytimes--;
            continue;
        }
        if (m_metaRepoHandler->Read(m_blockDataBitMap, curSegBlockCount) == curSegBlockCount) {
            DBGLOG("Read data range count to file success, %s", m_taskInfo.c_str());
            return true;
        }
        ERRLOG("Read dirty range bitmap failed, index(%llu) size(%llu), %s", curSegBlockStartIndex, curSegBlockCount,
            m_taskInfo.c_str());
        Utils::SleepSeconds(RETRY_INTERVAL_SECOND);
        retrytimes--;
    }
    return false;
}

bool BackupJob::IfDiskExpand(uint64_t &preVolBlockCount)
{
    std::string preVolFilePath = m_cacheRepoPath + VIRT_PLUGIN_PRE_VOLUMES_INFO +
        m_volHandler->GetVolumeUUID() + ".ovf";
    if (!m_cacheRepoHandler->Exists(preVolFilePath)) {
        ERRLOG("No pre volume file %s,", preVolFilePath.c_str());
        return false;
    }
    VolInfo preVolInfo;
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, preVolFilePath, preVolInfo) != SUCCESS) {
        ERRLOG("Load pre volume info failed, path:%s, %s", preVolFilePath.c_str(), m_taskInfo.c_str());
        return false;
    }
    uint64_t curVolSize = m_volHandler->GetVolumeSize();
    preVolBlockCount = (preVolInfo.m_volSizeInBytes + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
    INFOLOG("Cur volume size %llu, pre volume size %llu, pre volume block count %llu", curVolSize,
        preVolInfo.m_volSizeInBytes, preVolBlockCount);
    return curVolSize > preVolInfo.m_volSizeInBytes;
}

void BackupJob::SetBlockDataFlag(uint64_t offset, uint64_t confSegSize)
{
    if (m_ifSaveValidDataBitMap == NOT_SAVE_BLOCK_BIT_MAP ||
        (offset + DIRTY_RANGE_BLOCK_SIZE - 1 < m_curSegmentIndex * confSegSize)) {
        DBGLOG("No need save bitmap or offset is less than n*60G.");
        return;
    }
    uint64_t curBlockIndex = (offset + DIRTY_RANGE_BLOCK_SIZE - 1 - m_curSegmentIndex * confSegSize)
        / DIRTY_RANGE_BLOCK_SIZE;
    m_blockDataBitMap[curBlockIndex] = '1';  // 1表示此4M块包括有效数据
}

/**
 *  @brief 执行子任务_对当前分段的 DirtyRanges进行备份
 *
 *  @return 0 成功，非0 失败
 */

int BackupJob::BackupDirtyRanges()
{
#ifndef WIN32
    if (Module::ConfigReader::getUint("VOLUME_DATA_PROCESS", "BACKUP_WITH_AIO") != 0)
        return BackupDirtyRangesAio();
#endif
    uint64_t confSegSize = GetSegementSizeFromConf();
    uint64_t curSegCompBlocksNum = 0;
    uint32_t tasksCount = 0; // 当前已放入调度器队列的IO任务数量
    int32_t tasksExecRet = SUCCESS;
    uint64_t preReportNum = 0;
    DirtyRanges::iterator it = m_dirtyRanges.begin(DIRTY_RANGE_BLOCK_SIZE);
    for (; (tasksExecRet != FAILED) && (!it.End());) {
        if (IsAbortJob()) {
            ERRLOG("Receive abort req, break job, %s", m_taskInfo.c_str());
            break;
        }
        std::shared_ptr<BackupIoTask> task = std::make_shared<BackupIoTask>(
                it->Offset(), it->size, m_isCopyVerify);
        task->SetVolumeHandler(m_volHandler);
        task->SetRepositoryHandler(m_dataRepoHandler);
        bool goNext = ExecBlockTaskStart(task, tasksCount, tasksExecRet, curSegCompBlocksNum, m_sha256Result);
        if (goNext) { // goNext用于标记能否继续往调度器中添加任务，如果不能则重新加入进行重试
            SetBlockDataFlag(it->Offset(), confSegSize);
            ++it;
        }
        if (tasksExecRet != SUCCESS) {
            break;
        }
        if (preReportNum != (m_completedDataSize / GB_SIZE)) {
            preReportNum = (m_completedDataSize / GB_SIZE);
            ReportBackupSpeed(m_completedDataSize); // 上报数据量
        }
    }
    ExecBlockTaskEnd(tasksCount, tasksExecRet, curSegCompBlocksNum, m_sha256Result);
    if (IsAbortJob() || tasksExecRet != SUCCESS || SaveBlockDataBitMap(confSegSize) != SUCCESS) {
        ERRLOG("Receive abort req or IO task exec failed or save block data failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_isCopyVerify && m_sha256Result) {
        m_sha256Result = WriteShaValueToFile(); // 分段备份完成执行写sha256文件操作
    }
    m_dirtyRanges.clear();
    m_dirtyRanges.CleanDirtyRangesFile();
    m_dataRepoHandler->Flush(true);
    m_completedSegmentSize += m_curSegmentSize;
    if (SaveCheckpointInfo() != SUCCESS) {
        ERRLOG("Save checkpoint info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    CheckNextSegment();
    return SUCCESS;
}

#ifndef WIN32
bool BackupJob::PrepareForAioBackup()
{
    INFOLOG("Backup data with aio");
    m_dirtyRangesForAio.clear();
    DirtyRanges::iterator it = m_dirtyRanges.begin(DIRTY_RANGE_BLOCK_SIZE);
    VirtPlugin::DirtyRange tDirtyRange;
    for (; (!it.End());) {
        tDirtyRange.start = it->Offset();
        tDirtyRange.size = it->size;
        m_dirtyRangesForAio.push_back(tDirtyRange);
        ++it;
    }
    if (m_dirtyRangesForAio.size() == 0) {
        ERRLOG("No need to handle");
        return true;
    }
    for (auto aa : m_dirtyRangesForAio) {
        DBGLOG("DERTYRANGE---start: %llu, length: %llu", aa.start, aa.size);
    }
    std::string filename = m_dataRepoPath + Module::PATH_SEPARATOR + m_backupSubJob.m_volInfo.m_uuid + ".raw";
    if (!Module::CMpString::FormattingPath(filename)) {
        ERRLOG("FormattingPath failed");
        return false;
    }
    int fh = open(filename.c_str(), O_WRONLY, 0644);
    if (fh < 0) {
        int errnoCopy = errno;
        std::stringstream err;
        err << "Failed to open " << filename << " for read-write-only, errno=" << errnoCopy;
        ERRLOG("err: %ls", err.str().c_str());
        return false;
    }
    m_writerFD = std::shared_ptr<int>(new (std::nothrow) int(fh), [](int* fd) {
        if (fd) {
            INFOLOG("begin to close");
            close(*fd);
            delete (fd);
            INFOLOG("end to close");
        }
    });
    m_aioLogger = std::make_shared<VirtualFrameworkDataMoverLog>();
    if (m_volHandler == nullptr || m_volHandler->GetDiskDeviceFile() == nullptr || m_aioLogger == nullptr ||
        m_volHandler->GetDiskDeviceFile()->GetDiskHandle() == nullptr || m_writerFD == nullptr) {
        ERRLOG("empty ptr");
        return false;
    }
    m_readFD = m_volHandler->GetDiskDeviceFile()->GetDiskHandle();
    return true;
}

void BackupJob::HandleAioBackupLoop(AsioDataMover& temp)
{
    while (temp.GetNumThreads() != 0) {
        if (IsAbortJob()) {
            INFOLOG("begin to abort");
            temp.isAbortRequested.store(true);
        }
        m_completedDataSize = temp.GetBytesTransfered();
        ReportBackupSpeed(m_completedDataSize);
        DBGLOG("completed blocks %llu, vecDirtyRange: %llu", (m_completedDataSize/DIRTY_RANGE_BLOCK_SIZE),
               m_dirtyRangesForAio.size());
        Utils::SleepSeconds(1);
    }
}

int BackupJob::BackupDirtyRangesAio()
{
    if (!PrepareForAioBackup()) {
        ERRLOG("PrepareForAioBackup failed");
        return FAILED;
    }
    if (m_dirtyRangesForAio.size() > 0) {
        std::vector<BlockShaData>* p = m_isCopyVerify ? &m_blockShaData : nullptr;
        AsioDataMover temp(m_dirtyRangesForAio, m_aioLogger, m_volHandler, p, true);
        temp.SetIfBackup(true);
        if (!temp.Init(*m_readFD, *m_writerFD, 0, DEFAULT_DATA_MOVER_CONFIG)) {
            ERRLOG("AsioDataMover Init failed");
            return FAILED;
        }
        HandleAioBackupLoop(temp);
        m_completedDataSize = temp.GetBytesTransfered();
        ReportBackupSpeed(m_completedDataSize);
        DBGLOG("completed blocks %llu, m_volumeInfo.vecDirtyRange: %llu",
            (m_completedDataSize/DIRTY_RANGE_BLOCK_SIZE), m_dirtyRangesForAio.size());
        if (m_completedDataSize != m_dirtyRangesForAio.size()) {
            WARNLOG("ERROR");
        }
        if (temp.IsAborted()) {
            ERRLOG("datamover execute failed");
            return FAILED;
        }
        ReportBackupSpeed(m_completedDataSize);
        m_sha256Result = temp.GetSha256Success();
    }
    if (m_isCopyVerify && m_sha256Result) {
        m_sha256Result = WriteShaValueToFile(); // 分段备份完成执行写sha256文件操作
    }
    m_dirtyRanges.clear();
    m_dirtyRanges.CleanDirtyRangesFile();
    m_dataRepoHandler->Flush(true);
    m_completedSegmentSize += m_curSegmentSize;
    if (SaveCheckpointInfo() != SUCCESS) {
        ERRLOG("Save checkpoint info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Backup data with aio finished");
    CheckNextSegment();
    return SUCCESS;
}
#endif

void BackupJob::CheckNextSegment()
{
    ++m_curSegmentIndex;
    ReportBackupSpeed(m_completedDataSize); // 上报数据量
    DBGLOG("DirtyRange backup completed, size:%llu , cur segment:%llu, total segment:%llu, %s",
        m_completedSegmentSize, m_curSegmentIndex, m_totalSegmentNum, m_taskInfo.c_str());
    if (m_curSegmentIndex < m_totalSegmentNum) {
        m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_GET_DIRTY_RANGES); // 继续备份下一个分段
    } else {
        m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_POSTHOOK); // 最后一个分段备份完成
        DBGLOG("All dirty range backup finished, size:%llu, %s", m_completedDataSize, m_taskInfo.c_str());
    }
}

/**
 *  @brief 执行子任务_执行Block备份任务
 *
 *  @return true 可继续向Scheduler中添加新的task任务，false 当前Scheduler中任务数达上限，不允许添加新的任务
 */
bool BackupJob::ExecBlockTaskStart(const std::shared_ptr<BackupIoTask> &task, uint32_t &tasksCount,
    int32_t &tasksExecRet, uint64_t &completedBlocksNum, bool& sha256Success)
{
    tasksExecRet = SUCCESS;
    if (tasksCount < m_maxTaskThreadNum) {
        if (!m_taskScheduler->Put(task)) {
            ERRLOG("Put io task to pool failed, %s", m_taskInfo.c_str());
            return false;
        }
        ++tasksCount;
        return true;
    }
    std::shared_ptr<BlockTask> res = nullptr;
    while (m_taskScheduler->Get(res, DEFAULT_BLOCK_TIMEOUT_MS)) {
        --tasksCount;
        // 目前res->Result（）有4种结果:1.SUCCESS 2.DATA_SAME_IGNORE_WRITE 3.DATA_ALL_ZERO_IGNORE_WRITE 4.FAILED
        if (res->Result() == FAILED) {
            ERRLOG("Block task exec failed, res:[%d], %s", res->Result(), m_taskInfo.c_str());
            tasksExecRet = FAILED;
        } else if (res->Result() == SUCCESS) {
            // 执行成功（SUCCESS:成功写入数据）才累计DirtyRange数据量
            ++completedBlocksNum;
            m_completedDataSize += DIRTY_RANGE_BLOCK_SIZE;
        }
        CalculateSha256Operator(res, tasksExecRet, sha256Success);
    }
    return false;
}

/**
 *  @brief 执行子任务_ 处理尚未结束的任务的执行结果
 *
 *  @return
 */
void BackupJob::ExecBlockTaskEnd(uint32_t &tasksCount, int32_t &tasksExecRet, uint64_t &completedBlocksNum,
    bool& sha256Success)
{
    std::shared_ptr<BlockTask> res;
    while (tasksCount > 0) {
        if (!m_taskScheduler->Get(res)) {
            ERRLOG("Get block task result failed, %s", m_taskInfo.c_str());
            tasksExecRet = FAILED;
            break;
        }
        if (res->Result() == FAILED) {
            ERRLOG("Block task exec failed, res:[%d], %s", res->Result(), m_taskInfo.c_str());
            tasksExecRet = FAILED;
        } else if (res->Result() == SUCCESS) {
            // 执行成功（SUCCESS:成功写入数据）才累计DirtyRange数据量
            ++completedBlocksNum;
            m_completedDataSize += DIRTY_RANGE_BLOCK_SIZE;
        }
        CalculateSha256Operator(res, tasksExecRet, sha256Success);
        --tasksCount;
    }
}

/**
 *  @brief 执行子任务_上报备份速度
 *
 *  @return
 */
void BackupJob::ReportBackupSpeed(const uint64_t &dataSizeInByte)
{
    std::string description = "Virtual plugin report sub job speed.";
    ReportJobDetailsParam param = {
        "",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING,
        dataSizeInByte * 100 / m_totalVolumeSize,
        dataSizeInByte / KB_SIZE};
    ReportJobDetailsWithLabel(param, description);
}

EXTER_ATTACK int BackupJob::PostJob()
{
    DBGLOG("In PostJob(), %s", GetTaskId().c_str());
    int ret = PostJobInner();
    ReportJobResult(ret, "PostJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int BackupJob::GetJobInfoBody()
{
    if (m_jobCommonInfo == nullptr) {
        ERRLOG("Job common info is null, %s", GetTaskId().c_str());
        return FAILED;
    }
    m_backupPara = std::dynamic_pointer_cast<AppProtect::BackupJob>(m_jobCommonInfo->GetJobInfo());
    if (m_backupPara == nullptr) {
        ERRLOG("Backup para is null, %s", GetTaskId().c_str());
        return FAILED;
    }
    m_taskInfo = GetTaskId();
    if (!m_backupPara->extendInfo.empty()) {
        Json::Value extendInfo;
        if (!Module::JsonHelper::JsonStringToJsonValue(m_backupPara->extendInfo, extendInfo)) {
            ERRLOG("Trans json value advanceparams failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
        if (extendInfo.isMember("copy_verify") && extendInfo["copy_verify"].compare("true") == SUCCESS) {
            INFOLOG("Generate sha256 file is true, %s", m_taskInfo.c_str());
            m_isCopyVerify = true;
        }
        if (extendInfo.isMember("esn")) {
            m_xNNEsn = extendInfo["esn"].asString();
            INFOLOG("ESN provided for backup job: %s, %s", m_xNNEsn.c_str(), m_taskInfo.c_str());
            return SUCCESS;
        }
    }

    DBGLOG("GetJobInfoBody success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::CommonInit()
{
    if (GetJobInfoBody() != SUCCESS) {
        ERRLOG("Get job info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    if (!InitHandlers()) {
        ERRLOG("Init backup job failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void BackupJob::InitPreStateMachine()
{
    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_INIT);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_INIT)]
        = std::bind(&BackupJob::PrerequisiteInit, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_PREHOOK)]
        = std::bind(&BackupJob::PrerequisitePreHook, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STATE_PRE_CHECK_BEFORE_BACKUP)]
        = std::bind(&BackupJob::CheckBeforeBackup, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_CREATE_SNAPSHOT)]
        = std::bind(&BackupJob::CreateSnapshot, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_ACTIVE_SNAPSHOT_CONSISTENCY)]
        = std::bind(&BackupJob::ActiveSnapShotConsistency, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_GET_MACHINE_METADATA)]
        = std::bind(&BackupJob::GetMachineMetadata, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_GET_VOLUMES_METADATA)]
        = std::bind(&BackupJob::GetVolumesMetadata, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_INIT_REPO_PATH)]
        = std::bind(&BackupJob::InitRepoPath, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_SAVE_METADATA)]
        = std::bind(&BackupJob::SaveMetadata, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_PRE_POSTHOOK)]
        = std::bind(&BackupJob::PrerequisitePostHook, this);
}

int BackupJob::CheckBeforeBackup()
{
    if (m_protectEngine->CheckBeforeBackup() != SUCCESS) {
        ERRLOG("Check before backup failed, %s", m_taskInfo.c_str());
        ReportJobDetailWithErrorParams();
        return FAILED;
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_CREATE_SNAPSHOT);
    return SUCCESS;
}

int BackupJob::PrerequisiteInit()
{
    DBGLOG("In PrerequisiteInit(), %s", GetTaskId().c_str());
    if (CommonInit() != SUCCESS) {
        ERRLOG("Init backup job failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 存在主任务状态文件 则表明已经执行过当前任务了，直接返回成功
    if (CheckMainTaskStatusFileExist()) {
        INFOLOG("The main task has been executed.skip gen sub job. %s", m_taskInfo.c_str());
        m_nextState = static_cast<int>(BackupJobSteps::STATE_NONE);
        return SUCCESS;
    }
    if (InitCheckpointFolder() != SUCCESS) {
        ERRLOG("Checkpoint Folder initialization failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (GenShaInterruptFile() != SUCCESS) {
        ERRLOG("Gen sha interrupt file failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_PREHOOK);
    return SUCCESS;
}

bool BackupJob::InitHandlers()
{
    if (InitProtectEngineHandler(JobType::BACKUP) != SUCCESS) {
        ERRLOG("Initialize protect engine handler failed, %s", m_taskInfo.c_str());
        return false;
    }
    /* Set X Series all-in-one Box ESN to ProtectEngine. */
    m_protectEngine->SetXNNEsn(m_xNNEsn, m_taskInfo);
    for (const auto &repo : m_backupPara->repositories) {
        if (repo.repositoryType == RepositoryDataType::DATA_REPOSITORY) {
            if (!DoInitHandlers(repo, m_dataRepoHandler, m_dataRepoPath)) {
                ERRLOG("Init data repo handler failed, %s", m_taskInfo.c_str());
                return false;
            }
        } else if (repo.repositoryType == RepositoryDataType::META_REPOSITORY) {
            if (!DoInitHandlers(repo, m_metaRepoHandler, m_metaRepoPath)) {
                ERRLOG("Init meta repo handler failed, %s", m_taskInfo.c_str());
                return false;
            }
        } else if (repo.repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            if (!DoInitHandlers(repo, m_cacheRepoHandler, m_cacheRepoPath)) {
                ERRLOG("Init cache repo handler failed, %s", m_taskInfo.c_str());
                return false;
            }
        }
    }
    if (m_dataRepoHandler == nullptr || m_cacheRepoHandler == nullptr || m_metaRepoHandler == nullptr) {
        ERRLOG("Init repo handler failed. taskId:%s.", m_taskInfo.c_str());
        return false;
    }
    INFOLOG("Init repository handlers success, %s", m_taskInfo.c_str());
    return true;
}

int BackupJob::CreateSnapshot()
{
    std::string errCode;
    if (m_protectEngine->CreateSnapshot(m_snapshotInfo, errCode) != SUCCESS) {
        ERRLOG("Create snapshot failed, %s", m_taskInfo.c_str());

        std::string description = "Virtual plugin backup create snapshot failed.";
        ReportJobDetailsParam param;
        if (errCode == "APIGW.0308") {
            WARNLOG("Create snapshot failed, begin report label apigw.0308");
            param = {
                "virtual_plugin_backup_job_create_snapshot_apigw_failed_label",
                JobLogLevel::TASK_LOG_ERROR,
                SubJobStatus::FAILED, 0, 0 };
        } else if (errCode.empty()) {
            param = { "virtual_plugin_backup_job_create_snapshot_failed_label", JobLogLevel::TASK_LOG_ERROR,
                        SubJobStatus::FAILED, 0, 0, INVOKE_API_FAILED_GENERAL_CODE};
        } else {
            param = { "virtual_plugin_backup_job_create_snapshot_failed_label", JobLogLevel::TASK_LOG_ERROR,
                        SubJobStatus::FAILED, 0, 0, atoi(errCode.c_str())};
        }
        ReportJobDetailsWithLabel(param, description);
        return FAILED;
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_ACTIVE_SNAPSHOT_CONSISTENCY);
    INFOLOG("Create snapshot success, vmRef:%s, %s", m_vmInfo.m_moRef.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::ActiveSnapShotConsistency()
{
    if (m_protectEngine->ActiveSnapConsistency(m_snapshotInfo) != SUCCESS) {
        WARNLOG("Active snap consistency failed, %s", m_taskInfo.c_str());
        ReportJobDetailsParam param = {"virtual_plugin_backup_job_active_snapshot_consistency_failed",
                                       JobLogLevel::TASK_LOG_WARNING,
                                       SubJobStatus::FAILED, 0, 0};
        std::string description = "Virtual plugin active snapshot consistency failed.";
        ReportJobDetailsWithLabel(param, description);
        return FAILED;
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_GET_MACHINE_METADATA);
    return SUCCESS;
}

int BackupJob::GetMachineMetadata()
{
    if (m_protectEngine->GetMachineMetadata(m_vmInfo) != SUCCESS) {
        ERRLOG("Get machine metadata failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    GetProtectSubObjects();
    VolumeFilter();

    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_GET_VOLUMES_METADATA);
    INFOLOG("Get mechine metadata success, vm name: %s, %s", m_vmInfo.m_name.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

void BackupJob::GetProtectSubObjects()
{
    if (m_backupPara->protectSubObject.size() == 0) {
        /* if no sub object provided, backup all volumes */
        m_volToBeBackupMap.clear();
    } else {
        for (const auto &volObj : m_backupPara->protectSubObject) {
            m_volToBeBackupMap[volObj.id] = volObj;
        }
    }
}

void BackupJob::VolumeFilter()
{
    /* backup all volumes */
    if (m_volToBeBackupMap.size() == 0) {
        DBGLOG("Backup all volumes, %s", m_taskInfo.c_str());
        return;
    }
    /* backup volumes in the protectSubObject list */
    auto it = m_vmInfo.m_volList.begin();
    while (it != m_vmInfo.m_volList.end()) {
        if (m_volToBeBackupMap.find(it->m_uuid) == m_volToBeBackupMap.end()) {
            ReportJobDetailsParam param = {
                "virtual_plugin_backup_job_volume_not_protect_label",
                JobLogLevel::TASK_LOG_WARNING,
                SubJobStatus::RUNNING, 0, 0 };
            ReportJobDetailsWithLabel(param, it->m_uuid);
            it = m_vmInfo.m_volList.erase(it);
            continue;
        } else {
            /* save job subObj extend to volList extend info,
             * application can overwrite it if app has it's own one
             */
            // 总是获取最新的信息，若本次备份有设置volume-extendInfo，则以本次获取的信息为准，若没有则设置为传入的备份参数内的扩展信息
            // 错误场景：HCS磁盘扩容后，副本磁盘大小显示错误
            if (it->m_extendInfo.empty()) {
                it->m_extendInfo = m_volToBeBackupMap[it->m_uuid].extendInfo;
            }
            ++it;
        }
    }
}

int BackupJob::GetVolumesMetadata()
{
    DBGLOG("In GetVolumesMetadata(), %s", m_taskInfo.c_str());
    /* Do nothing - volume metadata returned from GetMachineMetadata */
    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_INIT_REPO_PATH);
    INFOLOG("Get volume metadata success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::InitRepoPath()
{
    std::string volPath = m_metaRepoPath + VIRT_PLUGIN_VOLUMES_META_DIR;
    DBGLOG("Checking volumes metadata directory: %s, %s", volPath.c_str(), m_taskInfo.c_str());
    if (!m_metaRepoHandler->Exists(volPath)) {
        DBGLOG("Creating volumes metadata directory: %s, %s", volPath.c_str(), m_taskInfo.c_str());
        int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_metaRepoHandler,
            volPath), true, "CreateDirectory");
        if (res != SUCCESS) {
            ERRLOG("Create volumes metadata directory failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }

    std::string preVolPath = m_cacheRepoPath + VIRT_PLUGIN_PRE_VOLUMES_INFO;
    if (!m_cacheRepoHandler->Exists(preVolPath)) {
        DBGLOG("Creating pre volumes cache directory: %s, %s", preVolPath.c_str(), m_taskInfo.c_str());
        int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_cacheRepoHandler,
            preVolPath), true, "CreateDirectory");
        if (res != SUCCESS) {
            ERRLOG("Create pre volumes cache directory failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }

    std::string volBlockBitmapDir = m_metaRepoPath + VIRT_PLUGIN_VOLUMES_BLOCK_BITMAP;
    if (!m_metaRepoHandler->Exists(volBlockBitmapDir)) {
        DBGLOG("Creating volumes block directory: %s, %s", volBlockBitmapDir.c_str(), m_taskInfo.c_str());
        int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_metaRepoHandler,
            volBlockBitmapDir), true, "CreateDirectory");
        if (res != SUCCESS) {
            ERRLOG("Create volumes block directory(%s) failed, %s", volBlockBitmapDir.c_str(), m_taskInfo.c_str());
            return FAILED;
        }
    }
    std::string backupJobPath = m_metaRepoPath + VIRT_PLUGIN_META_BACKUPJOB_ROOT;
    DBGLOG("Checking backup job directory: %s, %s", backupJobPath.c_str(), m_taskInfo.c_str());
    if (!m_metaRepoHandler->Exists(backupJobPath)) {
        DBGLOG("Creating backup job directory: %s, %s", backupJobPath.c_str(), m_taskInfo.c_str());
        int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_metaRepoHandler,
            backupJobPath), true, "CreateDirectory");
        if (res != SUCCESS) {
            ERRLOG("Create backup job directory failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }

    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_SAVE_METADATA);
    INFOLOG("Create virtual plugin repository directories success, %s", m_taskInfo.c_str());
    return SUCCESS;
}


int BackupJob::SaveMetadata()
{
    DBGLOG("In SaveMetadata(), %s", m_taskInfo.c_str());
    /* save pre volume info to cache */
    if (SavePreVolInfoToCache() != SUCCESS) {
        ERRLOG("Save pre vol info to cache failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    /* save snapshot info */
    if (SaveSnapshotInfo() != SUCCESS) {
        ERRLOG("Save snapshot info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    /* save volumes metadata */
    if (SaveVolumesMetadata() != SUCCESS) {
        ERRLOG("Save volume metadata failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    /* save vm info */
    if (SaveVMInfo() != SUCCESS) {
        ERRLOG("Save vm info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_POSTHOOK);
    INFOLOG("Save metadata success, %s", m_taskInfo.c_str());

    return SUCCESS;
}

int BackupJob::SavePreVolInfoToCache()
{
    if (m_backupPara->jobParam.backupType == AppProtect::BackupJobType::FULL_BACKUP) {
        INFOLOG("Full backup, no pre volinfo, dont save.");
        return SUCCESS;
    }
    std::string volCachePath = m_cacheRepoPath + VIRT_PLUGIN_PRE_VOLUMES_INFO;
    for (const auto &vol : m_vmInfo.m_volList) {
        std::string volMetaPath = m_metaRepoPath + VIRT_PLUGIN_VOLUMES_META_DIR + vol.m_uuid + ".ovf";
        if (!m_metaRepoHandler->Exists(volMetaPath)) {
            continue;
        }
        if (!m_cacheRepoHandler->CopyFile(volMetaPath, volCachePath)) {
            ERRLOG("Copy vol info %s to cache failed, %s.", volMetaPath.c_str(), m_taskInfo.c_str());
            return  FAILED;
        }
        INFOLOG("Copy vol info %s to cache  success, %s", volMetaPath.c_str(), m_taskInfo.c_str());
    }
    return SUCCESS;
}

/* save snapshot info */
int BackupJob::SaveSnapshotInfo()
{
    std::string snapshotInfoStr;
    if (!Module::JsonHelper::StructToJsonString(m_snapshotInfo, snapshotInfoStr)) {
        ERRLOG("Convert snapinfo to json string failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    std::string snapshotInfoMetaFile = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
    if (Utils::SaveToFileWithRetry(m_metaRepoHandler, snapshotInfoMetaFile, snapshotInfoStr) != SUCCESS) {
        return FAILED;
    }

    INFOLOG("Save snapshot info success, vmRef: %s, %s", m_vmInfo.m_moRef.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

/* save volumes metadata */
int BackupJob::SaveVolumesMetadata()
{
    for (auto &volume : m_vmInfo.m_volList) {
        std::string volStr;
#ifndef WIN32
        std::string volumeFileName = m_metaRepoPath + VIRT_PLUGIN_META_ROOT + "/volumes/" + volume.m_uuid + ".ovf";
#else
        std::string volumeFileName = m_metaRepoPath + VIRT_PLUGIN_META_ROOT + "volumes\\" + volume.m_uuid + ".ovf";
#endif
        if (!Module::JsonHelper::StructToJsonString(volume, volStr)) {
            ERRLOG("Convert volume info to json string failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
        if (Utils::SaveToFileWithRetry(m_metaRepoHandler, volumeFileName, volStr) != SUCCESS) {
            ERRLOG("Save volume info failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
        /* Note: We dont serialize metadata in vmInfo struct, so clear it after saved to volumes dir. */
        volume.m_metadata.clear();
        DBGLOG("Save volumes metadata success. Volume moRef: %s, %s", volume.m_uuid.c_str(), m_taskInfo.c_str());
    }
    INFOLOG("Save volumes metadata success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

/* save vm info */
int BackupJob::SaveVMInfo()
{
    std::string vmInfoStr;
    if (!Module::JsonHelper::StructToJsonString(m_vmInfo, vmInfoStr)) {
        ERRLOG("Convert vm info to json string failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    std::string vmInfoMetaFile = m_metaRepoPath + VIRT_PLUGIN_VM_INFO;
    if (Utils::SaveToFileWithRetry(m_metaRepoHandler, vmInfoMetaFile, vmInfoStr) != SUCCESS) {
        ERRLOG("Save vm info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    INFOLOG("Save vm info success. VM name: %s, %s", m_vmInfo.m_name.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::GenerateSubJobInner()
{
    DBGLOG("Begin to exeute backup generate sub job, %s", m_taskInfo.c_str());
    InitGenerateSubJobStateMachine();
    return RunStateMachine();
}

void BackupJob::InitGenerateSubJobStateMachine()
{
    m_nextState = static_cast<int>(BackupJobSteps::STEP_GENERATE_SUBJOB_INIT);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_GENERATE_SUBJOB_INIT)]
        = std::bind(&BackupJob::GenerateSubJobInit, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_GENERATE_PREHOOK)]
        = std::bind(&BackupJob::GenerateSubJobPreHook, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_GENERATE_LOAD_PRE_SNAPSHOT_INFO)]
        = std::bind(&BackupJob::LoadPreSnapshotInfo, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_GENERATE_LOAD_VM_METADATA)]
        = std::bind(&BackupJob::LoadVMMetadata, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_GENERATE_DO_GENERATE_SUBJOB)]
        = std::bind(&BackupJob::DoGenerateSubJob, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_GENERATE_POSTHOOK)]
        = std::bind(&BackupJob::GenerateSubJobPostHook, this);
}

int BackupJob::GenerateSubJobInit()
{
    DBGLOG("In GenerateSubJobInit(), %s", GetTaskId().c_str());
    if (CommonInit() != SUCCESS) {
        /* Log error inside. */
        return FAILED;
    }
    // 存在主任务状态文件 则表明已经执行过当前任务了，直接返回成功
    if (CheckMainTaskStatusFileExist()) {
        INFOLOG("The main task has been executed.skip gen sub job. %s", m_taskInfo.c_str());
        m_nextState = static_cast<int>(BackupJobSteps::STATE_NONE);
        return SUCCESS;
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_GENERATE_PREHOOK);
    return SUCCESS;
}

int BackupJob::LoadPreSnapshotInfo()
{
    std::string preSnapFile = m_metaRepoPath + VIRT_PLUGIN_PRE_SNAPSHOT_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, preSnapFile, m_preSnapshotInfo) != SUCCESS) {
        INFOLOG("Load pre snapshot info failed, %s", m_taskInfo.c_str());
        /* Not return FAILED if no pre-snapshot found. */
    }

    std::string curSnapFile = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, curSnapFile, m_snapshotInfo) != SUCCESS) {
        ERRLOG("Load current snapshot info failed, path:%s, %s", curSnapFile.c_str(), m_taskInfo.c_str());
        return FAILED;
    }

    m_nextState = static_cast<int>(BackupJobSteps::STEP_GENERATE_LOAD_VM_METADATA);
    INFOLOG("Load snapshot info success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::LoadVMMetadata()
{
    std::string vmInfoFile = m_metaRepoPath + VIRT_PLUGIN_VM_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, vmInfoFile, m_vmInfo) != SUCCESS) {
        ERRLOG("Load previous metadata failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_GENERATE_DO_GENERATE_SUBJOB);
    INFOLOG("Load previous metadata success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

bool BackupJob::FillUpBackupSubJob(const VolInfo &vol, BackupSubJobInfo &subJobInfo)
{
    INFOLOG("Generate sub job: Handling volume: %s, %s", vol.m_uuid.c_str(), m_taskInfo.c_str());
    subJobInfo.m_volInfo = vol;
    for (const auto &snap : m_preSnapshotInfo.m_volSnapList) {
        if (snap.m_volUuid == vol.m_uuid) {
            subJobInfo.m_preSnapshotInfo = snap;
        }
    }

    for (const auto &snap : m_snapshotInfo.m_volSnapList) {
        if (snap.m_volUuid == vol.m_uuid && !snap.m_volUuid.empty()) {
            subJobInfo.m_curSnapshotInfo = snap;
        }
    }

    if (subJobInfo.m_curSnapshotInfo.m_snapshotId.empty()
        || subJobInfo.m_curSnapshotInfo.m_volUuid.empty()) {
        ERRLOG("No snapshot found for current job, %s", m_taskInfo.c_str());
        return false;
    }

    return true;
}

int BackupJob::WriteGenerateSha256State(const int32_t execState)
{
    if (execState != SUCCESS) {
        return FAILED;
    }
    if (m_isCopyVerify && m_sha256Result) {
        INFOLOG("Generate sha256 success, %s", m_taskInfo.c_str());
        return SUCCESS;
    }

    VerifyFileStates verifyState;
    verifyState.m_verifyFileState = "false";

    std::string strShaFile = "";
    if (!Module::JsonHelper::StructToJsonString(verifyState, strShaFile)) {
        ERRLOG("Convert sha256 file state to string failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    std::string shaStateFilePath = m_cacheRepoPath + Module::PATH_SEPARATOR + SHA256_FILE_FAILED_STATE;
    if (Utils::SaveToFileWithRetry(m_cacheRepoHandler, shaStateFilePath, strShaFile) != SUCCESS) {
        ERRLOG("Save sha256file states failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    INFOLOG("Save sha256file failed states success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::FormatSubJobItems(const VolInfo &vol, std::vector<SubJob> &subJobs, const int index)
{
    BackupSubJobInfo subJobInfo;
    if (!FillUpBackupSubJob(vol, subJobInfo)) {
        ERRLOG("Format sub job info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    std::string subJobInfoStr;
    if (!Module::JsonHelper::StructToJsonString(subJobInfo, subJobInfoStr)) {
        ERRLOG("Failed to convert backup subjob struct to string, %s", m_taskInfo.c_str());
        return FAILED;
    }

    SubJob subJob;
    int priority = 1;
    subJob.__set_jobPriority(priority);
    subJob.__set_jobId(m_backupPara->jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName(BUSINESS_SUB_JOB_NAME_PREFIX + std::to_string(index));
    subJob.__set_policy(ExecutePolicy::ANY_NODE);
    subJob.__set_jobInfo(subJobInfoStr);
    subJobs.emplace_back(subJob);

    return SUCCESS;
}

int BackupJob::DoGenerateSubJob()
{
    ActionResult result = {};
    std::vector<SubJob> subJobs {};
    int index = 0;
    for (const auto &vol : m_vmInfo.m_volList) {
        if (IsAbortJob()) {
            ERRLOG("Receive abort req, stop generating sub job, %s", m_taskInfo.c_str());
            return FAILED;
        }
        if (!vol.m_supportBackup) {
            WARNLOG("Vol(%s) not support backup, %s", vol.m_name.c_str(), m_taskInfo.c_str());
            ReportJobDetailsParam param = {
                "virtual_plugin_backup_job_vol_not_support",
                JobLogLevel::TASK_LOG_WARNING,
                SubJobStatus::RUNNING, 0, 0 };
            ReportJobDetailsWithLabel(param, vol.m_name);
            continue;
        }
        if (FormatSubJobItems(vol, subJobs, index) != SUCCESS) {
            ERRLOG("Format sub job items failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
        ++index;
    }

    if (subJobs.empty()) {
        ERRLOG("No sub job generated, %s", m_taskInfo.c_str());
        return FAILED;
    }

    SubJob subJob {};
    subJob.__set_jobId(m_backupPara->jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName(REPORT_COPY_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::ANY_NODE);
    int priority = 2;
    subJob.__set_jobPriority(priority);
    subJob.__set_jobInfo("{}");
    subJobs.emplace_back(subJob);

    if (!AddNewJobWithRetry(subJobs)) {
        ERRLOG("Add new job fail, %s", m_taskInfo.c_str());
        return FAILED;
    }
    auto ret = GenMainTaskStatusToFile();
    if (ret != SUCCESS) {
        WARNLOG("Failed to save main task status to file, %s", m_taskInfo.c_str());
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_GENERATE_POSTHOOK);
    INFOLOG("Generate sub job success, main task id: %s", m_backupPara->jobId.c_str());

    return SUCCESS;
}

int BackupJob::GenMainTaskStatusToFile()
{
    std::string genMainTaskStatusInfoStr = "Sub job has been generated.";
    std::string mainTaskStatusInfoFile = m_cacheRepoPath + VIRT_PLUGIN_GEN_MAIN_TASK_STATUS_INFO;
    if (Utils::SaveToFileWithRetry(m_cacheRepoHandler, mainTaskStatusInfoFile, genMainTaskStatusInfoStr) != SUCCESS) {
        ERRLOG("Failed to generate main task status to file, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Generate main task status to file success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

bool BackupJob::CheckMainTaskStatusFileExist()
{
    std::string mainTaskStatusInfoFile = m_cacheRepoPath + VIRT_PLUGIN_GEN_MAIN_TASK_STATUS_INFO;
    return m_cacheRepoHandler->Exists(mainTaskStatusInfoFile);
}


int BackupJob::PostJobInner()
{
    InitPostJobStateMachine();
    return RunStateMachine();
}

void BackupJob::InitPostJobStateMachine()
{
    m_nextState = static_cast<int>(BackupJobSteps::STEP_POST_JOB_INIT);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_POST_JOB_INIT)]
        = std::bind(&BackupJob::PostJobInit, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_POST_PREHOOK)]
        = std::bind(&BackupJob::PostJobPreHook, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_POST_CLEANUP_SNAPSHOT)]
        = std::bind(&BackupJob::CleanupSnapshot, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_POST_UPDATE_SNAPSHOT_FILE)]
        = std::bind(&BackupJob::UpdateSnapshotFile, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_POST_CLEANUP_CHECKPOINT)]
        = std::bind(&BackupJob::CleanCheckpoint, this);
    m_stateHandles[static_cast<int>(BackupJobSteps::STEP_POST_POSTHOOK)]
        = std::bind(&BackupJob::PostJobPostHook, this);
}

int BackupJob::PostJobInit()
{
    DBGLOG("In PostJobInit(), %s", GetTaskId().c_str());
    if (CommonInit() != SUCCESS) {
        /* Log error inside. */
        return FAILED;
    }
    // 初始化续作点
    PostJobInitCheckpoint();
    m_nextState = static_cast<int>(BackupJobSteps::STEP_POST_PREHOOK);
    INFOLOG("PostJobInit success, %s", m_taskInfo.c_str());

    return SUCCESS;
}

int BackupJob::CleanupSnapshot()
{
    // try to clear residual snapshot and the alarm already send
    TryClearResidualSnapshotsAndAlarm();

    if (GetSnapshotToBeDelete() != SUCCESS) {
        ERRLOG("Get snapshot list failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    /* Try delete snapshot. */
    TryDeleteSnapshots();

    /* save list for next round */
    if (SaveDeleteList() != SUCCESS) {
        ERRLOG("Save snapshot list failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    m_nextState = static_cast<int>(BackupJobSteps::STEP_POST_UPDATE_SNAPSHOT_FILE);
    DBGLOG("END CleanupSnapshot(), %s", m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::CleanCheckpoint()
{
    INFOLOG("Start clean checkpoint file, %s", m_taskInfo.c_str());
    if (!m_checkpoint.Clear()) {
        ERRLOG("Clean checkpoint failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_nextState = static_cast<int>(BackupJobSteps::STEP_POST_POSTHOOK);
    return SUCCESS;
}

int BackupJob::GetSnapshotToBeDelete()
{
    /* 1. Load snapshot list to be deleted; */
    std::string snapsToBeDeletedFile = m_metaRepoPath + VIRT_PLUGIN_SNAP_TOBEDELETED;
    if (m_metaRepoHandler->Exists(snapsToBeDeletedFile)) {
        INFOLOG("Load snapshot list to be deleted, %s", m_taskInfo.c_str());
        if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, snapsToBeDeletedFile,
            m_snapListToBeDeleted) != SUCCESS) {
            ERRLOG("Load snapshot info failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }

    SnapshotInfo saveSnapInfo;
    if (m_protectEngine->IfDeleteLatestSnapShot()) {
        INFOLOG("Engine need delete all provided snapshot. %s.", m_taskInfo.c_str());
        LoadSnapshotsOfVMToBeDeleted(saveSnapInfo.m_volSnapList);
        return SUCCESS;
    }

    std::string curSnapshotInfoMetaFile = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
    if (m_jobResult != AppProtect::JobResult::type::SUCCESS) {
        INFOLOG("Task is not success, need delete snapshot which created in this time. %s.", m_taskInfo.c_str());
        LoadSnapshotToBeDeleted(curSnapshotInfoMetaFile);
        return SUCCESS;
    }

    INFOLOG("Need save snapshot which created in this time. %s.", m_taskInfo.c_str());
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, curSnapshotInfoMetaFile, saveSnapInfo) != SUCCESS) {
        WARNLOG("No save snapshot load from file(%s). %s", curSnapshotInfoMetaFile.c_str(), m_taskInfo.c_str());
        return FAILED;
    }

    if (m_backupPara->jobParam.backupType == AppProtect::BackupJobType::INCREMENT_BACKUP) {
        INFOLOG("Add previous snapshot info to the list when inc backup. %s.", m_taskInfo.c_str());
        std::string preSnapshotInfoMetaFile = m_metaRepoPath + VIRT_PLUGIN_PRE_SNAPSHOT_INFO;
        LoadSnapshotToBeDeleted(preSnapshotInfoMetaFile);
    } else {
        INFOLOG("Add volume's snapshots to the list except current snapshot when full backup. %s", m_taskInfo.c_str());
        LoadSnapshotsOfVMToBeDeleted(saveSnapInfo.m_volSnapList);
    }

    return SUCCESS;
}

void BackupJob::LoadSnapshotsOfVMToBeDeleted(const std::vector<VolSnapInfo>& exVolSnaps)
{
    std::string vmInfoFile = m_metaRepoPath + VIRT_PLUGIN_VM_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, vmInfoFile, m_vmInfo) != SUCCESS) {
        ERRLOG("Load vm info failed, %s", m_taskInfo.c_str());
        return;
    }
    SnapshotInfo snapInfo;
    for (const auto &volInfo : m_vmInfo.m_volList) {
        INFOLOG("Volume to check snapshot: %s, %s", volInfo.m_uuid.c_str(), m_taskInfo.c_str());
        std::vector<VolSnapInfo> snapList;
        if (m_protectEngine->GetSnapshotsOfVolume(volInfo, snapList) != SUCCESS) {
            ERRLOG("Get Snapshot list of volume failed, volId: %s, %s", volInfo.m_uuid.c_str(), m_taskInfo.c_str());
            continue;
        }
        for (const auto &volSnap : snapList) {
            if (IsSnapInVolSnapshots(volSnap, exVolSnaps)) {
                DBGLOG("Snap(%s) need save.", volSnap.m_snapshotName.c_str());
                continue;
            }
            snapInfo.m_volSnapList.push_back(volSnap);
        }
    }
    if (snapInfo.m_volSnapList.empty()) {
        INFOLOG("No more snapshots exist, vmId: %s, %s", m_vmInfo.m_uuid.c_str(), m_taskInfo.c_str());
        return;
    }
    snapInfo.m_vmMoRef = m_vmInfo.m_moRef;
    snapInfo.m_vmName = m_vmInfo.m_name;
    snapInfo.m_deleted = false; // 默认值设置为false，表示快照未删除，需要进行删除
    SnapToBeDeleted snap;
    snap.m_snapshotInfo = snapInfo;
    DBGLOG("Add snapshots(%s) to delete list, %s",
        GetSnapshotsLogDetails(snapInfo.m_volSnapList).c_str(), m_taskInfo.c_str());
    m_snapListToBeDeleted.m_snapList.push_back(snap);
}

void BackupJob::LoadSnapshotToBeDeleted(const std::string &file)
{
    SnapToBeDeleted snapToBeDeleted;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, file, snapToBeDeleted.m_snapshotInfo) != SUCCESS) {
        WARNLOG("No snapshot loaded from file(%s) to be deleted, %s", file.c_str(), m_taskInfo.c_str());
        return;
    }
    m_snapListToBeDeleted.m_snapList.push_back(snapToBeDeleted);
    ShowSnapshot(snapToBeDeleted.m_snapshotInfo);
    DBGLOG("From file(%s) load snapshot(%s) to be deleted, %s.", file.c_str(),
        GetSnapshotsLogDetails(snapToBeDeleted.m_snapshotInfo.m_volSnapList).c_str(),  m_taskInfo.c_str());
}

void BackupJob::ShowSnapshot(const SnapshotInfo &snap)
{
    DBGLOG("VM: %s, snapshot: %S, including volume snapshots, %s", snap.m_vmMoRef.c_str(), snap.m_moRef.c_str(),
        m_taskInfo.c_str());
    for (const auto &volSnap : snap.m_volSnapList) {
        DBGLOG("- Volume: %s, snapshot: %s, %s", volSnap.m_volUuid.c_str(), volSnap.m_snapshotId.c_str(),
            m_taskInfo.c_str());
    }
}

int BackupJob::SaveCheckpointInfo()
{
    m_backupCheckpointInfo.m_completedSegmentSize = m_completedSegmentSize;
    m_backupCheckpointInfo.m_completedDataSize = m_completedDataSize;
    if (!m_checkpoint.Set(m_backupCheckpointInfo)) {
        ERRLOG("Set checkpoint failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    DBGLOG("Set checkpoint successfully, m_completedSegmentSize:%llu, %s",
        m_backupCheckpointInfo.m_completedSegmentSize, m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::GenShaInterruptFile()
{
    // 增量备份：未启生成校验文件。存在校验文件信息 --生成中断校验文件；
    if (m_backupPara->jobParam.backupType == AppProtect::BackupJobType::INCREMENT_BACKUP && !m_isCopyVerify) {
        bool isSha256FileExist = false;
        std::string shaPathName = m_metaRepoPath + VIRT_PLUGIN_SHA_FILE_ROOT;
        std::vector <std::string> fileNames{};
        m_metaRepoHandler->GetFiles(shaPathName, fileNames);
        for (auto fileName: fileNames) {
            if (fileName.find("_sha256.info") != std::string::npos) {
                isSha256FileExist = true;
                INFOLOG("Find the sha 256 file, will generate sha interrupt file.");
                break;
            }
        }
        if (isSha256FileExist) {
            std::string shaInterruptStr = "Current sha file is interrupt";
            std::string shaStateFilePath = m_metaRepoPath + VIRT_PLUGIN_SHA_INTERRUPT_INFO;
            if (Utils::SaveToFileWithRetry(m_metaRepoHandler, shaStateFilePath, shaInterruptStr) !=
                SUCCESS) {
                ERRLOG("Save sha interrupt file failed.");
                return FAILED;
            }
            INFOLOG("Save sha interrupt file success.");
        }
    }
    return SUCCESS;
}

int BackupJob::InitCheckpointFolder()
{
    m_checkpoint.SetHandle(m_cacheRepoHandler, m_cacheRepoPath);
    m_checkpoint.SetMainJobPath(m_backupPara->jobId);
    if (!m_checkpoint.CreateCheckpointDirectory()) {
        ERRLOG("Create backup checkpoint main folder failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Init checkpoint Folder successfully, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::InitCheckpoint()
{
    m_checkpoint.SetHandle(m_cacheRepoHandler, m_cacheRepoPath);
    m_checkpoint.SetJobId(m_subJobInfo->jobId, m_subJobInfo->subJobId);
    if (m_checkpoint.Exist()) {
        return SUCCESS;
    }
    if (!m_checkpoint.CreateCheckpointDirectory()) {
        ERRLOG("Create backup checkpoint directory failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (!m_checkpoint.Create()) {
        ERRLOG("Create backup checkpoint file failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_checkpoint.Set(m_backupCheckpointInfo);
    INFOLOG("Init checkpoint success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::PostJobInitCheckpoint()
{
    m_checkpoint.SetHandle(m_cacheRepoHandler, m_cacheRepoPath);
    m_checkpoint.SetMainJobPath(m_subJobInfo->jobId);
    INFOLOG("PostJobInitCheckpoint success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::UpdateInfoByCheckpoint()
{
    if (!m_checkpoint.Get(m_backupCheckpointInfo)) {
        ERRLOG("Get checkpoint failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    DBGLOG("Got m_completedSegmentSize from file, value:%llu, %s", m_backupCheckpointInfo.m_completedSegmentSize,
        m_taskInfo.c_str());
    if (m_backupCheckpointInfo.m_completedSegmentSize > m_completedSegmentSize) {
        m_completedSegmentSize = m_backupCheckpointInfo.m_completedSegmentSize;
        INFOLOG("Got checkpoint, new m_completedSegmentSize:%llu, %s", m_completedSegmentSize, m_taskInfo.c_str());
    }

    m_curSegmentIndex = m_backupCheckpointInfo.m_completedSegmentSize / m_curSegmentSize;
    m_completedDataSize = m_backupCheckpointInfo.m_completedDataSize;
    return SUCCESS;
}

std::string BackupJob::GetSnapshotsLogDetails(const std::vector<VolSnapInfo>& volSnapshots)
{
    std::string msg;
    for (const auto &volSnap : volSnapshots) {
        msg += ("[vol id: " + volSnap.m_volUuid +
            ", snapshot name: " + volSnap.m_snapshotName + "]");
    }
    return std::move(msg);
}

void BackupJob::TryDeleteSnapshots()
{
    std::vector<SnapshotInfo> reSnapshotInfos;
    std::vector<SnapToBeDeleted>::iterator it = m_snapListToBeDeleted.m_snapList.begin();
    while (it != m_snapListToBeDeleted.m_snapList.end()) {
        INFOLOG("Deleting snapshot, VM: %s, snapshots: %s, %s.", it->m_snapshotInfo.m_vmName.c_str(),
            GetSnapshotsLogDetails(it->m_snapshotInfo.m_volSnapList).c_str(), m_taskInfo.c_str());
        it->m_tryDeleteCount++;
        if (DeleteVmSnapshot(it->m_snapshotInfo) == SUCCESS) {
            INFOLOG("Delete snapshot success. VM: %s, snapshot: %s, %s",
                it->m_snapshotInfo.m_vmName.c_str(), it->m_snapshotInfo.m_moRef.c_str(), m_taskInfo.c_str());
            it = m_snapListToBeDeleted.m_snapList.erase(it);
            continue;
        }
        if (it->m_tryDeleteCount > 0) {
            reSnapshotInfos.push_back(it->m_snapshotInfo);
            INFOLOG("Delete snaps fail after %lu times, Need save it into residual snapshots.", it->m_tryDeleteCount);
            it = m_snapListToBeDeleted.m_snapList.erase(it);
            continue;
        }
        WARNLOG("Delete snapshot failed after retry %lu times, %s", it->m_tryDeleteCount, m_taskInfo.c_str());
        it++;
    }

    SendAlarmAndRecordResidualSnapshots(reSnapshotInfos);
}

int BackupJob::SaveDeleteList()
{
    std::string snapToBeDeletedStr;
    if (!Module::JsonHelper::StructToJsonString(m_snapListToBeDeleted, snapToBeDeletedStr)) {
        ERRLOG("Convert snapshot list to json string failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    std::string snapListFile = m_metaRepoPath + VIRT_PLUGIN_SNAP_TOBEDELETED;
    if (Utils::SaveToFileWithRetry(m_metaRepoHandler, snapListFile, snapToBeDeletedStr) != SUCCESS) {
        ERRLOG("Save snapshot list failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    INFOLOG("Save snapshot list success, jobId: %s, %s", GetJobId().c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::SaveResidualSnapshots()
{
    std::string snapResidualListSaveInfoStr;
    if (!Module::JsonHelper::StructToJsonString(m_snapResidualListSaveInfo, snapResidualListSaveInfoStr)) {
        ERRLOG("Convert snapshot residual list to json string failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    std::string snapResidualFile = m_metaRepoPath + VIRT_PLUGIN_SNAP_RESIDUAL;
    if (Utils::SaveToFileWithRetry(m_metaRepoHandler, snapResidualFile, snapResidualListSaveInfoStr) != SUCCESS) {
        ERRLOG("Save snapshot residual list failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Save snapshot residual list success, jobId: %s, %s", GetJobId().c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

int BackupJob::SendAlarmAndRecordResidualSnapshots(const std::vector<SnapshotInfo>& reSnapshotInfos)
{
    // Get snapshots
    std::vector<VolSnapInfo> newResidualVolSnapshots;
    for (const SnapshotInfo& reSnap : reSnapshotInfos) {
        for (const VolSnapInfo& volSnap : reSnap.m_volSnapList) {
            if (IsSnapInToResidualSnapshots(volSnap)) {
                DBGLOG("Residual snapshot(%s) already send alarm.", volSnap.m_snapshotName.c_str());
                continue;
            }
            if (volSnap.m_deleted) {
                DBGLOG("Current snapshot(%s) already deleted.", volSnap.m_snapshotName.c_str());
                continue;
            }
            newResidualVolSnapshots.push_back(volSnap);
        }
    }
    if (newResidualVolSnapshots.empty()) {
        INFOLOG("No new residual snapshots (%s).", m_taskInfo.c_str());
        return SUCCESS;
    }
    // display residual snapshot
    ReportJobDetailsParam param = {
        "virtual_plugin_backup_job_delete_snapshot_failed_label",
        JobLogLevel::TASK_LOG_WARNING,
        SubJobStatus::RUNNING, 0, 0 };
    std::string residualSnapshotStr = GetSnapshotsLogDetails(newResidualVolSnapshots);
    ReportJobDetailsWithLabel(param, residualSnapshotStr);
    INFOLOG("Job(%s) report residule snap detail(%s).",
        GetJobId().c_str(), residualSnapshotStr.c_str());

    // send alarm
    ActionResult result;
    AppProtect::AlarmDetails alarm;
    alarm.alarmId = ALARM_CODE_FAILED_DELETE_SNAPSHOT;
    alarm.parameter = m_backupPara->protectEnv.type + "," + GetJobId();
    alarm.resourceId = m_backupPara->protectObject.id;
    JobService::SendAlarm(result, alarm);
    INFOLOG("Job(%s) send residule snap alrm(id: %s, param: %s). result: %d.",
        GetJobId().c_str(), alarm.alarmId.c_str(), alarm.parameter.c_str(), result.code);
    // record list
    SnapResidualInfo snapResidualInfo;
    snapResidualInfo.m_jobId = GetJobId();
    snapResidualInfo.m_alarmId = alarm.alarmId;
    snapResidualInfo.m_alarmParam = alarm.parameter;
    snapResidualInfo.m_volSnapshots = newResidualVolSnapshots;
    m_snapResidualListSaveInfo.m_snapResidualInfoList.push_back(snapResidualInfo);

    return SaveResidualSnapshots();
}

void BackupJob::TryClearResidualSnapshotsAndAlarm()
{
    // load snapshot residual list
    std::string snapResidualInfoFile = m_metaRepoPath + VIRT_PLUGIN_SNAP_RESIDUAL;
    if (!m_metaRepoHandler->Exists(snapResidualInfoFile)) {
        INFOLOG("No snap residual info file, %s", m_taskInfo.c_str());
        return;
    }
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, snapResidualInfoFile,
        m_snapResidualListSaveInfo) != SUCCESS) {
        ERRLOG("Load snap residual info file failed, %s", m_taskInfo.c_str());
        return;
    }
    INFOLOG("Load snap residual info file success, %s", m_taskInfo.c_str());

    // try to delete snap shot
    std::vector<SnapResidualInfo>::iterator it = m_snapResidualListSaveInfo.m_snapResidualInfoList.begin();
    while (it != m_snapResidualListSaveInfo.m_snapResidualInfoList.end()) {
        INFOLOG("Try to clear the residule snap alrm job(%s) sent. %s.", it->m_jobId.c_str(), m_taskInfo.c_str());
        it->m_tryDeleteCount++;
        SnapshotInfo snapInfo;
        snapInfo.m_vmName = m_vmInfo.m_name;
        snapInfo.m_deleted = false;
        snapInfo.m_volSnapList = it->m_volSnapshots;
        if (DeleteVmSnapshot(snapInfo) != SUCCESS) {
            WARNLOG("Snapshot delete failed, will retry in next round, retry times: %lu, %s",
                it->m_tryDeleteCount, m_taskInfo.c_str());
            ++it;
            continue;
        } else {
            // clear alarm
            ActionResult result;
            AppProtect::AlarmDetails alarm;
            alarm.alarmId = it->m_alarmId;
            alarm.parameter = it->m_alarmParam;
            JobService::ClearAlarm(result, alarm);
            INFOLOG("Clear the residule snap alrm(id: %s, param: %s) which job(%s) sent. result: %d.",
                alarm.alarmId.c_str(), alarm.parameter.c_str(), it->m_jobId.c_str(), result.code);

            it = m_snapResidualListSaveInfo.m_snapResidualInfoList.erase(it);
        }
    }

    SaveResidualSnapshots();
}

bool BackupJob::IsSnapInVolSnapshots(const VolSnapInfo& volSnap, const std::vector<VolSnapInfo>& volSnaps)
{
    for (const VolSnapInfo& tempSnap : volSnaps) {
        if (tempSnap.m_snapshotId == volSnap.m_snapshotId) {
            return true;
        }
    }
    return false;
}

bool BackupJob::IsSnapInToResidualSnapshots(const VolSnapInfo& volSnapInfo)
{
    for (const SnapResidualInfo& snapResidual : m_snapResidualListSaveInfo.m_snapResidualInfoList) {
        if (IsSnapInVolSnapshots(volSnapInfo, snapResidual.m_volSnapshots)) {
            return true;
        }
    }
    return false;
}

int BackupJob::DeleteVmSnapshot(SnapshotInfo& snap)
{
    /* delete snapshot */
    if (m_protectEngine->DeleteSnapshot(snap) != SUCCESS) {
            WARNLOG("Delete snapshot failed, vm: %s, %s", snap.m_vmMoRef.c_str(), m_taskInfo.c_str());
    }
    /* query snapshot to confirm and change vol snap status */
    if (m_protectEngine->QuerySnapshotExists(snap) != SUCCESS) {
        WARNLOG("Query snapshot exists fail, vm: %s, %s.", snap.m_vmMoRef.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    /* confirm whether snapshot deleted */
    if (!snap.m_deleted) {
        WARNLOG("Snapshot not deleted. vm: %s, %s.", snap.m_vmMoRef.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    return SUCCESS;
}

/* Save current snapshot info to previous */
int BackupJob::UpdateSnapshotFile()
{
    DBGLOG("In UpdateSnapshotFile(), %s", m_taskInfo.c_str());
    if (m_jobResult == AppProtect::JobResult::type::SUCCESS) {
        std::string preSnapshotFile = m_metaRepoPath + VIRT_PLUGIN_PRE_SNAPSHOT_INFO;
        if (!m_metaRepoHandler->Remove(preSnapshotFile)) {
            ERRLOG("Remove previous job context file failed, file:%s, %s", preSnapshotFile.c_str(),
                m_taskInfo.c_str());
            return FAILED;
        }

        std::string curSnapshotFile = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
        int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CopyFile, m_metaRepoHandler,
            curSnapshotFile, preSnapshotFile), true, "CopyFile");
        if (res != SUCCESS) {
            ERRLOG("Rename job context file failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
        DBGLOG("UpdateSnapshotFile success, from %s to %s, %s", curSnapshotFile.c_str(), preSnapshotFile.c_str(),
            m_taskInfo.c_str());
    } else {
        INFOLOG("Job execution failed in previous stage, skipping snapshot file updating in the post job, %s",
            m_taskInfo.c_str());
    }

    m_nextState = static_cast<int>(BackupJobSteps::STEP_POST_CLEANUP_CHECKPOINT);
    return SUCCESS;
}

int BackupJob::FormatBackupCopy(Copy &copy)
{
    std::string sha256FileState = "";
    GetSha256States(sha256FileState);
    std::string vmInfoFile = m_metaRepoPath + VIRT_PLUGIN_VM_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, vmInfoFile, m_vmInfo) != SUCCESS) {
        ERRLOG("Load previous metadata failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    CopyExtendInfo info;
    std::string strInfo;
    info.m_copyVerifyFile = sha256FileState;
    info.m_volList = m_vmInfo.m_volList;
    info.m_interfaceList = m_vmInfo.m_interfaceList;
    if (!Module::JsonHelper::StructToJsonString(info, strInfo)) {
        ERRLOG("Exit PostJob StructToJsonString Failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("FormatBackupCopy extendInfo: %s, %s", WIPE_SENSITIVE(strInfo).c_str(), m_taskInfo.c_str());
    copy.__set_extendInfo(strInfo);
    return SUCCESS;
}

void BackupJob::GetSha256States(std::string& strInfo)
{
    std::string shaStateFilePath = m_cacheRepoPath + Module::PATH_SEPARATOR + SHA256_FILE_FAILED_STATE;
    if (m_isCopyVerify && !m_cacheRepoHandler->Exists(shaStateFilePath)) {
        strInfo = "true";
        INFOLOG("Sha256Failed file not exist, %s", m_taskInfo.c_str());
    } else {
        strInfo = "false";
    }
}

int BackupJob::ReportCopy()
{
    DBGLOG("In ReportCopy(), %s", m_taskInfo.c_str());
    if (m_jobResult == AppProtect::JobResult::type::SUCCESS) {
        Copy image;
        if (FormatBackupCopy(image) != SUCCESS) {
            ERRLOG("Format backup copy failed, %s", m_taskInfo.c_str());
            return FAILED;
        }

        ActionResult returnValue;
        JobService::ReportCopyAdditionalInfo(returnValue, m_backupPara->jobId, image);
        if (returnValue.code != SUCCESS) {
            ERRLOG("Exit ReportCopyAdditionalInfo Failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    } else {
        INFOLOG("Job execution failed in previous stage, skipping report copy, %s", m_taskInfo.c_str());
    }

    m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_POSTHOOK);
    return SUCCESS;
}

int BackupJob::PrerequisitePreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::PRE_PREREQUISITE;
    para.jobExecRet = (m_jobResult == AppProtect::JobResult::type::SUCCESS) ? SUCCESS : FAILED;
    para.nextState = static_cast<int>(BackupJobSteps::STATE_PRE_CHECK_BEFORE_BACKUP);
    para.postHookState = static_cast<int>(BackupJobSteps::STEP_PRE_POSTHOOK);
    return ExecHook(para);
}

int BackupJob::PrerequisitePostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::PRE_PREREQUISITE;
    para.jobExecRet = (m_jobResult == AppProtect::JobResult::type::SUCCESS) ? SUCCESS : FAILED;
    para.nextState = static_cast<int>(BackupJobSteps::STATE_NONE);
    return ExecHook(para);
}

int BackupJob::GenerateSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.jobExecRet = (m_jobResult == AppProtect::JobResult::type::SUCCESS) ? SUCCESS : FAILED;
    para.nextState = static_cast<int>(BackupJobSteps::STEP_GENERATE_LOAD_PRE_SNAPSHOT_INFO);
    para.postHookState = static_cast<int>(BackupJobSteps::STEP_GENERATE_POSTHOOK);
    return ExecHook(para);
}

int BackupJob::GenerateSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.jobExecRet = (m_jobResult == AppProtect::JobResult::type::SUCCESS) ? SUCCESS : FAILED;
    para.nextState = static_cast<int>(BackupJobSteps::STATE_NONE);
    return ExecHook(para);
}

int BackupJob::ExecuteSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.jobExecRet = (m_jobResult == AppProtect::JobResult::type::SUCCESS) ? SUCCESS : FAILED;
    if (m_subJobInfo->jobName == REPORT_COPY_SUB_JOB) {
        para.nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_REPORT_COPY);
    } else {
        para.nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_GET_DIRTY_RANGES);
    }
    para.postHookState = static_cast<int>(BackupJobSteps::STEP_EXEC_POSTHOOK);
    return ExecHook(para);
}

int BackupJob::ExecuteSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.jobExecRet = (m_jobResult == AppProtect::JobResult::type::SUCCESS) ? SUCCESS : FAILED;
    para.nextState = static_cast<int>(BackupJobSteps::STATE_NONE);
    return ExecHook(para);
}

int BackupJob::PostJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::POST_JOB;
    para.jobExecRet = (m_jobResult == AppProtect::JobResult::type::SUCCESS) ? SUCCESS : FAILED;
    para.nextState = static_cast<int>(BackupJobSteps::STEP_POST_CLEANUP_SNAPSHOT);
    para.postHookState = static_cast<int>(BackupJobSteps::STEP_POST_POSTHOOK);
    int ret = ExecHook(para);
    ReportTaskLabel();
    return ret;
}

int BackupJob::PostJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::POST_JOB;
    para.jobExecRet = (m_jobResult == AppProtect::JobResult::type::SUCCESS) ? SUCCESS : FAILED;
    para.nextState = static_cast<int>(BackupJobSteps::STATE_NONE);
    return ExecHook(para);
}

bool BackupJob::CreateVerifyFile()
{
    std::string sha256Path = m_metaRepoPath + VIRT_PLUGIN_SHA_FILE_ROOT;
    int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_metaRepoHandler,
        sha256Path), true, "CreateDirectory");
    if (res != SUCCESS) {
        ERRLOG("Failed to Create sha256 directory. path：%s, %s", sha256Path.c_str(), m_taskInfo.c_str());
        return false;
    }

    std::string sha256FileName = sha256Path + m_backupSubJob.m_volInfo.m_uuid + "_sha256.info";
    if (m_metaRepoHandler->Open(sha256FileName, "a+") != SUCCESS) {
        ERRLOG("Failed to open sha256 file with a+, %s", m_taskInfo.c_str());
        return false;
    }

    if (m_metaRepoHandler->Close() != SUCCESS) {
        ERRLOG("Failed to close sha256 file , %s", m_taskInfo.c_str());
        return false;
    }
    return true;
}

bool BackupJob::CreateVerifyFailedFile()
{
    std::string sha256FailedPath = m_metaRepoPath + VIRT_PLUGIN_SHA_FILE_ROOT;
    int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_metaRepoHandler,
        sha256FailedPath), true, "CreateDirectory");
    if (res != SUCCESS) {
        ERRLOG("Failed to Create sha256 fail directory. path: %s, %s",
            sha256FailedPath.c_str(), m_taskInfo.c_str());
        return false;
    }
    std::string sha256FailedFileName = m_metaRepoPath + VIRT_PLUGIN_SHA_FAILED_INFO;
    if (m_metaRepoHandler->Open(sha256FailedFileName, "a+") != SUCCESS) {
        ERRLOG("Failed to open sha256 fail file with a+, %s", m_taskInfo.c_str());
        return false;
    }
    if (m_metaRepoHandler->Close() != SUCCESS) {
        ERRLOG("Failed to close sha256 file , %s", m_taskInfo.c_str());
        return false;
    }
    return true;
}

int32_t BackupJob::InitSha256File()
{
    if (m_metaRepoPath.empty()) {
        ERRLOG("Failed to data repo path is empty, %s", m_taskInfo.c_str());
        return FAILED;
    }

    std::string sha256FileName = m_metaRepoPath + VIRT_PLUGIN_SHA_FILE_ROOT +
                                 m_backupSubJob.m_volInfo.m_uuid + "_sha256.info";
    if (!m_metaRepoHandler->Exists(sha256FileName) && !CreateVerifyFile()) {
        ERRLOG("Failed to create verify file, %s", m_taskInfo.c_str());
        return FAILED;
    }

    if (m_metaRepoHandler->Open(sha256FileName, "r+") != SUCCESS) {
        ERRLOG("Failed to open sha256 file with r+, %s", m_taskInfo.c_str());
        return FAILED;
    }

    return SUCCESS;
}

int32_t BackupJob::CalculateSha256Operator(std::shared_ptr<BlockTask>& res,
    const int32_t tasksExecRet, bool& sha256Success)
{
    if (tasksExecRet == SUCCESS && m_isCopyVerify && sha256Success) {
        std::shared_ptr<uint8_t[]> shaBuff = nullptr;
        uint64_t startAddr = 0;
        bool calcSha256Success = true;
        std::shared_ptr<BackupIoTask> pBackupIO = std::dynamic_pointer_cast<BackupIoTask>(res);
        if (pBackupIO == nullptr || pBackupIO->GetSha256Data(shaBuff, startAddr, calcSha256Success) != SUCCESS ||
            !calcSha256Success) {
            ERRLOG("Get io task read volume data failed, %s", m_taskInfo.c_str());
            sha256Success = false;
            return FAILED;
        }
        if (SaveBlockSha256Value(shaBuff, startAddr) != SUCCESS) {
            sha256Success = false;
            ERRLOG("Save sha256 value data to buffer failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }
    return SUCCESS;
}

/**
 * @brief 写入当前分段的位图数据信息 1-当前4M位图有数据 0-当前4M位图无数据
 *
 * @param segBlockCount
 * @return int32_t
 */
int32_t BackupJob::SaveBlockDataBitMap(uint64_t confSegSize)
{
    if (m_ifSaveValidDataBitMap == NOT_SAVE_BLOCK_BIT_MAP) {
        return SUCCESS;
    }
    std::string blockBitMapFile = VolValidDataBitMapFile();
    // 修改需要为r+模式, a+ 只能在文件尾部追加
    if (m_metaRepoHandler->Open(blockBitMapFile, "r+") != SUCCESS) {
        ERRLOG("Failed to open %s, %s", blockBitMapFile.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    Utils::Defer _(nullptr, [&](...) {
        m_metaRepoHandler->Close();
    });
    int retrytimes = 3;
    uint64_t perSegBlockCount = (confSegSize + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
    uint64_t curSegBlockStartIndex = m_curSegmentIndex * perSegBlockCount;
    uint64_t curSegBlockCount = (m_curSegmentSize + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
    DBGLOG("blockBitMapFile is %s, CurSegBlockStartIndex is %ld, curSegBlockCount %ld, %s", blockBitMapFile.c_str(),
        curSegBlockStartIndex, curSegBlockCount, m_taskInfo.c_str());
    while (retrytimes) {
        if (m_metaRepoHandler->Seek(curSegBlockStartIndex) != SUCCESS) {
            ERRLOG("Seek dirty range index %llu failed, %s", curSegBlockStartIndex, m_taskInfo.c_str());
            Utils::SleepSeconds(RETRY_INTERVAL_SECOND);
            retrytimes--;
            continue;
        }
        if (m_metaRepoHandler->Write(m_blockDataBitMap, curSegBlockCount) == curSegBlockCount) {
            DBGLOG("Write data range count to file success, %s", m_taskInfo.c_str());
            return SUCCESS;
        }
        ERRLOG("Write dirty range bitmap failed, index(%llu) size(%llu), %s", curSegBlockStartIndex, curSegBlockCount,
            m_taskInfo.c_str());
        Utils::SleepSeconds(RETRY_INTERVAL_SECOND);
        retrytimes--;
    }
    return FAILED;
}

int32_t BackupJob::SaveBlockSha256Value(const std::shared_ptr<unsigned char[]>& shaBuf, const uint64_t& startAddr)
{
    if (shaBuf.get() == nullptr) {
        ERRLOG("Read volume shabuf data is empty, %s", m_taskInfo.c_str());
        return FAILED;
    }

    uint64_t shaDev = 0;
    if (CalculateSha256::CalculateSha256Deviation(startAddr, shaDev) != SUCCESS) {
        ERRLOG("Calculate shadev failed. startAddr:%llu, %s", startAddr, m_taskInfo.c_str());
        return FAILED;
    }

    BlockShaData blockShaData{shaDev, shaBuf};
    m_blockShaData.push_back(blockShaData);
    return SUCCESS;
}

bool BackupJob::WriteShaValueToFile()
{
    if (m_metaRepoHandler == nullptr) {
        ERRLOG("Sha256 handler is nullptr, %s", m_taskInfo.c_str());
        return false;
    }
    // 初始化sha256
    if (InitSha256File() != SUCCESS) {
        ERRLOG("Failed to init sha256 file, %s", m_taskInfo.c_str());
        return false;
    }
    Utils::Defer _(nullptr, [&](...) {
        m_metaRepoHandler->Close();
    });
    int32_t execCount = 0;
    for (auto blockShaData: m_blockShaData) {
        execCount = 0;
        while (execCount <= MAX_EXEC_COUNT) {
            if (execCount == MAX_EXEC_COUNT) {
                ERRLOG("Write sha256 to file failed, %s", m_taskInfo.c_str());
                return false;
            }
            if (m_metaRepoHandler->Seek(blockShaData.offset) != SUCCESS) {
                ERRLOG("Seek sha256 file <%llu> failed,will retry, %s", blockShaData.offset, m_taskInfo.c_str());
                Utils::SleepSeconds(RETRY_INTERVAL_SECOND);
                execCount++;
                continue;
            }
            if (m_metaRepoHandler->Write(blockShaData.sha256Value, SHA256_DIGEST_LENGTH) != SHA256_DIGEST_LENGTH) {
                ERRLOG("Write sha256 to file failed,will retry, %s", m_taskInfo.c_str());
                Utils::SleepSeconds(RETRY_INTERVAL_SECOND);
                execCount++;
                continue;
            }
            break;
        }
    }
    INFOLOG("Current m_blockShaData(size:%d) save success, %s", m_blockShaData.size(), m_taskInfo.c_str());
    // 保存当前段后需要清理掉m_blockShaData数据，以便于存放下个Segement的校验信息
    m_blockShaData.clear();
    return true;
}

void BackupJob::InitAndRegTracePoint()
{
#ifdef EBK_TRACE_POINT
    // 注册TP点
    (void)EbkTracePoint::GetInstance().RegTP("TP_1", "descriptor", TP_TYPE_CALLBACK, IntReturnFailHook);
    // EbkTracePoint::GetInstance().ActiveTPImpl("TP_1", 1, "UserParam", 0);
    (void)EbkTracePoint::GetInstance().RegTP("TP_2", "descriptor", TP_TYPE_CALLBACK, ResetReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_3", "descriptor", TP_TYPE_CALLBACK, BoolReturnFailHook);
#endif
}
VIRT_PLUGIN_NAMESPACE_END
