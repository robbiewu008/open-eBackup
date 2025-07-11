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
#ifndef JOB_SERVICE_HANDLER_H_
#define JOB_SERVICE_HANDLER_H_

#include "apps/appprotect/plugininterface/JobService.h"
#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"
#include "message/curlclient/DmeRestClient.h"
#include "common/JsonHelper.h"
#include "alarm/AlarmHandle.h"

namespace jobservice {
class JobServiceHandler : public AppProtect::JobServiceIf {
public:
    virtual ~JobServiceHandler()
    {}

    EXTER_ATTACK virtual void AddNewJob(AppProtect::ActionResult& _return, const std::vector<AppProtect::SubJob>& job);
    EXTER_ATTACK virtual void ReportJobDetails(AppProtect::ActionResult& _return,
        const AppProtect::SubJobDetails& jobInfo);
    EXTER_ATTACK virtual void ReportAsyncJobDetails(AppProtect::ActionResult& _return, const std::string &jobId,
        mp_int32 code, const AppProtect::ResourceResultByPage& results);
    EXTER_ATTACK virtual void ReportCopyAdditionalInfo(
        AppProtect::ActionResult& _return, const std::string& jobId, const AppProtect::Copy& copy);
    EXTER_ATTACK virtual void ComputerFileLocationInMultiFileSystem(std::map<std::string, std::string>& _return,
        const std::vector<std::string>& files, const std::vector<std::string>& fileSystems);
    EXTER_ATTACK virtual void QueryPreviousCopy(Copy& _return, const Application& application,
        const std::set<CopyDataType::type>& types, const std::string& copyId, const std::string& mainJobId);
    EXTER_ATTACK virtual void MountRepositoryByPlugin(AppProtect::ActionResult& _return,
        const AppProtect::PrepareRepositoryByPlugin& mountinfo);
    EXTER_ATTACK virtual void UnMountRepositoryByPlugin(AppProtect::ActionResult& _return,
        const AppProtect::PrepareRepositoryByPlugin& mountinfo);
    EXTER_ATTACK void SendAlarm(AppProtect::ActionResult& _return,
        const AppProtect::AlarmDetails& alarm);
    EXTER_ATTACK void ClearAlarm(AppProtect::ActionResult& _return,
        const AppProtect::AlarmDetails& alarm);
    EXTER_ATTACK void AddIpWhiteList(AppProtect::ActionResult& _return, const std::string &jobId,
        const std::string &ipListStr);
    EXTER_ATTACK void GetHcsToken(AppProtect::ApplicationEnvironment& env, const std::string &projectId,
        const std::string &isWorkSpace);

private:
    inline mp_void ThrowAppException(mp_int32 errCode, const mp_string& message)
    {
        AppProtect::AppProtectFrameworkException copyException;
        copyException.code = errCode;
        copyException.message = message;
        throw copyException;
    }

    mp_bool IsCurAgentFcOn(const mp_string& data);
    mp_bool IsSanClientMount(const mp_string& data);
    mp_int32 GetSanClientParam(const Json::Value &extendInfo, Json::Value &sanClientParam);
    mp_string GetLunInfo(const Json::Value& sanClientParam,  mp_string &mountProtocol, const mp_string &repositoryType,
        const mp_string &remotePath);
    mp_int32 MountDataturboFilesystem(const AppProtect::PrepareRepositoryByPlugin& mountinfo);
    mp_int32 MountNasFilesystem(const AppProtect::PrepareRepositoryByPlugin& mountinfo,
        mp_string& errorMsg, mp_int32& errCode);
    mp_int32 MountFileIoSystem(const AppProtect::PrepareRepositoryByPlugin& mountinfo);
    void HandleRestResult(AppProtect::ActionResult& _return, const HttpResponse& httpResponse);
    void RestoreRepositories(Json::Value &copyValue);
#ifdef WIN32
    mp_int32 MountCIFS(const AppProtect::PrepareRepositoryByPlugin& mountinfo, std::string& MountDirve);
#endif
    void TransAlarmParam(const AppProtect::AlarmDetails& alarm, alarm_param_t &alarmParam);
    AlarmHandle m_alarmHandle;
};
}  // namespace jobservice
#endif
