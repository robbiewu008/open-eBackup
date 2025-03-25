/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CurlHttpClient.cpp
 * @brief  Contains function declarations HTTP Send And Response
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include <sstream>
#include <set>
#include <algorithm>
#include "message/curlclient/CurlHttpClient.h"
#ifndef WIN32
#include "message/curlclient/SSLHandle.h"
#endif
#include "common/Log.h"
#include "common/MpString.h"
#include "common/Ip.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "common/ConfigXmlParse.h"
#include "common/Defines.h"
#include "common/Utils.h"

using namespace std;
#define CURL_STRING_TO_INTEGER(ss, strVal, intVal)                                                                     \
do     {                                                                                                               \
        (ss) << (strVal);                                                                                              \
        (ss) >> (intVal);                                                                                              \
        (ss).clear();                                                                                                  \
    } while (0)

static const int MP_SUCCESS_NO_IMPORTED = -1;
static const int MP_SUCCESS_IS_IMPORTED = 0;
namespace {
const int DEFAULT_TEST_CONNECTIVITY_TIMEOUT = 5;
const uint64_t HTTPS_ABNORMAL_RETRY_INTERVAL = 60;
const uint64_t NAME_NOT_RESOLV_RETRY_COUNT = 3;
const mp_int32 THOUSAND = 1000;
using Defer = std::shared_ptr<void>;
const std::set<std::pair<std::string, std::string>> INNER_REQUEST_URI = {
    std::make_pair("GET", "/v1/internal/dme-unified/tasks/"),
    std::make_pair("POST", "/v1/internal/deviceManager/rest/ip_rule/")
};
}

CurlHttpResponse::CurlHttpResponse() : IHttpResponse(), m_StatusCode(0), m_ErrorCode(CURLE_FAILED_INIT)
{
    m_Curl = NULL;
}

CurlHttpResponse::~CurlHttpResponse()
{
    if (m_Curl != NULL) {
        curl_easy_cleanup(m_Curl);
        m_Curl = NULL;
    }
}

mp_bool CurlHttpResponse::Init()
{
    m_Curl = curl_easy_init();
    if (m_Curl == NULL) {
        COMMLOG(OS_LOG_DEBUG, "curl initialize failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}

EXTER_ATTACK mp_int32 CurlHttpResponse::ConfigureTwoWayCertsAuthenticate(const HttpRequest& req)
{
    mp_int32 secure_channel;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secure_channel);
    if (secure_channel == 0) {
        return MP_SUCCESS;
    }

    // Converting Domain Names to IP Addresses
    DBGLOG("Converting Domain Names to IP Addresses");
    DBGLOG("Http url:%s, domain:%s ,hostinfo:%s",
        req.url.c_str(), req.domaininfo.c_str(), req.hostinfo.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_URL, req.domaininfo.c_str());

    iRet = SetCertInCurl(req);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Certificate information set up failed.");
        return MP_FAILED;
    }

    mp_string CipherStr;
    // 内置代理请求使用protectengine.dpa.svc.cluster.local对应的证书私钥密码
    if (IsInnerRequestUri(req)) {
        vector<mp_string> interKeyFilePass;
        iRet = CMpFile::ReadFile(INTERNAL_KEY_PASSWD_FILE, interKeyFilePass);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Read inter key file failed.");
            return iRet;
        }
        CipherStr = interKeyFilePass[0];
    } else {
        iRet = CConfigXmlParser::GetInstance().GetValueString(
            CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, CipherStr);
        if (iRet != MP_SUCCESS) {
            ERRLOG("get GetValueString of ssl_key_password failed.");
            return MP_FAILED;
        }
    }

    mp_string outStr;
    DecryptStr(CipherStr, outStr);
    if (outStr.empty()) {
        ERRLOG("DecryptStr private key password failed.");
        return MP_FAILED;
    }

    // client cert private key passwd
    curl_easy_setopt(m_Curl, CURLOPT_SSLKEYPASSWD, outStr.c_str());
    outStr.replace(0, outStr.length(), "");  // clear memory passwd
    return MP_SUCCESS;
}

bool CurlHttpResponse::IsInnerRequestUri(const HttpRequest& req)
{
    for (const auto &item : INNER_REQUEST_URI) {
        if ((item.first == req.method) && (req.url.find(item.second) != std::string::npos)) {
            DBGLOG("Inner request, method: %s, uri: %s", req.method.c_str(), req.url.c_str());
            return true;
        }
    }
    return false;
}

