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
#include "HostArchiveRestore.h"
#include <vector>
#include "system/System.hpp"
#include "ClientInvoke.h"
#include "constant/PluginConstants.h"
#include "PluginUtilities.h"
#include "ScanMgr.h"
#include "common/EnvVarManager.h"
#include "common/Thread.h"
#include "PluginUtilities.h"

using namespace std;
namespace FilePlugin {
namespace {
const auto MODULE = "HostArchiveRestore";

const int SUCCESS = 0;
const int FAILED = -1;

constexpr int PREPARE_ARCHIVE_CLIENT_INTERVAL = 10;
constexpr uint32_t CTRL_FILE_CNT = 100;
constexpr int ARCHIVE_DOWNLOAD_MONITOR_TIME = 30;

const std::string SCAN_CTRL_MAX_DATA_SIZE = "10737418240";
const std::string SCAN_CTRL_MIN_DATA_SIZE = "5368709120";
constexpr uint64_t SCAN_META_FILE_SIZE = 1024 * 1024 * 1024;

constexpr uint32_t BACKUP_MAX_BUF_SIZE = 10 * 1024;
constexpr uint32_t BACKUP_BLOCK_SIZE_4M = 4 * 1024 * 1024;

constexpr int REPORT_INTERVAL = 180;
constexpr int REPORT_MONITOR_TIME_30S = 30;
constexpr uint32_t SCANNER_REPORT_CIRCLE_TIME = 60;

constexpr uint32_t NUMBER4000 = 4000;
constexpr uint32_t NUMBER1000 = 1000;
constexpr uint32_t NUMBER10000 = 10000;
constexpr uint32_t NUMBER1024 = 1024;
constexpr uint32_t NUMBER120 = 120;
constexpr uint32_t NUMBER100 = 100;
constexpr uint32_t NUMBER10 = 10;
constexpr uint32_t NUMBER5 = 5;
constexpr uint32_t NUMBER1 = 1;

constexpr auto BACKUP_KEY_SUFFIX = "_backup_stats";

struct RestoreJobExtendInfo {
    std::string failedScript;
    std::string postScript;
    std::string preScript;
    std::string restoreOption;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(failedScript, failed_script)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(postScript, post_script)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(preScript, pre_script)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(restoreOption, restoreOption)
    END_SERIAL_MEMEBER
};

struct FileSetInfo {
    std::string filters;
    std::string paths;
    std::string templateId;
    std::string templateName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(filters, filters)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(paths, paths)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(templateId, templateId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(templateName, templateName)
    END_SERIAL_MEMEBER
};

struct ExtendArchiveInfo {
    struct IpPort {
        std::string ip;
        int port { -1 };
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(ip, ip)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(port, port)
        END_SERIAL_MEMEBER
    };

    std::vector<IpPort> serviceInfo;
    bool enableSSL {false};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(enableSSL, enable_ssl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(serviceInfo, service_info)
    END_SERIAL_MEMEBER
};
} // namespace

bool HostArchiveRestore::IsAbort() const
{
    return IsAbortJob() || IsJobPause();
}

int HostArchiveRestore::PrerequisiteJob()
{
    INFOLOG("Enter HostArchiveRestore PrerequisiteJob");
    m_jobCtrlPhase = JOB_CTRL_PHASE_PREJOB;
    int ret = PrerequisiteJobInner();
    INFOLOG("Report PrerequisiteJob State");
    if (ret != SUCCESS) {
        ERRLOG("PrerequisiteJobInner failed");
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100),
            "nas_plugin_hetro_restore_prepare_fail_label", INITIAL_ERROR_CODE);
        SetJobToFinish();
        return FAILED;
    }

    ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100),
        "nas_plugin_hetro_restore_prepare_succeed_label", INITIAL_ERROR_CODE);
    SetJobToFinish();
    return SUCCESS;
}

int HostArchiveRestore::GenerateSubJob()
{
    INFOLOG("Enter HostArchiveRestore GenerateSubJob");
    m_jobCtrlPhase = JOB_CTRL_PHASE_GENSUBJOB;
    ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0),
        "nas_plugin_archive_restore_scan_start_label", INITIAL_ERROR_CODE);
    m_jobState = ArchiveJobState::RUNNING;
    int ret = GenerateSubJobInner();
    INFOLOG("Report GenerateSubJob State");
    if (ret != SUCCESS) {
        ERRLOG("GenerateSubJobInner failed");
        if (m_jobState == ArchiveJobState::RUNNING) {
            m_jobState = ArchiveJobState::FAILED;
        }
    }

    ReportGenerateSubJob();
    SetJobToFinish();
    return SUCCESS;
}

void HostArchiveRestore::ReportGenerateSubJob()
{
    DBGLOG("Enter ReportGenerateSubJob, totFailedDirs: %d", m_scanStats.m_totFailedDirs);
    std::string jobLabel;
    JobLogLevel::type type = JobLogLevel::type::TASK_LOG_ERROR;
    SubJobStatus::type status = SubJobStatus::FAILED;

    switch (m_jobState) {
        case ArchiveJobState::SUCCESS:
            jobLabel = "nas_plugin_hetro_restore_scan_completed_label";
            type = JobLogLevel::type::TASK_LOG_INFO;
            status = SubJobStatus::COMPLETED;
            break;
        case ArchiveJobState::FAILED:
            jobLabel = "nas_plugin_archive_restore_scan_fail_label";
            type = JobLogLevel::type::TASK_LOG_ERROR;
            status = SubJobStatus::FAILED;
            break;
        case ArchiveJobState::ABORT:
            jobLabel = "nas_plugin_archive_restore_scan_fail_label";
            type = JobLogLevel::type::TASK_LOG_WARNING;
            status = SubJobStatus::ABORTED;
            break;
        case ArchiveJobState::EMPTY_COPY:
            jobLabel = "nas_plugin_hetro_restore_scan_completed_label";
            type = JobLogLevel::type::TASK_LOG_INFO;
            status = SubJobStatus::COMPLETED;
            break;
        default:
            break;
    }
    PrintScannerStats();
    if (m_scanStats.m_totFailedDirs != 0) {
        jobLabel = "nas_plugin_hetro_restore_scan_completed_with_warn_label";
        ReportJobDetailsWithLabelAndErrcode(make_tuple(type, status, PROGRESS100), jobLabel, INITIAL_ERROR_CODE,
            std::to_string(m_scanStats.m_totDirsToBackup), std::to_string(m_scanStats.m_totFilesToBackup),
            PluginUtils::FormatCapacity(m_scanStats.m_totalSizeToBackup));
    } else {
        ReportJobDetailsWithLabelAndErrcode(make_tuple(type, status, PROGRESS100), jobLabel, INITIAL_ERROR_CODE,
            std::to_string(m_scanStats.m_totDirsToBackup), std::to_string(m_scanStats.m_totFilesToBackup),
            PluginUtils::FormatCapacity(m_scanStats.m_totalSizeToBackup), std::to_string(m_scanStats.m_totDirsToBackup),
            std::to_string(m_scanStats.m_totFilesToBackup),
            PluginUtils::FormatCapacity(m_scanStats.m_totalSizeToBackup));
    }
    DBGLOG("Exit ReportScannerCompleteStatus");
}

