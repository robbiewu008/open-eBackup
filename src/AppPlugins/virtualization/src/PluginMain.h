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
#ifndef PLUGIN_MAIN_H
#define PLUGIN_MAIN_H
#include <string>
#include "job/JobFactoryBase.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "define/Defines.h"

#ifndef EXTER_ATTACK
#define EXTER_ATTACK
#endif

using AppProtect::ApplicationEnvironment;
using AppProtect::ResourceResultByPage;
using AppProtect::QueryByPage;
using AppProtect::Application;
using AppProtect::ApplicationResource;
using AppProtect::ActionResult;

#ifndef WIN32
#define VIRTUAL_PLUGIN_API __attribute__ ((visibility("default")))
#else
#define VIRTUAL_PLUGIN_API __declspec(dllexport)
#endif
#ifdef __cplusplus
extern "C" {
#endif
EXTER_ATTACK VIRTUAL_PLUGIN_API JobFactoryBase* CreateFactory();
EXTER_ATTACK VIRTUAL_PLUGIN_API int32_t AppInit(std::string &logPath);
EXTER_ATTACK VIRTUAL_PLUGIN_API void DiscoverApplications(std::vector<Application>& returnValue,
    const std::string& appType);
EXTER_ATTACK VIRTUAL_PLUGIN_API void CheckApplication(ActionResult& returnValue, const ApplicationEnvironment& appEnv,
    const Application& application);
EXTER_ATTACK VIRTUAL_PLUGIN_API void ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application, const ApplicationResource& parentResource);
EXTER_ATTACK VIRTUAL_PLUGIN_API void ListApplicationResourceV2(ResourceResultByPage &page,
    const ListResourceRequest &request);
EXTER_ATTACK VIRTUAL_PLUGIN_API void DiscoverHostCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv);
EXTER_ATTACK VIRTUAL_PLUGIN_API void DiscoverAppCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv, const Application& application);
EXTER_ATTACK VIRTUAL_PLUGIN_API void CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job);
EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::BackupLimit::type& limit);
EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowRestoreInLocalNode(ActionResult& returnValue,
    const AppProtect::RestoreJob& job);
EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowBackupSubJobInLocalNode(ActionResult& returnValue,
    const AppProtect::BackupJob& job, const AppProtect::SubJob& subJob);
EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowRestoreSubJobInLocalNode(ActionResult& returnValue,
    const AppProtect::RestoreJob& job, const AppProtect::SubJob& subJob);
EXTER_ATTACK VIRTUAL_PLUGIN_API void QueryJobPermission(AppProtect::JobPermission& returnJobPermission,
    const ApplicationEnvironment& appEnv, const Application& application);
EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowCheckCopyInLocalNode(ActionResult& returnValue,
    const AppProtect::CheckCopyJob& job);
EXTER_ATTACK VIRTUAL_PLUGIN_API void AllowCheckCopySubJobInLocalNode(ActionResult& returnValue,
    const AppProtect::CheckCopyJob& job, const AppProtect::SubJob& subJob);

void InitAppCfg();
void InitOpenStackCfg();
void InitApsaraStackCfg();
void InitCNwareCfg();
void InitHyperVCfg();
#ifndef WIN32
void RegisterWhitelist();
#endif
int CheckCopyVerifyInformation(const std::shared_ptr<VirtPlugin::RepositoryHandler> &metaRepoHandler,
                               const AppProtect::BackupJob &job, const std::string &metaRepoPath, bool &checkRet);

bool IsSha256FileExist(const std::shared_ptr<VirtPlugin::RepositoryHandler> &metaRepoHandler,
                       const std::string &metaRepoPath);

#ifdef __cplusplus
}
#endif
#endif