mp_int32 CurlHttpResponse::SetCertInCurl(const HttpRequest& req)
{
    // verfify server cerificate
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYPEER, 1);
    // don't verify host name
    curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYHOST, 0);
    if (req.caInfo.empty()) {
        ERRLOG("Two-way certs authenticate configure failed because of caInfo is empty.");
        return NOT_SET_ALL_TWO_WAY_CERTS;
    }
    if (req.sslCert.empty()) {
        ERRLOG("Two-way certs authenticate configure failed because of sslCert is empty.");
        return NOT_SET_ALL_TWO_WAY_CERTS;
    }
    if (req.sslKey.empty()) {
        ERRLOG("Two-way certs authenticate configure failed because of sslKey is empty.");
        return NOT_SET_ALL_TWO_WAY_CERTS;
    }

    mp_string caInfoPath = "";
    mp_string sslCertPath = "";
    mp_string sslKeyPath = "";
    // 内置代理请求使用protectengine.dpa.svc.cluster.local对应的证书
    if (IsInnerRequestUri(req)) {
        caInfoPath = req.caInfo;
        sslCertPath = req.sslCert;
        sslKeyPath = req.sslKey;
    } else {
        caInfoPath = CPath::GetInstance().GetNginxConfFilePath(req.caInfo);
        sslCertPath = CPath::GetInstance().GetNginxConfFilePath(req.sslCert);
        sslKeyPath = CPath::GetInstance().GetNginxConfFilePath(req.sslKey);
    }

    // the path of CA
    curl_easy_setopt(m_Curl, CURLOPT_CAINFO, caInfoPath.c_str());
    // set client cert path and tyoe
    curl_easy_setopt(m_Curl, CURLOPT_SSLCERT, sslCertPath.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_SSLCERTTYPE, "PEM");
    // set client cert private key path and type
    curl_easy_setopt(m_Curl, CURLOPT_SSLKEY, sslKeyPath.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_SSLKEYTYPE, "PEM");
    // Check CRL is imported
    mp_int32 iretCheckCRLStatus = CheckCRLStatus();
    string sslCRL;
    if (iretCheckCRLStatus == MP_SUCCESS_IS_IMPORTED) {
        mp_int32 iRetGetValueString = CConfigXmlParser::GetInstance().GetValueString(
            CFG_SECURITY_SECTION, CFG_SSL_CRL, sslCRL);
        if (iRetGetValueString != MP_SUCCESS) {
            ERRLOG("Failed to get sslCRL value. iRet = %d", iRetGetValueString);
            return iRetGetValueString;
        }
        mp_string sslCRLPath = CPath::GetInstance().GetNginxConfFilePath(sslCRL);
        curl_easy_setopt(m_Curl, CURLOPT_CRLFILE, sslCRLPath.c_str());
        return MP_SUCCESS;
    }
    if (iretCheckCRLStatus == MP_SUCCESS_NO_IMPORTED) {
        return MP_SUCCESS;
    }
    return MP_ERROR;
}

mp_int32 CurlHttpResponse::CheckCRLStatus()
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);

    std::vector<mp_string> vecOutput;
    mp_int32 iRetReadFile = CMpFile::ReadFile(strFilePath, vecOutput);
    if (MP_SUCCESS != iRetReadFile) {
        ERRLOG("Read file failed[%s].", strFilePath.c_str());
        return MP_ERROR;
    }

    for (const std::string& it : vecOutput) {
        if (it.find("CERTIFICATE_REVOCATION=") != 0) {
            continue;
        } else {
            mp_string strStatus = it.substr(it.find("=", 0) + 1);
            if (!strStatus.empty() && CMpString::SafeStoi(strStatus, 0) == 0) {
                DBGLOG("Certificate is not currently imported.");
                return MP_SUCCESS_NO_IMPORTED;
            }
            if (!strStatus.empty() && CMpString::SafeStoi(strStatus, 0) == 1) {
                WARNLOG("Certificate is currently imported.");
                return MP_SUCCESS_IS_IMPORTED;
            }
            ERRLOG("CERTIFICATE_REVOCATION value is abnormal");
            return MP_ERROR;
        }
    }

    ERRLOG("Can not find CERTIFICATE_REVOCATION in testcfg.tmp");
    return MP_ERROR;
}

