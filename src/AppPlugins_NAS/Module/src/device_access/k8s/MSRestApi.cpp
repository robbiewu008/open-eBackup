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
#include "device_access/k8s/MSRestApi.h"
#include "device_access/k8s/Interfaces.h"
#include "curl_http/HttpClientInterface.h"
#include "log/Log.h"
#include "system/System.hpp"
#include "common/CTime.h"
#include "config_reader/ConfigIniReader.h"

namespace Module {
    const std::string MODULE_NAME = "MSRestClient";
    const std::string REST_PARAMS_ERR = "Error";
    const std::string REST_PARAMS_CODE = "Code";
    const std::string REST_PARAMS_DESC = "Description";
    const std::string REST_PARAMS_DATA = "Data";
    const std::string REST_PARAMS_CONTENT_TYPE = "Content-Type";
    const std::string REST_PARAMS_ATTACHMENT_TYPE = "Attachment";
#ifdef NATIVE_PROFILE
    MSRestApi::MSRestApi():
        m_logined(true)
#else

    MSRestApi::MSRestApi() :
            m_logined(false)
#endif
    {
        m_pHttpCLient = IHttpClient::GetInstance();
    }

    MSRestApi::~MSRestApi() {
        IHttpClient::ReleaseInstance(m_pHttpCLient);
        m_pHttpCLient = NULL;
    }

    int MSRestApi::Login(
            bool retry,
            const std::string &url,
            const std::string &userName,
            const std::string &password,
            const std::string &cert,
            std::string &errDesc,
            bool globalIAM,
            const std::string &tenantID,
            const std::string &domainID,
            const std::string &specifyNetowrk
    ) {
        HttpRequest req;
        req.method = "POST";
        req.url = url;
        req.cert = cert;
        req.specialNetworkCard = specifyNetowrk;

        Json::Value rootValue, identityValue, userValue;
        Json::FastWriter jsonWriter;

        userValue[REST_PARAM_IAM_NAME] = userName;
        userValue[REST_PARAM_IAM_PASSWORD] = password;
        //added for replication
        if (globalIAM == true) {
            userValue[REST_PARAM_IAM_SCOPE_TYPE] = 5;
            userValue[REST_PARAM_IAM_DOMAIN_ID] = domainID;
            userValue[REST_PARAM_IAM_TENANT_ID] = tenantID;
        } else
            userValue[REST_PARAM_IAM_SCOPE_TYPE] = 4;

        identityValue[REST_PARAM_IAM_PASSWORD][REST_PARAM_IAM_USER] = userValue;
        identityValue[REST_PARAM_IAM_METHOD][0] = REST_PARAM_IAM_PASSWORD;

        rootValue[REST_PARAM_IAM_AUTH][REST_PARAM_IAM_IDENTITY] = identityValue;

        req.body = jsonWriter.write(rootValue);

        Json::Value data;
        int iRet = SendAndParseRsp(retry, req, data, errDesc, true);

        if (RETURN_OK == iRet) {
            m_logined = true;
        }
        for (std::string::iterator it = req.body.begin(); it != req.body.end(); ++it) {
            *it = 0;
        }
        return iRet;
    }

    int MSRestApi::Login(const std::string &url,
                         const Json::Value &loginBody,
                         const std::string &cert,
                         std::string &errDesc,
                         bool retry
    ) {
        Json::FastWriter jsonWriter;
        HttpRequest req;
        req.method = "POST";
        req.url = url;
        req.cert = cert;
        req.body = jsonWriter.write(loginBody);

        Json::Value data;
        int iRet = SendAndParseRsp(retry, req, data, errDesc, true);
        if (RETURN_OK == iRet) {
            m_logined = true;
        }
        for (std::string::iterator it = req.body.begin(); it != req.body.end(); ++it) {
            *it = 0;
        }
        return iRet;
    }

