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
#include "common/CleanMemPwd.h"

namespace Module {
enum CACertVerification {
    DO_NOT_VERIFY = 0,
    AGENT_VERIFY,
    VCENTER_VERIFY,
    INTERNAL_VERIFY
};
class IHttpResponse {
public:
    IHttpResponse() {}
    virtual ~IHttpResponse() {}

    virtual bool Success() = 0;

    virtual bool Busy(void) = 0;

    virtual uint32_t  GetHttpStatusCode() = 0;

    virtual std::string GetHttpStatusDescribe()  = 0;

    virtual int32_t GetErrCode() = 0;

    virtual std::string GetErrString() = 0;

    virtual std::set<std::string> GetHeadByName(const std::string& header_name) = 0;

    virtual std::string GetBody() = 0;

    virtual std::map<std::string, std::set<std::string> > GetHeaders() = 0;

    virtual std::set<std::string> GetCookies() = 0;

    virtual uint32_t GetStatusCode() = 0;
};

struct HttpRequest
{
    std::string method; // GET,POST,PUT,HEAD,DELETE,......
    std::string url;
    std::string certCA;
    std::string clientCert;
    std::string clientKey;
    std::string tmpUrl; // save temp value of incompletes url
    std::string vmPathInfo1; //  summary.config.vmPathName
    std::string vmPathInfo0; // datastore name of nvram file
    std::string cert;   // CA certificate
    std::string revocationList;  // CA revocation list
    std::set<std::pair<std::string, std::string> > heads; // HTTP heads
    std::string body; // http content body
    std::string specialNetworkCard;
    int isVerify = DO_NOT_VERIFY;
    std::string localFilePath;
    std::string domainInfo;
    std::string vSphereLoginInfo;
    bool ifCheckDomain = true; // CA certificate, default by domain
    bool ifConfirmbyCACert = false; // download/upload file and confirm by CA cert, default not
    bool enableProxy = false; // mgmt network forward proxy

    ~HttpRequest() {
        FreeContainer(certCA);
        FreeContainer(clientCert);
        FreeContainer(clientKey);
        FreeContainer(cert);
        FreeContainer(revocationList);
        CleanMemoryPwd(vSphereLoginInfo);
        heads.clear();
    }
};

class IHttpClient {
public:
    IHttpClient() {}
    virtual ~IHttpClient() {}

    static std::shared_ptr<IHttpClient> CreateClient();

    static IHttpClient* GetInstance();

    static void ReleaseInstance(IHttpClient* pClient);

    virtual std::shared_ptr<IHttpResponse> SendMemCertRequest(const HttpRequest& req, const uint32_t timeOut = 90)
    {
        return nullptr;
    }

    virtual std::shared_ptr<IHttpResponse> SendRequest(const HttpRequest& req, const uint32_t timeOut = 90) = 0;

    virtual std::shared_ptr<IHttpResponse> DownloadAttchment(const HttpRequest& req, const uint32_t timeOut = 90) = 0;

    virtual bool UploadAttachment(const HttpRequest& req, std::shared_ptr<IHttpResponse>& rsp,
        const uint32_t timeOut = 90) = 0;

    virtual bool TestConnect(const std::string& url, const uint32_t timeOut = 90) = 0;

    virtual uint32_t GetThunmPrint(const std::string& url, std::string& thunmPrint, const uint32_t timeOut = 90) = 0;

    virtual std::string GetCertificate(const std::string& url, const uint32_t timeOut = 90) = 0;
};
}

#endif // HTTP_CLIENT_INTERFACE_H
