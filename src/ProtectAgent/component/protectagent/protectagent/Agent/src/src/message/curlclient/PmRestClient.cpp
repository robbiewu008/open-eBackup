/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * @file PmRestClient.cpp
 * @brief common function for access pm
 * @version 1.1.0
 * @date 2023-11-21
 * @author h00668904
 */
#include "common/Log.h"
#include "common/CMpTime.h"
#include "common/Ip.h"
#include "common/ConfigXmlParse.h"
#include "message/curlclient/RestClientCommon.h"
#include "message/tcp/CSocket.h"

#include "message/curlclient/PmRestClient.h"

namespace {
    const mp_uint32 ONE_SECOND = 1 * 1000;  // 1000 ms
    const mp_uint32 MAX_RETRY_TIMES = 3;
    const mp_uint32 INVALID_PORT = 0;
    const mp_string HTTP_PROTOCOL = "http://";
    const mp_string HTTP_PROTOCOL_SECURE = "https://";
    const mp_uint32 SEND_HTTP_MAX_RETRY_TIMES = 3;
    const mp_uint32 SEND_HTTP_DELAY_TIME = 1000;  // 1s delay
}

mp_int32 BaseRestClient::SendReq(const HttpReqCommonParam& httpParam, HttpResponse& httpResponse)
{
    std::vector<mp_string> ipList = httpParam.vecIpList;
    size_t ipListSize = ipList.size();
    DBGLOG("Ip list size: %d", ipListSize);
    if (ipListSize == 0) {
        WARNLOG("Dst ip list is empty, Perhaps it has not been set yet.");
        return MP_FAILED;
    }

    HttpRequest req;
    mp_int32 iRet = IHttpClient::InitStructHttpRequest(req);
    req.method = httpParam.method;
    req.body = httpParam.body;
    req.caInfo = RestClientCommon::SecurityConfiguration(CFG_PM_CA_INFO);
    req.sslCert = RestClientCommon::SecurityConfiguration(CFG_SSL_CERT);
    req.sslKey = RestClientCommon::SecurityConfiguration(CFG_SSL_KEY);
    req.cert = RestClientCommon::SecurityConfiguration(CFG_CERT);
    req.nTimeOut = httpParam.nTimeOut;

    std::shared_ptr<IHttpClient> httpClient = IHttpClient::CreateNewClient();
    if (httpClient == nullptr) {
        ERRLOG("Create new http client failed.");
        return MP_FAILED;
    }

    mp_int32 retryTimes = 0;
    while (retryTimes < SEND_HTTP_MAX_RETRY_TIMES) {
        for (size_t i = 0; i < ipListSize; ++i) {
            UpdateUrlIp(req, ipList[i], httpParam);
            if (RestClientCommon::Send(httpClient.get(), req, httpResponse) == MP_SUCCESS) {
                return ReceiveCheckStatus(httpResponse, req);
            } else {
                CMpTime::DoSleep(SEND_HTTP_DELAY_TIME);
            }
        }
        ERRLOG("Send rest(url:%s), retry times:%d.", req.url.c_str(), retryTimes);
        retryTimes++;
    }
    ERRLOG("Send request(%s) failed.", httpParam.url.c_str());
    return MP_FAILED;
}

mp_int32 BaseRestClient::ReceiveCheckStatus(HttpResponse& httpResponse, HttpRequest req)
{
    mp_int32 iRet = MP_FAILED;
    if (httpResponse.statusCode == SC_OK) {
        DBGLOG("Send url:%s info success.", req.url.c_str());
        iRet = MP_SUCCESS;
    }
    ERRLOG("Send key request(%s) fail, status code: %d.", req.url.c_str(), httpResponse.statusCode);
    return iRet;
}

mp_int32 BaseRestClient::UpdateUrlIp(HttpRequest& req, const mp_string& ip, const HttpReqCommonParam &httpParam)
{
    req.url.clear();
    req.domaininfo.clear();
    req.hostinfo.clear();
    std::string tempIp = CIP::FormatFullUrl(ip);
    if (httpParam.port == INVALID_PORT) {
        COMMLOG(OS_LOG_WARN, "Port is invalid.");
        return MP_FAILED;
    }
    int port = httpParam.port;
    mp_int32 secureChannel;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secureChannel);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Failed to obtain the secure communication method.");
        return iRet;
    }
    if (secureChannel == 1) {
        mp_string domainName;
        iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DOMAIN_NAME_VALUE, domainName);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_WARN, "Failed to obtain the domain name.");
            return iRet;
        }
        req.url.append(HTTP_PROTOCOL_SECURE).append(domainName);
        req.domaininfo.append(HTTP_PROTOCOL_SECURE).append(domainName);
        req.hostinfo.append(domainName).append(":").append(std::to_string(port));
        req.hostinfo.append(":").append(tempIp);
    } else {
        req.url.append(HTTP_PROTOCOL).append(tempIp);
    }
    req.url.append(":").append(std::to_string(port)).append(httpParam.url);
}

mp_int32 PmRestClient::SendRequest(HttpReqCommonParam& httpParam, HttpResponse& httpResponse)
{
    std::vector<std::string> pmIps;
    std::string pmPort;

    if (GetPMIPandPort(pmIps, pmPort) != MP_SUCCESS) {
        ERRLOG("GetPMIPandPort failed.");
        return MP_FAILED;
    }

    pmIps = GetConnectIps(pmIps, pmPort);
    if (pmIps.size() == 0) {
        WARNLOG("pm ip list is empty, Perhaps it has not been set yet.");
        return MP_FAILED;
    }
    httpParam.vecIpList = pmIps;
    httpParam.port = CMpString::SafeStoi(pmPort);
    return SendReq(httpParam, httpResponse);
}

void PmRestClient::SetTimeOut(int timeout)
{
    m_timeout = timeout;
}

mp_int32 PmRestClient::GetPMIPandPort(std::vector<std::string>& pmIps, std::string& pmPort)
{
    mp_string ipstr;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, ipstr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip list address failed.");
        return iRet;
    }

    // 按“，”分割Ip,ip之间注意不要有空格
    std::vector<std::string> ips;
    CMpString::StrSplit(ips, ipstr, ',');
    if (!ips.empty() && ips.back().empty()) {
        COMMLOG(OS_LOG_ERROR, "Split PM ip failed, PM ip list is empty(%s).", ipstr.c_str());
        return MP_FAILED;
    }
    pmIps = ips;

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_IAM_PORT, pmPort);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM port failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Get PM ip %s, port %s success.", ipstr.c_str(), pmPort.c_str());
    return iRet;
}

std::vector<std::string> PmRestClient::GetConnectIps(const std::vector<std::string>& ips, const std::string& port)
{
    std::vector<std::string> connectedIps;
    std::shared_ptr<IHttpClient> httpClient = IHttpClient::CreateNewClient();
    if (httpClient == nullptr) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed when register to PM.");
        return connectedIps;
    }

    for (const auto& ip : ips) {
        mp_int32 retryTimes = 0;
        while (retryTimes < MAX_RETRY_TIMES) {
            if (httpClient->TestConnectivity(ip, port)) {
                connectedIps.push_back(ip);
                COMMLOG(OS_LOG_INFO, "Can cennect ip(%s).", ip.c_str());
                break;
            }
            retryTimes++;
            CMpTime::DoSleep(ONE_SECOND);
            COMMLOG(OS_LOG_WARN, "Can not cennect ip(%s).", ip.c_str());
        }
    }

    return connectedIps;
}