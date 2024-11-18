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
#include <functional>
#include <thread>
#include "common/Log.h"
#include "common/Path.h"
#include "common/CMpThread.h"
#include "common/ConfigXmlParse.h"
#include "message/rest/interfaces.h"
#include "common/CSystemExec.h"
#include "apps/appprotect/AppProtectService.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "pluginfx/ExternalPluginManager.h"
#include "pluginfx/ExternalFileClientManager.h"
#include "pluginfx/OraclePluginHandler.h"
#include "plugins/RestActionMapEx.h"
#include "message/curlclient/DmeRestClient.h"
#include "plugins/appprotect/AppProtectPlugin.h"

REGISTER_PLUGIN_EX(AppProtectPlugin);

AppProtectPlugin::AppProtectPlugin()
{
    REGISTER_ACTION(REST_APPPROTECT_RESOURCE_V1, REST_URL_METHOD_GET, &AppProtectPlugin::PluginResourceV1);
    REGISTER_ACTION(REST_APPPROTECT_DETAIL_V1, REST_URL_METHOD_POST, &AppProtectPlugin::PluginDetailV1);
    REGISTER_ACTION(REST_APPPROTECT_CHECK_V1, REST_URL_METHOD_POST, &AppProtectPlugin::PluginCheckV1);
    REGISTER_ACTION(REST_APPPROTECT_CLUSTER_V1, REST_URL_METHOD_POST, &AppProtectPlugin::PluginClusterV1);
    REGISTER_ACTION(REST_APPPROTECT_DETAIL_V2, REST_URL_METHOD_POST, &AppProtectPlugin::PluginDetailV2);
    REGISTER_ACTION(REST_APPPROTECT_FINALIZE_CLEAR, REST_URL_METHOD_POST, &AppProtectPlugin::FinalizeClear);
    REGISTER_ACTION(REST_APPPROTECT_WAKEUP_JOB, REST_URL_METHOD_POST, &AppProtectPlugin::WakeUpJob);
    REGISTER_ACTION(REST_APPPROTECT_SANCLIENT_JOB, REST_URL_METHOD_POST, &AppProtectPlugin::SanclientJob);
    REGISTER_ACTION(REST_APPPROTECT_SANCLIENT_JOB_V1, REST_URL_METHOD_GET, &AppProtectPlugin::SanclientJobForUbc);
    REGISTER_ACTION(REST_CLEAN_SANCLIENT_JOB, REST_URL_METHOD_POST, &AppProtectPlugin::SanclientCleanJob);
    REGISTER_ACTION(REST_APPPROTECT_ABORT_JOB, REST_URL_METHOD_PUT, &AppProtectPlugin::AbortJob);
    REGISTER_ACTION(REST_APPPROTECT_CONFIG_V1, REST_URL_METHOD_POST, &AppProtectPlugin::PluginConfigV1);
    REGISTER_ACTION(REST_APPPROTECT_DELIVER_JOB_STATUS_V1, REST_URL_METHOD_POST, &AppProtectPlugin::DeliverJobStatus);
    REGISTER_ACTION(REST_APPPROTECT_GET_ESN, REST_URL_METHOD_GET, &AppProtectPlugin::GetESN);
    REGISTER_ACTION(REST_APPPROTECT_REMOVE_PROTECT_V1, REST_URL_METHOD_POST, &AppProtectPlugin::RemoveProtect);
}

AppProtectPlugin::~AppProtectPlugin()
{
}
mp_int32 AppProtectPlugin::Initialize(std::vector<mp_uint32>& cmds)
{
    mp_int32 ret = CServicePlugin::Initialize(cmds);
    ret = InitializeExternalPlugMgr();
    return ret;
}

