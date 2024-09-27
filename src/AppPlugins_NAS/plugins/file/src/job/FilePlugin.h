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
#ifndef FILE_PLUGIN_H
#define FILE_PLUGIN_H
#include <string>
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "log/Log.h"
#include "CommonJobFactory.h"
#ifndef WIN32
#define FILEPLUGIN_API __attribute__ ((visibility("default")))
#else
#define FILEPLUGIN_API __declspec(dllexport)
#endif
#ifdef __cplusplus

extern "C" {
#endif

FILEPLUGIN_API int AppInit(std::string &logPath);
FILEPLUGIN_API void DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv);
FILEPLUGIN_API void DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
    const Application& application);
FILEPLUGIN_API JobFactoryBase* CreateFactory();

FILEPLUGIN_API void CheckApplication(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application);
FILEPLUGIN_API void ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application,
    const ApplicationResource& parentResource);
FILEPLUGIN_API void ListApplicationResourceV2(ResourceResultByPage& returnValue, const ListResourceRequest& request);
FILEPLUGIN_API void AbortJob(ActionResult& returnValue, const std::string& jobId,
    const std::string& subJobId, const std::string& appType);
FILEPLUGIN_API void PauseJob(ActionResult& returnValue, const std::string& jobId,
    const std::string& subJobId, const std::string& appType);
FILEPLUGIN_API void CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job);
FILEPLUGIN_API void AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::BackupLimit::type limit);
FILEPLUGIN_API void AllowBackupSubJobInLocalNode(
    ActionResult& returnValue, const AppProtect::BackupJob& job, const AppProtect::SubJob& subJob);
FILEPLUGIN_API void AllowRestoreInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job);
FILEPLUGIN_API void AllowRestoreSubJobInLocalNode(
    ActionResult& returnValue, const AppProtect::RestoreJob& job, const AppProtect::SubJob& subJob);
FILEPLUGIN_API void QueryJobPermission(AppProtect::JobPermission& returnJobPermission, const Application& application);
#ifdef __cplusplus
}
#endif
#endif