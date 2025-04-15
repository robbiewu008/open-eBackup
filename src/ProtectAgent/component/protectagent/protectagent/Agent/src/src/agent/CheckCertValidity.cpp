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
#include "agent/CheckCertValidity.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <ctime>
#include "host/host.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/AppVersion.h"
#include "common/Ip.h"
#include "host/host.h"
#include "common/Path.h"
#include "openssl/x509v3.h"
#include "common/JsonUtils.h"

namespace {
    const mp_string MY_CERT = "auto_match";
    const mp_string SEND_WARNING = "/v1/internal/alarms/agent";
    const mp_string SEND_CLEAR_WARNING = "/v1/internal/alarms/agent/action/clear";
    const mp_string SEQUENCE = "1";
    const mp_string ALARM_SOURCE = "Agent";
    const mp_string SOURCE_TYPE = "Certificate";
    const mp_string PARAMS = "ProtectAgent";
    const mp_int32 INT_DEFAULT = 1;
    const mp_string STRING_DEFAULT = "";
    const mp_string PROTOCOL = "http://";
    const mp_string PROTOCOL_SECURE = "https://";
    const mp_int32 DAY_TO_SECOND = 86400;
    const mp_int32 CHECK_CERT_CYCLE_DEFAULT = 86400;

    const mp_int32 ZERO = 0;
    const mp_int32 ONE = 1;
    const mp_int32 TEN = 10;
    const mp_int32 SEVENTY = 70;
    const mp_int32 HUNDRED = 100;
    const mp_int32 THOUSAND = 1000;
    const mp_int32 START_YEAR = 1900;
    const mp_char CHAR_ZERO = '0';
    const mp_int32 MAX_RETRY_TIMES = 3;
    const mp_int32 DELAY_TIME = 3 * 1000;
}

CheckCertValidity CheckCertValidity::m_Instance;

CheckCertValidity::~CheckCertValidity()
{
    m_bNeedExit = MP_TRUE;
    if (m_CheckCertValidityThread.os_id != 0) {
        CMpThread::WaitForEnd(&m_CheckCertValidityThread, NULL);
    }
}

mp_int32 CheckCertValidity::Init()
{
    (mp_void) memset_s(&m_CheckCertValidityThread, sizeof(m_CheckCertValidityThread),
        0, sizeof(m_CheckCertValidityThread));
    mp_int32 iRet = CMpThread::Create(&m_CheckCertValidityThread, CheckCertHandle, this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create checkcertvalidity handle thread failed, iRet = %d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

#ifdef WIN32
DWORD WINAPI CheckCertValidity::CheckCertHandle(LPVOID param)
#else
EXTER_ATTACK mp_void* CheckCertValidity::CheckCertHandle(mp_void* param)
#endif
{
    mp_int32 iRet;
 
    while (m_Instance.m_bNeedExit == MP_FALSE) {
        iRet = m_Instance.GetParameters();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to get parameters value");
            DoSleep(THOUSAND * CHECK_CERT_CYCLE_DEFAULT);
        }

        if (m_Instance.m_RemainTime <= ZERO) {
            COMMLOG(OS_LOG_ERROR, "The certificate already expired");
        } else if (m_Instance.m_RemainTime < m_Instance.m_WarningTime) {
            iRet = m_Instance.SendAlarm();
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "SendAlarm failed.");
            }
        } else {
            iRet = m_Instance.SendClearAlarm();
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "SendClearAlarm failed.");
            }
        }

        DoSleep(THOUSAND * m_Instance.m_CheckCertCycle);
    }

    CMPTHREAD_RETURN;
}

mp_int32 CheckCertValidity::GetParameters()
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SYSTEM_SECTION, CFG_CHECK_CERT_CYCLE, m_CheckCertCycle);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get the check cert cycle value");
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_WARING_TIME, m_WarningTime);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get the warning time value");
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_ALARM_ID, m_AlarmIDString);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get the alarmID value");
        return iRet;
    }
    std::stringstream tmp(m_AlarmIDString);
    tmp >> m_AlarmID;
    
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_PM_CA_INFO, m_CaInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get caInfo value");
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_SSL_CERT, m_SslCert);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get sslCert value");
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_SSL_KEY, m_SslKey);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get sslKey value");
        return iRet;
    }

    iRet = GetCertEndTime();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get the cert end time.");
        return iRet;
    }
    
    CMpTime::Now(m_NowTime);
    m_RemainTime = m_EndTime - m_NowTime;
    m_CheckCertCycle *= DAY_TO_SECOND;
    m_WarningTime *= DAY_TO_SECOND;

    COMMLOG(OS_LOG_DEBUG, "CheckCertCycle = %d, WarningTime = %d, EndTime = %d, NowTime = %d, RemainTime = %d",
        m_CheckCertCycle, m_WarningTime, m_EndTime, m_NowTime, m_RemainTime);

    return MP_SUCCESS;
}