bool CurlHttpResponse::TestConnectivity(const std::string& ip, const std::string& port,
    uint32_t testTimeout, const std::string& srcHost)
{
    std::string url = "http://";
    if (CIP::IsIPV4(ip)) {
        url += ip;
    } else {
        url = url + "[" + ip + "]";
    }
    url += (":" + port);

    int timeout = DEFAULT_TEST_CONNECTIVITY_TIMEOUT;
    if (testTimeout == 0) {
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_CURL_CONNECTIVITY_TIMEOUT, timeout);
        timeout = (timeout > 0) ? timeout : DEFAULT_TEST_CONNECTIVITY_TIMEOUT;
    } else {
        timeout = testTimeout;
    }

    char curl_error_str[CURL_ERROR_SIZE] = {0};

    curl_easy_setopt(m_Curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(m_Curl, CURLOPT_ERRORBUFFER, curl_error_str);
    curl_easy_setopt(m_Curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_CONNECT_ONLY, 1L);
    curl_easy_setopt(m_Curl, CURLOPT_CONNECTTIMEOUT, timeout);
    curl_easy_setopt(m_Curl, CURLOPT_TIMEOUT, timeout);

    if (ip != LOCAL_IPADDR) {
        SetBindToDevice(srcHost);
    }

    CURLcode ret = curl_easy_perform(m_Curl);
    if (ret == CURLE_OK) {
        COMMLOG(OS_LOG_DEBUG, "Host can connect url : %s.", url.c_str());
        return true;
    }
    COMMLOG(OS_LOG_DEBUG, "Host can not connect url : %s, ret code: %d, err msg: %s.",
        url.c_str(), ret, curl_error_str);
    return false;
}

void CurlHttpResponse::SendRequest(const HttpRequest& req, const mp_uint32 time_out)
{
    COMMLOG(OS_LOG_DEBUG,
        "Method : %s, url : %s, special card : %s.",
        req.method.c_str(), req.url.c_str(), req.specialNetworkCard.c_str());
    char curl_error_str[CURL_ERROR_SIZE] = {0};
    FILE* fp = nullptr;
    if (SetCallBack(req, &fp) != MP_SUCCESS) {
        ERRLOG("Failed to set the callback.");
        return;
    }

    if (!req.notBindToDevice) {
        SetBindToDevice(req.specialNetworkCard);
    }

    // redirect
    curl_easy_setopt(m_Curl, CURLOPT_FOLLOWLOCATION, 1L);
    // set timeout parameters
    ConfigureCurlTimeOut(m_Curl, static_cast<int>(time_out));

    curl_easy_setopt(m_Curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(m_Curl, CURLOPT_ERRORBUFFER, curl_error_str);

    mp_int32 iRet = ConfigureTwoWayCertsAuthenticate(req);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Configure two-way certs authenticate info failed, stop send request.");
        return;
    }

    struct curl_slist* host = nullptr;
    if (!req.hostinfo.empty()) {
        host = curl_slist_append(NULL, req.hostinfo.c_str());
        curl_easy_setopt(m_Curl, CURLOPT_RESOLVE, host);
    }

    curl_easy_setopt(m_Curl, CURLOPT_URL, req.url.c_str());
    SetMethod(req.method);
    curl_slist* headers = SetHeaders(req.heads);
    if (req.url.find("https://") == 0 && (req.sslKey == "" && req.sslCert == "")) {
        SetCert(req.cert);  // one-way auth with CA certs
    }

    if (!req.body.empty()) {
        curl_easy_setopt(m_Curl, CURLOPT_POSTFIELDS, req.body.c_str());
    }

    m_ErrorCode = curl_easy_perform(m_Curl);
    if (m_ErrorCode != CURLE_OK) {
        COMMLOG(OS_LOG_ERROR, "Http send request failed. Error is %s.", curl_error_str);
    }

    CleanCurl(headers, host, fp);
}

mp_bool CurlHttpResponse::Success()
{
    return ((m_StatusCode == SC_OK) || (m_StatusCode == SC_CREATED)) && (m_ErrorCode == CURLE_OK);
}

mp_bool CurlHttpResponse::Busy(void)
{
    return (m_StatusCode == SC_SERVICE_UNAVAILABLE);
}

