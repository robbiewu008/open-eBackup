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
#include "curl/curl.h"
#include "message/rest/http_status.h"
#include "message/curlclient/HttpClientInterface.h"
#include "common/Defines.h"
#include "common/JsonHelper.h"

const mp_string NONE_CERT_STRING = "none";
const mp_int32 NOT_SET_ALL_TWO_WAY_CERTS = -2;

class CurlHttpResponse : public IHttpResponse {
public:
    CurlHttpResponse();
    mp_bool Init();

    virtual ~CurlHttpResponse();

    virtual mp_bool Success();

    virtual mp_bool Busy(void);

    virtual mp_uint32 GetHttpStatusCode();

    virtual mp_string GetHttpStatusDescribe();

    virtual int32_t GetErrCode();

    virtual mp_string GetErrString();

    virtual std::set<mp_string> GetHeadByName(const mp_string& header_name);

    virtual mp_string GetBody();

    virtual std::map<mp_string, std::set<mp_string> > GetHeaders();

    virtual std::set<mp_string> GetCookies();

    virtual bool TestConnectivity(const std::string& ip, const std::string& port);

private:
    friend class CurlHttpClient;
    CurlHttpResponse(const CurlHttpResponse& src);
    CurlHttpResponse& operator=(const CurlHttpResponse&);

    EXTER_ATTACK mp_int32 ConfigureTwoWayCertsAuthenticate(const HttpRequest& req);

    bool IsInnerRequestUri(const HttpRequest& req);

    mp_int32 SetCertInCurl(const HttpRequest& req);

    mp_int32 CheckCRLStatus();

    void SetMethod(const mp_string& method);

    curl_slist* SetHeaders(const std::set<std::pair<mp_string, mp_string> >& heads);

    void SetCert(const mp_string& cert);

    void SendRequest(const HttpRequest& req, const mp_uint32 time_out);

    static size_t GetDataCallback(void* ptr, size_t size, size_t counts, void* self);

    static size_t GetFileStreamCallback(void* buffer, size_t size, size_t counts, void* userPtr);

    static size_t GetHeaderCallback(void* ptr, size_t size, size_t counts, void* self);

    void RecieveHeader(const mp_string& header);

    void ParseStatusLine(const mp_string& status_line);

    void RecieveData(const mp_string& data);

    void ConfigureCurlTimeOut(CURL* curl, int iTimeOut);

    mp_int32 SetCallBack(const HttpRequest& req, FILE** fp);

    void CleanCurl(curl_slist* headers, struct curl_slist* host, FILE* fp);

    void SetBindToDevice(const mp_string& networkCardName);

private:
    mp_uint32 m_StatusCode;
    mp_string m_StatusDescribe;
    mp_uint32 m_ErrorCode;
    mp_string m_Body;
    std::map<mp_string, std::set<mp_string> > m_Headers;
    CURL* m_Curl;
    mp_string m_cert;
};

class CurlHttpClient : public IHttpClient {
public:
    CurlHttpClient();

    virtual ~CurlHttpClient();

    virtual IHttpResponse* SendRequest(const HttpRequest& req, const mp_uint32 time_out = 90);

    virtual mp_bool TestConnect(const mp_string& url, const mp_uint32 time_out = 90);

    virtual bool TestConnectivity(const std::string& ip, const std::string& port);

    virtual mp_int32 GetThumbPrint(const mp_string& url, mp_string& thunmPrint, const mp_uint32 time_out = 90);

    mp_int32 GetGeneralThumbprint(const mp_string& url, mp_string& thunmPrint, const mp_string& algorithm,
    const mp_uint32 time_out = 90);

    virtual mp_string GetCertificate(const mp_string& url, const mp_uint32 time_out = 90);
};

bool SendHttpRequest(const HttpRequest &req, Json::Value &rsp, uint32_t timeout = 120, bool ifRetry = true);
bool DoSendHttpRequest(uint32_t retryTime, uint32_t retryTimes,
    const HttpRequest &req, Json::Value &rsp, uint32_t timeout);

#endif  // CURL_HTTP_CLIENT_INTERFACE_H
