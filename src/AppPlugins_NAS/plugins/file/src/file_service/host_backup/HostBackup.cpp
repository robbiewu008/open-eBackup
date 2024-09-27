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
#include "HostBackup.h"
#include <chrono>
#include <algorithm>
#include <thread>
#include <fstream>
#include <sstream>
#include "define/Types.h"
#include "ScanMgr.h"
#include "config_reader/ConfigIniReader.h"
#include "constant/ErrorCode.h"
#include "PluginUtilities.h"
#include "common/EnvVarManager.h"
#include "common/Thread.h"
#include "common/Path.h"
#include "host/OsIdentifier.h"
#include "host/ConcurrentTaskManager.h"
#include "BackupConstants.h"
#include "parser/Win32PathUtils.h"

#ifdef WIN32
#include "FileSystemUtil.h"
#include "snapshot_provider/VssSnapshotProvider.h"
#endif

#ifdef __linux__
#include "snapshot_provider/LvmSnapshotProvider.h"
#endif

#ifdef _AIX
#include "snapshot_provider/JfsSnapshotProvider.h"
#endif

#ifdef SOLARIS
#include "snapshot_provider/ZfsSnapshotProvider.h"
#endif

using namespace std;
using namespace PluginUtils;
using namespace Module;

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

namespace {
#ifdef WIN32
    const string HOST_BACKUP_SRC_ROOT_PATH = "";
#else
    const string HOST_BACKUP_SRC_ROOT_PATH = "/";
#endif
    const std::string PLUGIN_CONFIG_KEY = "FilePluginConfig";
    const std::string MODULE = "HostBackup";
    constexpr uint64_t BACKUP_INC_TO_FULL = 1577209901;
    constexpr uint32_t TWOHUNDREDOK = 200;
    constexpr uint32_t CTRL_FILE_CNT = 100;
    constexpr uint32_t NUMBER1 = 1;
    constexpr uint32_t NUMBER2 = 2;
    constexpr uint32_t NUMBER3 = 3;
    constexpr uint32_t NUMBER5 = 5;
    constexpr uint32_t NUMBER8 = 8;
    constexpr uint32_t NUMBER10 = 10;
    constexpr uint32_t NUMBER32 = 32;
    constexpr uint32_t REPORT_INTERVAL = 60;
    constexpr uint32_t MAXBUFFERCNT = 10;
    constexpr uint32_t MAXBUFFERSIZE = 10 * 1024;
    constexpr uint32_t MAXERRORFILES = 10;
    constexpr uint64_t NUMBER1024 = 1024;
    constexpr uint64_t NUMBER1200 = 1200;
    constexpr uint32_t SCANNER_REPORT_CIRCLE_TIME = 60; /* seconds */
    constexpr uint64_t POSIX_MAX_MEMORY = NUMBER1024 * NUMBER1024 * NUMBER1024; // 1 G
    constexpr uint32_t REPORT_RUNNING_TIMES = 6;
    constexpr uint32_t HOLD_GENERATE_SUB_TASK_SLEEP_TIME_SECOND = 5;
    const uint64_t DEFAULT_SCANQUEUE_SIZE = 10000;
    const uint32_t ENOENTERRNUM = 2;

// Snapshot mount path: ${SNAPSHOT_PARENT_PATH}/${jobId}/${originPath}

    const std::string EXCLUDE_FILESYSTEM_LIST_KEY = "ExcludeFileSystemList";
#ifdef WIN32
    // skip the subvolume dir, or it will be backup failed
    const string SYSTEMVOLUMEINFO                   = "System Volume Information";
    const string RECYCLEBIN                         = "$RECYCLE.BIN";
 
    const std::string SNAPSHOT_PARENT_PATH_KEY      = "Win32SnapshotParentPath";
    const std::string EXCLUDE_PATH_LIST_KEY         = "Win32ExcludePathList";
    const std::string SNAPSHOT_PARENT_PATH_DEFAULT  = WIN32_SNAPSHOT_PARENT_PATH_DEFAULT;
#endif

#ifdef _AIX
    const std::string SNAPSHOT_PARENT_PATH_KEY      = "AIXSnapshotParentPath";
    const std::string EXCLUDE_PATH_LIST_KEY         = "AIXExcludePathList";
    const std::string SNAPSHOT_PARENT_PATH_DEFAULT  = AIX_SNAPSHOT_PARENT_PATH_DEFAULT;
#endif

#ifdef SOLARIS
    const std::string SNAPSHOT_PARENT_PATH_KEY  = "SOLARISSnapshotParentPath";
    const std::string EXCLUDE_PATH_LIST_KEY     = "SOLARISExcludePathList";
    const std::string SNAPSHOT_PARENT_PATH_DEFAULT  = SOLARIS_SNAPSHOT_PARENT_PATH_DEFAULT;
#endif

#ifdef __linux__
    const std::string SNAPSHOT_PARENT_PATH_KEY      = "LinuxSnapshotParentPath";
    const std::string EXCLUDE_PATH_LIST_KEY         = "LinuxExcludePathList";
    const std::string SNAPSHOT_PARENT_PATH_DEFAULT  = LINUX_SNAPSHOT_PARENT_PATH_DEFAULT;
#endif

#ifdef WIN32
    const std::vector<std::string> SCAN_SKIP_DIRS = {
        ".", "..", ".snapshot", "~snapshot", "$recycle.bin", "$winreagent", "recycler"
    };
#elif defined(SOLARIS)
    const std::vector<std::string> SCAN_SKIP_DIRS = { ".", "..", ".snapshot", "~snapshot", ".zfs" };
#else
    const std::vector<std::string> SCAN_SKIP_DIRS = { ".", "..", ".snapshot", "~snapshot" };
#endif

    const string BACKUP_WORK_DIR = "backup-job";
    const string TRUE_STR = "true";
    const string FALSE_STR = "false";
    const string SEP = ", ";
    constexpr uint8_t LEN_SEP = 2;
    constexpr auto BACKUP_KEY_SUFFIX = "_backup_stats";
    const string META_VERSION_V10 = "1.0";
    const string META_VERSION_V20 = "2.0";
    constexpr uint64_t ERRNO_NO_SUCH_FILE_OR_DIR = 2;
    const string RESIDUAL_SNAPSHORTS_INFO_FILE = "residual_snapshots.info";
    const std::string ALARM_CODE_FAILED_DELETE_SNAPSHOT = "0x2064006F0001";
    const int MAX_RETRY_CNT = 3;
    const std::string VSS_MNT_ROOT = "C:\\vss_snapshots";
}

uint32_t HostBackup::m_numberOfSubTask = 0;

EXTER_ATTACK int HostBackup::CheckBackupJobType()
{
    if (!GetBackupJobInfo()) {
        return Module::FAILED;
    }
    SetJobCtrlPhase(JOB_CTRL_PHASE_CHECKBACKUPJOBTYPE);

    ENTER;
    int ret = CheckBackupJobTypeInner();
    EXIT;
    return ret;
}

HostBackup::HostBackup()
{
    INFOLOG("Construct HostBackup %llu", this_thread::get_id());
}

HostBackup::~HostBackup()
{
    INFOLOG("Destruct HostBackup %llu", this_thread::get_id());
}

int HostBackup::CheckBackupJobTypeInner()
{
    HCP_Log(INFO, MODULE) << "backupJobType: " << (IsFullBackup() ? "FULL" : "INC") << HCPENDLOG;

    /* For every full backupjob, UBC deletes old DTREE and creates a new DTREE. So always return success for FULL */
    if (IsFullBackup()) {
        return Module::SUCCESS;
    }

    if (!InitJobInfo()) {
        return Module::FAILED;
    }

    vector<string> subVolMetaDirName;
    if (!GetDirListInDirectory(m_scanMetaPath, subVolMetaDirName)) {
        ERRLOG("GetDirListInDirectory failed");
        return Module::FAILED;
    }
    bool ret = false;
    for (const string& name : subVolMetaDirName) {
        string prevMetaDir = PluginUtils::PathJoin(m_scanMetaPath, name + PREVIOUS);
        if (IsDirExist(prevMetaDir)) {
            INFOLOG("prevMetaDir for meta dir: %s exist", prevMetaDir.c_str());
            ret = true;
        }
    }
    if (!ret) {
        WARNLOG("There is none prev meta dir exist, Need change INC to FULL");
        return Module::FAILED;
    }
    if (NeedChangeIncToFull()) {
        WARNLOG("Need change INC to FULL");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

bool HostBackup::NeedChangeIncToFull()
{
    if (!GetPrevBackupCopyInfo()) {
        WARNLOG("Failed to get prev backup copy info, force change INC to FULL, jobId: %s", m_jobId.c_str());
        return true;
    }

    HostBackupCopy newBackupCopy {};
    newBackupCopy.m_backupFormat = m_dataLayoutExt.m_backupFormat;
    newBackupCopy.m_metadataBackupType = m_dataLayoutExt.m_metadataBackupType;
    newBackupCopy.m_backupFilter = m_fileset.m_filesetExt.m_filters;
    newBackupCopy.m_isConsistent = m_fileset.m_advParms.m_isConsistent;

    // MetaDataBackupType, BackupFormat, backupFilters comparision
    if ((m_prevBackupCopyInfo.m_metadataBackupType != newBackupCopy.m_metadataBackupType) ||
        (m_prevBackupCopyInfo.m_backupFormat != newBackupCopy.m_backupFormat) ||
        (m_prevBackupCopyInfo.m_backupFilter != newBackupCopy.m_backupFilter) ||
        (m_prevBackupCopyInfo.m_isConsistent != newBackupCopy.m_isConsistent)) {
        WARNLOG("NeedIncToFull, main-JobId: %s, "
                "prevBackupType: %s, newBackupType: %s, "
                "prevBackupFormat: %s, newBackupFormat: %s, "
                "prevIsConsistent: %s, newIsConsistent: %s, "
                "prevBackupFilter: %s, newBackupFilter: %s",
                m_jobId.c_str(),
                m_prevBackupCopyInfo.m_metadataBackupType.c_str(), newBackupCopy.m_metadataBackupType.c_str(),
                m_prevBackupCopyInfo.m_backupFormat.c_str(), newBackupCopy.m_backupFormat.c_str(),
                m_prevBackupCopyInfo.m_isConsistent.c_str(), newBackupCopy.m_isConsistent.c_str(),
                m_prevBackupCopyInfo.m_backupFilter.c_str(), newBackupCopy.m_backupFilter.c_str());
        return true;
    }
    return false;
}

EXTER_ATTACK int HostBackup::PrerequisiteJob()
{
    if (!GetBackupJobInfo()) {
        SetJobToFinish();
        return Module::FAILED;
    }
    SetJobCtrlPhase(JOB_CTRL_PHASE_PREJOB);

    ENTER;
    m_JobComplete = false;
    std::thread keepAliveThread = std::thread(&HostBackup::KeepJobAlive, this);
    int ret = PrerequisiteJobInner();
    m_JobComplete = true;
    EXIT;
    keepAliveThread.join();

    if (ret != Module::SUCCESS) {
        // TO-DO: 申请 error code
        string jobLabel = "nas_plugin_hetro_backup_prepare_fail_label";
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100),
            jobLabel, INITIAL_ERROR_CODE);
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
    }

    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int HostBackup::GenerateSubJob()
{
    if (!GetBackupJobInfo()) {
        SetJobToFinish();
        return Module::FAILED;
    }
    SetJobCtrlPhase(JOB_CTRL_PHASE_GENSUBJOB);

    ENTER;
    m_JobComplete = false;
    std::thread keepAliveThread = std::thread(&HostBackup::KeepJobAlive, this);
    int ret = GenerateSubJobInner();
    m_JobComplete = true;
    EXIT;
    keepAliveThread.join();

    if (ret != Module::SUCCESS) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0));
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
    }

    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int HostBackup::ExecuteSubJob()
{
    if (!GetBackupJobInfo()) {
        SetJobToFinish();
        return Module::FAILED;
    }
    SetJobCtrlPhase(JOB_CTRL_PHASE_EXECSUBJOB);
    if (m_subJobId.empty()) {
        return Module::FAILED;
    }

    ENTER;
    int ret = ExecuteSubJobInner();
    EXIT;

    if (ret != Module::SUCCESS) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0));
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
    }

    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int HostBackup::PostJob()
{
    if (!GetBackupJobInfo()) {
        SetJobToFinish();
        return Module::FAILED;
    }
    SetJobCtrlPhase(JOB_CTRL_PHASE_POSTJOB);

    ENTER;
    m_JobComplete = false;
    std::thread keepAliveThread = std::thread(&HostBackup::KeepJobAlive, this);
    int ret = PostJobInner();
    m_JobComplete = true;
    EXIT;
    keepAliveThread.join();

    if (ret != Module::SUCCESS) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0));
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
    }

    SetJobToFinish();
    return Module::SUCCESS;
}

int HostBackup::PrerequisiteJobInner()
{
    ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
    UpdateJobStartTime();
    LoadExcludePathList();
    if (!InitJobInfo()) {
        ERRLOG("InitJobInfo failed");
        return Module::FAILED;
    }
    if (m_fileset.m_protectedPaths.empty()) {
        WARNLOG("m_protectedPaths is empty");
        return Module::SUCCESS;
    }
    // Set the number of multi-channel
    SetNumOfChannels();
    // 清理残留的快照
    ClearResidualSnapshotsAndAlarm();
    if (!CreateSnapshot()) {
        ERRLOG("Create snapshots failed");
        return Module::FAILED;
    }
    if (!SetupCacheFsForBackupJob()) {
        ERRLOG("SetupChaceFsForBackupJob failed");
        return Module::FAILED;
    }
    GenerateCopyOsFlagRecord();
    INFOLOG("Pre requisite Job Inner success");
    return Module::SUCCESS;
}

bool HostBackup::InitSubBackupJobResources()
{
    if (m_subJobId.empty()) {
        ERRLOG("InitSubBackupJobResources failed, m_subJobId empty.");
        return false;
    }
    ShareResourceManager::GetInstance().SetResourcePath(m_statisticsPath, m_subJobId);
    bool ret = ShareResourceManager::GetInstance().InitResource(
        ShareResourceType::BACKUP, m_subJobId, m_subBackupStats);
    if (!ret) {
        ERRLOG("Init sub backup shared resourace failed");
    }
    return ret;
}

void HostBackup::DeleteSharedResources() const
{
    Remove(m_statisticsPath);
    Remove(m_scanStatusPath);
}

int HostBackup::GenerateSubJobInner()
{
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);
    LoadExcludePathList();
    if (!InitJobInfo()) {
        return Module::FAILED;
    }
    if (m_fileset.m_protectedPaths.empty()) {
        WARNLOG("m_protectedPaths is empty");
        return Module::SUCCESS;
    }
    if (!GetPrevBackupCopyInfo() || !InitIdGenerator()) {
        return Module::FAILED;
    }
    PrintJobInfo();
    if (!ShareResourceManager::GetInstance().InitResource(ShareResourceType::SCAN, m_jobId, m_scanStats)) {
        ERRLOG("Init scan resource failed");
        return Module::FAILED;
    }
    // lock scanner instances
    std::shared_ptr<void> defer(nullptr, [&](...) {
        ConcurrentTaskManager::GetConcurrentTaskManager().ReleaseLock(m_jobId);
    });
    HoldGenerateSubTaskAndKeepLive();
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);

    set<string> jobInfoSet;
    if (!CheckScanRedo(jobInfoSet)) {
        GetBackupSubVolumesPath();
        ReadSnapInfoFromFile(m_filesetPathsWithSnap, m_filesetSnapInfoFilePath);
        ReadSnapInfoFromFile(m_subVolPathWithSnap, m_subVolSnapInfoFilePath);
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_host_backup_scan_start_label");
        ShareResourceManager::GetInstance().CanReportStatToPM(m_jobId + "_scan"); // 初始化扫描上报时间
        HostScanStatistics preScanStats {};
        if (!ScanPrimalSourceVol(preScanStats, jobInfoSet)
            || !ScanPrimalSnapshotVol(preScanStats, jobInfoSet)
            || !ScanSubVolume(preScanStats, jobInfoSet)
            || !ScanFailedVolume(preScanStats, jobInfoSet)
            || !WriteScannSuccess(jobInfoSet)) {
            return Module::FAILED;
        }
    }
    if (!HandleScanCompletion(jobInfoSet)) {
        return Module::FAILED;
    }
    if (!CreateBackupSubJobTask(SUBJOB_TYPE_COPYMETA_PHASE) || !CreateBackupSubJobTask(SUBJOB_TYPE_TEARDOWN_PHASE) ||
        !CreateBackupSubJobTask(SUBJOB_TYPE_CHECK_SUBJOB_PHASE)) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

void HostBackup::HoldGenerateSubTaskAndKeepLive()
{
    INFOLOG("Enter HoldGenerateSubTaskAndKeepLive, jobId %s", m_jobId.c_str());
    uint32_t tick = 0;
    while (!ConcurrentTaskManager::GetConcurrentTaskManager().AccquireLock(m_jobId) && !IsAbortJob()) {
        Module::SleepFor(chrono::seconds(HOLD_GENERATE_SUB_TASK_SLEEP_TIME_SECOND));
        tick += HOLD_GENERATE_SUB_TASK_SLEEP_TIME_SECOND;
        if (tick > SCANNER_REPORT_CIRCLE_TIME) {
            ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
            tick = 0;
        }
    }
    INFOLOG("Exit HoldGenerateSubTaskAndKeepLive, jobId %s", m_jobId.c_str());
}

bool HostBackup::WriteScannSuccess(std::set<std::string>& jobInfoSet)
{
    SubJobInfoSet subJobInfoSet;
    subJobInfoSet.m_subJobInfoSet = jobInfoSet;
    if (!JsonFileTool::WriteToFile(subJobInfoSet, m_scanStatusPath)) {
        ERRLOG("Write Scanner status failed");
        return false;
    }
    return true;
}

bool HostBackup::CheckScanRedo(std::set<std::string>& jobInfoSet)
{
    INFOLOG("Enter check scanner redo");
    SubJobInfoSet subJobInfoSet;
    if (PluginUtils::IsFileExist(m_scanStatusPath) && JsonFileTool::ReadFromFile(m_scanStatusPath, subJobInfoSet)) {
        jobInfoSet = subJobInfoSet.m_subJobInfoSet;
        m_scanRedo = true;
        for (auto str : jobInfoSet) {
            INFOLOG("str:%s", str.c_str());
        }
        WARNLOG("Scanner restart");
        return true;
    }
    return false;
}

void HostBackup::ReportScannerCompleteStatus()
{
    HostScanStatistics scanStats {};
    bool ret = ShareResourceManager::GetInstance().QueryResource(ShareResourceType::SCAN, m_jobId, scanStats);
    if (!ret) {
        ERRLOG("Query total scan stats failed, jobId: %s", m_jobId.c_str());
        ShareResourceManager::GetInstance().Signal(ShareResourceType::SCAN, m_jobId);
        return;
    }
    uint64_t scanTotFiles = scanStats.m_totFiles > m_numberOfFailedFilesScaned ?
        scanStats.m_totFiles - m_numberOfFailedFilesScaned : scanStats.m_totFiles;
    INFOLOG("Enter ReportScannerCompleteStatus, m_totDirs: %llu, totalFiles: %llu,"
        "failedFiles: %llu, totalSize: %llu, totalFailedDirs: %llu, totalDirsToBackUp: %llu,"
        "totalFilesToBackup: %llu, totalSizeToBackup: %llu", scanStats.m_totDirs, scanStats.m_totFiles,
        m_numberOfFailedFilesScaned, scanStats.m_totalSize, scanStats.m_totFailedDirs, scanStats.m_totDirsToBackup,
        scanStats.m_totFilesToBackup, scanStats.m_totalSizeToBackup);
    if (scanStats.m_totFailedDirs != 0) {
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_host_backup_scan_completed_with_warn_label",
                       to_string(scanStats.m_totDirs),
                       to_string(scanTotFiles),
                       FormatCapacity(scanStats.m_totalSize),
                       to_string(scanStats.m_totFailedDirs),
                       to_string(scanStats.m_totDirsToBackup),
                       to_string(scanStats.m_totFilesToBackup),
                       FormatCapacity(scanStats.m_totalSizeToBackup));
    } else {
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_host_backup_scan_completed_label",
                       to_string(scanStats.m_totDirs),
                       to_string(scanTotFiles),
                       FormatCapacity(scanStats.m_totalSize),
                       to_string(scanStats.m_totDirsToBackup),
                       to_string(scanStats.m_totFilesToBackup),
                       FormatCapacity(scanStats.m_totalSizeToBackup));
    }
    m_numberOfFailedFilesScaned = 0;
    INFOLOG("Exit ReportScannerCompleteStatus");
}