void CurlHttpResponse::SetMethod(const mp_string& method)
{
    mp_string str("GET");
    if (!method.empty()) {
        str = method;
    }
    (void)transform(str.begin(), str.end(), str.begin(), ::toupper);
    if (str == "GET") {
        curl_easy_setopt(m_Curl, CURLOPT_HTTPGET, 1L);
    } else if (str == "POST") {
        curl_easy_setopt(m_Curl, CURLOPT_POST, 1L);
    } else {
        curl_easy_setopt(m_Curl, CURLOPT_CUSTOMREQUEST, str.c_str());
    }
}

curl_slist* CurlHttpResponse::SetHeaders(const std::set<std::pair<mp_string, mp_string> >& heads)
{
    struct curl_slist* slist = NULL;

    for (set<std::pair<mp_string, mp_string> >::const_iterator it = heads.begin(); it != heads.end(); ++it) {
        mp_string heard_string = it->first + ": " + it->second;
        slist = curl_slist_append(slist, heard_string.c_str());
    }

    mp_string type = "Content-Type: application/json";
    slist = curl_slist_append(slist, type.c_str());
    curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, slist);
    return slist;
}

void CurlHttpResponse::SetCert(const mp_string& cert)
{
    m_cert = NONE_CERT_STRING;
    if (!cert.empty()) {
        m_cert = cert;
    }
#ifndef WIN32
    SSLHandle::EnrichSSL(m_Curl, m_cert);
#endif
}

void CurlHttpResponse::SetBindToDevice(const mp_string& networkCardName)
{
    if (networkCardName != "") {
        curl_easy_setopt(m_Curl, CURLOPT_INTERFACE, networkCardName.c_str());
        COMMLOG(OS_LOG_DEBUG, "Bind to device %s.", networkCardName.c_str());
        return;
    }

    if (!StaticConfig::IsInnerAgent()) {
        return;
    }

    mp_string deployType;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DEPLOY_TYPE, deployType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get deploy type failed.");
        return;
    }

    if (deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT ||
        deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT_NO_BRAND ||
        deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT_CYBER_ENGINE ||
        deployType == HOST_ENV_DEPLOYTYPE_E6000 ||
        deployType == HOST_ENV_DEPLOYTYPE_DATABACKUP) {
        return;
    }

    mp_string agentIp;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_AGENT_IP, agentIp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get agent ip failed.");
        return;
    }

    if (!agentIp.empty()) {
        curl_easy_setopt(m_Curl, CURLOPT_INTERFACE, agentIp.c_str());
        COMMLOG(OS_LOG_DEBUG, "Bind to device %s.", agentIp.c_str());
    } else {
        curl_easy_setopt(m_Curl, CURLOPT_INTERFACE, "vrf-srv");
        COMMLOG(OS_LOG_DEBUG, "Bind to device vrf-srv.");
    }
}

size_t CurlHttpResponse::GetDataCallback(void* ptr, size_t size, size_t counts, void* self)
{
    if ((ptr == NULL) || (self == NULL)) {
        COMMLOG(OS_LOG_ERROR, "ptr or self is null.");
        return 0;
    }

    CurlHttpResponse* pThis = static_cast<CurlHttpResponse*>(self);
    pThis->RecieveData(mp_string(reinterpret_cast<char*>(ptr), size * counts));
    return size * counts;
}

size_t CurlHttpResponse::GetFileStreamCallback(void* buffer, size_t size, size_t counts, void* userPtr)
{
    FILE* fp = static_cast<FILE*>(userPtr);
    if (fp == nullptr) {
        return 0;
    }
    return fwrite(buffer, size, counts, fp);
}

size_t CurlHttpResponse::GetHeaderCallback(void* ptr, size_t size, size_t counts, void* self)
{
    if ((ptr == NULL) || (self == NULL)) {
        COMMLOG(OS_LOG_ERROR, "ptr or self is null.");
        return 0;
    }
    CurlHttpResponse* pThis = static_cast<CurlHttpResponse*>(self);
    mp_string data(reinterpret_cast<char*>(ptr), (size * counts));

    if (data.find_first_of(":") == mp_string::npos) {
        pThis->ParseStatusLine(data);
    } else {
        pThis->RecieveHeader(data);
    }
    return size * counts;
}

