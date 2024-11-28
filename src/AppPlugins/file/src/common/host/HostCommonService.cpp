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
#ifdef WIN32
#include <filesystem>
#else
#include <boost/filesystem.hpp>
#endif

#include "log/BackupFailureRecorder.h"
#include "common/EnvVarManager.h"
#include "common/Thread.h"
#include "common/Path.h"
#include "HostCommonService.h"

using namespace std;
using namespace PluginUtils;

#ifdef WIN32
namespace fs = std::filesystem;
#else
namespace fs = boost::filesystem;
#endif

namespace FilePlugin {
namespace {
    const string MODULE = "HostCommonService";
    const std::string PLUGIN_CONFIG_KEY = "FilePluginConfig";
    constexpr uint16_t ERR_MAS_LEN = 256;
    constexpr uint64_t NUM1024 = 1024;
    constexpr uint64_t NUM1 = 1;
    constexpr uint8_t TIMES1 = 1;
    const uint64_t DEFAULT_MAX_FAILURE_RECORDS_NUM = 1000000; /* take about 200MB disk space */
    const std::string DEFAULT_FAILURE_DEFAULT_ROOT =
        R"(\DataBackup\ProtectClient\ProtectClient-E\log\Plugins\FilePlugin)";
};

std::mutex HostCommonService::m_pluginAtrributeJsonFileMutex {};

HostCommonService::HostCommonService()
{
    /* use logger root as the failure records output root */
    m_failureRecordRoot = Module::CLogger::GetInstance().GetLogRootPath();
    INFOLOG("using logger root path %s as default root path for failure recorder", m_failureRecordRoot.c_str());
    if (m_failureRecordRoot.empty()) {
        WARNLOG("got empty failure record root from log root, set to default");
        m_failureRecordRoot =  Module::EnvVarManager::GetInstance()->GetAgentHomePath() + DEFAULT_FAILURE_DEFAULT_ROOT;
    }
    m_maxFailureRecordsNum = DEFAULT_MAX_FAILURE_RECORDS_NUM;
}

void HostCommonService::SerializeBackupStats(const BackupStats& backupStats, BackupStatistic& backupStatistic)
{
    HCP_Log(DEBUG, MODULE) << "start to BackupStats class convert to BackupStatistic struct" << HCPENDLOG;
    backupStatistic.noOfDirToBackup    = backupStats.noOfDirToBackup;
    backupStatistic.noOfFilesToBackup  = backupStats.noOfFilesToBackup;
    backupStatistic.noOfBytesToBackup  = backupStats.noOfBytesToBackup;
    backupStatistic.noOfDirToDelete    = backupStats.noOfDirToDelete;
    backupStatistic.noOfFilesToDelete  = backupStats.noOfFilesToDelete;
    backupStatistic.noOfDirCopied      = backupStats.noOfDirCopied;
    backupStatistic.noOfFilesCopied    = backupStats.noOfFilesCopied;
    backupStatistic.noOfBytesCopied    = backupStats.noOfBytesCopied;
    backupStatistic.skipFileCnt        = backupStats.skipFileCnt;
    backupStatistic.skipDirCnt         = backupStats.skipDirCnt;
    backupStatistic.noOfFilesWriteSkip = backupStats.noOfFilesWriteSkip;
    backupStatistic.noOfDirDeleted     = backupStats.noOfDirDeleted;
    backupStatistic.noOfFilesDeleted   = backupStats.noOfFilesDeleted;
    backupStatistic.noOfDirFailed      = backupStats.noOfDirFailed;
    backupStatistic.noOfFilesFailed    = backupStats.noOfFilesFailed;
    backupStatistic.backupspeed        = backupStats.backupspeed;
    backupStatistic.startTime          = backupStats.startTime;
    backupStatistic.noOfSrcRetryCount  = backupStats.noOfSrcRetryCount;
    backupStatistic.noOfDstRetryCount  = backupStats.noOfDstRetryCount;
    backupStatistic.noOfFailureRecordsWritten  = backupStats.noOfFailureRecordsWritten;
}

BackupStatistic HostCommonService::GetIncBackupStats(const BackupStats& currStats, BackupStatistic prevStats)
{
    DBGLOG("Enter GetBackupStatsInc");
    BackupStatistic incStats;
    incStats.noOfDirToBackup   =    currStats.noOfDirToBackup    - prevStats.noOfDirToBackup;
    incStats.noOfFilesToBackup =    currStats.noOfFilesToBackup  - prevStats.noOfFilesToBackup;
    incStats.noOfBytesToBackup =    currStats.noOfBytesToBackup  - prevStats.noOfBytesToBackup;
    incStats.noOfDirToDelete   =    currStats.noOfDirToDelete    - prevStats.noOfDirToDelete;
    incStats.noOfFilesToDelete =    currStats.noOfFilesToDelete  - prevStats.noOfFilesToDelete;
    incStats.noOfDirCopied     =    currStats.noOfDirCopied      - prevStats.noOfDirCopied;
    incStats.noOfFilesCopied   =    currStats.noOfFilesCopied    - prevStats.noOfFilesCopied;
    incStats.noOfBytesCopied   =    currStats.noOfBytesCopied    - prevStats.noOfBytesCopied;
    incStats.noOfDirDeleted    =    currStats.noOfDirDeleted     - prevStats.noOfDirDeleted;
    incStats.noOfFilesDeleted  =    currStats.noOfFilesDeleted   - prevStats.noOfFilesDeleted;
    incStats.noOfDirFailed     =    currStats.noOfDirFailed      - prevStats.noOfDirFailed;
    incStats.noOfFilesFailed   =    currStats.noOfFilesFailed    - prevStats.noOfFilesFailed;
    incStats.noOfSrcRetryCount =    currStats.noOfSrcRetryCount  - prevStats.noOfSrcRetryCount;
    incStats.noOfDstRetryCount =    currStats.noOfDstRetryCount  - prevStats.noOfDstRetryCount;
    incStats.noOfFailureRecordsWritten = currStats.noOfFailureRecordsWritten - prevStats.noOfFailureRecordsWritten;
    DBGLOG("Exist GetBackupStatsInc");
    return incStats;
}

void HostCommonService::UpdateScannerStats(const HostScanStatistics& preStats)
{
    ScanStatistics stats = m_scanner->GetStatistics();
    m_scanStats.m_scanDuration       = stats.mScanDuration       + preStats.m_scanDuration;
    m_scanStats.m_totDirs            = stats.mTotDirs            + preStats.m_totDirs;
    m_scanStats.m_totFiles           = stats.mTotFiles           + preStats.m_totFiles;
    m_scanStats.m_totalSize          = stats.mTotalSize          + preStats.m_totalSize;
    m_scanStats.m_totDirsToBackup    = stats.mTotDirsToBackup    + preStats.m_totDirsToBackup;
    m_scanStats.m_totFilesToBackup   = stats.mTotFilesToBackup   + preStats.m_totFilesToBackup;
    m_scanStats.m_totFilesDeleted    = stats.mTotFilesDeleted    + preStats.m_totFilesDeleted;
    m_scanStats.m_totDirsDeleted     = stats.mTotDirsDeleted     + preStats.m_totDirsDeleted;
    m_scanStats.m_totalSizeToBackup  = stats.mTotalSizeToBackup  + preStats.m_totalSizeToBackup;
    m_scanStats.m_totalControlFiles  = stats.mTotalControlFiles  + preStats.m_totalControlFiles;
    m_scanStats.m_totFailedDirs      = stats.mTotFailedDirs      + preStats.m_totFailedDirs;
    m_scanStats.m_totFailedFiles     = stats.mTotFailedFiles     + preStats.m_totFailedFiles;
    m_scanStats.mEntriesMayFailedToArchive = stats.mEntriesMayFailedToArchive + preStats.mEntriesMayFailedToArchive;
    PrintScannerStats();
    ShareResourceManager::GetInstance().Wait(ShareResourceType::SCAN, m_jobId);
    ShareResourceManager::GetInstance().UpdateResource(ShareResourceType::SCAN, m_jobId, m_scanStats);
    ShareResourceManager::GetInstance().Signal(ShareResourceType::SCAN, m_jobId);
    return;
}

void HostCommonService::PrintScannerStats() const
{
    INFOLOG("Scan status:                \t%d", static_cast<int>(m_scanStatus));
    INFOLOG("Num of Dirs Scanned:        \t%llu", m_scanStats.m_totDirs);
    INFOLOG("Num of Files Scanned:       \t%llu", m_scanStats.m_totFiles);
    INFOLOG("Num of Dirs Failed to Scan: \t%llu", m_scanStats.m_totFailedDirs);
    INFOLOG("Num of Files Failed to Scan:\t%llu", m_scanStats.m_totFailedFiles);
    INFOLOG("Num of Dirs to Bkup:        \t%llu", m_scanStats.m_totDirsToBackup);
    INFOLOG("Num of Files to Bkup:       \t%llu", m_scanStats.m_totFilesToBackup);
    INFOLOG("Num of Dirs to Del:         \t%llu", m_scanStats.m_totDirsDeleted);
    INFOLOG("Num of Files to Del:        \t%llu", m_scanStats.m_totFilesDeleted);
    INFOLOG("Num of Control Files:       \t%llu", m_scanStats.m_totalControlFiles);
    INFOLOG("Total Size:                 \t%s", FormatCapacity(m_scanStats.m_totalSize).c_str());
    INFOLOG("Total Size to Bkup:         \t%s", FormatCapacity(m_scanStats.m_totalSizeToBackup).c_str());
    return;
}

void HostCommonService::PrintBackupStats(const BackupStatistic &backupStats, const std::string& jobId,
    bool completed) const
{
    bool canPrint = ShareResourceManager::GetInstance().CanPrintBackupStats(jobId + "_backup");
    if (!canPrint && !completed) {
        return;
    }
    string backupStartTimeStr = FormatTimeToStr(backupStats.startTime);
    uint8_t noOfRunningSubJobs = ShareResourceManager::GetInstance().QueryRunningSubTasks(jobId);
    INFOLOG("Backup jobId:             \t%s, subjobId: %s, noOfRunningSubJobs: %d, status:%d",
        jobId.c_str(), m_subJobId.c_str(), noOfRunningSubJobs, static_cast<int>(m_backupStatus));
    INFOLOG("Backup start time:        \t%s", backupStartTimeStr.c_str());
    INFOLOG("Backup Speed:             \t%llu", backupStats.backupspeed);
    INFOLOG("Backup noOfDirToBackup:   \t%llu", backupStats.noOfDirToBackup);
    INFOLOG("Backup noOfFilesToBackup: \t%llu", backupStats.noOfFilesToBackup);
    INFOLOG("Backup noOfBytesToBackup: \t%llu", backupStats.noOfBytesToBackup);
    INFOLOG("Backup noOfDirToDelete:   \t%llu", backupStats.noOfDirToDelete);
    INFOLOG("Backup noOfFilesToDelete: \t%llu", backupStats.noOfFilesToDelete);
    INFOLOG("Backup noOfDirCopied:     \t%llu", backupStats.noOfDirCopied);
    INFOLOG("Backup noOfFilesCopied:   \t%llu", backupStats.noOfFilesCopied);
    INFOLOG("Backup noOfBytesCopied:   \t%llu", backupStats.noOfBytesCopied);
    INFOLOG("Backup noOfDirDeleted:    \t%llu", backupStats.noOfDirDeleted);
    INFOLOG("Backup noOfFilesDeleted:  \t%llu", backupStats.noOfFilesDeleted);
    INFOLOG("Backup noOfDirFailed:     \t%llu", backupStats.noOfDirFailed);
    INFOLOG("Backup noOfFilesFailed:   \t%llu", backupStats.noOfFilesFailed);
    INFOLOG("Backup noOfSrcRetryCount: \t%llu", backupStats.noOfSrcRetryCount);
    INFOLOG("Backup noOfDstRetryCount: \t%llu", backupStats.noOfDstRetryCount);
    INFOLOG("Backup noOfFailureRecordsWritten: \t%llu", backupStats.noOfFailureRecordsWritten);
    return;
}

bool HostCommonService::CheckFilePathAndGetSrcFileList(string srcDir, string dstDir,
    vector<string> &srcFileList)
{
    if (!Module::CFile::DirExist(srcDir.c_str())) {
        HCP_Log(ERR, MODULE) << "SrcDir does not exist: " << srcDir << HCPENDLOG;
        return false;
    }

    if (!Module::CFile::DirExist(dstDir.c_str())) {
        HCP_Log(ERR, MODULE) << "DstDir does not exist: " << dstDir << HCPENDLOG;
        return false;
    }

    if (!GetFileListInDirectory(srcDir, srcFileList)) {
        HCP_Log(ERR, MODULE) << "Get filelist for dir failed: " << srcDir << HCPENDLOG;
        return false;
    }

    return true;
}

bool HostCommonService::IsValidCtrlFile(const std::string ctrlFileFullPath) const
{
    if (ctrlFileFullPath.find("txt.tmp") != string::npos) {
        return false;
    }

    if (ctrlFileFullPath.find(dir_sep + "hardlink_control_") != string::npos) {
        return true;
    }

    if (ctrlFileFullPath.find(dir_sep + "mtime_") != string::npos) {
        return true;
    }

    if (ctrlFileFullPath.find(dir_sep + "delete_control_") != string::npos) {
        return true;
    }

    if (ctrlFileFullPath.find(dir_sep + "control_") != string::npos) {
        return true;
    }

    return false;
}

bool HostCommonService::InitSubTask(SubJob &subJob, const string& ctrlFile, bool ignoFail,
    const string& prefix, const string& fsId)
{
    string subTaskName {};
    uint32_t subTaskPrio {};
    uint32_t subTaskType {};
    GetSubTaskInfoByFileName(ctrlFile, subTaskName, subTaskType, subTaskPrio);
    DBGLOG("InitSubTask, ctrlFile: %s, subTaskType: %d, prefix: %s, fsId: %s",
        ctrlFile.c_str(), subTaskType, prefix.c_str(), fsId.c_str());
    BackupSubJob backupSubJob {ctrlFile, subTaskType, prefix, fsId};
    string backupSubJobStr;
    if (!Module::JsonHelper::StructToJsonString(backupSubJob, backupSubJobStr)) {
        ERRLOG("Convert to json failed for file: %s", backupSubJob.controlFile.c_str());
        return false;
    }
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobName(subTaskName);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo(backupSubJobStr);
    subJob.__set_jobPriority(subTaskPrio);
    subJob.__set_ignoreFailed(ignoFail);
    return true;
}

bool HostCommonService::CreateSubTask(vector<SubJob> &subJobList, vector<string> &ctrlFileList)
{
    ActionResult ret;
    if (subJobList.empty() || ctrlFileList.empty()) {
        return true;
    }

    HCP_Log(INFO, MODULE) << "Enter CreateSubTasksFromCtrlFile, CreateSubTask" << HCPENDLOG;

    for (size_t i = 0; i < subJobList.size(); i++) {
        INFOLOG("Create subtask, jobId:%s jobName:%s, jobType:%d, jobPrio:%llu, jobInfo:%s",
            subJobList[i].jobId.c_str(), subJobList[i].jobName.c_str(), subJobList[i].jobType,
            subJobList[i].jobPriority, subJobList[i].jobInfo.c_str());
    }
    HCP_Log(INFO, MODULE) << "CreateSubTasksFromCtrlFile, Call AddJob start" << HCPENDLOG;
    uint64_t firstAddNewJobTime = PluginUtils::GetCurrentTimeInSeconds();
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        INFOLOG("Enter AddNewJob, retryTimes:%d", retryTimes);
        JobService::AddNewJob(ret, subJobList);
        if (ret.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        uint64_t currJobAddNewJobTime = PluginUtils::GetCurrentTimeInSeconds();
        if (currJobAddNewJobTime - firstAddNewJobTime >
            Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "ADD_NEW_SUBJOB_STUCK_TIME")) {
            ERRLOG("Add new job stuck for %d, failed",
                Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "ADD_NEW_SUBJOB_STUCK_TIME"));
            return false;
        }
        if (ret.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, jobId: %s, code: %d, bodyErr: %d", m_jobId.c_str(), ret.code, ret.bodyErr);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    if (ret.code != Module::SUCCESS) {
        ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
        return false;
    }

