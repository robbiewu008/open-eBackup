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
#include "pluginfx/OraclePluginHandler.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Ip.h"
#include "common/DB.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/CMpTime.h"
#include "securecom/CryptAlg.h"
#include "alarm/Trap.h"
#include "alarm/AlarmCode.h"
#include "pluginfx/AutoReleasePlugin.h"

OraclePluginHandler OraclePluginHandler::m_Instance;
const int ARCHIVE_THREAD_TIMEOUT = 1800000;
const mp_string ARCHIVE_THRESHOLD = "80";
const mp_int32 ALARM_SEVERITY_MAJOR = 3;
using defer = std::shared_ptr<void>;

mp_void OraclePluginHandler::CreateCheckArchiveThread()
{
    if (!m_thOracleCheckAchive) {
        m_thOracleCheckAchive = std::make_unique<std::thread>([this]() { CheckArchiveThread(); });
    }
}

mp_void OraclePluginHandler::CheckArchiveThread()
{
    INFOLOG("CheckArchiveThread start to run.");
    while (!m_bTExitCheckArchiveArea) {
        OracleCheckArchiveArea();

        mp_int32 nTimeout = ARCHIVE_THREAD_TIMEOUT;
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_ARCHIVE_THREAD_TIMEOUT, nTimeout);
        CMpTime::DoSleep(nTimeout);
    }
    INFOLOG("CheckArchiveThread stop to run.");
}