    int MSRestApi::Logout(const std::string &url,
                          const std::string &cert,
                          std::string &errDesc) {
        if (!m_logined) {
            HCP_Log(INFO, MODULE_NAME) << "It is not logined." << HCPENDLOG;
            return RETURN_OK;
        }

        HttpRequest req;
        req.method = "DELETE";
        req.url = url;
        req.cert = cert;
        req.heads.insert(std::make_pair(HTTP_HEAD_AUTH_TOKEN, m_tokenID));

        Json::Value data;
        int iRet = SendAndParseRsp(true, req, data, errDesc, true);

        m_logined = false;
        m_tokenID.clear();
        return iRet;
    }


    int MSRestApi::SendRequest(const HttpRequest &req,
                               Json::Value &data,
                               std::string &errorDes,
                               bool retry
    ) {
        if (!m_logined) {
            HCP_Log(ERR, MODULE_NAME) << "It is not logined." << HCPENDLOG;
            return RETURN_ERROR;
        }

        HttpRequest httpRequest = req;
        httpRequest.heads.insert(std::make_pair(HTTP_HEAD_AUTH_TOKEN, m_tokenID));

        return SendAndParseRsp(retry, httpRequest, data, errorDes);
    }

    int MSRestApi::SendRequestDirectly(const HttpRequest &req,
                                       Json::Value &data,
                                       std::string &errorDes,
                                       bool retry /*default true*/) {
        return SendAndParseRsp(retry, req, data, errorDes, false);
    }

    int MSRestApi::SendRequestDirectly(const HttpRequest &req,
                                       std::map<std::string, std::set<std::string> > &rspHeaders,
                                       std::string &rspData, std::string &errorDes,
                                       bool retry /*default true*/) {
        return SendRequest(req, rspHeaders, rspData, errorDes, retry);
    }

