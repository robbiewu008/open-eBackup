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
#include "alarm/AlarmHandle.h"
#include "common/Ip.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "message/rest/interfaces.h"
#include "common/JsonUtils.h"
#include "common/Utils.h"
#include "alarm/Trap.h"
#include "host/ConnectivityManager.h"

#include <fstream>
using namespace std;
namespace {
    const mp_int32 MAX_RETRY_TIMES = 3;
    const mp_uint32 DELAY_TIME = 3 * 1000;  // 3s delay
    const mp_int32 ALARM_SEVERITY_DEFAULT = 1;
    const mp_int32 ALARM_SEVERITY_MAJOR = 3;
    const mp_int64 ALARM_SEQUENCE_DEFAULT = 1;
    const mp_string ALARM_SOURCE_AGENT = "Agent";
    const mp_string ALARM_SOURCE_TYPE_DEFAULT = "backupCluster";
    const mp_int32 ALARM_TYPE_DEVICE = 3;
}
AlarmHandle::AlarmHandle()
{}

AlarmHandle::~AlarmHandle()
{}

mp_int32 AlarmHandle::Alarm(const alarm_param_t& alarmParam)
{
    LOGGUARD("");
    alarm_Info_t alarmInfo;
    mp_int32 iRet = AlarmDB::GetAlarmInfoByParam(alarmParam.iAlarmID, alarmParam.strAlarmParam, alarmInfo);
    if (MP_SUCCESS != iRet) {
        // 记录日志
        COMMLOG(OS_LOG_ERROR, "GetAlarmInfoByParam failed, alarmID = %s, alarmParam = %s",
            alarmParam.iAlarmID.c_str(), alarmParam.strAlarmParam.c_str());
        return iRet;
    }

    // 如果db中没有相同告警，新生成一条
    if (alarmInfo.iAlarmID != alarmParam.iAlarmID) {
        iRet = NewAlarm(alarmParam, alarmInfo);
        if (MP_SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "NewAlarmRecord failed, alarmID = %s, alarmParam=%s",
                alarmParam.iAlarmID.c_str(), alarmParam.strAlarmParam.c_str());
            return iRet;
        } else {
            COMMLOG(OS_LOG_INFO, "newAlarmRecord success, alarmID = %s, alarmParam=%s",
                alarmParam.iAlarmID.c_str(), alarmParam.strAlarmParam.c_str());
        }
    }
    COMMLOG(OS_LOG_INFO, "Alarm, alarmID=%s, alarmParam=%s.",
        alarmInfo.iAlarmID.c_str(), alarmInfo.strAlarmParam.c_str());

    m_bAlarm = true;
    return SendAlarm_Http(alarmInfo);
}

mp_int32 AlarmHandle::Event(const alarm_param_t& eventParam)
{
    alarm_Info_t alarmInfo;
    UpdateAlmInfo(eventParam, alarmInfo);
    m_bAlarm = true;
    return SendAlarm_Http(alarmInfo);
}

mp_int32 AlarmHandle::ClearAlarm(const alarm_param_t& alarmParam)
{
    LOGGUARD("");
    alarm_Info_t alarmInfo;
    (mp_void)AlarmDB::GetCurrentAlarmInfoByAlarmID(alarmParam.iAlarmID, alarmInfo);
    if (alarmInfo.iAlarmSN == -1) {
        return MP_SUCCESS;
    }
    COMMLOG(OS_LOG_INFO, "ClearAlarm, alarmID=%s, alarmParam=%s.",
        alarmInfo.iAlarmID.c_str(), alarmInfo.strAlarmParam.c_str());
    CMpTime::Now(alarmInfo.strEndTime);
    m_bAlarm = false;

    AlarmDB::DeleteAlarmInfo(alarmInfo.iAlarmSN, alarmInfo.iAlarmID);
    return SendAlarm_Http(alarmInfo);
}

mp_int32 AlarmHandle::NewAlarm(const alarm_param_t& alarmParam, alarm_Info_t& alarmInfo)
{
    mp_int32 iCurrSN = 0;
    mp_int32 iRet = AlarmDB::GetSN(iCurrSN);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get CurSN Failed iRet is: %d", iRet);
        return iRet;
    }
    alarmInfo.iAlarmSN = (iCurrSN == MAX_ALARM_ID) ? 0 : (iCurrSN + 1);  // 新流水号
    UpdateAlmInfo(alarmParam, alarmInfo);
    iRet = AlarmDB::InsertAlarmInfo(alarmInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "InsertAlarmInfo failed");
        return iRet;
    }
    // 更新流水号
    iRet = AlarmDB::SetSN(alarmInfo.iAlarmSN);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "SetSN Failed iRet is: %d", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

