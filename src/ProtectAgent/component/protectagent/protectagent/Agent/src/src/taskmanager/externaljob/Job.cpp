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
#include <algorithm>
#include <vector>
#include "pluginfx/ExternalPluginManager.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "common/JsonUtils.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"
#include "host/host.h"
#include "securecom/RootCaller.h"
#include "securecom/SecureUtils.h"
#include "taskmanager/externaljob/PluginLogBackup.h"
#include "servicecenter/services/device/RepositoryFactory.h"
#include "message/curlclient/DmeRestClient.h"
#include "taskmanager/externaljob/Job.h"

namespace AppProtect {
namespace {
    constexpr int32_t MAX_JOB_REPORT_INTERVAL = 5;
    constexpr int32_t FIRST_COPY = 1;
    constexpr int32_t FIRST_LINE = 1;
    constexpr int32_t SECOND_COPY = 2;
    constexpr int32_t ONE_LAYER_PATH = 1;
    constexpr int32_t TWO_LAYER_PATH = 2;
    constexpr int32_t MOUNT_KEEP_ALIVE_REPORT_INTERVAL = 30; // 30s
    constexpr int32_t MOUNT_KEEP_ALIVE_SLEEP_INTERVAL = 1;
    constexpr uint32_t REPOSITORY_META_REMOTE_PATH = 0;
    const mp_string IN_AGENT_TYPE = "1";
    const mp_string OUT_AGENT_TYPE = "0";
    const mp_string DORADO_ENVIRONMENT = "0";
    const mp_string VIRTUAL_DORADO_ENVIRONMENT = "1";
    const int32_t DATA_REPOSITORY_TYPE = 1;
    const mp_string NAS_SHARE_APP_TYPE = "NasShare";
    const mp_string NAS_FILESYSTEM_APP_TYPE = "NasFileSystem";
    constexpr uint32_t MAX_INC_ACQUIRE_INTERVAL_TIMES = 5;
    constexpr uint32_t DEFAULT_ACQUIRE_INTERVAL = 2;
}

mp_bool PluginJobData::IsCurAgentFcOn() const
{
    LOGGUARD("");
    Json::Value fcSwitch;
    if (param.isMember("extendInfo") && param["extendInfo"].isObject() &&
        param["extendInfo"].isMember("fibreChannel") && param["extendInfo"]["fibreChannel"].isString()) {
        if (!JsonHelper::JsonStringToJsonValue(param["extendInfo"]["fibreChannel"].asString(), fcSwitch)) {
            ERRLOG("Parse params failed.");
            return MP_FALSE;
        }
        Json::FastWriter fw;
        DBGLOG("The fibreChannel object is: %s.", fw.write(fcSwitch).c_str());
    } else {
        DBGLOG("No fibreChannel key.");
        return MP_FALSE;
    }
    CHost host;
    mp_string strSN;
    mp_int32 iRet = host.GetHostSN(strSN);
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, iRet %d.", iRet);
        return MP_FALSE;
    }
    mp_bool isFcOn = MP_FALSE;
    if (fcSwitch.isMember(strSN) && fcSwitch[strSN].isString()) {
        isFcOn = fcSwitch[strSN].asString() == "true" ? MP_TRUE : MP_FALSE;
    }
    return isFcOn;
}

mp_bool PluginJobData::IsSanClientMount() const
{
    LOGGUARD("");
    mp_bool usedSanclient = MP_FALSE;
    if (!param.isMember("extendInfo") || !param["extendInfo"].isObject()) {
        ERRLOG("Parse params failed.");
        return usedSanclient;
    }
    Json::Value extendInfo = param["extendInfo"];
    if (extendInfo.isMember("sanclientInvolved") && extendInfo["sanclientInvolved"].isString()) {
        mp_string useSanClient = extendInfo["sanclientInvolved"].asString();
        if (useSanClient == "true") {
            usedSanclient = MP_TRUE;
        }
    }
    DBGLOG("Check use sanclient completed, result: %d", usedSanclient);
    return usedSanclient;
}

mp_bool PluginJobData::IsFileClientMount() const
{
    LOGGUARD("");
    mp_bool usedFileClient = MP_FALSE;
    if (!param.isMember("extendInfo") || !param["extendInfo"].isObject()) {
        ERRLOG("Parse params failed.");
        return usedFileClient;
    }
    Json::Value extendInfo = param["extendInfo"];
    if (extendInfo.isMember("agentMountType") && extendInfo["agentMountType"].isString()) {
        mp_string agentMountType = extendInfo["agentMountType"].asString();
        if (agentMountType == "fuse") {
            usedFileClient = MP_TRUE;
        }
    }
    DBGLOG("Check use FileClient completed, result: %d", usedFileClient);
    return usedFileClient;
}


void PluginJobData::SetNoNetworkErrorOccur()
{
    if (!flagNetErrorOccur) {
        return;
    }
    INFOLOG("Job %s recovery from network error.", mainID.c_str());
    flagNetErrorOccur = false;
}

void PluginJobData::SetNetworkErrorOccur()
{
    if (flagNetErrorOccur) {
        return;
    }
    flagNetErrorOccur = true;
    timeNetErrorStart = std::chrono::steady_clock::now().time_since_epoch().count();
    WARNLOG("Handle job %s occur network error.", mainID.c_str());
}

bool PluginJobData::IsNetworkLongTimeError()
{
    if (!flagNetErrorOccur) {
        return false;
    }

    auto start =  std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(timeNetErrorStart));
    return (std::chrono::steady_clock::now() - start) > std::chrono::minutes(NETWIORK_ERROR_IDENTIFY_TIME);
}

