#ifdef LINUX
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <random>
#include <algorithm>
#include <curl/curl.h>

#include "message/curlclient/DmeRestClient.h"
#include "common/Log.h"
#include "common/CMpTime.h"
#include "common/Ip.h"
#include "common/ConfigXmlParse.h"
#include "message/curlclient/RestClientCommon.h"
#include "apps/appprotect/AppProtectService.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "message/tcp/CSocket.h"
#include "common/Utils.h"

using namespace AppProtect;

namespace {
const mp_string HOST_ENV_DEPLOYTYPE = "DEPLOY_TYPE";
const mp_string OPENSTACK_METADATA_URL = "/openstack/latest/meta_data.json";

const mp_uint32 SEND_HTTP_DELAY_TIME = 1000;  // 1s delay
const mp_uint32 SEND_HTTP_MAX_RETRY_TIMES = 3;
const mp_uint32 SEND_KEY_REQ_DELAY_TIME = 10000;  // 10s delay
const mp_uint32 SEND_KEY_REQ_MAX_RETRY_TIMES = 10; // 重试时间最大10*10秒
const mp_uint32 INVALID_PORT = 0;
const mp_uint32 TEST_CONNECT_TIMEOUT = 1000;  // 1s 超时
const mp_uint32 TEST_CONNECT_RETRY_TIMES = 3;
};
thread_lock_t DmeRestClient::m_dmeInstanceMutex;
DmeRestClient *DmeRestClient::m_intance = nullptr;
bool DmeRestClient::m_init = MP_FALSE;
DmeRestClient *DmeRestClient::GetInstance()
{
    if (m_intance == nullptr) {
        (mp_void)CMpThread::InitLock(&m_dmeInstanceMutex);
        CThreadAutoLock lock(&m_dmeInstanceMutex);
        m_intance = new DmeRestClient();
        if (m_intance == nullptr) {
            ERRLOG("Initialize DmeRestClient");
            return nullptr;
        }
    }
    if (!m_init && (m_intance->UpdateSecureInfo() != MP_SUCCESS)) {
        ERRLOG("Get secure info failed.");
        return nullptr;
    }
    m_init = MP_TRUE;
    return m_intance;
}

DmeRestClient::DmeRestClient()
{
    (mp_void)CMpThread::InitLock(&m_dmeInfoMutex);
    if (CIP::GetHostIPList(m_localIpv4List, m_localIpv6List) != MP_SUCCESS) {
        ERRLOG("Get host ip list failed.");
    }
    if (CIP::GetHostEnv(HOST_ENV_DEPLOYTYPE, m_strDeployType) != MP_SUCCESS) {
        ERRLOG("Get host deploytype failed .");
    }
}

DmeRestClient::~DmeRestClient()
{
    if (m_intance) {
        delete m_intance;
        m_intance = nullptr;
        (mp_void)CMpThread::DestroyLock(&m_dmeInfoMutex);
    }
}

mp_void DmeRestClient::UpdateDmeAddr(
    const mp_string &mainJobId, const std::vector<mp_string> &dmeIpList, mp_uint32 dmePort)
{
    {
        std::lock_guard<std::mutex> lock(m_dmeIpMapMutex);
        auto pos = m_mainJobDmeIpList.find(mainJobId);
        if (pos != m_mainJobDmeIpList.end()) {
            DBGLOG("mainJobId is exist,no need to update ubc iplist.");
            return;
        }
    }
    std::vector<mp_string> srcDmeIpList = dmeIpList;
    std::vector<mp_string> dstDmeIpList;
    RefreshDmeAddrOrder(srcDmeIpList, dstDmeIpList);
    {
        CThreadAutoLock lock(&m_dmeInfoMutex);
        m_dmeIpList.clear();
        m_dmeIpList = dstDmeIpList;
        m_dmePort = dmePort;
    }
    {
        std::lock_guard<std::mutex> lock(m_dmeIpMapMutex);
        m_mainJobDmeIpList.insert(std::make_pair(mainJobId, dstDmeIpList));
    }
    INFOLOG("Update Dme addr success.");
}

std::vector<mp_string> DmeRestClient::GetDmeAddr()
{
    CThreadAutoLock lock(&m_dmeInfoMutex);
    return m_dmeIpList;
}

