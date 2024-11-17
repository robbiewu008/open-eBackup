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
#include "common/Log.h"
#include "common/MpString.h"
#include "common/ErrorCode.h"
#include "securecom/ConsistentHashRing.h"
#include "securecom/SecureUtils.h"
#include "securecom/CryptAlg.h"
#include "message/curlclient/RestClientCommon.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "taskmanager/externaljob/PluginLogBackup.h"
#include "securecom/RootCaller.h"
#include "host/host.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "apps/appprotect/plugininterface/JobServiceHandler.h"

namespace jobservice {
static const mp_string REPOSITORY_TYPE_ARRAY[] = {"meta", "data", "cache", "log", "index", "log_meta"};
namespace {
constexpr int64_t REST_INTERNALE_ERROR = 200;
constexpr int64_t MAX_MultiFileSystem_NUM = 100;
constexpr int64_t PATH_DATA_TYPE = 1;
constexpr int64_t LOG_REPO_TYPE = 3;
constexpr int64_t ERROR_MOUNT_FAILED = 199;
constexpr int64_t ERROR_MOUNTPATH_BLOCk = 200;
constexpr int64_t ERROR_POINT_MOUNTED = 201;
constexpr int64_t ERROR_MOUNTPATH = 185;
const mp_string APPTYPE = "appType";
}

EXTER_ATTACK void JobServiceHandler::AddNewJob(AppProtect::ActionResult& _return,
    const std::vector<AppProtect::SubJob>& jobs)
{
    if (jobs.empty()) {
        ERRLOG("empty job list, please check!");
        _return.code = MP_FAILED;
        _return.__set_message("empty job list, please check");
        return;
    }
    for (const AppProtect::SubJob& subJob : jobs) {
        INFOLOG(
            "jobId=%s, jobName=%s, jobPriority=%d.", subJob.jobId.c_str(), subJob.jobName.c_str(), subJob.jobPriority);
    }
    Json::Value jobValue;
    StructToJson(jobs, jobValue);

    // report to DME
    DmeRestClient::HttpReqParam param("POST",
        "/v1/dme-unified/tasks/sub-tasks", jobValue.toStyledString());
    param.nTimeOut = HTTP_TIME_OUT_ONE_MIN;
    param.mainJobId = jobs.front().jobId;
    HttpResponse response;
    _return.code = DmeRestClient::GetInstance()->SendRequest(param, response);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Fail to notify dme add new jobs");
        _return.code = REST_INTERNALE_ERROR;
    } else {
        HandleRestResult(_return, response);
    }
}

EXTER_ATTACK void JobServiceHandler::ReportJobDetails(AppProtect::ActionResult& _return,
    const AppProtect::SubJobDetails& jobInfo)
{
    INFOLOG("jobId=%s,subjobId=%s, jobStatus=%d, progress=%d, speed=%d, dataSize=%lld.",
        jobInfo.jobId.c_str(),
        jobInfo.subJobId.c_str(),
        mp_int32(jobInfo.jobStatus),
        jobInfo.progress,
        jobInfo.speed,
        jobInfo.dataSize);
    _return.code = AppProtect::AppProtectJobHandler::GetInstance()->ReportJobDetails(_return, jobInfo);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Fail to notify dme report job detail");
    }
}

EXTER_ATTACK void JobServiceHandler::ReportCopyAdditionalInfo(
    AppProtect::ActionResult& _return, const std::string& jobId, const AppProtect::Copy& copy)
{
    LOGGUARD("");
    Json::Value jobValue;
    StructToJson(copy, jobValue);

    AppProtect::PluginLogBackup walBackupHandler;
    _return.code = walBackupHandler.AssembleCopyInfo(jobId, jobValue);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Failed to exec AssembleCopyInfo, mainID : %s", jobId.c_str());
    }
    RestoreRepositories(jobValue);
    // report to DME
    DmeRestClient::HttpReqParam param("PUT",
        "/v1/dme-unified/copies/" + jobId, jobValue.toStyledString());
    param.mainJobId = jobId;
    HttpResponse response;
    _return.code = DmeRestClient::GetInstance()->SendKeyRequest(param, response);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Fail ReportCopyAdditionalInfo");
        _return.code = REST_INTERNALE_ERROR;
    } else {
        HandleRestResult(_return, response);
    }
}