void PluginJobData::UpdateNextAcquireInterval(bool acquireSuccess)
{
    int32_t baseInterval = DEFAULT_ACQUIRE_INTERVAL;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_JOB_FRAM_SECTION, CFG_GET_JOB_BASE_INTERVAL, baseInterval);

    int32_t incInterval = DEFAULT_ACQUIRE_INTERVAL;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_JOB_FRAM_SECTION, CFG_GET_JOB_INC_INTERVAL, incInterval);

    int32_t maxAdjustTimes = MAX_INC_ACQUIRE_INTERVAL_TIMES;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_JOB_FRAM_SECTION,
        CFG_GET_JOB_INTERVAL_MAX_ADJUST_TIMES, maxAdjustTimes);

    if (acquireSuccess) {
        acquireAdjustTimes = (acquireAdjustTimes <= 0) ? 0 : acquireAdjustTimes - 1;
    } else {
        acquireAdjustTimes = (acquireAdjustTimes >= maxAdjustTimes) ? maxAdjustTimes : acquireAdjustTimes + 1;
    }

    currentAcquireInterval = 0;
    nextAcquireInterval = acquireAdjustTimes * incInterval + baseInterval;
    DBGLOG("Update next acquire job %s interval %d.", mainID.c_str(), nextAcquireInterval);
}

void PluginJobData::UpdateCurrentAcquireInterval()
{
    currentAcquireInterval += DEFAULT_ACQUIRE_INTERVAL;
}

bool PluginJobData::IsNeedTriggerAcquire()
{
    return currentAcquireInterval >= nextAcquireInterval;
}

Job::~Job()
{
    if (m_mountKeepAliveTh) {
        m_mountKeepAliveTh->join();
        m_mountKeepAliveTh.reset();
    }
}

mp_int32 Job::Initialize()
{
    SplitRepositories();

    PluginJobData jobData;
    mp_int32 iRet = JobStateDB::GetInstance().QueryJob(m_data.mainID, m_data.subID, jobData);
    if (iRet == MP_NOEXISTS) {
        iRet = JobStateDB::GetInstance().InsertRecord(m_data); // insert new main job when there is no job
    } else if (iRet != MP_SUCCESS) {  // return failed when query job failed
        ERRLOG("QueryJob faild");
        return ERROR_COMMON_INVALID_PARAM;
    }
    return iRet;
}

bool Job::IsCompleted()
{
    if (IsMainJob()) {
        return m_data.status == mp_uint32(MainJobState::COMPLETE) ||
            m_data.status == mp_uint32(MainJobState::FAILED);
    } else {
        return m_data.status == mp_uint32(SubJobState::SubJobComplete) ||
            m_data.status == mp_uint32(SubJobState::SubJobFailed);
    }
}

bool Job::IsCompleted(mp_int32 jobStage)
{
    if (IsMainJob()) {
        return jobStage == mp_uint32(MainJobState::COMPLETE) ||
            jobStage == mp_uint32(MainJobState::FAILED);
    } else {
        return jobStage == mp_uint32(SubJobState::SubJobComplete) ||
            jobStage == mp_uint32(SubJobState::SubJobFailed);
    }
}

bool Job::IsFailed()
{
    if (IsMainJob()) {
        return m_data.status == mp_uint32(MainJobState::FAILED);
    } else {
        return m_data.status == mp_uint32(SubJobState::SubJobFailed) ||
            m_data.status == mp_uint32(SubJobState::PrepareFailed);
    }
}

mp_int32 Job::SendAbortToPlugin()
{
    ActionResult ret;
    // preSubJo and genSubJob, main job id is equal to sub id id
    ProtectServiceCall(&ProtectServiceIf::AsyncAbortJob, ret, m_data.mainID, m_data.subID, m_data.appType);
    if (ret.code != MP_SUCCESS) {
        ERRLOG("Send abort Job jobId=%s, subJobId=%s req failed:%d.",
            m_data.mainID.c_str(),
            m_data.subID.c_str(),
            ret.code);
    } else {
        INFOLOG("Send abort Job jobId=%s, subJobId=%s req success.", m_data.mainID.c_str(), m_data.subID.c_str());
    }
    return ret.code;
}

std::shared_ptr<thriftservice::IThriftClient> Job::GetThriftClient()
{
    std::shared_ptr<thriftservice::IThriftClient> pClient;
    int32_t retryTime = 3;
    while (retryTime--) {
        auto plugin = ExternalPluginManager::GetInstance().GetPlugin(m_data.appType);
        if (plugin != nullptr && (pClient = plugin->GetPluginClient()) != nullptr) {
            m_pluginPID = plugin->GetPluginInfo().processId;
            break;
        }
        ERRLOG("Get thrift client Failed, retry:%d time left, plugin:[%s]", retryTime, m_data.appType.c_str());
        SleepFor(std::chrono::seconds(1));
    }
    return pClient;
}

std::shared_ptr<ProtectServiceIf> Job::GetProtectServiceClient(
    std::shared_ptr<thriftservice::IThriftClient> pThriftClient)
{
    return std::dynamic_pointer_cast<ProtectServiceIf>(
        pThriftClient->GetConcurrentClientIf<ProtectServiceConcurrentClient>("ProtectService"));
}

mp_int32 Job::UmountNas()
{
    INFOLOG("Job jobId=%s, subJobId=%s start to umount nas", m_data.mainID.c_str(), m_data.subID.c_str());
    if (m_data.IsCurAgentFcOn()) {
        for (auto mountPointIter = m_data.dtbMountPoints.begin(); mountPointIter != m_data.dtbMountPoints.end();
             ++mountPointIter) {
            RepositoryFactory repositoryHandler;
            std::shared_ptr<Repository> pRepository = repositoryHandler.CreateRepository(mountPointIter->first);
            pRepository->Umount(mountPointIter->second, m_data.mainID);
        }
    } else {
        for (auto mountPointIter = m_data.mountPoints.begin(); mountPointIter != m_data.mountPoints.end();
             ++mountPointIter) {
            RepositoryFactory repositoryHandler;
            std::shared_ptr<Repository> pRepository = repositoryHandler.CreateRepository(mountPointIter->first);
            pRepository->Umount(mountPointIter->second, m_data.mainID);
        }
    }
    return MP_SUCCESS;
}

mp_int32 Job::IsScriptValid(const mp_string& scriptName)
{
    for (int i = 0; i < SCRIPT_CMD_INJECT_CHARS.size(); i++) {
        if (scriptName.find(SCRIPT_CMD_INJECT_CHARS.at(i)) != mp_string::npos) {
            return MP_FALSE;
        }
    }
    return MP_TRUE;
}

