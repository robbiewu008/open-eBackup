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
#ifndef DB_PLUGIN_H
#define DB_PLUGIN_H

#ifdef Win32
#include <winsock2.h>
#include <windows.h>
#endif

#include <string>
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "log/Log.h"
#include "CommonJobFactory.h"

#ifdef WIN32
#define DBPLUGIN_API __declspec(dllexport)
#else
#define DBPLUGIN_API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

DBPLUGIN_API mp_int32 AppInit(std::string &logPath);
DBPLUGIN_API void DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv);
DBPLUGIN_API void DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
    const Application& application);
DBPLUGIN_API void OracleCheckArchiveArea(ActionResult& _return,
    const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList);
DBPLUGIN_API JobFactoryBase* CreateFactory();

DBPLUGIN_API void CheckApplication(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application);
DBPLUGIN_API void ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application,
    const ApplicationResource& parentResource);
DBPLUGIN_API void ListApplicationResourceV2(ResourceResultByPage& returnValue,
    const ListResourceRequest& request);
DBPLUGIN_API void CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job);
DBPLUGIN_API void AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::BackupLimit::type limit);
DBPLUGIN_API void AllowBackupSubJobInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::SubJob& subJob);
DBPLUGIN_API void AllowRestoreInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job);
DBPLUGIN_API void AllowRestoreSubJobInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job,
    const AppProtect::SubJob& subJob);
DBPLUGIN_API void QueryJobPermission(AppProtect::JobPermission& returnJobPermission,
    const ApplicationEnvironment& appEnv, const Application& application);
DBPLUGIN_API void ListApplicationConfig(std::map<std::string, std::string>& returnValue, const std::string& script);
DBPLUGIN_API void DeliverTaskStatus(ActionResult& returnValue, const std::string& status, const std::string& taskId,
    const std::string& script);
DBPLUGIN_API void AllowCheckCopyInLocalNode(ActionResult& returnValue, const AppProtect::CheckCopyJob& job);
DBPLUGIN_API void AllowCheckCopySubJobInLocalNode(ActionResult& returnValue, const AppProtect::CheckCopyJob& job,
    const AppProtect::SubJob& subJob);
DBPLUGIN_API void RemoveProtect(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application);
#ifdef __cplusplus
}
#endif
#endif