EXTER_ATTACK void JobServiceHandler::ComputerFileLocationInMultiFileSystem(std::map<std::string, std::string>& _return,
    const std::vector<std::string>& files, const std::vector<std::string>& fileSystems)
{
    LOGGUARD("");
    if (files.size() > MAX_MultiFileSystem_NUM || fileSystems.size() > MAX_MultiFileSystem_NUM) {
        ThrowAppException(MP_FAILED, "param is too large.");
        return;
    }

    ConsistentHashRing hashRing;
    for (auto strFs : fileSystems) {
        if (hashRing.AddNode(strFs) == MP_FALSE) {
            ThrowAppException(MP_FAILED, "generate hash failed.");
            return;
        }
    }
    for (auto strFile : files) {
        mp_string strFs;
        if (hashRing.AssignNode(strFile, strFs) == MP_FALSE) {
            _return.clear();
            ThrowAppException(MP_FAILED, "generate hash failed.");
            return;
        }
        _return[strFile] = strFs;
        DBGLOG("file %s in %s.", strFile.c_str(), strFs.c_str());
    }
}

EXTER_ATTACK void JobServiceHandler::QueryPreviousCopy(Copy& _return, const Application& application,
    const std::set<CopyDataType::type>& types, const std::string& copyId, const std::string& mainJobId)
{
    std::vector<mp_string> vecType;
    for (auto type : types) {
        auto iter = std::find_if(TransferMap_CopyDataType.begin(), TransferMap_CopyDataType.end(),
            [type](const std::pair<std::string, CopyDataType::type>& item) -> bool { return item.second == type; });
        if (iter == TransferMap_CopyDataType.end()) {
            ERRLOG("CopyDataType error, rsp[%d].", static_cast<int32_t>(type));
            ThrowAppException(MP_FAILED, "CopyDataType error");
        } else {
            vecType.push_back(iter->first);
        }
    }
    mp_string reqUrl = "/v1/dme-unified/copies/last?protect_uuid=" + application.id;
    reqUrl = vecType.empty() ? reqUrl : reqUrl + "&type=" + CMpString::StrJoin(vecType, STR_COMMA);
    reqUrl = copyId.empty() ? reqUrl : reqUrl + "&copy_id=" + copyId;
    DmeRestClient::HttpReqParam param("GET", reqUrl, "");
    param.mainJobId = mainJobId;
    HttpResponse response;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(param, response);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get copy by appid=%s, type=%s, copyid=%s failed.", application.id, application.type, copyId);
        ThrowAppException(iRet, "Get copy information failed. error code:" + std::to_string(iRet));
    }
    if (response.statusCode != SC_OK) {
        RestClientCommon::RspMsg rspSt;
        iRet = RestClientCommon::ConvertStrToRspMsg(response.body, rspSt);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Parse rsp failed.");
            ThrowAppException(iRet, "Get copy information failed");
        }
        if (rspSt.errorCode != "0") {
            ERRLOG("responseBody message have error, rsp[%s].", rspSt.errorCode.c_str());
            ThrowAppException(atoi(rspSt.errorCode.c_str()), rspSt.errorMessage);
        }
    }
    Json::Value value;
    if (CJsonUtils::ConvertStringtoJson(response.body, value) != MP_SUCCESS) {
        ERRLOG("Convert copy to json value failed.");
        ThrowAppException(MP_FAILED, "Get copy information is illegal.");
        return;
    }
    JsonToStruct(value, _return);
    return;
}