// 在这里上报结果！！！
mp_int32 Job::ExecScriptCommon(mp_int32 commandId, const mp_string& scriptName, std::vector<mp_string> pvecResult[])
{
#ifndef WIN32
    CRootCaller rootCaller;
    mp_string strInput = scriptName + NODE_COLON + "1" + NODE_COLON;
    mp_int32 iRet = rootCaller.Exec(commandId, strInput, pvecResult);
#else
    mp_int32 iRet = MP_FAILED;
    if (commandId == ROOT_COMMAND_THIRDPARTY) {
        // SecureCom::SysExecScript只获取到bin目录下，需组装至thridparty目录
        mp_string strInput = mp_string(AGENT_THIRDPARTY_DIR) + PATH_SEPARATOR + scriptName;
        // 调用时，默认参数设为FALSE，不验证脚本签名
        iRet = SecureCom::SysExecScript(strInput, "", pvecResult, MP_FALSE);
    } else {
        iRet = SecureCom::SysExecUserScript(scriptName, "", pvecResult, MP_FALSE);
    }

    if (iRet != MP_SUCCESS) {
        mp_int32 iRettmp = ErrorCode::GetInstance().GetErrorCode(iRet);
        ERRLOG("Exec jobId=%s script failed, initial return code is %d, tranformed return code is %d",
            m_data.mainID.c_str(), iRet, iRettmp);
        iRet = iRettmp;
    }
#endif
    // 上报结果  参数有任务脚本名称 执行结果
    ReportExcScriptResult(scriptName, pvecResult);
    return iRet;
}

mp_int32 Job::ExecScript(const mp_string& scriptName)
{
    INFOLOG("Begin to exec script %s, jobId=%s", scriptName.c_str(), m_data.mainID.c_str());
    if (!IsScriptValid(scriptName)) {
        ERRLOG("Script exists command inject risk, filename is %s", scriptName.c_str());
        return ERROR_SCRIPT_COMMON_PATH_WRONG;
    }

    std::vector<mp_string> *pvecResult = nullptr;
    std::vector<mp_string> vecResult;

    mp_int32 commandId = ROOT_COMMAND_THIRDPARTY;
    mp_string scriptPath = scriptName;
    if (CMpFile::CheckFileName(scriptPath) == MP_SUCCESS) {
        scriptPath = CMpString::BlankComma(
            CPath::GetInstance().GetThirdPartyFilePath(scriptPath, "1"));
        pvecResult = &vecResult;
        commandId = ROOT_COMMAND_SCRIPT_USER_DEFINED_USER_DO;
        DBGLOG("Plugin's task script, it is a file in thirty directory.");
    } else {
        pvecResult = &vecResult;
        commandId = ROOT_COMMAND_SCRIPT_USER_DEFINED_USER_DO;
        DBGLOG("Plugin's task script, it is a absolute path.");
    }
    mp_int32 iRet = ExecScriptCommon(commandId, scriptPath, pvecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_THIRDPARTY_EXEC_FAILED);
    if (iRet == ERROR_COMMON_SCRIPT_FILE_NOT_EXIST) {
        WARNLOG("Exec jobId=%s script, %s is not exist.", m_data.mainID.c_str(), scriptPath.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }
    if (iRet != MP_SUCCESS) {
        iRet = (iRet == INTER_ERROR_SRCIPT_FILE_NOT_EXIST) ? ERROR_COMMON_SCRIPT_FILE_NOT_EXIST : iRet;
        ERRLOG("Exec jobId=%s thirdparty script failed, iRet %d.", m_data.mainID.c_str(), iRet);
        return iRet;
    }
    INFOLOG("Exec script %s Success, jobId=%s", scriptPath.c_str(), m_data.mainID.c_str());
    return MP_SUCCESS;
}

mp_int32 Job::ExecPostScript(const mp_string& scriptNameKey)
{
    m_data.status = mp_uint32(SubJobState::Running);
    mp_string strScriptFileName = "";
    if (m_data.param.isMember(KEY_TASKPARAMS) && m_data.param[KEY_TASKPARAMS].isObject() &&
        m_data.param[KEY_TASKPARAMS].isMember(KEY_SCRIPTS) && m_data.param[KEY_TASKPARAMS][KEY_SCRIPTS].isObject() &&
        m_data.param[KEY_TASKPARAMS][KEY_SCRIPTS].isMember(scriptNameKey)) {
        CJsonUtils::GetJsonString(m_data.param[KEY_TASKPARAMS][KEY_SCRIPTS], scriptNameKey, strScriptFileName);
    }
    if (strScriptFileName.empty()) {
        INFOLOG("Post Script(key=%s) not exist, jobId=%s", scriptNameKey.c_str(), m_data.mainID.c_str());
        return MP_SUCCESS;
    }
    m_data.scriptFileName = strScriptFileName;
    
    return ExecScript(strScriptFileName);
}

mp_int32 Job::MountNas()
{
    if (m_data.appType == "CloudBackupFileSystem") {
        return MP_SUCCESS;
    }
    INFOLOG("Job jobId=%s, subJobId=%s start to mount nas.", m_data.mainID.c_str(), m_data.subID.c_str());
    StartKeepAliveThread();
    JobPermission jobPermit;
    GetPermission(jobPermit);

    std::map<Json::ArrayIndex, std::vector<Json::Value>> mapJsonRep;
    std::vector<Json::Value> vecJsonBackupRep;
    CJsonUtils::GetJsonArrayJson(m_data.param, "repositories", vecJsonBackupRep);
    mapJsonRep.insert(std::make_pair(0, vecJsonBackupRep));
    mp_int32 noNeedMountCopyCount = 0;
    if (IsNonNativeRestore()) {
        NonNativeSplitRepo(mapJsonRep, noNeedMountCopyCount);
    } else if (m_data.mainType != MainJobType::CHECK_COPY_JOB && m_data.IsSanClientMount()) {
        INFOLOG("Sanclient backup mode no need search the copies repositories.");
    } else {
        for (Json::ArrayIndex index = 0; index < m_data.param["copies"].size(); ++index) {
            std::vector<Json::Value> vecJsonCopyRep;
            CJsonUtils::GetJsonArrayJson(m_data.param["copies"][index], "repositories", vecJsonCopyRep);
            mapJsonRep.insert(std::make_pair(index + 1, vecJsonCopyRep));
        }
    }
    std::map<Json::ArrayIndex, Json::Value> mapJsonRep_new;
    for (auto iter = mapJsonRep.begin(); iter != mapJsonRep.end(); ++iter) {
        Json::Value JsonRep_new;
        mp_int32 ret = MountNas_Ex(iter->second, JsonRep_new, jobPermit);
        if (ret != MP_SUCCESS) {
            StopKeepAliveThread();
            return ret;
        }
        mapJsonRep_new.insert(std::make_pair(iter->first, JsonRep_new));
    }

    for (auto iter = mapJsonRep_new.begin(); iter != mapJsonRep_new.end(); ++iter) {
        if (iter->first == 0) {
            m_data.param["repositories"] = iter->second;
        } else {
            m_data.param["copies"][noNeedMountCopyCount + iter->first - 1]["repositories"] = iter->second;
        }
    }
    StopKeepAliveThread();
    INFOLOG("Job jobId=%s, subJobId=%s mount nas finish.", m_data.mainID.c_str(), m_data.subID.c_str());
    return MP_SUCCESS;
}

mp_int32 Job::MountNas_Ex(const std::vector<Json::Value>& vecJsonRep,
    Json::Value& JsonRep_new, const JobPermission &jobPermit)
{
    for (auto jsonRep : vecJsonRep) {
        StorageRepository stRep;
        std::string jsonRepStr;
        WipeSensitiveForJsonData(jsonRep.toStyledString(), jsonRepStr);
        DBGLOG("RepoInfo=%s.", jsonRepStr.c_str());
        JsonToStruct(jsonRep, stRep);
        SetPostScanParam(stRep, jsonRep);
        if (stRep.remotePath.empty() || (!m_data.IsCurAgentFcOn() && stRep.remoteHost.empty())) {
            JsonRep_new.append(std::move(jsonRep));
            INFOLOG("remotePath=%s, IsCurAgentFcOn=%d, remoteHost.empty=%d, skip Mount option, jobId=%s, subJobId=%s",
                    stRep.remotePath.c_str(), m_data.IsCurAgentFcOn(), stRep.remoteHost.empty(),
                    m_data.mainID.c_str(), m_data.subID.c_str());
            continue;
        }
        if (!NeedMount(jsonRep)) {
            JsonRep_new.append(std::move(jsonRep));
            INFOLOG("No need mount.");
            continue;
        }
        MountPermission permit = { jobPermit.user, jobPermit.group, jobPermit.fileMode };
        RepositoryFactory repositoryHandler;
        std::shared_ptr<Repository> pRepository = repositoryHandler.CreateRepository(stRep.repositoryType);
        mp_int32 iRet = pRepository->Mount(m_data, stRep, JsonRep_new, permit);
        ClearString(stRep.auth.authPwd);
        ClearString(stRep.cifsAuth.authPwd);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Mount repository failed, jobId=%s, subJobId=%s, repositoryType=%d.",
                m_data.mainID.c_str(), m_data.subID.c_str(), mp_int32(stRep.repositoryType));
            return iRet;
        }
    }
    return MP_SUCCESS;
}