void CurlHttpResponse::ParseStatusLine(const mp_string& status_line)
{
    vector<mp_string> strs;
    CMpString::StrSplit(strs, status_line, ' ');

    if (strs.size() > 1) {
        stringstream ss;
        mp_string strTrim = CMpString::Trim(strs[1].c_str());
        mp_string statusDescribe;
        statusDescribe.reserve(strs.size());

        CURL_STRING_TO_INTEGER(ss, strTrim, m_StatusCode);

        for (mp_uint32 ind = 2; ind < strs.size(); ++ind) {
            statusDescribe.append(" " + strs[ind]);
        }
        m_StatusDescribe = CMpString::Trim(statusDescribe.c_str());
    }
}

void CurlHttpResponse::RecieveHeader(const mp_string& header)
{
    std::vector<mp_string> strs;
    const mp_char headerLen = 2;
    CMpString::StrSplit(strs, header, ':');
    if (strs.size() == headerLen) {
        mp_string key = CMpString::Trim(strs[0].c_str());
        mp_string value = CMpString::Trim(strs[1].c_str());
        m_Headers[key].emplace(value);
    }
}

mp_uint32 CurlHttpResponse::GetHttpStatusCode()
{
    return m_StatusCode;
}

mp_string CurlHttpResponse::GetHttpStatusDescribe()
{
    return m_StatusDescribe;
}

int32_t CurlHttpResponse::GetErrCode()
{
    return m_ErrorCode;
}

mp_string CurlHttpResponse::GetErrString()
{
    mp_string strErr("Unknow error!");
    const char* errDes = curl_easy_strerror(static_cast<CURLcode>(m_ErrorCode));
    if (errDes != NULL) {
        strErr = errDes;
    }
    return strErr;
}

std::set<mp_string> CurlHttpResponse::GetHeadByName(const mp_string& header_name)
{
    std::map<mp_string, std::set<mp_string> >::iterator it = m_Headers.find(header_name);
    if (it != m_Headers.end()) {
        return it->second;
    }
    return std::set<mp_string>();
}

std::set<mp_string> CurlHttpResponse::GetCookies()
{
    return GetHeadByName("Set-Cookie");
}

void CurlHttpResponse::RecieveData(const mp_string& data)
{
    m_Body.append(data);
}

mp_string CurlHttpResponse::GetBody()
{
    return m_Body;
}

std::map<mp_string, std::set<mp_string> > CurlHttpResponse::GetHeaders()
{
    return m_Headers;
}

void CurlHttpResponse::ConfigureCurlTimeOut(CURL* curl, int iTimeOut)
{
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, iTimeOut);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, iTimeOut);
}

mp_int32 CurlHttpResponse::SetCallBack(const HttpRequest& req, FILE** fp)
{
    curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, &CurlHttpResponse::GetDataCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(m_Curl, CURLOPT_HEADERFUNCTION, &CurlHttpResponse::GetHeaderCallback);
    curl_easy_setopt(m_Curl, CURLOPT_WRITEHEADER, this);

    if (!req.fileName.empty()) {
        *fp = fopen(req.fileName.c_str(), "wb");
        if (*fp == nullptr) {
            ERRLOG("Failed to open the file.");
            return MP_FAILED;
        }
        curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, &CurlHttpResponse::GetFileStreamCallback);
        curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, *fp);
    }
    return MP_SUCCESS;
}

