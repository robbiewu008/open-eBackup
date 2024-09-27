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
#include "PluginMain.h"
#include <vector>
#include <unordered_set>
#include "log/Log.h"
#include "common/Path.h"
#include "common/Macros.h"
#include "common/Structs.h"
#include "common/utils/Utils.h"
#include "job_controller/factory/VirtualizationJobFactory.h"
#include "protect_engines/engine_factory/EngineFactory.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "config_reader/ConfigIniReader.h"
#include "config_reader/ConfigIniReaderImpl.h"
#include "utils/JsonTransUtil.h"
#ifndef WIN32
#include "inc/utils/Utils.h"
#else
#include "../../framework/inc/utils/Utils.h"
#include <repository_handlers/win32filesystem/Win32FileSystemHandler.h>
#endif

using namespace std;
using namespace VirtPlugin;
namespace {
static const std::string VIRTUAL_LOG_NAME = "VirtualPlugin.log";
const std::string MODULE_NAME = "PluginMain";
const std::string ENV_APPTYPE_HCS = "HCSContainer";
const std::string ENV_APPTYPE_HCSENVOP = "HcsEnvOp";
const uint64_t MIN_SEGMENT_THRESHOLD = 1024;       // MB
const uint64_t MAX_SEGMENT_THRESHOLD = 60 * 1024;
const int MAX_BACKUP_THREADS = 4;
const int MIN_BACKUP_THREADS = 1;
const int MIN_CREATE_SNAPSHOT_RETRY = 3;
const int MAX_CREATE_SNAPSHOT_RETRY = 20;
const int NOT_DELETE_SNAPSHOT_VOLUME = 0;
const int DELETE_SNAPSHOT_VOLUME = 1;
const int DEFAULT_CREATE_SNAPSHOT_RETRY = 10;
const int MIN_CREATE_SNAPSHOT_LIMIT = 1;
const int MAX_CREATE_SNAPSHOT_LIMIT = 30;
const int DEFAULT_CREATE_SNAPSHOT_LIMIT = 10;
const int MIN_RETRY_INTERVAL = 0;
const int MAX_RETRY_INTERVAL = 600;
const int DEFAULT_RETRY_INTERVAL = 30;
const int MIN_RETRY_COUNT = 0;
const int MAX_RETRY_COUNT = 300;
const int DEFAULT_RETRY_COUNT = 5;
const std::unordered_set<std::string> CMD_WHITE_LIST = {"sudo", "sed", "cp", "ps", "grep", "iscsiadm", "awk"};
const int MAX_TASK_AIO_MEM_IN_MB = 2048;
const int DEFAULT_TASK_AIO_MEM_IN_MB = 1024;
const int MIN_TASK_AIO_MEM_IN_MB = 128;
const int MIN_TIME_OUT_ONE_VOLUME = 86400;    // 24h
const int MAX_TIME_OUT_ONE_VOLUME = 86400 * 7;
const int MIN_RATE = 0;
const int MAX_RATE = 100;
const int DEFAULT_CPU_USAGE_RATE_LIMIT = 80;
const int DEFAULT_MEMORY_USAGE_RATE_LIMIT = 80;
const int DEFAULT_STORAGE_USAGE_RATE_LIMIT = 80;
const int DEFAULT_MAX_STORAGE = 80;
const int DEFAULT_WAIT_TIME = 20;
const int DEFAULT_RETRY_TIME = 20;
const int DEFAULT_MAX_PAGENUM = 500;
const int MAX_PAGENUM = 1000;
const int MIN_LENTH = 0;
const int DEFAULT_LENTH = 193;
const int MAX_LENTH = 1024;
}

EXTER_ATTACK VIRTUAL_PLUGIN_API JobFactoryBase* CreateFactory()
{
    DBGLOG("Enter virtualization plugin create factory.");
    return VirtualizationJobFactory::GetInstance();
}

void InitConfigItemsForAIO()
{
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("VOLUME_DATA_PROCESS", "MAX_TASK_AIO_THREADS",
        MIN_BACKUP_THREADS, MAX_BACKUP_THREADS, MAX_BACKUP_THREADS);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("VOLUME_DATA_PROCESS", "MAX_TASK_AIO_MEM_IN_MB",
        MIN_TASK_AIO_MEM_IN_MB, MAX_TASK_AIO_MEM_IN_MB, DEFAULT_TASK_AIO_MEM_IN_MB);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("VOLUME_DATA_PROCESS", "BACKUP_WITH_AIO",
        0, 1, 0);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("VOLUME_DATA_PROCESS", "RESTORE_WITH_AIO",
        0, 1, 0);
}

