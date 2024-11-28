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
#ifndef CURL_HTTP_CLIENT_INTERFACE_H
#define CURL_HTTP_CLIENT_INTERFACE_H

#include <map>
#include <curl/curl.h>
#include "curl_http/HttpClientInterface.h"
#include "common/JsonHelper.h"

namespace Module {
const std::string INTERNAL_CA_CERT_FILE = "/opt/logpath/infrastructure/cert/internal/ca/ca.crt.pem";
const uint64_t HTTPS_ABNORMAL_RETRY_INTERVAL = 60;
const uint64_t NAME_NOT_RESOLV_RETRY_COUNT = 3;
const std::string STORE_FILE_PATH = "/opt/logpath/protectmanager/kmc/master.ks";
const std::string STORE_BAK_FILE_PATH = "/kmc_conf/..data/backup.ks";

namespace CurlCertCommon {
// 内存证书结构体
struct MemberCertInfo {
    std::string m_strClusterCA;
    std::string m_strClusterCrl;
    std::string m_strClientCrt;
    std::string m_strClientKey;
};

static CURLcode SSLTwoWayAuthenticationCallbackFunc(CURL* curl, void* sslctx, void* parm);

static CURLcode SSLTAuthenticationCA(void* &sslctx, const std::string& strClusterCA);

static CURLcode SSLTAuthenticationCrl(void* &sslctx, const std::string& strClusterCrl);

static CURLcode SSLClientCert(void* &sslctx, const std::string& strClientCrt);

static CURLcode SSLClientKey(void* &sslctx, const std::string& strClientKey);
}

class CurlHttpResponse : public IHttpResponse {
public:
    CurlHttpResponse();

    virtual ~CurlHttpResponse();

    virtual bool Success();

    virtual bool Busy(void);

    virtual uint32_t  GetHttpStatusCode();

    virtual std::string GetHttpStatusDescribe();

    virtual int32_t GetErrCode();

    virtual std::string GetErrString();

    virtual std::set<std::string> GetHeadByName(const std::string& header_name);

    virtual std::string GetBody();

    virtual std::map<std::string, std::set<std::string> > GetHeaders();

    virtual std::set<std::string> GetCookies();

    virtual uint32_t GetStatusCode();

private:
    friend class CurlHttpClient;
    CurlHttpResponse(const CurlHttpResponse& src);
    CurlHttpResponse& operator=(const CurlHttpResponse&);

    void SetMethod(const std::string& method);

    curl_slist* SetHeaders(const std::set<std::pair<std::string, std::string> >& heads);

    void SetCert(const std::string& cert);

    void SetCert(int verifyCert, const std::string& cert);

    void SetTwoWayAuthentication(const HttpRequest& req);

    void SendTwoWayCertRequest(const HttpRequest& req, const uint32_t timeOut);

    void SendRequest(const HttpRequest& req, const uint32_t timeOut );

    bool PerformUpload(const std::string& attachmentPath);

    void DownloadAttchment(const HttpRequest& req, const uint32_t timeOut);

    bool UploadAttachment(const HttpRequest& req, const uint32_t timeOut);

    static size_t DownloadAttchmentCallback(void* ptr, size_t size, size_t count, void* self);

    static size_t GetDataCallback(void* ptr, size_t size, size_t count, void* self);

    static size_t GetHeaderCallback(void* ptr, size_t size, size_t count, void* self);

    void RecieveHeader(const std::string& header);

    void ParseStatusLine(const std::string& status_line);

    void RecieveData(const std::string& data);

    void SetCertParam(const HttpRequest& req);

    void SetTimeOut(CURL* curlPtr, const uint32_t timeOut);

    const CURL* GetPtr();

    void CleanCertInfo();
private:
    uint32_t m_StatusCode;
    std::string m_StatusDescribe;
    uint32_t m_ErrorCode;
    std::string m_Body;
    std::map<std::string, std::set<std::string> > m_Headers;
    CURL* m_Curl = nullptr;
    std::string m_cert;
    const uint64_t NAME_NOT_RESOLV_RETRY_COUNT = 3;
    const uint64_t NAME_NOT_RESOLV_RETRY_INTERVAL = 1;
    CurlCertCommon::MemberCertInfo m_stMemCertInfo;
};

class CurlHttpClient : public IHttpClient {
public:
    CurlHttpClient();

    virtual ~CurlHttpClient();

    virtual std::shared_ptr<IHttpResponse> SendMemCertRequest(const HttpRequest& req, const uint32_t timeOut = 90);

    virtual std::shared_ptr<IHttpResponse> SendRequest(const HttpRequest& req, const uint32_t timeOut = 90);

    virtual std::shared_ptr<IHttpResponse> DownloadAttchment(const HttpRequest& req, const uint32_t timeOut = 90);

    virtual bool UploadAttachment(const HttpRequest& req, std::shared_ptr<IHttpResponse>& rsp,
        const uint32_t timeOut = 90);

    virtual bool TestConnect(const std::string& url, const uint32_t timeOut = 90);

    virtual uint32_t GetThunmPrint(const std::string& url, std::string& thunmPrint, const uint32_t timeOut = 90);

    virtual std::string GetCertificate(const std::string& url, const uint32_t timeOut = 90);
private:
    std::string FormatUrl(const std::string& fullUrl);
};

class CurlKmcManagerInterface {
public:
    CurlKmcManagerInterface();
    ~CurlKmcManagerInterface();
    bool InitKmc(const std::string& kmcFile = STORE_FILE_PATH, const std::string& kmcBackupFile = STORE_BAK_FILE_PATH);
    void UnInitKmc();
    int32_t Decrypt(std::string &plainText, const std::string &cipherText);
    int32_t DecryptV1(std::string &plainText, const std::string &cipherText);
    bool m_isInited = false;
};

bool SendHttpRequest(const HttpRequest &req, Json::Value &rsp, uint32_t timeout = 120, bool ifRetry = true,
    bool ifCheckBody = true);

namespace {
    static std::unique_ptr<CurlKmcManagerInterface> g_kmcInstance = std::make_unique<CurlKmcManagerInterface>();
    static bool InitKmcInstance()
    {
        if (g_kmcInstance.get() == nullptr) {
            HCP_Logger_noid(ERR, "HttpClient") << "g_kmcInstance is null." << HCPENDLOG;
            return false;
        }
        if (!g_kmcInstance->InitKmc()) {
            HCP_Logger_noid(ERR, "HttpClient") << "Init kmc failed." << HCPENDLOG;
            return false;
        }
        return true;
    }
}
}
#endif // CURL_HTTP_CLIENT_INTERFACE_H