int HostBackup::ExecuteSubJobInner()
{
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);
    if (!InitJobInfo()) {
        HCP_Log(ERR, MODULE) << "Init Job Info failed" << HCPENDLOG;
        return Module::FAILED;
    }

    PrintSubJobInfo(m_subJobInfo);
    BackupSubJob backupSubJob {};
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, backupSubJob)) {
        HCP_Log(ERR, MODULE) << "Get backup subjob info failed" << HCPENDLOG;
        return Module::FAILED;
    }

    m_subJobRequestId = GenerateHash(m_jobId + m_subJobId);
    HCP_Log(INFO, MODULE) << "mainJob ID: " << m_jobId << ", subJobID: " << m_subJobId <<
        ", subJobRequestId: 0x" << setw(NUMBER8) << setfill('0') << hex << (m_subJobRequestId & 0xFFFFFFFF) << dec <<
        HCPENDLOG;
    HCPTSP::getInstance().reset(m_subJobRequestId);

    ShareResourceManager::GetInstance().IncreaseRunningSubTasks(m_jobId);
    m_subTaskType = backupSubJob.subTaskType;
    int ret = Module::SUCCESS;
    if (m_subTaskType == SUBJOB_TYPE_TEARDOWN_PHASE) {
        ret = ExecuteTeardownSubJobInner();
    } else if (m_subTaskType == SUBJOB_TYPE_COPYMETA_PHASE) {
        ret = ExecuteCopyMetaSubJobInner();
    } else if (m_subTaskType == SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE && IsAggregate()) {
        ret = HandleAggregatedDirPhase(backupSubJob.controlFile);
    } else if (m_subTaskType == SUBJOB_TYPE_CHECK_SUBJOB_PHASE) {
        ret = ExecuteCheckSubJobInner();
    }else {
        ret = ExecuteBackupSubJobInner(backupSubJob);
    }
    ShareResourceManager::GetInstance().DecreaseRunningSubTasks(m_jobId);
    return ret;
}

/* latest -> pre */
int HostBackup::ExecuteTeardownSubJobInner()
{
    HCP_Log(INFO, MODULE) << "Enter ExecuteTeardownSubJobInner" << HCPENDLOG;
    /* Update the backup speed. The end time of mtime is calculated for the backup time.
       Teardown time is not calculated. */
    ReportBackupCompletionStatus();
    HostBackupCopy hostBackupCopy {};
    FillBackupCopyInfo(hostBackupCopy);
    if (!JsonFileTool::WriteToFile(hostBackupCopy, m_backupCopyInfoFilePath)) {
        ERRLOG("WriteBackupCpyToFile failed");
        return Module::FAILED;
    }
    if (!SaveScannerMeta()) {
        ERRLOG("SaveScannerMeta failed");
        return Module::FAILED;
    }
    if (!PostReportCopyAdditionalInfo()) {
        ERRLOG("PostReportCopyAdditionalInfo failed");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

bool HostBackup::ReportBackupCompletionStatus()
{
    BackupStatistic mainBackupStats {};
    if (!UpdateMainBackupStats(mainBackupStats)) {
        ERRLOG("UpdateMainBackupStats failed");
        return false;
    }
    // get scan stats, to see if there is failed files or failed dirs
    HostScanStatistics scanStats {};
    bool ret = ShareResourceManager::GetInstance().QueryResource(ShareResourceType::SCAN, m_jobId, scanStats);
    INFOLOG("get scan stats: %llu, %llu", scanStats.m_totDirsToBackup, scanStats.m_totFilesToBackup);
    if (ret && (scanStats.m_totDirsToBackup > mainBackupStats.noOfDirCopied ||
        scanStats.m_totFilesToBackup > mainBackupStats.noOfFilesCopied)) {
        WARNLOG("force set to failed dirs and files. total dir: %llu, %llu, toatl files, %llu, %llu",
            scanStats.m_totDirsToBackup, mainBackupStats.noOfDirCopied, scanStats.m_totFilesToBackup,
            mainBackupStats.noOfFilesCopied);
        mainBackupStats.noOfDirFailed = scanStats.m_totDirsToBackup - mainBackupStats.noOfDirCopied;
        mainBackupStats.noOfFilesFailed = scanStats.m_totFilesToBackup - mainBackupStats.noOfFilesCopied;
    }
    /* As we report this from teardown-subjob or postjob, set datasize to 0. SO that UBC do not consider this size
       for speed calc */
    m_dataSize =  CalculateSizeInKB(mainBackupStats.noOfBytesCopied);
    INFOLOG("noOfDirsFailed: %llu, noOfFilesFailed: %llu, m_jobId: %s",
        mainBackupStats.noOfDirFailed, mainBackupStats.noOfFilesFailed, m_jobId.c_str());
    if (mainBackupStats.noOfDirFailed != 0 || mainBackupStats.noOfFilesFailed != 0) {
        ReportJobDetailsWithLabelAndErrcode(
            make_tuple(JobLogLevel::TASK_LOG_WARNING, SubJobStatus::RUNNING, PROGRESS0),
            "file_plugin_host_backup_data_completed_with_warn_label", INITIAL_ERROR_CODE,
            to_string(mainBackupStats.noOfDirCopied),
            to_string(mainBackupStats.noOfFilesCopied),
            FormatCapacity(mainBackupStats.noOfBytesCopied),
            to_string(mainBackupStats.noOfDirFailed),
            to_string(mainBackupStats.noOfFilesFailed));
    } else {
        ReportJobDetailsWithLabelAndErrcode(
            make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0),
            "file_plugin_host_backup_data_completed_label", INITIAL_ERROR_CODE,
            to_string(mainBackupStats.noOfDirCopied),
            to_string(mainBackupStats.noOfFilesCopied),
            FormatCapacity(mainBackupStats.noOfBytesCopied));
    }
    std::string statisticInfo;
    if (!Module::JsonHelper::StructToJsonString(mainBackupStats, statisticInfo)) {
        return false;
    }
    std::string path = PluginUtils::PathJoin(m_metaFsPath, "lastCtrl", "statistic.json");
    if (!PluginUtils::WriteFile(path, statisticInfo)) {
        return false;
    }
    return true;
}

bool HostBackup::SaveScannerMeta() const
{
    vector<string> subVolMetaDirName;
    if (!GetDirListInDirectory(m_scanMetaPath, subVolMetaDirName)) {
        ERRLOG("GetDirListInDirectory failed");
        return Module::FAILED;
    }
    for (const string& name : subVolMetaDirName) {
        if (!IsDirExist(m_scanMetaPath + dir_sep + name + LATEST)) {
            Remove(m_scanMetaPath + dir_sep + name);
            continue;
        }
        string prevScanDir = PluginUtils::PathJoin(m_scanMetaPath, name + PREVIOUS);
        string currScanDir = PluginUtils::PathJoin(m_scanMetaPath, name + LATEST);
        if (IsDirExist(currScanDir) && !Rename(currScanDir, prevScanDir)) {
            ERRLOG("SaveScannerMeta failed. previous: %s, latest: %s", prevScanDir.c_str(), currScanDir.c_str());
            return false;
        }
    }
    return true;
}

bool HostBackup::IsAggregate() const
{
    return m_fileset.m_advParms.m_isAggregate == TRUE_STR;
}

bool HostBackup::GetAggCopyExtendInfo(string& jsonString)
{
    AggCopyExtendInfo aggCopyExtendInfo;
    if (IsAggregate()) {
        aggCopyExtendInfo.isAggregation = TRUE_STR;
        aggCopyExtendInfo.dataPathSuffix = m_backupJobPtr->copy.id;
        aggCopyExtendInfo.metaPathSuffix = m_backupJobPtr->copy.id;
        aggCopyExtendInfo.maxSizeToAggregate = m_fileset.m_advParms.m_maxSizeToAggregate;
        aggCopyExtendInfo.maxSizeAfterAggregate = m_fileset.m_advParms.m_maxSizeAfterAggregate;
    } else {
        aggCopyExtendInfo.isAggregation = FALSE_STR;
        aggCopyExtendInfo.dataPathSuffix = "";
        aggCopyExtendInfo.metaPathSuffix = "";
        aggCopyExtendInfo.maxSizeToAggregate = "0";
        aggCopyExtendInfo.maxSizeAfterAggregate = "0";
    }
    if (!Module::JsonHelper::StructToJsonString(aggCopyExtendInfo, jsonString)) {
        ERRLOG("Exit ReportCopyAdditionalInfo Failed,aggCopyExtendInfo json trans failed");
        return false;
    }
    return true;
}

void HostBackup::FillAggregateFileSet(vector<string>& aggregateFileSet)
{
    for (string path : m_fileset.m_protectedPaths) {
        if (IsDirExist(path)) {
            aggregateFileSet.push_back(path);
            continue;
        }
        aggregateFileSet.push_back(GetPathName(path));
    }
}

bool HostBackup::PostReportCopyAdditionalInfo()
{
    if (m_jobResult == AppProtect::JobResult::type::SUCCESS) {
        string dataRemotePath = IsAggregate() ? (m_dataFs.remotePath + "/" + m_backupJobPtr->copy.id) :
            m_dataFs.remotePath;
        string metaRemotePath = IsAggregate() ? (m_metaFs.remotePath + "/" + m_backupJobPtr->copy.id) :
            m_metaFs.remotePath;
        DBGLOG("PostReportCopyAdditionalInfo, Meta repository:metaFsId: %s, metaFsPath: %s",
            m_metaFs.id.c_str(), metaRemotePath.c_str());
        DBGLOG("PostReportCopyAdditionalInfo, Data repository:dataFsId: %s, dataFsPath: %s",
            m_dataFs.id.c_str(), dataRemotePath.c_str());
      
        Copy image;
        image.__set_id(m_backupJobPtr->copy.id); // 副本ID
        image.__set_formatType(m_backupJobPtr->copy.formatType);
        string extendInfo;
        if (!GetAggCopyExtendInfo(extendInfo)) {
            return false;
        }
        image.__set_extendInfo(extendInfo);
        // 构造数据仓和meta仓地址上报给UBC
        vector<StorageRepository> repositories;
        StorageRepository metaStorageRep;
        metaStorageRep.__set_id(m_metaFs.id); // 文件系统ID
        metaStorageRep.__set_repositoryType(RepositoryDataType::META_REPOSITORY);
        metaStorageRep.__set_isLocal(m_metaFs.isLocal);
        metaStorageRep.__set_remotePath(metaRemotePath);
        metaStorageRep.__set_remoteHost(m_metaFs.remoteHost);
        metaStorageRep.__set_protocol(m_metaFs.protocol);
        metaStorageRep.__set_extendInfo(m_metaFs.extendInfo);
        repositories.push_back(metaStorageRep);

        StorageRepository dataStorageRep;
        dataStorageRep.__set_id(m_dataFs.id);
        dataStorageRep.__set_repositoryType(RepositoryDataType::DATA_REPOSITORY);
        dataStorageRep.__set_isLocal(m_dataFs.isLocal);
        dataStorageRep.__set_remotePath(dataRemotePath);
        dataStorageRep.__set_remoteHost(m_dataFs.remoteHost);
        dataStorageRep.__set_protocol(m_dataFs.protocol);
        dataStorageRep.__set_extendInfo(m_dataFs.extendInfo);
        repositories.push_back(dataStorageRep);
        image.__set_repositories(repositories);
        AppProtect::ActionResult returnValue;
        JobService::ReportCopyAdditionalInfo(returnValue, m_backupJobPtr->jobId, image);
        if (returnValue.code != Module::SUCCESS) {
            ERRLOG("Exit ReportCopyAdditionalInfo Failed,returnCode: %d,jobId: %s",
                returnValue.code, m_backupJobPtr->jobId.c_str());
            return false;
        }
        INFOLOG("ReportCopyAdditionalInfo Success,jobId: %s", m_backupJobPtr->jobId.c_str());
    }
    return true;
}

int HostBackup::ExecuteCopyMetaSubJobInner()
{
    DBGLOG("Enter ExecuteCopyMetaSubJobInner");
    vector<string> subVolMetaDirName;
    if (!IsDirExist(m_scanMetaPath)) {
        ERRLOG("ExecuteCopyMetaSubJobInner failed, m_scanMetaPath: %s is not exist", m_scanMetaPath.c_str());
        return Module::FAILED;
    }
    if (!GetDirListInDirectory(m_scanMetaPath, subVolMetaDirName)) {
        ERRLOG("ExecuteCopyMetaSubJobInner failed, GetDirListInDirectory failed");
        return Module::FAILED;
    }
    HandleFailedRecord(subVolMetaDirName);
    m_isCopying = true;
    std::thread monitorCopyThread = std::thread(&HostBackup::ZipMetaThread, this, subVolMetaDirName);
    while (m_isCopying) {
        INFOLOG("Wait for copy finish!");
        SendJobReportForAliveness();
        Module::SleepFor(chrono::seconds(GENERATE_SUBTASK_MONITOR_DUR_IN_SEC));
    }
    monitorCopyThread.join();
    if (!m_isCopySuccessFlag) {
        ERRLOG("Copy Meta failed");
        return Module::FAILED;
    }
    m_isCopying = false;
    DBGLOG("Copy Meta sub job finish");
    return Module::SUCCESS;
}

void HostBackup::HandleFailedRecord(const std::vector<std::string>& subVolMetaDirName)
{
    INFOLOG("Enter HandleFailedRecord!, %u", subVolMetaDirName.size());
    for (const std::string& subVolDir: subVolMetaDirName) {
        std::string curPath = PathJoin(m_scanMetaPath, subVolDir, "latest");
        std::string tmpFile = PathJoin(curPath, "failed_file_record");
        // 获取所有的failed_file_record_subJobId
        std::vector<std::string> fileList;
        std::ofstream outputFile(tmpFile, std::ios::app);
        if (!outputFile) {
            ERRLOG("can not open target file: %s", tmpFile.c_str());
            outputFile.close();
            continue;
        }
        if (!GetFileListInDirectory(curPath, fileList)) {
            ERRLOG("get fileList failed!");
            outputFile.close();
            continue;
        }
        // merge all failed_file_record_subjobid to failed_file_record
        for (const std::string& file : fileList) {
            INFOLOG("deal with file: %s", file.c_str());
            if (file.find("failed_file_record_") == -1) {
                continue;
            }
            std::ifstream inputFile(file);
            INFOLOG("read file: %s", file.c_str());
            if (inputFile) {
                outputFile << inputFile.rdbuf();
                inputFile.close();
            }
            PluginUtils::RemoveFile(file);
        }
        outputFile.close();
        HandleUpdateMeta(tmpFile, subVolDir);
    }
    return;
}

void HostBackup::HandleUpdateMeta(const std::string& failedRecordPath, const std::string& subVolDir)
{
    uint32_t openedMetaIndex = UINT32_MAX;
    std::unordered_map<uint64_t, FileMeta> tmpRecordMap;
    std::unique_ptr<Module::MetaParser> metaParser;
    std::ifstream ifs(failedRecordPath);
    if (!ifs.is_open()) {
#ifndef WIN32
        WARNLOG("failedRecordPath is not open: %s, %d", failedRecordPath.c_str(), errno);
#else
        WARNLOG("failedRecordPath is not open: %s, %d", failedRecordPath.c_str(), ::GetLastError());
#endif
        return;
    }
    std::string line;
    std::string metaFilePath = PathJoin(m_scanMetaPath, subVolDir);
    while (std::getline(ifs, line)) {
        FailedRecordItem item;
        bool ret = ProcessFailedRecordLine(line, item);
        if (!ret) {
            WARNLOG("invalid line : %s", line.c_str());
            continue;
        }
        // this condition means done with current metafile, need to process anothor
        if (openedMetaIndex != item.metaIndex) {
            if (metaParser != nullptr) {
                metaParser->Close(CTRL_FILE_OPEN_MODE::READ);
            }
            DoRealUpdateMeta(tmpRecordMap, metaParser);
            std::string metaFileName = PathJoin(metaFilePath, "latest", "meta_file_" + to_string(item.metaIndex));
            INFOLOG("HandleUpdateMeta process metafile: %s", metaFileName.c_str());
            metaParser = std::make_unique<MetaParser>(metaFileName);
            CTRL_FILE_RETCODE openRet = metaParser->Open(CTRL_FILE_OPEN_MODE::READ);
            if (openRet != CTRL_FILE_RETCODE::SUCCESS) {
                WARNLOG("Open  metafile failed! %s", metaFileName.c_str());
                continue;
            }
            openedMetaIndex = item.metaIndex;
        }
        Module::FileMeta fileMeta;
        metaParser->ReadFileMeta(fileMeta, item.offset);
        // change meta
        fileMeta.m_err = item.errNum;
        tmpRecordMap.emplace(item.offset, fileMeta);
    }
    if (metaParser != nullptr) {
        metaParser->Close(CTRL_FILE_OPEN_MODE::READ);
    }
    DoRealUpdateMeta(tmpRecordMap, metaParser);
    ifs.close();
    return;
}

void HostBackup::DoRealUpdateMeta(std::unordered_map<uint64_t, FileMeta>& tmpRecordMap,
    const std::unique_ptr<Module::MetaParser>& metaParser)
{
    if (tmpRecordMap.empty()) {
        return;
    }
    metaParser->OpenForWrite();
    for (const auto& pair : tmpRecordMap) {
        DBGLOG("update meta in offset: %llu", pair.first);
        metaParser->UpdateFileMeta(pair.second, pair.first);
    }
    metaParser->CloseForWrite();
    tmpRecordMap.clear();
}

int HostBackup::ZipMetaThread(const vector<string>& subVolMetaDirName)
{
    for (const string& name : subVolMetaDirName) {
        DBGLOG("Begin Copy Meta file, m_scanMetaPath: %s, name: %s", m_scanMetaPath.c_str(), name.c_str());
        if (ZipSubMetaFileToMetaRepo(name) != Module::SUCCESS) {
            ERRLOG("ZipSubMetaFileToMetaRepo failed");
            m_isCopying = false;
            m_isCopySuccessFlag = false;
            return Module::FAILED;
        }
    }
    if (ZipFinalMeta() != Module::SUCCESS) {
        m_isCopying = false;
        m_isCopySuccessFlag = false;
        ERRLOG("ZipFinalMeta failed");
        return Module::FAILED;
    }
    m_isCopying = false;
    m_isCopySuccessFlag = true;
    return Module::SUCCESS;
}

int HostBackup::ZipSubMetaFileToMetaRepo(const string& metaDirName)
{
    string scanMetaFilePath = m_scanMetaPath + dir_sep + metaDirName + LATEST;
    if (!IsDirExist(scanMetaFilePath)) {
        WARNLOG("scanMetaFilePath not exist, path: %s", scanMetaFilePath.c_str());
        return Module::SUCCESS;
    }
    string metaFileZipPath = m_metaFsPath + dir_sep + METAFILE_PARENT_DIR + TMP_DIR + metaDirName;
    CreateDirectory(metaFileZipPath);
    string metaFileZipName = metaFileZipPath + dir_sep + METAFILE_ZIP_NAME;
    Remove(metaFileZipName);
#ifndef WIN32
    vector<string> output;
    vector<string> errput;
#ifdef _AIX
    string cmd = "tar cf - -C " + scanMetaFilePath + " . | gzip > " + metaFileZipName;
#elif defined(SOLARIS)
    string metaFileTarName = metaFileZipPath + dir_sep + METAFILE_TAR_NAME;
    string cmd = "tar -cf " + metaFileTarName + " -C " + scanMetaFilePath + " . && gzip " + metaFileTarName +
        " >/dev/null";
#else
    string cmd = "tar -zcf " + metaFileZipName + " -C " + scanMetaFilePath + " .";
#endif
    DBGLOG("run cmd: %s", cmd.c_str());
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errput);
    if (ret != 0) {
        ERRLOG("tar meta file failed, cmd: %s", cmd.c_str());
        for_each(output.begin(), output.end(), [&] (const string& v) { ERRLOG("output: %s", v.c_str());});
        for_each(errput.begin(), errput.end(), [&] (const string& v) { ERRLOG("errput: %s", v.c_str());});
        return Module::FAILED;
    }
#else
    string win7z = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    string cmd = win7z + " a -tzip " + ReverseSlash(metaFileZipName) + " " + ReverseSlash(scanMetaFilePath) + "\\*";
    INFOLOG("compress cmd : %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        return Module::FAILED;
    }
#endif
    return Module::SUCCESS;
}