mp_bool JobServiceHandler::IsCurAgentFcOn(const mp_string& data)
{
    LOGGUARD("");
    mp_bool isFcOn = MP_FALSE;
#ifndef WIN32
    Json::Value jsonValue;
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(data, jsonValue);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Parse params failed.");
        return MP_FALSE;
    }
    mp_string strTemp = "";
    if (jsonValue.isObject() && jsonValue.isMember("fibreChannel") && jsonValue["fibreChannel"].isString()) {
        strTemp = jsonValue["fibreChannel"].asString();
        DBGLOG("The fibreChannel is: %s.", strTemp.c_str());
    } else {
        DBGLOG("No fibreChannel key.");
        return MP_FALSE;
    }
    Json::Value fcSwitch;
    iRet = CJsonUtils::ConvertStringtoJson(strTemp, fcSwitch);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Parse fibreChannel params failed.");
        return MP_FALSE;
    }
    CHost host;
    mp_string strSN;
    iRet = host.GetHostSN(strSN);
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, iRet %d.", iRet);
        return MP_FALSE;
    }
    if (fcSwitch.isObject() && fcSwitch.isMember(strSN) && fcSwitch[strSN].isString()) {
        isFcOn = fcSwitch[strSN].asString() == "true" ? MP_TRUE : MP_FALSE;
    }
#endif
    return isFcOn;
}

mp_bool JobServiceHandler::IsSanClientMount(const mp_string& data)
{
    LOGGUARD("");
    mp_bool isSanClient = MP_FALSE;
#ifndef WIN32
    Json::Value jsonValue;
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(data, jsonValue);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Parse params failed.");
        return MP_FALSE;
    }
    if (jsonValue.isObject() && jsonValue.isMember("sanclientInvolved") && jsonValue["sanclientInvolved"].isString()) {
        mp_string strTemp = jsonValue["sanclientInvolved"].asString();
        isSanClient = strTemp == "true" ? MP_TRUE : MP_FALSE;
        DBGLOG("The sanclientInvolved is: %s.", strTemp.c_str());
    } else {
        DBGLOG("No sanclientInvolved key.");
        return MP_FALSE;
    }
#endif
    return isSanClient;
}

mp_int32 JobServiceHandler::GetSanClientParam(const Json::Value &extendInfo, Json::Value &sanClientParam)
{
    if (!extendInfo.isMember("agents") || !extendInfo["agents"].isArray()) {
        ERRLOG("No agents param in extendinfo, parse sanclient param failed.");
        return MP_FAILED;
    }
    mp_string agentId;
    CHost hostHandeler;
    mp_string hostId;
    if (hostHandeler.GetHostSN(hostId) != MP_SUCCESS) {
        ERRLOG("Get agent id failed, check param.");
        return MP_FAILED;
    }
    for (const auto &agent : extendInfo["agents"]) {
        if (!agent.isObject()) {
            continue;
        }
        GET_JSON_STRING(agent, "id", agentId);
        if (agentId == hostId) {
            if (agent.isMember("sanClients") && agent["sanClients"].isArray()) {
                sanClientParam["sanClients"] = agent["sanClients"];
                return MP_SUCCESS;
            }
        }
    }
    ERRLOG("Not find sanclient params.");
    return MP_FAILED;
}