void Job::SetPostScanParam(const StorageRepository& repo, const Json::Value& repoJson)
{
    if ((m_data.mainType != MainJobType::BACKUP_JOB) || (m_data.subType != SubJobType::type::POST_SUB_JOB)) {
        return;
    }
    if (repo.repositoryType == RepositoryDataType::type::DATA_REPOSITORY ||
        repo.repositoryType == RepositoryDataType::type::LOG_REPOSITORY) {
        if (!repoJson["extendInfo"].isMember("isAgentNeedScan")) {
            return;
        }
        INFOLOG("The value of isAgentNeedScan is %s.", repoJson["extendInfo"]["isAgentNeedScan"].asString().c_str());
        m_isAgentNeedScan = repoJson["extendInfo"]["isAgentNeedScan"].asString() == "true" ? true : false;
    }
}

bool Job::NeedMount(const Json::Value &jsonRep)
{
    if (IsNasLiveMountJob()) {
        INFOLOG("Is nas livemount job, skip Mount option, jobId=%s, subJobId=%s",
            m_data.mainID.c_str(), m_data.subID.c_str());
        return false;
    }
    if (!IsNeedShareMount(jsonRep)) {
        INFOLOG("isNeedShare is false, skip Mount option, jobId=%s, subJobId=%s",
            m_data.mainID.c_str(), m_data.subID.c_str());
        return false;
    }
    if (IsManualMount(jsonRep)) {
        INFOLOG("Mount manually is true, skip Mount option, jobId=%s, subJobId=%s",
            m_data.mainID.c_str(), m_data.subID.c_str());
        return false;
    }
    return true;
}

bool Job::IsNasLiveMountJob()
{
    if ((m_data.appType == NAS_SHARE_APP_TYPE || m_data.appType == NAS_FILESYSTEM_APP_TYPE)
        && m_data.mainType == MainJobType::LIVEMOUNT_JOB) {
        INFOLOG("Job is 'NasShare' or 'NasFileSystem' livemount, jobId=%s, subJobId=%s",
            m_data.mainID.c_str(), m_data.subID.c_str());
        return true;
    }
    INFOLOG("Job is not 'NasShare' or 'NasFileSystem' livemount, jobId=%s, subJobId=%s",
            m_data.mainID.c_str(), m_data.subID.c_str());
    return false;
}

mp_bool Job::IsLogBackupJob()
{
    if (m_data.mainType != MainJobType::BACKUP_JOB) {
        return MP_FALSE;
    }
    BackupJob jobParam;
    JsonToStruct(m_data.param, jobParam);
    if (jobParam.jobParam.backupType == BackupJobType::LOG_BAKCUP) {
        return MP_TRUE;
    }
    return MP_FALSE;
}

