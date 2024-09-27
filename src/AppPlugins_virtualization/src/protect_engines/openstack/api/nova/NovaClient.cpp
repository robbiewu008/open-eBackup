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
#include "NovaClient.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"

namespace {
const std::string MODULE_NAME = "NovaClient";
using Defer = std::shared_ptr<void>;
}

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

bool NovaClient::CheckParams(ModelBase &model)
{
    if (!model.UserInfoIsSet()) {
        ERRLOG("User info does not set.");
        return false;
    }
    return true;
}

std::shared_ptr<GetServerDetailsResponse> NovaClient::GetServerDetails(GetServerDetailsRequest &request)
{
    if (!CheckParams(request) || !request.ServerIdIsSet()) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{endpoint}/servers/{serverId}";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_pathParams["serverId"] = request.GetServerId();
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetServerDetailsResponse> response = std::make_shared<GetServerDetailsResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get server details response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Get server details serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<GetProjectServersResponse> NovaClient::GetProjectServers(GetProjectServersRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{endpoint}/servers/detail";
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    if (request.ServerLimitIsSet()) {
        requestInfo.m_queryParams["limit"] = std::to_string(request.GetServerLimit());
    }
    if (request.ServerMarkerIsSet()) {
        requestInfo.m_queryParams["marker"] = request.GetServerMarker();
    }
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetProjectServersResponse> response = std::make_shared<GetProjectServersResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get server details response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Get project servers serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<AttachServerVolumeResponse> NovaClient::AttachServerVolume(AttachServerVolumeRequest &request)
{
    if (!CheckParams(request) || !request.ServerIdIsSet() || !request.VolumeIdIsSet()) {
        ERRLOG("Attach server request check params failed.");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "{endpoint}/servers/{serverId}/os-volume_attachments";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_pathParams["serverId"] = request.GetServerId();
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    // 准备请求body体
    Json::Value volumeBody;
    volumeBody["volumeId"] = request.GetVolumeId();
    if (request.DeviceIsSet()) {
        volumeBody["device"] = request.GetDevice();
    }
    Json::Value jsonReq;
    jsonReq["volumeAttachment"] = volumeBody;
    Json::FastWriter fastWriter;
    requestInfo.m_body = fastWriter.write(jsonReq);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<AttachServerVolumeResponse> response = std::make_shared<AttachServerVolumeResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get Attach server response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Attach Volume failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<ServerResponse> NovaClient::DetachServerVolume(DetachVolumeRequest &request,
    const std::string& volumeId)
{
    if (!CheckParams(request) || !request.IsServerIdSet() || volumeId.empty()) {
        ERRLOG("Attach server request check params failed.");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "DELETE";
    requestInfo.m_resourcePath = "{endpoint}/servers/{serverId}/os-volume_attachments/{volumeId}";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_pathParams["serverId"] = request.GetServerId();
    requestInfo.m_pathParams["volumeId"] = volumeId;
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<ServerResponse> response = std::make_shared<ServerResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get Attach server response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return response;
    }
    return response;
}

std::shared_ptr<ServerResponse> NovaClient::ActServer(ServerRequest &request, const std::string& requestType)
{
    if (!CheckParams(request) || !request.IsServerIdSet()) {
        ERRLOG("%s server request check params failed.", requestType.c_str());
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "{endpoint}/servers/{serverId}/action";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_pathParams["serverId"] = request.GetServerId();
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    // 准备请求body体s
    Json::Value LockBody;
    Json::Value jsonReq;
    jsonReq[requestType] = LockBody;
    Json::FastWriter fastWriter;
    requestInfo.m_body = fastWriter.write(jsonReq);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<ServerResponse> response = std::make_shared<ServerResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get %s response failed, errorCode:%d, errorString:%s", requestType.c_str(), response->GetErrCode(),
            response->GetErrString().c_str());
    }
    return response;
}
 
std::shared_ptr<ServerResponse> NovaClient::PowerOnServer(ServerRequest &request)
{
    if (!CheckParams(request) || !request.IsServerIdSet()) {
        ERRLOG("Power on server request check params failed.");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "{endpoint}/servers/{serverId}/action";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_pathParams["serverId"] = request.GetServerId();
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);

    Json::Value jsonReq;
    jsonReq["os-start"] = Json::Value::null;               // 开机参数
    Json::FastWriter fastWriter;
    requestInfo.m_body = fastWriter.write(jsonReq);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<ServerResponse> response = std::make_shared<ServerResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get Power on server response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}

std::shared_ptr<ServerResponse> NovaClient::PowerOffServer(ServerRequest &request)
{
    if (!CheckParams(request) || !request.IsServerIdSet()) {
        ERRLOG("Power off server request check params failed.");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "{endpoint}/servers/{serverId}/action";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_pathParams["serverId"] = request.GetServerId();
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);

    Json::Value jsonReq;
    jsonReq["os-stop"] = Json::Value::null;                // 关机参数
    Json::FastWriter fastWriter;
    requestInfo.m_body = fastWriter.write(jsonReq);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<ServerResponse> response = std::make_shared<ServerResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get Power off server response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}

std::shared_ptr<GetFlavorsDetailResponse> NovaClient::GetFlavorsDetail(GetFlavorsDetailRequest &request)
{
    if (!CheckParams(request)) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{endpoint}/flavors/detail";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetFlavorsDetailResponse> response = std::make_shared<GetFlavorsDetailResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get flavor details response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Get flavor details serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<CreateServerResponse> NovaClient::CreateServer(CreateServerRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "{endpoint}/servers";
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    requestInfo.m_body = request.GetBody();
    INFOLOG("create vm with req body: %s", requestInfo.m_body.c_str());
    std::shared_ptr<CreateServerResponse> response = std::make_shared<CreateServerResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("create server response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Get server serial failed.");
        return nullptr;
    }
    return response;
}
std::shared_ptr<ServerResponse> NovaClient::DeleteServer(ServerRequest &request)
{
    if (!CheckParams(request) || !request.IsServerIdSet()) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "DELETE";
    requestInfo.m_resourcePath = "{endpoint}/servers/{serverId}";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_pathParams["serverId"] = request.GetServerId();
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<ServerResponse> response = std::make_shared<ServerResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Delete server response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}

std::shared_ptr<ServerResponse> NovaClient::SwapServerVolume(ServerRequest &request, const std::string& oldVolumeId,
    const std::string& newVolumeId)
{
    if (!CheckParams(request) || !request.IsServerIdSet() || oldVolumeId.empty() || newVolumeId.empty()) {
        ERRLOG("Swap volume request check params failed, old volume id %s, new volume id %s.",
            oldVolumeId.c_str(), newVolumeId.c_str());
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "PUT";
    requestInfo.m_resourcePath = "{endpoint}/servers/{serverId}/os-volume_attachments/{oldVolumeId}";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_pathParams["serverId"] = request.GetServerId();
    requestInfo.m_pathParams["oldVolumeId"] = oldVolumeId;
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    Json::Value volumeBody;
    volumeBody["volumeId"] = newVolumeId;
    Json::Value jsonReq;
    jsonReq["volumeAttachment"] = volumeBody;
    Json::FastWriter fastWriter;
    requestInfo.m_body = fastWriter.write(jsonReq);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<ServerResponse> response = std::make_shared<ServerResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get swap volume response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return response;
    }
    return response;
}

std::shared_ptr<GetAvailabilityZonesResponse> NovaClient::GetAvailabilityZones(ServerRequest &request)
{
    if (!CheckParams(request)) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{endpoint}/os-availability-zone/detail";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetAvailabilityZonesResponse> response = std::make_shared<GetAvailabilityZonesResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get availability zones response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return response;
    }
    if (!response->Serial()) {
        ERRLOG("Get availability zone serial failed.");
        return nullptr;
    }
    return response;
}
 
bool NovaClient::UpdateToken(ModelBase &request, std::string &tokenStr)
{
    if (!OpenStackTokenMgr::GetInstance().ReacquireToken(request, tokenStr)) {
        ERRLOG("Get token failed.");
        return false;
    }
    return true;
}

bool NovaClient::GetTokenEndPoint(ModelBase &request, std::string &tokenStr, std::string &endpoint)
{
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return false;
    }
    return true;
}

OPENSTACK_PLUGIN_NAMESPACE_END
