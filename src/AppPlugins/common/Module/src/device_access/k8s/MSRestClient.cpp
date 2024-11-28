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
#include "device_access/k8s/MSRestClient.h"
#include "device_access/k8s/MSRestApi.h"
#include "log/Log.h"
#include "curl_http/CurlHttpClient.h"
#include "config_reader/ConfigIniReader.h"

namespace Module {
    const std::string MODULE_NAME = "MSRestCLient";

    MSRestCLient::MSRestCLient()
    {
        CMpThread::InitLock(&m_lockRestApi);
        CMpThread::InitLock(&m_lockLoginToken);
    }

    MSRestCLient::~MSRestCLient()
    {
        CMpThread::DestroyLock(&m_lockRestApi);
        CMpThread::DestroyLock(&m_lockLoginToken);
    }

    int MSRestCLient::Login(MSRestApi &restApi, const MSRestRequest &req, std::string &errDesc)
    {
        CThreadAutoLock tlock(&m_lockRestApi);
        int ret = restApi.Login(req.retry,
                                req.IAMUrl,
                                req.userName,
                                req.password,
                                req.cert,
                                errDesc);
        if (RETURN_OK != ret) {
            HCP_Log(ERR, MODULE_NAME) << "Login failed. ret: " << ret << HCPENDLOG;
            Token("");  // clear cache token
            return ret;
        }
        // Cache token
        if (restApi.Token().empty()) {
            HCP_Log(ERR, MODULE_NAME) << "Login failed, token is empty." << HCPENDLOG;
            return RETURN_ERROR;
        }
        Token(restApi.Token());
        HCP_Log(INFO, MODULE_NAME) << "Login IAM successful. ret: " << ret << HCPENDLOG;

        return RETURN_OK;
    }

    int MSRestCLient::LogoutRequest(const MSRestRequest &req, const std::string &tokenID, Json::Value &rsp,
        std::string &errDesc, int &errorCode)
    {
        MSRestApi restApi;
        Json::Value jsonBody;
        jsonBody["Id"] = req.userName;

        HttpRequest httpRequest;
        httpRequest.method = REST_URL_METHOD_DELETE;
        httpRequest.url = ConfigReader::getString("MicroService", "LoadbalancerAddress") + "/v3/auth/tokens";
        httpRequest.cert = req.cert;
        if (!jsonBody.empty()) {
            Json::FastWriter jsonWriter;
            httpRequest.body = jsonWriter.write(jsonBody);
        }
        httpRequest.heads.insert(std::make_pair(HTTP_HEAD_AUTH_TOKEN, tokenID));
        return restApi.SendRequest(httpRequest, rsp, errDesc, NULL, errorCode);
    }

    int MSRestCLient::LoginRequest(const MSRestRequest &req, std::string *tokenID, Json::Value &rsp,
        std::string &errDesc, int &errorCode)
    {
        HttpRequest httpRequest;
        httpRequest.method = req.httpMethod;
        httpRequest.url = req.serviceUrl;
        httpRequest.cert = req.cert;
        if (!req.reqBody.empty()) {
            Json::FastWriter jsonWriter;
            httpRequest.body = jsonWriter.write(req.reqBody);
        }

        if (m_iamAccessIP.empty()) {
            int ret = GetAccessIPFromIAM(req);
            if (ret != RETURN_OK) {
                return ret;
            }
        }
        httpRequest.heads.insert(std::make_pair(HTTP_HEAD_X_FORWARDED_FOR, m_iamAccessIP));
        MSRestApi restApi;
        return restApi.SendRequest(httpRequest, rsp, errDesc, tokenID, errorCode);
    }

    int MSRestCLient::GetAccessIPFromIAM(const MSRestRequest &loginReq)
    {
        HttpRequest req;
        req.method = "GET";
        req.url = ConfigReader::getString("MicroService", "LoadbalanceAddress") + "/v3/auth_inner/get_access_ip";
        req.cert = loginReq.cert;

        Json::Value data;
        std::string errDesc;
        int errorCode = 0;
        MSRestApi restApi;
        int iRet = restApi.SendRequest(req, data, errDesc, nullptr, errorCode);
        if (iRet == RETURN_OK) {
            m_iamAccessIP = data["access_ip"].asString();
        } else {
            HCP_Logger_noid(ERR, MODULE_NAME)
                    << "Get access ip from iam failed. ret is " << iRet
                    << ", error is " << WIPE_SENSITIVE(errDesc) << HCPENDLOG;
        }

        return iRet;
    }

