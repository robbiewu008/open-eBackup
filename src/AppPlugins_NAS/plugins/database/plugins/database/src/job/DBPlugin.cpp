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
#include "DBPlugin.h"
#include "ClusterOperation.h"
#include "CommonBackupService.h"
#include "common/Path.h"
#include "JobControl.h"
#include "DBPluginPath.h"
#include "config_reader/ConfigIniReader.h"

using namespace GeneralDB;
namespace {
static const mp_string GENERALDN_LOG_NAME = "generaldbplugin.log";
static const mp_string GENERAL_LOG_PATH = "/opt/DataBackup/ProtectClient/ProtectClient-E/slog/GeneralDBPlugin/log";
}

DBPLUGIN_API mp_int32 AppInit(std::string &logPath)
{
    std::string logFilePath;
    if (logPath.empty()) {
        logFilePath = GENERAL_LOG_PATH;
    } else {
        logFilePath = logPath;
    }
    mp_int32 iLogLevel = Module::ConfigReader::getInt("General", "LogLevel");
    mp_int32 iLogCount = Module::ConfigReader::getInt("General", "LogCount");
    mp_int32 iLogMaxSize = Module::ConfigReader::getInt("General", "LogMaxSize");
    Module::CLogger::GetInstance().Init(
        GENERALDN_LOG_NAME.c_str(), logFilePath, iLogLevel, iLogCount, iLogMaxSize);
    if (DBPluginPath::GetInstance()->SetDBPluginPath(logPath) != MP_SUCCESS) {
        return MP_FAILED;
    }
    HCP_Log(INFO, "AppInit") << "App init success." << HCPENDLOG;
    return MP_SUCCESS;
}

DBPLUGIN_API void DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv)
{
    return ClusterOperation::DiscoverHostCluster(returnEnv, appEnv);
}

DBPLUGIN_API void DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
    const Application& application)
{
    return ClusterOperation::DiscoverAppCluster(returnEnv, appEnv, application);
}

DBPLUGIN_API void OracleCheckArchiveArea(ActionResult& _return,
    const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList)
{
    return ClusterOperation::OracleCheckArchiveArea(_return, appType, dbInfoList);
}

DBPLUGIN_API JobFactoryBase* CreateFactory()
{
    return CommonJobFactory::GetInstance();
}

DBPLUGIN_API void CheckApplication(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    return ClusterOperation::CheckApplication(returnValue, appEnv, application);
}

DBPLUGIN_API void ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application,
    const ApplicationResource& parentResource)
{
    return ClusterOperation::ListApplicationResource(returnValue, appEnv, application, parentResource);
}

DBPLUGIN_API void ListApplicationResourceV2(ResourceResultByPage& returnValue,
    const ListResourceRequest& request)
{
    return ClusterOperation::ListApplicationResourceV2(returnValue, request);
}
DBPLUGIN_API void CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job)
{
    return CommonBackupService::CheckBackupJobType(returnValue, job);
}

DBPLUGIN_API void AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::BackupLimit::type limit)
{
    return CommonBackupService::AllowBackupInLocalNode(returnValue, job, limit);
}

DBPLUGIN_API void AllowBackupSubJobInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::SubJob& subJob)
{
    return CommonBackupService::AllowBackupSubJobInLocalNode(returnValue, job, subJob);
}

DBPLUGIN_API void AllowRestoreInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job)
{
    return CommonBackupService::AllowRestoreInLocalNode(returnValue, job);
}

DBPLUGIN_API void AllowRestoreSubJobInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job,
    const AppProtect::SubJob& subJob)
{
    return CommonBackupService::AllowRestoreSubJobInLocalNode(returnValue, job, subJob);
}

DBPLUGIN_API void QueryJobPermission(AppProtect::JobPermission& returnJobPermission,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    return CommonBackupService::QueryJobPermission(returnJobPermission, appEnv, application);
}

DBPLUGIN_API void ListApplicationConfig(std::map<std::string, std::string>& returnValue, const std::string& script)
{
    return ClusterOperation::ListApplicationConfig(returnValue, script);
}

DBPLUGIN_API void DeliverTaskStatus(ActionResult& returnValue, const std::string& status, const std::string& taskId,
    const std::string& script)
{
    return CommonBackupService::DeliverTaskStatus(returnValue, status, taskId, script);
}

DBPLUGIN_API void AllowCheckCopyInLocalNode(ActionResult& returnValue, const AppProtect::CheckCopyJob& job)
{
    return CommonBackupService::AllowCheckCopyInLocalNode(returnValue, job);
}

DBPLUGIN_API void AllowCheckCopySubJobInLocalNode(ActionResult& returnValue, const AppProtect::CheckCopyJob& job,
    const AppProtect::SubJob& subJob)
{
    return CommonBackupService::AllowCheckCopySubJobInLocalNode(returnValue, job, subJob);
}

DBPLUGIN_API void RemoveProtect(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    return ClusterOperation::RemoveProtect(returnValue, appEnv, application);
}