mp_int32 DmeRestClient::GetDmeAddrByMainId(const mp_string &mainJobId, std::vector<mp_string> &ubcIps)
{
    std::lock_guard<std::mutex> lock(m_dmeIpMapMutex);
    if (m_mainJobDmeIpList.find(mainJobId) != m_mainJobDmeIpList.end()) {
        ubcIps=m_mainJobDmeIpList[mainJobId];
        return MP_SUCCESS;
    }
    ERRLOG("MainJob is not exists. jobId=%s", mainJobId.c_str());
    return MP_FAILED;
}
 
mp_void DmeRestClient::DeleteDmeAddrByMainId(const mp_string &mainJobId)
{
    std::lock_guard<std::mutex> lock(m_dmeIpMapMutex);
    if (m_mainJobDmeIpList.find(mainJobId) != m_mainJobDmeIpList.end()) {
        m_mainJobDmeIpList.erase(mainJobId);
    }
    return;
}
 
// 将连通性好的ip放在数组前面, 连通性差的ip放在后面
void DmeRestClient::RefreshDmeAddrOrder(const std::vector<mp_string>& srcDmeIpList,
    std::vector<mp_string>& dstDmeIpList)
{
    dstDmeIpList.resize(srcDmeIpList.size());
    int start = 0;
    int end = dstDmeIpList.size();
    for (const auto& ip : srcDmeIpList) {
        if (IsDmeIpValid(ip)) {
            dstDmeIpList[start] = ip;
            ++start;
        } else {
            --end;
            dstDmeIpList[end] = ip;
        }
    }
    if (start > 1) {
        // 随机打乱能通的ip列表，负荷分担
        std::random_device rd;
        std::shuffle(dstDmeIpList.begin(), dstDmeIpList.begin() + start, rd);
    }
    std::string ips;
    for (const auto& ip : dstDmeIpList) {
        ips += (ip + ",");
    }
    INFOLOG("Dme ips %s.", ips.c_str());
}

// 若发送时发现使用的首个ip不通，其他的通，则进行更新（将连通性更好的ip提前），防止网络不好时，连通性检测可能过不了
void DmeRestClient::AdjustMoreProperDmeIp(int index, const std::string& ip)
{
    CThreadAutoLock lock(&m_dmeInfoMutex);
    if (index == 0) {                      // 使用的就是首个ip时不进行刷新
        return;
    }
    if (index >= m_dmeIpList.size()) {
        return;
    }

    auto iter = m_dmeIpList.begin() + index;
    if (*iter == ip) {
        m_dmeIpList.erase(iter);
        m_dmeIpList.insert(m_dmeIpList.begin(), ip);
        INFOLOG("Find ip %s link status more properly.", ip.c_str());
    }
}

void DmeRestClient::AdjustMoreProperDmeIpByMainId(const mp_string &mainJobId, const mp_string &ip)
{
    LOGGUARD("");
    std::lock_guard<std::mutex> lock(m_dmeIpMapMutex);
    auto iter = m_mainJobDmeIpList.find(mainJobId);
    if (iter == m_mainJobDmeIpList.end()) {
        return;
    }
    std::vector<mp_string> ubcIps = m_mainJobDmeIpList[mainJobId];
    auto pos = std::find_if(ubcIps.begin(), ubcIps.end(), [&](const mp_string &item) -> bool { return ip == item; });
    if (pos == std::begin(ubcIps) || pos == std::end(ubcIps)) {
        return;
    }
    ubcIps.erase(pos);
    ubcIps.insert(ubcIps.begin(), ip);
    m_mainJobDmeIpList[mainJobId] = std::move(ubcIps);
    INFOLOG("Ip %s is change to the first of ubc iplist.", ip.c_str());
}

