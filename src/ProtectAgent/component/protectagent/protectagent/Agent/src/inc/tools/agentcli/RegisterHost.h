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
#ifndef _AGENTCLI_REGISTER_HOST_H_
#define _AGENTCLI_REGISTER_HOST_H_

#include "message/curlclient/CurlHttpClient.h"
#include "common/Defines.h"
#include "common/JsonUtils.h"
#include "common/JsonHelper.h"
#define CHANGE_HOSTSN_NOTICE "[Warning:]Duplicate host IP address exists in the ProtectManager.\n"
#define CHANGE_HOSTSN_NOTICE1                                                                       \
    "If you want to overwrite the original host information, the copy of the original host will be" \
    "inherited from the host. The implementation solution is to update the original UUID to the host.\n"
#define CHANGE_HOSTSN_VALUE "Are you sure you want to overwrite the host information?(y/n) "

struct RegisterApplication {
    std::string appValue;
    std::string appDesc;
    std::string appLabel;
    std::string pluginName;
    bool isChosen {false};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(appValue, appValue)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(appDesc, appDesc)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(appLabel, appLabel)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(pluginName, pluginName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isChosen, isChosen)
    END_SERIAL_MEMEBER
};

struct RegisterMenu {
    std::string menuValue;
    std::string menuDesc;
    std::string menuLabel;
    bool isChosen {false};
    std::vector<RegisterApplication> applications;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(menuValue, menuValue)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(menuDesc, menuDesc)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(menuLabel, menuLabel)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isChosen, isChosen)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(applications, applications)
    END_SERIAL_MEMEBER
};

struct RegisterApplicationInfo {
    std::vector<RegisterMenu> registerMenu;
    std::vector<std::string> pluginNames;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(registerMenu, menus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(pluginNames, pluginNames)
    END_SERIAL_MEMEBER
};

class RegisterHost {
public:
    static mp_int32 Handle(const mp_string& actionType, const mp_string& actionType2, const mp_string& actionType3);
    EXTER_ATTACK static mp_int32 RegisterHost2PM();
    static mp_int32 GetPMIPandPort();

private:
    static mp_int32 ReportHost();
    static mp_int32 DeleteHost();
    static mp_int32 DeleteHostFromPM();
    static mp_int32 InitRegisterReq(HttpRequest& req, mp_string& m_PMIp);
    static mp_int32 InitDeleteHostReq(const mp_string& hostid, HttpRequest& req, mp_string& m_PMIp);
    static mp_int32 SendRequest(const HttpRequest& req, mp_string& responseBody, mp_uint32& _statusCode);
    static mp_int32 GetHostInfo(Json::Value& hostInfo);
    static mp_void PrintResult(const mp_string& actionType, const mp_int32 ret);
    static mp_void PrintActionType(const mp_string& actionType);
    static mp_int32 UpdateNginxConf();
    static mp_int32 RestartNginx();
    static mp_void SecurityConfiguration(HttpRequest& req, const mp_string& actionType);
    static mp_int32 ReplaceHostSN(const Json::Value& jsonRspBody);
    static mp_int32 ParseRegisterHostResponds(const mp_string& rspBody);
    static mp_int32 GetAndReplaceHostSN(const mp_string& rspBody);
    static mp_int32 ParseRegisterHostRespondsV2(const mp_string& rspBody, mp_uint32& statusCode);
    static mp_void GetOtherHostInfo(Json::Value& hostInfo);
    static mp_int32 UpdateNginxConfAndRestart();
    static mp_int32 InitUrl(HttpRequest& req, mp_string& m_PMIp);
    static mp_int32 GetExtendInfo(Json::Value& jvalueExtendInfo, mp_string& endpoint);
    static mp_int32 GetHostInfoV2(Json::Value& hostMsg);
    static mp_bool IsInstallDataTurbo();
    static mp_int32 ObtainEsn();
    static mp_int32 QueryEcsMetaDataInfo(const mp_string& agentIpList, Json::Value& jv);
    static mp_int32 SendMetaDataRequest(HttpRequest& req, Json::Value& jv);

    static mp_int32 GetRegisterAppInfo(std::string& registerAppInfo);
    static mp_void DelAppFromMenu(RegisterApplicationInfo& info, const std::string& plugin,
        std::vector<std::string>& unsupportApps);
    static mp_int32 GetValueFromConfFile(const std::string& confFile, const std::string& key, std::string& value,
        const std::string& delimiter = "=");
    static mp_int32 GetUnSupportPlugins(std::vector<std::string>& unsupportPlugins);
    static mp_int32 SetUnSupportApplications(std::vector<std::string>& unsupportApps);
private:
    static std::vector<mp_string> m_PMIpVec;
    static mp_string m_PMPort;
    static mp_string m_outputStr;
    static mp_string pm_ip;
    static mp_string pm_port;
    static mp_int32 m_registerFlag;
    static mp_int32 m_proxyRole;
    static mp_int32 m_registerType;
    static mp_int32 m_errCode;
};

#endif