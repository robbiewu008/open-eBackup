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
#include "curl_http/HttpStatus.h"
#include "protect_engines/cnware/common/Structs.h"
#include "common/utils/Utils.h"
#include "model/CreateSnapshotRequest.h"
#include "model/CreateSnapshotResponse.h"
#include "model/InquiriesSnapshotRequest.h"
#include "model/InquiriesSnapshotResponse.h"
#include "model/AddEmptyDiskRequest.h"
#include "model/AddEmptyDiskResponse.h"
#include "model/AttachvolumeSnapshotResponse.h"
#include "model/AttachVolumeSnapshotRequest.h"
#include "CNwareClient.h"

using namespace VirtPlugin;
namespace CNwarePlugin {
namespace {
const std::string MODULE_NAME = "CNwareClient";
const std::string CNWARE_DB_VMINFO = "3";
const int RETRY_TIME = 10;
const int64_t CNWARE_AUTH_FAILED = 401;
const int64_t CNWARE_JOB_CONFLICT = 409;
const int64_t UNFINISHED_JOB_EXISTS = 2182;
const int64_t REFRESH_POOL_JOB = 4095;
const int64_t REFRESH_POOL_JOB_CONFLICT = 2382;
const int64_t UNSUPPORT_LUN = 200037;
const int NUM_2 = 2;
}

bool CNwareClient::CheckParams(ModelBase &model)
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

int32_t CNwareClient::InitCNwareClient(const ApplicationEnvironment &appEnv)
{
    SetRetryTimes(1); // 使用session会话，不使用restClient的token重试机制
    std::lock_guard<std::mutex> lock(m_mutexCache);
    m_taskId = (appEnv.id);
    if (m_cnwareSessionCache == nullptr) {
        m_cnwareSessionCache = CNwareSessionCache::GetInstance();
    }
    if (m_cnwareSessionCache == nullptr) {
        ERRLOG("Init CNwareclient failed! Taskid: %s", appEnv.id.c_str());
        return FAILED;
    }
    DBGLOG("SUCCESS Initiate CNware client, %s", appEnv.auth.extendInfo.c_str());
    return SUCCESS;
}

bool CNwareClient::CheckSessionValidity(const std::tuple<std::string, std::string> &cnwareInfo)
{
    m_cnwareSessionPtr = m_cnwareSessionCache->GetCNwareSession(cnwareInfo);
    if (m_cnwareSessionPtr == nullptr) {
        DBGLOG("Do not find valid session, %s", m_taskId.c_str());
        return false;
    }
    return true;
}

int32_t CNwareClient::GetSessionAndlogin(CNwareRequest &req, int64_t &errorCode,
    std::string &errorDes)
{
    std::lock_guard<std::mutex> clock(m_mutexCache);
    std::lock_guard<std::mutex> slock(m_mutexSession);
    if (CheckSessionValidity(std::make_tuple(req.GetEndpoint(), req.GetUserInfo().name))) {
        INFOLOG("Already login to CNware client! Ip: %s", req.GetEndpoint().c_str());
        return SUCCESS;
    }
    if (Login(req, errorCode, errorDes) != SUCCESS) {
        ERRLOG("Failed login to CNware client! Ip: %s", req.GetEndpoint().c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareClient::Login(CNwareRequest &req, int64_t &errorCode, std::string &errorDes)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return FAILED;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "https://{cnwareAddr}:{port}/api/login";
    std::string address = req.GetEndpoint();
    requestInfo.m_pathParams["cnwareAddr"] = Utils::CheckIpv6Valid(address) ? "[" + address + "]" : address;
    requestInfo.m_pathParams["port"] = req.GetPort();
    requestInfo.m_headerParams["Content-Type"] = "application/json";
    requestInfo.m_queryParams = {};
    requestInfo.m_auth = req.GetUserInfo();
    if (!BuildLoginBody(requestInfo.m_body)) {
        ERRLOG("Failed to build login body. Ip: %s", req.GetEndpoint().c_str());
        return FAILED;
    }
    std::shared_ptr<ResponseModel> response = std::make_shared<ResponseModel>();
    SetRetryTimes(1);
    if (CallApi(requestInfo, response, req) != SUCCESS) {
        ParseResonseBody(response, errorCode, errorDes, req);
        ERRLOG("Failed to send request. Ip: %s", req.GetEndpoint().c_str());
        return FAILED;       // cert error, CallApi will return FALSE, this scene need judge err code.
    }
    if (response.get() == nullptr) {
        ERRLOG("Return response is empty.Task id: %s", m_taskId.c_str());
        return FAILED;
    }
    ParseResonse(response, errorCode, errorDes, req);
    if (RefreshSession(response, req)) {
        ERRLOG("Failed to refresh session. Ip: %s", req.GetEndpoint().c_str());
        return FAILED;
    }
    INFOLOG("Login. Ip: %s", req.GetEndpoint().c_str());
    return SUCCESS;
}

bool CNwareClient::ParseResonseBody(const std::shared_ptr<ResponseModel> &response, int64_t &errorCode,
    std::string &errorDes, const CNwareRequest &req)
{
    if (response.get() == nullptr) {
        ERRLOG("Return response is empty.Task id: %s", m_taskId.c_str());
        return false;
    }
    LoginResponseBody bodyValue;
    if (Module::JsonHelper::JsonStringToStruct(response->GetBody(), bodyValue)) {
        ERRLOG("Transfer LoginResponseBody failed, %s", m_taskId.c_str());
        errorCode = bodyValue.m_errorCode;
        errorDes = bodyValue.m_message;
    } else {
        errorCode = response->GetErrCode();
        errorDes = response->GetErrString();
    }
    return true;
}

int32_t CNwareClient::ParseResonse(const std::shared_ptr<ResponseModel> &response, int64_t &errorCode,
    std::string &errorDes, const CNwareRequest &req)
{
    if (response.get() == nullptr) {
        ERRLOG("Return response is empty.Task id: %s", m_taskId.c_str());
        return FAILED;
    }
    // curl success, http response success
    if (response->Success()) {
        DBGLOG("curl success and http success. Task id: %s", m_taskId.c_str());
        return SUCCESS;
    }

    if (ParseErrorBody(response, errorCode, errorDes)) {
        ERRLOG("ParseErrorBody failed, %s", m_taskId.c_str());
        return FAILED;
    }

    // not 200 OK
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Request ip(%s) return failed.", req.GetEndpoint().c_str());
        return FAILED;
    }

    // 1.curl success,http response error with http status codes
    if (response->GetErrCode() == 0) {
        errorDes = response->GetHttpStatusDescribe(); // http status error description
        errorCode = response->GetErrCode();
        ERRLOG("Curl ok,HttpStatusCode: %d, Http response error. Error is %s, Task id: %s",
            response->GetHttpStatusCode(), errorDes.c_str(), m_taskId.c_str());
        return FAILED;
    // 2.curl error,need directly retry
    } else {
        errorDes = response->GetErrString();
        errorCode = response->GetErrCode();
        ERRLOG(" Curl error. errorCode: %d errorDes: %s, Task id: %s",
            errorCode, errorDes.c_str(), m_taskId.c_str());
        return FAILED;
    }

    return FAILED;
}

int32_t CNwareClient::ParseErrorBody(
    const std::shared_ptr<ResponseModel> &response, int64_t &errorCode, std::string &errorDes)
{
    if (!Module::JsonHelper::JsonStringToStruct(response->GetBody(), m_rspErrMsg)) {
        ERRLOG("Transfer Response Body failed, %s", m_taskId.c_str());
        return FAILED;
    }
    errorCode = m_rspErrMsg.GetErrCode();
    errorDes = m_rspErrMsg.GetErrString();
    return SUCCESS;
}

int32_t CNwareClient::RefreshSession(const std::shared_ptr<ResponseModel> &response, CNwareRequest &req)
{
    if (response->GetCookies().empty()) {
        ERRLOG("Parse cookie failed, cookie is empty.");
        return FAILED;
    }
    std::string cookieValue = *(response->GetCookies().begin());
    std::vector<std::string> strs;
    (void)boost::split(strs, cookieValue, boost::is_any_of(";"));
    if (!strs.empty()) {
        std::shared_ptr<CNwareSession> sessionPtr = std::make_shared<CNwareSession>(
            req.GetEndpoint(), req.GetPort(), req.GetUserInfo().name, strs[0]);
        m_cnwareSessionCache->AddCNwareSession(sessionPtr);
        return SUCCESS;
    }
    return FAILED;
}

int32_t CNwareClient::Loginout()
{
    std::lock_guard<std::mutex> clock(m_mutexCache);
    std::lock_guard<std::mutex> slock(m_mutexSession);
    if (m_cnwareSessionCache == nullptr) {
        WARNLOG("Pointer m_cnwareSessionCache nullptr!");
        return SUCCESS;
    }
    if (!m_cnwareSessionCache->IsSessionRemain() || m_cnwareSessionPtr == nullptr) {
        return SUCCESS;
    }
    // try to get the last session that this cnware client used
    m_cnwareSessionPtr = m_cnwareSessionCache->GetCNwareSession(std::make_tuple(
        m_cnwareSessionPtr->m_ip, m_cnwareSessionPtr->m_userId));
    if (m_cnwareSessionPtr != nullptr && m_cnwareSessionPtr.use_count() <= NUM_2) {
        INFOLOG("The last client using session, trigger loginOut");
        if (!LoginoutCNware()) {
            WARNLOG("Login out CNware failed! Ip:%s", m_cnwareSessionPtr->m_ip.c_str());
            return FAILED;
        }
        m_cnwareSessionCache->EraseSession(std::make_tuple(m_cnwareSessionPtr->m_ip.c_str(),
            m_cnwareSessionPtr->m_userId.c_str()));
    } else {
        DBGLOG("Session is being used by other clients, skip loginOut");
    }
    return SUCCESS;
}

bool CNwareClient::LoginoutCNware()
{
    int64_t errorCode;
    std::string errorDes;
    for (int retryTimes = RETRY_TIME; retryTimes > 0;) {
        if (m_cnwareSessionPtr == nullptr) {
            ERRLOG("m_cnwareSessionPtr nullptr.");
            return false;
        }
        retryTimes--;
        CNwareRequest req;
        RequestInfo requestInfo;
        requestInfo.m_method = "POST";
        requestInfo.m_resourcePath = "https://{cnwareAddr}:{port}/api/logout";
        requestInfo.m_pathParams["cnwareAddr"] = Utils::CheckIpv6Valid(m_cnwareSessionPtr->m_ip) ? "[" +
            m_cnwareSessionPtr->m_ip + "]" : m_cnwareSessionPtr->m_ip;
        requestInfo.m_pathParams["port"] = m_cnwareSessionPtr->m_port;
        requestInfo.m_headerParams["Content-Type"] = "application/json";
        requestInfo.m_queryParams = {};
        requestInfo.m_auth = req.GetUserInfo();
        requestInfo.m_headerParams["Cookie"] = m_cnwareSessionPtr->m_cookie;
        std::shared_ptr<ResponseModel> response = std::make_shared<ResponseModel>();
        if (CallApi(requestInfo, response, req) != SUCCESS) {
            ERRLOG("Failed to send request. Ip: %s", m_cnwareSessionPtr->m_ip.c_str());
            continue;       // cert error, CallApi will return FALSE, this scene need judge err code.
        }
        if (response.get() == nullptr) {
            ERRLOG("Return response is empty.Task id: %s", m_taskId.c_str());
            return false;
        }
        int32_t iRet = ParseResonse(response, errorCode, errorDes, req);
        if (iRet == SUCCESS) {
            INFOLOG("Unlogin. Ip: %s", m_cnwareSessionPtr->m_ip.c_str());
            return true;
        }
    }
    return false;
}

int32_t CNwareClient::GetVersionInfo(CNwareRequest &req, ApplicationEnvironment &returnEnv)
{
    std::shared_ptr<ResponseModel> response = GetVersionInfo(req);
    if (response == nullptr) {
        ERRLOG("Response get nullptr.");
        return FAILED;
    }
    if (!SetVersionInfoResonse(response, returnEnv)) {
        ERRLOG("Failed to send get version info request.");
        return FAILED;
    }
    return SUCCESS;
}

std::shared_ptr<ResponseModel> CNwareClient::GetVersionInfo(CNwareRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "api/identity/about/us/product";
    RequestInfo requestInfo;
    requestInfo.m_method = "Get";
    requestInfo.m_queryParams = {};
    std::shared_ptr<ResponseModel> response = std::make_shared<ResponseModel>();
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        return nullptr;
    }

    return response;
}

int32_t CNwareClient::GetResource(CNwareRequest &req, std::shared_ptr<ResponseModel> response,
    const QueryByPage &pageInfo, CNwareType parentType)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return FAILED;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    RequestInfo requestInfo;
    requestInfo.m_method = "Get";
    requestInfo.m_queryParams["needPoolName"] = "True";
    requestInfo.m_queryParams["needClusterName"] = "True";
    requestInfo.m_queryParams["needHostName"] = "True";
    requestInfo.m_queryParams["needDiskInfo"] = "True";
    requestInfo.m_queryParams["needInterface"] = "True";
    requestInfo.m_queryParams["size"] = std::to_string(pageInfo.pageSize);
    requestInfo.m_queryParams["start"] = std::to_string(pageInfo.pageNo);
    if (!req.GetDomain().empty()) {
        requestInfo.m_queryParams["nameLike"] = req.GetDomain();
    }
    SetResourceParams(requestInfo, req, parentType);
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        return FAILED;
    }
    return SUCCESS;
}