mp_string JobServiceHandler::GetLunInfo(const Json::Value& sanClientParam,  mp_string &mountProtocol,
    const mp_string &repositoryType, const mp_string &remotePath)
{
    LOGGUARD("");
    mp_string lunInfoString = repositoryType;
    DBGLOG("Lun info: %s", sanClientParam.toStyledString().c_str());
    lunInfoString += CHAR_COLON;
    for (const auto& sanClient: sanClientParam["sanClients"]) {
        if (sanClient.isMember("iqns") && sanClient["iqns"].isArray() && !sanClient["iqns"].empty()) {
            mountProtocol = "iqns";
        } else {
            mountProtocol = "fc";
        }
        for (auto &lun : sanClient["luninfo"]) {
            mp_int32 repositoryTypeIndex;
            try {
                repositoryTypeIndex = std::stoi(lun["lunName"].asString());
            } catch (const std::exception& erro) {
                ERRLOG("Invalid lunName , erro: %s.", erro.what());
                return "";
            }
            mp_string fileSystemSize = std::to_string(lun["filesystemsize"].asInt() - 1);
            if (REPOSITORY_TYPE_ARRAY[repositoryTypeIndex] == repositoryType &&
                lun["path"].asString().find(remotePath) != mp_string::npos) {
                mp_string wpnInfo = lun["sanclientWwpn"].asString() + CHAR_SLASH + lun["lunId"].asString() +
                    CHAR_COMMA + repositoryType +  CHAR_COMMA + fileSystemSize +
                    CHAR_COMMA + lun["agentWwpn"].asString();
                lunInfoString += wpnInfo + CHAR_SLASH + CHAR_SLASH;
            }
        }
    }
    return lunInfoString;
}

mp_int32 JobServiceHandler::MountDataturboFilesystem(const AppProtect::PrepareRepositoryByPlugin& mountinfo)
{
#ifndef WIN32
    LOGGUARD("");
    CHost host;
    mp_string storageName;
    mp_int32 iRet = host.GetHostSN(storageName);  // 使用HostSN做Dataturbo对象的storage_name
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, iRet %d.", iRet);
        return iRet;
    }
    CRootCaller rootCaller;
    for (auto repository : mountinfo.repository) {
        if (repository.path.empty()) {
            ERRLOG("Input param repository.path is empty.");
            return MP_FAILED;
        }
        std::ostringstream scriptParam;
        scriptParam << "storageName=" << storageName << NODE_COLON << "nasFileSystemName=" << repository.remotePath
                    << NODE_COLON << "mountPath=" << repository.path[0] << NODE_COLON
                    << "repositoryType=" << REPOSITORY_TYPE_ARRAY[mp_int32(repository.repositoryType)] << NODE_COLON
                    << "runAccount=rdadmin" << NODE_COLON << "subPath=" << NODE_COLON
                    << "uid=" << mountinfo.permission.user << NODE_COLON << "gid=" << mountinfo.permission.group
                    << NODE_COLON << "mode=" << mountinfo.permission.fileMode << NODE_COLON;
        iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_MOUNT_DATATURBO_FILESYS, scriptParam.str(), NULL);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Mount filesystem by dataturbo failed, iRet %d.", iRet);
            return iRet;
        }
    }

#endif
    return MP_SUCCESS;
}