    int MSRestApi::SendRequestDirectly(const HttpRequest &req,
                                       std::string &rspData, std::string &errorDes,
                                       bool retry /*default true*/) {
        std::map<std::string, std::set<std::string> > rspHeaders;
        return SendRequest(req, rspHeaders, rspData, errorDes, retry);
    }

//send raw request information (with errorCode)
    int MSRestApi::SendRequest(const HttpRequest &req, Json::Value &data, std::string &errorDes, std::string *token,
                               int &errorCode) {
        if (NULL == m_pHttpCLient) {
            HCP_Log(ERR, MODULE_NAME) << "HttpClient is NULL. " << HCPENDLOG;
            return RETURN_ERROR;
        }

        /*lint -e118*/
        std::shared_ptr<IHttpResponse> rsp = m_pHttpCLient->SendRequest(req);
        if (NULL == rsp.get()) {
            HCP_Log(ERR, MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return RETURN_ERROR;
        }
        /*lint -e118*/
        /*lint -e734*/
        HCP_Log(DEBUG, MODULE_NAME) << "StatsCode: " << rsp->GetHttpStatusCode() << HCPENDLOG;
        if (!rsp->Success()) {
            if (rsp->GetErrCode() == 0) {
                errorDes = rsp->GetHttpStatusDescribe();
                HCP_Log(ERR, MODULE_NAME) << "StatsCode: " << rsp->GetHttpStatusCode()
                                          << " , Http response error. Error is " << errorDes << HCPENDLOG;
                (void) GetErrorCode(rsp->GetBody(), data, errorDes, errorCode);
                return rsp->GetHttpStatusCode();    //return http status code
            } else {
                errorDes = rsp->GetErrString();
                HCP_Log(ERR, MODULE_NAME) << "StatsCode: " << rsp->GetHttpStatusCode()
                                          << " , Send http request occur network error. Error is " << errorDes
                                          << HCPENDLOG;
                return HTTP_NETWORK_ERROR;
            }
        }
        /*lint +e734*/

        int iRet = GetErrorCode(rsp->GetBody(), data, errorDes, errorCode);
        if (RETURN_OK != iRet) {
            return iRet;
        }

        if (token) {
            const std::set<std::string> &tokenlist = rsp->GetHeadByName(HTTP_HEAD_SUBJECT_TOKEN);
            if (tokenlist.empty()) {
                HCP_Log(ERR, MODULE_NAME) << "Token is empty. " << HCPENDLOG;
                return RETURN_ERROR;
            } else {
                *token = *tokenlist.begin();
            }
        }
        return RETURN_OK;
    }


//lint -e734
    int MSRestApi::SendRequest(const HttpRequest &req,
                               std::map<std::string, std::set<std::string> > &rspHeaders,
                               std::string &rspData,
                               std::string &errorDes,
                               bool retry
    ) {
        if (NULL == m_pHttpCLient) {
            HCP_Log(ERR, MODULE_NAME) << "HttpClient is NULL." << HCPENDLOG;
            return RETURN_ERROR;
        }
        std::shared_ptr<IHttpResponse> rsp;
        uint64_t StartTime, Count = 0;

        StartTime = CTime::GetTimeSec();
        do {
            uint32_t timeout = retry ? 90 : 45;
            HttpRequest tempReq = req;
            tempReq.url = FormatFullUrl(tempReq.url);
            rsp = m_pHttpCLient->SendRequest(tempReq, timeout);//lint !e55 !e63 !e118
            Count++;
            if (NULL == rsp.get()) {
                HCP_Log(ERR, MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
                return RETURN_ERROR;
            }
            if (rsp->Busy() && retry) {
                uint64_t CurTime = CTime::GetTimeSec();
                if (CurTime >=
                    (StartTime + ConfigReader::getInt("General", "HTTP_BUSY_REPEAT_TIME"))) {
                    break;
                } else {
                    HCP_Log(INFO, MODULE_NAME) << "remote service is busy. cur count:" << Count
                                               << ", Cur Time:" << CurTime
                                               << ", interval:" << ConfigReader::getInt("General",
                                                                                        "HTTP_BUSY_REPEAT_INTERVAL")
                                               << ", End Time:" << (StartTime +
                                                                    ConfigReader::getInt("General",
                                                                                         "HTTP_BUSY_REPEAT_TIME"))
                                               << HCPENDLOG;
                    if (CurTime < (StartTime + Count * ConfigReader::getInt("General",
                                                                            "HTTP_BUSY_REPEAT_INTERVAL"))) {
                        hcp_sleep(StartTime + Count * ConfigReader::getInt("General",
                                                                           "HTTP_BUSY_REPEAT_INTERVAL") - CurTime);
                    }
                }
            } else {
                break;
            }
        } while (1);
        // if network status is not 200
        if (!rsp->Success()) {
            // if libcurl rc is 0 means libcurl send success
            if (rsp->GetErrCode() == 0) {
                errorDes = rsp->GetHttpStatusDescribe();
                HCP_Log(ERR, MODULE_NAME) << "Http response error. Error is: (" << rsp->GetHttpStatusDescribe()
                                          << "), network status code :" << rsp->GetStatusCode() << HCPENDLOG;
                // retrun network status code
                return rsp->GetStatusCode();
            } else {
                // when libcurl send failed return ec of libcurl
                errorDes = rsp->GetErrString();
                HCP_Log(ERR, MODULE_NAME) << "Send http request occur network error. Error is: (" << rsp->GetErrString()
                                          << "), libcurl ec is:" << rsp->GetErrCode() << HCPENDLOG;
                return rsp->GetErrCode();
            }
        }

        rspHeaders = rsp->GetHeaders();
        rspData = rsp->GetBody();
        return RETURN_OK;
    }

    int MSRestApi::SendAndParseRsp(bool retry,
                                   const HttpRequest &req,
                                   Json::Value &data,
                                   std::string &errorDes,
                                   const bool is_login /*= false*/

    ) {

        std::map<std::string, std::set<std::string> > headers;
        std::string dataStr;

        int ret = SendRequest(req, headers, dataStr, errorDes, retry);
        if (RETURN_OK != ret) {
            HCP_Log(ERR, MODULE_NAME) << "Send request failed." << HCPENDLOG;
            return ret;
        }

        int iRet = ParseRsp(dataStr, data, errorDes);
        if (RETURN_OK != iRet) {
            HCP_Log(ERR, MODULE_NAME) << "Parse body failed " << HCPENDLOG;
            return iRet;
        }

        if (is_login) {
            return ParseToken(MSRestApi::GetHeadByName(headers, HTTP_HEAD_SUBJECT_TOKEN));
        }
        return RETURN_OK;
    }

    std::set<std::string> MSRestApi::GetHeadByName(const std::map<std::string, std::set<std::string> > &headers,
                                                   const std::string &header_name) {
        std::map<std::string, std::set<std::string> >::const_iterator it = headers.find(header_name);
        if (it != headers.end()) {
            return it->second;
        }
        return std::set<std::string>();
    }
//lint +e734

    bool MSRestApi::IsLogined() {
        return m_logined;
    }


    int MSRestApi::DownloadAttchment(const HttpRequest &req, std::string &errorDes) {
        if (!m_logined) {
            HCP_Log(ERR, MODULE_NAME) << "It is not logined." << HCPENDLOG;
            return RETURN_ERROR;
        }

        HttpRequest httpRequest = req;
        {
            httpRequest.heads.insert(std::make_pair(HTTP_HEAD_AUTH_TOKEN, Token()));
        }

        if (m_pHttpCLient == NULL) {
            HCP_Log(ERR, MODULE_NAME) << "DownloadAttchment, HttpClient is NULL. " << HCPENDLOG;
            return RETURN_ERROR;
        }
        std::shared_ptr<IHttpResponse> rsp = m_pHttpCLient->DownloadAttchment(httpRequest);
        if (rsp.get() == NULL) {
            HCP_Log(ERR, MODULE_NAME) << "Return response is empty. " << HCPENDLOG;
            return RETURN_ERROR;
        }

        if (rsp->GetErrCode() != 0) {
            errorDes = rsp->GetErrString();
            HCP_Log(ERR, MODULE_NAME) << "Send http request occur network error. Error  is " << rsp->GetErrString()
                                      << HCPENDLOG;
            return rsp->GetErrCode();
        }
        return RETURN_OK;
    }

    int MSRestApi::UploadAttachment(const HttpRequest &req, Json::Value &data, std::string &errorDes) {
        if (!m_logined) {
            HCP_Log(ERR, MODULE_NAME) << "It is not logined." << HCPENDLOG;
            return RETURN_ERROR;
        }

        HttpRequest httpRequest = req;
        {
            httpRequest.heads.insert(std::make_pair(HTTP_HEAD_AUTH_TOKEN, Token()));
        }
        httpRequest.heads.insert(std::make_pair(REST_PARAMS_CONTENT_TYPE, REST_PARAMS_ATTACHMENT_TYPE));

        return UploadAttachmentAndParseRsp(httpRequest, data, errorDes);
    }

    int MSRestApi::UploadAttachmentAndParseRsp(const HttpRequest &req, Json::Value &data, std::string &errorDes) {
        if (NULL == m_pHttpCLient) {
            HCP_Log(ERR, MODULE_NAME) << "HttpClient is NULL. " << HCPENDLOG;
            return RETURN_ERROR;
        }

        std::shared_ptr<IHttpResponse> rsp;
        if (!m_pHttpCLient->UploadAttachment(req, rsp)) {
            HCP_Log(ERR, MODULE_NAME) << "Upload attachment failed." << HCPENDLOG;
            return RETURN_ERROR;
        }

        HCP_Log(DEBUG, MODULE_NAME) << "StatsCode: " << rsp->GetHttpStatusCode() << HCPENDLOG;

        if (rsp->GetErrCode() == Token_Expired) {
            errorDes = rsp->GetErrString();
            HCP_Log(ERR, MODULE_NAME) << "token is expired" << WIPE_SENSITIVE(rsp->GetErrString()) << HCPENDLOG;
            return rsp->GetErrCode();
        }

        if (rsp->GetErrCode() != 0) {
            errorDes = rsp->GetErrString();
            HCP_Log(ERR, MODULE_NAME) << "Send http request occur network error. Error  is "
                                      << WIPE_SENSITIVE(rsp->GetErrString()) << HCPENDLOG;
            return rsp->GetErrCode();
        }

        int iRet = ParseRsp(rsp->GetBody(), data, errorDes);
        if (RETURN_OK != iRet) {
            HCP_Log(ERR, MODULE_NAME) << "Parse body failed " << HCPENDLOG;
            return iRet;
        }
        //Get Set-Cookie from HTTP response
        std::string cookie_value;
        const std::set<std::string> &cookie = rsp->GetHeadByName(HTTP_HEAD_SET_COOKIE);
        if (cookie.empty()) {
            HCP_Log(DEBUG, MODULE_NAME) << "Set-Cookie in response is empty " << HCPENDLOG;
        } else {
            cookie_value = *cookie.begin();
            std::vector<std::string> strs;
            (void) boost::split(strs, cookie_value, boost::is_any_of(";"));
            data[HTTP_HEAD_SET_COOKIE] = (strs.size() > 0) ? strs[0] : "";
        }

        return RETURN_OK;
    }

    int MSRestApi::ParseRsp(const std::string &json_data,
                            Json::Value &data,
                            std::string &errorDes) {
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(json_data, jsonValue)) {
            errorDes = "Parse json string failed";
            HCP_Log(ERR, MODULE_NAME) << "Parse json string failed." << HCPENDLOG;
            return RETURN_ERROR;
        }
        if (jsonValue.isObject()) {
            if (jsonValue.isMember("data") && jsonValue["data"].isArray()) {
                HCP_Log(DEBUG, MODULE_NAME) << "It's a response from infrastructure. Not need to check" << HCPENDLOG;
                data = jsonValue["data"];
                return RETURN_OK;
            }
            if (jsonValue.isMember("kind") && jsonValue["kind"] == "PodList") {
                HCP_Log(DEBUG, MODULE_NAME) << "It's a k8s response. Not need to check" << HCPENDLOG;
                data = jsonValue;
                return RETURN_OK;
            }
            if (jsonValue.isMember("items")) {
                HCP_Log(DEBUG, MODULE_NAME) << "It's a GetVcenterAuthenticationInfoFormPM response. Not need to check"
                                            << HCPENDLOG;
                data = jsonValue;
                return RETURN_OK;
            }
            if (!jsonValue.isMember(REST_PARAMS_ERR)
                || !jsonValue[REST_PARAMS_ERR].isMember(REST_PARAMS_CODE)
                || !jsonValue[REST_PARAMS_ERR].isMember(REST_PARAMS_DESC)) {
                errorDes = "Json object format is error. ";
                HCP_Log(ERR, MODULE_NAME) << "Json object error." << HCPENDLOG;
                return RETURN_ERROR;
            }

            if (0 != jsonValue[REST_PARAMS_ERR][REST_PARAMS_CODE].asInt()) {
                errorDes = jsonValue[REST_PARAMS_ERR][REST_PARAMS_DESC].asString();
                HCP_Log(ERR, MODULE_NAME) << "code : " << jsonValue[REST_PARAMS_ERR][REST_PARAMS_CODE].asInt()
                                          << ", Describe : " << WIPE_SENSITIVE(errorDes) << HCPENDLOG;
                return jsonValue[REST_PARAMS_ERR][REST_PARAMS_CODE].asInt();
            }

            if (jsonValue.isMember(REST_PARAMS_DATA)) {
                data = jsonValue[REST_PARAMS_DATA];
            }
        } else {
            // response from PM maybe an array, just return that as data
            data = jsonValue;
        }

        return RETURN_OK;
    }

    int MSRestApi::ParseToken(const std::set<std::string> &tokens) {
        if (tokens.empty()) {
            HCP_Log(ERR, MODULE_NAME) << "Token is empty. " << HCPENDLOG;
            return RETURN_ERROR;
        }

        m_tokenID = *tokens.begin();
        return RETURN_OK;
    }

    int MSRestApi::GetErrorCode(const std::string &json_data,
                                Json::Value &data,
                                std::string &errorDes,
                                int &errorCode) {
        int iRet = ParseRsp(json_data, data, errorDes);
        if (RETURN_ERROR == iRet) {
            HCP_Log(ERR, MODULE_NAME) << "Parse body failed " << HCPENDLOG;
            return iRet;
        }
        errorCode = iRet;   //get errorCode

        return RETURN_OK;
    }

    void MSRestApi::CleanToken() {
        for (std::string::size_type i = 0; i < m_tokenID.size(); ++i) {
            m_tokenID[i] = (char) 0xcc;
        }
    }
}