mp_bool DmeRestClient::IsDmeIpValid(const mp_string& dmeIp)
{
    mp_string ip = dmeIp;
#ifdef LINUX
    if (m_strDeployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT_NO_BRAND ||
        m_strDeployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT_CYBER_ENGINE ||
        m_strDeployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT) {
        struct hostent *host_entry = nullptr;
        host_entry = gethostbyname(m_domainName.c_str());
        if (host_entry == nullptr) {
            ERRLOG("Get domain:%s ip failed may be network have some problems", dmeIp.c_str());
            return MP_FALSE;
        }
        ip = inet_ntoa(*((struct in_addr*)
                        host_entry->h_addr_list[0]));
        DBGLOG("Get domain: %s ip: %s.", m_domainName.c_str(), ip.c_str());
    }
#endif

    std::vector<mp_string> useLocalIps;
    if (StaticConfig::IsInnerAgentMainDeploy()) {
        mp_string agentIp;
        if (!StaticConfig::GetAgentIp(agentIp)) {
            COMMLOG(OS_LOG_ERROR, "Get agent ip failed.");
            return MP_FALSE;
        }
        useLocalIps.push_back(agentIp);
    } else {
        mp_bool isIpv4;
        if (CIP::IsIPV4(ip)) {
            isIpv4 = true;
        } else if (CIP::IsIPv6(ip)) {
            isIpv4 = false;
        } else {
            ERRLOG("ip address %s is invalid.", ip.c_str());
            return MP_FALSE;
        }
        useLocalIps = isIpv4 ? m_localIpv4List : m_localIpv6List;
    }

    for (auto localIp : useLocalIps) {
        if (CSocket::CheckHostLinkStatus(localIp, ip, m_dmePort) == MP_SUCCESS) {
            INFOLOG("Local ip %s link dst ip %s success.", dmeIp.c_str(), ip.c_str());
            return MP_TRUE;
        }
    }
    WARNLOG("Can not link dst ip %s.", ip.c_str());
    return MP_FALSE;
}

mp_void DmeRestClient::ChangeCertForInternalTask(const HttpReqParam &httpParam, HttpRequest &req)
{
    if (httpParam.method == "GET") {
        // 内置代理/v1/internal/dme-unified/tasks请求使用protectengine.dpa.svc.cluster.local对应的证书
        if (httpParam.url.find("/v1/internal/dme-unified/tasks/") == 0) {
            req.caInfo = INTERNAL_CA_FILE;
            req.sslCert = INTERNAL_CERT_FILE;
            req.sslKey = INTERNAL_KEY_FILE;
        }
    }
}

mp_int32 DmeRestClient::SendRequest(const HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    std::vector<mp_string> ipList;
    if (!httpParam.vecIpList.empty()) {
        ipList = httpParam.vecIpList;
    } else if (httpParam.mainJobId.empty() || GetDmeAddrByMainId(httpParam.mainJobId, ipList) != MP_SUCCESS) {
        ipList = GetDmeAddr();
    }
    size_t ipListSize = ipList.size();
    DBGLOG("Ip list size: %d", ipListSize);
    if (ipListSize == 0) {
        WARNLOG("Dme ip list is empty, Perhaps it has not been set yet.");
        return MP_FAILED;
    }

    HttpRequest req;
    mp_int32 iRet = IHttpClient::InitStructHttpRequest(req);
    req.method = httpParam.method;
    req.body = httpParam.body;

    req.caInfo = RestClientCommon::SecurityConfiguration(CFG_PM_CA_INFO);
    req.sslCert = RestClientCommon::SecurityConfiguration(CFG_SSL_CERT);
    req.sslKey = RestClientCommon::SecurityConfiguration(CFG_SSL_KEY);
    ChangeCertForInternalTask(httpParam, req);
    req.cert = RestClientCommon::SecurityConfiguration(CFG_CERT);
    req.nTimeOut = httpParam.nTimeOut;

    IHttpClient* httpClient = IHttpClient::GetInstance();
    if (httpClient == nullptr) {
        ERRLOG("SendRequest failed.");
        return MP_FAILED;
    }

    mp_int32 retryTimes = 0;
    while (retryTimes < SEND_HTTP_MAX_RETRY_TIMES) {
        for (size_t i = 0; i < ipListSize; ++i) {
            UpdateUrlIp(req, ipList[i], httpParam);
            if (RestClientCommon::Send(httpClient, req, httpResponse) == MP_SUCCESS) {
                DBGLOG("Send url:%s info success.", req.url.c_str());
                AdjustMoreProperDmeIp(i, ipList[i]);
                AdjustMoreProperDmeIpByMainId(httpParam.mainJobId, ipList[i]);
                IHttpClient::ReleaseInstance(httpClient);
                return MP_SUCCESS;
            }
            CMpTime::DoSleep(SEND_HTTP_DELAY_TIME);
        }
        ERRLOG("Send rest(url:%s), retry times:%d.", req.url.c_str(), retryTimes);
        retryTimes++;
    }
    ERRLOG("Send request(%s) failed.", httpParam.url.c_str());
    IHttpClient::ReleaseInstance(httpClient);
    return MP_FAILED;
}