std::shared_ptr<CNwareResponse> CNwareClient::CreateSnapshot(CreateSnapshotRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    req.url = "/api/compute/custom/domains/{domainId}/snapshots";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();

    if (req.CustomDomainSnapshotReqToJson() != SUCCESS) {
        ERRLOG("Struct of snap info to Json body failed!");
        return nullptr;
    }

    std::string errorDes;
    int64_t errorCode;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Create CreateSnapshotResponse pointer failed.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send CreateSnapshot request.");
        return nullptr;
    }

    if (!response->Serial()) {
        ERRLOG("Get CNware CreateSnapshot taskId fail");
        return nullptr;
    }
    DBGLOG("Get CNware CreateSnapshot taskId success, domainId is: %s!", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<InquiriesSnapshotResponse> CNwareClient::InquiriesSnapshot(InquiriesSnapshotRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    req.url = "/api/compute/custom/domainBackup/{taskId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "Get";
    requestInfo.m_pathParams["taskId"] = req.GetTaskId();
    std::string errorDes;
    int64_t errorCode;
    std::shared_ptr<InquiriesSnapshotResponse> response = std::make_shared<InquiriesSnapshotResponse>();
    if (response == nullptr) {
        ERRLOG("Create InquiriesSnapshotResponse pointer failed.");
        return nullptr;
    }

    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send InquiriesSnapshot request. errorCode is %lld, errorDes is %s.",
            errorCode, errorDes.c_str());
        return nullptr;
    }

    if (!response->Serial()) {
        ERRLOG("Get CNware InquiriesSnapshot info fail");
        return nullptr;
    }
    DBGLOG("Get CNware InquiriesSnapshot info success, domainId is: %s!", req.GetTaskId().c_str());
    return response;
}