mp_void AlarmHandle::UpdateAlmInfo(const alarm_param_t& alarmParam, alarm_Info_t& alarmInfo)
{
    alarmInfo.iAlarmID = alarmParam.iAlarmID;
    alarmInfo.iAlarmType = alarmParam.alarmType == -1 ? ALARM_TYPE_EQUPMENTFAULT : alarmParam.alarmType;
    alarmInfo.sourceType = alarmParam.sourceType;
    alarmInfo.severity = alarmParam.severity == -1 ?  ALARM_SEVERITY_DEFAULT : alarmParam.severity;
    CMpTime::Now(alarmInfo.strStartTime);
    alarmInfo.strAlarmParam = alarmParam.strAlarmParam;
    alarmInfo.strEndTime = 0;
    alarmInfo.iAlarmCategoryType = ALARM_CATEGORY_FAULT;
    alarmInfo.iAlarmClass = alarmParam.iAlarmClass;
    alarmInfo.isSuccess = alarmParam.isSuccess;
}

EXTER_ATTACK mp_int32 AlarmHandle::SendAlarm_Http(const alarm_Info_t& alarmInfo)
{
    mp_string strTmp;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, strTmp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip list address failed.");
        return iRet;
    }
    std::vector<mp_string> vecIP;
    CMpString::StrSplit(vecIP, strTmp, ',');
    if (vecIP.empty()) {
        COMMLOG(OS_LOG_ERROR, "Split PM ip failed, PM ip list is empty(%s).", strTmp.c_str());
        return MP_FAILED;
    }
    mp_string port;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_IAM_PORT, port);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM port failed.");
        return iRet;
    }
    
    std::vector<std::string> connectIP =
        ConnectivityManager::GetInstance().GetConnectedIps(vecIP, CMpString::SafeStoi(port));
    if (connectIP.size() == 0) {
        ERRLOG("Failed to get connected IP to send alarm!");
        return MP_FAILED;
    }
    for (int i = 0; i < connectIP.size(); i++) {
        HttpRequest req;
        BuildHttpBody(req, alarmInfo);
        iRet = BuildHttpRequest(req, connectIP[i], port);
        iRet = SendRequest(req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_WARN, "Send Alarm PmIp(%s) failed.", connectIP[i].c_str());
        } else {
            COMMLOG(OS_LOG_INFO, "Send Alarm PmIp(%s) success.", connectIP[i].c_str());
            break;
        }
    }
    return iRet;
}

mp_int32 AlarmHandle::BuildHttpRequest(HttpRequest& req, const mp_string& ip, const mp_string& port)
{
    mp_int32 secure_channel;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secure_channel);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Failed to obtain the secure communication method.");
        return iRet;
    }
    req.method = m_bAlarm ? "POST" : "PUT";
    if (secure_channel == 1) {
        mp_string domain_name;
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(
            CFG_SYSTEM_SECTION, CFG_DOMAIN_NAME_VALUE, domain_name);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to obtain the domain name.");
            return iRet;
        }
        req.url.append("https://").append(domain_name);
        req.domaininfo.append("https://").append(domain_name);
        req.hostinfo.append(domain_name).append(":").append(port).append(":").append(CIP::FormatFullUrl(ip));
    } else {
        req.url.append("http://").append(CIP::FormatFullUrl(ip));
    }
    req.url.append(":").append(port).append(m_bAlarm ? REST_ALARM : REST_ALARM_CLEAR);

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_PM_CA_INFO, req.caInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get caInfo value");
        return iRet;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_SSL_CERT, req.sslCert);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get sslCert value");
        return iRet;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_SSL_KEY, req.sslKey);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get sslKey value");
        return iRet;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_CERT, req.cert);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get cert value");
        return iRet;
    }

    return iRet;
}