mp_int32 JobServiceHandler::MountNasFilesystem(
    const AppProtect::PrepareRepositoryByPlugin& mountinfo, mp_string& errorMsg, mp_int32& errCode)
{
#ifndef WIN32
    LOGGUARD("");
    mp_string mountOption;
    mp_string mountProtocol;
    AppProtect::MountNasParam mountNasParam;
    mountNasParam.appType = "general_type";
    Json::Value extendInfoValue;
    JsonHelper::JsonStringToJsonValue(mountinfo.extendInfo, extendInfoValue);
    CJsonUtils::GetJsonString(extendInfoValue, APPTYPE, mountNasParam.appType);
    AppProtect::PrepareFileSystem prepareobj;
    mp_string pluginName;
    mp_int32 iRet = prepareobj.GetMountParam(mountNasParam, mountOption, mountProtocol, pluginName);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Mount Param Failed!");
        return MP_FAILED;
    }
    CRootCaller rootCaller;
    std::vector<mp_string> vecErrMsg;
    for (auto repository : mountinfo.repository) {
        std::ostringstream scriptParam;
        scriptParam << "storageIp=" << repository.remoteHost[0].ip << NODE_COLON << "nasFileSystemName="
                    << repository.remotePath << NODE_COLON << "mountPath=" << repository.path[0] << NODE_COLON
                    << "mountOption:" << mountOption << NODE_COLON << "repositoryType=" << repository.repositoryType
                    << NODE_COLON << "mountProtocol=" << mountProtocol << NODE_COLON << "runAccount=rdadmin"
                    << NODE_COLON << "subPath=" << NODE_COLON << "uid=" << mountinfo.permission.user
                    << NODE_COLON << "gid=" << mountinfo.permission.group << NODE_COLON << "mode="
                    << mountinfo.permission.fileMode << NODE_COLON << "pluginName=" << pluginName << NODE_COLON;
        mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_MOUNT_NAS_FILESYS, scriptParam.str(), &vecErrMsg);
        if (ret != MP_SUCCESS) {
            ERRLOG("Mount Filesystem Failed,ErrMsg = %s, ret = %d!",
                vecErrMsg.empty() ? "" : vecErrMsg.front().c_str(), ret);
            errCode = ret;
            if (ret == ERROR_MOUNT_SRICPT_FAILED && !vecErrMsg.empty()) {
                errorMsg = vecErrMsg.front();
            } else if (ret == ERROR_MOUNTPATH_BLOCk) {
                errorMsg = "The mount path is in the blocklist.";
            } else if (ret == ERROR_POINT_MOUNTED) {
                errorMsg = "Mount point has mounted other file system.";
            } else if (ret == ERROR_MOUNTPATH) {
                errorMsg = "Ceate mount point failed.";
            } else {
                errCode = REST_INTERNALE_ERROR;
            }
            return MP_FAILED;
        }
    }
#endif
    return MP_SUCCESS;
}

mp_int32 JobServiceHandler::MountFileIoSystem(const AppProtect::PrepareRepositoryByPlugin& mountinfo)
{
    LOGGUARD("");
#ifndef WIN32
    Json::Value extendInfoValue;
    if (CJsonUtils::ConvertStringtoJson(mountinfo.extendInfo, extendInfoValue) != MP_SUCCESS) {
        ERRLOG("Convert extendInfo failed.");
        return MP_FAILED;
    }
    Json::Value sanClientParam;
    if (GetSanClientParam(extendInfoValue, sanClientParam) != MP_SUCCESS) {
        ERRLOG("Parse extendInfo failed.");
        return MP_FAILED;
    }

    mp_string mountProtocol;
    CRootCaller rootCaller;
    for (auto repository : mountinfo.repository) {
        if (repository.path.empty()) {
            continue;
        }
        mp_string repositoryType = REPOSITORY_TYPE_ARRAY[repository.repositoryType];
        mp_string lunInfo = GetLunInfo(sanClientParam, mountProtocol, repositoryType, repository.remotePath);
        mp_string mountPath = repository.path[0];
        std::ostringstream scriptParam;
        scriptParam << "mountPath=" << mountPath << NODE_COLON
            << "repositoryType=" << repositoryType << NODE_COLON
            << "protocolType=" << mountProtocol << NODE_COLON
            << "subPath=" << NODE_COLON
            << "LunInfo=" << lunInfo << NODE_COLON
            << "recordFile=" << NODE_COLON
            << "uid=" << mountinfo.permission.user  << NODE_COLON
            << "gid=" << mountinfo.permission.group << NODE_COLON
            << "mode=" << mountinfo.permission.fileMode  << NODE_COLON;
        mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_MOUNT_SANCLIENT_FILESYS, scriptParam.str(), NULL);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Mount Filesystem Failed, ret = %d!", iRet);
            return MP_FAILED;
        }
    }
#endif
    return MP_SUCCESS;
}

void JobServiceHandler::HandleRestResult(AppProtect::ActionResult& _return, const HttpResponse& response)
{
    if (response.statusCode != SC_OK) {
        RestClientCommon::RspMsg rspSt;
        auto iRet = RestClientCommon::ConvertStrToRspMsg(response.body, rspSt);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Parse rsp failed.");
            _return.code = REST_INTERNALE_ERROR;
            _return.__set_message("Parse rsp failed.");
        } else if (rspSt.errorCode != "0") {
            ERRLOG("responseBody message have error, rsp[%s].", rspSt.errorCode.c_str());
            _return.code = REST_INTERNALE_ERROR;
            _return.__set_bodyErr(atoi(rspSt.errorCode.c_str()));
            _return.__set_message(rspSt.errorMessage);
        }
    }
    return;
}