void Job::UpdateReportTimepoint()
{
    DBGLOG("Update Job report timepoint jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
    m_timePoint = std::chrono::steady_clock::now();
}

void Job::StartReportTiming()
{
    m_startTiming = true;
}

void Job::StopReportTiming()
{
    m_startTiming = false;
}

bool Job::IsReportTimeout()
{
    auto now = std::chrono::steady_clock::now();
    return m_startTiming && (now - m_timePoint) > std::chrono::minutes(MAX_JOB_REPORT_INTERVAL);
}

mp_int32 Job::PauseJob()
{
    auto retCode = PauseJobToPlugin();
    DBGLOG("Notify job to Pasue Job, result is :%d", retCode);
    return NotifyPauseJob();
}

mp_int32 Job::PauseJobToPlugin()
{
    ActionResult ret;
    // preSubJo and genSubJob, main job id is equal to sub id id
    ProtectServiceCall(&ProtectServiceIf::PauseJob, ret, m_data.mainID, m_data.subID, m_data.appType);
    if (ret.code != MP_SUCCESS) {
        ERRLOG("Send Pause tJob jobId=%s, subJobId=%s req failed.%d",
            m_data.mainID.c_str(),
            m_data.subID.c_str(),
            ret.code);
    } else {
        DBGLOG("Send Pause Job jobId=%s, subJobId=%s req success.", m_data.mainID.c_str(), m_data.subID.c_str());
    }
    return ret.code;
}

/**
 * Description: Split repositories
 * Example: "repositories" : [{ "remotePath" : [{ "type" : 1, "path" : "path1" }, { "type" : 0, "path" : "path2" }] }]
 *          "repositories" : [{ "remotePath" : "path1" }, { "remotePath" : "path2" }]
 * Create By: kWX884906
 */
mp_void Job::SplitRepositories()
{
    if (m_data.appType == "CloudBackupFileSystem") {
        return;
    }

    std::map<Json::ArrayIndex, std::vector<Json::Value>> mapJsonRep;
    std::vector<Json::Value> vecJsonBackupRep;
    CJsonUtils::GetJsonArrayJson(m_data.param, "repositories", vecJsonBackupRep);
    mapJsonRep.insert(std::make_pair(0, vecJsonBackupRep));
    for (Json::ArrayIndex index = 0; index < m_data.param["copies"].size(); ++index) {
        std::vector<Json::Value> vecJsonCopyRep;
        CJsonUtils::GetJsonArrayJson(m_data.param["copies"][index], "repositories", vecJsonCopyRep);
        mapJsonRep.insert(std::make_pair(index + 1, vecJsonCopyRep));
    }

    std::map<Json::ArrayIndex, Json::Value> mapJsonRep_new;
    for (auto iter = mapJsonRep.begin(); iter != mapJsonRep.end(); ++iter) {
        Json::Value JsonRep_new;
        for (auto jsonRep : iter->second) {
            SplitRepositoriesParam(jsonRep, JsonRep_new);
        }
        mapJsonRep_new.insert(std::make_pair(iter->first, JsonRep_new));
    }

    for (auto iter = mapJsonRep_new.begin(); iter != mapJsonRep_new.end(); ++iter) {
        if (iter->first == 0) {
            m_data.param["repositories"] = iter->second;
        } else {
            m_data.param["copies"][iter->first - 1]["repositories"] = iter->second;
        }
    }
}

mp_int32 Job::SplitRepositoriesParam(Json::Value &jsonRep, Json::Value &JsonRep_new)
{
    std::vector<Json::Value> vecJsonRemotePath;
    GET_JSON_ARRAY_JSON(jsonRep, "remotePath", vecJsonRemotePath);
    if (vecJsonRemotePath.empty()) {
        jsonRep["remotePath"] = "";
        JsonRep_new.append(jsonRep);
    }
    std::for_each(vecJsonRemotePath.begin(), vecJsonRemotePath.end(), [&](Json::Value jPath) -> void {
        Json::Value tempJsonRep = jsonRep;
        tempJsonRep["remotePath"] = jPath["path"];
        tempJsonRep["shareName"] = jPath["shareName"];
        tempJsonRep["remoteHost"] = jPath["remoteHost"];
        tempJsonRep["subDirPath"] = "";
        if (jPath.isMember("subDirPath") && !jPath["subDirPath"].empty()) {
            tempJsonRep["subDirPath"] = jPath["subDirPath"];
        }
        tempJsonRep["extendInfo"]["fsId"] = jPath["id"];
        if (jPath.isMember("type") && jPath["type"].isUInt() &&
            jPath["type"].asUInt() == REPOSITORY_META_REMOTE_PATH) {
            tempJsonRep["type"] = REPOSITORY_META_REMOTE_PATH;
            tempJsonRep["protocol"] = RepositoryProtocolType::type::NFS;
        }
        DBGLOG("RemoteHost size is %d, remotePath size is %d.", tempJsonRep["remoteHost"].size(),
            tempJsonRep["remotePath"].size());
        JsonRep_new.append(std::move(tempJsonRep));
    });
    return MP_SUCCESS;
}

void Job::FilerUnvalidDoradoIps(const std::vector<mp_string>& validDoradoIps)
{
    for (mp_string ip : validDoradoIps) {
        DBGLOG("validDoradoIp=%s.", ip.c_str());
    }
    std::map<Json::ArrayIndex, std::vector<Json::Value>> mapJsonRep;
    std::vector<Json::Value> vecJsonBackupRep;
    CJsonUtils::GetJsonArrayJson(m_data.param, "repositories", vecJsonBackupRep);
    mapJsonRep.insert(std::make_pair(0, vecJsonBackupRep));
    for (Json::ArrayIndex index = 0; index < m_data.param["copies"].size(); ++index) {
        std::vector<Json::Value> vecJsonCopyRep;
        CJsonUtils::GetJsonArrayJson(m_data.param["copies"][index], "repositories", vecJsonCopyRep);
        mapJsonRep.insert(std::make_pair(index + 1, vecJsonCopyRep));
    }

    std::map<Json::ArrayIndex, Json::Value> mapJsonRep_new;
    for (auto iter = mapJsonRep.begin(); iter != mapJsonRep.end(); ++iter) {
        Json::Value JsonRep_new;
        for (auto jsonRep : iter->second) {
            FilerUnvalidDoradoIpsEx(jsonRep, validDoradoIps);
            JsonRep_new.append(std::move(jsonRep));
        }
        mapJsonRep_new.insert(std::make_pair(iter->first, JsonRep_new));
    }

    for (auto iter = mapJsonRep_new.begin(); iter != mapJsonRep_new.end(); ++iter) {
        if (iter->first == 0) {
            m_data.param["repositories"] = iter->second;
        } else {
            m_data.param["copies"][iter->first - 1]["repositories"] = iter->second;
        }
    }
}

void Job::FilerUnvalidDoradoIpsEx(Json::Value& jsonRep, const std::vector<mp_string>& validDoradoIps)
{
    std::vector<Json::Value> vecJsonRemoteHost;
    CJsonUtils::GetJsonArrayJson(jsonRep, "remoteHost", vecJsonRemoteHost);
    jsonRep["remoteHost"] = Json::Value();
    for (auto jsonHost : vecJsonRemoteHost) {
        mp_string strIp;
        if (jsonHost["ip"].isString()) {
            strIp = jsonHost["ip"].asString();
        }
        if (std::find(validDoradoIps.begin(), validDoradoIps.end(), strIp) != validDoradoIps.end()) {
            jsonRep["remoteHost"].append(std::move(jsonHost));
        }
    }
}

void Job::AddDoradoIpToRepExtendInfo(Json::Value &repo,
    const std::multimap<mp_string, std::vector<mp_string>> &doradoIp)
{
    std::string remotePath = repo["remotePath"].asString();
    auto pos = doradoIp.find(remotePath);
    if (pos == doradoIp.end()) {
        WARNLOG("Not find remote path %s.", remotePath.c_str());
        return;
    }
    repo["extendInfo"]["storageLogicalIps"].resize(0);
    for (const auto &ip: pos->second) {
        repo["extendInfo"]["storageLogicalIps"].append(ip);
    }
}

void Job::AddDoradoIpToExtendInfo(const std::multimap<mp_string, std::vector<mp_string>> &doradoIp)
{
    if (m_data.mainType == MainJobType::BACKUP_JOB) {
        for (auto &repo: m_data.param["repositories"]) {
            AddDoradoIpToRepExtendInfo(repo, doradoIp);
        }
    } else if (m_data.mainType == MainJobType::RESTORE_JOB) {
        for (auto &copy: m_data.param["copies"]) {
            for (auto &repo: copy["repositories"]) {
                AddDoradoIpToRepExtendInfo(repo, doradoIp);
            }
        }
    }
}

void Job::CheckReplaceHost(const std::vector<mp_string>& containerBackendIps, const mp_string& esnLocal,
    Json::Value& JsonRep_new, std::map<Json::ArrayIndex, std::vector<Json::Value>>::iterator& jsonVec)
{
    for (auto jsonRep : jsonVec->second) {
        mp_string esn = jsonRep["extendInfo"]["esn"].isString() ? jsonRep["extendInfo"]["esn"].asString() : "";
        mp_string remotePath = jsonRep["remotePath"].isString() ? jsonRep["remotePath"].asString() : "";
        DBGLOG("dorado esn is :%s, remote path is %s", esn.c_str(), remotePath.c_str());
        if (!esn.empty() && esn != esnLocal && !esnLocal.empty()) {
            JsonRep_new.append(std::move(jsonRep));
            DBGLOG("Esn %s checked, no need replace remote host.", esn.c_str());
            continue;
        }
        if (jsonRep["protocol"].isInt() && (jsonRep["protocol"].asInt() == RepositoryProtocolType::type::CIFS ||
            jsonRep["protocol"].asInt() == RepositoryProtocolType::type::S3 ||
            jsonRep["protocol"].asInt() == RepositoryProtocolType::type::TAPE)) {
            JsonRep_new.append(std::move(jsonRep));
            DBGLOG("Protocol is cifs or s3 or tape, inner agent no need replace remote host.");
            continue;
        }
        jsonRep["remoteHost"].clear();
        for (mp_string strIp : containerBackendIps) {
            Json::Value jHost;
            jHost["ip"] = strIp;
            jHost["port"] = 0;
            jsonRep["remoteHost"].append(std::move(jHost));
        }
        JsonRep_new.append(std::move(jsonRep));
        DBGLOG("Remote path %s have been replace remote host", remotePath.c_str());
    }
}

void Job::ReplaceRemoteHost(const std::vector<mp_string>& containerBackendIps)
{
    std::map<Json::ArrayIndex, std::vector<Json::Value>> mapJsonRep;
    std::vector<Json::Value> vecJsonBackupRep;
    CJsonUtils::GetJsonArrayJson(m_data.param, "repositories", vecJsonBackupRep);
    mapJsonRep.insert(std::make_pair(0, vecJsonBackupRep));
    for (Json::ArrayIndex index = 0; index < m_data.param["copies"].size(); ++index) {
        std::vector<Json::Value> vecJsonCopyRep;
        CJsonUtils::GetJsonArrayJson(m_data.param["copies"][index], "repositories", vecJsonCopyRep);
        mapJsonRep.insert(std::make_pair(index + 1, vecJsonCopyRep));
    }
    std::map<Json::ArrayIndex, Json::Value> mapJsonRep_new;
    mp_string esnLocal;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_INNER_ESN, esnLocal);

    for (auto iter = mapJsonRep.begin(); iter != mapJsonRep.end(); ++iter) {
        Json::Value JsonRep_new;
        CheckReplaceHost(containerBackendIps, esnLocal, JsonRep_new, iter);
        mapJsonRep_new.insert(std::make_pair(iter->first, JsonRep_new));
    }

    for (auto iter = mapJsonRep_new.begin(); iter != mapJsonRep_new.end(); ++iter) {
        if (iter->first == 0) {
            m_data.param["repositories"] = iter->second;
        } else {
            m_data.param["copies"][iter->first - 1]["repositories"] = iter->second;
        }
    }
}