int HostArchiveRestore::ExecuteSubJob()
{
    INFOLOG("Enter HostArchiveRestore ExecuteSubJob");
    m_jobCtrlPhase = JOB_CTRL_PHASE_EXECSUBJOB;
    ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
    int ret = ExecuteSubJobInner();

    INFOLOG("Report ExecuteSubJobInner State");
    if (ret != SUCCESS && m_jobState != ArchiveJobState::SKIP_PHASE) {
        ERRLOG("ExecuteSubJobInner failed");
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0),
            "nas_plugin_archive_restore_data_fail_label", INITIAL_ERROR_CODE);
        SetJobToFinish();
        return FAILED;
    }

    ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
    SetJobToFinish();
    return SUCCESS;
}

int HostArchiveRestore::PostJob()
{
    INFOLOG("Enter HostArchiveRestore PostJob");
    m_jobCtrlPhase = JOB_CTRL_PHASE_POSTJOB;
    int ret = PostJobInner();
    if (ret != SUCCESS) {
        ERRLOG("PostJob failed");
        m_jobState = ArchiveJobState::FAILED;
    } else if (m_backupStats.noOfDirFailed != 0 || m_backupStats.noOfFilesFailed != 0) {
        m_jobState = ArchiveJobState::PARTIAL_SUCCESS;
    } else {
        m_jobState = ArchiveJobState::SUCCESS;
    }
    INFOLOG("Report PostJob State");
    ReportPostJob();
    SetJobToFinish();
    // persist statistic in cache
    return SUCCESS;
}

void HostArchiveRestore::ReportPostJob()
{
    std::string jobLabel;
    SubJobStatus::type status = SubJobStatus::FAILED;
    JobLogLevel::type type = JobLogLevel::type::TASK_LOG_ERROR;
    switch (m_jobState) {
        case ArchiveJobState::SUCCESS:
            jobLabel = "nas_plugin_hetro_restore_data_completed_label";
            type = JobLogLevel::type::TASK_LOG_INFO;
            status = SubJobStatus::COMPLETED;
            break;
        case ArchiveJobState::PARTIAL_SUCCESS:
            jobLabel = "nas_plugin_hetro_restore_data_completed_with_warn_label";
            type = JobLogLevel::type::TASK_LOG_WARNING;
            status = SubJobStatus::FAILED;
            break;
        case ArchiveJobState::FAILED:
            jobLabel = "";
            type = JobLogLevel::type::TASK_LOG_ERROR;
            status = SubJobStatus::FAILED;
            break;
        default:
            break;
    }
    ReportJobDetailsWithLabelAndErrcode(make_tuple(type, status, PROGRESS100), jobLabel, INITIAL_ERROR_CODE,
                                        std::to_string(m_backupStats.noOfDirCopied),
                                        std::to_string(m_backupStats.noOfFilesCopied),
                                        PluginUtils::FormatCapacity(m_backupStats.noOfBytesCopied),
                                        std::to_string(m_backupStats.noOfDirFailed),
                                        std::to_string(m_backupStats.noOfFilesFailed));
}

bool HostArchiveRestore::InitJobInfo()
{
    INFOLOG("Enter InitJobInfo");
    m_jobInfoPtr = std::dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobCommonInfo->GetJobInfo());
    m_jobId = m_jobInfoPtr->jobId;
    INFOLOG("Init job info, job id: %s", m_jobId.c_str());

    if (m_jobInfoPtr->copies.empty()) {
        return false;
    }
    m_copyId = m_jobInfoPtr->copies[0].id; // 副本ID
    m_resourceId = m_jobInfoPtr->copies[0].protectObject.id;
    std::string protectId = m_jobInfoPtr->copies[0].protectObject.id;
    m_sourceContextMd = "source_policy_" + protectId + "_Context_Global_MD";
    m_sourceContext = "source_policy_" + protectId + "_Context";

    INFOLOG("Copy id: %s", m_copyId.c_str());
    INFOLOG("Meta file path prefix: %s", m_sourceContextMd.c_str());
    INFOLOG("Meta file path prefix: %s", m_sourceContext.c_str());

    m_isAggCopy = (m_jobInfoPtr->copies[0].formatType == CopyFormatType::type::INNER_DIRECTORY) ? true : false;
    INFOLOG("Copy form is %s", m_isAggCopy ? "aggregate" : "normal");

    return true;
}

bool HostArchiveRestore::GetBackupCopyInfo(const std::string& backupCopyInfo)
{
    Json::Value value;
    if (!Module::JsonHelper::JsonStringToJsonValue(backupCopyInfo, value)) {
        ERRLOG("Convert backupCopyInfo json string to json value failed.");
        return false;
    }
    if (!(value.isObject() && value.isMember("extendInfo"))) {
        return false;
    }
    Json::Value aggCopyExtValue = value["extendInfo"];
    if (!Module::JsonHelper::JsonValueToStruct(aggCopyExtValue, m_aggCopyInfo)) {
        ERRLOG("JsonStringToStruct failed, aggCopyExtendInfo");
        return false;
    }
    INFOLOG("GetBackupCopyInfo success");
    return true;
}

// Get repo info from RestoreJob
/** cache normal
* | -- CacheFsPath                      // CacheFsPath from UBC --> m_cacheFsPath
*    | -- source_policy                 // download meta from archive m_cacheMdPath
*        | -- filemeta                  // m_downloadMetePath
*           |-- metafile.zip
*        | -- volume                    // metefile.zip unzip to this -> m_volumePath
*           | -- snapshotPrimaryVolume
*           | -- sourcePrimaryVolume
*           | -- SubVolume_x            // subVolPath
*             | -- metafile.zip         // unzip to current dir
*             | -- dcahe-fcache-metafile
*             | -- scan_ctrl             // m_scanControlFilePath
*             | -- backup_ctrl           // m_backupControlFilePath
*             | -- sqlite                        // index file
*    | -- statistics                    // Folder to save the statistic of backup main job and sub-jobs
*/
bool HostArchiveRestore::InitRepoInfo()
{
    INFOLOG("Enter InitRepoInfo");
    StorageRepository cacheFs;
    for (unsigned int i = 0; i < m_jobInfoPtr->copies[0].repositories.size(); i++) {
        if (m_jobInfoPtr->copies[0].repositories[i].repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            cacheFs = m_jobInfoPtr->copies[0].repositories[i];
        }
    }

    if (cacheFs.path.empty()) {
        ERRLOG("Cache repo path is empty");
        return false;
    }
    m_cacheFsPath = cacheFs.path[0];
    m_cacheFsRemotePath = cacheFs.remotePath;
    INFOLOG("Archive restore cache repo path: %s", m_cacheFsPath.c_str());

    if (!PluginUtils::IsDirExist(m_cacheFsPath)) {
        ERRLOG("m_cacheFsPath path no exist: %s", m_cacheFsPath.c_str());
        return false;
    }

    std::string backupCopyInfo =  m_jobInfoPtr->copies[0].extendInfo;
    INFOLOG("Backup copy info: %s", Module::WipeSensitiveDataForLog(backupCopyInfo.c_str()).c_str());

    if (!m_isAggCopy) {
        m_cacheMdPath =  m_cacheFsPath + Module::PATH_SEPARATOR + m_sourceContextMd;
        m_downloadMetePath = m_cacheMdPath + Module::PATH_SEPARATOR + "filemeta";
#ifdef WIN32
        m_volumePath = PluginUtils::PathJoin(m_cacheFsPath, "volumes");  // 由于路径超长，win下去除sourcepolicy路径
#else
        m_volumePath = m_cacheMdPath + Module::PATH_SEPARATOR + "volume";
#endif
        m_dataPath = Module::PATH_SEPARATOR + m_sourceContext; // dst s3
    } else {
        GetBackupCopyInfo(backupCopyInfo);
        m_cacheMdPath =  m_cacheFsPath + Module::PATH_SEPARATOR + m_sourceContextMd + Module::PATH_SEPARATOR +
            m_aggCopyInfo.metaPathSuffix;
        m_downloadMetePath = m_cacheMdPath + Module::PATH_SEPARATOR + "filemeta";
#ifdef WIN32
        m_volumePath = PluginUtils::PathJoin(m_cacheFsPath, m_aggCopyInfo.metaPathSuffix, "volumes");
#else
        m_volumePath = m_cacheMdPath + Module::PATH_SEPARATOR + "volume";
#endif
        m_dataPath = Module::PATH_SEPARATOR + m_aggCopyInfo.metaPathSuffix; // dst s3
    }
#ifdef WIN32
    transform(m_dataPath.begin(), m_dataPath.end(), m_dataPath.begin(), ::tolower);
#endif

    m_statsPath = m_cacheFsPath + Module::PATH_SEPARATOR + "statistics";
    INFOLOG("m_cacheMdPath: %s, downloadMetaPath:%s, volumePath:%s, dataPath:%s, statPath:%s", m_cacheMdPath.c_str(),
        m_downloadMetePath.c_str(), m_volumePath.c_str(), m_dataPath.c_str(), m_statsPath.c_str());
    return true;
}