    for (const string& ctrlFile : ctrlFileList) {
        RemoveFile(ctrlFile);
    }

    subJobList.clear();
    ctrlFileList.clear();
    HCP_Log(INFO, MODULE) << "Exit CreateSubTasksFromCtrlFile, CreateSubTask" << HCPENDLOG;
    return true;
}

bool HostCommonService::CreateSubTask(const SubJob &subJob)
{
    ActionResult ret;
    HCP_Log(INFO, MODULE)
        << "Create subtask, jobId: " << subJob.jobId
        << ", jobName: " << subJob.jobName
        << ", jobType: " << subJob.jobType
        << ", jobPrio: " << subJob.jobPriority
        << ", jobInfo: " << subJob.jobInfo
        << ", time: " << PluginUtils::GetCurrentTimeInSeconds()
        << HCPENDLOG;
    vector<SubJob> subJobList;
    subJobList.push_back(subJob);
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, subJobList);
        if (ret.code == Module::SUCCESS) {
            return true;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        if (ret.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, jobId: %s, code: %d, bodyErr: %d", m_jobId.c_str(), ret.code, ret.bodyErr);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
    return false;
}

void HostCommonService::GetSubTaskInfoByFileName(const std::string &fileName, std::string &subTaskName,
    uint32_t &subTaskType, uint32_t &subTaskPrio)
{
    if (m_idGenerator == nullptr) {
        InitIdGenerator();
    }
    string uniqueId = to_string(m_idGenerator->GenerateId());
    if (fileName.find(dir_sep + "hardlink_control_") != string::npos) {
        subTaskName = "HostBackup_HardlinkCtrlFile_" + uniqueId;
        subTaskType = SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE;
        subTaskPrio = SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE_PRIO;
    } else if (fileName.find(dir_sep + "mtime_") != string::npos) {
        subTaskName = "HostBackup_DirMtimeFile_" + uniqueId;
        subTaskType = SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE;
        subTaskPrio = SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE_PRIO;
    } else if (fileName.find(dir_sep + "delete_control_") != string::npos) {
        subTaskName = "HostBackup_DelFile_" + uniqueId;
        subTaskType = SUBJOB_TYPE_DATACOPY_DELETE_PHASE;
        subTaskPrio = SUBJOB_TYPE_DATACOPY_DELETE_PHASE_PRIO;
    } else if (fileName.find(dir_sep + "control_") != string::npos) {
        subTaskName = "HostBackup_CtrlFile_" + uniqueId;
        subTaskType = SUBJOB_TYPE_DATACOPY_COPY_PHASE;
        subTaskPrio = SUBJOB_TYPE_DATACOPY_COPY_PHASE_PRIO;
    }
}

uint32_t HostCommonService::GetSubJobTypeByFileName(const std::string &fileName) const
{
    uint32_t subTaskType {};
    if (fileName.find(dir_sep + "hardlink_control_") != string::npos) {
        subTaskType = SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE;
    } else if (fileName.find(dir_sep + "mtime_") != string::npos) {
        subTaskType = SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE;
    } else if (fileName.find(dir_sep + "delete_control_") != string::npos) {
        subTaskType = SUBJOB_TYPE_DATACOPY_DELETE_PHASE;
    } else if (fileName.find(dir_sep + "control_") != string::npos) {
        subTaskType = SUBJOB_TYPE_DATACOPY_COPY_PHASE;
    }
    return subTaskType;
}

void HostCommonService::PrintSubJobInfo(shared_ptr<SubJob> &m_subJobInfo)
{
    HCP_Log(DEBUG, MODULE) << "m_subJobInfo->jobId       : " << m_subJobInfo->jobId << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << "m_subJobInfo->subJobId    : " << m_subJobInfo->subJobId << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << "m_subJobInfo->jobType     : " << m_subJobInfo->jobType << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << "m_subJobInfo->jobName     : " << m_subJobInfo->jobName << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << "m_subJobInfo->jobPriority : " << m_subJobInfo->jobPriority << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << "m_subJobInfo->policy      : " << m_subJobInfo->policy << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << "m_subJobInfo->jobInfo     : " << m_subJobInfo->jobInfo << HCPENDLOG;
    return;
}

void HostCommonService::SetJobCtrlPhase(const std::string& jobCtrlPhase)
{
    m_jobCtrlPhase = jobCtrlPhase;
    return;
}

void HostCommonService::SetMainJobId(const std::string& jobId)
{
    m_jobId = jobId;
}

void HostCommonService::SetSubJobId()
{
    m_subJobId = GetSubJobId();
}

bool HostCommonService::InitGenerateResource(const std::string& path)
{
    ShareResourceManager::GetInstance().SetResourcePath(path, m_jobId);
    bool ret = ShareResourceManager::GetInstance().InitResource(ShareResourceType::GENERAL, m_jobId, m_generalInfo);
    if (!ret) {
        ERRLOG("Init general shared resourace failed, jobId: %s", m_jobId.c_str());
    }
    return ret;
}

string HostCommonService::CheckMetaFileVersion(const std::string& metaPath) const
{
    vector<string> metaFileList;
    if (!GetFileListInDirectory(metaPath, metaFileList)) {
        ERRLOG("Failed to list meta files in directory.");
        return META_VERSION_V20;
    }
    for (auto file : metaFileList) {
        if (file.find("xmeta") != string::npos) {
            return META_VERSION_V20;
        }
    }
    return META_VERSION_V10;
}

int HostCommonService::ReadDirCountForMtimeStats(const std::string& control, const std::string& metaPath) const
{
    string metaVersion = CheckMetaFileVersion(metaPath);
    if (metaVersion == META_VERSION_V20) {
        unique_ptr<Module::MtimeCtrlParser> mtimeCtrlParser = make_unique<Module::MtimeCtrlParser>(control);
        if (mtimeCtrlParser == nullptr) {
            ERRLOG("Create parser failed.");
            return 0;
        }
        if (mtimeCtrlParser->Open(Module::CTRL_FILE_OPEN_MODE::READ) != Module::CTRL_FILE_RETCODE::SUCCESS) {
            ERRLOG("Open control failed.");
            return 0;
        }
        Module::MtimeCtrlParser::Header header {};
        if (mtimeCtrlParser->GetHeader(header) != Module::CTRL_FILE_RETCODE::SUCCESS) {
            ERRLOG("Read header failed.");
            return 0;
        }
        return header.stats.noOfDirs;
    } else if (metaVersion == META_VERSION_V10) {
        unique_ptr<BackupMtimeCtrl> backupMtimeCtrl = make_unique<BackupMtimeCtrl>(control);
        if (backupMtimeCtrl == nullptr) {
            ERRLOG("Create ctrl failed.");
            return 0;
        }
        if (backupMtimeCtrl->Open(NAS_CTRL_FILE_OPEN_MODE_READ) != NAS_CTRL_FILE_RET_SUCCESS) {
            ERRLOG("Open control failed.");
            return 0;
        }
        BackupMtimeCtrlHeader header {};
        if (backupMtimeCtrl->GetHeader(header) != NAS_CTRL_FILE_RET_SUCCESS) {
            ERRLOG("Read header failed.");
            return 0;
        }
        return header.stats.noOfDirs;
    } else {
        ERRLOG("Incorrect Meta Version.");
        return 0;
    }
}

bool HostCommonService::InitIdGenerator()
{
    m_idGenerator = make_shared<Module::Snowflake>();
    if (m_idGenerator == nullptr) {
        ERRLOG("Init idGenerator failed, iter is nullptr. jobid: %s", m_jobId.c_str());
        return false;
    }
    size_t machineId = Module::GetMachineId();
    m_idGenerator->SetMachine(machineId);
    return true;
}

void HostCommonService::MergeBackupFailureRecords()
{
    try {
        INFOLOG("start to merge failure records, output directory root: path %s, jobID: %s",
            m_failureRecordRoot.c_str(), m_jobId.c_str());
        std::string jobRecordRootPath = PathJoin(m_failureRecordRoot, m_jobId);
        if (!fs::exists(jobRecordRootPath) || !fs::is_directory(jobRecordRootPath)) {
            return;
        }
        if (!Module::BackupFailureRecorder::Merge(m_failureRecordRoot, m_jobId)) {
            ERRLOG("merge backup failure records failed");
            return;
        }
        WARNLOG("FailureRecord file exist!");
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING,
            "file_plugin_report_failure_record_file_label", jobRecordRootPath);
    } catch (const std::exception& e) {
        ERRLOG("merge backup failure records failed with exception: %s", e.what());
    }
}