mp_int32 CheckCertValidity::GetCertEndTime()
{
    mp_string caInfoPath = CPath::GetInstance().GetNginxConfFilePath(m_SslCert);
    COMMLOG(OS_LOG_DEBUG, "caInfoPat = %s", caInfoPath.c_str());
    
    BIO* in = BIO_new_file(caInfoPath.c_str(), "r");
    if (!in) {
        COMMLOG(OS_LOG_ERROR, "Failed to get cert end time.");
        return MP_FAILED;
    }

    X509 *pCert = PEM_read_bio_X509(in, NULL, NULL, NULL);
    if (!pCert) {
        COMMLOG(OS_LOG_ERROR, "Failed to get cert end time.");
        BIO_free_all(in);
        return MP_FAILED;
    }

    ASN1_TIME *tEnd = X509_get_notAfter(pCert);
    m_EndTime = Asn1ToTimeT(tEnd);

    BIO_free_all(in);
    X509_free(pCert);
    return MP_SUCCESS;
}

time_t CheckCertValidity::Asn1ToTimeT(ASN1_TIME* time)
{
    struct tm t;
    (mp_void)memset_s(&t, sizeof(t), 0, sizeof(t));
    const mp_char* str = (const mp_char*) time->data;
    mp_int32 i = ZERO;

    if (time->type == V_ASN1_UTCTIME) {  // two digit year
        t.tm_year = (str[i++] - CHAR_ZERO) * TEN;
        t.tm_year += str[i++] - CHAR_ZERO;
        if (t.tm_year < SEVENTY) {
            t.tm_year += HUNDRED;
        }
    } else if (time->type == V_ASN1_GENERALIZEDTIME) {  // four digit year
        t.tm_year = (str[i++] - CHAR_ZERO) * THOUSAND;
        t.tm_year += str[i++] - CHAR_ZERO * HUNDRED;
        t.tm_year += (str[i++] - CHAR_ZERO) * TEN;
        t.tm_year += (str[i++] - CHAR_ZERO);
        t.tm_year -= START_YEAR;
    }

    t.tm_mon = (str[i++] - CHAR_ZERO) * TEN;
    t.tm_mon += str[i++] - CHAR_ZERO - ONE;
    t.tm_mday = (str[i++] - CHAR_ZERO) * TEN;
    t.tm_mday += str[i++] - CHAR_ZERO;
    t.tm_hour = (str[i++] - CHAR_ZERO) * TEN;
    t.tm_hour += str[i++] - CHAR_ZERO;
    t.tm_min = (str[i++] - CHAR_ZERO) * TEN;
    t.tm_min += str[i++] - CHAR_ZERO;
    t.tm_sec  = (str[i++] - CHAR_ZERO) * TEN;
    t.tm_sec += str[i++] - CHAR_ZERO;

    return mktime(&t);
}

mp_int32 CheckCertValidity::SendAlarm()
{
    // 通过告警参数查询db中是否存在同样告警
    alarm_Info_t alarmInfo;
    mp_int32 iRet = AlarmDB::GetCurrentAlarmInfoByAlarmID(m_AlarmID, alarmInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetCurrentAlarmInfoByAlarmID failed, alarmID = %lld", m_AlarmID);
        return iRet;
    }

    // 如果db中没有相同告警，新生成一条
    if (alarmInfo.iAlarmID != m_AlarmID) {
        iRet = NewAlarmRecord();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "NewAlarmRecord failed");
            return iRet;
        }
        COMMLOG(OS_LOG_DEBUG, "Insert database:alarmID=%lld", alarmInfo.iAlarmID);
    }

    iRet = SendMessage(MP_TRUE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send alarm message failed");
        return iRet;
    }
    
    return MP_SUCCESS;
}

mp_int32 CheckCertValidity::SendClearAlarm()
{
    // 通过告警参数查询db中是否存在同样告警
    alarm_Info_t alarmInfo;
    mp_int32 iRet = AlarmDB::GetCurrentAlarmInfoByAlarmID(m_AlarmID, alarmInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetCurrentAlarmInfoByAlarmID failed, alarmID = %lld", m_AlarmID);
        return iRet;
    }

    // 找到了告警记录，直接删除并告警清除
    if (alarmInfo.iAlarmID == m_AlarmID) {
        iRet = AlarmDB::DeleteAlarmInfo(alarmInfo.iAlarmSN, alarmInfo.iAlarmID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Delete alarm failed");
            return iRet;
        }
        COMMLOG(OS_LOG_DEBUG, "Delete database:alarmID=%lld", alarmInfo.iAlarmID);
        iRet = SendMessage (MP_FALSE);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Send alarm clear message failed");
            return iRet;
        }
    }

    return MP_SUCCESS;
}