bool Job::IsCrossNodeORCifsMount()
{
    std::map<Json::ArrayIndex, std::vector<Json::Value>> mapJsonRep;
    std::vector<Json::Value> vecJsonBackupRep;
    CJsonUtils::GetJsonArrayJson(m_data.param, "repositories", vecJsonBackupRep);
    mapJsonRep.insert(std::make_pair(0, vecJsonBackupRep));
    for (Json::ArrayIndex index = 0; index < m_data.param["copies"].size(); ++index) {
        std::vector<Json::Value> vecJsonCopyRep;
        CJsonUtils::GetJsonArrayJson(m_data.param["copies"][index], "repositories", vecJsonCopyRep);
        mapJsonRep.insert(std::make_pair(index + 1, vecJsonCopyRep));
    }
    mp_string esnLocal;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_INNER_ESN, esnLocal);

    for (auto iter = mapJsonRep.begin(); iter != mapJsonRep.end(); ++iter) {
        for (auto jsonRep : iter->second) {
            mp_string esn = jsonRep["extendInfo"]["esn"].isString() ? jsonRep["extendInfo"]["esn"].asString() : "";
            DBGLOG("dorado esn is :%s, local esn is :%s", esn.c_str(), esnLocal.c_str());
            if (!esn.empty() && esn != esnLocal && !esnLocal.empty()) {
                DBGLOG("Esn %s checked, is cross node mount.", esn.c_str());
                return true;
            }
            if (jsonRep["protocol"].isInt() && jsonRep["protocol"].asInt() == RepositoryProtocolType::type::CIFS) {
                DBGLOG("Protocol is cifs, cifs mount.");
                return true;
            }
        }
    }
    return false;
}