mp_int32 DmeRestClient::SendKeyRequest(const HttpReqParam &httpParam, HttpResponse& httpResponse, mp_int32 expectCode)
{
    // for some key request, we expect it can send successfully
    int retryCount = 0;
    int maxRetryTimes = SEND_KEY_REQ_MAX_RETRY_TIMES;
    int retryInterval = SEND_KEY_REQ_DELAY_TIME;
    while (true) {
        SendRequest(httpParam, httpResponse);
        if (httpResponse.statusCode == expectCode) {
            INFOLOG("Send key request(%s) success, status code: %d.", httpParam.url.c_str(), httpResponse.statusCode);
            return MP_SUCCESS;
        }
        if (retryCount > maxRetryTimes) {
            break;
        }
        WARNLOG("Send key request(%s) fail, status code: %d.", httpParam.url.c_str(), httpResponse.statusCode);
        retryCount++;
        CMpTime::DoSleep(retryInterval);
    }
    ERRLOG("Send key request(%s) fail after retry %d times.", httpParam.url.c_str(), maxRetryTimes);
    return MP_FAILED;
}

EXTER_ATTACK mp_int32 DmeRestClient::UpdateUrlIp(HttpRequest& req, const mp_string &ip, const HttpReqParam &httpParam)
{
    req.url.clear();
    req.domaininfo.clear();
    req.hostinfo.clear();
    std::string tempIp = CIP::FormatFullUrl(ip);
    mp_uint32 port = m_dmePort;
    if (httpParam.port != INVALID_PORT) {
        port = httpParam.port;
    }
    mp_string domainName = "";
    domainName = m_domainName;
    if (m_secureChannel == 1) {
        if (httpParam.method == "GET") {
            // 内置代理的GET的/v1/internal/dme-unified/tasks请求，服务端转向protectengine.dpa.svc.cluster.local:18089
            if (httpParam.url.find("/v1/internal/dme-unified/tasks/") == 0) {
                domainName = "protectengine.dpa.svc.cluster.local";
                port = INTERNAL_DME_PORT;
                req.notBindToDevice = true;
            }
        }
 
        req.url.append("https://").append(domainName);
        req.domaininfo.append("https://").append(domainName);
        req.hostinfo.append(domainName).append(":").append(std::to_string(port)).append(":");
        req.hostinfo.append(tempIp);
        mp_string deploytypeEnv;
        CIP::GetHostEnv(HOST_ENV_DEPLOYTYPE, deploytypeEnv);
        if (deploytypeEnv == HOST_ENV_DEPLOYTYPE_HYPERDETECT ||
            deploytypeEnv == HOST_ENV_DEPLOYTYPE_HYPERDETECT_NO_BRAND ||
            deploytypeEnv ==  HOST_ENV_DEPLOYTYPE_HYPERDETECT_CYBER_ENGINE) {
            req.hostinfo = "";
        }
        // req.hostinfo为域名对应的IP，但内置代理场景下protectengine.dpa.svc.cluster.local域名不是DmeIpList的IP，需要DNS重新解析，不能写死，此处置为空
        if (httpParam.method == "GET" && httpParam.url.find("/v1/internal/dme-unified/tasks/") == 0) {
            req.hostinfo = "";
        }
    } else {
        req.url.append("http://").append(tempIp);
    }
    req.url.append(":").append(std::to_string(port)).append(httpParam.url);
    return MP_SUCCESS;
}