void CurlHttpResponse::CleanCurl(curl_slist* headers, struct curl_slist* host, FILE* fp)
{
    if (headers != nullptr) {
        curl_slist_free_all(headers);
    }

    if (host != nullptr) {
        curl_slist_free_all(host);
    }

    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

bool CurlHttpClient::TestConnectivity(const std::string& ip, const std::string& port,
    uint32_t testTimeout, const std::string& srcHost)
{
    std::shared_ptr<CurlHttpResponse> pRsp = std::make_shared<CurlHttpResponse>();
    if (pRsp && pRsp->Init()) {
        return pRsp->TestConnectivity(ip, port, testTimeout, srcHost);
    }
    COMMLOG(OS_LOG_ERROR, "Rsp init fail.");
    return false;
}

// need delete return value
IHttpResponse* CurlHttpClient::SendRequest(const HttpRequest& req, const mp_uint32 time_out)
{
    CurlHttpResponse* pRsp = new (std::nothrow) CurlHttpResponse();
    if (pRsp && pRsp->Init()) {
        pRsp->SendRequest(req, time_out);
    } else {
        return NULL;
    }
    return pRsp;
}

std::shared_ptr<IHttpClient> IHttpClient::CreateNewClient()
{
    return std::make_shared<CurlHttpClient>();
}

IHttpClient* IHttpClient::CreateClient()
{
    IHttpClient* pClient = new (std::nothrow) CurlHttpClient();
    return pClient;
}

IHttpClient* IHttpClient::GetInstance()
{
    IHttpClient* pHttpClient = new (std::nothrow) CurlHttpClient();
    if (pHttpClient == NULL) {
        COMMLOG(OS_LOG_ERROR, "Get CurlHttpClient instance failed.");
        return NULL;
    }
    return pHttpClient;
}

void IHttpClient::ReleaseInstance(IHttpClient* pClient)
{
    if (pClient != NULL) {
        delete pClient;
        pClient = NULL;
    }
}

mp_int32 IHttpClient::InitStructHttpRequest(HttpRequest& req)
{
    req.method = "";  // GET,POST,PUT,HEAD,DELETE,......
    req.url = "";
    req.caInfo = "";  // CA certificate
    req.sslKey = "";  // CURLOPT_SSLKEY, client private key file name
    req.sslCert = ""; // CURLOPT_SSLCERT, client cert file name
    req.cert = "";
    req.heads.clear(); // HTTP heads
    req.body = "";   // http content body
    req.specialNetworkCard = "";
    req.hostinfo = "";
    req.domaininfo = "";
    req.fileName = "";

    return MP_SUCCESS;
}

CurlHttpClient::~CurlHttpClient()
{}

CurlHttpClient::CurlHttpClient()
{}

mp_bool CurlHttpClient::TestConnect(const mp_string& url, const mp_uint32 time_out)
{
    COMMLOG(OS_LOG_DEBUG, "url : %s, timeout : %u.", url.c_str(), time_out);

    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        COMMLOG(OS_LOG_DEBUG, "Test net connectiong is failed. url : %s.", url.c_str());
        return false;
    }

    char curl_error_str[CURL_ERROR_SIZE] = {0};
    // set timeout parameters
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, static_cast<int>(time_out));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<int>(time_out));

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_error_str);

    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    if (url.find("https") == 0) {
        mp_string certText(NONE_CERT_STRING);
#ifndef WIN32
        SSLHandle::EnrichSSL(curl, certText);
#endif
    }

    mp_uint32 errorCode = curl_easy_perform(curl);
    if (CURLE_OK != errorCode) {
        COMMLOG(OS_LOG_ERROR, "Http send request failed. Error is %s", curl_error_str);
    }

    curl_easy_cleanup(curl);
    curl = NULL;

    return CURLE_OK == errorCode;
}

static size_t WriteData(char* d, size_t n, size_t l, void* p)
{
    static_cast<void>(d);
    static_cast<void>(p);
    return n * l;
}

mp_string CurlHttpClient::GetCertificate(const mp_string& url, const mp_uint32 time_out)
{
    CURLcode res;
    char curl_error_str[CURL_ERROR_SIZE] = {0};
    string certStr = "";
    union {
        struct curl_slist* to_info;
        struct curl_certinfo* to_certinfo;
    } ptr;

    // Update by wrz
    COMMLOG(OS_LOG_DEBUG, "url : %s, timeout : %d.", url.c_str(), time_out);

    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        COMMLOG(OS_LOG_ERROR, "Test net connectiong is failed. url : %s.", url.c_str());
        return "";
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // set timeout parameters
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, static_cast<int>(time_out));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<int>(time_out));
    // setup curl verbose
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_error_str);
    // no print to screen
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
    // set no verify
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        COMMLOG(OS_LOG_ERROR, "curl easy perform return %d, error : %s.", res, curl_error_str);
        curl_easy_cleanup(curl);
        return "";
    }

    ptr.to_info = NULL;
    res = curl_easy_getinfo(curl, CURLINFO_CERTINFO, &ptr.to_info);
    if (res != CURLE_OK) {
        COMMLOG(OS_LOG_ERROR, "curl_easy_getinfo return %d.", res);
        curl_easy_cleanup(curl);
        return "";
    }

    COMMLOG(OS_LOG_DEBUG, "certs num is %u.", ptr.to_certinfo->num_of_certs);
    if (ptr.to_info && ptr.to_certinfo->num_of_certs != 0) {
        struct curl_slist* _slist = ptr.to_certinfo->certinfo[0];
        for (; _slist; _slist = _slist->next) {
            certStr += "\n" + mp_string(_slist->data);
        }
        curl_slist_free_all(_slist);
    }
    curl_easy_cleanup(curl);
    return certStr;
}