mp_int32 AppProtectPlugin::InitializeExternalPlugMgr()
{
    mp_string externalPluginDir = CPath::GetInstance().GetRootPath()
        + PATH_SEPARATOR + mp_string("..") + PATH_SEPARATOR + EXTERNAL_PLUGIN_PATH;
    if (CMpFile::DirExist(externalPluginDir.c_str())) {
        INFOLOG("Begin create external plugin manager.");
        mp_int32 iRet = ExternalPluginManager::GetInstance().Init();
        if (iRet != MP_SUCCESS) {
            ERRLOG("Init external plugin manager failed.");
            return MP_FAILED;
        }
    }
    if (AppProtect::AppProtectJobHandler::GetInstance() == nullptr) {
        ERRLOG("Init AppProtectJobHandler failed.");
        return MP_FAILED;
    }
    if (AppProtectService::GetInstance() == nullptr) {
        ERRLOG("Init AppProtectService failed.");
        return MP_FAILED;
    }

    mp_int32 iRet = ExternalFileClientManager::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        WARNLOG("Init FileClient manager failed.");
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectPlugin::DoAction(CRequestMsg& req, CResponseMsg& rsp)
{
    DO_ACTION(AppProtectPlugin, req, rsp);
}

EXTER_ATTACK mp_int32 AppProtectPlugin::PluginResourceV1(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = ExternalPluginManager::GetInstance().QueryPluginResource(strAppType, req, rsp);
    DealInvokePluginFailed(iRet, rsp);
    return iRet;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::PluginDetailV1(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = ExternalPluginManager::GetInstance().QueryPluginDetail(strAppType, req, rsp);
    DealInvokePluginFailed(iRet, rsp);

    if (iRet == MP_SUCCESS && strAppType.find("oracle") != mp_string::npos) {
        OraclePluginHandler::GetInstance().OracleUpdateDBInfo(req.GetMsgBody().GetJsonValueRef());
    }
    return iRet;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::PluginCheckV1(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = ExternalPluginManager::GetInstance().CheckPlugin(strAppType, req, rsp);
    DealInvokePluginFailed(iRet, rsp);

    if (iRet == MP_SUCCESS && strAppType.find("oracle") != mp_string::npos) {
        OraclePluginHandler::GetInstance().CreateCheckArchiveThread();
    }
    return iRet;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::PluginClusterV1(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = ExternalPluginManager::GetInstance().QueryRemoteCluster(strAppType, req, rsp);
    DealInvokePluginFailed(iRet, rsp);
    return iRet;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::PluginDetailV2(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = ExternalPluginManager::GetInstance().QueryPluginDetailV2(strAppType, req, rsp);
    DealInvokePluginFailed(iRet, rsp);
    return iRet;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::FinalizeClear(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = ExternalPluginManager::GetInstance().PluginFinalizeClear(strAppType, req, rsp);
    DealInvokePluginFailed(iRet, rsp);
    return iRet;
}
 
EXTER_ATTACK mp_int32 AppProtectPlugin::RemoveProtect(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = ExternalPluginManager::GetInstance().RemoveProtect(strAppType, req, rsp);
    DealInvokePluginFailed(iRet, rsp);
    return iRet;
}

mp_int32 AppProtectPlugin::SanclientPreParamCheck(const Json::Value& jvReq)
{
    if (SanclientPreParamCheckIsVaild(jvReq) == MP_FAILED) {
        ERRLOG("Sanclient parm is not valid.");
        return MP_FAILED;
    }
    Json::Value sanclientInfo = jvReq["sanclient"];
    if (jvReq.isMember("agentWwpns") && jvReq["agentWwpns"].isArray() && sanclientInfo.isMember("sanClientWwpns")
        && sanclientInfo["sanClientWwpns"].isArray()) {
        return MP_SUCCESS;
    }
    if (jvReq.isMember("agentIqns") && jvReq["agentIqns"].isArray() && sanclientInfo.isMember("iqns") &&
        sanclientInfo["iqns"].isArray()) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 AppProtectPlugin::SanclientPreParamCheckIsVaild(const Json::Value& jvReq)
{
    if (!jvReq.isObject() || !jvReq.isMember("sanclient") || !jvReq["sanclient"].isObject()) {
        ERRLOG("Param format is:%s.", jvReq.toStyledString().c_str());
        return MP_FAILED;
    }
    Json::Value sanclientInfo = jvReq["sanclient"];
    if (!sanclientInfo.isMember("ip") || !sanclientInfo["ip"].isString() ||
        !sanclientInfo.isMember("openLanFreeSwitch") || !sanclientInfo["openLanFreeSwitch"].isBool() ||
        !sanclientInfo.isMember("srcDeduption") || !sanclientInfo["srcDeduption"].isBool()) {
        ERRLOG("SanclientInfo format is:%s.", sanclientInfo.toStyledString().c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectPlugin::SanclientPrepareJob(const Json::Value& jvReq)
{
    INFOLOG("Start Sanclient Job with UBC!");
    mp_string taskId;
    mp_int32 iRet = SanclientPreParamCheck(jvReq); // sanclient任务参数校验
    if (iRet != MP_SUCCESS) {
        ERRLOG("SanclientPreParamCheck faild.");
        return iRet;
    }
    GET_JSON_STRING(jvReq, "taskID",  taskId);
    mp_string sanclient_ip = jvReq["sanclient"]["ip"].asString();
    mp_string lanfree_switch = (jvReq["sanclient"]["openLanFreeSwitch"].asBool() ||
        jvReq["sanclient"]["srcDeduption"].asBool()) ? "true" : "false";
    std::shared_ptr<AppProtectService> appProtectServiceInstance = AppProtectService::GetInstance();
    if (appProtectServiceInstance == nullptr) {
        ERRLOG("new AppProtectService failed.");
        return MP_FAILED;
    }
    iRet = appProtectServiceInstance->EnvCheck(taskId); // sanclient任务环境检查
    if (iRet != MP_SUCCESS) {
        ERRLOG("EnvCheck faild.");
        return iRet;
    }
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
    iRet = appProtectServiceInstance->SanclientMount(jvReq, taskId, filesysteminfo, lanfree_switch); // sanclient挂载
    if (iRet != MP_SUCCESS) {
        ERRLOG("SanclientMount failed!");
        return iRet;
    }
    if (jvReq.isMember("agentWwpns") && jvReq["agentWwpns"].isArray() && !jvReq["agentWwpns"].empty()) {
        std::vector<mp_string> agentwwpns;
        GET_ARRAY_STRING_WITHOUT_BRACES(jvReq["agentWwpns"], agentwwpns);
        std::vector<mp_string> wwpns;
        GET_ARRAY_STRING_WITHOUT_BRACES(jvReq["sanclient"]["sanClientWwpns"], wwpns);
        iRet = AppProtectService::GetInstance()->CreateLun(agentwwpns, wwpns, filesysteminfo, taskId);
    } else if (jvReq.isMember("agentIqns") && jvReq["agentIqns"].isArray() && !jvReq["agentIqns"].empty()) {
        std::vector<mp_string> agentIqns;
        GET_ARRAY_STRING_WITHOUT_BRACES(jvReq["agentIqns"], agentIqns);
        std::vector<mp_string> sanclientIqns;
        GET_ARRAY_STRING_WITHOUT_BRACES(jvReq["sanclient"]["iqns"], sanclientIqns);
        if (agentIqns.empty() || sanclientIqns.empty()) {
            ERRLOG("There is no agentIqns or sanclientIqns!");
            return MP_FAILED;
        }
        iRet = AppProtectService::GetInstance()->CreateLunISCSI(agentIqns[0], sanclientIqns[0], filesysteminfo, taskId);
    }
    INFOLOG("Sanclient pre-job end. Create Lun return value:%d!", iRet);
    return iRet;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::SanclientJob(CRequestMsg& req, CResponseMsg& rsp)
{
    const Json::Value& jvReq = req.GetMsgBody().GetJsonValueRef();
    std::shared_ptr<AppProtectPlugin> timeHanlder = std::make_shared<AppProtectPlugin>();
    std::thread SanclientPrepareJobthread(&AppProtectPlugin::SanclientPrepareJob, timeHanlder, jvReq);
    SanclientPrepareJobthread.detach();
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::SanclientJobForUbc(CRequestMsg& req, CResponseMsg& rsp)
{
    std::shared_ptr<AppProtectService> appProtectServiceInstance = AppProtectService::GetInstance();
    if (appProtectServiceInstance == nullptr) {
        ERRLOG("Init AppProtectService failed.");
        return MP_FAILED;
    }
    Json::Value &jvReq = rsp.GetJsonValueRef();
    mp_int32 iRet = appProtectServiceInstance->SanclientJobForUbc(jvReq, req);
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::SanclientCleanJob(CRequestMsg& req, CResponseMsg& rsp)
{
    std::shared_ptr<AppProtectService> appProtectServiceInstance = AppProtectService::GetInstance();
    if (appProtectServiceInstance == nullptr) {
        ERRLOG("Init AppProtectService failed.");
        return MP_FAILED;
    }
    const Json::Value& jvReq = req.GetMsgBody().GetJsonValueRef();
    if (!jvReq.isObject() || !jvReq.isMember("task_id") || !jvReq["task_id"].isString()) {
        ERRLOG("Get taskid failed.");
        return MP_FAILED;
    }
    mp_string taskId = jvReq["task_id"].asString();
    Json::Value& jvRsp = rsp.GetJsonValueRef();
    INFOLOG("Start to clean job, job id is:%s", taskId.c_str());
    mp_int32 iRet = appProtectServiceInstance->CleanEnv(taskId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("CleanEnv faild.");
        jvRsp["result"] = "false";
        return iRet;
    }
    jvRsp["result"] = "true";
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::WakeUpJob(CRequestMsg& req, CResponseMsg& rsp)
{
    std::shared_ptr<AppProtectService> appProtectServiceInstance = AppProtectService::GetInstance();
    if (appProtectServiceInstance == nullptr) {
        ERRLOG("new AppProtectService failed.");
        return MP_FAILED;
    }
    mp_int32 iRet = appProtectServiceInstance->WakeUpJob(req, rsp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Call appProtectService WakeUpJob failed, ret is %d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::AbortJob(CRequestMsg& req, CResponseMsg& rsp)
{
    std::shared_ptr<AppProtectService> appProtectServiceInstance = AppProtectService::GetInstance();
    if (appProtectServiceInstance == nullptr) {
        ERRLOG("new AppProtectService failed.");
        return MP_FAILED;
    }
    mp_int32 iRet = appProtectServiceInstance->AbortJob(req, rsp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Call appProtectService AbortJob failed, ret is %d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::PluginConfigV1(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = ExternalPluginManager::GetInstance().QueryPluginConfig(strAppType, req, rsp);
    DealInvokePluginFailed(iRet, rsp);
    return iRet;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::DeliverJobStatus(CRequestMsg& req, CResponseMsg& rsp)
{
    std::shared_ptr<AppProtectService> appProtectServiceInstance = AppProtectService::GetInstance();
    if (appProtectServiceInstance == nullptr) {
        ERRLOG("new AppProtectService failed.");
        return MP_FAILED;
    }
    mp_string strAppType = req.GetURL().GetSpecialQueryParam("appType");
    mp_int32 iRet = appProtectServiceInstance->DeliverJobStatus(strAppType, req, rsp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Call appProtectService DeliverJobStatus failed, ret is %d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 AppProtectPlugin::GetESN(CRequestMsg& req, CResponseMsg& rsp)
{
    DBGLOG("Start to get esn.");
    mp_string esn;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_BACKUP_ESN, esn);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Call Function GetValueString failed, ret is %d.", iRet);
        return iRet;
    }
    Json::Value& jValueRsp = rsp.GetJsonValueRef();
    jValueRsp["esn"] = esn;
    return MP_SUCCESS;
}

mp_void AppProtectPlugin::DealInvokePluginFailed(mp_int32 iRet, CResponseMsg& rsp)
{
    if (iRet != MP_SUCCESS) {
        Json::Value &jValueRsp = rsp.GetJsonValueRef();
        if (jValueRsp.isNull()) {
            jValueRsp["code"] = ERR_OPERATION_FAILED;
            jValueRsp["bodyErr"] = ERR_OPERATION_FAILED;
            jValueRsp["message"] = "Agent invoke Plugins failed.";
        }
    }
}

