/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file JobServiceHandler.h
 * @brief  Factory for Service Center
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
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
