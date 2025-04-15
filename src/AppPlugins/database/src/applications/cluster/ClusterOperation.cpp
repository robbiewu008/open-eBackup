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
#include "ClusterOperation.h"
#include "LocalCmdExector.h"
#include "trjsontostruct.h"
#include "trstructtojson.h"
#include "DBPluginPath.h"
#include "common/File.h"
#include "log/Log.h"

using namespace GeneralDB;
using namespace AppProtect;
namespace {
    const mp_string MODULE_NAME = "ClusterOperation";
    const mp_string CHECK_APPLICATION = "CheckApplication";
    const mp_string LIST_APPLICTION_V1 = "ListApplicationResource";
    const mp_string LIST_APPLICTION_V2 = "ListApplicationResourceV2";
    const mp_string FINAL_CLEAR = "FinalizeClear";
    const mp_string GENERAL_APP_TYPE = "generaldb";
    const mp_string GENERAL_APP_CONF_FILE = "conf.json";
    const mp_int32 RPC_FAILED_CODE = 200;
    const mp_int32 ERR_OPERATION_FAILED = 0x64032B03; // 操作失败
}

void ClusterOperation::DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv)
{
    Json::Value envParam;
    StructToJson(appEnv, envParam);
    Json::Value result;
    Param val;
    val.param = envParam;
    int retRes = LocalCmdExector::GetInstance().Exec(val, result);
    if (retRes != MP_SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Exec failed.retRes = " << retRes << HCPENDLOG;
        return;
    }
    JsonToStruct(result, returnEnv);
    HCP_Log(INFO, MODULE_NAME) << "Discover host cluster success." << HCPENDLOG;
}

void ClusterOperation::DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
    const Application& application)
{
    Json::Value combinParam;
    StructToJson(appEnv, combinParam["appEnv"]);
    StructToJson(application, combinParam["application"]);
    Json::Value result;
    Param val;
    val.param = combinParam;
    val.appType = application.subType;
    val.cmdType = "QueryCluster";
    val.isAsyncInterface = MP_FALSE;
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(application.subType, combinParam["application"],
        val.scriptDir);
    int retRes = LocalCmdExector::GetInstance().Exec(val, result);
    if (retRes != MP_SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Exec failed.retRes = " << retRes << HCPENDLOG;
        return;
    }
    JsonToStruct(result, returnEnv);
    HCP_Log(INFO, MODULE_NAME) << "Discover app cluster success." << HCPENDLOG;
}

mp_void ClusterOperation::CheckApplication(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    HCP_Log(DEBUG, MODULE_NAME) << "Enter CheckApplication general." << HCPENDLOG;
    Json::Value jsValue;
    Json::Value appEnvJs;
    Json::Value applicationJs;

    StructToJson(appEnv, appEnvJs);
    StructToJson(application, applicationJs);
    jsValue["appEnv"] = appEnvJs;
    jsValue["application"] = applicationJs;

    Param shellParam;
    shellParam.param = jsValue;
    shellParam.appType = application.subType;
    shellParam.cmdType = CHECK_APPLICATION;
    shellParam.isAsyncInterface = MP_FALSE;

    Json::Value retValue;
    mp_int32 ret = MP_SUCCESS;
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(application.subType, applicationJs, shellParam.scriptDir);
    ret = LocalCmdExector::GetInstance().Exec(shellParam, retValue);
    if (ret != MP_SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Running script failed." << HCPENDLOG;
        returnValue.__set_code(ret);
        returnValue.__set_bodyErr(ERR_OPERATION_FAILED);
        returnValue.__set_message("Running check application script failed.");
        return;
    }
    JsonToStruct(retValue, returnValue);
    HCP_Log(DEBUG, MODULE_NAME) << "Leave CheckApplication general." << HCPENDLOG;
}

mp_void ClusterOperation::ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application, const ApplicationResource& parentResource)
{
    LOGGUARD("");
    Json::Value jsValue;
    Json::Value appEnvJs;
    Json::Value applicationJs;
    Json::Value parentResourceJs;
    StructToJson(appEnv, appEnvJs);
    StructToJson(application, applicationJs);
    StructToJson(parentResource, parentResourceJs);
    jsValue["appEnv"] = appEnvJs;
    jsValue["application"] = applicationJs;
    jsValue["parentResource"] = parentResourceJs;

    Param shellParam;
    shellParam.param = jsValue;
    shellParam.appType = application.subType;
    shellParam.cmdType = LIST_APPLICTION_V1;
    shellParam.isAsyncInterface = MP_FALSE;

    Json::Value retValue;
    mp_int32 ret = MP_SUCCESS;
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(application.subType, applicationJs, shellParam.scriptDir);
    ret = LocalCmdExector::GetInstance().Exec(shellParam, retValue);
    if (ret != MP_SUCCESS) {
        HCP_Log(ERR, MODULE_NAME) << "Running script failed." << HCPENDLOG;
        return;
    }
    GetException(retValue);
    if (retValue.isObject() && retValue.isMember("resourceList") && retValue["resourceList"].isArray()) {
        for (Json::ArrayIndex index = 0; index < retValue["resourceList"].size(); ++index) {
            ApplicationResource temp;
            JsonToStruct(retValue["resourceList"][index], temp);
            returnValue.push_back(temp);
        }
    } else {
        ERRLOG("ResourceList key not exist or resourceList is not array.");
        return;
    }
}