std::shared_ptr<ResponseModel> CNwareClient::DelCephSnapshots(CNwareRequest &req, const std::string &snapId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    req.url = "/api/compute/custom/domains/del/snapshot/{taskId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "Post";
    requestInfo.m_pathParams["taskId"] = snapId;
    std::string errorDes;
    int64_t errorCode;
    std::shared_ptr<ResponseModel> response = std::make_shared<ResponseModel>();
    if (response == nullptr) {
        ERRLOG("Create ResponseModel pointer failed.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send DelCephSnapshots request. errorCode is %lld, errorDes is %s.",
            errorCode, errorDes.c_str());
        return nullptr;
    }
    WARNLOG("++++++++++++++ %s", response->GetBody().c_str());
    DBGLOG("Get CNware DelCephSnapshots info success, domainId is: %s!", snapId.c_str());
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::AttachVolumeSnapshot(AttachVolumeSnapshotRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Attach disk to vm %s, %s", req.GetDomainId().c_str(), req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/devices/disk/mount";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();

    if (req.MountDiskReqToJson() != SUCCESS) {
        ERRLOG("Struct of MountDiskReq to Json body failed!");
        return nullptr;
    }

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    DBGLOG("Get CNware AttachVolumeSnapshot taskId success, domainId is: %s!", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<AddEmptyDiskResponse> CNwareClient::AddEmptyDisk(AddEmptyDiskRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    req.url = "/api/compute/domains/{domainId}/devices/disk/add";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
    if (req.GetAddDiskReqString() != SUCCESS) {
        ERRLOG("Format request body failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    std::shared_ptr<AddEmptyDiskResponse> response = std::make_shared<AddEmptyDiskResponse>();
    if (response == nullptr) {
        ERRLOG("AddEmptyDiskResponse pointer failed.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send AddEmptyDisk request. errorCode is %lld, errorDes is %s.",
            errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Get CNware AddEmptyDisk taskId fail");
        return nullptr;
    }
    DBGLOG("Get CNware AddEmptyDisk taskId success, domainId is: %s!", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<AssociateHostResponse> CNwareClient::AssociateHost(AssociateHostRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    req.url = "/api/storage/stores/{storeId}/hosts/rel";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["storeId"] = req.GetStoreId();
    if (req.AssociateHostRequestToJson() != SUCCESS) {
        ERRLOG("Struct of Associate Host to json body failed!");
        return nullptr;
    }

    std::string errorDes;
    int64_t errorCode;
    std::shared_ptr<AssociateHostResponse> response = std::make_shared<AssociateHostResponse>();
    if (response == nullptr) {
        ERRLOG("AssociateHostResponse pointer failed. ");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send AssociateHost request. errorCode is %lld, errorDes is %s.",
            errorCode, errorDes.c_str());
        return nullptr;
    }

    if (!response->Serial()) {
        ERRLOG("Get CNware AssociateHost hostIdList fail");
        return nullptr;
    }
    DBGLOG("Get CNware AssociateHost hostId success, hostIdList is: %s!", req.GetStoreId().c_str());
    return response;
}

std::shared_ptr<RemoveAssociateHostResponse> CNwareClient::RemoveAssociateHost(RemoveAssociateHostRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    req.url = "/api/storage/stores/{storeId}/hosts/rel/remove";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["storeId"] = req.GetStoreId();
    if (req.RemoveAssociateHostRequestToJson() != SUCCESS) {
        ERRLOG("Struct of RemoveAssociate Host to json body failed!");
        return nullptr;
    }
    requestInfo.m_body = req.GetReqBody();
    std::string errorDes;
    int64_t errorCode;
    std::shared_ptr<RemoveAssociateHostResponse> response = std::make_shared<RemoveAssociateHostResponse>();
    if (response == nullptr) {
        ERRLOG("RemoveAssociateHostResponse pointer failed.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send RemoveAssociateHost request. errorCode is %lld, errorDes is %s.",
            errorCode, errorDes.c_str());
        return nullptr;
    }

    if (!response->Serial()) {
        ERRLOG("Get CNware RemoveAssociateHost hostIdList fail");
        return nullptr;
    }
    DBGLOG("Get CNware RemoveAssociateHost hostId success, hostIdList is: %s!", req.GetStoreId().c_str());
    return response;
}

std::shared_ptr<StoragePoolResponse> CNwareClient::GetStoragePoolInfo(CNwareRequest &req,
    const std::string &hostId, const std::string &poolId, const int32_t &start, const int32_t &size)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    req.url = "/api/storage/storagePools";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_queryParams["hostId"] = hostId;
    requestInfo.m_queryParams["size"] = std::to_string(size);
    requestInfo.m_queryParams["start"] = std::to_string(start);
    requestInfo.m_queryParams["offset"] = std::to_string((start - 1) * size);
    requestInfo.m_queryParams["order"] = "desc";
    if (poolId != "") {
        requestInfo.m_queryParams["id"] = poolId;
    }
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<StoragePoolResponse> response = std::make_shared<StoragePoolResponse>();
    if (response == nullptr) {
        ERRLOG("StoragePoolResponse pointer failed.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send StoragePoolResponse request. errorCode is %lld, errorDes is %s.",
            errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serial CNware StoragePoolResponse fail");
        return nullptr;
    }
    DBGLOG("Get CNware StoragePoolResponse success, hostIdList is: %s!", hostId.c_str());
    return response;
}

std::shared_ptr<PortGroupResponse> CNwareClient::GetPortGroupInfo(CNwareRequest &req,
    const std::string &hostId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    req.url = "/api/compute/hosts/{hostId}/portGroups";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["hostId"] = hostId;
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<PortGroupResponse> response = std::make_shared<PortGroupResponse>();
    if (response == nullptr) {
        ERRLOG("PortGroupResponse pointer failed.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send PortGroupResponse request. errorCode is %lld, errorDes is %s.",
            errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serial CNware PortGroupResponse fail");
        return nullptr;
    }
    DBGLOG("Get CNware PortGroupResponse success, hostIdList is: %s!", hostId.c_str());
    return response;
}

void CNwareClient::SetResourceParams(RequestInfo &requestInfo, CNwareRequest &req, CNwareType &parentType)
{
    switch (parentType) {
        case CNwareType::All:
            break;
        case CNwareType::Pool:
            requestInfo.m_queryParams["poolId"] = req.GetResourceId();
            break;
        case CNwareType::Cluster:
            requestInfo.m_queryParams["clusterId"] = req.GetResourceId();
            break;
        case CNwareType::Host:
            requestInfo.m_queryParams["hostId"] = req.GetResourceId();
            break;
        default:
            INFOLOG("Set Resouce! Type is %d", parentType);
            return;
    }
}

int32_t CNwareClient::SendRequest(std::shared_ptr<ResponseModel> response, CNwareRequest &req,
    RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode)
{
    requestInfo.m_resourcePath = "https://{cnwareAddr}:{port}/" + req.url;
    std::string address = req.GetEndpoint();
    requestInfo.m_pathParams["cnwareAddr"] = Utils::CheckIpv6Valid(address) ? "[" + address + "]" : address;
    requestInfo.m_pathParams["port"] = req.GetPort();
    requestInfo.m_headerParams["Content-Type"] = "application/json";
    requestInfo.m_body = req.GetBody();
    requestInfo.m_auth = req.GetUserInfo();
    int32_t retryNum = 0;
    if (!SetSession(req, requestInfo)) {
        if (Login(req, errorCode, errorDes) != SUCCESS || !SetSession(req, requestInfo)) {
            ERRLOG("Retry login and set session failed. Ip: %s", req.GetEndpoint().c_str());
            return FAILED;
        }
    }
    while (retryNum < RETRY_TIME) {
        INFOLOG("send request for %d time to %s, %s", (retryNum + 1),
            WIPE_SENSITIVE(req.url).c_str(), m_taskId.c_str());
        if (DoSendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
            WARNLOG("Do send request failed. Url: %s, http status: %d, errCode: %d, error message: %d, %s",
                requestInfo.m_resourcePath.c_str(), response->GetHttpStatusCode(),
                errorCode, errorDes.c_str(), m_taskId.c_str());
            if (NeedRetry(response, req, requestInfo, errorDes, errorCode)) {
                DelayTimeSendRequest();
                ++retryNum;
            } else {
                break;
            }
        } else {
            return SUCCESS;
        }
    }
    ERRLOG("send request failed.");
    if (!Module::JsonHelper::JsonStringToStruct(response->GetBody(), m_error)) {
        ERRLOG("Transfer CNwareError failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return FAILED;
}

bool CNwareClient::NeedRetry(std::shared_ptr<ResponseModel> response, CNwareRequest &req,
    RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode)
{
    if (response->GetHttpStatusCode() == CNWARE_AUTH_FAILED &&
        ResponseSuccessHandle(req, requestInfo, response, errorDes, errorCode) == SUCCESS) {
        return true;
    }
    if (response->GetHttpStatusCode() == CNWARE_JOB_CONFLICT && (errorCode == UNFINISHED_JOB_EXISTS ||
        errorCode == REFRESH_POOL_JOB || errorCode== REFRESH_POOL_JOB_CONFLICT)) {
        WARNLOG(" Url: %s, http status: %d, errCode: %d, error message: %d, request conflicts. %s",
            requestInfo.m_resourcePath.c_str(), CNWARE_JOB_CONFLICT,
            UNFINISHED_JOB_EXISTS, errorDes.c_str(), m_taskId.c_str());
        return true;
    }
    if (response->GetErrCode() != 0 || errorCode == UNSUPPORT_LUN) {
        WARNLOG(" Url: %s, http status: %d, errCode: %d, error message: %d, request conflicts. %s",
            requestInfo.m_resourcePath.c_str(), response->GetHttpStatusCode(),
            response->GetErrCode(), response->GetErrString().c_str(), m_taskId.c_str());
        return true;
    }
    return false;
}

CNwareError CNwareClient::GetErrorCode()
{
    return m_error;
}

int32_t CNwareClient::DoSendRequest(std::shared_ptr<ResponseModel> response, CNwareRequest &req,
    RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode)
{
    if (CallApi(requestInfo, response, req) != SUCCESS) {
        ERRLOG("Failed to send request. Task id: %s", m_taskId.c_str());
        ParseResonse(response, errorCode, errorDes, req);
        return FAILED;
    }
    int32_t iRet = ParseResonse(response, errorCode, errorDes, req);
    if (iRet == SUCCESS) {
        return iRet;
    }
    return FAILED;
}

void CNwareClient::DelayTimeSendRequest()
{
    auto now = std::chrono::steady_clock::now();
    while ((double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -
        now).count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den) <
        m_retryIntervalTime) {
        INFOLOG("Waiting for cnware client ... ");
        sleep(1);
    }
    return;
}

int32_t CNwareClient::ResponseSuccessHandle(CNwareRequest &req, RequestInfo &requestInfo,
    std::shared_ptr<ResponseModel> &response, std::string &errorDes, int64_t &errorCode)
{
    if (Login(req, errorCode, errorDes) != SUCCESS || !SetSession(req, requestInfo)) {
        ERRLOG("Retry login and set session failed. Ip: %s", req.GetEndpoint().c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool CNwareClient::SetVersionInfoResonse(const std::shared_ptr<ResponseModel> &response,
    ApplicationEnvironment &returnEnv)
{
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Request VersionInfo return failed. Task id: %s", m_taskId.c_str());
        return false;
    }
    CNwareVersionInfo resBody;
    if (!Module::JsonHelper::JsonStringToStruct(response->GetBody(), resBody)) {
        ERRLOG("Transfer CNwareVersionInfo failed, %s", m_taskId.c_str());
        return false;
    }
    Json::Value body;
    body["productVersion"] = resBody.m_productVersion;
    Json::FastWriter writer;
    returnEnv.__set_extendInfo(writer.write(body));
    return true;
}

bool CNwareClient::SetSession(const CNwareRequest &request, RequestInfo &requestInfo)
{
    std::lock_guard<std::mutex> clock(m_mutexCache);
    std::lock_guard<std::mutex> slock(m_mutexSession);
    if (m_cnwareSessionCache == nullptr) {
        ERRLOG("SetSession pointer error. Task id: %s", m_taskId.c_str());
        return false;
    }
    m_cnwareSessionPtr = m_cnwareSessionCache->GetCNwareSession(std::make_tuple(
        request.GetEndpoint(), request.GetUserInfo().name));
    if (m_cnwareSessionPtr == nullptr) {
        ERRLOG("Cache Session pointer error. Task id: %s", m_taskId.c_str());
        return false;
    }
    requestInfo.m_headerParams["Cookie"] = m_cnwareSessionPtr->m_cookie;
    return true;
}

bool CNwareClient::BuildLoginBody(std::string &body)
{
    LoginRequestBody loginBody;
    loginBody.m_user = m_auth.authkey;
    loginBody.m_pwd = m_auth.authPwd;
    std::string reqBody;
    if (!Module::JsonHelper::StructToJsonString(loginBody, reqBody)) {
        ERRLOG("Convert LoginRequestBody to json string failed");
        return false;
    }
    body = reqBody;
    return true;
}

std::string CNwareClient::UrlEncode(const std::string& str)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (auto c : str) {
        // Keep alphanumeric and other safe characters intact
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            // Any other characters are percent-encoded
            escaped << std::uppercase;
            escaped << '%' << std::setw(NUM_2) << int((unsigned char) c);
            escaped << std::nouppercase;
        }
    }

    return escaped.str();
}

std::shared_ptr<CheckNameUniqueResponse> CNwareClient::CheckNameUnique(CheckNameUniqueRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/chk/unique";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_queryParams["name"] = UrlEncode(req.GetName());
    requestInfo.m_queryParams["domainname"] = req.GetDomainName();

    std::shared_ptr<CheckNameUniqueResponse> response = std::make_shared<CheckNameUniqueResponse>();
    if (response == nullptr) {
        ERRLOG("Create CheckNameUniqueResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Check name unique success!");
    return response;
}

std::shared_ptr<VolExistResponse> CNwareClient::StorageVolumesExist(CNwareRequest &request,
    const std::string &volname)
{
    if (!CheckParams(request)) {
        ERRLOG("Failed to check param. Ip: %s", request.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("StorageVolumesExist CNware client ip: %s", request.GetEndpoint().c_str());
    request.url = "/api/storage/storagePools/{storagePoolId}/volName/exist";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["storagePoolId"] = request.GetDomain();
    requestInfo.m_queryParams["name"] = UrlEncode(volname);

    std::shared_ptr<VolExistResponse> response = std::make_shared<VolExistResponse>();
    if (response == nullptr) {
        ERRLOG("Create VolExistResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, request, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Check name unique success!");
    return response;
}

std::shared_ptr<GetVMInfoResponse> CNwareClient::GetVMInfo(GetVMInfoRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
    requestInfo.m_queryParams["domainFlag"] = CNWARE_DB_VMINFO;
    requestInfo.m_queryParams["includeHostRsp"] = "true";
    requestInfo.m_queryParams["needClusterName"] = "true";
    requestInfo.m_queryParams["needHostName"] = "true";
    requestInfo.m_queryParams["needPoolName"] = "true";

    std::shared_ptr<GetVMInfoResponse> response = std::make_shared<GetVMInfoResponse>();
    if (response == nullptr) {
        ERRLOG("Create GetVMInfoResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request, domainId is: %s.", req.GetDomainId().c_str());
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Get CNware client info success, domainId is: %s!", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<GetVMDiskInfoResponse> CNwareClient::GetVMDiskInfo(GetVMDiskInfoRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/domainDiskInfo";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();

    std::shared_ptr<GetVMDiskInfoResponse> response = std::make_shared<GetVMDiskInfoResponse>();
    if (response == nullptr) {
        ERRLOG("Create GetVMDiskInfoResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request, domainId is: %s.", req.GetDomainId().c_str());
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Get CNware disk info on vm success, domainId is: %s!", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<GetDiskInfoOnStorageResponse> CNwareClient::GetDiskInfoOnStorage(GetDiskInfoOnStorageRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/storageVolumes/{volId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["volId"] = req.GetVolId();

    std::shared_ptr<GetDiskInfoOnStorageResponse> response = std::make_shared<GetDiskInfoOnStorageResponse>();
    if (response == nullptr) {
        ERRLOG("Create GetDiskInfoOnStorageResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS &&
        response->GetStatusCode() != static_cast<uint32_t>(Module::SC_CONFLICT)) {
        ERRLOG("Failed to send request, domainId is: %s.", req.GetVolId().c_str());
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Get CNware disk info on storage success, VolId is: %s!", req.GetVolId().c_str());
    return response;
}

std::shared_ptr<StorageVolumeResponse> CNwareClient::GetStorageVolume(CNwareRequest &req,
    const std::string &storageId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/storagePools/{storageId}/storageVolumes";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["storageId"] = storageId;

    std::shared_ptr<StorageVolumeResponse> response = std::make_shared<StorageVolumeResponse>();
    if (response == nullptr) {
        ERRLOG("Create GetDiskInfoOnStorageResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS &&
        response->GetStatusCode() != static_cast<uint32_t>(Module::SC_CONFLICT)) {
        ERRLOG("Failed to send request, storageId is: %s.", storageId.c_str());
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("GGetStorageVolume success, storageId is: %s!", storageId.c_str());
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::DetachDiskOnVM(DetachDiskOnVMRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/devices/by_bus_dev/{busDev}/deleteType/{deleteType}";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
    requestInfo.m_pathParams["busDev"] = req.GetBusDev();
    requestInfo.m_pathParams["deleteType"] = "0";

    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Create DetachDiskOnVMResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request, domainId is: %s.", req.GetDomainId().c_str());
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Delete disk for CNware client domainId: %s success!", req.GetDomainId().c_str());
    return response;
}

// 卸载和删除磁盘的返回消息体一致，故都使用DeleteDiskResponse
std::shared_ptr<DeleteDiskResponse> CNwareClient::UninstallOrDeleteDiskVolId(UninstallOrDeleteDiskRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/vols/{volId}/deleteType/{deleteType}";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
    requestInfo.m_pathParams["volId"] = req.GetVolId();
    requestInfo.m_pathParams["deleteType"] = req.GetDeleteType();

    std::shared_ptr<DeleteDiskResponse> response = std::make_shared<DeleteDiskResponse>();
    if (response == nullptr) {
        ERRLOG("Create DeleteDiskResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request, domainId is: %s, volId is: %s!", req.GetDomainId().c_str(),
            req.GetVolId().c_str());
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Uninstall Or delete disk success, domainId is: %s, volId is: %s!", req.GetDomainId().c_str(),
        req.GetVolId().c_str());

    return response;
}

std::shared_ptr<ResponseModel> CNwareClient::DeleteSnapshotVolume(CNwareRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/custom/domains/del/snapshot/{taskId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["taskId"] = m_taskId;

    std::shared_ptr<ResponseModel> response = std::make_shared<DeleteDiskResponse>();
    if (response == nullptr) {
        ERRLOG("Create ResponseModel pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Delete snapshot success!");
    return response;
}

std::shared_ptr<BuildNewVMResponse> CNwareClient::BuildNewClient(BuildNewVMRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    if (req.AddDomainRequestToJson() != SUCCESS) {
        ERRLOG("Struct of new VM info to Json body failed!");
        return nullptr;
    }

    std::shared_ptr<BuildNewVMResponse> response = std::make_shared<BuildNewVMResponse>();
    if (response == nullptr) {
        ERRLOG("Create BuildNewVMResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("request of building a new VM send success!");
    return response;
}

std::shared_ptr<MigrateHostsResponse> CNwareClient::MigrateHosts(MigrateHostsRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/hosts/migrate";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    if (req.MigrateHostsReqToJson() != SUCCESS) {
        ERRLOG("Struct of migratr hosts resquest to Json body failed!");
        return nullptr;
    }

    std::string errorDes;
    int64_t errorCode;
    std::shared_ptr<MigrateHostsResponse> response = std::make_shared<MigrateHostsResponse>();
    if (response == nullptr) {
        ERRLOG("Create MigrateHostsResponse pointer failed.");
        return nullptr;
    }

    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }

    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("request of migrate hosts  send success!");
    return response;
}

std::shared_ptr<CheckShareFileSysConfUniqueResponse> CNwareClient::CheckShareFileSysConfUnique(
    CheckShareFileSysConfUniqueRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/storages/chk/unique";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_queryParams["directory"] = req.GetDirectory();
    requestInfo.m_queryParams["id"] = req.GetId();
    requestInfo.m_queryParams["tpye"] = req.GetType();

    std::shared_ptr<CheckShareFileSysConfUniqueResponse> response =
        std::make_shared<CheckShareFileSysConfUniqueResponse>();
    if (response == nullptr) {
        ERRLOG("Create CheckShareFileSysConfUniqueResponse pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    DBGLOG("Send request if checking shared file system conf unique success!");
    return response;
}

std::shared_ptr<GetHostInfoResponse> CNwareClient::GetHostInfo(CNwareRequest &req,
    const std::string & hostId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/hosts/{hostId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["hostId"] = hostId;

    std::shared_ptr<GetHostInfoResponse> response = std::make_shared<GetHostInfoResponse>();
    if (response == nullptr) {
        ERRLOG("Create AddNfs Response pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return response;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    return response;
}

std::shared_ptr<StoreScanResponse> CNwareClient::AddNfs(AddNfsRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/stores/nfs";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";

    std::shared_ptr<StoreScanResponse> response = std::make_shared<StoreScanResponse>();
    if (response == nullptr) {
        ERRLOG("Create AddNfs Response pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    return response;
}

std::shared_ptr<NfsStoreResponse> CNwareClient::GetNfsInfo(NfsRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/stores";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";

    std::shared_ptr<NfsStoreResponse> response = std::make_shared<NfsStoreResponse>();
    if (response == nullptr) {
        ERRLOG("Create AddNfs Response pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    return response;
}

std::shared_ptr<StoreScanResponse> CNwareClient::AddNfsStorage(AddNfsStorageRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/storagePools/nfs";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";

    std::shared_ptr<StoreScanResponse> response = std::make_shared<StoreScanResponse>();
    if (response == nullptr) {
        ERRLOG("Create AddNfsStorage Response pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    return response;
}

std::shared_ptr<StoreScanResponse> CNwareClient::ScanNfsStorage(CNwareRequest &req,
    const std::string &storeId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/stores/{storeId}/scan";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["storeId"] = storeId;

    std::shared_ptr<StoreScanResponse> response = std::make_shared<StoreScanResponse>();
    if (response == nullptr) {
        ERRLOG("Create StoreScanResponse Response pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    return response;
}

std::shared_ptr<StoreScanResponse> CNwareClient::RefreshStoragePool(CNwareRequest &req,
    const std::string &poolId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/storagePools/{poolId}/refresh";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["poolId"] = poolId;

    std::shared_ptr<StoreScanResponse> response = std::make_shared<StoreScanResponse>();
    if (response == nullptr) {
        ERRLOG("Create StoreScanResponse Response pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    return response;
}

std::shared_ptr<StoreResourceResponse> CNwareClient::GetNfsStorageResource(StoreResourceRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/storage/stores/resources";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";

    std::shared_ptr<StoreResourceResponse> response = std::make_shared<StoreResourceResponse>();
    if (response == nullptr) {
        ERRLOG("Create AddNfsStorage Response pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::DeleteVM(DeleteVMRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Delete vm, endpoint: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/{deleteType}";
    RequestInfo requestInfo;
    requestInfo.m_method = "DELETE";
    requestInfo.m_pathParams["deleteType"] = std::to_string(static_cast<int32_t>(req.GetDeleteType()));
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
    requestInfo.m_queryParams["isNowDo"] = req.GetIsNowDo() ? "true" : "false";

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Send delete vm request success! domain id: %s", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::PowerOnVM(CNwareRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Poweron vm, endpoint: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/act/start";
    RequestInfo requestInfo;
    requestInfo.m_method = "PATCH";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
 
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Send poweron vm request success! domain id: %s", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::PowerOffVM(CNwareRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Poweroff vm, endpoint: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/act/shutdown";
    RequestInfo requestInfo;
    requestInfo.m_method = "PATCH";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Send poweroff request success! domain id: %s", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<QueryTaskResponse> CNwareClient::QueryTask(QueryTaskRequest &req, const bool &needDetail)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    if (req.GetTaskId().empty()) {
        ERRLOG("Query cnware interface with empty task, info : %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Query cnware interface task %s info : %s", req.GetTaskId().c_str(), req.GetEndpoint().c_str());
    req.url = needDetail ? "/api/notify/tasks/{taskId}/info" : "/api/notify/tasks/{taskId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["taskId"] = req.GetTaskId();

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<QueryTaskResponse> response = std::make_shared<QueryTaskResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Query  request success! Task id: %s", req.GetTaskId().c_str());
    return response;
}

std::shared_ptr<QueryVMTaskResponse> CNwareClient::QueryVMTasks(CNwareRequest &req, const std::string &domainId,
    const std::string &status, const int32_t &start, const int32_t &size)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Query cnware vm %s task info : %s", domainId.c_str(), req.GetEndpoint().c_str());
    req.url = "/api/notify/tasks";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_queryParams["domainId"] = domainId;
    requestInfo.m_queryParams["status"] = status;
    requestInfo.m_queryParams["size"] = std::to_string(size);
    requestInfo.m_queryParams["offset"] = std::to_string((start - 1) * size);
    requestInfo.m_queryParams["start"] = std::to_string(start);
    requestInfo.m_queryParams["order"] = "desc";

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<QueryVMTaskResponse> response = std::make_shared<QueryVMTaskResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Query MTasks success! domain id: %s", domainId.c_str());
    return response;
}

std::shared_ptr<DeleteDiskOnStorageResponse> CNwareClient::DeleteDiskOnStorage(DelDiskOnStorageRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Delete disk %s on storage task info : %s", req.GetVolId().c_str(), req.GetEndpoint().c_str());
    req.url = "/api/storage/storageVolumes/{volId}/delete";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["volId"] = req.GetVolId();
    DeleteStorageVolumeReq reqDel;
    req.SetDelDiskOnStorageRequest(reqDel);
    if (req.DelDiskOnStorageRequestToJson() != SUCCESS) {
        ERRLOG("Struct of DelDiskOnStorageRequest to Json body failed!");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<DeleteDiskOnStorageResponse> response = std::make_shared<DeleteDiskOnStorageResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Delete disk on storage task start success! vol id: %s", req.GetVolId().c_str());
    return response;
}

std::shared_ptr<GetVMListResponse> CNwareClient::GetVMList(GetVMListRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Get vm list: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_queryParams["name"] = UrlEncode(req.GetQueryName());
    requestInfo.m_queryParams["needDiskInfo"] = "true";
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<GetVMListResponse> response = std::make_shared<GetVMListResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Get VM list success!");
    return response;
}

std::shared_ptr<GetVMInterfaceInfoResponse> CNwareClient::GetVMInterfaceInfo(GetVMInterfaceInfoRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Get vm(%s) interface info : %s", req.GetDomainId().c_str(), req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/domainInterfaceInfo";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<GetVMInterfaceInfoResponse> response = std::make_shared<GetVMInterfaceInfoResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Get vm(%s) interface info success!", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::ModifyVMBoots(ModifyBootsRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Modify vm(%s) boot order : %s", req.GetDomainId().c_str(), req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/boot";
    if (req.UpdateDomainBootRequestToJson() != SUCCESS) {
        ERRLOG("Struct of UpdateDomainBootRequest to Json body failed!");
        return nullptr;
    }

    RequestInfo requestInfo;
    requestInfo.m_method = "Post";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Send modify vm(%s) boot order req success!", req.GetDomainId().c_str());
    return response;
}

// 迁移虚拟机磁盘，MigrationRequest.AddMigVol添加迁移信息，SetMigReq设置原hostId
std::shared_ptr<CNwareResponse> CNwareClient::MigrateVols(MigrationRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("MigrateVols vm(%s).", req.GetDomainId().c_str());
    req.url = "/api/compute/domains/{domainId}/migrate/to/storage_pools";

    RequestInfo requestInfo;
    requestInfo.m_method = "Patch";
    requestInfo.m_pathParams["domainId"] = req.GetDomain();

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create MigrateVols response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send MigrateVols request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Send MigrateVols vm(%s) req success!", req.GetDomainId().c_str());
    return response;
}

std::shared_ptr<VswitchsResponse> CNwareClient::GetVswitchInfo(CNwareRequest &req, const std::string & hostId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("GetVswitchInfo from: %s", hostId.c_str());
    req.url = "/api/network/network/host/{hostId}/vswitchs";

    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["hostId"] = hostId;

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<VswitchsResponse> response = std::make_shared<VswitchsResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create MigrateVols response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send MigrateVols request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Send GetVswitchInfo(%s) req success!", hostId.c_str());
    return response;
}

std::shared_ptr<DelStoragePoolResponse> CNwareClient::DelStoragePool(DelStoragePoolRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }

    INFOLOG("Delete storage pool %s task info : %s", req.GetPoolId().c_str(), req.GetEndpoint().c_str());
    req.url = "/api/storage/storagePools/{storagePoolId}/delete";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["storagePoolId"] = req.GetPoolId();
    DeleteStoragePoolReq reqDel;
    req.SetDelStoragePoolRequest(reqDel);
    if (req.DelStoragePoolRequestToJson() != SUCCESS) {
        ERRLOG("Struct of DelDiskOnStorageRequest to Json body failed!");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<DelStoragePoolResponse> response = std::make_shared<DelStoragePoolResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Delete storage pool(%s) task start success!", req.GetPoolId().c_str());
    return response;
}

std::shared_ptr<StoragePoolExResponse> CNwareClient::GetStoragePool(CNwareRequest &req, const std::string &poolId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    if (poolId.empty()) {
        ERRLOG("Get cnware interface with empty poolId, info : %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Get storage pool %s task info : %s", poolId.c_str(), req.GetEndpoint().c_str());
    req.url = "/api/storage/storagePools/{storagePoolId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["storagePoolId"] = poolId;

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<StoragePoolExResponse> response = std::make_shared<StoragePoolExResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create GetStoragePool response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send storagePool request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize GetStoragePool response body failed!");
        return nullptr;
    }
    INFOLOG("Get storage pool(%s) task start success!", poolId.c_str());
    return response;
}

std::shared_ptr<AddDiskResponse> CNwareClient::AddDisk(AddDiskRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
 
    INFOLOG("Add disk to vm: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/devices/disk";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
    if (req.BuildRequestBodyString() != SUCCESS) {
        ERRLOG("Struct of DelDiskOnStorageRequest to Json body failed!");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<AddDiskResponse> response = std::make_shared<AddDiskResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Add disk to vm, send request success!");
    return response;
}

std::shared_ptr<QueryHostListResponse> CNwareClient::QueryHostList(QueryHostListRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
 
    INFOLOG("Query host info: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/hosts";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_queryParams["hostIds"] = req.GetHostIdsString();

    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<QueryHostListResponse> response = std::make_shared<QueryHostListResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Query host info success!");
    return response;
}

std::shared_ptr<UpdateVMDiskResponse> CNwareClient::UpdateDomainDiskMetadata(UpdateVMDiskRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Update domain disk metadata: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/devices/disk/{busDev}";
    RequestInfo requestInfo;
    requestInfo.m_method = "PUT";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
    requestInfo.m_pathParams["busDev"] = req.GetBusDev();
    if (req.BuildRequestBodyString() != SUCCESS) {
        ERRLOG("Build request body failed!");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<UpdateVMDiskResponse> response = std::make_shared<UpdateVMDiskResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }
    INFOLOG("Update domain disk metadata success!");
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::HealthVm(HealthRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Health Request ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/health";
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    if (req.BuildRequestBodyString() != SUCCESS) {
        ERRLOG("Build request body failed!");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create HealthVm response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send HealthVm. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize HealthVm response body failed!");
        return nullptr;
    }
    INFOLOG("Health Vm success!");
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::ModifyInterface(InterfaceRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Health Request ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/devices/interface/{mac}";
    RequestInfo requestInfo;
    requestInfo.m_method = "PUT";
    requestInfo.m_pathParams["domainId"] = req.GetDomain();
    requestInfo.m_pathParams["mac"] = req.GetMac();
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create Modify Interface response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send Modify Interface. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize Modify Interface response body failed!");
        return nullptr;
    }
    INFOLOG("Modify Interface success!");
    return response;
}

std::shared_ptr<CNwareResponse> CNwareClient::AddNetworkCard(AddNetworkCardRequest &req)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Add NetworkCard Request ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/domains/{domainId}/devices/interface";
    if (req.AddNetworkCardReqToJsonString() != SUCCESS) {
        ERRLOG("Build request body failed!");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_pathParams["domainId"] = req.GetDomainId();
    std::string errorDes;
    int64_t errorCode = 0;
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    if (response == nullptr) {
        ERRLOG("Failed to create Add NetworkCard response handler.");
        return nullptr;
    }
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send Add NetworkCard. errorCode: %d, errorDes: %s", errorCode, errorDes.c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize Add NetworkCard response body failed!");
        return nullptr;
    }
    INFOLOG("Add NetworkCard success!");
    return response;
}

std::shared_ptr<GetHostResourceStatResponse> CNwareClient::GetHostResourceStat(CNwareRequest &req,
    const std::string & hostId)
{
    if (!CheckParams(req)) {
        ERRLOG("Failed to check param. Ip: %s", req.GetEndpoint().c_str());
        return nullptr;
    }
    INFOLOG("Login to CNware client ip: %s", req.GetEndpoint().c_str());
    req.url = "/api/compute/stat/resources/host/{hostId}";
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_pathParams["hostId"] = hostId;

    std::shared_ptr<GetHostResourceStatResponse> response = std::make_shared<GetHostResourceStatResponse>();
    if (response == nullptr) {
        ERRLOG("Create AddNfs Response pointer failed.");
        return nullptr;
    }
    std::string errorDes;
    int64_t errorCode;
    if (SendRequest(response, req, requestInfo, errorDes, errorCode) != SUCCESS) {
        ERRLOG("Failed to send request.");
        response = nullptr;
        return response;
    }
    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        response = nullptr;
        return nullptr;
    }
    return response;
}
}