mp_void AlarmHandle::BuildHttpBody(HttpRequest& req, const alarm_Info_t& alarmInfo)
{
    Json::Value reqBody;
    reqBody["alarmId"] = alarmInfo.iAlarmID;
    reqBody["alarmType"] = Json::Int64(alarmInfo.iAlarmType == -1 ? ALARM_TYPE_DEVICE : alarmInfo.iAlarmType);
    reqBody["sourceType"] = alarmInfo.sourceType.empty() ? ALARM_SOURCE_TYPE_DEFAULT : alarmInfo.sourceType;
    reqBody["severity"] = Json::Int64(alarmInfo.severity == -1 ? ALARM_SEVERITY_MAJOR : alarmInfo.severity);
    reqBody["sequence"] = Json::Int64(alarmInfo.iAlarmSN == -1 ? ALARM_SEQUENCE_DEFAULT : alarmInfo.iAlarmSN);
    reqBody["alarmSource"] = ALARM_SOURCE_AGENT;
    reqBody["createTime"] = Json::Int64(alarmInfo.strStartTime);
    reqBody["params"] = alarmInfo.strAlarmParam;
    reqBody["type"] = alarmInfo.iAlarmClass;
    reqBody["isSuccess"] = alarmInfo.isSuccess;
    reqBody["resourceId"] = alarmInfo.resourceId;
    Json::StreamWriterBuilder builder;
    req.body = Json::writeString(builder, reqBody);
}

mp_int32 AlarmHandle::SendRequest(const HttpRequest& req)
{
    IHttpClient* httpClient = IHttpClient::GetInstance();
    if (httpClient == NULL) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed when register to PM.");
        return MP_FAILED;
    }

    mp_int32 retryTimes = 0;
    IHttpResponse* dpaHttpRespone = NULL;

    while (retryTimes < MAX_RETRY_TIMES) {
        dpaHttpRespone = httpClient->SendRequest(req, HTTP_TIME_OUT);
        if (dpaHttpRespone == NULL) {
            ++retryTimes;
            COMMLOG(OS_LOG_WARN, "curl http initialization response failed.");
            continue;
        }
        mp_int32 errCode = dpaHttpRespone->GetErrCode();
        mp_uint32 statusCode = dpaHttpRespone->GetHttpStatusCode();
        if (dpaHttpRespone->Success()) {
            COMMLOG(OS_LOG_DEBUG, "status: %u, send times = %d.", dpaHttpRespone->GetHttpStatusCode(), retryTimes + 1);
            break;
        } else {
            COMMLOG(OS_LOG_WARN, "req token failed now, err %d, status code %u.", errCode, statusCode);
            delete dpaHttpRespone;
            dpaHttpRespone = NULL;
        }

        ++retryTimes;
        DoSleep(DELAY_TIME);
    }

    if (dpaHttpRespone) {
        delete dpaHttpRespone;
        dpaHttpRespone = NULL;
    }
    IHttpClient::ReleaseInstance(httpClient);

    if (retryTimes >= MAX_RETRY_TIMES) {
        COMMLOG(OS_LOG_WARN, "send url:%s info with %d times failed.", req.url.c_str(), retryTimes);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "send url:%s info with %d times success.", req.url.c_str(), retryTimes);
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 AlarmHandle::ForwardAlarmReq(const alarm_param_t& alarmParam, bool sendAlarm)
{
    alarm_Info_t alarmInfo;
    TransAlarmParam2AlarmInfo(alarmParam, alarmInfo);
    m_bAlarm = sendAlarm;
    return SendAlarm_Http(alarmInfo);
}

mp_void AlarmHandle::TransAlarmParam2AlarmInfo(const alarm_param_t& alarmParam, alarm_Info_t& alarmInfo)
{
    alarmInfo.iAlarmID = alarmParam.iAlarmID;
    alarmInfo.iAlarmSN = alarmParam.sequence;
    alarmInfo.iAlarmType = alarmParam.alarmType == -1 ? ALARM_TYPE_EQUPMENTFAULT : alarmParam.alarmType;
    alarmInfo.strAlarmParam = alarmParam.strAlarmParam;
    CMpTime::Now(alarmInfo.strStartTime);
    alarmInfo.strEndTime = 0;
    alarmInfo.severity = alarmParam.severity == -1 ? ALARM_SEVERITY_DEFAULT : alarmParam.severity;
    alarmInfo.sourceType = alarmParam.sourceType;
    alarmInfo.iAlarmCategoryType = ALARM_CATEGORY_FAULT;
    alarmInfo.iAlarmClass = alarmParam.iAlarmClass;
    alarmInfo.isSuccess = alarmParam.isSuccess;
    alarmInfo.resourceId = alarmParam.resourceId;
}