bool HostArchiveRestore::InitRestoreInfo()
{
    INFOLOG("Enter InitRestoreInfo");
    // get restore root path
    std::string extJsonString = m_jobInfoPtr->targetObject.extendInfo;
    INFOLOG("Extend info json string: %s", extJsonString.c_str());
    FileSetInfo fileSetInfo;
    if (!Module::JsonHelper::JsonStringToStruct(extJsonString, fileSetInfo)) {
        ERRLOG("Convert to FileSetInfo failed.");
        return false;
    }

    std::string paths = fileSetInfo.paths;
    INFOLOG("Restore root path: %s", paths.c_str());
    Json::Value value;
    if (!Module::JsonHelper::JsonStringToJsonValue(paths, value)) {
        ERRLOG("Convert paths json string to json value failed.");
        return false;
    }

    for (const auto& v : value) {
        if (!(v.isObject() && v.isMember("name") && v["name"].isString())) {
            return false;
        }
        std::string path = v["name"].asString();
        DBGLOG("Fileset Path: %s", path.c_str());
        m_restorePathList.push_back(path);
    }

    // make file list for fine restore
    if (IsFineRestore()) {
        for (const auto& info : m_jobInfoPtr->restoreSubObjects) {
            std::string name = info.name;
            if (!name.empty()) {
                m_fineRestoreObj.push_back(name);
                DBGLOG("Restore src path: %s", name.c_str());
            }
        }
    }
    return true;
}

bool HostArchiveRestore::InitInfo()
{
    INFOLOG("Enter InitInfo");
    if (!InitJobInfo()) {
        ERRLOG("Init job info failed.");
        return false;
    }
    if (!InitRepoInfo()) {
        ERRLOG("Init repo info failed.");
        return false;
    }
    if (!InitRestoreInfo()) {
        ERRLOG("Init restore info failed.");
        return false;
    }
    if (!InitArchiveInfo()) {
        ERRLOG("Init archive info failed.");
        return false;
    }
    return true;
}

bool HostArchiveRestore::InitArchiveInfo()
{
    INFOLOG("Enter InitArchiveInfo");
    std::string copyExtInfoString {};
    for (const auto& copy : m_jobInfoPtr->copies) {
        for (const auto& repo : copy.repositories) {
            if (repo.protocol == RepositoryProtocolType::type::S3) {
                copyExtInfoString = repo.extendInfo;
            }
        }
    }
    if (copyExtInfoString.empty()) {
        ERRLOG("Copy extend info of archive is empty.");
        return false;
    }
    INFOLOG("copyExtInfoString: %s", copyExtInfoString.c_str());

    ExtendArchiveInfo extendArchiveInfo {};
    if (!Module::JsonHelper::JsonStringToStruct(copyExtInfoString, extendArchiveInfo)) {
        ERRLOG("Convert archive server json string to struct failed");
        return false;
    }

    if (extendArchiveInfo.serviceInfo.empty()) {
        ERRLOG("Archive server service info is empty");
        return false;
    }

    m_archiveServerInfo.port = extendArchiveInfo.serviceInfo[0].port;
    m_archiveServerInfo.enableSSL = extendArchiveInfo.enableSSL;
    for (const auto& info : extendArchiveInfo.serviceInfo) {
        m_archiveServerInfo.ipList.push_back(info.ip);
    }

    return true;
}

bool HostArchiveRestore::IsFineRestore() const
{
    return m_jobInfoPtr->jobParam.restoreType == RestoreJobType::FINE_GRAINED_RESTORE;
}

bool HostArchiveRestore::InitArchiveClient()
{
    DBGLOG("Enter InitArchiveClinet");
    m_archiveClient = std::make_shared<ArchiveClient>(m_jobId, m_copyId);
    const auto& [ipList, port, ssl] = m_archiveServerInfo;
    if (m_archiveClient->InitClient(ipList, port, ssl) != SUCCESS) {
        ERRLOG("Archive client connect to server failed.");
        return false;
    }
    DBGLOG("Client Connect success");
    return true;
}

int HostArchiveRestore::PrerequisiteJobInner()
{
    INFOLOG("Enter PrerequisiteJobInner");
    if (!InitInfo()) {
        return FAILED;
    }

    // test connect to archive server
    if (!InitArchiveClient()) {
        return FAILED;
    }

    if (m_archiveClient->Disconnect() != SUCCESS) {
        ERRLOG("Check disconnect archive server failed.");
        return FAILED;
    }

    INFOLOG("Exit PrerequisiteJobInner");
    return SUCCESS;
}

int HostArchiveRestore::GenerateSubJobInner()
{
    INFOLOG("Enter GenerateSubJobInner");
    if (!InitInfo()) {
        return FAILED;
    }

    if (!InitCacheRepoDir()) {
        return FAILED;
    }

    // parase csv download dchache/fcahce/metafile no need to copy to cache
    if (!DownloadMetaFile()) {
        ERRLOG("Download meta file from archive server failed.");
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100),
            "nas_plugin_archive_restore_download_metafile_fail_label", INITIAL_ERROR_CODE);
        return FAILED;
    }

    // MD/filemeta/metafile.tar.gz
    auto ret = std::async(std::launch::async,
        std::bind(&HostArchiveRestore::UnzipMetafileZip, this, m_downloadMetePath, m_volumePath));
    while (!m_isPrepareMetaFinish) {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
        Module::SleepFor(std::chrono::seconds(REPORT_MONITOR_TIME_30S));
    }
    m_isPrepareMetaFinish = false; // 重新初始化
    if (!ret.get()) {
        ERRLOG("UnzipMetafileZip failed");
        return FAILED;
    }
    // generate subjob to report start_to_restore_label
    GenerateReportStartRestoreSubJob();

    if (!GenerateControlFile()) {
        return FAILED;
    }

    INFOLOG("Exit GenerateSubJobInner");
    return SUCCESS;
}