mp_int32 CurlHttpClient::GetThumbPrint(const mp_string& url, mp_string& thumbPrint, const mp_uint32 time_out)
{
    return GetGeneralThumbprint(url, thumbPrint, "SHA-1", time_out);
}

mp_int32 CurlHttpClient::GetGeneralThumbprint(const mp_string& url, mp_string& thumbPrint,
    const mp_string& algorithm, const mp_uint32 time_out)
{
    mp_string certStr = GetCertificate(url, time_out);
    if (certStr.empty()) {
        COMMLOG(OS_LOG_ERROR, "cert string is empty.");
        return -1;
    }

    mp_string startStr = "Cert:";
    string::size_type positionStart = certStr.find(startStr.c_str()) + startStr.size();
    mp_string endStr = "-----END CERTIFICATE-----";
    string::size_type positionEnd = certStr.find(endStr.c_str()) + endStr.size();
    if ((positionStart == string::npos) || (positionEnd == string::npos) || (positionEnd <= positionStart)) {
        COMMLOG(OS_LOG_ERROR, "find cert string failed.");
        return -1;
    }
#ifndef WIN32  // now window do not support ssl, remain next plan to do
    certStr = certStr.substr(positionStart, positionEnd - positionStart);
    SecuritySS_RC sRet = SSLHandle::GetGeneralThumbprint(certStr, thumbPrint, algorithm);
    if (sRet != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "find thumbPrint string failed.");
        return -1;
    }
#endif
    return 0;
}

bool SendHttpRequest(const HttpRequest &req, Json::Value &rsp, uint32_t timeout, bool ifRetry)
{
    uint32_t retryTime = 0;
    uint32_t retryTimes = ifRetry ? NAME_NOT_RESOLV_RETRY_COUNT : 0;

    if (!DoSendHttpRequest(retryTime, retryTimes, req, rsp, timeout)) {
        COMMLOG(OS_LOG_ERROR, "The retryTime exceeds the NAME_NOT_RESOLV_RETRY_COUNT(%d).",
            NAME_NOT_RESOLV_RETRY_COUNT);
        return false;
    }

    return true;
}

bool DoSendHttpRequest(uint32_t retryTime, uint32_t retryTimes,
    const HttpRequest &req, Json::Value &rsp, uint32_t timeout)
{
    do {
        IHttpResponse *httpRsp = nullptr;
        IHttpClient *pHttpClient = IHttpClient::GetInstance();
        if (pHttpClient == nullptr) {
            COMMLOG(OS_LOG_ERROR, "Get instance prt is empty. ");
            return false;
        }
        Defer _(nullptr, [&](...) {
            if (pHttpClient) {
                delete pHttpClient;
            }
            if (httpRsp) {
                delete httpRsp;
            }
        });

        httpRsp = pHttpClient->SendRequest(req, timeout);
        if (httpRsp == nullptr) {
            COMMLOG(OS_LOG_ERROR, "Return response is empty. ");
            return false;
        }

        std::string errorDes;
        if (!httpRsp->Success()) {
            if (httpRsp->GetErrCode() == 0) {
                errorDes = httpRsp->GetHttpStatusDescribe();
                COMMLOG(OS_LOG_ERROR, "StatusCode: %d, error: %s", httpRsp->GetHttpStatusCode(), errorDes.c_str());
            } else {
                errorDes = httpRsp->GetErrString();
                COMMLOG(OS_LOG_ERROR, "StatusCode: %d, error: %s", httpRsp->GetHttpStatusCode(), errorDes.c_str());
            }
            continue;
        }

        if (httpRsp->GetHttpStatusCode() != SC_OK) {
            COMMLOG(OS_LOG_ERROR, "HttpStatus: %d, retryTime: %d", httpRsp->GetHttpStatusCode(), retryTime);
#ifdef WIN32
            Sleep(HTTPS_ABNORMAL_RETRY_INTERVAL * THOUSAND);
#else
            sleep(HTTPS_ABNORMAL_RETRY_INTERVAL);
#endif
            continue;
        }

        if (!JsonHelper::JsonStringToJsonValue(httpRsp->GetBody(), rsp)) {
            COMMLOG(OS_LOG_ERROR, "Parse json failed!");
            return false;
        }

        return true;
    } while (++retryTime < retryTimes);

    return false;
}