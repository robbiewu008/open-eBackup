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
#include "FilePlugin.h"
#include "file_resource/AppService.h"
#include "Module/src/common/Path.h"
#include "common/EnvVarManager.h"
#include "define/Types.h"
#include "utils/PluginUtilities.h"
#include "config_reader/ConfigIniReader.h"
#include "host_backup/HostBackup.h"
#ifdef __linux__
#include "volume_backup/VolumeBackup.h"
#endif

using namespace std;
using namespace AppProtect;
using namespace Module;
using namespace FilePlugin;

namespace {
const std::string FILESET_STR = "Fileset";
const std::string VOLUME_STR = "Volume";
static const std::string GENERALDN_LOG_NAME = "AppPlugins.log";
/* agent安装目录的相对路径，使用时需要在前面加上agent的安装目录 */
#ifdef WIN32
const std::string DEFAULT_GENERAL_LOG_PATH = "/DataBackup/ProtectClient/ProtectClient-E/slog/FilePlugin";
#else
const std::string DEFAULT_GENERAL_LOG_PATH = R"(\DataBackup\ProtectClient\ProtectClient-E\log\Plugins\FilePlugin)";
#endif

const string MODULE = "commonJobFactory";
const std::string FILE_PLUGIN_CONFIG_KEY = "FilePluginConfig";
constexpr uint64_t BACKUP_INC_TO_FULL = 1577209901;
const int MIN_CTRLFILE_SIZE = 1024 * 1024;
const int MAX_CTRLFILE_SIZE = 100 * 1024 * 1024;
const int DEFAULT_CTRLFILE_SIZE = 4 * 1024 * 1024;
const int MIN_CTRLENTRIES_MAX_SIZE = 10000;
const int MAX_CTRLENTRIES_MAX_SIZE = 1000000;
const int DEFAULT_CTRLENTRIES_MAX_SIZE = 100000;
const int MIN_CTRLENTRIES_MIN_SIZE = 1000;
const int MAX_CTRLENTRIES_MIN_SIZE = 100000;
const int DEFAULT_CTRLENTRIES_MIN_SIZE = 10000;
const int MIN_THREAD_NUM = 1;
const int MAX_THREAD_NUM = 64;
const int DEFAULT_THREAD_NUM = 8;
const int MIN_MEMORY = 20 * 1024 * 1024;
const int MAX_MEMORY = 200 * 1024 * 1024;
const int DEFAULT_MEMORY = 50 * 1024 * 1024;
const int BACKUP_STUCK_TIME_MIN = 120;
const int BACKUP_STUCK_TIME_MAX = 480;
const int BACKUP_STUCK_TIME_DEFAULT = 300;
const int ADD_NEW_SUBJOB_STUCK_TIME_MIN = 3600;
const int ADD_NEW_SUBJOB_STUCK_TIME_MAX = 864000;
const int ADD_NEW_SUBJOB_STUCK_TIME_DEFAULT = 86400;
const int SCAN_CONCURRENT_COUNT_MIN = 1;
const int SCAN_CONCURRENT_COUNT_MAX = 20;
const int SCAN_CONCURRENT_COUNT_DEFAULT = 1;
const int SNAP_WAIT_TIME_MIN = 0;
const int SNAP_WAIT_TIME_MAX = 60000;
const int SNAP_WAIT_TIME_DEFAULT = 30;
const std::string LVM_SNAPSHOT_CAPACITY_PERCENT_DEFAULT = "5";
const std::string KEEP_RFI_IN_CACHE_REPO_DEFAULT = "0";
const uint64_t MIN_SCAN_META_FILE_SIZE = 10 * 1024 * 1024; // 10MB
const uint64_t MAX_SCAN_META_FILE_SIZE = 10 * 1024 * 1024 * 1024; // 10GB
const uint64_t DEFAULT_SCAN_META_FILE_SIZE = 100 * 1024 * 1024; // 100MB
const uint64_t MAX_SCAN_WRITE_QUEUE_SIZE = 100000;
const uint64_t DEFAULT_SCAN_WRITE_QUEUE_SIZE = 100;

// default hcpconf.ini config
void InitConfigInfoForFilePlugin1()
{
    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "PosixCopyCtrlFileSize", MIN_CTRLFILE_SIZE, MAX_CTRLFILE_SIZE, DEFAULT_CTRLFILE_SIZE);
    ConfigReader::setStringConfigInfo(FILE_PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlDataSize", "1073741824");
    ConfigReader::setStringConfigInfo(FILE_PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlDataSize", "536870912");

    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlEntriesFullBackup",
        MIN_CTRLENTRIES_MAX_SIZE, MAX_CTRLENTRIES_MAX_SIZE, DEFAULT_CTRLENTRIES_MAX_SIZE);
    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlEntriesFullBackup",
        MIN_CTRLENTRIES_MIN_SIZE, MAX_CTRLENTRIES_MIN_SIZE, DEFAULT_CTRLENTRIES_MIN_SIZE);
    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlEntriesIncBackup",
        MIN_CTRLENTRIES_MAX_SIZE, MAX_CTRLENTRIES_MAX_SIZE, DEFAULT_CTRLENTRIES_MAX_SIZE);
    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlEntriesIncBackup",
        MIN_CTRLENTRIES_MIN_SIZE, MAX_CTRLENTRIES_MIN_SIZE, DEFAULT_CTRLENTRIES_MIN_SIZE);

    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "PosixReaderThreadNum", MIN_THREAD_NUM, MAX_THREAD_NUM, DEFAULT_THREAD_NUM);
    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "PosixWriterThreadNum", MIN_THREAD_NUM, MAX_THREAD_NUM, DEFAULT_THREAD_NUM);
    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "PosixAggregatorThreadNum", MIN_THREAD_NUM, MAX_THREAD_NUM, DEFAULT_THREAD_NUM);
    ConfigReader::setIntConfigInfo(FILE_PLUGIN_CONFIG_KEY, "PosixMaxMemory", MIN_MEMORY, MAX_MEMORY, DEFAULT_MEMORY);

    ConfigReader::setIntConfigInfo(FILE_PLUGIN_CONFIG_KEY, "ScanWriteQueueSize",
        1, MAX_SCAN_WRITE_QUEUE_SIZE, DEFAULT_SCAN_WRITE_QUEUE_SIZE);
    ConfigReader::setIntConfigInfo(FILE_PLUGIN_CONFIG_KEY, "ScanDefaultMetaFileSize",
        MIN_SCAN_META_FILE_SIZE, MAX_SCAN_META_FILE_SIZE, DEFAULT_SCAN_META_FILE_SIZE);
    ConfigReader::setIntConfigInfo(FILE_PLUGIN_CONFIG_KEY, "ScanProducerThreadCount",
        MIN_THREAD_NUM, MAX_THREAD_NUM, DEFAULT_THREAD_NUM);
}