void JobServiceHandler::RestoreRepositories(Json::Value &copyValue)
{
    Json::Value repoValueList;
    for (Json::ArrayIndex index = 0; index < copyValue["repositories"].size(); ++index) {
        Json::Value repoValue;
        Json::Value remotePathValue;
        repoValue["id"] = copyValue["repositories"][index]["id"];
        repoValue["type"] = copyValue["repositories"][index]["type"];
        repoValue["role"] = copyValue["repositories"][index]["role"];
        repoValue["protocol"] = copyValue["repositories"][index]["protocol"];
        repoValue["isLocal"] = copyValue["repositories"][index]["isLocal"];
        repoValue["extendInfo"] = copyValue["repositories"][index]["extendInfo"];
        remotePathValue["path"] = copyValue["repositories"][index]["remotePath"];
        remotePathValue["shareName"] = copyValue["repositories"][index]["shareName"];
        remotePathValue["remoteHost"] = copyValue["repositories"][index]["remoteHost"];
        remotePathValue["id"] = copyValue["repositories"][index]["extendInfo"]["fsId"];
        if (copyValue["repositories"][index]["type"] == LOG_REPO_TYPE) {
            remotePathValue["type"] = PATH_DATA_TYPE;
        } else {
            remotePathValue["type"] = copyValue["repositories"][index]["type"];
        }
        repoValue["remotePath"].append(std::move(remotePathValue));
        repoValueList.append(std::move(repoValue));
    }
    copyValue["repositories"] = std::move(repoValueList);
}

#ifndef WIN32
EXTER_ATTACK void JobServiceHandler::MountRepositoryByPlugin(AppProtect::ActionResult& _return,
    const AppProtect::PrepareRepositoryByPlugin& mountinfo)
{
    INFOLOG("Plugin call interface mount repository.");
    if (mountinfo.repository.empty()) {
        ERRLOG("Empty repository list, please check!");
        _return.code = MP_FAILED;
        _return.message = "empty repository list, please check!";
        return;
    }
    mp_bool isSanclient = IsSanClientMount(mountinfo.extendInfo);
    mp_bool isFcOn = IsCurAgentFcOn(mountinfo.extendInfo);
    mp_int32 iRet = MP_FAILED;
    mp_string errorMsg = "Mount repository Failed!";
    if (isSanclient) {
        iRet = MountFileIoSystem(mountinfo);
    } else if (isFcOn) {
        iRet = MountDataturboFilesystem(mountinfo);
    } else {
        mp_int32 errCode;
        iRet = MountNasFilesystem(mountinfo, errorMsg, errCode);
        if (iRet != MP_SUCCESS) {
            ERRLOG("resCode:%d, errorMsg:%s, errorCode:%d",
                errCode,
                errorMsg.c_str(),
                ERROR_MOUNT_SRICPT_FAILED);
            _return.code = errCode;
            _return.__set_message(errorMsg);
            _return.__set_bodyErr(ERROR_MOUNT_SRICPT_FAILED);
            return;
        }
    }
    if (iRet == MP_SUCCESS) {
        _return.code = MP_SUCCESS;
        _return.__set_message("Mount repository list Sucess!");
    } else {
        _return.code = REST_INTERNALE_ERROR;
        _return.__set_message("Mount repository Failed!");
    }
}
#else
EXTER_ATTACK void JobServiceHandler::MountRepositoryByPlugin(AppProtect::ActionResult& _return,
    const AppProtect::PrepareRepositoryByPlugin& mountinfo)
{
    INFOLOG("Plugin call interface mount repository.");
    if (mountinfo.repository.empty()) {
        ERRLOG("Empty repository list, please check!");
        _return.code = MP_FAILED;
        _return.__set_message("empty repository list, please check!");
        return;
    }
    Json::Value MountDirveInfo;
    std::string MountDirve;
    mp_int32 ret = MP_FAILED;
    try {
        ret = MountCIFS(mountinfo, MountDirve);
    } catch (std::exception& e) {
        ERRLOG("MountCIFS throw a exception. %s", e.what());
    }
    if (ret != MP_SUCCESS) {
        ERRLOG("Mount CIFS file system failed");
        _return.code = MP_FAILED;
        _return.__set_message("Mount CIFS Failed!");
        _return.__set_bodyErr(ERROR_MOUNT_SRICPT_FAILED);
    } else {
        MountDirveInfo["MountDirve"] = MountDirve;
        _return.code = MP_SUCCESS;
        _return.__set_message(MountDirveInfo.toStyledString());
    }
}
 