    int MSRestCLient::SendReqeust(const MSRestRequest &req, Json::Value &rsp, std::string &errDesc)
    {
        MSRestApi restApi;
        int ret = RETURN_OK;
        // Send request
        ret = DoSendReqeust(req, rsp, errDesc);
        if (RETURN_OK != ret) {
            HCP_Log(ERR, MODULE_NAME) << "Send failed, ret: " << ret << HCPENDLOG;
            return ret;
        }
        HCP_Log(DEBUG, MODULE_NAME) << "Send succ,url:" << WIPE_SENSITIVE(req.serviceUrl) << HCPENDLOG;
        return RETURN_OK;
    }

// Send request directly without login.
    int MSRestCLient::DoSendReqeust(const MSRestRequest &req, Json::Value &rsp, std::string &errDesc)
    {
        // [Step.1]. Build request
        HttpRequest httpRequest;
        httpRequest.method = req.httpMethod;
        httpRequest.url = req.serviceUrl;
        httpRequest.cert = req.cert;
        httpRequest.isVerify = req.isVerify;
        if (!req.k8sToken.empty()) {
            httpRequest.heads.insert(std::make_pair(g_headerAuthorization, req.k8sToken));
        }

        if (!req.clientIP.empty()) {
            httpRequest.heads.insert(std::make_pair(HTTP_HEAD_CLIENT_IP, req.clientIP));
        }

        if (!req.reqBody.empty()) {
            Json::FastWriter jsonWriter;
            httpRequest.body = jsonWriter.write(req.reqBody);
        }

        // [Step.2]. Send request
        MSRestApi restApi;
        int ret = restApi.SendRequestDirectly(httpRequest, rsp, errDesc, req.retry);
        if (RETURN_OK != ret) {
            HCP_Log(ERR, MODULE_NAME) << "Send failed. ret: " << ret << HCPENDLOG;
            return ret;
        }

        return RETURN_OK;
    }

    int MSRestCLient::SendReqeust(const MSRestRequest &req, std::string s_tokenID, std::string *c_tokenID,
        Json::Value &rsp, std::string &errDesc, int &errorCode)
    {
        MSRestApi restApi;
        HttpRequest httpRequest;
        httpRequest.method = req.httpMethod;
        httpRequest.url = req.serviceUrl;
        httpRequest.cert = req.cert;

        if ("" != req.ip_route) {
            HCP_Log(DEBUG, MODULE_NAME) << "This task need session hold, ip_route: " << req.ip_route << HCPENDLOG;
            httpRequest.heads.insert(std::make_pair(HTTP_HEAD_IP_ROUTE, req.ip_route));
        }

        if (!req.reqBody.empty()) {
            Json::FastWriter jsonWriter;
            httpRequest.body = jsonWriter.write(req.reqBody);
        }
        httpRequest.heads.insert(std::make_pair(HTTP_HEAD_AUTH_TOKEN, s_tokenID));
        if (c_tokenID != NULL)
            httpRequest.heads.insert(std::make_pair(HTTP_HEAD_SUBJECT_TOKEN, *c_tokenID));
        return restApi.SendRequest(httpRequest, rsp, errDesc, NULL, errorCode);
    }