int HostBackup::ZipFinalMeta() const
{
    string metaFileZipPath = m_metaFsPath + dir_sep + METAFILE_PARENT_DIR + TMP_DIR;
    CreateDirectory(metaFileZipPath);
    string metaFileZipName = PluginUtils::PathJoin(m_metaFsPath, METAFILE_PARENT_DIR, METAFILE_ZIP_NAME);
    Remove(metaFileZipName);
#ifndef WIN32
    vector<string> output;
    vector<string> errput;
#ifdef _AIX
    string cmd = "tar cf - -C " + metaFileZipPath + " . | gzip > " + metaFileZipName +
        " && rm -rf " + metaFileZipPath;
#elif defined(SOLARIS)
    string metaFileTarName = PluginUtils::PathJoin(m_metaFsPath, METAFILE_PARENT_DIR, METAFILE_TAR_NAME);
    string cmd = "tar -cf " + metaFileTarName + " -C " + metaFileZipPath + " . && gzip " + metaFileTarName +
        " && rm -rf " + metaFileZipPath + " >/dev/null";
#else
    string cmd = "tar -zcf " + metaFileZipName + " -C " + metaFileZipPath + " . && rm -rf " + metaFileZipPath;
#endif
    DBGLOG("run cmd: %s", cmd.c_str());
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errput);
    if (ret != 0) {
        ERRLOG("tar meta file failed, cmd: %s", cmd.c_str());
        for_each(output.begin(), output.end(), [&] (const string& v) { ERRLOG("output: %s", v.c_str());});
        for_each(errput.begin(), errput.end(), [&] (const string& v) { ERRLOG("errput: %s", v.c_str());});
        return Module::FAILED;
    }
    DBGLOG("tar final meta success.");
#else
    string win7z = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    string cmd = win7z + " a -tzip " + ReverseSlash(metaFileZipName) + " " + ReverseSlash(metaFileZipPath) + "\\*";
    INFOLOG("compress cmd : %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        return Module::FAILED;
    }
#endif

    return Module::SUCCESS;
}

int HostBackup::ExecuteBackupSubJobInner(BackupSubJob backupSubJob)
{
    INFOLOG("Enter ExecuteBackupSubJobInner, jobId: %s, subJobId: %s", m_jobId.c_str(), m_subJobId.c_str());
    /* update the control file path */
    backupSubJob.controlFile = m_cacheFsPath + backupSubJob.controlFile;
    MONITOR_BACKUP_RES_TYPE monitorRet;
    int retryCnt = 0;
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    // 另外开一个线程去更新子任务状态和主任务状态
    m_isBackupInProgress = true;
    std::thread updateTaskInfoThread = std::thread(&HostBackup::UpdateTaskInfo, this);
    do {
        m_backup.reset();
        bool ret = DoRealBackup(backupSubJob, jobStatus, monitorRet, retryCnt);
        if (!ret) {
            updateTaskInfoThread.join();
            return Module::FAILED;
        }
        retryCnt++;
        std::this_thread::sleep_for(1s);
    } while (monitorRet == MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_NEEDRETRY && ++retryCnt < MAX_RETRY_CNT);

    m_isBackupInProgress = false;
    int ret = ReportJobProgress(jobStatus);
    m_backup.reset();  // ReportJobProgress失败场景下会调用m_backup
    if (updateTaskInfoThread.joinable()) {
        updateTaskInfoThread.join();
    }
    if (retryCnt >= MAX_RETRY_CNT && monitorRet == MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_NEEDRETRY) {
        // seems this sub job is stuck for some reason , copy this control file to meta repo for further check
        INFOLOG("subjob is stuck, %s, copy controlFile: %s", m_subJobId.c_str(), backupSubJob.controlFile.c_str());
        CopyFile(backupSubJob.controlFile, m_metaFsPath);
    }
    INFOLOG("Exit ExecuteBackupSubJobInner, jobId: %s, subJobId: %s", m_jobId.c_str(), m_subJobId.c_str());
    return ret;
}

bool HostBackup::DoRealBackup(const BackupSubJob& backupSubJob, SubJobStatus::type& jobStatus,
    HostCommonService::MONITOR_BACKUP_RES_TYPE& ret, int retryCnt)
{
    if (!StartBackup(backupSubJob)) {
        ERRLOG("StartBackup failed");
        ShareResourceManager::GetInstance().DeleteResource(ShareResourceType::BACKUP, m_subJobId);
        m_isBackupInProgress = false;
        return false;
    }
    // StartBackup检查续作通过判断是否存在子任务share resources的文件来判断，所以需要在start后创建subjob的share resources
    if (retryCnt == 0) {
        if (!InitSubBackupJobResources()) {
            ERRLOG("InitSubBackupJobResources failed");
            m_isBackupInProgress = false;
            return false;
        }
    }
    ret = MonitorBackup(jobStatus);
    std::unordered_set<FailedRecordItem, FailedRecordItemHash> failedRecordItem = m_backup->GetFailedDetails();
    if (!failedRecordItem.empty()) {
        std::string tmpFile = PathJoin(m_scanMetaPath, m_volumeName, "latest", "failed_file_record_" + m_subJobId);
        std::ofstream outfile(tmpFile);
        if (outfile.is_open()) {
            for (const auto& item : failedRecordItem) {
                outfile << item.metaIndex << "," << item.errNum << "," << item.offset << ","
                    << item.filePath << std::endl;
            }
            outfile.close();
        } else {
#ifdef WIN32
            WARNLOG("failed_file_record not open!, %s, %d", tmpFile.c_str(), ::GetLastError());
#else
            WARNLOG("failed_file_record not open!, %s, %d", tmpFile.c_str(), errno);
#endif
        }
    }
    if (m_backup != nullptr) {
        m_backup->Destroy();
    }
    return true;
}

int HostBackup::ExecuteCheckSubJobInner()
{
    INFOLOG("Enter Check SubJob info");
    BackupStatistic mainStats;
    UpdateMainBackupStats(mainStats);
    if (mainStats.noOfDirFailed == 0 && mainStats.noOfFilesFailed == 0) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS0));
    } else {
        WARNLOG("some of files or dirs failed, set main job to partial success");
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::FAILED, PROGRESS0));
    }
    return Module::SUCCESS;
}

bool HostBackup::StartBackup(BackupSubJob backupSubJob)
{
    BackupParams backupParams {};
    FillBackupConfig(backupParams, backupSubJob);
    DBGLOG("Start Backup, backup phase: %d", static_cast<int>(backupParams.phase));
    m_backup = FS_Backup::BackupMgr::CreateBackupInst(backupParams);
    if (m_backup == nullptr) {
        ERRLOG("Create backup instance failed");
        return false;
    }
#ifdef WIN32
    if (BackupRetCode::SUCCESS != m_backup->Enqueue(PluginUtils::ReverseSlash(backupSubJob.controlFile))) {
#else
    if (BackupRetCode::SUCCESS != m_backup->Enqueue(backupSubJob.controlFile)) {
#endif
        ERRLOG("enqueue backup instance failed");
        return false;
    }
    if (BackupRetCode::SUCCESS != m_backup->Start()) {
        ERRLOG("Start backup instance failed");
        return false;
    }
    return true;
}

HostCommonService::MONITOR_BACKUP_RES_TYPE HostBackup::MonitorBackup(SubJobStatus::type &jobStatus)
{
    HCP_Log(INFO, MODULE) << "Enter Monitor Backup" << HCPENDLOG;
    BackupStats tmpStats;
    time_t statLastUpdateTime = PluginUtils::GetCurrentTimeInSeconds();
    time_t lastReportTime = statLastUpdateTime;
    do {
        m_backupStatus = m_backup->GetStatus();
        HCP_Log(INFO, MODULE) << "m_backupStatus:" << static_cast<int>(m_backupStatus) << HCPENDLOG;
        tmpStats = m_backup->GetStats();
        /* 若文件已经全被write，单仍然未完成说明聚合的sql任务还未结束，不用重新备份 */
        if (m_backupStats != tmpStats) {
            statLastUpdateTime = PluginUtils::GetCurrentTimeInSeconds();
            INFOLOG("backup statistics last update time: %ld", statLastUpdateTime);
            m_backupStats = tmpStats;
        } else if ((m_backupStatus == BackupPhaseStatus::INPROGRESS) &&
            (tmpStats.noOfFilesCopied + tmpStats.noOfFilesFailed != tmpStats.noOfFilesToBackup) &&
            (PluginUtils::GetCurrentTimeInSeconds() - statLastUpdateTime >
            Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "BACKUP_STUCK_TIME"))) {
            UpdateSubBackupStats(true);
            HandleMonitorStuck(jobStatus);
            return MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_NEEDRETRY;
        }
        if (!IsBackupStatusInprogress(jobStatus)) {
            HCP_Log(DEBUG, MODULE) << "m_backupStatus: " << static_cast<int>(m_backupStatus) << HCPENDLOG;
            break;
        }
        if (IsAbortJob()) {
            HCP_Log(INFO, MODULE) << "Backup - Abort is invocked for taskid: " << m_jobId
                << ", subtaskid: " << m_subJobId << HCPENDLOG;
            if (BackupRetCode::SUCCESS != m_backup->Abort()) {
                HCP_Log(ERR, MODULE) << "backup Abort is failed" << HCPENDLOG;
            }
        }
        time_t currentTime = PluginUtils::GetCurrentTimeInSeconds();
        // 一分钟上报一次
        if ((currentTime - lastReportTime) > REPORT_INTERVAL) {
            ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
            lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
        }
        Module::SleepFor(std::chrono::seconds(NUMBER3));
    } while (true);
    UpdateSubBackupStats(false);
    HCP_Log(INFO, MODULE) << "Exit Monitor Backup" << HCPENDLOG;
    return MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_SUCCESS;
}

void HostBackup::HandleMonitorStuck(SubJobStatus::type &jobStatus)
{
    WARNLOG("backup statistic has not been update for %ds",
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "BACKUP_STUCK_TIME"));
    if (BackupRetCode::SUCCESS != m_backup->Abort()) {
        HCP_Log(ERR, MODULE) << "backup Abort is failed" << HCPENDLOG;
    }
    jobStatus = SubJobStatus::COMPLETED;
}

void HostBackup::FillFailureRecorderParams(CommonParams &commonParams)
{
    commonParams.jobId = m_jobId;
    commonParams.subJobId = m_subJobId;
    commonParams.reqID = m_subJobRequestId;
    commonParams.failureRecordRootPath = m_failureRecordRoot;
    INFOLOG("fill failure recorder params, jobID %s, subJobId %s, failureRecordRootPath %s",
        m_jobId.c_str(), m_subJobId.c_str(), m_failureRecordRoot.c_str());
    /* recorder quota ran out */
    if (m_subBackupStats.noOfFailureRecordsWritten >= m_maxFailureRecordsNum) {
        commonParams.maxFailureRecordsNum = 0;
    }
}

#ifdef WIN32
void HostBackup::FillBackupConfigWin32(BackupParams &backupParams, BackupSubJob &backupSubJob)
{
    backupParams.srcEngine = BackupIOEngine::WIN32_IO;
    backupParams.dstEngine = BackupIOEngine::WIN32_IO;
    string controlFileParentDirName = GetParentDirName(PluginUtils::ReverseSlash(backupSubJob.controlFile));
    INFOLOG("control file parent dir: %s", controlFileParentDirName.c_str());
    m_volumeName = controlFileParentDirName;
    backupParams.scanAdvParams.metaFilePath = PathJoin(m_scanMetaPath, controlFileParentDirName + LATEST);
    backupSubJob.prefix = backupSubJob.prefix.substr(0, backupSubJob.prefix.length() - NUMBER2);
    string srcRootPath = backupSubJob.prefix.empty() ? HOST_BACKUP_SRC_ROOT_PATH : backupSubJob.prefix;
    DBGLOG("srcRootPath:%s", srcRootPath.c_str());
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>();
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>();
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath = srcRootPath;
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = m_dataFsPath;
}
#else
void HostBackup::FillBackupConfigPosix(BackupParams &backupParams, BackupSubJob &backupSubJob)
{
    backupParams.srcEngine = BackupIOEngine::POSIX;
    backupParams.dstEngine = BackupIOEngine::POSIX;
    string controlFileParentDirName = GetParentDirName(backupSubJob.controlFile);
    INFOLOG("control file parent dir: %s", controlFileParentDirName.c_str());
    m_volumeName = controlFileParentDirName;
    backupParams.scanAdvParams.metaFilePath = PathJoin(m_scanMetaPath, controlFileParentDirName + LATEST);
    string prefix = backupSubJob.prefix;
#ifdef SOLARIS
    std::string srcRootPath = prefix.empty() ? HOST_BACKUP_SRC_ROOT_PATH :
        PluginUtils::PathJoin(prefix, ZFS_SNAPSHOT_DIR, m_jobId);
    if (!prefix.empty()) {
        backupParams.commonParams.trimReaderPrefix = prefix;
        INFOLOG("prefix: %s, srcRootPath: %s, jobId: %s, sub jobId: %s",
            prefix.c_str(), srcRootPath.c_str(), m_jobId.c_str(), m_subJobId.c_str());
    }
#else
    std::string srcRootPath = prefix.empty() ? HOST_BACKUP_SRC_ROOT_PATH : prefix;
#endif
    DBGLOG("srcRootPath:%s", srcRootPath.c_str());
    HostBackupAdvanceParams posixBackupAdvanceParams {};
    posixBackupAdvanceParams.dataPath = srcRootPath;
    posixBackupAdvanceParams.threadNum = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixReaderThreadNum");
    posixBackupAdvanceParams.maxMemory = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMaxMemory");
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    posixBackupAdvanceParams.dataPath = m_dataFsPath;
    posixBackupAdvanceParams.threadNum = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixWriterThreadNum");
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
}
#endif

void HostBackup::FillCommonParams(CommonParams& commonParams)
{
    commonParams.maxBufferCnt = MAXBUFFERCNT;
    commonParams.maxBufferSize = MAXBUFFERSIZE; // 10kb
    commonParams.maxErrorFiles = MAXERRORFILES;
    if (IsSubTaskStatsFileExists()) {
        WARNLOG("subTask %s is being re-executed!", m_subJobId.c_str());
        commonParams.isReExecutedTask = true;
    }
    if (IsAggregate()) {
        commonParams.backupDataFormat = BackupDataFormat::AGGREGATE;
        commonParams.maxAggregateFileSize = stoul(m_fileset.m_advParms.m_maxSizeAfterAggregate);
        commonParams.maxFileSizeToAggregate = stoul(m_fileset.m_advParms.m_maxSizeToAggregate);
        commonParams.aggregateThreadNum = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixAggregatorThreadNum");
        commonParams.writeMeta = false;
        commonParams.writeAcl = false;
        commonParams.writeExtendAttribute = false;
        FillAggregateFileSet(commonParams.aggregateFileSet);
    } else {
        commonParams.backupDataFormat = BackupDataFormat::NATIVE;
        commonParams.writeMeta = true;
        commonParams.writeAcl = true;
        commonParams.writeExtendAttribute = true;
    }
#ifdef WIN32
    commonParams.writeExtendAttribute = false;
    commonParams.metaPath = PluginUtils::ReverseSlash(m_metaFsPath);
#else
    commonParams.metaPath = m_metaFsPath;
#endif
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.skipFailure = (m_fileset.m_advParms.m_isContinueOnFailed == "true");
    commonParams.writeSparseFile = (m_fileset.m_advParms.m_isSparseFileDetection == "true");
    commonParams.discardReadError = false;
    if (Module::ConfigReader::getInt("FilePluginConfig", "BACKUP_READ_FAILED_DISCARD") > 0) {
        WARNLOG("discard read error option enabled! jobId %s, subJobId %s", m_jobId.c_str(), m_subJobId.c_str());
        commonParams.discardReadError = true;
    }
    if (Module::ConfigReader::getInt("FilePluginConfig", "FORCE_DISABLE_ACL") > 0) {
        WARNLOG("force disable ACL during backup by config! jobId %s, subJobId %s",
            m_jobId.c_str(), m_subJobId.c_str());
        commonParams.writeAcl = false;
    }
    if (m_fileset.m_advParms.m_isEnableAcl == FALSE_STR) {
        WARNLOG("disable ACL during backup! jobId %s, subJobId %s", m_jobId.c_str(), m_subJobId.c_str());
        commonParams.writeAcl = false;
    }
    FillFailureRecorderParams(commonParams);
}

void HostBackup::FillBackupConfig(BackupParams &backupParams, BackupSubJob &backupSubJob)
{
    backupParams.backupType = IsFullBackup() ? BackupType::BACKUP_FULL : BackupType::BACKUP_INC;
    FillBackupConfigPhase(backupParams, backupSubJob);
#ifdef WIN32
    FillBackupConfigWin32(backupParams, backupSubJob);
#else
    FillBackupConfigPosix(backupParams, backupSubJob);
#endif
    FillCommonParams(backupParams.commonParams);
    PrintBackupConfig(backupParams);
}

void HostBackup::PrintBackupConfig(const BackupParams& backupParams)
{
    INFOLOG("FillBackupConfig jobId: %s, subJobId: %s", m_jobId.c_str(), m_subJobId.c_str());
    INFOLOG("FillBackupConfig metaFilePath: %s", backupParams.scanAdvParams.metaFilePath.c_str());
#ifndef WIN32
    INFOLOG("FillBackupConfig SRC_ROOT_PATH: %s",
        dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath.c_str());
    INFOLOG("FillBackupConfig maxMemory: %u",
        dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->maxMemory);
    INFOLOG("FillBackupConfig posixReaderThreadNum: %u",
        dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->threadNum);
    INFOLOG("FillBackupConfig posixWriterThreadNum: %u",
        dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->threadNum);
#else
    INFOLOG("FillBackupConfig SRC_ROOT_PATH: %s",
        dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath.c_str());
#endif
    INFOLOG("FillBackupConfig DST_ROOT_PATH: %s", m_dataFsPath.c_str());
    INFOLOG("FillBackupConfig isAggregate: %d", static_cast<int>(IsAggregate()));
    INFOLOG("FillBackupConfig maxAggregateFileSize: %u", backupParams.commonParams.maxAggregateFileSize);
    INFOLOG("FillBackupConfig maxFileSizeToAggregate: %u", backupParams.commonParams.maxFileSizeToAggregate);
    INFOLOG("FillBackupConfig posixAggregatorThreadNum: %u", backupParams.commonParams.aggregateThreadNum);
    INFOLOG("FillBackupConfig enableACL: %u", backupParams.commonParams.writeAcl);
}

void HostBackup::FillBackupConfigPhase(BackupParams &backupParams, BackupSubJob &backupSubJob)
{
    if (backupSubJob.subTaskType == SUBJOB_TYPE_DATACOPY_COPY_PHASE) {
        backupParams.phase = BackupPhase::COPY_STAGE;
        return;
    } else if (backupSubJob.subTaskType == SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE) {
        backupParams.phase = BackupPhase::HARDLINK_STAGE;
        return;
    } else if (backupSubJob.subTaskType == SUBJOB_TYPE_DATACOPY_DELETE_PHASE) {
        backupParams.phase = BackupPhase::DELETE_STAGE;
        return;
    } else if (backupSubJob.subTaskType == SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE) {
        backupParams.phase = BackupPhase::DIR_STAGE;
        return;
    }
    return;
}