void HostArchiveRestore::GenerateReportStartRestoreSubJob()
{
    ActionResult ret;
    vector<SubJob> subJobList;
    SubJob subjob;
    subjob.__set_jobName(SUBJOB_TYPE_ARCHIVE_REPORT_START_LABEL);
    subjob.__set_jobPriority(SUBJOB_TYPE_SETUP_PHASE_PRIO);
    subjob.__set_jobId(m_jobId);
    subjob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subjob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subjob.__set_jobInfo("");
    subjob.__set_ignoreFailed(true);
    subJobList.push_back(subjob);
    JobService::AddNewJob(ret, subJobList);
    if (ret.code != Module::SUCCESS) {
        ERRLOG("add report label subjob failed!");
        return;
    }
    INFOLOG("add report label subjob success!");
    return;
}

bool HostArchiveRestore::GenerateControlFile()
{
    INFOLOG("Enter GenerateControlFile");
    std::shared_ptr<void> defer(nullptr, [&](...) {
        if (m_scanner != nullptr) {
            m_scanner->Destroy();
        }
    });

    std::vector<std::string> subDirList;
    if (!PluginUtils::GetDirListInDirectory(m_volumePath, subDirList)) {
        ERRLOG("GetDirListInDirectory failed");
        return false;
    }

    for (const auto& dir : subDirList) {
        if (!InitSubDir(dir)) {
            return false;
        }

        auto ret = std::async(std::launch::async,
            std::bind(&HostArchiveRestore::UnzipMetafileZip, this, m_downloadMetePath, m_scanInputMetePath));
        while (!m_isPrepareMetaFinish) {
            ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
            Module::SleepFor(std::chrono::seconds(REPORT_MONITOR_TIME_30S));
        }
        m_isPrepareMetaFinish = false; // 重新初始化
        if (!ret.get()) {
            ERRLOG("UnzipMetafileZip failed");
            return false;
        }

        if (!StartScanner()) {
            ERRLOG("Start Scanner Failed");
            return false;
        }
        MonitorScan();
        if (m_scanner != nullptr) {
            m_scanner->Destroy();
            m_scanner.reset();
        }
    }
    INFOLOG("Exit GenerateControlFile");
    return true;
}

bool HostArchiveRestore::InitSubDir(const std::string& dir)
{
    m_downloadMetePath = m_volumePath + Module::PATH_SEPARATOR + dir;
    m_scanInputMetePath = m_downloadMetePath;
    m_scanControlFilePath = m_downloadMetePath + Module::PATH_SEPARATOR + "scan_ctrl";
    m_backupControlFilePath = m_downloadMetePath + Module::PATH_SEPARATOR + "backup_ctrl";
    INFOLOG("m_scanControlFilePath: %s", m_scanControlFilePath.c_str());
    INFOLOG("m_backupControlFilePath: %s", m_backupControlFilePath.c_str());

    if (!PluginUtils::CreateDirectory(m_scanControlFilePath)) {
        ERRLOG("m_scanControlFilePath path create failed: %s", m_scanControlFilePath.c_str());
        return false;
    }
    if (!PluginUtils::CreateDirectory(m_backupControlFilePath)) {
        ERRLOG("m_backupControlFilePath path create failed: %s", m_backupControlFilePath.c_str());
        return false;
    }
    return true;
}

bool HostArchiveRestore::InitCacheRepoDir()
{
    if (!PluginUtils::CreateDirectory(m_cacheMdPath)) {
        ERRLOG("m_cacheMdPath path create failed: %s", m_cacheMdPath.c_str());
        return false;
    }

    if (!PluginUtils::CreateDirectory(m_statsPath)) {
        ERRLOG("m_statsPath path create failed: %s", m_statsPath.c_str());
        return false;
    }

    if (!InitMainResources()) {
        ERRLOG("InitMainResources failed");
        return false;
    }

    if (!QueryMainScanResources()) {
        ERRLOG("QueryMainScanResources failed");
        return false;
    }
    return true;
}

bool HostArchiveRestore::SendJobReportForAliveness()
{
    int64_t currTime = PluginUtils::GetCurrentTimeInSeconds();
    if ((currTime - m_lastJobReportTime) > REPORT_INTERVAL) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        m_lastJobReportTime = PluginUtils::GetCurrentTimeInSeconds();
    }
    return true;
}

bool HostArchiveRestore::DownloadMetaFile()
{
    INFOLOG("Enter DownloadCacheFile, %s", m_cacheFsPath.c_str());
    std::vector<std::string> pathList { "/" + m_sourceContextMd + "/" };
    ArchiveDownloadParam param = ArchiveDownloadParam(m_jobId, m_copyId, m_resourceId,
        m_cacheFsPath, m_cacheFsRemotePath);
    ArchiveDownloadFile downloadFile(param, m_archiveServerInfo);
    std::thread downloadThread(std::bind(&ArchiveDownloadFile::Start,
        &downloadFile, m_cacheFsPath, pathList));
    
    while (downloadFile.m_state == ArchiveDownloadState::RUNNING) {
        if (IsAbort()) {
            WARNLOG("Abort ArchiveDownloadFile");
            downloadFile.SetAbort();
        }
        DBGLOG("Archive Downloading...");
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        Module::SleepFor(std::chrono::seconds(ARCHIVE_DOWNLOAD_MONITOR_TIME));
    }
    downloadThread.join();

    if (downloadFile.m_state == ArchiveDownloadState::FAILED) {
        ERRLOG("download failed");
        return false;
    }

    if (downloadFile.m_state == ArchiveDownloadState::EMPTY_COPY) {
        m_jobState = ArchiveJobState::EMPTY_COPY;
        WARNLOG("Empty archive copy");
        return false;
    }

    INFOLOG("Archive download success");
    // get archive fs id in S3
    m_archiveFsId = downloadFile.GetFileSystemsId();

    return true;
}

