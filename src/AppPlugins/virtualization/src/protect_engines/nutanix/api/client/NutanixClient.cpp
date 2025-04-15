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
#include <common/JsonHelper.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <sstream>
#include <cstdlib>
#include "system/System.hpp"
#include "protect_engines/cnware/common/Structs.h"
#include "common/utils/Utils.h"
#include "curl_http/CodeConvert.h"
#include "protect_engines/nutanix/common/ErrorCode.h"
#include "NutanixClient.h"

using namespace VirtPlugin;
namespace {
const std::string MODULE_NAME = "NutanixClient";
const int RETRY_TIME = 3;
const int BASE64_SIZE_NUM = 2;
const int32_t CLIENT_RETRY_INTERVAL = 15;
const int32_t CLIENT_RETRY_TIMES = 3;
}
namespace NutanixPlugin {
NutanixClient::NutanixClient(Authentication auth) : m_auth(auth), m_retryIntervalTime(CLIENT_RETRY_INTERVAL),
    m_retryTimes(CLIENT_RETRY_TIMES)
{
    // http返回的错误信息;
    m_rspErrMsg.errorCode.code = SUCCESS;
    m_rspErrMsg.message = "";
    m_rspErrMsg.detailedMessage = "";
};

bool NutanixClient::CheckParams(ModelBase &model)
{
    if (!model.UserInfoIsSet()) {
        ERRLOG("User info does not set. Task id: %s", m_taskId.c_str());
        return false;
    }
    if (!model.EndPointIsSet()) {
        ERRLOG("The param endpoint not be set. Task id: %s", m_taskId.c_str());
        return false;
    }
    return true;
}

int32_t NutanixClient::Init(const ApplicationEnvironment &appEnv)
{
    SetRetryTimes(1); // 使用session会话，不使用restClient的token重试机制
    m_taskId = appEnv.id;
    DBGLOG("SUCCESS Initiate CNware client");
    return SUCCESS;
}


int32_t NutanixClient::ParseErrorBody(const std::shared_ptr<ResponseModel> &response)
{
    if (!Module::JsonHelper::JsonStringToStruct(response->GetBody(), m_rspErrMsg)) {
        ERRLOG("Transfer Response Body failed, %s", m_taskId.c_str());
        return FAILED;
    }
    ERRLOG("http error code:%d, message:%s, taskID:%s", m_rspErrMsg.errorCode.code, m_rspErrMsg.message.c_str(),
        m_taskId.c_str());
    return SUCCESS;
}

int32_t NutanixClient::SendRequest(std::shared_ptr<ResponseModel> response, NutanixRequest &req,
    RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode)
{
    requestInfo.m_resourcePath = "https://{addr}:{port}/" + req.url;
    std::string address = req.GetEndpoint();
    requestInfo.m_pathParams["addr"] = Utils::CheckIpv6Valid(address) ? "[" + address + "]" : address;
    requestInfo.m_pathParams["port"] = req.GetPort();
    requestInfo.m_auth = req.GetUserInfo();
    int32_t retryNum = 0;
    while (retryNum < RETRY_TIME) {
        SetSession(req, requestInfo);
        INFOLOG("send request for %d time to %s, %s", (retryNum + 1),
            WIPE_SENSITIVE(req.url).c_str(), m_taskId.c_str());
        if (DoSendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
            WARNLOG("Do send request failed. Url: %s, http status: %d, errCode: %d, error message: %d, %s",
                requestInfo.m_resourcePath.c_str(), response->GetHttpStatusCode(),
                errorCode, errorDes.c_str(), m_taskId.c_str());
            DelayTimeSendRequest();
            ++retryNum;
        } else {
            return SUCCESS;
        }
    }
    ERRLOG("send request failed.");
    return FAILED;
}

NutanixErrorMsg NutanixClient::GetErrorCode(void)
{
    return m_rspErrMsg;
}

void NutanixClient::RefreshSession(RequestInfo &requestInfo, const std::shared_ptr<ResponseModel> &response,
    NutanixRequest &req)
{
    if (IsPasswordAccess(requestInfo)) {
        // 如果是密码账号访问，就去刷新token;
        if (response->GetCookies().empty()) {
            ERRLOG("Parse cookie failed, cookie is empty.");
        } else {
            IP_PORT_USER_KEY info = std::make_tuple(req.GetEnvAddress(), req.GetPort(), m_auth.authkey);
            std::shared_ptr<NutanixSession> session = std::make_shared<NutanixSession>(response->GetCookies());
            NutanixSessionCacheManage::GetInstance()->Update(info, session);
        }
    }
}

void NutanixClient::CleanSession(RequestInfo &requestInfo, NutanixRequest &req)
{
    IP_PORT_USER_KEY info = std::make_tuple(req.GetEnvAddress(), req.GetPort(), m_auth.authkey);

    std::shared_ptr<NutanixSession> session = std::make_shared<NutanixSession>(requestInfo.m_headerParams["Cookie"]);
    NutanixSessionCacheManage::GetInstance()->Clean(info, session);
}

bool NutanixClient::IsPasswordAccess(RequestInfo &requestInfo)
{
    std::map<std::string, std::string>::const_iterator token = requestInfo.m_headerParams.find("Cookie");
    std::map<std::string, std::string>::const_iterator passwd = requestInfo.m_headerParams.find("Authorization");
    // 没有cookie，但是有Authorization;
    return passwd != requestInfo.m_headerParams.end() && token == requestInfo.m_headerParams.end();
}

int32_t NutanixClient::DoSendRequest(std::shared_ptr<ResponseModel> response, NutanixRequest &req,
    RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode)
{
    if (CallApi(requestInfo, response, req) != SUCCESS) {
        // 未到curl层就失败了，无需解析;
        ERRLOG("Failed to send request. Task id: %s", m_taskId.c_str());
        return FAILED;
    }
    if (response->GetErrCode() != CURLE_OK) {
        // curl层发生错误，无需解析;
        ERRLOG("Failed to send request. Task id: %s, curl error code:%d, msg:%s", m_taskId.c_str(),
            response->GetErrCode(), response->GetErrString().c_str());
        NutanixRetryCodeStruct retryCode;
        if (retryCode.IsCurlRetryCode(response->GetErrCode())) {
            // 走重试逻辑;
            return FAILED;
        }
        return SUCCESS;
    }
    // http消息发送成功，以下是解析http错误;
    if (response->GetStatusCode() == Module::SC_UNAUTHORIZED) {
        // 登录失败;
        (void)ParseErrorBody(response);
        if (IsPasswordAccess(requestInfo)) {
            // 如果是密码失败，就不重试了;
            ERRLOG("Failed to send request. Task id: %s, user:%s, password error", m_taskId.c_str(),
                WIPE_SENSITIVE(m_auth.authkey).c_str());
            return SUCCESS;
        }  else {
            // tooken登录失败，需要切换为账号密码重试登录;
            CleanSession(requestInfo, req);
            return FAILED;
        }
    }
    // 登录成功，只是业务处理逻辑失败，不影响刷新session;
    RefreshSession(requestInfo, response, req);
    if (response->GetStatusCode() != Module::SC_OK && response->GetStatusCode() != Module::SC_CREATED) {
        NutanixRetryCodeStruct retryCode;
        if (retryCode.IsHttpStatusRetryCode(response->GetStatusCode())) {
            // 走重试逻辑;
            return FAILED;
        }
    }
    // 业务逻辑错误就不重试，返回给上层处理;
    return SUCCESS;
}

void NutanixClient::DelayTimeSendRequest()
{
    auto now = std::chrono::steady_clock::now();
    while ((double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -
        now).count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den) <
        m_retryIntervalTime) {
        INFOLOG("Waiting for Nutanix client ... ");
        sleep(1);
    }
    return;
}