bool HostBackup::UpdateSubBackupStats(bool forceComplete)
{
    if (m_subJobId.empty()) {
        DBGLOG("UpdateBackupTaskStats - subJobId is empty, main jobId: %s", m_jobId.c_str());
        return true;
    }
    BackupStats currStats = m_backup->GetStats();
    if (forceComplete) {
        // 目录阶段计算目录的失败数
        if (m_backup->m_backupParams.phase == BackupPhase::DIR_STAGE) {
            currStats.noOfDirFailed = currStats.noOfDirToBackup - currStats.noOfDirCopied;
        } else {
            currStats.noOfFilesFailed = currStats.noOfFilesToBackup - currStats.noOfFilesCopied;
        }
    }
    SerializeBackupStats(currStats, m_subBackupStats);
    ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, m_subJobId);
    bool ret = ShareResourceManager::GetInstance().UpdateResource(
        ShareResourceType::BACKUP, m_subJobId, m_subBackupStats);
    ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, m_subJobId);
    if (!ret) {
        ERRLOG("Update sub job stats failed, jobId: %s, sub jobId: %s", m_jobId.c_str(), m_subJobId.c_str());
        return false;
    }
    return true;
}

bool HostBackup::UpdateMainBackupStats(BackupStatistic& mainStats)
{
    vector<string> statsList;
    if (!GetFileListInDirectory(m_statisticsPath, statsList)) {
        ERRLOG("Get backup stats list failed, jobId: %s", m_jobId.c_str());
        return false;
    }
    bool ret = ShareResourceManager::GetInstance().QueryResource(ShareResourceType::GENERAL, m_jobId, m_generalInfo);
    time_t currTime = PluginUtils::GetCurrentTimeInSeconds();
    time_t backDuration = currTime - m_generalInfo.m_backupCopyPhaseStartTime;
    DBGLOG("UpdateMainBackupStats, currTime: %d, startTime: %d, duration: %d",
        currTime, m_generalInfo.m_backupCopyPhaseStartTime, backDuration);
    BackupStatistic subStats;
    for (const string& path : statsList) {
        string::size_type pos = path.find(BACKUP_KEY_SUFFIX);
        if (pos == string::npos) {
            continue;
        }
        string subJobId = path.substr(m_statisticsPath.length() + NUMBER1, m_subJobId.length());
        DBGLOG("UpdateMainBackupStats, path: %s, jobId: %s, subJobId: %s",
            path.c_str(), m_jobId.c_str(), subJobId.c_str());
        ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, subJobId);
        bool ret = ShareResourceManager::GetInstance().QueryResource(path, subStats);
        ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, subJobId);
        DBGLOG("UpdateMainBackupStats, sub: %llu, speed: %llu, main: %llu, speed: %llu", subStats.noOfBytesCopied,
            subStats.backupspeed, mainStats.noOfBytesCopied, mainStats.backupspeed);
        if (!ret) {
            ERRLOG("Query failed, jobId: %s, subJobId: %s, path: %s", m_jobId.c_str(), path.c_str(), subJobId.c_str());
            return false;
        }
        mainStats = CalcuSumStructBackupStatistic(mainStats, subStats);
    }
    uint64_t dataInKB = mainStats.noOfBytesCopied / NUMBER1024;
    m_jobSpeed = dataInKB / backDuration;
    DBGLOG("UpdateMainBackupStats, data: %lluKB, duration: %llus, m_jobSpeed: %lluKB/s",
        dataInKB, backDuration, m_jobSpeed);
    return true;
}

bool HostBackup::ReportBackupRunningStats(const BackupStatistic& backupStats)
{
    PrintBackupStats(backupStats, m_jobId);
    if (!ShareResourceManager::GetInstance().CanReportStatToPM(m_jobId + "_backup")) {
        return true;
    }
    DBGLOG("Data copied: %llu Byte, jobId: %s, subJobId: %s",
        backupStats.noOfBytesCopied, m_jobId.c_str(), m_subJobId.c_str());
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_host_backup_data_inprogress_label",
        to_string(backupStats.noOfDirCopied),
        to_string(backupStats.noOfFilesCopied),
        FormatCapacity(backupStats.noOfBytesCopied));
    return true;
}

int HostBackup::PostJobInner()
{
    if (!InitJobInfo()) {
        HCP_Log(ERR, MODULE) << "InitJobInfo failed" << HCPENDLOG;
        return Module::FAILED;
    }
    MergeBackupFailureRecords();
    DeleteSnapshot();
    DeleteSharedResources();
    if (!HandleCacheDirectories()) {
        ERRLOG("HandleCacheDirectories failed");
        return Module::FAILED;
    }
    if (IsDirExist(m_cacheFsPath + dir_sep + m_jobId)) {
        Remove(m_cacheFsPath + dir_sep + m_jobId);
    }
    return Module::SUCCESS;
}

bool HostBackup::HandleCacheDirectories() const
{
    string prevBackDir = PluginUtils::PathJoin(m_cacheFsPath, "backup-job", "backup", "prevctrl");
    string currBackDir = PluginUtils::PathJoin(m_cacheFsPath, "backup-job", "backup", "ctrl");
    string prevMetaDir = PluginUtils::PathJoin(m_cacheFsPath, "backup-job", "scan", "meta", "prevmeta");
    string currMetaDir = PluginUtils::PathJoin(m_cacheFsPath, "backup-job", "scan", "meta", "latest");

    /* Rename the control files at the end of backup */
    if (IsDirExist(currBackDir) && !Rename(currBackDir, prevBackDir)) {
        ERRLOG("Rename backup ctrol dir failed, prev: %s, curr: %s", prevBackDir.c_str(), currBackDir.c_str());
        return false;
    }
    if (IsDirExist(currMetaDir) && !Rename(currMetaDir, prevMetaDir)) {
        ERRLOG("Rename scan meta dir failed, prev: %s, curr: %s", prevBackDir.c_str(), currBackDir.c_str());
        return false;
    }
    return true;
}

bool HostBackup::EnqueueSubjobInfo(set<string>& jobInfoSet, const string& prefix)
{
    DBGLOG("Enter EnqueueSubjobInfo");
    BackupSubJobInfo backupSubJobInfo { m_scanControlPath, m_backupControlPath, prefix, m_incControlDir };
    string jobInfoStr;
    if (!Module::JsonHelper::StructToJsonString(backupSubJobInfo, jobInfoStr)) {
        ERRLOG("backupSubJobInfo struct to json string failed");
        return false;
    }
    DBGLOG("Enqueue job info: %s", jobInfoStr.c_str());
    jobInfoSet.emplace(jobInfoStr);
    return true;
}

// 若未开启一致性备份，则仅进行此阶段扫描
// 若开启一致性备份，此阶段处理文件集所选路径不支持快照的（支持快照的在下一阶段处理）
bool HostBackup::ScanPrimalSourceVol(HostScanStatistics& preScanStats, set<string>& jobInfoSet)
{
    if (m_fileset.m_advParms.m_isConsistent == TRUE_STR
        && m_fileset.m_protectedPaths.size() == m_filesetPathsWithSnap.size()) {
        DBGLOG("All fileset selected path has snapshot, don't scan primary source");
        return true;
    }
    ScanConfig scanConfig {};
    FillScanConfigMetaPath(scanConfig, SOURCE_PRIMARY_VOLUME);
    FillScanConfig(scanConfig);
    ExcludePathsInConfig(scanConfig);
    // 跨卷 TOBE implement
    FillCrossFilterConfig(scanConfig);
    FilterAllSubVol(scanConfig);
    m_scanner = ScanMgr::CreateScanInst(scanConfig);
    std::vector<std::string> scanSrcPaths;
    for (const string& path : m_fileset.m_protectedPaths) {
        if (m_filesetPathsWithSnap.count(path) != 0) {
            continue;
        }
#ifdef _AIX
        if (m_deviceMountPtr->GetFsType(path) == "nfs3") {
            m_protectedPathsForNfs.emplace_back(path);
            continue;
        }
#endif
#if defined (__linux__) || defined(SOLARIS)
        if (m_deviceMountPtr->GetFsType(path).find("nfs") == 0) {
            m_protectedPathsForNfs.emplace_back(path);
            continue;
        }
#endif

        scanSrcPaths.push_back(path);
    }
    if (!StartScanner(scanConfig, scanSrcPaths)) {
        return false;
    }
    SubJobStatus::type jobStatus = SubJobStatus::FAILED;
    string jobLogLabel = "file_plugin_host_backup_scan_fail_label";
    MonitorScanner(preScanStats, jobStatus, jobLogLabel);
    if (m_scanStats.mEntriesMayFailedToArchive != 0) {
        WARNLOG("%llu entries may failed to archive due to long path/filename", m_scanStats.mEntriesMayFailedToArchive);
    }
    if (jobStatus != SubJobStatus::COMPLETED) {
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, jobLogLabel);
        return false;
    }
    if (!EnqueueSubjobInfo(jobInfoSet) || !ScanPrimalSourceVolForNfs(preScanStats, jobInfoSet)) {
        return false;
    }
    return true;
}

// some *nix VFS have no support for 'getfacl' or 'setfacl'
bool HostBackup::ScanPrimalSourceVolForNfs(HostScanStatistics& preScanStats, set<string>& jobInfoSet)
{
#if defined(_AIX) || defined(SOLARIS) || defined(__linux__)
    if (m_protectedPathsForNfs.empty()) {
        INFOLOG("no nfs primary volume need to be scanned");
        return true;
    }
    ScanConfig scanConfig {};
    FillScanConfigMetaPath(scanConfig, SOURCE_PRIMARY_NFS_VOLUME);
    FillScanConfig(scanConfig);
    // Aix不支持扫描nfsv3的acl
    scanConfig.scanAcl = false;
    scanConfig.scanExtendAttribute = false;
    FilterAllSubVol(scanConfig);
    for (const std::string& nfsProtectedPath : m_protectedPathsForNfs) {
        INFOLOG("scan nfs primary volume %s with Acl disabled", nfsProtectedPath.c_str());
    }
    if (!StartScanner(scanConfig, m_protectedPathsForNfs)) {
        return false;
    }
    SubJobStatus::type jobStatus = SubJobStatus::FAILED;
    string jobLogLabel = "file_plugin_host_backup_scan_fail_label";
    MonitorScanner(preScanStats, jobStatus, jobLogLabel);
    if (jobStatus != SubJobStatus::COMPLETED) {
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, jobLogLabel);
        return false;
    }
    if (!EnqueueSubjobInfo(jobInfoSet)) {
        return false;
    }
    return true;
#else
    INFOLOG("skip ScanPrimalSourceVolForNfs");
    return true;
#endif
}

void HostBackup::ScanPrimalSnapshotVol1(std::string& prefix, std::string srcVolId, ScanConfig& scanConfig,
    std::map<string, string> driverLetterMapper, vector<string>& scanSrcPaths)
{
    DBGLOG("Enter ScanPrimalSnapshotVol1");
#ifdef WIN32
    string driverLetter = driverLetterMapper[srcVolId];
    prefix = LoadSnapshotParentPath() + dir_sep + m_jobId + dir_sep + srcVolId + dir_sep + driverLetter;
#elif !defined(SOLARIS)
    prefix = LoadSnapshotParentPath() + dir_sep + m_jobId + dir_sep + srcVolId;
#endif
    for (const string& path : m_filesetPathsWithSnap) {
        DBGLOG("m_filesetPathsWithSnap path: %s", path.c_str());
        string volId = m_deviceMountPtr->GetVolumeId(path);
        if (volId.empty() || volId != srcVolId) {
            DBGLOG("volId: %s, srcVolId: %s", volId.c_str(), srcVolId.c_str());
            continue;
        }
#ifdef SOLARIS
        if (prefix.empty()) {
            prefix = m_deviceMountPtr->FindDevice(path)->mountPoint;
            DBGLOG("solaris prefix: %s", prefix.c_str());
        }
#endif
        scanSrcPaths.push_back(path);
        FilterSubVol(scanConfig, path, prefix);
    }
    string pathId = VOLUM_PREFIX + srcVolId;
    FillScanConfigMetaPath(scanConfig, pathId);
    FillScanConfig(scanConfig);
    ExcludePathsInConfig(scanConfig, prefix);
    FillScanConfigMapFunc(scanConfig, prefix);
}

/*******************************************************************************
  函 数 名		:  ScanPrimalSnapshotVol
  功能描述		:  在一致性备份场景下，对用户所选路径下支持快照的原生卷依次扫描
  输入参数		:  preScanStats:上一次扫描的统计量（累加）;    jobInfoSet:上次扫描生成的子任务信息（累加）
  输出参数		:  None
  返 回 值		:  true/false
*******************************************************************************/
bool HostBackup::ScanPrimalSnapshotVol(HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet)
{
    if (m_fileset.m_advParms.m_isConsistent == FALSE_STR || m_filesetPathsWithSnap.empty()) {
        DBGLOG("Do not need ScanPrimalSnapshotVol");
        return true;
    }
    DBGLOG("Enter ScanPrimalSnapshotVol");
    set<string> snapSrcVolIds;
    std::map<string, string> driverLetterMapper;
    for (const string& path : m_filesetPathsWithSnap) {
        std::string id = m_deviceMountPtr->GetVolumeId(path);
        snapSrcVolIds.emplace(id);
        DBGLOG("path: %s, volume id: %s", path.c_str(), id.c_str());
        driverLetterMapper[id] = path[0];
    }
    for (const string& srcVolId : snapSrcVolIds) {
        ScanConfig scanConfig {};
        vector<string> scanSrcPaths;
        string prefix;
        ScanPrimalSnapshotVol1(prefix, srcVolId, scanConfig, driverLetterMapper, scanSrcPaths);
        if (!StartScanner(scanConfig, scanSrcPaths, prefix)) {
            return false;
        }
        SubJobStatus::type jobStatus = SubJobStatus::FAILED;
        string jobLogLabel = "file_plugin_host_backup_scan_fail_label";
        MonitorScanner(preScanStats, jobStatus, jobLogLabel);
        if (jobStatus != SubJobStatus::COMPLETED) {
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, jobLogLabel);
            return false;
        }
        if (!EnqueueSubjobInfo(jobInfoSet, prefix)) {
            return false;
        }
    }
    return true;
}

bool HostBackup::StartScanner(const ScanConfig& scanConfig, const vector<string>& paths, const std::string& prefix)
{
    DBGLOG("Enter StartScanner, prefix: %s", prefix.c_str());
    m_scanner = ScanMgr::CreateScanInst(scanConfig);
    for (const std::string& path : paths) {
        std::string scanPath = path;
        if (!prefix.empty()) {
#ifdef SOLARIS
            size_t pos = prefix.size();
            // path相对于所在卷挂载路径下的路径
            std::string volumeRelativePath = path.substr(pos);
            DBGLOG("volumeRelativePath: %s", volumeRelativePath.c_str());
            scanPath = PluginUtils::PathJoin(prefix, ZFS_SNAPSHOT_DIR, m_jobId, volumeRelativePath);
#elif defined(WIN32)
            scanPath = PluginUtils::PathJoin(prefix, path.substr(NUMBER2));
#else
            scanPath = PluginUtils::PathJoin(prefix, path);
#endif
        }
#ifdef SOLARIS
        INFOLOG("EnqueueV2 path: %s, jobId: %s", scanPath.c_str(), m_jobId.c_str());
        SCANNER_STATUS ret = m_scanner->EnqueueV2(scanPath);
#else
        /* When scann the original volumes, the prefix is empty. */
        INFOLOG("Enqueue path: %s, prefix: %s, jobId: %s", scanPath.c_str(), prefix.c_str(), m_jobId.c_str());
        SCANNER_STATUS ret = m_scanner->Enqueue(scanPath, prefix);
#endif
        if (ret != SCANNER_STATUS::SUCCESS) {
            ReportJobLabel(JobLogLevel::TASK_LOG_WARNING,
                "file_plugin_backup_selected_path_not_accessible_label", scanPath);
            ERRLOG("Enqueue failed, path: %s, jobId: %s", scanPath.c_str(), m_jobId.c_str());
            return false;
        }
    }
    if (m_scanner->Start() != SCANNER_STATUS::SUCCESS) {
        if (m_scanner != nullptr) {
            m_scanner->Destroy();
        }
        ERRLOG("Start scanner instance failed, jobId: %s", m_jobId.c_str());
        return false;
    }
    return true;
}

void HostBackup::FillScanConfigMapFunc(ScanConfig& scanConfig, std::string& orginalMntPoint)
{
#ifdef SOLARIS
    if (orginalMntPoint.empty()) {
        return;
    }
    while (orginalMntPoint.back() == '/' && orginalMntPoint != "/") {
        orginalMntPoint.pop_back();
    }
    DBGLOG("orginalMntPoint: %s", orginalMntPoint.c_str());
    size_t pos = orginalMntPoint.size();
    std::string infix = PluginUtils::PathJoin(ZFS_SNAPSHOT_DIR, m_jobId);
    DBGLOG("infix: %s", infix.c_str());
    scanConfig.pathMapper = std::make_shared<InfixSnapshotPathMapper>(pos, infix);
#endif
}

/*******************************************************************************
  函 数 名		:  GetVolumeMountPath
  功能描述		:  获取路径path所在文件系统的挂载路径
  输入参数		:  path：查询路径
  输出参数		:  None
  返 回 值		:  文件所在文件系统的挂载点
*******************************************************************************/
string HostBackup::GetVolumeMountPath(const string& path) const
{
    if (path.empty()) {
        WARNLOG("Path is empty, cann't get volume mount path, jobId: %s", m_jobId.c_str());
        return "";
    }
    shared_ptr<FsDevice> fsDevicePtr = m_deviceMountPtr->FindDevice(path);
    if (fsDevicePtr == nullptr) {
        ERRLOG("fsDevicePtr is nullptr, jobId: %s", m_jobId.c_str());
        return "";
    }
    string mntPath = fsDevicePtr->mountPoint;
    if (mntPath.empty()) {
        ERRLOG("Get vol mnt path failed, path: %s, jobId: %s", path.c_str(), m_jobId.c_str());
    } else {
        INFOLOG("Get vol mnt path, path: %s, mntPath: %s, jobId: %s", path.c_str(), mntPath.c_str(), m_jobId.c_str());
    }
    return mntPath;
}

void HostBackup::ExcludePathsInConfig(ScanConfig &scanConfig, const std::string& pathFix)
{
    if (m_excludePathList.empty()) {
        return;
    }
    for (const std::string& path : m_excludePathList) {
        std::string filterPath = path;
        if (!pathFix.empty()) {
#ifdef SOLARIS
            shared_ptr<FsDevice> fsDevicePtr = m_deviceMountPtr->FindDevice(path);
            if (fsDevicePtr == nullptr) {
                scanConfig.crossVolumeSkipSet.emplace(path);
                INFOLOG("fsDevicePtr is nullptr, filterPath: %s, jobId: %s", path.c_str(), m_jobId.c_str());
                continue;
            }
            string mntPath = fsDevicePtr->mountPoint;
            filterPath = PluginUtils::PathJoin(mntPath, ZFS_SNAPSHOT_DIR, path.substr(mntPath.size()));
#elif defined(WIN32)
            filterPath = pathFix + path.substr(NUMBER2);
            transform(filterPath.begin(), filterPath.end(), filterPath.begin(), ::tolower);
#else
            filterPath = pathFix + path;
#endif
        }
        scanConfig.crossVolumeSkipSet.emplace(filterPath);
        INFOLOG("filterPath: %s, jobId: %s", filterPath.c_str(), m_jobId.c_str());
    }
}

void HostBackup::FilterSubVol(ScanConfig &scanConfig, std::string path, const string& prefix)
{
    if (m_fileset.m_advParms.m_isConsistent == FALSE_STR || m_fileset.m_advParms.m_isCrossFileSystem == FALSE_STR) {
        DBGLOG("consistent is [false] or crossFileSystem is [false], don't need do this");
        return;
    }
    vector<shared_ptr<FsDevice>> subVolumes;
    if (!m_deviceMountPtr->GetSubVolumes(path, subVolumes)) {
        ERRLOG("Get sub volumes for path: %s failed, jobId: %s", path.c_str(), m_jobId.c_str());
        return;
    }
    for (auto devicePtr : subVolumes) {
        if (!IsBackupDevice(devicePtr)) {
            DBGLOG("This Dir: %s has been added to the cross-volume filter", devicePtr->mountPoint.c_str());
            continue;
        }
#ifdef SOLARIS
        string filterPath;
        if (prefix.empty()) {
            filterPath = devicePtr->mountPoint;
        } else {
            string subVolMntPoint = devicePtr->mountPoint;
            size_t pos = prefix.size();
            // path相对于所在卷挂载路径下的路径
            std::string volumeRelativePath = subVolMntPoint.substr(pos);
            DBGLOG("volumeRelativePath: %s", volumeRelativePath.c_str());
            filterPath = PluginUtils::PathJoin(prefix, ZFS_SNAPSHOT_DIR, m_jobId, volumeRelativePath);
        }
#elif defined(WIN32)
        string filterPath = prefix + devicePtr->mountPoint.substr(NUMBER2);
        filterPath = filterPath.substr(0, filterPath.length()-1);
        transform(filterPath.begin(), filterPath.end(), filterPath.begin(), ::tolower);
#else
        string filterPath = prefix + devicePtr->mountPoint;
#endif
        DBGLOG("FilterSubVol - DIR: %s", filterPath.c_str());
        scanConfig.crossVolumeSkipSet.emplace(filterPath);
    }
}