bool HostCommonService::IsBackupStatusInprogress(SubJobStatus::type &jobStatus) const
{
    if (m_backupStatus == BackupPhaseStatus::COMPLETED) {
        INFOLOG("Monitor Backup - BackupPhaseStatus::COMPLETED");
        jobStatus = SubJobStatus::COMPLETED;
        return false;
    } else if (m_backupStatus == BackupPhaseStatus::FAILED ||
        m_backupStatus == BackupPhaseStatus::FAILED_NOACCESS ||
        m_backupStatus == BackupPhaseStatus::FAILED_NOSPACE ||
        m_backupStatus == BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE ||
        m_backupStatus == BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE) {
        ERRLOG("Monitor Backup - BackupPhaseStatus::FAILED");
        jobStatus = SubJobStatus::FAILED;
        return false;
    } else if (m_backupStatus == BackupPhaseStatus::ABORTED) {
        INFOLOG("Monitor Backup - BackupPhaseStatus::ABORTED");
        jobStatus = SubJobStatus::ABORTED;
        return false;
    } else if (m_backupStatus == BackupPhaseStatus::ABORT_INPROGRESS) {
        INFOLOG("Monitor Backup - BackupPhaseStatus::ABORT_INPROGRESS");
        jobStatus = SubJobStatus::ABORTING;
        return true;
    } else if (m_backupStatus == BackupPhaseStatus::INPROGRESS) {
        INFOLOG("Monitor Backup - BackupPhaseStatus::RUNNING");
        jobStatus = SubJobStatus::RUNNING;
        return true;
    }
    return true;
}