bool NutanixClient::SetSession(const NutanixRequest &request, RequestInfo &requestInfo)
{
    IP_PORT_USER_KEY info = std::make_tuple(request.GetEnvAddress(), request.GetPort(), m_auth.authkey);
    NutanixSession session;

    SessionCacheRetvalue ret = NutanixSessionCacheManage::GetInstance()->Get(info, session);
    requestInfo.m_headerParams.erase("Cookie");
    requestInfo.m_headerParams.erase("Authorization");
    if (ret == SessionCacheRetvalue::SESSION_IS_INVALID_AND_NEED_UPDATE) {
        std::string auth_header;

        std::string encoded_credentials = m_auth.authkey + ":" + m_auth.authPwd;
        Module::CodeConvert::EncodeBase64(encoded_credentials.size() * BASE64_SIZE_NUM, encoded_credentials,
            auth_header);
        requestInfo.m_headerParams["Authorization"] = "Basic " + auth_header;
        DBGLOG("SESSSION_IS_INVALID_AND_NEED_UPDATE");
    } else if (ret == SessionCacheRetvalue::SESSION_IS_VALID) {
        requestInfo.m_headerParams["Cookie"] = session.m_cookie;
        DBGLOG("SESSION_IS_VALID");
    }  else {
        // SESSION_IS_INVALID_AND_NONEED_UPDATE;
        ERRLOG("SESSION_IS_INVALID_AND_NONEED_UPDATE");
        return false;
    }
    return true;
}