void HostBackup::skipSubVolumeDir(const std::string& prefix, const std::string& path, ScanConfig& scanConfig)
{
#ifdef WIN32
    // for subvolume, skip recycleBin && systemVolumeInfo dir
    string systemVolumeInfo = prefix + path.substr(NUMBER2) + SYSTEMVOLUMEINFO;
    string recycleBin = prefix + path.substr(NUMBER2) + RECYCLEBIN;
    transform(systemVolumeInfo.begin(), systemVolumeInfo.end(), systemVolumeInfo.begin(), ::tolower);
    transform(recycleBin.begin(), recycleBin.end(), recycleBin.begin(), ::tolower);
    scanConfig.crossVolumeSkipSet.emplace(systemVolumeInfo);
    scanConfig.crossVolumeSkipSet.emplace(recycleBin);
    DBGLOG("systemVolumeInfo:%s, recycleBin:%s", systemVolumeInfo.c_str(), recycleBin.c_str());
#endif
}

bool HostBackup::ScanSubVolume(HostScanStatistics& preScanStats, set<string>& jobInfoSet)
{
    if (m_fileset.m_advParms.m_isConsistent == FALSE_STR || m_fileset.m_advParms.m_isCrossFileSystem == FALSE_STR) {
        INFOLOG("isCrossFileSystem is false, do not scan sub volume, jobId: %s", m_jobId.c_str());
        return true;
    }
    // 对每一个子卷单独扫描
    for (const auto& subVolInfo : m_subVolInfo) {
        ScanConfig scanConfig {};
        string pathId = SUB_VOLUM_PREFIX + to_string(subVolInfo.second);
        DBGLOG("pathId:%s", pathId.c_str());
        FillScanConfigMetaPath(scanConfig, pathId);
        string prefix;
        string path = subVolInfo.first;
        if (m_subVolPathWithSnap.count(path) != 0) {
#ifdef SOLARIS
            prefix = m_deviceMountPtr->FindDevice(path)->mountPoint;
#elif defined(WIN32)
            prefix = PathJoin(
                LoadSnapshotParentPath(), m_jobId, m_deviceMountPtr->GetVolumeId(path), path.substr(0, 1));
#else
            prefix = PathJoin(LoadSnapshotParentPath(), m_jobId, m_deviceMountPtr->GetVolumeId(path));
#endif
        }
        FillScanConfig(scanConfig);
        ExcludePathsInConfig(scanConfig, prefix);
        FillScanConfigMapFunc(scanConfig, prefix);
        FillScanAclConfig(scanConfig, path);
        FilterSubVol(scanConfig, path, prefix);
        skipSubVolumeDir(prefix, path, scanConfig);
        std::vector<std::string> scanSrcPaths { path };
        if (!StartScanner(scanConfig, scanSrcPaths, prefix)) {
            return false;
        }
        SubJobStatus::type jobStatus = SubJobStatus::FAILED;
        string jobLogLabel = "file_plugin_host_backup_scan_fail_label";
        MonitorScanner(preScanStats, jobStatus, jobLogLabel);
        if (jobStatus != SubJobStatus::COMPLETED) {
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, jobLogLabel);
            return false;
        }
        if (!EnqueueSubjobInfo(jobInfoSet, prefix)) {
            return false;
        }
    }
    return true;
}

// 对上次备份失败的文件单独做全量扫描
bool HostBackup::ScanFailedVolume(HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet)
{
    INFOLOG("Enter ScanFailedVolume!");
    if (IsFullBackup()) {
        return true;
    }
    vector<string> subVolMetaDirName;
    std::string scanMetaPath = PathJoin(m_cacheFsPath, "backup-job", "scan", "meta");
    if (!GetDirListInDirectory(scanMetaPath, subVolMetaDirName)) {
        WARNLOG("get volume list for scan failed volume failed!");
        return true;
    }
    std::vector<std::string> failedRecords;
    for (uint32_t i = 0; i < subVolMetaDirName.size(); i++) {
        std::string failedRecordPath = PathJoin(scanMetaPath, subVolMetaDirName[i], "previous", "failed_file_record");
        INFOLOG("check failedRecordPath : %s", failedRecordPath.c_str());
        if (!PluginUtils::IsFileExist(failedRecordPath)) {
            INFOLOG("previous failed record file not exist! skip ScanFailedVolume!");
            continue;
        }
        
        std::ifstream infile(failedRecordPath);
        if (!infile.is_open()) {
#ifdef WIN32
            WARNLOG("failed_file_record not open!, %s, %d", failedRecordPath.c_str(), ::GetLastError());
#else
            WARNLOG("failed_file_record not open!, %s, %d", failedRecordPath.c_str(), errno);
#endif
            continue;
        }
        std::string line;
        while (std::getline(infile, line)) {
            FailedRecordItem item;
            bool ret = ProcessFailedRecordLine(line, item);
            if (!ret) {
                continue;
            }
            if (IsErrNeedSkip(item.errNum)) {
                DBGLOG("skip failed file: %s", line.c_str());
                continue;
            }
            DBGLOG("external enqueue: %s", item.filePath.c_str());
            failedRecords.push_back(item.filePath);
        }
        PluginUtils::RemoveFile(failedRecordPath);
    }
    if (failedRecords.empty()) {
        INFOLOG("no failed record to scan!");
        return true;
    }
    return DoRealScanFailedVolume(failedRecords, preScanStats, jobInfoSet);
}

bool HostBackup::ProcessFailedRecordLine(const std::string& line, FailedRecordItem& item)
{
    try {
        // get metaIndex, errno, offset from single line
        size_t commaPos = line.find(',');
        item.metaIndex = std::stoul(line.substr(0, commaPos));
        std::string filePath = line.substr(commaPos + 1);
        commaPos = filePath.find(',');
        item.errNum = std::stoul(filePath.substr(0, commaPos));
        filePath = filePath.substr(commaPos + 1);
        commaPos = filePath.find(',');
        item.offset = std::stoull(filePath.substr(0, commaPos));
        item.filePath = filePath.substr(commaPos + 1);
#ifdef WIN32
        item.filePath = Module::Win32PathUtil::PosixToWin32(item.filePath);
#endif
        return true;
    } catch (const std::out_of_range& e) {
        WARNLOG("out of range err: %s. line content: %s", e.what(), line.c_str());
        return false;
    } catch (...) {
        WARNLOG("UNKNOWN exception! line content: %s", line.c_str());
        return false;
    }
    return false;
}

bool HostBackup::IsErrNeedSkip(const uint32_t errNum)
{
    if (errNum == ENOENTERRNUM) {
        return true;
    }
    // 这个报错要跳过是因为增量备份本来就要扫这种报错的文件， 这里失败文件就跳过不用扫了
    if (errNum == E_BACKUP_READ_LESS_THAN_EXPECTED) {
        return true;
    }
    return false;
}

bool HostBackup::DoRealScanFailedVolume(const std::vector<std::string>& failedRecords,
    HostScanStatistics& preScanStats, std::set<std::string>& jobInfoSet)
{
    ScanConfig scanConfig {};
    string pathId = "failedVolume";
    FillScanConfigMetaPath(scanConfig, pathId);
    FillScanConfig(scanConfig);
    scanConfig.scanType = ScanJobType::FULL;
    scanConfig.lastBackupTime = 0;
    scanConfig.maxScanQueueSize = failedRecords.size() + DEFAULT_SCANQUEUE_SIZE;
    m_scanner = ScanMgr::CreateScanInst(scanConfig);
    for (const std::string& file : failedRecords) {
        m_scanner->Enqueue(file);
    }
    if (m_scanner->Start() != SCANNER_STATUS::SUCCESS) {
        if (m_scanner != nullptr) {
            m_scanner->Destroy();
        }
        ERRLOG("Start scanner instance failed, jobId: %s", m_jobId.c_str());
        return false;
    }
    string jobLogLabel = "file_plugin_host_backup_scan_fail_label";
    SubJobStatus::type jobStatus = SubJobStatus::FAILED;
    ScanStatistics stats = MonitorScanner(preScanStats, jobStatus, jobLogLabel);
    m_numberOfFailedFilesScaned = stats.mTotFiles;
    if (jobStatus != SubJobStatus::COMPLETED) {
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, jobLogLabel);
        return false;
    }
    if (!EnqueueSubjobInfo(jobInfoSet, "")) {
        return false;
    }
    return true;
}

void HostBackup::FillScanAclConfig(ScanConfig &scanConfig, const std::string& path) const
{
#ifdef _AIX
    // aix nfs3 文件系统不扫描acl
    if (m_deviceMountPtr->GetFsType(path) == "nfs3") {
        INFOLOG("AIX nfs3 sub volume, disable scanAcl, path: %s", path.c_str());
        scanConfig.scanAcl = false;
        return;
    }
#endif

#ifdef SOLARIS
    // disable scanning ACL for Solaris for it's VFS has no 'setfacl' driver support
    if (m_deviceMountPtr->GetFsType(path).find("nfs") == 0) {
        INFOLOG("Solaris nfs sub volume, disable scanAcl, path: %s", path.c_str());
        scanConfig.scanAcl = false;
        return;
    }
#endif

#ifdef __linux__
    // disable scanning ACL for linux for it's VFS has no 'setfacl' driver support
    if (m_deviceMountPtr->GetFsType(path).find("nfs") == 0) {
        INFOLOG("Solaris nfs sub volume, disable scanAcl, path: %s", path.c_str());
        scanConfig.scanAcl = false;
        scanConfig.scanExtendAttribute = false;
        return;
    }
#endif

    return;
}

void HostBackup::GetBackupSubVolumesPath()
{
    vector<shared_ptr<FsDevice>> subVolumes;
    for (const string& path : m_fileset.m_protectedPaths) {
        if (!m_deviceMountPtr->GetSubVolumes(path, subVolumes)) {
            continue;
        }
        for (auto devicePtr : subVolumes) {
            if (!IsBackupDevice(devicePtr)) {
                DBGLOG("This Dir: %s has been added to the cross-volume filter", devicePtr->mountPoint.c_str());
                continue;
            }
            m_subVolInfo.emplace(devicePtr->mountPoint, devicePtr->devNo);
        }
    }
    for (const auto& subVolInfo : m_subVolInfo) {
        INFOLOG("path in subVolPath: %s", subVolInfo.first.c_str());
    }
    return;
}

int HostBackup::ReportJobProgress(SubJobStatus::type &jobStatus)
{
    if (jobStatus == SubJobStatus::COMPLETED) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS100));
        return Module::SUCCESS;
    }
    uint64_t errCode = INITIAL_ERROR_CODE;
    if (m_backupStatus == BackupPhaseStatus::FAILED_NOSPACE) {
        errCode = E_BACKUP_FAILED_NOSPACE_ERROR;
    } else if (m_backupStatus == BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE) {
        errCode = E_BACKUP_BACKUP_SECONDARY_SERVER_NOT_REACHABLE;
    }
    if (m_backupStatus != BackupPhaseStatus::FAILED) {
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0),
            "file_plugin_host_backup_data_fail_label", errCode);
        return Module::FAILED;
    }
    ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0),
        "file_plugin_host_backup_data_fail_label", errCode);
    return Module::FAILED;
}

void HostBackup::GeneratedCopyCtrlFileCb(const void *usrData, string ctrlFile)
{
    (void)usrData;
    DBGLOG("Generated copy ctrl file: %s", ctrlFile.c_str());
}

void HostBackup::GeneratedHardLinkCtrlFileCb(const void *usrData, string ctrlFile)
{
    (void)usrData;
    DBGLOG("Generated hard link ctrl file: %s",  ctrlFile.c_str());
}

void HostBackup::FillScanConfig(ScanConfig &scanConfig)
{
    scanConfig.jobId = m_jobId;
    scanConfig.subJobId = m_jobId; // scanner subJobId equals to jobId
    scanConfig.failureRecordRootPath = m_failureRecordRoot;
    scanConfig.reqID = m_mainJobRequestId;

    scanConfig.scanType = IsFullBackup() ? (ScanJobType::FULL) : (ScanJobType::INC);

    scanConfig.scanIO = IOEngine::POSIX;
#ifdef WIN32
    scanConfig.scanIO = IOEngine::WIN32_IO;
#endif
    scanConfig.usrData = (void *)this;
    scanConfig.lastBackupTime = IsFullBackup() ? 0 : m_prevBackupCopyInfo.m_lastBackupTime;
    DBGLOG("scanConfig.lastBackupTime: %s", ConvertToReadableTime(m_prevBackupCopyInfo.m_lastBackupTime).c_str());

    /* config meta/control file path */
#ifdef WIN32
    scanConfig.metaPath = PluginUtils::ReverseSlash(m_scanMetaPath);
    scanConfig.metaPathForCtrlFiles = PluginUtils::ReverseSlash(m_scanControlPath);
#else
    scanConfig.metaPath = m_scanMetaPath;
    scanConfig.metaPathForCtrlFiles = m_scanControlPath;
#endif

    scanConfig.scanCheckPointEnable = false;
    scanConfig.scanSparseFile = (m_fileset.m_advParms.m_isSparseFileDetection == "true");
    /* Scanner callback function */
    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;

    scanConfig.maxCommonServiceInstance = 1;
    scanConfig.scanCopyCtrlFileSize = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixCopyCtrlFileSize");
    scanConfig.scanCtrlMaxDataSize = Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlDataSize");
    scanConfig.scanCtrlMinDataSize = Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlDataSize");
    scanConfig.scanCtrlFileTimeSec = SCAN_CTRL_FILE_TIMES_SEC;
    scanConfig.scanCtrlMaxEntriesFullBkup =
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlEntriesFullBackup");
    scanConfig.scanCtrlMaxEntriesIncBkup =
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlEntriesIncBackup");
    scanConfig.scanCtrlMinEntriesFullBkup =
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlEntriesFullBackup");
    scanConfig.scanCtrlMinEntriesIncBkup =
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlEntriesIncBackup");
    scanConfig.scanMetaFileSize = ONE_GB;

    scanConfig.writeQueueSize = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "ScanWriteQueueSize");
    scanConfig.scanMetaFileSize = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "ScanDefaultMetaFileSize");
    scanConfig.producerThreadCount = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "ScanProducerThreadCount");

    scanConfig.triggerTime = PluginUtils::GetCurrentTimeInSeconds();
#ifdef SOLARIS
    scanConfig.enableV2 = true;
#else
    scanConfig.enableV2 = false;
#endif
    scanConfig.skipDirs = SCAN_SKIP_DIRS;
    FillScanFilterConfig(scanConfig);
}

void HostBackup::FilterAllSubVol(ScanConfig &scanConfig)
{
    if (m_fileset.m_advParms.m_isConsistent == FALSE_STR || m_fileset.m_advParms.m_isCrossFileSystem == FALSE_STR) {
        DBGLOG("consistent is [false] or crossFileSystem is [false], don't need do this");
        return;
    }
    for (const auto& subVolInfo : m_subVolInfo) {
        INFOLOG("FillCrossFilterForSnapshot - Dir: %s", subVolInfo.first.c_str());
        scanConfig.crossVolumeSkipSet.emplace(subVolInfo.first);
    }
    return;
}

void HostBackup::FillScanConfigMetaPath(ScanConfig &scanConfig, string pathId)
{
    m_scanMetaPath = PathJoin(m_cacheFsPath, "backup-job", "scan", "meta", pathId);
    m_scanControlPath = PathJoin(m_cacheFsPath, "backup-job", "scan", "ctrl", pathId);
    m_backupControlPath = PathJoin(m_cacheFsPath, "backup-job", "backup", "ctrl", pathId);
    m_incControlDir = PathJoin(m_metaFsPath, "lastCtrl", pathId);
    scanConfig.metaPath = m_scanMetaPath;
    scanConfig.metaPathForCtrlFiles = m_scanControlPath;
    CreateDirectory(scanConfig.metaPath);
    CreateDirectory(scanConfig.metaPathForCtrlFiles);
    CreateDirectory(m_backupControlPath);
}

void HostBackup::FillCrossFilterConfig(ScanConfig &scanConfig)
{
    FillInvailMountPoints(scanConfig);
    set<string> excludePaths;
    for (string path : m_fileset.m_protectedPaths) {
        GetExcludeSubPath(path, excludePaths);
    }
    if (excludePaths.empty()) {
        DBGLOG("no Dir need to filter - jobId: %s", m_jobId.c_str());
        return;
    }
    for (auto& excludePath : excludePaths) {
        DBGLOG("FillCrossFilterConfig - DIR: %s", excludePath.c_str());
        scanConfig.crossVolumeSkipSet.emplace(excludePath);
    }
    return;
}

void HostBackup::FillInvailMountPoints(ScanConfig &scanConfig)
{
    DBGLOG("Enter FillInvailMountPoints");
    set<string> invaildMountPoints;
    if (!m_deviceMountPtr->GetInValidMountPoints(invaildMountPoints)) {
        return;
    }
    for (const string& path : invaildMountPoints) {
        if (IsProtectedSubPath(path)) {
            scanConfig.crossVolumeSkipSet.emplace(path);
            DBGLOG("Push invailed mount point to scanner filter: %s", path.c_str());
        }
    }
    DBGLOG("Exit FillInvailMountPoints");
    return;
}

bool HostBackup::IsProtectedSubPath(const string& subPath)
{
    for (const string& path : m_fileset.m_protectedPaths) {
        string pathSep = path + dir_sep;
        size_t pos = pathSep.size();
        if (subPath.size() <= pos) {
            continue;
        }
        if (subPath.substr(0, pos) == pathSep) {
            DBGLOG("subPath: %s, parent path: %s", subPath.c_str(), path.c_str());
            return true;
        }
    }
    return false;
}

void HostBackup::FillScanFilterConfig(ScanConfig &scanConfig)
{
    ScanDirectoryFilter scanDirectoryFilter {};
    ScanFileFilter scanFileFilter {};
    for (size_t i = 0; i < m_backupJobPtr->jobParam.filters.size(); i++) {
        if (m_backupJobPtr->jobParam.filters[i].type == FILTER_TYPE_DIR) {
            FillScanFilterType(m_backupJobPtr->jobParam.filters[i].mode, scanDirectoryFilter.type);
            for (const string &filter : m_backupJobPtr->jobParam.filters[i].values) {
                scanDirectoryFilter.dirList.push_back(filter);
            }
        } else if (m_backupJobPtr->jobParam.filters[i].type == FILTER_TYPE_FILE) {
            FillScanFilterType(m_backupJobPtr->jobParam.filters[i].mode, scanFileFilter.type);
            for (const string &filter : m_backupJobPtr->jobParam.filters[i].values) {
                scanFileFilter.fileList.push_back(filter);
            }
        }
    }
    for (string filter : scanDirectoryFilter.dirList) {
        DBGLOG("FillScanFilterConfig - DIR,  dirList: %s", filter.c_str());
    }
    for (string filter : scanFileFilter.fileList) {
        DBGLOG("FillScanFilterConfig - FILE, fileList: %s", filter.c_str());
    }
    scanConfig.dFilter = scanDirectoryFilter;
    scanConfig.fFilter = scanFileFilter;
}

void HostBackup::FillScanFilterType(const string& mode, FILTER_TYPE& filterType)
{
    if (mode == FILTER_MODEL_EXCLUDE) {
        filterType = FILTER_TYPE::EXCLUDE;
        DBGLOG("FillScanFilterType, type: %s", FILTER_MODEL_EXCLUDE.c_str());
    } else if (mode == FILTER_MODEL_INCLUDE) {
        filterType = FILTER_TYPE::INCLUDE;
        DBGLOG("FillScanFilterType, type: %s", FILTER_MODEL_INCLUDE.c_str());
    } else {
        filterType = FILTER_TYPE::DISABLED;
        DBGLOG("FillScanFilterType, type: DISABLED");
    }
    return;
}