void InitConfigInfoForFilePlugin2()
{
    ConfigReader::setStringConfigInfo(FILE_PLUGIN_CONFIG_KEY, "SOLARISExcludePathList", "");
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "ExcludeFileSystemList", EXCLUDE_FILESYSTEM_LIST_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "Win32ExcludePathList", WIN32_EXCLUDE_PATH_LIST_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "LinuxExcludePathList", LINUX_EXCLUDE_PATH_LIST_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "AIXExcludePathList", AIX_EXCLUDE_PATH_LIST_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "SOLARISExcludePathList", SOLARIS_EXCLUDE_PATH_LIST_DEFAULT);
 
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "Win32SnapshotParentPath", WIN32_SNAPSHOT_PARENT_PATH_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "LinuxSnapshotParentPath", LINUX_SNAPSHOT_PARENT_PATH_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "AIXSnapshotParentPath", AIX_SNAPSHOT_PARENT_PATH_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "SOLARISSnapshotParentPath", SOLARIS_SNAPSHOT_PARENT_PATH_DEFAULT);

    ConfigReader::setIntConfigInfo(FILE_PLUGIN_CONFIG_KEY, "BACKUP_STUCK_TIME",
        BACKUP_STUCK_TIME_MIN, BACKUP_STUCK_TIME_MAX, BACKUP_STUCK_TIME_DEFAULT);
    ConfigReader::setIntConfigInfo(FILE_PLUGIN_CONFIG_KEY, "ADD_NEW_SUBJOB_STUCK_TIME",
        ADD_NEW_SUBJOB_STUCK_TIME_MIN, ADD_NEW_SUBJOB_STUCK_TIME_MAX, ADD_NEW_SUBJOB_STUCK_TIME_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "LVM_SNAPSHOT_CAPACITY_PERCENT", LVM_SNAPSHOT_CAPACITY_PERCENT_DEFAULT);
    ConfigReader::setStringConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "KEEP_RFI_IN_CACHE_REPO", KEEP_RFI_IN_CACHE_REPO_DEFAULT);
    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "RESTORE_SUBJOB_IGNORE_FAILED", 0, 1, 1);
    ConfigReader::setIntConfigInfo(
        FILE_PLUGIN_CONFIG_KEY, "BACKUP_READ_FAILED_DISCARD", 0, 1, 0);
    ConfigReader::setIntConfigInfo(FILE_PLUGIN_CONFIG_KEY, "SCAN_CONCURRENT_COUNT",
        SCAN_CONCURRENT_COUNT_MIN, SCAN_CONCURRENT_COUNT_MAX, SCAN_CONCURRENT_COUNT_DEFAULT);
    ConfigReader::setIntConfigInfo(FILE_PLUGIN_CONFIG_KEY, "FORCE_DISABLE_ACL", 0, 1, 0);
    umask(0);
}
}