    int MSRestCLient::UploadAttachment(const MSRestRequest &req,
        Json::Value &rsp,
        std::string &errDesc, bool firstTime)
    {
        if (req.attachmentPath.empty()) {
            HCP_Log(ERR, MODULE_NAME) << "There is no attachment file full path. " << HCPENDLOG;
            return RETURN_ERROR;
        }

        MSRestApi restApi;
        int ret = Login(restApi, req, errDesc);
        if (RETURN_OK != ret) {
            HCP_Log(ERR, MODULE_NAME) << "Login into IAM failed. ret: " << ret << HCPENDLOG;
            return ret;
        }

        HttpRequest httpRequest;
        httpRequest.method = req.httpMethod;
        httpRequest.url = req.serviceUrl;
        httpRequest.cert = req.cert;
        httpRequest.body = req.attachmentPath;

        if ("" != req.cookie) {
            httpRequest.heads.insert(std::make_pair(HTTP_HEAD_SESSION_COOKIE, req.cookie));
        }

        ret = restApi.UploadAttachment(httpRequest, rsp, errDesc);
        if ((ret == Token_Expired || ret == Token_Invalid) && firstTime) {
            HCP_Log(ERR, MODULE_NAME) << "token is expired,need to resend request " << HCPENDLOG;
            errDesc = "";
            int reSendRet = UploadAttachment(req, rsp, errDesc, false);
            if (reSendRet != RETURN_OK) {
                HCP_Log(ERR, MODULE_NAME) << "resend uploadAttachment failed. " << HCPENDLOG;
                return reSendRet;
            }
            return RETURN_OK;
        }
        if (RETURN_OK != ret) {
            HCP_Log(ERR, MODULE_NAME) << "upload failed. " << HCPENDLOG;
            return ret;
        }

        return ret;
    }


    int MSRestCLient::DownloadAttchment(const MSRestRequest &req, std::string &errDesc, bool firstTime /*= true */)
    {
        MSRestApi restApi;
        int ret = Login(restApi, req, errDesc);
        if (ret != RETURN_OK) {
            HCP_Log(ERR, MODULE_NAME) << "Login into IAM failed. ret: " << ret << HCPENDLOG;
            return ret;
        }

        HttpRequest httpRequest;
        if (req.cookie != "") {
            httpRequest.heads.insert(std::make_pair(HTTP_HEAD_SESSION_COOKIE, req.cookie));
        }

        httpRequest.method = req.httpMethod;
        httpRequest.url = req.serviceUrl;
        httpRequest.cert = req.cert;
        httpRequest.body = req.attachmentPath;

        ret = restApi.DownloadAttchment(httpRequest, errDesc);
        if ((ret == Token_Expired || ret == Token_Invalid) && firstTime) {
            HCP_Log(ERR, MODULE_NAME) << "token is expired,need to resend request " << HCPENDLOG;
            errDesc = "";
            int reSendRet = DownloadAttchment(req, errDesc, false);
            if (reSendRet != RETURN_OK) {
                HCP_Log(ERR, MODULE_NAME) << "resend DownloadAttchment failed. " << HCPENDLOG;
                return reSendRet;
            }
            return RETURN_OK;
        }

        if (ret != RETURN_OK) {
            HCP_Log(ERR, MODULE_NAME) << "Download file from "
                                      << WIPE_SENSITIVE(httpRequest.url) << "failed. " << HCPENDLOG;
            return ret;
        }
        return RETURN_OK;
    }

    MSRestCLient &MSRestCLient::Instance()
    {
        static MSRestCLient restClient;
        return restClient;
    }

    MSRestCLient &MSRestCLient::OutConnectInstance()
    {
        static MSRestCLient outRestClient;
        return outRestClient;
    }

/*lint -e1788*/
    int MSRestCLient::GetRestApi(std::string &key, MSRestApi *&)
    {
        CThreadAutoLock tlock(&m_lockRestApi);
        return 0;
    }//lint !e1788


//    void MSRestCLient::BuildBaseRequest(MSRestRequest &req, const std::string method, const std::string api,
//                                        const std::string params, const std::string cert) {
//
//        std::string IAMAuthURLPostfix = "/v3/auth/tokens";
//        SecString password;
//        SecString decPasswd;
//            HCP_Log(ERR, MODULE_NAME) << "Failed to decInFilePwdForStore the IAMPasswd." << HCPENDLOG;
//        }
//
//
//        HCP_Logger_noid(DEBUG, MODULE_NAME) << "LBAddress: " << LBAddress << HCPENDLOG;
//        HCP_Logger_noid(DEBUG, MODULE_NAME) << "serviceURL: " << WIPE_SENSITIVE(serviceURL) << HCPENDLOG;
//    }
}


/*lint +e1788*/