EXTER_ATTACK VIRTUAL_PLUGIN_API int32_t AppInit(std::string& logPath)
{
    std::string logFilePath = "";
    if (logPath.empty()) {
        logFilePath = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + VIRTUAL_LOG_PATH;
    } else {
        logFilePath = logPath;
    }
    int32_t iLogLevel = Module::ConfigReader::getInt(
        std::string(Module::MS_CFG_GENERAL_SECTION), std::string(Module::MS_CFG_LOG_LEVEL));
    int32_t iLogCount = Module::ConfigReader::getInt(
        std::string(Module::MS_CFG_GENERAL_SECTION), std::string(Module::MS_CFG_LOG_COUNT));
    int32_t iLogMaxSize = Module::ConfigReader::getInt(
        std::string(Module::MS_CFG_GENERAL_SECTION), std::string(Module::MS_CFG_LOG_MAX_SIZE));

    Module::CLogger::GetInstance().Init(VIRTUAL_LOG_NAME.c_str(), logFilePath, iLogLevel, iLogCount, iLogMaxSize);

    Module::ConfigReaderImpl::instance()->putStringConfigInfo("StorageModel", "OceanStorage", "61,62,63,64,68,69,70,"
        "71,72,73,74,82,84,85,86,87,88,89,90,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,112,113,114,"
        "115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,137,139,805,"
        "OceanStorV3,OceanStoreV3,OceanStorV5,OceanStorV6");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("StorageModel", "DoradoStorage", "811,812,813,814,815,"
        "816,817,818,819,821,822,823,824,825,826,827,828,829,830,831,832,833,834,835,836,837,838,839,840,1202,"
        "DoradoV3,DoradoV6");
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("VOLUME_DATA_PROCESS", "VOLUME_SEGMENT_THRESHOLD",
        MIN_SEGMENT_THRESHOLD, MAX_SEGMENT_THRESHOLD, MAX_SEGMENT_THRESHOLD);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("VOLUME_DATA_PROCESS", "MAX_BACKUP_THREADS",
        MIN_BACKUP_THREADS, MAX_BACKUP_THREADS, MAX_BACKUP_THREADS);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("HcsConfig", "CreateSnapshotApigwFailedRetry",
        MIN_CREATE_SNAPSHOT_RETRY, MAX_CREATE_SNAPSHOT_RETRY, DEFAULT_CREATE_SNAPSHOT_RETRY);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("HcsConfig", "CreateSnapshotLimit",
        MIN_CREATE_SNAPSHOT_LIMIT, MAX_CREATE_SNAPSHOT_LIMIT, DEFAULT_CREATE_SNAPSHOT_LIMIT);
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("HcsConfig", "ECSSupportBackupStatus",
        "active,stopped,suspended");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("HcsConfig", "ECSSupportRestoreStatus", "active,stopped");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("HcsConfig", "EVSSupportStatus", "in-use");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("General", "RecoverIgnoreBadBlock", "no");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("General", "DmiDecodeUUID", "");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("HcsConfig", "VdcUserRole", "vdcServiceManager");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("HcsConfig", "FusionStorageApiMode", "ISCSI");
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("HcsConfig", "LongestTimeBackUpOneVolume",
        MIN_TIME_OUT_ONE_VOLUME, MAX_TIME_OUT_ONE_VOLUME, MIN_TIME_OUT_ONE_VOLUME);
    InitConfigItemsForAIO();
    InitAppCfg();
#ifndef WIN32
    RegisterWhitelist();
#endif
    Utils::InitAndRegTracePoint();
    std::vector<std::string> configFiles {Module::EnvVarManager::GetInstance()->GetAgentHomePath() +
        VIRTUAL_CONF_PATH + VIRTUAL_CONF_NAME};
    Module::ConfigReaderImpl::instance()->refresh(configFiles);
    INFOLOG("Virtual log init success.");
    return SUCCESS;
}

void InitAppCfg()
{
    InitOpenStackCfg();
    InitApsaraStackCfg();
    InitCNwareCfg();
    InitHyperVCfg();
}