mp_void ClusterOperation::ListApplicationResourceV2(ResourceResultByPage& returnValue,
    const ListResourceRequest& request)
{
    DBGLOG("Enter ListApplicationResourceV2 general.");
    Json::Value jsValue;
    Json::Value appEnvJs;
    Json::Value conditionJs;

    StructToJson(request.appEnv, appEnvJs);
    StructToJson(request.condition, conditionJs);

    for (int index = 0; index < request.applications.size(); ++index) {
        Json::Value tmp;
        StructToJson(request.applications[index], tmp);
        jsValue["applications"].append(tmp);
    }
    jsValue["appEnv"] = appEnvJs;
    jsValue["condition"] = conditionJs;

    auto lens = request.applications.size();
    if (lens <= 0) {
        ERRLOG("Applications is empty.");
        return;
    }
    Param shellParam;
    shellParam.param = jsValue;
    shellParam.appType = request.applications[lens - 1].subType;
    shellParam.cmdType = LIST_APPLICTION_V2;
    shellParam.isAsyncInterface = MP_FALSE;

    Json::Value retValue;
    mp_int32 ret = MP_SUCCESS;
    Json::Value::ArrayIndex index = jsValue["applications"].size();
    Json::Value applicationJs = jsValue["applications"][index - 1];
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(shellParam.appType, applicationJs, shellParam.scriptDir);
    ret = LocalCmdExector::GetInstance().Exec(shellParam, retValue);
    if (ret != MP_SUCCESS) {
        ERRLOG("Running script failed.");
        return;
    }
    GetException(retValue);
    JsonToStruct(retValue, returnValue);
    DBGLOG("Leave ListApplicationResourceV2 general.");
}

void ClusterOperation::OracleCheckArchiveArea(ActionResult& _return,
    const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList)
{
    INFOLOG("OracleCheckArchiveArea start.");
    Json::Value result;
    Param val;
    for (AppProtect::OracleDBInfo info : dbInfoList) {
        Json::Value tmp;
        StructToJson(info, tmp);
        val.param["dbInfos"].append(tmp);
    }
    val.appType = appType;
    val.cmdType = "OracleCheckArchiveArea";
    val.isAsyncInterface = MP_FALSE;
    int retRes = LocalCmdExector::GetInstance().Exec(val, result);
    if (retRes != MP_SUCCESS) {
        ERRLOG("Exec failed. retRes=%d.", retRes);
        _return.__set_code(RPC_FAILED_CODE);
        _return.__set_bodyErr(ERR_OPERATION_FAILED);
        return;
    }
    JsonToStruct(result, _return);
    INFOLOG("OracleCheckArchiveArea success.");
}

mp_void ClusterOperation::ListApplicationConfig(std::map<std::string, std::string>& returnValue,
    const std::string& script)
{
    LOGGUARD("");
    // script为空，获取所有应用配置
    mp_string generalDBPath = DBPluginPath::GetInstance()->GetScriptPath() + GENERAL_APP_TYPE;
    if (script.empty()) {
        std::vector<std::string> elementList;
        if (Module::CFile::GetFolderDir(generalDBPath, elementList) != MP_SUCCESS) {
            ERRLOG("Get general db subdir failed.");
            return;
        }
        for (const auto &it : elementList) {
            mp_string configFilePath = generalDBPath + "/" + it + "/" + GENERAL_APP_CONF_FILE;
            mp_string fileContent;
            if (Module::CFile::ReadFile(configFilePath, fileContent) != MP_SUCCESS) {
                ERRLOG("Read application config file failed, appType=%s.", it.c_str());
                continue;
            }
            returnValue.insert(std::pair<mp_string, mp_string>(it, fileContent));
        }
    } else {
        std::vector<mp_string> parentDirs;
#ifdef WIN32
        parentDirs.emplace_back("..\\");
#endif
        parentDirs.emplace_back("../");
        for (mp_string parentDir : parentDirs) {
            if (script.find(parentDir) != std::string::npos) {
                ERRLOG("application config file can not contains '../', script=%s.", script.c_str());
                return;
            }
        }
        mp_string configFilePath = generalDBPath + "/" + script + "/" + GENERAL_APP_CONF_FILE;
        mp_string fileContent;
        if (Module::CFile::ReadFile(configFilePath, fileContent) != MP_SUCCESS) {
            ERRLOG("Read application config file failed, appType=%s.", script.c_str());
            return;
        }
        returnValue.insert(std::pair<mp_string, mp_string>(script, fileContent));
    }
}