bool HostArchiveRestore::UnzipMetafileZip(const std::string& downloadMetePath, const std::string& outputMetaPath)
{
    std::shared_ptr<void> defer(nullptr, [&](...) { m_isPrepareMetaFinish = true; });

    INFOLOG("UnzipMetafileZip outputMetaPath: %s", outputMetaPath.c_str());
    if (!PluginUtils::CreateDirectory(outputMetaPath)) {
        ERRLOG("Create directory failed: %s", outputMetaPath.c_str());
        return false;
    }

    std::string metafileZip = downloadMetePath + Module::PATH_SEPARATOR + METAFILE_ZIP_NAME;
    INFOLOG("UnzipMetafileZip metafileZip: %s", metafileZip.c_str());
    
    if (!PluginUtils::IsFileExist(metafileZip.c_str())) {
        ERRLOG("Meta file zip not exist metafileZip: %s", metafileZip.c_str());
        return false;
    }
#ifdef WIN32
    string win7zPath = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    string cmd =
        win7zPath + " x " + PluginUtils::ReverseSlash(metafileZip) + " -o" + PluginUtils::ReverseSlash(outputMetaPath);
    INFOLOG("compress cmd : %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        return Module::FAILED;
    }
#else
    std::vector<std::string> output;
    std::vector<std::string> errOutput;
#if defined(_AIX) || defined(SOLARIS)
    string cmd = "cd " + outputMetaPath + " && gunzip -c " + metafileZip + " | tar -xf - && cd -";
#else
    std::string cmd = "tar -zxf " + metafileZip + " -C " + outputMetaPath;
#endif
    int ret = Module::runShellCmdWithOutput(INFO, "HostArchiveRestore", 0, cmd, {}, output, errOutput);
    if (ret != 0) {
        ERRLOG("unzip failed: %s, ret: %d", cmd.c_str(), ret);
        std::for_each(output.begin(), output.end(),
            [&] (const std::string& v) { ERRLOG("output: %s", v.c_str());});
        std::for_each(errOutput.begin(), errOutput.end(),
            [&] (const std::string& v) { ERRLOG("output: %s", v.c_str());});
        return false;
    }
#endif
    INFOLOG("Unzip finish ret: %d", ret);
    return true;
}

bool HostArchiveRestore::StartScanner()
{
    INFOLOG("Enter StartScanner");
    m_lastScannerReportTime = PluginUtils::GetCurrentTimeInSeconds();
    ScanConfig config;
    FillScanConfig(config);
    AddFilterRule(config);
    m_scanner = ScanMgr::CreateScanInst(config);
    if (m_scanner == nullptr) {
        ERRLOG("New scanner failed");
        return false;
    }
    if (m_scanner->Start() != SCANNER_STATUS::SUCCESS) {
        ERRLOG("Start scanner instance failed");
        return false;
    }
    INFOLOG("Start scanner instance success");
    return true;
}

inline bool HostArchiveRestore::IsDir(const std::string& name) const
{
    return name.back() == '/';
}

void HostArchiveRestore::AddFilterRule(ScanConfig& scanConfig)
{
    DBGLOG("Enter AddFilterRule");
    std::vector<std::string> dirFilterRule;
    std::vector<std::string> fileFilterRule;
    for (const auto& obj : m_jobInfoPtr->restoreSubObjects) {
        std::string path = obj.name;
        DBGLOG("Filter the path : %s", path.c_str());
#ifdef WIN32
        transform(path.begin(), path.end(), path.begin(), ::tolower);
#endif
        if (IsDir(path)) {
            dirFilterRule.push_back(path);
        } else {
            fileFilterRule.push_back(path);
        }
    }
    scanConfig.dCtrlFltr = dirFilterRule;
    scanConfig.fCtrlFltr = fileFilterRule;
}

void HostArchiveRestore::FillScanConfig(ScanConfig& scanConfig)
{
    INFOLOG("Enter FillScanConfig");
    scanConfig.reqID  = PluginUtils::GenerateHash(m_jobId);
    scanConfig.jobId = m_jobId;

    scanConfig.scanIO = IOEngine::DEFAULT;
    scanConfig.lastBackupTime = 0;

    /* config meta path */
    scanConfig.curDcachePath  = m_scanInputMetePath;
    scanConfig.metaPathForCtrlFiles = m_scanControlFilePath;

    scanConfig.maxOpendirReqCount = NUMBER4000;

    /* 记录线程数 */
    scanConfig.maxCommonServiceInstance = 1;

    scanConfig.usrData = (void*)this;
    scanConfig.scanResultCb = [](void* usrData, std::string) {};
    scanConfig.scanHardlinkResultCb = [](void* usrData, std::string) {};

    scanConfig.scanCtrlMaxDataSize = SCAN_CTRL_MAX_DATA_SIZE;
    scanConfig.scanCtrlMinDataSize = SCAN_CTRL_MIN_DATA_SIZE;
    scanConfig.scanCtrlFileTimeSec = NUMBER5;
    scanConfig.scanCtrlMaxEntriesFullBkup = NUMBER10000;
    scanConfig.scanCtrlMaxEntriesIncBkup = NUMBER1000;
    scanConfig.scanCtrlMinEntriesFullBkup = NUMBER10000;
    scanConfig.scanCtrlMinEntriesIncBkup = NUMBER1000;
    scanConfig.scanMetaFileSize = SCAN_META_FILE_SIZE; // one GB

    scanConfig.maxWriteQueueSize = NUMBER10000;
    scanConfig.scanType = ScanJobType::CONTROL_GEN; // use dcache/fcache to create control file, skip enqueue
    scanConfig.triggerTime = PluginUtils::GetCurrentTimeInSeconds();
    scanConfig.generatorIsFull =  true;
    scanConfig.scanCheckPointEnable = false;
    INFOLOG("EXIT FillScanConfig");
}

void HostArchiveRestore::MonitorScan()
{
    HCP_Log(INFO, MODULE) << "Enter Monitor Scanner" << HCPENDLOG;
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    std::string jobLogLabel = "";
    m_jobState = ArchiveJobState::RUNNING;

    do {
        m_scanStatus = m_scanner->GetStatus();
        if (m_scanStatus == SCANNER_STATUS::INIT) {
            Module::SleepFor(std::chrono::seconds(SUBTASK_WAIT_FOR_SCANNER_READY_IN_SEC));
            SendJobReportForAliveness();
            continue;
        }

        UpdateScannerStats(m_totalScanStats);
        ReportScannerRunningStatus();

        SetArchiveJobState();
        if (m_jobState != ArchiveJobState::RUNNING) {
            HCP_Log(INFO, MODULE) << "m_jobState: " << static_cast<int>(m_jobState) << HCPENDLOG;
            break;
        }

        if (IsAbort()) {
            INFOLOG("Scanner - Abort is invocked for TaskID: %s, subTaskID: %s", m_jobId.c_str(), m_subJobId.c_str());
            if (SCANNER_STATUS::SUCCESS != m_scanner->Abort()) {
                HCP_Log(ERR, MODULE) << "Scanner Abort is failed" << HCPENDLOG;
            }
            jobStatus = SubJobStatus::ABORTED;
            return;
        }

        /* Generate backup sub tasks based on the control file generated during scanning */
        if (!CreateSubTasksFromCtrlFile(jobStatus)) {
            HCP_Log(ERR, MODULE) << "Create subtask failed, abort scan" << HCPENDLOG;
            m_scanner->Abort();
        }
        Module::SleepFor(std::chrono::seconds(GENERATE_SUBTASK_MONITOR_DUR_IN_SEC));
    } while (true);

    if (!CreateSubTasksFromCtrlFile(jobStatus, true)) {
        HCP_Log(ERR, MODULE) << "Create subtask failed" << HCPENDLOG;
        jobLogLabel = "file_plugin_host_backup_scan_fail_label";
        jobStatus = SubJobStatus::FAILED;
        return;
    }

    m_totalScanStats = m_scanStats; // update total scan info
    HCP_Log(INFO, MODULE) << "Exit Monitor Scanner" << HCPENDLOG;
}