mp_int32 JobServiceHandler::MountCIFS(const AppProtect::PrepareRepositoryByPlugin& mountinfo, std::string& MountDirve)
{
    for (auto rep : mountinfo.repository) {
        if (rep.remoteHost.size() == 0) {
            ERRLOG("No remote host can find in repository.");
            return MP_FAILED;
        }
        AppProtect::MountNasParam mountParam;
        mountParam.jobID = "Windows_CIFS_MOUNT";
        mountParam.vecStorageIp = {rep.remoteHost[0].ip};
        mountParam.storageName = rep.remoteName;
        mountParam.authKey = rep.auth.authkey;
        mountParam.authPwd = rep.auth.authPwd;
        mountParam.cifsAuthKey = rep.cifsAuth.authkey;
        mountParam.cifsAuthPwd = rep.cifsAuth.authPwd;
        mountParam.nCopyFormat = CopyFormatType::type::INNER_SNAPSHOT;
        mountParam.useMemSave = false;

        if (mountParam.cifsAuthKey.empty()) {
            mountParam.cifsAuthKey = mountParam.authKey;
            mountParam.cifsAuthPwd = mountParam.authPwd;
        }

        AppProtect::PrepareFileSystem mountHandler;
        std::vector<mp_string> mountPoints;
        mp_int32 ret = mountHandler.MountNasFileSystem(mountParam, mountPoints);
        if (ret != MP_SUCCESS) {
            ERRLOG("Mount nas file system \\\\%s\\%s failed.", rep.remoteHost[0].ip.c_str(), rep.remoteName.c_str());
            return ret;
        }
        if (mountPoints.size() == 0) {
            ERRLOG("No mount points have not been set.");
            return MP_FAILED;
        }
        MountDirve = mountPoints[0];
        INFOLOG("Mount nas file system \\\\%s\\%s success.", rep.remoteHost[0].ip.c_str(), rep.remoteName.c_str());
    }
    return MP_SUCCESS;
}
#endif

EXTER_ATTACK void JobServiceHandler::UnMountRepositoryByPlugin(AppProtect::ActionResult& _return,
    const AppProtect::PrepareRepositoryByPlugin& mountinfo)
{
    INFOLOG("Plugin call interface unmount repository.");
    if (mountinfo.repository.empty()) {
        ERRLOG("empty repository list, please check!");
        _return.code = MP_FAILED;
        _return.message = "empty repository list, please check!";
        return;
    }

#ifndef WIN32
    for (auto repository : mountinfo.repository) {
        if (repository.path.empty()) {
            ERRLOG("Input param repository.path is empty.");
            _return.code = REST_INTERNALE_ERROR;
            _return.__set_message("Input param repository.path is empty.");
            return;
        }
        std::ostringstream scriptParam;
        mp_string umountPath = repository.path[0] + "@" + repository.remotePath;
        // sanclient模式在路径上添加标识进行区别
        if (IsSanClientMount(mountinfo.extendInfo)) {
            umountPath += "#fileio";
        }
        scriptParam << "umountPath=" << umountPath << NODE_COLON;
        CRootCaller rootCaller;
        mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_UMOUNT_NAS_FILESYS, scriptParam.str(), NULL);
        if (ret != MP_SUCCESS) {
            _return.code = MP_FAILED;
            _return.message = "Umount repository list Failed!";
            ERRLOG("Umount repository %s Failed!", repository.path[0].c_str());
            return;
        }
    }
