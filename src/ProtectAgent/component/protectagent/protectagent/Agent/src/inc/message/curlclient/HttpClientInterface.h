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
#ifndef HTTP_CLIENT_INTERFACE_H
#define HTTP_CLIENT_INTERFACE_H
#include <stdint.h>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <map>
#include "common/Defines.h"

class IHttpResponse {
public:
    IHttpResponse() {}
    virtual ~IHttpResponse() {}

    virtual mp_bool Success() = 0;

    virtual mp_bool Busy(void) = 0;

    virtual mp_uint32  GetHttpStatusCode() = 0;

    virtual mp_string GetHttpStatusDescribe() = 0;

    virtual int32_t GetErrCode() = 0;

    virtual mp_string GetErrString() = 0;

    virtual std::set<mp_string> GetHeadByName(const mp_string& header_name) = 0;

    virtual mp_string GetBody() = 0;

    virtual std::map<mp_string, std::set<mp_string> > GetHeaders() = 0;

    virtual std::set<mp_string> GetCookies() = 0;
};

struct HttpRequest {
    mp_string method;  // GET,POST,PUT,HEAD,DELETE,......
    mp_string url;
    mp_string caInfo;  // CA certificate
    mp_string sslKey;  // CURLOPT_SSLKEY, client private key file name
    mp_string sslCert; // CURLOPT_SSLCERT, client cert file name
    mp_string cert;
    std::set<std::pair<mp_string, mp_string> > heads; // HTTP heads
    mp_string body;   // http content body
    mp_string specialNetworkCard;
    mp_string hostinfo;
    mp_string domaininfo;
    mp_uint32 nTimeOut = HTTP_TIME_OUT;
    mp_string fileName;
    bool notBindToDevice {false};     // default: attempt to bind to specialNetworkCard
};

struct HttpResponse {
    mp_string version;
    mp_uint32 statusCode {0};
    mp_uint32 curlErrCode {0};
    mp_string statusString;
    mp_string body;
    std::set<std::pair<mp_string, mp_string> > heads;
};

class IHttpClient {
public:
    IHttpClient() {}
    virtual ~IHttpClient() {}

    static std::shared_ptr<IHttpClient> CreateNewClient();

    static IHttpClient* CreateClient();

    static IHttpClient* GetInstance();

    static void ReleaseInstance(IHttpClient* pClient);

    static mp_int32 InitStructHttpRequest(HttpRequest& req);

    virtual IHttpResponse* SendRequest(const HttpRequest& req, const mp_uint32 time_out = 90) = 0;

    virtual mp_bool TestConnect(const mp_string& url, const mp_uint32 time_out = 90) = 0;

    virtual bool TestConnectivity(const std::string& ip, const std::string& port,
        uint32_t testTimeout = 0, const std::string& srcHost = "") = 0;

    virtual mp_int32 GetThumbPrint(const mp_string& url, mp_string& thunmPrint, const mp_uint32 time_out = 90) = 0;

    virtual mp_string GetCertificate(const mp_string& url, const mp_uint32 time_out = 90) = 0;
};

#endif // HTTP_CLIENT_INTERFACE_H
