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
#ifndef _CLIENT_INVOKE_H_
#define _CLIENT_INVOKE_H_
#include <map>
#include <set>
#include <string>
#include <thread>
#include <chrono>
#include "PluginThriftClient.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "ApplicationProtectFramework_types.h"
#include "log/Log.h"

using namespace AppProtect;

#ifdef WIN32
class AGENT_API ShareResource {
#else
class ShareResource {
#endif
public:
    /** create new resource for job */
    static void CreateResource(ActionResult& returnValue, const AppProtect::Resource& resource,
        const std::string& mainJobId);
    /** query job information */
    static void QueryResource(AppProtect::ResourceStatus& returnValue, const AppProtect::Resource& resource,
        const std::string& mainJobId);
    /** update job value */
    static void UpdateResource(ActionResult& returnValue, const AppProtect::Resource& resource,
        const std::string& mainJobId);
    /** delete job */
    static void DeleteResource(ActionResult& returnValue, const AppProtect::Resource& resource,
        const std::string& mainJobId);
    /** lock the job */
    static void LockResource(ActionResult& returnValue, const AppProtect::Resource& resource,
        const std::string& mainJobId);
    /** unlock the job */
    static void UnLockResource(ActionResult& returnValue, const AppProtect::Resource& resource,
        const std::string& mainJobId);
    /** lock the job resource */
    static void LockJobResource(ActionResult &returnValue, const AppProtect::Resource& resource,
        const std::string &mainJobId);
};

#ifdef WIN32
class AGENT_API JobService {
#else
class JobService {
#endif
public:
    /** when generating job, the interface can be used to create new sub job */
    static void AddNewJob(ActionResult& returnValue, const std::vector<SubJob>& job);
    /** report job information to platform, then the job will be display in GUI */
    static void ReportJobDetails(ActionResult& returnValue, const AppProtect::SubJobDetails& jobInfo);
    /** report exteneral image when backup using external backup repository */
    static void ReportCopyAdditionalInfo(ActionResult& returnValue, const std::string& jobId, const Copy& copy);
    /** when protect an appliation using multiple filesystem, every file have fix location in multiple file system */
    static void ComputerFileLocationInMultiFileSystem
        (std::map<std::string, std::string>& returnValue, const std::vector<std::string>& files,
         const std::vector<std::string>& fileSystems);
    /** Query the previous most recent copy from copy data type and specified copy */
    static void QueryPreviousCopy
        (Copy& returnValue, const Application& application,
         const std::set<AppProtect::CopyDataType>& _types, const std::string& copyId, const std::string& mainJobId);
    /** mount repository by plugin */
    static void MountRepositoryByPlugin(
        ActionResult& returnValue, const AppProtect::PrepareRepositoryByPlugin& mountinfo);
    /** umount repository by plugin */
    static void UnMountRepositoryByPlugin(
        ActionResult& returnValue, const AppProtect::PrepareRepositoryByPlugin& mountinfo);
    /** Plugin send alarm */
    static void SendAlarm(ActionResult& returnValue, const AppProtect::AlarmDetails& alarm);
    /** Plugin clear alarm */
    static void ClearAlarm(ActionResult& returnValue, const AppProtect::AlarmDetails& alarm);
    static void AddIpWhiteList(ActionResult& returnValue, const std::string &jobId, const std::string &ipListStr);
};

class RegisterPluginService {
public:
    /** when plugin starting, the plugin need call the interface for register to platform */
    static void RegisterPlugin(ActionResult& returnValue, const ApplicationPlugin& plugin);
    /** when plugin is uninstalled, the plugin need call the interface for unregister from platform */
    static void UnRegisterPlugin(ActionResult& returnValue, const ApplicationPlugin& plugin);
};

class SecurityService {
public:
    /** when plugin need check Certificate thumbprint info validity call the interface **/
    static void CheckCertThumbPrint(ActionResult& returnValue, const std::string& ip,
                                    const int32_t port, const std::string& thumbPrint);
};

class FrameworkService {
    public:
    /** Plugin check whether the Agent framework is normal **/
    static void HeartBeat(ActionResult& returnValue);
};

#endif