uint64_t HostCommonService::CalculateSizeInKB(uint64_t bytes) const
{
    uint64_t kiloBytes = bytes / NUM1024;
    if (kiloBytes == 0) { // consider less than 1KB as 1KB
        return NUM1;
    }
    return kiloBytes;
}

bool HostCommonService::SetNumOfChannels(const std::string& channelCntStr) const
{
    int channelCnt = Module::SafeStoi(channelCntStr);
    if (channelCnt <= 0) {
        WARNLOG("Invaid channelCnt: %s, don't set Cnt.", channelCntStr.c_str());
        return false;
    }
    std::string path = PluginUtils::PathJoin(Module::CPath::GetInstance().GetRootPath(), PLUGIN_ATT_JSON);
    DBGLOG("SetNumOfChannels, plugin_attribute_1.0.0.json path: %s", path.c_str());
    try {
        std::lock_guard<std::mutex> lk(m_pluginAtrributeJsonFileMutex);
        std::string pluginAttributeContent {};
        if (!PluginUtils::ReadFile(path, pluginAttributeContent)) {
            ERRLOG("Read plugin_attribute_1.0.0.json failed.");
            return false;
        }
        Json::Value jsonVal;
        if (!Module::JsonHelper::JsonStringToJsonValue(pluginAttributeContent, jsonVal)) {
            ERRLOG("JsonStringToJsonValue error, str: %s", pluginAttributeContent.c_str());
            return false;
        }
        if (jsonVal["application_sub_job_cnt_max"]["Fileset"].isInt() &&
            jsonVal["application_sub_job_cnt_max"]["Fileset"].asInt() == channelCnt) {
            INFOLOG("no need to write channel cnt to json file, cnt: %d", channelCnt);
            return true;
        }
        INFOLOG("Current channels of Fileset: %s, set to %d",
            jsonVal["application_sub_job_cnt_max"]["Fileset"].asString().c_str(), channelCnt);
        jsonVal["application_sub_job_cnt_max"]["Fileset"] = Json::Value(channelCnt);
        Json::StyledWriter sWriter;
        std::ofstream outFileStream(path, std::ios::out | std::ios::trunc);
        if (!outFileStream.is_open()) {
#ifdef WIN32
            uint32_t errcode = ::GetLastError();
            ERRLOG("Open file %s failed, errno[%lu]:%s.", path.c_str(), errcode, strerror(errcode));
#else
            ERRLOG("Open file %s failed, errno[%d]:%s.", path.c_str(), errno, strerror(errno));
#endif
            return false;
        }
        outFileStream << sWriter.write(jsonVal);
        outFileStream.close();
    } catch (const std::exception &ex) {
        ERRLOG("Standard C++ Exception: %s", ex.what());
        return false;
    }
    return true;
}

}