mp_int32 OraclePluginHandler::OracleUpdateInitDbInfoJson(const Json::Value& requestParam,
    AppProtect::OracleDBInfo& dbInfo)
{
    if (!requestParam.isObject() || !requestParam.isMember("application") || !requestParam["application"].isObject()) {
        ERRLOG("Check 'application' failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    Json::Value jApplication = requestParam["application"];
    if (!jApplication.isMember("auth") || !jApplication["auth"].isObject()) {
        ERRLOG("Check 'auth' failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    if (!jApplication.isMember("extendInfo") || !jApplication["extendInfo"].isObject()) {
        ERRLOG("Check 'extendInfo' failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    CJsonUtils::GetJsonString(jApplication["extendInfo"], "installUsername", dbInfo.oracleUser);
    CJsonUtils::GetJsonString(jApplication, "name", dbInfo.dbName);
    CJsonUtils::GetJsonString(jApplication["extendInfo"], "inst_name", dbInfo.instanceName);
    CJsonUtils::GetJsonString(jApplication["auth"], "authKey", dbInfo.dbUser);
    CJsonUtils::GetJsonString(jApplication["auth"], "authPwd", dbInfo.dbPassword);
    mp_string strAsmInfo;
    if (!jApplication["auth"].isObject() || !jApplication["auth"].isMember("extendInfo")) {
        ERRLOG("Check 'jApplication' failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    CJsonUtils::GetJsonString(jApplication["auth"]["extendInfo"], "asmInfo", strAsmInfo);
    CJsonUtils::GetJsonString(jApplication["auth"]["extendInfo"], "runUserPwd", dbInfo.runUserPwd);
    if (dbInfo.runUserPwd.empty()) {
        CJsonUtils::GetJsonString(jApplication["auth"]["extendInfo"], "runUserInfo", dbInfo.runUserPwd);
    }
    CJsonUtils::GetJsonString(jApplication["extendInfo"], "accessOracleHome", dbInfo.accessOracleHome);
    CJsonUtils::GetJsonString(jApplication["extendInfo"], "accessOracleBase", dbInfo.accessOracleBase);
    Json::Value jAsmInfo;
    JsonHelper::JsonStringToJsonValue(strAsmInfo, jAsmInfo);
    CJsonUtils::GetJsonString(jAsmInfo, "installUsername", dbInfo.gridUser);
    return MP_SUCCESS;
}
mp_void OraclePluginHandler::OracleUpdateDBInfo(const Json::Value& requestParam)
{
    LOGGUARD("");
    AppProtect::OracleDBInfo dbInfo;
    mp_int32 iRet = OracleUpdateInitDbInfoJson(requestParam, dbInfo);
    if (iRet != MP_SUCCESS) {
        return;
    }
    defer _(nullptr, [&](...) {
        ClearString(dbInfo.dbPassword);
    });
    
    DbParamStream dps;
    std::ostringstream removeStream;
    removeStream << "delete from " << g_OracleDbInfo << " where " << g_OracleDBInfo_DbName << "== ?";
    dps << DbParam(dbInfo.dbName);
    iRet = CDB::GetInstance().ExecSql(removeStream.str(), dps);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Remove oracle db info failed, dbName=%s, iRet=%d.", dbInfo.dbName.c_str(), iRet);
        return;
    }

    dps.Clear();
    std::ostringstream insertStream;
    insertStream << "insert into " << g_OracleDbInfo << "(" << g_OracleDBInfo_DbName << ","
        << g_OracleDBInfo_DbInstance << "," << g_OracleDBInfo_DbUser << "," << g_OracleDBInfo_DbPassword << ","
        << g_OracleDBInfo_ASMInstance << "," << g_OracleDBInfo_ASMUser << "," << g_OracleDBInfo_ASMPassword << ","
        << g_OracleDBInfo_OracleUser << "," << g_OracleDBInfo_GridUser << "," << g_OracleDBInfo_RunUserPwd << ","
        << g_OracleDBInfo_AccessOracleHome << "," << g_OracleDBInfo_AccessOracleBase
        << ") values(?,?,?,?,?,?,?,?,?,?,?,?);";
    mp_string strCipherDBPwd;
    if (!dbInfo.dbPassword.empty()) {
        EncryptStr(dbInfo.dbPassword, strCipherDBPwd);
    }
    mp_string strCipherASMPwd;
    if (!dbInfo.asmPassword.empty()) {
        EncryptStr(dbInfo.asmPassword, strCipherASMPwd);
    }
    mp_string strCipherRunUserPwd;
    if (!dbInfo.runUserPwd.empty()) {
        EncryptStr(dbInfo.runUserPwd, strCipherRunUserPwd);
    }
    dps << DbParam(std::move(dbInfo.dbName)) << DbParam(std::move(dbInfo.instanceName))
        << DbParam(std::move(dbInfo.dbUser)) << DbParam(std::move(strCipherDBPwd))
        << DbParam(std::move(dbInfo.asmName)) << DbParam(std::move(dbInfo.asmUser))
        << DbParam(std::move(strCipherASMPwd)) << DbParam(std::move(dbInfo.oracleUser))
        << DbParam(std::move(dbInfo.gridUser)) << DbParam(std::move(strCipherRunUserPwd))
        << DbParam(std::move(dbInfo.accessOracleHome)) << DbParam(std::move(dbInfo.accessOracleBase));
    iRet = CDB::GetInstance().ExecSql(insertStream.str(), dps);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Insert oracle db info failed, iRet=%d.", iRet);
    }
}

void OraclePluginHandler::OracleQueryDBInfo(std::vector<AppProtect::OracleDBInfo>& vecDBInfo)
{
    DbParamStream dps;
    DBReader readBuff;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;

    std::ostringstream buff;
    buff << "select * from " << g_OracleDbInfo;
    mp_int32 iRet = CDB::GetInstance().QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query oracle db info failed, iRet %d.", iRet);
        return;
    }

    mp_string strDecrypt;
    vecDBInfo.reserve(iRowCount);
    for (mp_int32 i = 0; i < iRowCount; ++i) {
        AppProtect::OracleDBInfo info;
        if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(
            CFG_BACKUP_SECTION, CFG_ARCHIVE_THRESHOLD, info.archThreshold)) {
            ERRLOG("Get Progress Report Interval failed, set Default value %s.", ARCHIVE_THRESHOLD);
            info.archThreshold = ARCHIVE_THRESHOLD;
        }
        info.__set_archThreshold(info.archThreshold);
        readBuff >> info.dbName;
        readBuff >> info.instanceName;
        readBuff >> info.dbUser;
        info.__set_dbUser(info.dbUser);
        readBuff >> strDecrypt;
        if (!strDecrypt.empty()) {
            DecryptStr(strDecrypt, info.dbPassword);
        }
        info.__set_dbPassword(info.dbPassword);
        readBuff >> info.asmName;
        info.__set_asmName(info.asmName);
        readBuff >> info.asmUser;
        info.__set_asmUser(info.asmUser);
        readBuff >> strDecrypt;
        if (!strDecrypt.empty()) {
            DecryptStr(strDecrypt, info.asmPassword);
        }
        info.__set_asmPassword(info.asmPassword);
        readBuff >> info.oracleUser;
        info.__set_oracleUser(info.oracleUser);
        readBuff >> info.gridUser;
        info.__set_gridUser(info.gridUser);
        readBuff >> strDecrypt;
        if (!strDecrypt.empty()) {
            DecryptStr(strDecrypt, info.runUserPwd);
        }
        info.__set_runUserPwd(info.runUserPwd);
        readBuff >> info.accessOracleHome;
        info.__set_accessOracleHome(info.accessOracleHome);
        readBuff >> info.accessOracleBase;
        info.__set_accessOracleBase(info.accessOracleBase);
        vecDBInfo.emplace_back(info);
    }
    ClearString(strDecrypt);
}

mp_void OraclePluginHandler::OracleCheckArchiveArea()
{
    LOGGUARD("");
    std::vector<AppProtect::OracleDBInfo> vecDBInfo;
    OracleQueryDBInfo(vecDBInfo);
    if (vecDBInfo.empty()) {
        DBGLOG("No database information.");
        return;
    }
    defer _(nullptr, [&](...) {
        for (AppProtect::OracleDBInfo& info : vecDBInfo) {
            ClearString(info.dbPassword);
            ClearString(info.asmPassword);
        }
    });

    mp_string strAppType = "Oracle";
    auto plugin = ExternalPluginManager::GetInstance().GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);

    ActionResult _return;
    try {
        appServiceClient->OracleCheckArchiveArea(_return, strAppType, vecDBInfo);
    } catch (apache::thrift::transport::TTransportException &ex) {
        ERRLOG("TTransportException. %s", ex.what());
    } catch (const std::exception &ex) {
        ERRLOG("Standard C++ Exception. %s", ex.what());
    } catch (...) {
        ERRLOG("Unknown exception.");
    }

    AlarmHandle alarmHandle;
    if (_return.message.empty()) {
        alarmHandle.ClearAlarm({ ALARM_ORACLE_ARCHIVE_AREA });
    } else {
        mp_string listenIp;
        mp_int32 listenPort = 0;
        GetNginxListenIP(listenIp, listenPort);
        alarm_param_t alarmParam = { ALARM_ORACLE_ARCHIVE_AREA, listenIp + "," + _return.message };
        alarmParam.severity = ALARM_SEVERITY_MAJOR;
        if (alarmHandle.Alarm(alarmParam) != MP_SUCCESS) {
            CTrapSenderManager::CreateSender(A8000).SendAlarm(alarmParam);
        }
    }
}

std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> OraclePluginHandler::GetApplicationServiceClient(
    const std::shared_ptr<thriftservice::IThriftClient>& pThriftClient)
{
    return pThriftClient->GetConcurrentClientIf<AppProtect::ApplicationServiceConcurrentClient>("ApplicationService");
}