bool HostArchiveRestore::ReportScannerRunningStatus()
{
    if ((PluginUtils::GetCurrentTimeInSeconds() - m_lastScannerReportTime) > SCANNER_REPORT_CIRCLE_TIME) {
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0),
                                            "nas_plugin_hetro_backup_scan_inprogress_label",
                                            INITIAL_ERROR_CODE,
                                            std::to_string(m_scanStats.m_totDirsToBackup),
                                            std::to_string(m_scanStats.m_totFilesToBackup),
                                            PluginUtils::FormatCapacity(m_scanStats.m_totalSizeToBackup),
                                            std::to_string(m_scanStats.m_totDirsToBackup),
                                            std::to_string(m_scanStats.m_totFilesToBackup),
                                            PluginUtils::FormatCapacity(m_scanStats.m_totalSizeToBackup));
        m_lastScannerReportTime = PluginUtils::GetCurrentTimeInSeconds();
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
    }
    return true;
}

void HostArchiveRestore::SetArchiveJobState()
{
    INFOLOG("Scanner status: %d", static_cast<int>(m_scanStatus));
    if (m_scanStatus == SCANNER_STATUS::COMPLETED) {
        m_jobState = ArchiveJobState::SUCCESS;
    }
    if (m_scanStatus == SCANNER_STATUS::FAILED ||
        m_scanStatus == SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE ||
        m_scanStatus == SCANNER_STATUS::PROTECTED_SERVER_NOT_REACHABLE) {
        m_jobState = ArchiveJobState::FAILED;
    }
    if (m_scanStatus == SCANNER_STATUS::ABORT_IN_PROGRESS || m_scanStatus == SCANNER_STATUS::ABORTED) {
        m_jobState = ArchiveJobState::ABORT;
    }
}

bool HostArchiveRestore::CreateSubTasksFromCtrlFile(SubJobStatus::type &jobStatus, bool isFinal)
{
    std::vector<std::string> srcFileList {};
    std::vector<SubJob> subJobList {};
    std::vector<std::string> ctrlFileList {};
    uint32_t validCtrlFileCntr = 0;

    if (!CheckFilePathAndGetSrcFileList(m_scanControlFilePath, m_backupControlFilePath, srcFileList)) {
        return false;
    }

    INFOLOG("Enter CreateSubTasksFromCtrlFile, size: %d, isFinal: %d", srcFileList.size(), isFinal);
    for (uint32_t i = 0; i < srcFileList.size(); i++) {
        SendJobReportForAliveness();
        if (IsAbort()) {
            INFOLOG("Exit CreateSubTasksFromCtrlFile, Abort is invocked for taskid: %s, subtaskid: %s",
                m_jobId.c_str(), m_subJobId.c_str());
            jobStatus = SubJobStatus::ABORTED;
            return true;
        }

        std::string ctrlFileFullPath = srcFileList[i];
        if (!IsValidCtrlFile(ctrlFileFullPath)) {
            continue;
        }

        if (!isFinal && validCtrlFileCntr++ >= CTRL_FILE_CNT) {
            break;
        }

        std::string dstCtrlFileFullPath = m_backupControlFilePath + Module::PATH_SEPARATOR +
            PluginUtils::GetFileName(ctrlFileFullPath);
        std::string dstCtrlFileInCacheFsPath = dstCtrlFileFullPath.substr(m_cacheFsPath.length(), std::string::npos);

        HCP_Log(DEBUG, MODULE) << "cpFile, src: " << ctrlFileFullPath << ", dst: " << dstCtrlFileFullPath <<
            ", dstCtrlFileInCacheFsPath " << dstCtrlFileInCacheFsPath << HCPENDLOG;
        if (!PluginUtils::CopyFile(ctrlFileFullPath, dstCtrlFileFullPath)) {
            return false;
        }
        if (!GenerateSubJobList(subJobList, ctrlFileList, dstCtrlFileInCacheFsPath, ctrlFileFullPath)) {
            return false;
        }
        // We create 10 Jobs at a time. If 10 is not accumulated, continue
        if (subJobList.size() % 10 != 0) {
            continue;
        }
        if (!CreateSubTask(subJobList, ctrlFileList)) {
            return false;
        }
    }
    if (!CreateSubTask(subJobList, ctrlFileList)) {
        return false;
    }

    HCP_Log(INFO, MODULE) << "Exit CreateSubTasksFromCtrlFile Success" << HCPENDLOG;
    return true;
}

bool HostArchiveRestore::GenerateSubJobList(std::vector<SubJob> &subJobList, std::vector<std::string> &ctrlFileList,
    const std::string &dstCtrlFileRelPath, const std::string &ctrlFileFullPath)
{
    INFOLOG("GenerateSubJobList m_jobId: %s", m_jobId.c_str());
    SubJob subJob {};
    if (!InitSubTask(subJob, dstCtrlFileRelPath, false, "", m_archiveFsId)) {
        HCP_Log(ERR, MODULE) << "Init subtask failed" << HCPENDLOG;
        return false;
    }
    // TO-DO UpdateCopyPhaseStartTimeInGenRsc

    subJobList.push_back(subJob);
    ctrlFileList.push_back(ctrlFileFullPath);

    return true;
}

int HostArchiveRestore::ExecuteSubJobInner()
{
    INFOLOG("Enter ExecuteSubJobInner");
    if (!InitInfo()) {
        return FAILED;
    }
    m_subJobId = m_subJobInfo->subJobId;
    if (m_subJobInfo->jobName == SUBJOB_TYPE_ARCHIVE_REPORT_START_LABEL) {
        INFOLOG("It's report label subjob");
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0),
            "nas_plugin_archive_restore_data_start_label", 0);
        return SUCCESS;
    }

    if (!GetRestorePolicy()) {
        ERRLOG("GetRestorePolicy failed");
        return FAILED;
    }

    if (!InitArchiveClient()) {
        return FAILED;
    }

    if (!InitSubBackupJobResources()) {
        return FAILED;
    }

    if (!StartRestore()) {
        ERRLOG("Start restore failed.");
        return FAILED;
    }

    SubJobStatus::type monitorRet = MonitorRestore();

    if (m_backup != nullptr) {
        m_backup->Destroy();
        m_backup.reset();
    }

    INFOLOG("Exit ExecuteSubJobInner");
    return monitorRet == SubJobStatus::COMPLETED ? SUCCESS : FAILED;
}

bool HostArchiveRestore::GetRestorePolicy()
{
    HCP_Log(INFO, MODULE) << "Enter GetRestoreCoverPolicy" << HCPENDLOG;
    RestoreJobExtendInfo extendInfo;
    if (!Module::JsonHelper::JsonStringToStruct(m_jobInfoPtr->extendInfo, extendInfo)) {
        HCP_Log(ERR, MODULE) << "Convert to RestoreJobExtendInfo json failed." << HCPENDLOG;
        return false;
    }
    if (extendInfo.restoreOption == "OVERWRITING") {
        m_coveragePolicy = RestoreReplacePolicy::OVERWRITE;
    } else if (extendInfo.restoreOption == "SKIP") {
        m_coveragePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    } else if (extendInfo.restoreOption == "REPLACE") {
        m_coveragePolicy = RestoreReplacePolicy::OVERWRITE_OLDER;
    } else {
        HCP_Log(ERR, MODULE) << "extendInfo.restoreOption is: "
                             << extendInfo.restoreOption << HCPENDLOG;
        HCP_Log(ERR, MODULE) << "Get Restore Cover Policy failed. extendInfo is: "
                            << m_jobInfoPtr->extendInfo << HCPENDLOG;
        return false;
    }
    return true;
};