mp_int32 CheckCertValidity::NewAlarmRecord()
{
    alarm_Info_t alarmInfo;
    alarmInfo.iAlarmSN = INT_DEFAULT;
    alarmInfo.iAlarmID = m_AlarmID;
    alarmInfo.strAlarmParam = PARAMS;
    alarmInfo.iAlarmType = INT_DEFAULT;
    alarmInfo.severity = INT_DEFAULT;
    CMpTime::Now(alarmInfo.strStartTime);
    alarmInfo.strEndTime = 0;
    alarmInfo.iAlarmCategoryType = INT_DEFAULT;

    mp_int32 iRet = AlarmDB::InsertAlarmInfo(alarmInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "InsertAlarmInfo failed");
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 CheckCertValidity::SendMessage(mp_bool isAlarmMsg)
{
    mp_int32 iRet = GetRequestPara();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get request para.");
        return iRet;
    }

    Json::Value reqBody;
    iRet = BuildMessageBody(reqBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to build message body");
        return iRet;
    }

    for (mp_int32 IpNum = 0; IpNum < m_PmIpVec.size(); ++IpNum) {
        HttpRequest req;
        iRet = InitMessage(isAlarmMsg, m_PmIpVec[IpNum], req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to init message");
            continue;
        }

        Json::StreamWriterBuilder builder;
        req.body = Json::writeString(builder, reqBody);

        iRet = m_Instance.SendRequest(req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_WARN, "Send request use PmIp(%s) failed.", m_PmIpVec[IpNum].c_str());
        } else {
            COMMLOG(OS_LOG_DEBUG, "Send request use PmIp(%s) success.", m_PmIpVec[IpNum].c_str());
            break;
        }
    }

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Send request to PM failed.");
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 CheckCertValidity::InitMessage(mp_bool isAlarmMsg, const mp_string &PmIp, HttpRequest &req)
{
    req.method = isAlarmMsg ? "POST" : "PUT";
    mp_string tempIp = CIP::FormatFullUrl(PmIp);
    COMMLOG(OS_LOG_DEBUG, "tempIp: %s", tempIp.c_str());

    if (m_SecureChannel == 1) {
        mp_string domain_name;
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION,
            CFG_DOMAIN_NAME_VALUE, domain_name);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to obtain the domain name.");
            return iRet;
        }

        req.url.append(PROTOCOL_SECURE);
        req.url.append(domain_name);
        req.domaininfo.append(PROTOCOL_SECURE);
        req.domaininfo.append(domain_name);
        req.hostinfo.append(domain_name);
        req.hostinfo.append(":");
        req.hostinfo.append(m_Instance.m_PmPort);
        req.hostinfo.append(":");
        req.hostinfo.append(tempIp);
    } else {
        req.url.append(PROTOCOL);
        req.url.append(tempIp);
    }

    req.url.append(":");
    req.url.append(m_PmPort);
    req.url.append(isAlarmMsg ? SEND_WARNING : SEND_CLEAR_WARNING);
    req.caInfo = m_CaInfo;
    req.sslCert = m_SslCert;
    req.sslKey = m_SslKey;
    req.cert = MY_CERT;

    return MP_SUCCESS;
}

mp_int32 CheckCertValidity::BuildMessageBody(Json::Value &reqBody)
{
    reqBody["sequence"] = SEQUENCE;
    reqBody["alarmSource"] = ALARM_SOURCE;
    reqBody["sourceType"] = SOURCE_TYPE;
    reqBody["alarmId"] = m_AlarmIDString;
    char int64buffer[32] = {0};
    mp_int32 iRet = sprintf_s(int64buffer, sizeof(int64buffer), "%lld", m_NowTime);
    if (iRet == -1) {
        DBGLOG("Exec function sprintf_s failure, ret: '%d'.", iRet);
        return iRet;
    }
    reqBody["createTime"] = std::string(int64buffer);
    mp_int32 days =  m_RemainTime / DAY_TO_SECOND;
    memset_s(int64buffer, sizeof(int64buffer), 0, sizeof(int64buffer));
    iRet = sprintf_s(int64buffer, sizeof(int64buffer), "%d", days);
    if (iRet == -1) {
        DBGLOG("Exec function sprintf_s failure, ret: '%d'.", iRet);
        return iRet;
    }
    mp_string param = PARAMS + " " + m_HostIp + "," + std::string(int64buffer);
    reqBody["params"] = param;
    return MP_SUCCESS;
}

mp_int32 CheckCertValidity::GetRequestPara()
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION,
        CFG_SECURE_CHANNEL, m_SecureChannel);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to obtain the secure communication method.");
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, m_PmIP);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip list address failed.");
        return iRet;
    }

    m_PmIpVec.clear();
    CMpString::StrSplit(m_PmIpVec, m_PmIP, ',');
    if (!m_PmIpVec.empty() && m_PmIpVec.back().empty()) {
        COMMLOG(OS_LOG_ERROR, "Split PM ip failed, PM ip list is empty(%s).", m_PmIP.c_str());
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_IAM_PORT, m_PmPort);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM port failed.");
        return iRet;
    }

    mp_string listenPort;
    iRet = CIP::GetListenIPAndPort(m_HostIp, listenPort);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Agent listen IP and port failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "pm_ip: %s; pm_port: %s; host_ip: %s.", m_PmIP.c_str(), m_PmPort.c_str(), m_HostIp.c_str());
    return MP_SUCCESS;
}

mp_int32 CheckCertValidity::SendRequest(const HttpRequest& req)
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

    COMMLOG(OS_LOG_INFO, "send url:%s info with %d times success.", req.url.c_str(), retryTimes);
    return MP_SUCCESS;
}