#ifndef WIN32
void RegisterWhitelist()
{
    std::unordered_set<std::string> pathWhiteList;
    std::string virtPluginPath = Module::EnvVarManager::GetInstance()->GetAgentHomePath() +
        "/DataBackup/ProtectClient/Plugins/VirtualizationPlugin/";
    pathWhiteList.insert(virtPluginPath + "vbstool");
    pathWhiteList.insert(virtPluginPath + "bin");
    pathWhiteList.insert(virtPluginPath + "conf");
    if (!Module::RegisterWhitelist(CMD_WHITE_LIST, pathWhiteList)) {
        ERRLOG("Register white list failed.");
    }
}
#endif

void InitHyperVCfg()
{
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("HyperVConfig", "PathLenthLimit",
        MIN_LENTH, MAX_LENTH, DEFAULT_LENTH);
}

void InitOpenStackCfg()
{
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "VMSupportBackupStatus",
        "active,stopped,suspended");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "VMSupportRestoreStatus",
        "active,stopped");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "VolSupportStatus",
        "in-use,available");
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("OpenStackConfig", "CreateSnapshotApigwFailedRetry",
        MIN_SEGMENT_THRESHOLD, MAX_SEGMENT_THRESHOLD, MAX_SEGMENT_THRESHOLD);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("OpenStackConfig", "CreateSnapshotLimit",
        MIN_CREATE_SNAPSHOT_LIMIT, MAX_CREATE_SNAPSHOT_LIMIT, DEFAULT_CREATE_SNAPSHOT_LIMIT);
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "DomainName", "default");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("MicroService", "ProcessRootPath",
        "/opt/DataBackup/ProtectClient/Plugins/VirtualizationPlugin");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "AdminRoleProject", "admin");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "AdminRoleDomain", "Default");
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("OpenStackConfig", "DeleteSnapshotVolume",
        NOT_DELETE_SNAPSHOT_VOLUME, DELETE_SNAPSHOT_VOLUME, DELETE_SNAPSHOT_VOLUME);
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "RegisterServiceToOpenStack", "false");
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("OpenStackConfig", "CreateVolumeWaitInterval",
        MIN_RETRY_INTERVAL, MAX_RETRY_INTERVAL, DEFAULT_RETRY_INTERVAL);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("OpenStackConfig", "CreateVolumeWaitRetryTimes",
        MIN_RETRY_COUNT, MAX_RETRY_COUNT, DEFAULT_RETRY_COUNT);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("OpenStackConfig", "CreateMachineWaitRetryTimes",
        MIN_RETRY_COUNT, MAX_RETRY_COUNT, DEFAULT_RETRY_COUNT);
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "CinderApiVersion",
        "volume 3.59");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("OpenStackConfig", "SupportCloneVolume", "false");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("General", "DataRepoPathBalance", "false");
}

void InitApsaraStackCfg()
{
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("ApsaraStackConfig", "SupportBackupStatus",
        "Running,Stopped");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("ApsaraStackConfig", "SupportRestoreStatus",
        "Running,Stopped");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("ApsaraStackConfig", "VolSupportStatus",
        "In_use,Available");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("ApsaraStackConfig", "CreateConsistenSnapshot",
        "false");
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("ApsaraStackConfig", "FakeDiskId",
        "Default");
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("ApsaraStackConfig", "CreateSnapshotWaitInterval",
        MIN_RETRY_INTERVAL, MAX_RETRY_INTERVAL, DEFAULT_WAIT_TIME);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("ApsaraStackConfig", "CreateSnapshotWaitRetryTimes",
        MIN_CREATE_SNAPSHOT_RETRY, MAX_CREATE_SNAPSHOT_RETRY, DEFAULT_RETRY_TIME);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("ApsaraStackConfig", "CreateDiskWaitInterval",
        MIN_RETRY_INTERVAL, MAX_RETRY_INTERVAL, DEFAULT_RETRY_INTERVAL);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("ApsaraStackConfig", "CreateDiskWaitRetryTimes",
        MIN_RETRY_COUNT, MAX_RETRY_COUNT, DEFAULT_RETRY_COUNT);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("ApsaraStackConfig", "DeleteSnapshotVolume",
        NOT_DELETE_SNAPSHOT_VOLUME, DELETE_SNAPSHOT_VOLUME, DELETE_SNAPSHOT_VOLUME);
}