FILEPLUGIN_API int AppInit(std::string &logPath)
{
    string logFilePath;
    if (logPath.empty()) {
        logFilePath = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + DEFAULT_GENERAL_LOG_PATH;
    } else {
        logFilePath = logPath;
    }
    int logLevel = Module::ConfigReader::getInt("General", "LogLevel");
    int logCount = Module::ConfigReader::getInt("General", "LogCount");
    int logMaxSize = Module::ConfigReader::getInt("General", "LogMaxSize");

    CLogger::GetInstance().Init(GENERALDN_LOG_NAME.c_str(), logFilePath);
    Module::CLogger::GetInstance().SetLogConf(logLevel, logCount, logMaxSize);

    InitConfigInfoForFilePlugin1();
    InitConfigInfoForFilePlugin2();
    HCP_Log(INFO, "AppInit") << "App init success." << HCPENDLOG;
    return Module::SUCCESS;
}

FILEPLUGIN_API void DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv)
{
    return;
}

FILEPLUGIN_API void DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
    const Application& application)
{
    return;
}

FILEPLUGIN_API JobFactoryBase* CreateFactory()
{
    HCP_Log(INFO, MODULE) << "Enter file plugin create factory." << HCPENDLOG;
    return CommonJobFactory::GetInstance();
}

FILEPLUGIN_API void CheckApplication(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    return;
}

FILEPLUGIN_API void ListApplicationResource(vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application,
    const ApplicationResource& parentResource)
{
    return AppServiceExport::ListApplicationResource(returnValue, appEnv, application, parentResource);
}

FILEPLUGIN_API void ListApplicationResourceV2(ResourceResultByPage& returnValue, const ListResourceRequest& request)
{
    AppServiceExport::ListApplicationResourceV2(returnValue, request);
}

FILEPLUGIN_API void AbortJob(ActionResult& returnValue, const string& jobId,
    const string& subJobId, const string& appType)
{
    return;
}

FILEPLUGIN_API void PauseJob(ActionResult& returnValue, const string& jobId,
    const string& subJobId, const string& appType)
{
    return;
}

FILEPLUGIN_API void CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job)
{
    INFOLOG("Enter file plugin CheckBackupJobType.");
    int ret = Module::SUCCESS;
    auto jobCommonInfoPtr = make_shared<JobCommonInfo>(make_shared<BackupJob>(job));
    std::string appType = job.protectObject.subType;
    if (appType == VOLUME_STR) {
#ifdef __linux__
        auto jobptr = std::make_shared<VolumeBackup>();
        jobptr->SetJobInfo(jobCommonInfoPtr);
        ret = jobptr->CheckBackupJobType();
#else
    ERRLOG("Volume backup is not implemented on this platform");
#endif
    } else if (appType == FILESET_STR) {
        auto jobptr = std::make_shared<HostBackup>();
        jobptr->SetJobInfo(jobCommonInfoPtr);
        ret = jobptr->CheckBackupJobType();
    }
    if (ret != Module::SUCCESS) {
        DBGLOG("Exit CheckBackupJobType, report BACKUP_INC_TO_FULL");
        returnValue.__set_bodyErr(BACKUP_INC_TO_FULL); // Specified error code, ubc will do INC to FULL
        returnValue.__set_code(INNER_ERROR);
        return;
    }
    returnValue.__set_code(0);
    INFOLOG("Exit file plugin CheckBackupJobType.");
    return;
}

FILEPLUGIN_API void AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::BackupLimit::type limit)
{
    HCP_Log(INFO, MODULE) << "Enter file plugin AllowBackupInLocalNode." << HCPENDLOG;
    returnValue.__set_code(0);
    return ;
}

FILEPLUGIN_API void AllowBackupSubJobInLocalNode(
    ActionResult& returnValue, const AppProtect::BackupJob& job, const AppProtect::SubJob& subJob)
{
    HCP_Log(INFO, MODULE) << "Enter file plugin AllowBackupSubJobInLocalNode." << HCPENDLOG;
    returnValue.__set_code(0);
    return ;
}

FILEPLUGIN_API void AllowRestoreInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job)
{
    HCP_Log(INFO, MODULE) << "Enter file plugin AllowRestoreInLocalNode." << HCPENDLOG;
    returnValue.__set_code(0);
    return ;
}

FILEPLUGIN_API void AllowRestoreSubJobInLocalNode(
    ActionResult& returnValue, const AppProtect::RestoreJob& job, const AppProtect::SubJob& subJob)
{
    HCP_Log(INFO, MODULE) << "Enter file plugin AllowRestoreSubJobInLocalNode." << HCPENDLOG;
    returnValue.__set_code(0);
    return ;
}

FILEPLUGIN_API void QueryJobPermission(AppProtect::JobPermission& returnJobPermission, const Application& application)
{
    return ;
}