mp_int32 DmeRestClient::UpdateSecureInfo()
{
    mp_int32 iRet =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, m_secureChannel);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to obtain the secure communication method.");
        return iRet;
    }

    mp_string cfgDomainName = CFG_DOMAIN_NAME_VALUE;
    mp_string deploytypeEnv;
    CIP::GetHostEnv(HOST_ENV_DEPLOYTYPE, deploytypeEnv);
    if (deploytypeEnv == HOST_ENV_DEPLOYTYPE_HYPERDETECT ||
        deploytypeEnv == HOST_ENV_DEPLOYTYPE_HYPERDETECT_NO_BRAND ||
        deploytypeEnv ==  HOST_ENV_DEPLOYTYPE_HYPERDETECT_CYBER_ENGINE) {
        cfgDomainName = CFG_DOMAIN_NAME_VALUE_DME;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, cfgDomainName, m_domainName);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to obtain the domain name.");
    }
    return iRet;
}

mp_int32 DmeRestClient::CheckEcsIpConnect(mp_string& metadataServerIp, mp_string& metadataServerPort)
{
    if (CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION,
                                                       CFG_OPENSTACK_METADATA_SERVER_IP,
                                                       metadataServerIp) == MP_FAILED) {
        ERRLOG("Get openstack_metadata_server_ip from config file failed.");
        return MP_FAILED;
    }
    if (CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION,
                                                       CFG_OPENSTACK_METADATA_SERVER_PORT,
                                                       metadataServerPort) == MP_FAILED) {
        ERRLOG("Get openstack_metadata_server_port from config file failed.");
        return MP_FAILED;
    }
    CURL* curl;
    CURLcode res;
    mp_int32 checkResult = MP_FAILED;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        // 设置目标URL，包括端口号
        mp_string curlUrl = "http://" + metadataServerIp + ":" + metadataServerPort;
        DBGLOG("The curlUrl is %s.", curlUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, curlUrl.c_str());

        // 跳过证书验证（虽然对于HTTP协议可能没有实际意义，但可以设置）
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        // 设置超时时间为10秒
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);  // 超时时间设置为10秒

        // 执行请求
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            if (res == CURLE_OPERATION_TIMEDOUT) {
                WARNLOG("CheckEcsIpConnect curl timeout, error code: %d.", res);
            } else {
                WARNLOG("CheckEcsIpConnect curl error str is: %s", curl_easy_strerror(res));
            }
        } else {
            // 输出服务器响应内容
            checkResult = MP_SUCCESS;
            INFOLOG("CheckEcsIpConnect curl success.");
        }
        // 清理
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return checkResult;
}

mp_int32 DmeRestClient::CheckEcsMetaInfo()
{
    mp_string metadataServerIp;
    mp_string metadataServerPort;
    
    if (CheckEcsIpConnect(metadataServerIp, metadataServerPort) == MP_FAILED) {
        return MP_FAILED;
    }
    HttpRequest req;
    if (IHttpClient::InitStructHttpRequest(req) != MP_SUCCESS) {
        ERRLOG("Init http request fail.");
        return MP_FAILED;
    }
    req.method = "GET";
    req.url = "http://" + CIP::FormatFullUrl(metadataServerIp) + ":" + metadataServerPort + OPENSTACK_METADATA_URL;
    req.caInfo = RestClientCommon::SecurityConfiguration(CFG_PM_CA_INFO);
    req.sslCert = RestClientCommon::SecurityConfiguration(CFG_SSL_CERT);
    req.sslKey = RestClientCommon::SecurityConfiguration(CFG_SSL_KEY);
    req.cert = RestClientCommon::SecurityConfiguration(CFG_CERT);
    IHttpClient* httpClient = IHttpClient::GetInstance();
    if (httpClient == nullptr) {
        ERRLOG("Get http instance failed");
        return MP_FAILED;
    }
    HttpResponse httpResponse;
    mp_int32 tryCnt = 0;
    while (tryCnt++ < SEND_HTTP_MAX_RETRY_TIMES) {
        INFOLOG("Begin send to meta data server times %d.", tryCnt);
        if (RestClientCommon::Send(httpClient, req, httpResponse) == MP_SUCCESS) {
            DBGLOG("Send url:%s info success.", req.url.c_str());
            IHttpClient::ReleaseInstance(httpClient);
            return MP_SUCCESS;
        } else {
            CMpTime::DoSleep(SEND_HTTP_DELAY_TIME);
        }
        ERRLOG("Send url failed messaget is %s", httpResponse.body.c_str());
    }
    IHttpClient::ReleaseInstance(httpClient);
    return MP_FAILED;
}