void InitCNwareCfg()
{
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("CNwareConfig", "SupportVersion",
        "8.2.2,8.2.3,9.2,9.3");
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("CNwareConfig", "CpuLimit",
        MIN_RATE, MAX_RATE, DEFAULT_CPU_USAGE_RATE_LIMIT);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("CNwareConfig", "MemoryLimit",
        MIN_RATE, MAX_RATE, DEFAULT_MEMORY_USAGE_RATE_LIMIT);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("CNwareConfig", "StorageLimit",
        MIN_RATE, MAX_RATE, DEFAULT_STORAGE_USAGE_RATE_LIMIT);
    Module::ConfigReaderImpl::instance()->putIntConfigInfo("CNwareConfig", "RequestPageNums",
        MIN_RATE, MAX_PAGENUM, DEFAULT_MAX_PAGENUM);
    Module::ConfigReaderImpl::instance()->putStringConfigInfo("CNwareConfig", "DomainName", "default");
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void DiscoverApplications(std::vector<Application>& returnValue,
    const std::string& appType)
{
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(appType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    protectEngine->DiscoverApplications(returnValue, appType);
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void CheckApplication(ActionResult& returnValue, const ApplicationEnvironment& appEnv,
    const Application& application)
{
    Json::Value appEnvJv;
    StructToJson(appEnv, appEnvJv);
    Json::Value applicationJv;
    StructToJson(application, applicationJv);
    std::unordered_map<std::string, Json::Value> params = {
        {"ApplicationEnvironment", appEnvJv},
        {"Application", applicationJv}
    };
    if (!ParamCheck(params, returnValue)) {
        return;
    }
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(appEnv.subType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    protectEngine->CheckApplication(returnValue, appEnv, application);
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application, const ApplicationResource& parentResource)
{
    Json::Value appEnvJv;
    StructToJson(appEnv, appEnvJv);
    Json::Value applicationJv;
    StructToJson(application, applicationJv);
    Json::Value parentResourceJv;
    StructToJson(parentResource, parentResourceJv);
    std::unordered_map<std::string, Json::Value> params = {
        {"ApplicationEnvironment", appEnvJv},
        {"Application", applicationJv},
        {"ApplicationResource", parentResourceJv}
    };
    ParamCheck(params);
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(appEnv.subType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    protectEngine->ListApplicationResource(returnValue, appEnv, application, parentResource);
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void ListApplicationResourceV2(ResourceResultByPage &page,
    const ListResourceRequest &request)
{
    Json::Value requestJv;
    StructToJson(request, requestJv);
#ifndef WIN32
    ParamCheck("ListResourceRequest", requestJv);
#endif
    std:string appType = request.appEnv.subType;
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(appType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    protectEngine->ListApplicationResourceV2(page, request);
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void DiscoverHostCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv)
{
    Json::Value appEnvJv;
    StructToJson(appEnv, appEnvJv);
#ifndef WIN32
    ParamCheck("ApplicationEnvironment", appEnvJv);
#endif
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(appEnv.subType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    protectEngine->DiscoverHostCluster(returnEnv, appEnv);
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void DiscoverAppCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    Json::Value appEnvJv;
    StructToJson(appEnv, appEnvJv);
    Json::Value applicationJv;
    StructToJson(application, applicationJv);
    std::unordered_map<std::string, Json::Value> params = {
        {"ApplicationEnvironment", appEnvJv},
        {"Application", applicationJv}
    };
#ifndef WIN32
    ParamCheck(params);
#endif
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(appEnv.subType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    protectEngine->DiscoverAppCluster(returnEnv, appEnv, application);
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job)
{
    Json::Value jobJv;
    StructToJson(job, jobJv);
#ifndef WIN32
    if (!ParamCheck("BackupJob", jobJv, returnValue)) {
        return;
    }
#endif
    returnValue.__set_code(INNER_ERROR);
    if (job.jobParam.backupType == AppProtect::BackupJobType::FULL_BACKUP) {
        returnValue.__set_code(SUCCESS);
        return;
    }
    std::shared_ptr<ProtectEngine> protectEg = EngineFactory::CreateProtectEngineWithoutTask(job.protectEnv.subType);
    if (protectEg.get() == nullptr) {
        return;
    }
    std::string metaRepoPath;
    std::shared_ptr<RepositoryHandler> metaRepoHandler = nullptr;
    if (Utils::GetMetaRepoPath(metaRepoPath, metaRepoHandler, job) != SUCCESS) {
        return;
    }
    SnapshotInfo preSnapshotInfo;
    std::string preSnapFile = metaRepoPath + VIRT_PLUGIN_PRE_SNAPSHOT_INFO;
    if (Utils::LoadFileToStruct(metaRepoHandler, preSnapFile, preSnapshotInfo) != SUCCESS) {
        WARNLOG("Load pre snapshot info failed,convert to full backup.");
        returnValue.__set_bodyErr(BACKUP_INC_TO_FULL);
        return;
    }
    JobTypeParam param = {job, preSnapshotInfo};
    bool checkRet = false;
    if ((protectEg->CheckBackupJobType(param, checkRet) != SUCCESS) ||
        (CheckCopyVerifyInformation(metaRepoHandler, job, metaRepoPath, checkRet) != SUCCESS)) {
        return;
    }
    if (!checkRet) {
        returnValue.__set_bodyErr(BACKUP_INC_TO_FULL); // Specified error code, ubc will do INC to FULL
        return;
    }
    returnValue.__set_code(SUCCESS);
    return;
}

int CheckCopyVerifyInformation(const std::shared_ptr<RepositoryHandler> &metaRepoHandler,
                               const AppProtect::BackupJob &job, const std::string &metaRepoPath, bool &checkRet)
{
    bool isCopyVerify = false;
    if (!job.extendInfo.empty()) {
        Json::Value extendInfo;
        if (!Module::JsonHelper::JsonStringToJsonValue(job.extendInfo, extendInfo)) {
            ERRLOG("Trans job extend info to json value failed");
            return FAILED;
        }
        if (extendInfo.isMember("copy_verify") && extendInfo["copy_verify"].asString() == "true") {
            INFOLOG("Generate sha256 file is true.");
            isCopyVerify = true;
        }
    }
    // 增量备份：开启生成校验文件。
    // 1.存在中断校验文件 --转全备；
    // 2.不存在中断校验文件且不存在校验文件信息 --转全备

    // 增量备份：未启生成校验文件。存在校验文件信息 --生成中断校验文件； --在backupJob GenShaInterruptFile中
    if (isCopyVerify) {
        std::string shaStateFilePath = metaRepoPath + VIRT_PLUGIN_SHA_INTERRUPT_INFO;
        bool IsShaInterruptFileExist = metaRepoHandler->Exists(shaStateFilePath);
        if (IsShaInterruptFileExist) {
            WARNLOG("Verify file is interrupted before the current backup,convert to full backup.");
            checkRet = false;
        } else if (!IsSha256FileExist(metaRepoHandler, metaRepoPath)) {
            WARNLOG("The previous verify file does not exist,convert to full backup.");
            checkRet = false;
        }
    }
    return SUCCESS;
}

bool IsSha256FileExist(const std::shared_ptr<RepositoryHandler> &metaRepoHandler, const std::string &metaRepoPath)
{
    bool result = false;
    std::string shaPathName = metaRepoPath + VIRT_PLUGIN_SHA_FILE_ROOT;
    std::vector <std::string> fileNames{};
    metaRepoHandler->GetFiles(shaPathName, fileNames);
    for (auto fileName: fileNames) {
        if (fileName.find("_sha256.info") != std::string::npos) {
            result = true;
            INFOLOG("Find the sha 256 file.");
            break;
        }
    }
    return result;
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::BackupLimit::type& limit)
{
    Json::Value jobJv;
    StructToJson(job, jobJv);
#ifndef WIN32
    if (!ParamCheck("BackupJob", jobJv, returnValue)) {
        return;
    }
#endif
    returnValue.__set_code(INNER_ERROR);
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(
        job.protectEnv.subType);
    if (protectEngine.get() == nullptr) {
        ERRLOG("ProtectEngine is nullptr.");
        return;
    }
    int32_t errorCode = SUCCESS;
    if (protectEngine->AllowBackupInLocalNode(job, errorCode) != SUCCESS) {
        ERRLOG("Can not allow backup in local node. Job type:%s", job.protectEnv.subType.c_str());
        if (job.protectEnv.subType == ENV_APPTYPE_HCS || job.protectEnv.subType == ENV_APPTYPE_HCSENVOP) {
            std::vector<std::string> certParams;
            protectEngine->SetErrorCodeParam(errorCode, certParams);
            if (!certParams.empty()) {
                returnValue.__set_bodyErrParams(certParams);
            }
        }
        returnValue.__set_bodyErr(errorCode);
        return;
    }
    returnValue.__set_code(SUCCESS);
    return;
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowRestoreInLocalNode(ActionResult& returnValue,
    const AppProtect::RestoreJob& job)
{
    Json::Value jobJv;
    StructToJson(job, jobJv);
#ifndef WIN32
    if (!ParamCheck("RestoreJob", jobJv, returnValue)) {
        return;
    }
#endif
    returnValue.__set_code(INNER_ERROR);
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(
        job.targetEnv.subType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    int32_t errorCode = SUCCESS;
    if (protectEngine->AllowRestoreInLocalNode(job, errorCode) != SUCCESS) {
        ERRLOG("Can not allow recover in local node. Job type:%s", job.targetEnv.subType.c_str());
        if (job.targetEnv.subType == ENV_APPTYPE_HCS || job.targetEnv.subType == ENV_APPTYPE_HCSENVOP) {
            std::vector<std::string> certParams;
            protectEngine->SetErrorCodeParam(errorCode, certParams);
            if (!certParams.empty()) {
                returnValue.__set_bodyErrParams(certParams);
            }
        } else {
            returnValue.__set_bodyErrParams(protectEngine->GetNoTasksArgs());
        }
        returnValue.__set_bodyErr(errorCode);
        return;
    }
    returnValue.__set_code(SUCCESS);
    return;
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowBackupSubJobInLocalNode(ActionResult& returnValue,
    const AppProtect::BackupJob& job, const AppProtect::SubJob& subJob)
{
    Json::Value jobJv;
    StructToJson(job, jobJv);
    Json::Value subJobJv;
    StructToJson(subJob, subJobJv);
    std::unordered_map<std::string, Json::Value> params = {
        {"BackupJob", jobJv},
        {"SubJob", subJobJv}
    };
#ifndef WIN32
    if (!ParamCheck(params, returnValue)) {
        return;
    }
#endif
    returnValue.__set_code(INNER_ERROR);
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(
        job.protectEnv.subType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    int32_t errorCode = SUCCESS;
    if (protectEngine->AllowBackupSubJobInLocalNode(job, subJob, errorCode) != SUCCESS) {
        ERRLOG("Can not allow backup in local node.");
        if (job.protectEnv.subType == ENV_APPTYPE_HCS || job.protectEnv.subType == ENV_APPTYPE_HCSENVOP) {
            std::vector<std::string> certParams;
            protectEngine->SetErrorCodeParam(errorCode, certParams);
            if (!certParams.empty()) {
                returnValue.__set_bodyErrParams(certParams);
            }
        } else {
            returnValue.__set_bodyErrParams(protectEngine->GetNoTasksArgs());
        }
        returnValue.__set_bodyErr(errorCode);
        return;
    }
    returnValue.__set_code(SUCCESS);
    return;
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowRestoreSubJobInLocalNode(ActionResult& returnValue,
    const AppProtect::RestoreJob& job, const AppProtect::SubJob& subJob)
{
    Json::Value jobJv;
    StructToJson(job, jobJv);
    Json::Value subJobJv;
    StructToJson(subJob, subJobJv);
    std::unordered_map<std::string, Json::Value> params = {
        {"RestoreJob", jobJv},
        {"SubJob", subJobJv}
    };
#ifndef WIN32
    if (!ParamCheck(params, returnValue)) {
        return;
    }
#endif
    returnValue.__set_code(INNER_ERROR);
    std::shared_ptr<ProtectEngine> protectEngine = EngineFactory::CreateProtectEngineWithoutTask(
        job.targetEnv.subType);
    if (protectEngine.get() == nullptr) {
        return;
    }
    int32_t errorCode = SUCCESS;
    if (protectEngine->AllowRestoreSubJobInLocalNode(job, subJob, errorCode) != SUCCESS) {
        ERRLOG("Can not allow restore in local node.");
        if (job.targetEnv.subType == ENV_APPTYPE_HCS || job.targetEnv.subType == ENV_APPTYPE_HCSENVOP) {
            std::vector<std::string> certParams;
            protectEngine->SetErrorCodeParam(errorCode, certParams);
            if (!certParams.empty()) {
                returnValue.__set_bodyErrParams(certParams);
            }
        } else {
            returnValue.__set_bodyErrParams(protectEngine->GetNoTasksArgs());
        }
        returnValue.__set_bodyErr(errorCode);
        return;
    }
    returnValue.__set_code(SUCCESS);
    return;
}

EXTER_ATTACK VIRTUAL_PLUGIN_API void QueryJobPermission(AppProtect::JobPermission& returnJobPermission,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    return;
}
EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowCheckCopyInLocalNode(ActionResult& returnValue,
    const AppProtect::CheckCopyJob& job)
{
    returnValue.__set_code(SUCCESS);
    return;
}
EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowCheckCopySubJobInLocalNode(ActionResult& returnValue,
    const AppProtect::CheckCopyJob& job, const AppProtect::SubJob& subJob)
{
    returnValue.__set_code(SUCCESS);
    return;
}