void Job::FilterRemoteHost(FilterAction filterAction, mp_bool isInner)
{
    filterAction(m_data, isInner);
}

void Job::SetJobRetry(bool retry)
{
    INFOLOG("jobId=%s, subJobId=%s need to retry", m_data.mainID.c_str(), m_data.subID.c_str());
    m_shouldRetry = retry;
}

bool Job::NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID)
{
    return true;
}

mp_bool Job::IsManualMount(const Json::Value &JsonManual)
{
    if (!JsonManual["extendInfo"].isNull()) {
        Json::Value jsonexten = JsonManual["extendInfo"];
        if (jsonexten.isObject() && jsonexten.isMember("manualMount") && !jsonexten["manualMount"].isNull() &&
            jsonexten["manualMount"].isString()) {
            if (jsonexten["manualMount"].asString() == "true") {
                return MP_TRUE;
            }
        }
    }
    return MP_FALSE;
}

mp_bool Job::IsNeedShareMount(const Json::Value &jsonRepo)
{
    if (!jsonRepo["extendInfo"].isNull()) {
        Json::Value jsonextend = jsonRepo["extendInfo"];
        if (jsonextend.isMember("IsNeedShare") && !jsonextend["IsNeedShare"].isNull()) {
            if (jsonextend["IsNeedShare"].asString() == "false") {
                return MP_FALSE;
            }
        }
    }
    return MP_TRUE;
}

void Job::SetDisableCifs()
{
    m_data.disbaleCifs = true;
}

bool Job::CheckCifsDisable(const Json::Value &jsonRepo)
{
    if (!m_data.disbaleCifs) {
        return false;
    }
    if (jsonRepo["protocol"].isInt() && jsonRepo["protocol"].asInt() == RepositoryProtocolType::type::CIFS) {
        return true;
    }
    return false;
}

mp_bool Job::IsNonNativeRestore()
{
#ifdef WIN32
    return MP_FALSE;
#else
    if (m_data.mainType != MainJobType::RESTORE_JOB) {
        return MP_FALSE;
    }
    if (m_data.param["copies"].size() < 1) {
        return MP_FALSE;
    }
    // 非原生格式：首个副本为目录格式
    if (m_data.param["copies"][0].isObject() && m_data.param["copies"][0]["format"].isUInt() &&
        m_data.param["copies"][0]["format"].asUInt() == CopyFormatType::type::INNER_DIRECTORY) {
        return MP_TRUE;
    } else {
        return MP_FALSE;
    }
#endif // WIN32
}

mp_void Job::NonNativeSplitRepo(std::map<Json::ArrayIndex, std::vector<Json::Value>> &mapJsonRep,
    mp_int32 &noNeedMountCopyCount)
{
    Json::Value copiesJv = m_data.param["copies"];
    if (m_data.IsSanClientMount()) {
        std::vector<Json::Value> &jsonRep = mapJsonRep[0];
        for (auto &jsonCopyRep : jsonRep) {
            if (jsonCopyRep.isObject() && jsonCopyRep["type"] == DATA_REPOSITORY_TYPE) {
                std::vector<mp_string> output;
                CMpString::StrSplit(output, jsonCopyRep["remotePath"].asString(), CHAR_SLASH);
                jsonCopyRep["remotePath"] = output.size() > DATA_REPOSITORY_TYPE ?
                    CHAR_SLASH + output[DATA_REPOSITORY_TYPE] : jsonCopyRep["remotePath"];
                INFOLOG("jsonCopyRep remote path is: %s.", jsonCopyRep["remotePath"].asString().c_str());
            }
        }
    }
    Json::ArrayIndex lastIndex = copiesJv.size() - 1;
    /*
        非原生格式日志恢复：取日志副本和最后一个数据副本挂载
        非原生格式数据恢复：取最后一个数据副本挂载
    */
    if (copiesJv[lastIndex]["type"].isString() &&
        copiesJv[lastIndex]["type"].asString() == "log" && copiesJv.size() > 1) {
        if (copiesJv[lastIndex - 1]["format"].isUInt() &&
            copiesJv[lastIndex - 1]["format"].asUInt() == CopyFormatType::type::INNER_DIRECTORY) {
            std::vector<Json::Value> dataCopyRep;
            CJsonUtils::GetJsonArrayJson(copiesJv[lastIndex - 1], "repositories", dataCopyRep);
            SplitRepoRemotePath(dataCopyRep);
            mapJsonRep.insert(std::make_pair(FIRST_COPY, dataCopyRep));
            std::vector<Json::Value> logCopyRep;
            CJsonUtils::GetJsonArrayJson(copiesJv[lastIndex], "repositories", logCopyRep);
            mapJsonRep.insert(std::make_pair(SECOND_COPY, logCopyRep));
        }
        noNeedMountCopyCount = copiesJv.size() - SECOND_COPY;
    } else if (copiesJv[lastIndex ]["format"].isUInt() &&
        copiesJv[lastIndex ]["format"].asUInt() == CopyFormatType::type::INNER_DIRECTORY) {
        std::vector<Json::Value> dataCopyRep;
        CJsonUtils::GetJsonArrayJson(copiesJv[lastIndex], "repositories", dataCopyRep);
        SplitRepoRemotePath(dataCopyRep);
        mapJsonRep.insert(std::make_pair(FIRST_COPY, dataCopyRep));
        noNeedMountCopyCount = copiesJv.size() - FIRST_COPY;
    }
}