int32_t NutanixClient::CheckAuth(GetClusterListRequest &req, int64_t &errorCode, std::string &errorDes)
{
    if (!CheckParams(req)) {
        ERRLOG("action check auth: Failed to check param. Ip: %s. %s", req.GetEndpoint().c_str(), m_taskId.c_str());
        return FAILED;
    }
    RequestInfo requestInfo;
    req.FillRequest(requestInfo);
    std::shared_ptr<NutanixResponse<struct ClusterListDataResponse>> response =
        std::make_shared<NutanixResponse<struct ClusterListDataResponse>>();
    if (response == nullptr) {
        ERRLOG("Failed to create checkauth response handler. %s", m_taskId.c_str());
        return FAILED;
    }
    requestInfo.m_resourcePath = "https://{addr}:{port}/" + req.url;
    std::string address = req.GetEndpoint();
    requestInfo.m_pathParams["addr"] = Utils::CheckIpv6Valid(address) ? "[" + address + "]" : address;
    requestInfo.m_pathParams["port"] = req.GetPort();
    requestInfo.m_auth = req.GetUserInfo();
    requestInfo.m_headerParams.erase("Cookie");
    requestInfo.m_headerParams.erase("Authorization");
    std::string auth_header;
    std::string encoded_credentials = m_auth.authkey + ":" + m_auth.authPwd;
    Module::CodeConvert::EncodeBase64(encoded_credentials.size() * BASE64_SIZE_NUM, encoded_credentials,
        auth_header);
    requestInfo.m_headerParams["Authorization"] = "Basic " + auth_header;
    if (DoSendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        errorCode = response->GetErrCode();
        errorDes = response->GetErrString();
        ERRLOG("Do send request failed. Url: %s, http status: %d, errCode: %d, error message: %d, %s",
            requestInfo.m_resourcePath.c_str(), response->GetHttpStatusCode(),
            errorCode, errorDes.c_str(), m_taskId.c_str());
        return FAILED;
    }
    if (response->GetStatusCode() == Module::SC_OK) {
        INFOLOG("CheckAuth: success! %s", m_taskId.c_str());
        RefreshSession(requestInfo, response, req);
        return SUCCESS;
    }
    if (response->GetStatusCode() == Module::SC_UNAUTHORIZED) {
        errorCode = NutanixErrorCode::USER_OR_PASSWORD_IS_INVALID;
    }
    return FAILED;
}
}