#endif

    _return.code = MP_SUCCESS;
    _return.message = "unMount repository list Sucess!";
    return;
}

EXTER_ATTACK void JobServiceHandler::SendAlarm(AppProtect::ActionResult& _return,
    const AppProtect::AlarmDetails& alarm)
{
    alarm_param_t alarmParam;
    TransAlarmParam(alarm, alarmParam);
    alarmParam.iAlarmClass = ALARM_CLASS::ALARM;
    INFOLOG("Start to send alarm %s.", alarmParam.iAlarmID.c_str());
    if (m_alarmHandle.ForwardAlarmReq(alarmParam) != MP_SUCCESS) {
        ERRLOG("Send alarm %s failed.", alarmParam.iAlarmID.c_str());
        _return.code = REST_INTERNALE_ERROR;
        _return.message = "Send alarm failed.";
        return;
    }
    _return.code = MP_SUCCESS;
    _return.message = "Send alarm success.";
}

EXTER_ATTACK void JobServiceHandler::ClearAlarm(AppProtect::ActionResult& _return,
    const AppProtect::AlarmDetails& alarm)
{
    alarm_param_t alarmParam;
    TransAlarmParam(alarm, alarmParam);
    INFOLOG("Start to clear alarm %s.", alarmParam.iAlarmID.c_str());
    if (m_alarmHandle.ForwardAlarmReq(alarmParam, false) != MP_SUCCESS) {
        ERRLOG("Send clear alarm %s failed.", alarmParam.iAlarmID.c_str());
        _return.code = REST_INTERNALE_ERROR;
        _return.message = "Clear alarm failed.";
        return;
    }
    _return.code = MP_SUCCESS;
    _return.message = "Clear alarm success.";
}

void JobServiceHandler::TransAlarmParam(const AppProtect::AlarmDetails& alarm, alarm_param_t &alarmParam)
{
    alarmParam.iAlarmID = alarm.alarmId;
    alarmParam.sequence = alarm.sequence;
    alarmParam.alarmType = alarm.alarmType;
    alarmParam.sourceType = alarm.sourceType;
    alarmParam.severity = alarm.severity;
    alarmParam.strAlarmParam = alarm.parameter;
    alarmParam.resourceId = alarm.resourceId;
    DBGLOG(
        "Alarm details, id:%s, sequence:%ld, alarmType:%d, sourceType:%s, severity:%d, parameter:%s,resourceId:%s",
        alarm.alarmId.c_str(),
        alarm.sequence,
        alarm.alarmType,
        alarm.sourceType.c_str(),
        alarm.severity,
        alarm.parameter.c_str(),
        alarm.resourceId.c_str());
}

void JobServiceHandler::AddIpWhiteList(AppProtect::ActionResult& _return, const std::string &jobId,
    const std::string &ipListStr)
{
    INFOLOG("Start to add white list, job id: %s, ip list: %s.", jobId.c_str(), ipListStr.c_str());
    DmeRestClient::HttpReqParam req;
    req.method = "POST";
    req.url = "/v1/dme-unified/tasks/" + jobId + "/white-list?ip_list=" + ipListStr;
    req.mainJobId = jobId;
    HttpResponse response;
    _return.code = DmeRestClient::GetInstance()->SendRequest(req, response);
    if (_return.code != MP_SUCCESS) {
        ERRLOG("Fail to notify dme add white list");
        _return.code = REST_INTERNALE_ERROR;
    } else {
        HandleRestResult(_return, response);
    }
}
}  // namespace jobservice