ScanStatistics HostBackup::MonitorScanner(
    HostScanStatistics &scanStats, SubJobStatus::type &jobStatus, string &jobLogLabel)
{
    SCANNER_TASK_STATUS scanTaskStatus = SCANNER_TASK_STATUS::INPROGRESS;
    jobStatus = SubJobStatus::RUNNING;
    jobLogLabel = "";

    HostScanStatistics preScanStats = scanStats;
    do {
        /* Ensure scanner is ready and start to scan */
        m_scanStatus = m_scanner->GetStatus();
        if (m_scanStatus == SCANNER_STATUS::INIT) {
            Module::SleepFor(std::chrono::seconds(SUBTASK_WAIT_FOR_SCANNER_READY_IN_SEC));
            SendJobReportForAliveness();
            continue;
        }
        ReportScannerRunningStats(preScanStats);
        /* Update <scanTaskStatus, jobStatus, jobLogLabel, jobProgress> based on current scan status <m_scanStatus> */
        FillMonitorScannerVarDetails(scanTaskStatus, jobStatus, jobLogLabel);
        if (scanTaskStatus != SCANNER_TASK_STATUS::INPROGRESS) {
            INFOLOG("scanTaskStatus: %d", static_cast<int>(scanTaskStatus));
            scanStats = m_scanStats;
            break;
        }
        if (IsAbortJob()) {
            INFOLOG("Scanner - Abort is invocked for TaskID: %s, subTaskID: %s", m_jobId.c_str(), m_subJobId.c_str());
            if (SCANNER_STATUS::SUCCESS != m_scanner->Abort()) {
                ERRLOG("Scanner Abort is failed");
            }
        }
        Module::SleepFor(std::chrono::seconds(SLEEP_TEN_SECONDS));
    } while (true);
    ScanStatistics stats = m_scanner->GetStatistics();
    if (m_scanner != nullptr) {
        m_scanner->Destroy();
    }
    return stats;
}

bool HostBackup::ReportScannerRunningStats(const HostScanStatistics &preScanStats)
{
    UpdateScannerStats(preScanStats);
    bool canReport = ShareResourceManager::GetInstance().CanReportStatToPM(m_jobId + "_scan");
    if (canReport) {
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0),
            "file_plugin_host_backup_scan_inprogress_label", INITIAL_ERROR_CODE, to_string(m_scanStats.m_totDirs),
            to_string(m_scanStats.m_totFiles), FormatCapacity(m_scanStats.m_totalSize),
            to_string(m_scanStats.m_totDirsToBackup), to_string(m_scanStats.m_totFilesToBackup),
            FormatCapacity(m_scanStats.m_totalSizeToBackup));
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
    }
    return true;
}

bool HostBackup::SendJobReportForAliveness()
{
    int64_t currTime = PluginUtils::GetCurrentTimeInSeconds();
    if ((currTime - m_lastJobReportTime) > REPORT_INTERVAL) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        m_lastJobReportTime = currTime;
    }
    return true;
}

void HostBackup::FillMonitorScannerVarDetails(SCANNER_TASK_STATUS &scanTaskStatus, SubJobStatus::type &jobStatus,
    string &jobLogLabel)
{
    DBGLOG("m_scanStatus: %d", static_cast<int>(m_scanStatus));
    if (m_scanStatus == SCANNER_STATUS::COMPLETED) {
        HCP_Log(INFO, MODULE) << "Scan completed" << HCPENDLOG;
        jobStatus = SubJobStatus::COMPLETED;
        jobLogLabel = "file_plugin_host_backup_scan_completed_label";
        scanTaskStatus = SCANNER_TASK_STATUS::SUCCESS;
    } else if (m_scanStatus == SCANNER_STATUS::FAILED) {
        HCP_Log(ERR, MODULE) << "Scan failed" << HCPENDLOG;
        jobStatus = SubJobStatus::FAILED;
        jobLogLabel = "file_plugin_host_backup_scan_fail_label";
        scanTaskStatus = SCANNER_TASK_STATUS::FAILED;
    } else if (m_scanStatus == SCANNER_STATUS::ABORT_IN_PROGRESS) {
        HCP_Log(ERR, MODULE) << "Scan abort in progress" << HCPENDLOG;
        jobStatus = SubJobStatus::ABORTING;
        jobLogLabel = "";
    } else if (m_scanStatus == SCANNER_STATUS::ABORTED) {
        HCP_Log(ERR, MODULE) << "Scan aborted" << HCPENDLOG;
        jobStatus = SubJobStatus::ABORTED;
        jobLogLabel = "";
        scanTaskStatus = SCANNER_TASK_STATUS::ABORTED;
    } else if (m_scanStatus == SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE) {
        HCP_Log(ERR, MODULE) << "Scan failed as sec nas server is not reachable" << HCPENDLOG;
        jobStatus = SubJobStatus::FAILED;
        jobLogLabel = "file_plugin_host_backup_scan_fail_label";
        scanTaskStatus = SCANNER_TASK_STATUS::FAILED;
    } else if (m_scanStatus == SCANNER_STATUS::PROTECTED_SERVER_NOT_REACHABLE) {
        HCP_Log(ERR, MODULE) << "Scan failed as protected nas server is not reachable" << HCPENDLOG;
        jobStatus = SubJobStatus::FAILED;
        jobLogLabel = "file_plugin_host_backup_scan_fail_label";
        scanTaskStatus = SCANNER_TASK_STATUS::FAILED;
    }
    return;
}

bool HostBackup::HandleScanCompletion(const std::set<std::string>& jobInfoSet)
{
    DBGLOG("Enter HandleScanCompletion, jobInfoSet size: %d", jobInfoSet.size());
    if (!m_scanRedo) {
        ReportScannerCompleteStatus();
    }
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    m_generalInfo.m_backupCopyPhaseStartTime = 0;
    for (const string jobInfoStr : jobInfoSet) {
        DBGLOG("jobInfoStr: %s", jobInfoStr.c_str());
        if (!CreateSubTasksFromCtrlFile(jobInfoStr, jobStatus)) {
            ERRLOG("Create subtask failed, jobstr: %s, jobId: %s", jobInfoStr.c_str(), m_jobId.c_str());
            return false;
        }
    }
    return true;
}

bool HostBackup::CreateSubTasksFromCtrlFile(const string& jobInfoStr, SubJobStatus::type& jobStatus)
{
    BackupSubJobInfo backupSubJobInfo;
    if (!Module::JsonHelper::JsonStringToStruct(jobInfoStr, backupSubJobInfo)) {
        ERRLOG("backupSubJobInfo JsonStringToStruct failed");
        return false;
    }
    string scanCtrlDir = backupSubJobInfo.scanCtrlDir;
    string backupCtrlDir = backupSubJobInfo.backupCtrlDir;
    string prefix = backupSubJobInfo.prefix;
    vector<string> srcFileList {};
    vector<SubJob> subJobList {};
    vector<string> ctrlFileList {};
    if (!CheckFilePathAndGetSrcFileList(scanCtrlDir, backupCtrlDir, srcFileList)) {
        return false;
    }
    CreateCtrlDirectory(backupSubJobInfo.incControlDir);
    DBGLOG("Enter CreateSubTasksFromCtrlFile, size: %d", srcFileList.size());
    for (const std::string& scanCtrlFile: srcFileList) {
        INFOLOG("process ctrl file: %s", scanCtrlFile.c_str());
        SendJobReportForAliveness();
        if (IsAbortJob()) {
            WARNLOG("Abort is invocked for taskid: %s, subtaskid: %s", m_jobId.c_str(), m_subJobId.c_str());
            jobStatus = SubJobStatus::ABORTED;
            return true;
        }
        if (!IsValidCtrlFile(scanCtrlFile)) {
            WARNLOG("not valid ctrl file: %s", scanCtrlFile.c_str());
            continue;
        }
        string backupCtrlFile = backupCtrlDir + dir_sep + GetFileName(scanCtrlFile);
        string backupCtrlFileInCacheFsPath = backupCtrlFile.substr(m_cacheFsPath.length(), string::npos);
        string lastBackupCtrlFile = backupSubJobInfo.incControlDir + dir_sep + GetFileName(scanCtrlFile);
        DBGLOG("cpFile, src: %s, dst: %s", scanCtrlFile.c_str(), backupCtrlFile.c_str());
        if ((!CopyFile(scanCtrlFile, backupCtrlFile)) || (!CopyFile(scanCtrlFile, lastBackupCtrlFile))) {
            return false;
        }
        if (!GenerateSubJobList(subJobList, ctrlFileList, scanCtrlFile, backupCtrlFileInCacheFsPath, prefix)) {
            return false;
        }
        if (subJobList.size() % NUMBER10 != 0) {
            continue;
        }
        if (!CreateSubTask(subJobList, ctrlFileList)) {
            return false;
        }
    }
    if (!CreateSubTask(subJobList, ctrlFileList)) {
        return false;
    }
    DBGLOG("Exit CreateSubTasksFromCtrlFile Success");
    return true;
}

bool HostBackup::CreateBackupSubJobTask(const uint32_t &subTaskType)
{
    string backupSubJobStr;
    BackupSubJob backupSubJob {};
    SubJob subJob {};
    bool ignoreFailed = false;
    if (subTaskType == SUBJOB_TYPE_CHECK_SUBJOB_PHASE) {
        backupSubJob.subTaskType = SUBJOB_TYPE_CHECK_SUBJOB_PHASE;
        subJob.__set_jobName(SUBJOB_TYPE_CHECK_SUBJOB_JOBNAME);
        subJob.__set_jobPriority(SUBJOB_TYPE_CHECK_SUBJOB_PHASE_PRIO);
        ignoreFailed = true;
    } else if (subTaskType == SUBJOB_TYPE_TEARDOWN_PHASE) {
        backupSubJob.subTaskType = SUBJOB_TYPE_TEARDOWN_PHASE;
        subJob.__set_jobName(SUBJOB_TYPE_TEARDOWN_JOBNAME);
        subJob.__set_jobPriority(SUBJOB_TYPE_TEARDOWN_PHASE_PRIO);
    } else if (subTaskType == SUBJOB_TYPE_COPYMETA_PHASE) {
        backupSubJob.subTaskType = SUBJOB_TYPE_COPYMETA_PHASE;
        subJob.__set_jobName(SUBJOB_TYPE_COPYMETA_JOBNAME);
        subJob.__set_jobPriority(SUBJOB_TYPE_COPYMETA_PHASE_PRIO);
    } else {
        HCP_Log(ERR, MODULE) << "Wrong subjob type: " << subTaskType << HCPENDLOG;
        return false;
    }

    if (!Module::JsonHelper::StructToJsonString(backupSubJob, backupSubJobStr)) {
        HCP_Log(ERR, MODULE) << "Exit CreateBackupJobTeardownTask failed" << HCPENDLOG;
        return false;
    }

    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo(backupSubJobStr);
    subJob.__set_ignoreFailed(ignoreFailed);

    if (!CreateSubTask(subJob)) {
        HCP_Log(ERR, MODULE) << "Exit CreateBackupSubjobTask failed, subjob type: " << subTaskType << HCPENDLOG;
        return false;
    }
    return true;
}

bool HostBackup::GenerateSubJobList(vector<SubJob> &subJobList, vector<string> &ctrlFileList,
    const string &scanCtrlFile, const string &backupCtrlFileInCacheFsPath, const string& prefix)
{
    uint32_t subTaskType = GetSubJobTypeByFileName(backupCtrlFileInCacheFsPath);
    if (IsAggregateSkipeSubJob(subTaskType)) {
        INFOLOG("Backup mode is aggregation, sub task type is: %d, skip without generating subtask", subTaskType);
        return true;
    }
    SubJob subJob {};
    bool ignoreFailed = (m_fileset.m_advParms.m_isContinueOnFailed == FALSE_STR) ? false : true;
    if (!InitSubTask(subJob, backupCtrlFileInCacheFsPath, ignoreFailed, prefix)) {
        HCP_Log(ERR, MODULE) << "Init subtask failed" << HCPENDLOG;
        return false;
    }
    if ((subTaskType == SUBJOB_TYPE_DATACOPY_COPY_PHASE) && (!UpdateCopyPhaseStartTimeInGenRsc())) {
        HCP_Log(ERR, MODULE) << "Updated backup start time failed" << HCPENDLOG;
        return false;
    }
    subJobList.push_back(subJob);
    ctrlFileList.push_back(scanCtrlFile);
    return true;
}

void HostBackup::UpdateJobStartTime()
{
    m_generalInfo.m_jobStartTime = PluginUtils::GetCurrentTimeInSeconds();
}

bool HostBackup::UpdateCopyPhaseStartTimeInGenRsc()
{
    if (m_generalInfo.m_backupCopyPhaseStartTime != 0) {
        DBGLOG("Not first generate backup job, don't need report!");
        return true;
    }
    m_generalInfo.m_backupCopyPhaseStartTime = PluginUtils::GetCurrentTimeInSeconds();
    if (!InitGenerateResource(m_statisticsPath)) {
        return false;
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_host_backup_data_start_label");
    ShareResourceManager::GetInstance().CanReportStatToPM(m_jobId + "_backup");
    return true;
}

bool HostBackup::IsFullBackup() const
{
    return (m_backupJobPtr->jobParam.backupType == AppProtect::BackupJobType::FULL_BACKUP);
}

bool HostBackup::IsSubTaskStatsFileExists() const
{
    boost::system::error_code ec;
    ShareResourceManager::GetInstance().SetResourcePath(m_statisticsPath, m_subJobId);
    std::string filePath = ShareResourceManager::GetInstance().GetFileName(ShareResourceType::BACKUP, m_subJobId);
    DBGLOG("check is subtask stats file exists: %s", filePath.c_str());
    try {
        if (boost::filesystem::exists(filePath, ec)) {
            return true;
        }
    } catch (...) {
        ERRLOG("check subtask stats file exits got exception");
        return false;
    }
    return false;
}

void HostBackup::CloseAggregateSwitchByBackupType()
{
    if ((m_backupJobPtr->jobParam.backupType == AppProtect::BackupJobType::PERMANENT_INCREMENTAL_BACKUP) &&
        (m_fileset.m_advParms.m_isAggregate == TRUE_STR)) {
        WARNLOG("Automatically close the aggregate feature in the case of permanent incremental backup!");
        m_fileset.m_advParms.m_isAggregate = FALSE_STR;
        m_fileset.m_advParms.m_maxSizeAfterAggregate = "0";
        m_fileset.m_advParms.m_maxSizeToAggregate = "0";
    }
}

bool HostBackup::InitJobInfo()
{
    /* Protected fileset details */
    if (!InitFilesetInfo()) {
        return false;
    }
    CloseAggregateSwitchByBackupType();
    /* Data layout details */
    if (!InitDataLayoutInfo()) {
        return false;
    }
    /* MetaFs and BackupFs to be used */
    if (!InitMetaDataCacheBackupFs()) {
        return false;
    }
    /* Config the repository IP and path */
    InitRepoPaths();
    m_lastBackupTime = PluginUtils::GetCurrentTimeInSeconds();
    DBGLOG("InitJobInfo, m_lastBackupTime: %s", ConvertToReadableTime(m_lastBackupTime).c_str());
    ShareResourceManager::GetInstance().SetResourcePath(m_statisticsPath, m_jobId);
    PrintJobInfo();
    return true;
}

bool HostBackup::InitFilesetInfo()
{
    if (m_backupJobPtr->protectSubObject.empty()) {
        ERRLOG("Invalid fileset, fileset is empty");
        return false;
    }
    string notExistPath;
    string notBackupPath;
    for (AppProtect::ApplicationResource resource : m_backupJobPtr->protectSubObject) {
        if (!PluginUtils::IsPathExists(resource.name)) {
            ERRLOG("The selected path: %s is not exit", resource.name.c_str());
            notExistPath += (resource.name + SEP);
            continue;
        }
        if (!IsBackupPath(resource.name)) {
            ERRLOG("The selected path: %s is system path, do not backup", resource.name.c_str());
            notBackupPath += (resource.name + SEP);
            continue;
        }
        m_fileset.m_protectedPaths.emplace(resource.name);
        INFOLOG("protected path push %s", resource.name.c_str());
    }
    if (!notExistPath.empty() && m_jobCtrlPhase == JOB_CTRL_PHASE_PREJOB) {
        size_t pos = notExistPath.size() - LEN_SEP; // 上报的path不带最后一个逗号
        /* path not exists or have no sufficient privelege will both use this label */
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_backup_selected_path_not_accessible_label",
            notExistPath.substr(0, pos));
    }
    if (!notBackupPath.empty() && m_jobCtrlPhase == JOB_CTRL_PHASE_PREJOB) {
        size_t pos = notBackupPath.size() - LEN_SEP; // 上报的path不带最后一个逗号
        /* path not exists or have no sufficient privelege will both use this label */
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_backup_protected_path_invalid_warn_label",
            notBackupPath.substr(0, pos));
    }
    FilesetPathDeduplication();
    if (!Module::JsonHelper::JsonStringToStruct(m_backupJobPtr->protectObject.extendInfo, m_fileset.m_filesetExt)) {
        ERRLOG("JsonStringToStruct failed, m_filesetExt");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_backupJobPtr->extendInfo, m_fileset.m_advParms)) {
        ERRLOG("JsonStringToStruct failed, m_advParms");
        return false;
    }
    if (!FilterProtectPaths()) {
        return false;
    }
    if (m_fileset.m_protectedPaths.empty() && m_jobCtrlPhase == JOB_CTRL_PHASE_PREJOB) {
        ERRLOG("Protected path is empty, init file set info failed!");
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_backup_protected_path_empty_warn_label");
        return false;
    }
    return true;
}

bool HostBackup::FilterProtectPaths()
{
    if (m_jobCtrlPhase == JOB_CTRL_PHASE_PREJOB || m_jobCtrlPhase == JOB_CTRL_PHASE_GENSUBJOB) {
        m_deviceMountPtr = make_shared<DeviceMount>();
        if (m_deviceMountPtr == nullptr) {
            ERRLOG("m_deviceMountPtr is nullptr");
            return false;
        }
        m_deviceMountPtr->LoadDevice();
        std::string notSupportPath;
        std::string notDevicePath;
        for (auto it = m_fileset.m_protectedPaths.begin(); it != m_fileset.m_protectedPaths.end();) {
            shared_ptr<FsDevice> fsDevice = m_deviceMountPtr->FindDevice(*it);
            if (fsDevice == nullptr) {
                DBGLOG("fsDevice is nullptr, continue");
                notDevicePath += (*it + SEP);
                it = m_fileset.m_protectedPaths.erase(it);
                continue;
            }
            if (!IsBackupDevice(fsDevice)) {
                notSupportPath += (*it + SEP);
                it = m_fileset.m_protectedPaths.erase(it);
                WARNLOG("The selected path is designed not to be backed up, path: %s, fsType: %s",
                    fsDevice->mountPoint.c_str(), fsDevice->fsType.c_str());
                continue;
            }
            ++it;
        }
        if (!notSupportPath.empty() && m_jobCtrlPhase == JOB_CTRL_PHASE_PREJOB) {
            size_t posSupport = notSupportPath.size() - LEN_SEP;
            ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_backup_selected_path_not_support_label",
            notSupportPath.substr(0, posSupport));
        }
        if (!notDevicePath.empty() && m_jobCtrlPhase == JOB_CTRL_PHASE_PREJOB) {
            size_t posDevice = notDevicePath.size() - LEN_SEP;
            ReportJobLabel(JobLogLevel::TASK_LOG_WARNING,
            "file_plugin_backup_selected_path_volume_not_accessible_label",
            notDevicePath.substr(0, posDevice));
        }
    }
    return true;
}

void HostBackup::GenerateCopyOsFlagRecord() const
{
    OsIdentifier::CreateSystemNameRecordFile(m_metaFsPath);
    OsIdentifier::CreateSystemVersionRecordFile(m_metaFsPath);
}

bool HostBackup::InitDataLayoutInfo()
{
    if (!Module::JsonHelper::JsonStringToStruct(m_backupJobPtr->extendInfo, m_dataLayoutExt)) {
        HCP_Log(ERR, MODULE) << "Failed to parse protctEnv extendInfo json to struct" << HCPENDLOG;
        return false;
    }
    return true;
}