bool HostArchiveRestore::StartRestore()
{
    INFOLOG("Subjob info: %s", m_subJobInfo->jobInfo.c_str());
    // get subjob info include control file, backup phase, fs id
    BackupSubJob subJobInfo;
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, subJobInfo)) {
        HCP_Log(ERR, MODULE) << "Get restore subjob info failed" << HCPENDLOG;
        return false;
    }
    subJobInfo.controlFile = m_cacheFsPath + subJobInfo.controlFile; // add cache path make full path
    m_archiveClient->SetFsId(subJobInfo.fsId); // set arcive s3 id

    BackupParams backupParams = FillRestoreConfig(subJobInfo);

    // 这版本暂时不实现归档硬链接
    if (subJobInfo.subTaskType == SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE ||
        subJobInfo.subTaskType == SUBJOB_TYPE_DATACOPY_DELETE_PHASE) {
        WARNLOG("Skip delete or hardlink phase");
        m_jobState = ArchiveJobState::SKIP_PHASE;
        return false;
    }

    m_backup = FS_Backup::BackupMgr::CreateBackupInst(backupParams);
    if (m_backup == nullptr) {
        HCP_Log(ERR, MODULE) << "Create backup instance failed" << HCPENDLOG;
        return false;
    }

    if (m_backup->Enqueue(subJobInfo.controlFile) != BackupRetCode::SUCCESS) {
        HCP_Log(ERR, MODULE) << "enqueue backup instance failed" << HCPENDLOG;
        return false;
    }
    if (m_backup->Start() != BackupRetCode::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Start backup task failed" << HCPENDLOG;
        return false;
    }
    return true;
}

BackupParams HostArchiveRestore::FillRestoreConfig(const BackupSubJob& subJobInfo)
{
    HCP_Log(INFO, MODULE) << "Enter FillRestoreConfig." << HCPENDLOG;

    BackupParams backupParams {};
    backupParams.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    backupParams.srcAdvParams = std::make_shared<ArchiveRestoreAdvanceParams>(m_archiveClient);
    std::dynamic_pointer_cast<ArchiveRestoreAdvanceParams>(backupParams.srcAdvParams)->dataPath = m_dataPath;

    std::string destinationPath = m_jobInfoPtr->targetObject.name;
#ifdef WIN32
    backupParams.dstEngine = BackupIOEngine::WIN32_IO;
#else
    backupParams.dstEngine = BackupIOEngine::POSIX;
#endif
    backupParams.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();
    std::dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = destinationPath;
    backupParams.backupType = BackupType::RESTORE;

    // volume/subvol/backup_control/control.txt -- Upper Two dirs
    std::string subVolPath = PluginUtils::GetPathName(PluginUtils::GetPathName(subJobInfo.controlFile));
    backupParams.scanAdvParams.metaFilePath = subVolPath;
    HCP_Log(INFO, MODULE) << "resourcePath is: " << m_dataPath <<  HCPENDLOG;
    HCP_Log(INFO, MODULE) << "destinationPath is: " << destinationPath <<  HCPENDLOG;
    HCP_Log(INFO, MODULE) << "backupParams.scanAdvParams.metaFilePath is: " << subVolPath <<  HCPENDLOG;

    if (subJobInfo.subTaskType == SUBJOB_TYPE_DATACOPY_COPY_PHASE)
        backupParams.phase = BackupPhase::COPY_STAGE;
    else if (subJobInfo.subTaskType == SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE)
        backupParams.phase = BackupPhase::HARDLINK_STAGE;
    else if (subJobInfo.subTaskType == SUBJOB_TYPE_DATACOPY_DELETE_PHASE)
        backupParams.phase = BackupPhase::DELETE_STAGE;
    else if (subJobInfo.subTaskType == SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE)
        backupParams.phase = BackupPhase::DIR_STAGE;

    CommonParams commonParams {};
    commonParams.maxBufferCnt = NUMBER10;
    commonParams.maxBufferSize = BACKUP_MAX_BUF_SIZE; // 10kb
    commonParams.maxErrorFiles = NUMBER100;
    commonParams.backupDataFormat = m_isAggCopy ? BackupDataFormat::AGGREGATE : BackupDataFormat::NATIVE;
    commonParams.restoreReplacePolicy = m_coveragePolicy;
    commonParams.jobId = m_subjobId;
    commonParams.subJobId = m_subJobId;
    commonParams.blockSize = BACKUP_BLOCK_SIZE_4M;
    backupParams.commonParams = commonParams;
    backupParams.commonParams.metaPath = m_cacheMdPath;
    backupParams.commonParams.writeAcl = true;
    if (m_isAggCopy) {
        backupParams.commonParams.maxAggregateFileSize = std::stoul(m_aggCopyInfo.maxSizeAfterAggregate);
        backupParams.commonParams.maxFileSizeToAggregate = std::stoul(m_aggCopyInfo.maxSizeToAggregate);
    }

    return backupParams;
}

SubJobStatus::type HostArchiveRestore::MonitorRestore()
{
    HCP_Log(INFO, MODULE) << "Enter MonitorRestore" << HCPENDLOG;
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    do {
        m_backupStatus = m_backup->GetStatus();
        HCP_Log(INFO, MODULE) << "m_backupStatus:" << static_cast<int>(m_backupStatus) << HCPENDLOG;
        UpdateBackupStatistics();
        UpdateMainBackupStats();
        ReportBackupRunningStatus();
        if (!IsBackupStatusInprogress(jobStatus)) {
            HCP_Log(INFO, MODULE) << "m_backupStatus: " << static_cast<int>(m_backupStatus) << HCPENDLOG;
            break;
        }

        if (IsAbort()) {
            HCP_Log(INFO, MODULE) << "Backup - Abort is invocked for taskid: " << m_jobId
                << ", subtaskid: " << m_subJobId << HCPENDLOG;
            if (BackupRetCode::SUCCESS != m_backup->Abort()) {
                HCP_Log(ERR, MODULE) << "backup Abort is failed" << HCPENDLOG;
            }
            jobStatus = SubJobStatus::ABORTED;
            return jobStatus;
        }
        Module::SleepFor(std::chrono::seconds(EXECUTE_SUBTASK_MONITOR_DUR_IN_SEC));
    } while (true);
    UpdateBackupStatistics();
    UpdateMainBackupStats();
    ReportBackupRunningStatus();
    HCP_Log(INFO, MODULE) << "Exit Monitor Backup" << HCPENDLOG;
    return jobStatus;
}