mp_void Job::SplitRepoRemotePath(std::vector<Json::Value> &dataCopyRep)
{
    std::vector<Json::Value> newCopyRep;
    for (auto repo : dataCopyRep) {
        if (repo["type"].isUInt() && repo["type"].asUInt() == RepositoryDataType::type::DATA_REPOSITORY &&
            repo["remotePath"].isString()) {
            // data区取第一层
            mp_string remotePath = repo["remotePath"].asString();
            std::vector<mp_string> vecPath;
            CMpString::StrSplit(vecPath, remotePath, '/');
            if (vecPath.size() > ONE_LAYER_PATH) {
                repo["remotePath"] = "/" + vecPath[ONE_LAYER_PATH];
            }
            newCopyRep.push_back(repo);
        } else if (repo["type"].isUInt() && repo["type"].asUInt() == RepositoryDataType::type::META_REPOSITORY &&
            repo["remotePath"].isString()) {
            // meta区取前两层
            mp_string remotePath = repo["remotePath"].asString();
            std::vector<mp_string> vecPath;
            CMpString::StrSplit(vecPath, remotePath, '/');
            if (vecPath.size() > TWO_LAYER_PATH) {
                repo["remotePath"] = "/" + vecPath[ONE_LAYER_PATH] + "/" + vecPath[TWO_LAYER_PATH];
            }
            newCopyRep.push_back(repo);
        } else {
            newCopyRep.push_back(repo);
        }
    }
    dataCopyRep = std::move(newCopyRep);
}

mp_void Job::SetAgentsToExtendInfo(Json::Value &param)
{
    param["extendInfo"]["agents"] = param["agents"];
    param["extendInfo"]["failedAgents"] = param["failedAgents"];
}

mp_void Job::StartKeepAliveThread()
{
    if (m_data.subType == SubJobType::type::BUSINESS_SUB_JOB || m_data.subType == SubJobType::type::POST_SUB_JOB) {
        DBGLOG("Start keep alive thread, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        m_stopMountKeepAliveTheadFlag.store(false);
        m_mountKeepAliveTh = std::make_shared<std::thread>([this]() { return ReportSubJobRunning(); });
    }
}

mp_void Job::StopKeepAliveThread()
{
    m_stopMountKeepAliveTheadFlag.store(true);
    if (m_mountKeepAliveTh) {
        m_mountKeepAliveTh->join();
        m_mountKeepAliveTh.reset();
        DBGLOG("Keep alive thread join");
    }
}

mp_void Job::ReportSubJobRunning()
{
    DBGLOG("Keep alive thread start, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
    int count = MOUNT_KEEP_ALIVE_REPORT_INTERVAL;
    while (!m_stopMountKeepAliveTheadFlag.load()) {
        count++;
        if (count <= MOUNT_KEEP_ALIVE_REPORT_INTERVAL) {
            SleepFor(std::chrono::seconds(MOUNT_KEEP_ALIVE_SLEEP_INTERVAL));
            continue;
        }
        count = 0;

        SubJobDetails subJobDetails;
        subJobDetails.__set_jobId(m_data.mainID);
        subJobDetails.__set_subJobId(m_data.subID);
        subJobDetails.__set_jobStatus(SubJobStatus::type::RUNNING);
        Json::Value detail;
        StructToJson(subJobDetails, detail);
        DmeRestClient::HttpReqParam param("PUT",
            "/v1/dme-unified/tasks/details", detail.toStyledString());
        HttpResponse response;
        param.mainJobId=m_data.mainID;
        if (DmeRestClient::GetInstance()->SendRequest(param, response) == MP_SUCCESS && response.statusCode == SC_OK) {
            DBGLOG("Keep alive report subjob datail success, jobId=%s, subJobId=%s.",
                m_data.mainID.c_str(), m_data.subID.c_str());
            UpdateReportTimepoint();
        } else {
            WARNLOG("Keep alive report subjob datail failed, jobId=%s, subJobId=%s.",
                m_data.mainID.c_str(), m_data.subID.c_str());
        }
    }
    DBGLOG("Keep alive thread end, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
}

mp_int32 Job::ReportExcScriptResult(const mp_string& scriptName, std::vector<mp_string> pvecResult[])
{
    // pvecResult 转化成字符串
    mp_string msgEcho = "";
     if (pvecResult != nullptr) {
        // pvecResult 第一行输出不是脚本输出
        for (auto iter = pvecResult->begin(); iter != pvecResult->end(); ++iter) {
            msgEcho += *iter + "\n";
        }
    } else {
        ERRLOG("The result vector of script is empty.");
        return MP_FAILED;
    }

    Json::Value jInfo;
    jInfo["ScriptName"] = scriptName;
    jInfo["MsgEcho"] = msgEcho;
    mp_string msgToReport = jInfo.toStyledString();
    m_data.scriptResult = msgToReport;
    return MP_SUCCESS;
}
}  // namespace AppProtect