bool HostBackup::InitMetaDataCacheBackupFs()
{
    for (unsigned int i = 0; i < m_backupJobPtr->repositories.size(); i++) {
        if (m_backupJobPtr->repositories[i].repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheFs = m_backupJobPtr->repositories[i];
        } else if (m_backupJobPtr->repositories[i].repositoryType == RepositoryDataType::DATA_REPOSITORY) {
            m_dataFs = m_backupJobPtr->repositories[i];
            m_localStorageIps = m_dataFs.endpoint.ip;
        } else if (m_backupJobPtr->repositories[i].repositoryType == RepositoryDataType::META_REPOSITORY) {
            m_metaFs = m_backupJobPtr->repositories[i];
        }
    }

    if (m_metaFs.path.empty() || m_dataFs.path.empty() || m_cacheFs.path.empty()) {
        HCP_Log(DEBUG, MODULE) << "Received info is wrong, m_dataFs size: " << m_dataFs.path.size() <<
            ", m_cacheFs.path.size(): " << m_cacheFs.path.size() << ", m_metaFs.path.size(): " <<
            m_metaFs.path.size() << HCPENDLOG;
        return false;
    }
    return true;
}

void HostBackup::InitRepoPaths()
{
    // plugin can use any mounted data path given by agent ,so using first one
    // TO-DO: how to config which one to use
    m_dataFsPath = IsAggregate() ?
        (m_dataFs.path[(m_numberOfSubTask++) % m_dataFs.path.size()] + dir_sep + m_backupJobPtr->copy.id) :
        m_dataFs.path[(m_numberOfSubTask++) % m_dataFs.path.size()];
    HCP_Log(DEBUG, MODULE) << " m_dataFs.remotePath: " << m_dataFs.remotePath << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << " m_dataFsPath: " << m_dataFsPath << HCPENDLOG;

    // plugin can use any mounted cache path given by agent ,so using first one
    m_cacheFsPath = m_cacheFs.path[0];
    m_scanStatusPath = m_cacheFsPath + SCANNER_STAT;
    HCP_Log(DEBUG, MODULE) << " m_cacheFs.remotePath: " << m_cacheFs.remotePath << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << " Before m_cacheFsPath: " << m_cacheFsPath << HCPENDLOG;
    m_cacheFsPath = GetPathName(m_cacheFsPath); // cacahe仓使用jobId外层的，不使用jobId目录
    HCP_Log(DEBUG, MODULE) << " After change m_cacheFsPath: " << m_cacheFsPath << HCPENDLOG;

    m_scanMetaPath = PathJoin(m_cacheFsPath, "backup-job", "scan", "meta");
    m_scanControlPath = PathJoin(m_cacheFsPath, "backup-job", "scan", "ctrl");
    m_backupControlPath = PathJoin(m_cacheFsPath, "backup-job", "backup", "ctrl");
    m_statisticsPath = PathJoin(m_cacheFs.path[0], "statistics", m_backupJobPtr->copy.id);
    m_subVolSnapInfoFilePath = PathJoin(m_cacheFsPath, "SnapInfo", m_jobId + "subVolPathWithSnapInfo.json");
    m_filesetSnapInfoFilePath = PathJoin(m_cacheFsPath, "SnapInfo", m_jobId + "filesetPathWithSnapInfo.json");
    m_filesetSnapIdFilePath = PathJoin(m_cacheFsPath, "SnapInfo", m_jobId + "filesetSnapId.json");
    DBGLOG("m_cacheFsPath: %s", m_cacheFsPath.c_str());
    DBGLOG("m_scanControlPath: %s", m_scanControlPath.c_str());
    DBGLOG("m_backupControlPath: %s", m_backupControlPath.c_str());
    DBGLOG("m_statisticsPath: %s", m_statisticsPath.c_str());
    DBGLOG("m_subVolSnapInfoFilePath: %s", m_subVolSnapInfoFilePath.c_str());
    DBGLOG("m_filesetSnapIdFilePath: %s", m_filesetSnapIdFilePath.c_str());
    DBGLOG("m_filesetSnapInfoFilePath: %s", m_filesetSnapInfoFilePath.c_str());

    // plugin can use any mounted meta path given by agent ,so using first one
    m_metaFsPath = IsAggregate() ? (m_metaFs.path[0] + dir_sep + m_backupJobPtr->copy.id) : m_metaFs.path[0];
    m_backupCopyInfoFilePath = PathJoin(m_metaFs.path[0], BACKUP_COPY_METAFILE);
    HCP_Log(DEBUG, MODULE) << " m_metaFs.remotePath: " << m_metaFs.remotePath << HCPENDLOG;
    HCP_Log(DEBUG, MODULE) << " m_metaFsPath: " << m_metaFsPath << HCPENDLOG;
    return;
}

void HostBackup::PrintJobInfo() const
{
    INFOLOG("jobPhase: %s, jobId: %s, subJobId: %s", m_jobCtrlPhase.c_str(), m_jobId.c_str(), m_subJobId.c_str());
    INFOLOG("backupJobType: %s", (IsFullBackup() ? "FULL" : "INC"));
    INFOLOG("copy.id: %s", m_backupJobPtr->copy.id.c_str());
    for (const string &path : m_fileset.m_protectedPaths) {
        INFOLOG("advPars.m_protectedPaths: %s", path.c_str());
    }
    INFOLOG("advPars.m_filesetExt.m_filters: %s", m_fileset.m_filesetExt.m_filters.c_str());
    INFOLOG("advPars.m_isConsistent: %s", m_fileset.m_advParms.m_isConsistent.c_str());
    INFOLOG("advPars.m_isCrossFileSystem: %s", m_fileset.m_advParms.m_isCrossFileSystem.c_str());
    INFOLOG("advPars.m_isBackupNfs: %s", m_fileset.m_advParms.m_isBackupNfs.c_str());
    INFOLOG("advPars.m_isBackupSMB: %s", m_fileset.m_advParms.m_isBackupSMB.c_str());
    INFOLOG("advPars.m_isSparseFileDetection: %s", m_fileset.m_advParms.m_isSparseFileDetection.c_str());
    INFOLOG("advPars.m_isContinueOnFailed: %s", m_fileset.m_advParms.m_isContinueOnFailed.c_str());
    INFOLOG("advPars.m_isAggregate: %s", m_fileset.m_advParms.m_isAggregate.c_str());
    INFOLOG("advPars.m_maxSizeAfterAggregate: %s", m_fileset.m_advParms.m_maxSizeAfterAggregate.c_str());
    INFOLOG("advPars.m_maxSizeToAggregate: %s", m_fileset.m_advParms.m_maxSizeToAggregate.c_str());
    INFOLOG("advPars.m_channels: %s", m_fileset.m_advParms.m_channels.c_str());
    INFOLOG("dataLayout.backupFormat: %s", m_dataLayoutExt.m_backupFormat.c_str());
    INFOLOG("dataLayout.m_fileReplaceStrategy: %s", m_dataLayoutExt.m_fileReplaceStrategy.c_str());
    INFOLOG("dataLayout.metadataBackupType: %s", m_dataLayoutExt.m_metadataBackupType.c_str());
    DBGLOG("qos.bandwidth: %d", m_backupJobPtr->jobParam.qos.bandwidth);
    DBGLOG("qos.protectIops: %d", m_backupJobPtr->jobParam.qos.protectIops);
    DBGLOG("qos.backupIops: %d", m_backupJobPtr->jobParam.qos.backupIops);
    DBGLOG("jobParam.BackupType: %d", m_backupJobPtr->jobParam.backupType);
    for (AppProtect::ResourceFilter filter : m_backupJobPtr->jobParam.filters) {
        INFOLOG("jobParam.filter, filterBy: %s, type: %s, rule: %s, mode: %s",
            filter.filterBy.c_str(), filter.type.c_str(), filter.rule.c_str(), filter.mode.c_str());
        for (const string& value : filter.values) {
            INFOLOG("jobParam.filter, value: %s", value.c_str());
        }
    }
}

bool HostBackup::SetupCacheFsForBackupJob() const
{
    /**
     * /<m_cacheFsPath.path>: m_cacheFsPath fs path passed by DME_UBC for NAS Plugin to save metadata's
     *
     * Create folders,
     * | -- CacheFsPath             // CacheFsPath from UBC
     *    | -- backup-job           // For backup job
     *       | -- scan              // Info saved by SCAN module
     *          | -- meta           // Meta info
     *             | -- previous    // Meta info (metafile, dcache, fcache) of the previous scan
     *             | -- latest      // Meta info (metafile, dcache, fcache) of the current scan
     *          | -- ctrl           // Control info (this in input to BACKUP module)
     *       | -- backup            // Info saved by BACKUP module (TO-DO: We will remove this)
     *          | -- ctrl           // Control info (Get from scan modeule)
     *    | -- statistics           // Folder to save the statistic of backup main job and sub-jobs
     */
    CreateDirectory(m_scanMetaPath);
    CreateDirectory(m_scanControlPath);
    CreateDirectory(m_backupControlPath);
    CreateDirectory(m_metaFsPath);
    CreateDirectory(m_dataFsPath);
    CreateDirectory(m_failureRecordRoot);
    return true;
}

std::shared_ptr<SnapshotProvider> HostBackup::BuildSnapshotProvider() const {
#ifdef WIN32
    shared_ptr<VssSnapshotProvider> snapPtr = make_shared<VssSnapshotProvider>(
        m_deviceMountPtr, m_jobId, LoadSnapshotParentPath());
#elif defined(_AIX)
    shared_ptr<JfsSnapshotProvider> snapPtr = make_shared<JfsSnapshotProvider>(
        m_deviceMountPtr, m_jobId, LoadSnapshotParentPath());
#elif defined(SOLARIS)
    shared_ptr<ZfsSnapshotProvider> snapPtr = make_shared<ZfsSnapshotProvider>(m_deviceMountPtr, m_jobId);
#else
    shared_ptr<LvmSnapshotProvider> snapPtr = make_shared<LvmSnapshotProvider>(
        m_deviceMountPtr, m_jobId, LoadSnapshotParentPath());
#endif
    try {
        return std::dynamic_pointer_cast<SnapshotProvider>(snapPtr);
    } catch (const exception& e) {
        ERRLOG("dynamic cast error, %s", e.what());
    }
    return nullptr;
}

bool HostBackup::CreateSnapshot()
{
    if (m_fileset.m_advParms.m_isConsistent == FALSE_STR) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS50));
        return true;
    }
    string notSupports;
#ifdef WIN32
    m_deviceMountPtr->LoadDevice();
#endif
    std::shared_ptr<SnapshotProvider> snapPtr = BuildSnapshotProvider();
    if (snapPtr == nullptr) {
        ERRLOG("invalid snapshot provider!");
        return false;
    }
    
    SnapshotResult snapshotResult{};
    bool isCross = (m_fileset.m_advParms.m_isCrossFileSystem == TRUE_STR);
    string snapshotNames;
    set<string> spaceless;
    set<string> mountedPaths;
    for (string path : m_fileset.m_protectedPaths) {
        snapshotResult = snapPtr->CreateSnapshot(path, isCross);
        if (snapshotResult.snapShotStatus == SNAPSHOT_STATUS::FAILED) {
            ReportJobLabel(
                JobLogLevel::TASK_LOG_ERROR, "file_plugin_host_backup_prepare_create_snap_failed_label", path);
            return false;
        }
        if (snapshotResult.snapShotStatus == SNAPSHOT_STATUS::UNSUPPORTED) {
            notSupports += (path + SEP);
            WARNLOG("The selected path: %s doesn't support snapshot", path.c_str());
            continue;
        }
        if (!snapshotResult.spacelessVgs.empty()) {
            for (const string& vg : snapshotResult.spacelessVgs) {
                spaceless.emplace(vg);
            }
        }
        if (snapshotResult.snapshotVolumeMapper.empty()) {
            DBGLOG("Rnapshot result map is empty!");
            continue;
        }
        if (!MountSnapshot(path, snapPtr, snapshotResult, snapshotNames, mountedPaths)) {
            return false;
        }
    }
    ReportSnapshotResult(spaceless, snapshotNames, notSupports);
    return true;
}

void HostBackup::ReportSnapshotResult(const set<string>& spaceless, const string& snapNames, const string& notSupports)
{
    if (!spaceless.empty()) {
        string vgStr;
        for (const string& vg : spaceless) {
            vgStr += (vg + SEP);
        }
        size_t pos = vgStr.size() - LEN_SEP; // 上报的path不带最后一个逗号
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING,
            "file_plugin_host_backup_prepare_create_snap_space_not_enough_warn_label", vgStr.substr(0, pos));
        Remove(m_filesetSnapIdFilePath);
        Remove(m_subVolSnapInfoFilePath);
    }
    if (!notSupports.empty()) {
        size_t pos = notSupports.size() - LEN_SEP; // 上报的path不带最后一个逗号
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_host_backup_prepare_donot_support_snap_label",
            notSupports.substr(0, pos));
    }
    INFOLOG("snapNames: %s", snapNames.c_str());
    if (!snapNames.empty()) {
        size_t pos = snapNames.size() - LEN_SEP; // 上报的path不带最后一个逗号
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_host_backup_prepare_create_snap_succeed_label",
            snapNames.substr(0, pos));
        WriteSnapInfoToFile(m_filesetPathsWithSnap, m_filesetSnapInfoFilePath);
        WriteSnapInfoToFile(m_subVolPathWithSnap, m_subVolSnapInfoFilePath);
        WriteSnapInfoToFile(m_allSnapVolumeInfos, m_filesetSnapIdFilePath);
        RecordResidualSnapshots(m_allSnapVolumeInfos);
    }
}

bool HostBackup::MountSnapshot(
    const string& path,
    shared_ptr<FilePlugin::SnapshotProvider> shotProviderPtr,
    const SnapshotResult& snapshotResult,
    string& snapshotNames,
    set<string>& mountedPaths)
{
    DBGLOG("Enter MountSnapshot");
    std::shared_ptr<FsDevice> pFsDevice = m_deviceMountPtr->FindDevice(path);
    if (pFsDevice == nullptr) {
        return false;
    }
    string volPath = pFsDevice->mountPoint;
    DBGLOG("MountSnapshot, path %s, mountpoint:%s", path.c_str(), volPath.c_str());
    if (snapshotResult.snapshotVolumeMapper.count(volPath)) {
        if (!mountedPaths.count(volPath)) {
            mountedPaths.emplace(volPath);
            std::string snapVolDevice = snapshotResult.snapshotVolumeMapper.at(volPath);
#if defined(__linux__) || defined(_AIX)
            std::string snapMntPath = snapshotResult.snapshotsMapper.at(volPath);
            CreateDirectory(snapMntPath);
            bool ret = shotProviderPtr->MountSnapshot(snapVolDevice, snapMntPath);
            if (!ret) {
                ERRLOG("MountSnapshot failed, device: %s, mountpoint: %s", snapVolDevice.c_str(), snapMntPath.c_str());
                ReportJobLabel(
                    JobLogLevel::TASK_LOG_ERROR, "file_plugin_host_backup_prepare_create_snap_failed_label", path);
                return false;
            }
#endif
            snapshotNames += snapVolDevice + SEP;
            DBGLOG("snapshotNames:%s", snapshotNames.c_str());
            string snapInfo;
            /*
             * linux/solaris 系统snapVolumeInfo为lvm卷名称  linux: volumeGroup/volumeName, solaris: zpool/fs;
             * aix/windows系统snapVolumeInfo为快照卷ID
             */
#if defined(WIN32) || defined(_AIX)
            snapInfo = snapshotResult.deviceVolumeSnapMap.at(volPath);
#else
            snapInfo = snapshotResult.snapshotVolumeMapper.at(volPath);
#endif
            INFOLOG("Push snapshot volume to m_allSnapVolumeInfos, snapshot volume name: %s", snapInfo.c_str());
            m_allSnapVolumeInfos.emplace(snapInfo);
        }
        DBGLOG("Push fileset path to m_filesetPathsWithSnap, path: %s", path.c_str());
        m_filesetPathsWithSnap.emplace(path);
    }
    return MountSnapshotForSubVolumes(path, shotProviderPtr, snapshotResult, snapshotNames, mountedPaths);
}

bool HostBackup::MountSnapshotForSubVolumes(
    const string& path,
    shared_ptr<FilePlugin::SnapshotProvider> shotProviderPtr,
    const SnapshotResult& snapshotResult,
    string& snapshotNames,
    set<string>& mountedPaths)
{
    std::vector<shared_ptr<FsDevice>> subVolumes;
    if (!m_deviceMountPtr->GetSubVolumes(path, subVolumes)) {
        return true;
    }
    sort(subVolumes.begin(), subVolumes.end(),
        [](const shared_ptr<FsDevice> fsDevPtr1, const shared_ptr<FsDevice> fsDevPtr2) {
        return (fsDevPtr1->mountPoint) < (fsDevPtr2->mountPoint);
    });
    for (const auto& volume : subVolumes) {
        DBGLOG("sub module path: %s", volume->mountPoint.c_str());
    }
    for (const auto& volume : subVolumes) {
        string subVolPath = volume->mountPoint;
        if (!snapshotResult.snapshotVolumeMapper.count(subVolPath)) {
            continue;
        }
        string subSnapVolDevice = snapshotResult.snapshotVolumeMapper.at(subVolPath);
        m_subVolPathWithSnap.emplace(subVolPath);
        if (mountedPaths.count(subVolPath)) {
            continue;
        }
        mountedPaths.emplace(subVolPath);
#if defined(__linux__) || defined(_AIX)
        string subSnapMntPath = snapshotResult.snapshotsMapper.at(subVolPath);
        CreateDirectory(subSnapMntPath);
        bool ret = shotProviderPtr->MountSnapshot(subSnapVolDevice, subSnapMntPath);
        if (!ret) {
            ERRLOG("MountSnapshot failed dev: %s, mntpnt: %s", subSnapVolDevice.c_str(), subSnapMntPath.c_str());
            ReportJobLabel(
                JobLogLevel::TASK_LOG_ERROR, "file_plugin_host_backup_prepare_create_snap_failed_label", path);
            return false;
        }
#endif
        size_t sepPos = subSnapVolDevice.rfind(dir_sep);
#ifndef WIN32
        snapshotNames += subSnapVolDevice + SEP;
#else
        snapshotNames += subSnapVolDevice.substr(sepPos + 1) + SEP;
#endif
        string snapInfo;
        /*
            * linux/solaris 系统snapVolumeInfo为lvm卷名称  linux: volumeGroup/volumeName, solaris: zpool/fs;
            * aix/windows系统snapVolumeInfo为快照卷ID
            */
#if defined(WIN32) || defined(_AIX)
        snapInfo = snapshotResult.deviceVolumeSnapMap.at(subVolPath);
#else
        snapInfo = snapshotResult.snapshotVolumeMapper.at(subVolPath);
#endif
        m_allSnapVolumeInfos.emplace(snapInfo);
    }
    return true;
}

bool HostBackup::GetPrevBackupCopyInfo()
{
    if (IsFullBackup()) {
        return true;
    }

    if (!JsonFileTool::ReadFromFile(m_backupCopyInfoFilePath, m_prevBackupCopyInfo)) {
        HCP_Log(ERR, MODULE) << "Read Backup Copy meta info from file failed" << HCPENDLOG;
        return false;
    }
    return true;
}

void HostBackup::FillBackupCopyInfo(HostBackupCopy &HostBackupCopy)
{
    HostBackupCopy.m_backupFormat = m_dataLayoutExt.m_backupFormat;
    HostBackupCopy.m_metadataBackupType = m_dataLayoutExt.m_metadataBackupType;
    HostBackupCopy.m_backupFilter = m_fileset.m_filesetExt.m_filters;
    HostBackupCopy.m_isConsistent = m_fileset.m_advParms.m_isConsistent;
    HostBackupCopy.m_lastBackupTime = m_lastBackupTime;
    DBGLOG("FillBackupCopyInfo, m_lastBackupTime: %s", ConvertToReadableTime(m_lastBackupTime).c_str());
    return;
}

bool HostBackup::GetBackupJobInfo()
{
    m_backupJobPtr = dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (m_backupJobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "Failed to get backupJobPtr." << HCPENDLOG;
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0));
        return false;
    }
    SetMainJobId(m_backupJobPtr->jobId);
    SetSubJobId();
    return true;
}