bool HostArchiveRestore::UpdateMainBackupStats()
{
    BackupStatistic mainStats {};
    std::vector<std::string> statsList;
    if (!PluginUtils::GetFileListInDirectory(m_statsPath, statsList)) {
        ERRLOG("Get archive restore stats list failed, jobId: %s", m_jobId.c_str());
        return false;
    }
    BackupStatistic subStats;
    for (const string& path : statsList) {
        std::string::size_type pos = path.find(BACKUP_KEY_SUFFIX);
        if (pos == string::npos) {
            continue;
        }
        std::string subJobId = path.substr(m_statsPath.length() + NUMBER1, m_subJobId.length());
        DBGLOG("UpdateMainBackupStats, path: %s, jobId: %s, subJobId: %s",
            path.c_str(), m_jobId.c_str(), subJobId.c_str());
        ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, subJobId);
        bool ret = ShareResourceManager::GetInstance().QueryResource(path, subStats);
        ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, subJobId);
        DBGLOG("UpdateMainBackupStats, sub: %llu, speed: %llu, main: %llu, speed: %llu",
            subStats.noOfBytesCopied, subStats.backupspeed, mainStats.noOfBytesCopied, mainStats.backupspeed);
        if (!ret) {
            ERRLOG("Query failed, jobId: %s, subJobId: %s, path: %s", m_jobId.c_str(), path.c_str(), subJobId.c_str());
            return false;
        }
        mainStats = CalcuSumStructBackupStatistic(mainStats, subStats);
    }
    m_backupStats = mainStats;
    m_dataSize = m_backupStats.noOfBytesCopied / NUMBER1024;
    DBGLOG("copied dataSize: %llu", m_dataSize.load());
    PrintBackupStatistics(m_backupStats);
    return true;
}

bool HostArchiveRestore::UpdateBackupStatistics()
{
    BackupStats currStats = m_backup->GetStats();
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

bool HostArchiveRestore::ReportBackupRunningStatus()
{
    bool canReport = ShareResourceManager::GetInstance().CanReportStatToPM(m_jobId);
    if (!canReport) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        return true;
    }
    /* if any bytes copied or any file/dir copied, report concreate data */
    DBGLOG("ReportBackupRunningStatus, noOfBytesCopied: %d", m_backupStats.noOfBytesCopied);
    ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0),
                                        "nas_plugin_hetro_restore_data_inprogress_label", INITIAL_ERROR_CODE,
                                        std::to_string(m_backupStats.noOfDirCopied),
                                        std::to_string(m_backupStats.noOfFilesCopied),
                                        PluginUtils::FormatCapacity(m_backupStats.noOfBytesCopied));
    return true;
}

void HostArchiveRestore::PrintBackupStatistics(const BackupStatistic &backupStatistic)
{
    std::string backupStartTimeStr = PluginUtils::FormatTimeToStr(backupStatistic.startTime);

    INFOLOG("Backup jobId:             \t%s", m_jobId.c_str());
    INFOLOG("Backup subjobId:          \t%s", m_subJobId.c_str());
    INFOLOG("Backup status:            \t%d", static_cast<int>(m_backupStatus));
    INFOLOG("Backup start time:        \t%s", backupStartTimeStr.c_str());
    INFOLOG("Backup Speed:             \t%lu", backupStatistic.backupspeed);
    INFOLOG("Backup noOfDirToBackup:   \t%lu", backupStatistic.noOfDirToBackup);
    INFOLOG("Backup noOfFilesToBackup: \t%lu", backupStatistic.noOfFilesToBackup);
    INFOLOG("Backup noOfBytesToBackup: \t%lu", backupStatistic.noOfBytesToBackup);
    INFOLOG("Backup noOfDirToDelete:   \t%lu", backupStatistic.noOfDirToDelete);
    INFOLOG("Backup noOfFilesToDelete: \t%lu", backupStatistic.noOfFilesToDelete);
    INFOLOG("Backup noOfDirCopied:     \t%lu", backupStatistic.noOfDirCopied);
    INFOLOG("Backup noOfFilesCopied:   \t%lu", backupStatistic.noOfFilesCopied);
    INFOLOG("Backup noOfBytesCopied:   \t%lu", backupStatistic.noOfBytesCopied);
    INFOLOG("Backup noOfDirDeleted:    \t%lu", backupStatistic.noOfDirDeleted);
    INFOLOG("Backup noOfFilesDeleted:  \t%lu", backupStatistic.noOfFilesDeleted);
    INFOLOG("Backup noOfDirFailed:     \t%lu", backupStatistic.noOfDirFailed);
    INFOLOG("Backup noOfFilesFailed:   \t%lu", backupStatistic.noOfFilesFailed);
    INFOLOG("Backup noOfSrcRetryCount: \t%lu", backupStatistic.noOfSrcRetryCount);
    INFOLOG("Backup noOfDstRetryCount: \t%lu", backupStatistic.noOfDstRetryCount);
}

int HostArchiveRestore::PostJobInner()
{
    if (!InitInfo()) {
        return FAILED;
    }

    // 获取统计总量
    ShareResourceManager::GetInstance().SetResourcePath(m_statsPath, m_jobId);

    if (!InitArchiveClient()) {
        return FAILED;
    }

    if (m_archiveClient->EndRecover() != SUCCESS) {
        ERRLOG("Check disconnect archive server failed.");
        return FAILED;
    }

    if (m_archiveClient->Disconnect() != SUCCESS) {
        ERRLOG("Check disconnect archive server failed.");
        // post job no need to return failed
    }
    
    if (!UpdateMainBackupStats()) {
        ERRLOG("UpdateMainBackupStats failed");
        return FAILED;
    }
    INFOLOG("Exit PostJobInner");
    return SUCCESS;
}

bool HostArchiveRestore::InitMainResources()
{
    INFOLOG("Enter InitMainResources stats path:%s", m_statsPath.c_str());
    ShareResourceManager::GetInstance().SetResourcePath(m_statsPath, m_jobId);
    
    bool ret = ShareResourceManager::GetInstance().InitResource(ShareResourceType::SCAN, m_jobId, m_scanStats);
    if (!ret) {
        HCP_Log(ERR, MODULE) << "Init scan shared resourace failed" << HCPENDLOG;
        return ret;
    }
    return ret;
}

bool HostArchiveRestore::InitSubBackupJobResources()
{
    INFOLOG("Enter InitSubBackupJobResources");
    ShareResourceManager::GetInstance().SetResourcePath(m_statsPath, m_jobId);
    ShareResourceManager::GetInstance().SetResourcePath(m_statsPath, m_subJobId);
    bool ret = ShareResourceManager::GetInstance().InitResource(
        ShareResourceType::BACKUP, m_subJobId, m_subBackupStats);
    if (!ret) {
        HCP_Log(ERR, MODULE) << "Init sub backup shared resourace failed" << HCPENDLOG;
        return ret;
    }
    return ret;
}

bool HostArchiveRestore::QueryMainScanResources()
{
    INFOLOG("Enter QueryMainScanResources");
    ShareResourceManager::GetInstance().Wait(ShareResourceType::SCAN, m_jobId);
    bool ret = ShareResourceManager::GetInstance().QueryResource(ShareResourceType::SCAN, m_jobId, m_scanStats);
    ShareResourceManager::GetInstance().Signal(ShareResourceType::SCAN, m_jobId);
    if (!ret) {
        HCP_Log(ERR, MODULE) << "Query scan shared resourace failed" << HCPENDLOG;
    }
    return ret;
}

}