mp_void ClusterOperation::GetException(const Json::Value &retValue)
{
    if (!retValue.isObject() || !retValue.isMember("exception")) {
        return;
    }
    AppProtectPluginException exception;
    if (retValue["exception"].isObject() && retValue["exception"].isMember("code")
        && retValue["exception"]["code"].isInt()) {
        exception.code = retValue["exception"]["code"].asInt();
    } else {
        WARNLOG("Exception value is not object or exception have no code key.");
        return;
    }
    if (retValue["exception"].isMember("message") && retValue["exception"]["message"].isString()) {
        exception.message = retValue["exception"]["message"].asString();
    }
    if (retValue["exception"].isMember("codeParams") && retValue["exception"]["codeParams"].isArray()) {
        Json::Value codeParams = retValue["exception"]["codeParams"];
        size_t codeParamsNum = codeParams.size();
        for (Json::ArrayIndex index = 0; index < codeParamsNum; ++index) {
            if (codeParams[index].isString()) {
                exception.codeParams.push_back(codeParams[index].asString());
            }
        }
    }
    ERRLOG("Get exception, code=%d, message=%s", exception.code, exception.message.c_str());
    throw exception;
}

mp_void ClusterOperation::RemoveProtect(ActionResult& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    DBGLOG("Enter RemoveProtect general.");
    Json::Value jsValue;
    Json::Value appEnvJs;
    Json::Value applicationJs;
 
    StructToJson(appEnv, appEnvJs);
    StructToJson(application, applicationJs);
    jsValue["appEnv"] = appEnvJs;
    jsValue["application"] = applicationJs;
 
    Param shellParam;
    shellParam.param = jsValue;
    shellParam.appType = application.subType;
    shellParam.cmdType = "RemoveProtect";
    shellParam.isAsyncInterface = MP_FALSE;
 
    Json::Value retValue;
    mp_int32 ret = MP_SUCCESS;
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(application.subType, applicationJs, shellParam.scriptDir);
    ret = LocalCmdExector::GetInstance().Exec(shellParam, retValue);
    if (ret != MP_SUCCESS) {
        ERRLOG("Running script failed.");
        returnValue.__set_code(ret);
        returnValue.__set_bodyErr(ERR_OPERATION_FAILED);
        returnValue.__set_message("Running remove protect script failed.");
        return;
    }
    JsonToStruct(retValue, returnValue);
    DBGLOG("Leave RemoveProtect general.");
}

mp_void ClusterOperation::FinalizeClear(ActionResult& returnValue, const ApplicationEnvironment& appEnv,
    const Application& application, const std::map<std::string, std::string>& extendInfo)
{
    INFOLOG("Enter FinalizeClear general.");
    Json::Value jsValue;
    Json::Value appEnvJs;
    Json::Value applicationJs;
    Json::Value extendInfoJs;

    StructToJson(appEnv, appEnvJs);
    StructToJson(application, applicationJs);
    for (auto iter: extendInfo) {
        extendInfoJs[iter.first] = iter.second;
    }

    jsValue["appEnv"] = appEnvJs;
    jsValue["application"] = applicationJs;
    jsValue["extendInfo"] = extendInfoJs;

    Param shellParam;
    shellParam.param = jsValue;
    shellParam.appType = application.subType;
    shellParam.cmdType = FINAL_CLEAR;
    shellParam.isAsyncInterface = MP_FALSE;

    Json::Value retValue;
    mp_int32 ret = MP_SUCCESS;

    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(shellParam.appType, applicationJs, shellParam.scriptDir);
    ret = LocalCmdExector::GetInstance().Exec(shellParam, retValue);
    if (ret != MP_SUCCESS) {
        ERRLOG("ClusterOperation::FinalizeClear： running finalize clear script failed.");
        return;
    }
    GetException(retValue);
    JsonToStruct(retValue, returnValue);
    INFOLOG("Leave FinalizeClear general.");
}