void HostBackup::GetExcludeSubPath(const string& path, set<string>& excludePaths)
{
    if (m_deviceMountPtr == nullptr) {
        ERRLOG("GetExcludeSubPath failed, m_deviceMountPtr is nullptr");
        return;
    }
    vector<shared_ptr<FsDevice>> subVolumes;
    if (!m_deviceMountPtr->GetSubVolumes(path, subVolumes)) {
        DBGLOG("path: %s has no sub volume", path.c_str());
        return;
    }
    vector<shared_ptr<FsDevice>> exculdeSubVolumes;
    if (m_fileset.m_advParms.m_isCrossFileSystem == TRUE_STR) {
        DBGLOG("Backup cross file system is [true], backup all supported file systems");
        for (auto volumesPtr : subVolumes) {
            DBGLOG("volumesPtr mount point: %s", volumesPtr->mountPoint.c_str());
            if (!IsBackupDevice(volumesPtr)) {
                exculdeSubVolumes.push_back(volumesPtr);
            }
        }
    } else {
        DBGLOG("Backup cross file system is [false], all sub file systems will not backup");
        exculdeSubVolumes = subVolumes;
    }
    if (!exculdeSubVolumes.empty()) {
        for (auto volumesPtr : exculdeSubVolumes) {
            DBGLOG("Path: %s, sub volume path: %s, type: %s",
                path.c_str(), volumesPtr->mountPoint.c_str(), volumesPtr->fsType.c_str());
            excludePaths.emplace(volumesPtr->mountPoint);
        }
    }
    return;
}

bool HostBackup::IsBackupDevice(shared_ptr<FsDevice> fsDevicePtr)
{
    if (fsDevicePtr == nullptr) {
        ERRLOG("fsDevicePtr is nullptr");
        return false;
    }
    std::string mntPath = fsDevicePtr->mountPoint;
#ifdef WIN32
    mntPath = PluginUtils::LowerCase(mntPath);
#endif
    INFOLOG("Enter Is Backup Device, mnt path: %s", mntPath.c_str());
    for (const string path : m_excludePathList) {
        if (mntPath == path) {
            WARNLOG("%s is in exclude path list", mntPath.c_str());
            return false;
        } else if (mntPath.size() > path.size()) {
            bool ret = (mntPath.substr(0, path.size() + 1) == (path + dir_sep));
            if (ret) {
                WARNLOG("%s is in exclude path list", mntPath.c_str());
                return false;
            }
        }
    }

    std::vector<std::string> excludeFilesystemList = LoadExcludeFileSystemList();
    if (std::count(excludeFilesystemList.begin(), excludeFilesystemList.end(), fsDevicePtr->fsType) > 0) {
        WARNLOG("%s is in exclude filesystem list", fsDevicePtr->fsType.c_str());
        return false;
    }

#ifdef WIN32
    if (fsDevicePtr->fsType == "CIFS" && m_fileset.m_advParms.m_isBackupSMB == FALSE_STR) {
        DBGLOG("isBackupSMB is [false], path: %s is CIFS fileset, don't backup", mntPath.c_str());
        return false;
    }
#else
    if (fsDevicePtr->fsType.substr(0, NUMBER3) == "nfs" && m_fileset.m_advParms.m_isBackupNfs == FALSE_STR) {
        DBGLOG("isBackupNfs is [false], path: %s is nfs fileset, don't backup", mntPath.c_str());
        return false;
    }
#endif
    return true;
}

bool HostBackup::IsBackupPath(std::string path) {
#ifdef WIN32
    path = PluginUtils::LowerCase(path);
#endif
    INFOLOG("Enter Is Backup path, path: %s", path.c_str());
    for (const string excludePath : m_excludePathList) {
        if (path == excludePath) {
            WARNLOG("%s is in exclude path list", path.c_str());
            return false;
        } else if (path.size() > excludePath.size()) {
            bool ret = (path.substr(0, excludePath.size() + 1) == (excludePath + "/"));
            if (ret) {
                WARNLOG("%s is in exclude path list", path.c_str());
                return false;
            }
        }
    }

    std::string pathName = PluginUtils::GetFileNameOfPath(path);
    if (std::find(SCAN_SKIP_DIRS.begin(), SCAN_SKIP_DIRS.end(), pathName) != SCAN_SKIP_DIRS.end()) {
        WARNLOG("%s is in scan skip dir vector", path.c_str());
        return false;
    }
    return true;
}

void HostBackup::DeleteSnapshot()
{
    if (m_fileset.m_advParms.m_isConsistent == FALSE_STR) {
        DBGLOG("Consistent backup is [false], don't need delete snapshot!");
        return;
    }
    ReadSnapInfoFromFile(m_allSnapVolumeInfos, m_filesetSnapIdFilePath);
    for (const string& vol : m_allSnapVolumeInfos) {
        INFOLOG("DeleteSnapshot for snapshot volume: %s", vol.c_str());
    }
    std::shared_ptr<SnapshotProvider> snapshotPtr = BuildSnapshotProvider();
    if (snapshotPtr == nullptr) {
        ERRLOG("invalid snapshot provider!");
        return;
    }
    SnapshotDeleteResult delRet = snapshotPtr->DeleteAllSnapshots(m_allSnapVolumeInfos);
    // Save the snapshot information that fails to be deleted to a file and delete it in the next backup task.
    if (!delRet.status) {
        ERRLOG("Delete snapshot failed, jobid: %s", m_jobId.c_str());
        SendAlarmForResidualSnapshots(delRet.snapshots);
    }
    UpdateResidualSnapshots(delRet.snapshots);
    string snapInfoPath = m_cacheFsPath + dir_sep + "SnapInfo";
    if (IsDirExist(snapInfoPath)) {
        Remove(snapInfoPath);
    }
}

void HostBackup::SendAlarmForResidualSnapshots(const set<string>& snapshotInfos)
{
    // report residule snapshot infos to pm
    string msg;
    for (const string& info : snapshotInfos) {
        msg += ("[snapshot ID: " + info + "]");
    }
    INFOLOG("Report residule snapshot infos: %s, jobId: %s", msg.c_str(), m_jobId.c_str());
    ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_host_backup_post_delete_snap_failed_label", msg);
    // send alarm for residule snapshot infos
    ActionResult result;
    AppProtect::AlarmDetails alarm;
    alarm.alarmId = ALARM_CODE_FAILED_DELETE_SNAPSHOT;
    alarm.parameter = m_backupJobPtr->protectObject.type + "," + GetJobId();
    JobService::SendAlarm(result, alarm);
}

bool HostBackup::RecordResidualSnapshots(const set<string>& snapshotInfos)
{
    HostSnapResidualInfo snapResidualInfo {};
    snapResidualInfo.jobId = m_jobId;
    snapResidualInfo.snapshotInfos.assign(snapshotInfos.begin(), snapshotInfos.end());
    string snapResidualFile = PathJoin(m_cacheFsPath, RESIDUAL_SNAPSHORTS_INFO_FILE);
    INFOLOG("RecordResidualSnapshots, snapResidualFile: %s, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
    if (IsFileExist(snapResidualFile) && !JsonFileTool::ReadFromFile(snapResidualFile, m_snapResidualInfos)) {
        ERRLOG("Read snap residual infos from file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return false;
    }
    m_snapResidualInfos.infos.push_back(snapResidualInfo);
    if (!JsonFileTool::WriteToFile(m_snapResidualInfos, snapResidualFile)) {
        ERRLOG("Write snap residual infos to file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return false;
    }
    return true;
}

bool HostBackup::UpdateResidualSnapshots(const set<string>& snapshotInfos)
{
    string snapResidualFile = PathJoin(m_cacheFsPath, RESIDUAL_SNAPSHORTS_INFO_FILE);
    INFOLOG("UpdateResidualSnapshots, snapResidualFile: %s, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
    if (IsFileExist(snapResidualFile) && !JsonFileTool::ReadFromFile(snapResidualFile, m_snapResidualInfos)) {
        ERRLOG("Read snap residual infos from file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return false;
    }
    HostSnapResidualInfoList updateResidualInfos;
    updateResidualInfos.infos.reserve(m_snapResidualInfos.infos.size());
    for (auto& info : m_snapResidualInfos.infos) {
        if (info.jobId == m_jobId) {
            info.snapshotInfos.clear();
            info.snapshotInfos.assign(snapshotInfos.begin(), snapshotInfos.end());
            if (info.snapshotInfos.empty()) {
                continue;
            }
        }
        updateResidualInfos.infos.push_back(info);
    }
    if (updateResidualInfos.infos.empty() && IsFileExist(snapResidualFile)) {
        DBGLOG("m_snapResidualInfos is empty, remove snapshot residual file. jobId: %s", m_jobId.c_str());
        Remove(snapResidualFile);
        return true;
    }
    if (!JsonFileTool::WriteToFile(updateResidualInfos, snapResidualFile)) {
        ERRLOG("Write snap residual infos to file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return false;
    }
    return true;
}

void HostBackup::RemoveSnapDir(const std::string& jobId)
{
#ifdef _WIN32
    std::string snapDirPath = PluginUtils::PathJoin(VSS_MNT_ROOT, jobId);
    INFOLOG("remove snapdir %s", snapDirPath.c_str());
    Remove(snapDirPath);
#endif
}

void HostBackup::ClearResidualSnapshotsAndAlarm()
{
    string snapResidualFile = PathJoin(m_cacheFsPath, RESIDUAL_SNAPSHORTS_INFO_FILE);
    if (!IsFileExist(snapResidualFile)) {
        INFOLOG("No residual snapshot exists, return directly! jobId: %s", m_jobId.c_str());
        return;
    }
    if (!JsonFileTool::ReadFromFile(snapResidualFile, m_snapResidualInfos)) {
        ERRLOG("Read snap residual infos from file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return;
    }
    if (m_snapResidualInfos.infos.empty()) {
        INFOLOG("m_snapResidualInfos is empty, return directly! jobId: %s", m_jobId.c_str());
        return;
    }
    HostSnapResidualInfoList newResidualInfos;
    for (const auto& info : m_snapResidualInfos.infos) {
        SnapshotDeleteResult delRet {};
        set<string> snaps(info.snapshotInfos.begin(), info.snapshotInfos.end());
        std::shared_ptr<SnapshotProvider> snapshotPtr = BuildSnapshotProvider();
        if (snapshotPtr == nullptr) {
            ERRLOG("invalid snapshot provider!");
            return;
        }
        delRet = snapshotPtr->DeleteAllSnapshots(snaps);
        RemoveSnapDir(info.jobId);
        if (delRet.status) {
            ActionResult result;
            AppProtect::AlarmDetails alarm;
            alarm.alarmId = ALARM_CODE_FAILED_DELETE_SNAPSHOT;
            alarm.parameter = m_backupJobPtr->protectObject.type + "," + info.jobId;
            JobService::ClearAlarm(result, alarm);
        } else {
            // 清理失败, 更新失败快照信息
            HostSnapResidualInfo snapResidualInfo {};
            snapResidualInfo.jobId = info.jobId;
            snapResidualInfo.snapshotInfos.assign(delRet.snapshots.begin(), delRet.snapshots.end());
            newResidualInfos.infos.push_back(snapResidualInfo);
        }
    }
    if (newResidualInfos.infos.empty()) {
        // 残留快照都被删除，删除残留快照信息文件
        Remove(snapResidualFile);
    } else if (!JsonFileTool::WriteToFile(newResidualInfos, snapResidualFile)) {
        // 残留快照未被完全删除，更新残留快照文件
        ERRLOG("Write snap residual infos to file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
    }
    return;
}

void HostBackup::LoadExcludePathList()
{
    const char comma = ',';
    const char space = ' ';
    const char* ws = " \t\n\r\f\v";
    std::string excludePathStr = Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, EXCLUDE_PATH_LIST_KEY);
    INFOLOG("using ExcludePathList key: %s, value = %s", EXCLUDE_PATH_LIST_KEY.c_str(), excludePathStr.c_str());
    std::replace(excludePathStr.begin(), excludePathStr.end(), comma, space);
    std::stringstream ss(excludePathStr);
    std::string path;
    while (ss >> path) {
        // trim here
        path.erase(path.find_last_not_of(ws) + 1);
        path.erase(0, path.find_first_not_of(ws));
        DBGLOG("detected exclude path: [%s]", path.c_str());
#ifdef WIN32
        path = PluginUtils::LowerCase(path);
#endif
        m_excludePathList.emplace_back(path);
    }
    // Push agent install path to excludePathList
    std::string agentInstallPath = Module::EnvVarManager::GetInstance()->GetPluginInstallPath();
    INFOLOG("Push agent install path: %s to exclude path list", agentInstallPath.c_str());
    m_excludePathList.emplace_back(agentInstallPath);
#ifdef WIN32
    std::string winSysVolInfoPath = R"(c:\system volume information)";
    m_excludePathList.emplace_back(winSysVolInfoPath);
#endif
}

std::vector<std::string> HostBackup::LoadExcludeFileSystemList() const
{
    const char comma = ',';
    const char space = ' ';
    const char* ws = " \t\n\r\f\v";
    std::vector<std::string> excludeFilesystemList{};
    std::string excludeFilesystemStr = Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, EXCLUDE_FILESYSTEM_LIST_KEY);
    INFOLOG("using EXCLUDE_FILESYSTEM_LIST_KEY key: %s, value = %s",
        EXCLUDE_FILESYSTEM_LIST_KEY.c_str(), excludeFilesystemStr.c_str());
    std::replace(excludeFilesystemStr.begin(), excludeFilesystemStr.end(), comma, space);
    std::stringstream ss(excludeFilesystemStr);
    std::string path;
    while (ss >> path) {
        // trim here
        path.erase(path.find_last_not_of(ws) + 1);
        path.erase(0, path.find_first_not_of(ws));
        DBGLOG("detected exclude filesystem: [%s]", path.c_str());
        excludeFilesystemList.emplace_back(path);
    }
    return excludeFilesystemList;
}

std::string HostBackup::LoadSnapshotParentPath() const
{
    std::string snapshotParentPath = Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, SNAPSHOT_PARENT_PATH_KEY);
    INFOLOG("using SnapshotParentPath key: %s, value = %s",
        SNAPSHOT_PARENT_PATH_KEY.c_str(), snapshotParentPath.c_str());
    if (snapshotParentPath.empty()) {
        WARNLOG("read empty SnapshotParentPath, use default: %s", SNAPSHOT_PARENT_PATH_DEFAULT.c_str());
        return SNAPSHOT_PARENT_PATH_DEFAULT;
    }
    return snapshotParentPath;
}

bool HostBackup::SetNumOfChannels() const
{
    std::string path = PluginUtils::PathJoin(Module::CPath::GetInstance().GetRootPath(), PLUGIN_ATT_JSON);
    DBGLOG("SetNumOfChannels, plugin_attribute_1.0.0.json path: %s", path.c_str());
    if (!PluginUtils::IsFileExist(path)) {
        ERRLOG("plugin_attribute_1.0.0.json is not exist!");
    }
    try {
        std::ifstream readStream(path, std::ios::in);
        if (!readStream.is_open()) {
            ERRLOG("Open file %s failed, errno[%d]:%s.", path.c_str(), errno, strerror(errno));
            return false;
        }
        std::stringstream buffer;
        buffer << readStream.rdbuf();
        readStream.close();
        std::string pluginAttributeContent(buffer.str());
        Json::Value jsonVal;
        if (!Module::JsonHelper::JsonStringToJsonValue(pluginAttributeContent, jsonVal)) {
            ERRLOG("JsonStringToJsonValue error, str: %s", pluginAttributeContent.c_str());
            return false;
        }
        int channels = std::stoi(m_fileset.m_advParms.m_channels);
        INFOLOG("Current maxSubCount: %d, set to %d",
            jsonVal["application_sub_job_cnt_max"]["Fileset"].asInt(), channels);
        jsonVal["application_sub_job_cnt_max"]["Fileset"] = Json::Value(channels);
        Json::StyledWriter sWriter;
        std::ofstream outFileStream(path, std::ios::out | std::ios::trunc);
        if (!outFileStream.is_open()) {
            ERRLOG("Open file %s failed, errno[%d]:%s.", path.c_str(), errno, strerror(errno));
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

bool HostBackup::WriteSnapInfoToFile(const set<string>& snapshotInfo, const string& infoFilePath)
{
    DBGLOG("Enter WriteSnapInfoToFile");
    if (snapshotInfo.empty()) {
        INFOLOG("snapshotInfo is empty");
        return true;
    }
    if (infoFilePath.empty()) {
        ERRLOG("m_filesetSnapInfoFilePath path is empty");
        return false;
    }
    CreateDirectory(GetPathName(infoFilePath));
    ofstream file(infoFilePath, ios::out | ios::trunc);
    if (!file.is_open()) {
        ERRLOG("WriteSnapshotInfoToFile open file failed, path: %s", infoFilePath.c_str());
        return false;
    }
    ostream_iterator<string> iter(file, "\n");
    copy(snapshotInfo.begin(), snapshotInfo.end(), iter);
    for (string path : snapshotInfo) {
        DBGLOG("snapshot info path: %s has been writen to file", path.c_str());
    }
    return true;
}

void HostBackup::ReadSnapInfoFromFile(set<string>& snapshotInfo, const string& infoFilePath)
{
    if (!IsFileExist(infoFilePath)) {
        return;
    }
    DBGLOG("Enter ReadSnapInfoFromFile");
    ifstream file(infoFilePath);
    while (file) {
        string line;
        getline(file, line);
        if (line.empty()) {
            continue;
        }
        snapshotInfo.emplace(line);
        DBGLOG("Path writed to snapshotInfo: %s", line.c_str());
    }
}

void HostBackup::FilesetPathDeduplication()
{
    if (m_fileset.m_protectedPaths.empty()) {
        return;
    }
    set<string> rets;
    auto prefix = *(m_fileset.m_protectedPaths.begin());
    rets.emplace(prefix);
    for (auto ptr = ++(m_fileset.m_protectedPaths.begin()); ptr != m_fileset.m_protectedPaths.end(); ptr++) {
        if ((ptr->size() > prefix.size() && !ptr->compare(0, prefix.size() + 1, prefix + dir_sep))) {
            continue;
        } else {
            rets.emplace(*ptr);
        }
    }
    m_fileset.m_protectedPaths = rets;
}

bool HostBackup::IsAggregateSkipeSubJob(uint32_t subTaskType)
{
    if (!IsAggregate()) {
        return false;
    }
    if (subTaskType == SUBJOB_TYPE_DATACOPY_DELETE_PHASE) {
        return true;
    }
    return false;
}

int HostBackup::HandleAggregatedDirPhase(const string& controlFile)
{
    string metaFilePath = m_scanMetaPath + dir_sep + GetParentDirName(controlFile) + LATEST;
    string controlFilePath = m_cacheFsPath + controlFile;
    m_subBackupStats.noOfDirCopied = ReadDirCountForMtimeStats(controlFilePath, metaFilePath);
    if (!InitSubBackupJobResources()) {
        ERRLOG("InitSubBackupJobResources failed");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

bool HostBackup::CreateCtrlDirectory(const std::string &path)
{
    if (IsDirExist(path)) {
        Remove(path);
    }
    if (!CreateDirectory(path)) {
        return false;
    }
    return true;
}

void HostBackup::UpdateTaskInfo()
{
    INFOLOG("==== UpdateTaskInfo thread start ====");
    while (m_isBackupInProgress) {
        if (m_backup != nullptr) {
            UpdateSubBackupStats(false);
            BackupStatistic mainStats;
            if (UpdateMainBackupStats(mainStats)) {
                ReportBackupRunningStats(mainStats);
            }
            std::this_thread::sleep_for(std::chrono::seconds(NUMBER10));
        }
        std::this_thread::sleep_for(std::chrono::seconds(NUMBER1));
    }
    INFOLOG("==== UpdateTaskInfo thread join ====");
}

void HostBackup::KeepJobAlive()
{
    INFOLOG("Start keep job alive for job(%s).", m_jobId.c_str());
    int count = 0;
    while (true) {
        if (m_JobComplete) {
            INFOLOG("jobKeepAlive thread shut down.");
            break;
        }
        ++count;
        // 使用count加sleep记时，120s上报一次
        if (count == NUMBER1200) {
            ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
            count = 0;
        }
        Module::SleepFor(std::chrono::milliseconds(100)); // sleep 0.1s, 线程能更快被join退出
    }
}
}