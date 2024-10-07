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
#ifndef _MS_REST_CLIENT_H_
#define _MS_REST_CLIENT_H_
#include "json/json.h"
#include <map>
#include "common/Thread.h"

// URL Method
#define REST_URL_METHOD_GET                       "GET"
#define REST_URL_METHOD_PUT                       "PUT"
#define REST_URL_METHOD_POST                      "POST"
#define REST_URL_METHOD_DELETE                    "DELETE"
#define REST_URL_METHOD_PATCH                     "PATCH"
#define HTTP_HEAD_SESSION_COOKIE                       "Cookie"
#define HTTP_HEAD_SET_COOKIE                           "Set-Cookie"
#define HTTP_HEAD_IP_ROUTE                             "IP-Route"

namespace Module {
    class MSRestApi;
    static const char* HTTP_HEAD_AUTH_TOKEN             = "X-Auth-Token";
    static const char* HTTP_HEAD_X_FORWARDED_FOR        = "X-FORWARDED-FOR";
    static const char* HTTP_HEAD_SUBJECT_TOKEN_UPPER    = "HTTP_X_SUBJECT_TOKEN";
    static const char* HTTP_HEAD_CLIENT_IP_UPPER        = "HTTP_X_CLIENT_IP";
    static const char* HTTP_HEAD_AUTH_TOKEN_UPPER       = "HTTP_X_AUTH_TOKEN";
    static const char* HTTP_HEAD_SUBJECT_TOKEN          = "X-Subject-Token";
    static const char* HTTP_HEAD_CLIENT_IP              = "X-Client-IP";
    const std::string g_headerAuthorization             = "Authorization";


    struct MSRestRequest {
        MSRestRequest() {
            retry = true;
            timeout = 0;
            isVerify = 0;
        }

        MSRestRequest(std::string IAMUrlValue, std::string userNameValue, std::string passwordValue,
                      std::string httpMethodValue, std::string serviceUrlValue, std::string certValue) {
            IAMUrl = IAMUrlValue;
            userName = userNameValue;
            password = passwordValue;
            httpMethod = httpMethodValue;
            serviceUrl = serviceUrlValue;
            cert = certValue;
            retry = true;
            timeout = 0;
            isVerify = 0;
        }

        MSRestRequest(std::string IAMUrlValue, std::string userNameValue, std::string passwordValue,
                      std::string httpMethodValue, std::string serviceUrlValue, std::string certValue,
                      Json::Value body) {
            IAMUrl = IAMUrlValue;
            userName = userNameValue;
            password = passwordValue;
            httpMethod = httpMethodValue;
            serviceUrl = serviceUrlValue;
            cert = certValue;
            reqBody = body;
            retry = true;
            timeout = 0;
            isVerify = 0;
        }

        std::string IAMUrl;
        std::string userName;
        std::string password;
        std::string cert;
        std::string serviceUrl;
        std::string httpMethod;
        std::string cookie;
        std::string clientIP;
        std::string ip_route;
        Json::Value reqBody;
        std::string attachmentPath;
        bool retry;
        uint32_t timeout;
        std::string k8sToken;
        int isVerify;
    };

    class MSRestCLient {
    public:

        virtual int SendReqeust(
                const MSRestRequest &req,
                Json::Value &rsp,
                std::string &errDesc);

        virtual int DoSendReqeust(
                const MSRestRequest &req,
                Json::Value &rsp,
                std::string &errDesc);

        virtual int SendReqeust(
                const MSRestRequest &req,
                std::string s_tokenID,
                std::string *c_tokenID,
                Json::Value &rsp,
                std::string &errDesc,
                int &errorCode);

        virtual int DownloadAttchment(
                const MSRestRequest &req,
                std::string &errDesc, bool firstTime = true);

        virtual int UploadAttachment(
                const MSRestRequest &req,
                Json::Value &rsp,
                std::string &errDesc, bool firstTime = true);

//        static void BuildBaseRequest(MSRestRequest &req, const std::string method, const std::string api,
//                                     const std::string params = "", const std::string cert = "");

        int LoginRequest(
                const MSRestRequest &req, std::string *tokenID, Json::Value &rsp, std::string &errDesc, int &errorCode);

        int LogoutRequest(
                const MSRestRequest &req, const std::string &tokenID, Json::Value &rsp, std::string &errDesc,
                int &errorCode);

        int GetAccessIPFromIAM(const MSRestRequest &loginReq);

        static MSRestCLient &Instance();

        static MSRestCLient &OutConnectInstance();

    private:
        int GetRestApi(std::string &key, MSRestApi *&restApi);

        MSRestCLient(const MSRestCLient &);

        const MSRestCLient &operator=(const MSRestCLient &);

        MSRestCLient();

        virtual ~MSRestCLient();

        const std::string Token() {
            CThreadAutoLock tlock(&m_lockLoginToken);
            return m_token;
        }

        void Token(const std::string inValue) {
            CThreadAutoLock tlock(&m_lockLoginToken);
            m_token = inValue;
        }

        int Login(MSRestApi &restApi, const MSRestRequest &req, std::string &errDesc);

    private:
        std::map<std::string, MSRestApi *> m_restApis;
        thread_lock_t m_lockRestApi;
        std::string m_token;    //Cache token
        thread_lock_t m_lockLoginToken;
        std::string m_iamAccessIP;
    };
}


#endif//_MS_REST_CLIENT_H_
