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
#include "CinderClient.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"

namespace {
const std::string MODULE_NAME = "OpenStackCinderClient";
const std::string SNAPSHOT_PTRFIX_NAME = "snapshot-";  // fusionstorage 快照名称前缀

using Defer = std::shared_ptr<void>;
}

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

bool CinderClient::CheckParams(ModelBase &model)
{
    if (!model.UserInfoIsSet()) {
        ERRLOG("User info does not set.");
        return false;
    }
    if (!model.DomainIsSet()) {
        ERRLOG("Domain does not set.");
        return false;
    }
    return true;
}

bool CinderClient::GetTokenEndPoint(ModelBase &request, std::string &tokenStr, std::string &endpoint)
{
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return false;
    }
    return true;
}

std::shared_ptr<GetProjectVolumesResponse> CinderClient::GetProjectVolumes(GetProjectVolumesRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{endpoint}/volumes/detail";
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    if (request.GetVolumeLimitIsSet()) {
        requestInfo.m_queryParams["limit"] = std::to_string(request.GetVolumeLimit());
    }
    if (request.GetVolumeOffsetIsSet()) {
        requestInfo.m_queryParams["offset"] = std::to_string(request.GetVolumeOffSet());
    }
    if (!request.GetSnapShotId().empty()) {
        requestInfo.m_queryParams["snapshot_id"] = request.GetSnapShotId();
    }
    if (!request.GetVolumeStatus().empty()) {
        requestInfo.m_queryParams["status"] = request.GetVolumeStatus();
    }
    std::shared_ptr<GetProjectVolumesResponse> response = std::make_shared<GetProjectVolumesResponse>();
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

std::shared_ptr<GetSnapshotResponse> CinderClient::GetSnapshot(GetSnapshotRequest &request)
{
    if (!CheckParams(request) || !request.SnapshotIdIsSet()) {
        ERRLOG("Get Snapshot request check params failed.");
        return nullptr;
    }
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "GET";
    reqInfo.m_resourcePath = "{endpoint}/snapshots/{snapid}";
    reqInfo.m_pathParams["endpoint"] = std::move(endpoint);
    reqInfo.m_pathParams["snapid"] = request.GetSnapshotId();
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);

    reqInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetSnapshotResponse> response = std::make_shared<GetSnapshotResponse>();
    if (CallApi(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Cinder get snap info failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Cinder get snap serial failed.");
        return response;
    }
    return response;
}

void CinderClient::FormRequestInfo(ModelBase &srcRequest, ModelBase &dstRequest)
{
    dstRequest.SetScopeValue(srcRequest.GetScopeValue());
    dstRequest.SetEnvAddress(srcRequest.GetEnvAddress());
    dstRequest.SetUserInfo(srcRequest.GetUserInfo());
    dstRequest.SetDomain(srcRequest.GetDomain());
}

std::shared_ptr<CreateSnapshotResponse> CinderClient::CreateSnapshot(CreateSnapshotRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    CreateSnapshotRequestBodyMsg requestBody = request.GetBody();
    RequestInfo reqInfo;
    reqInfo.m_method = "POST";
    reqInfo.m_resourcePath = "{endpoint}/snapshots";
    reqInfo.m_pathParams["endpoint"] = endpoint;
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = tokenStr;
    reqInfo.m_auth = request.GetUserInfo();
    DBGLOG("Request name: %s", requestBody.m_snapshot.m_name.c_str());
    if (!Module::JsonHelper::StructToJsonString(requestBody, reqInfo.m_body)) {
        ERRLOG("Convert request.m_body to json string failed");
        return nullptr;
    }

    DBGLOG("Request body: %s", reqInfo.m_body.c_str());
    std::shared_ptr<CreateSnapshotResponse> response = std::make_shared<CreateSnapshotResponse>();
    if (CallApi(reqInfo, response, request) != SUCCESS) {
        ERRLOG("Call create snapshot API failed. Error: %d - %s", response->GetErrCode(),
            response->GetErrString().c_str());
        return response;
    }

    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Get snapshot details failed. status code:%d", response->GetStatusCode());
        return response;
    }

    if (!response->Serial()) {
        ERRLOG("Get snapshot details failed. status code:%d", response->GetStatusCode());
        return nullptr;
    }

    GetSnapshotRequest req;
    FormRequestInfo(request, req);
    if (!ConfirmSnapshotReady(req, response)) {
        ERRLOG("Create snapshot failed.");
        return response;
    }
    return response;
}

bool CinderClient::ConfirmSnapshotReady(GetSnapshotRequest &req, std::shared_ptr<CreateSnapshotResponse> response)
{
    SnapshotDetailsMsg snap = response->GetSnapshotDetails();
    if (snap.m_snapshotDetails.m_status == SNAPSHOT_STATUS_AVALIABLE) {
        DBGLOG("Snapshot is available, snapshot id: %s", snap.m_snapshotDetails.m_id.c_str());
        return true;
    }

    DBGLOG("Confirm snapshot status, snapshot id: %s", snap.m_snapshotDetails.m_id.c_str());
    req.SetSnapshotId(snap.m_snapshotDetails.m_id);
    uint32_t retryTimes = 0;
    while (snap.m_snapshotDetails.m_status != SNAPSHOT_STATUS_AVALIABLE && retryTimes++ < SNAPSHOT_RETRY_TIMES) {
        std::shared_ptr<GetSnapshotResponse> reps = GetSnapshot(req);
        if (reps == nullptr) {
            continue;
        }
        if (reps->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
            WARNLOG("Show snapshot return %d", reps->GetStatusCode());
            continue;
        }
        DBGLOG("Show snapshot return: %s", WIPE_SENSITIVE(reps->GetBody()).c_str());
        snap = reps->GetSnapshotDetails();
        if (snap.m_snapshotDetails.m_status == SNAPSHOT_STATUS_AVALIABLE) {
            response->SetGetBody(reps->GetBody());
            return response->Serial();
        }
        if (snap.m_snapshotDetails.m_status == SNAPSHOT_STATUS_ERROR) {
            response->SetGetBody(reps->GetBody());
            return false;
        }
        sleep(SNAPSHOT_RETRY_PERIOD);
    }
    ERRLOG("Snapshot status abnormal.");
    return false;
}

std::shared_ptr<GroupTypeResponse> CinderClient::CreateGroupType(GroupTypeRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }

    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "{endpoint}/group_types";
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
    requestInfo.m_auth = request.GetUserInfo();
 
    GroupTypeRequestBodyMsg requestBody = request.GetBody();
    if (!Module::JsonHelper::StructToJsonString(requestBody, requestInfo.m_body)) {
        ERRLOG("Convert request.m_body to json string failed");
        return nullptr;
    }

    std::shared_ptr<GroupTypeResponse> response = std::make_shared<GroupTypeResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get server details response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }

    if (!response->Serial()) {
        ERRLOG("Serial group type details failed.");
        return nullptr;
    }
    return response;
}
std::shared_ptr<VolumeResponse> CinderClient::GetVolumeDetail(VolumeRequest &request)

{
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{endpoint}/volumes/{volume_id}";
    requestInfo.m_auth = request.GetUserInfo();
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = endpoint;
    requestInfo.m_pathParams["volume_id"] = request.GetVolumeId();
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("GetVolumeDetail CallApi failed.");
        return nullptr;
    }
    if (response == nullptr) {
        ERRLOG("GetVolumeDetail CallApi failed.");
        return nullptr;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Get volume(%s) detail return failed.", request.GetVolumeId().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        return nullptr;
    }
    return response;
}

std::shared_ptr<VolumeResponse> CinderClient::CreateVolume(VolumeRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "POST";
    reqInfo.m_resourcePath = "{endpoint}/volumes";
    reqInfo.m_pathParams["endpoint"] = endpoint;
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    reqInfo.m_auth = request.GetUserInfo();
    reqInfo.m_body = request.GetBody();
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    if (CallApi(reqInfo, response, request) != SUCCESS) {
        ERRLOG("Call create volume API failed. Error: %d - %s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }

    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Create volume failed, status code:%d, body: %s", response->GetStatusCode(),
            response->GetBody().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Serial volume details failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<VolumeResponse> CinderClient::DeleteVolume(VolumeRequest &request)
{
    RequestInfo requestInfo;
    requestInfo.m_method = "DELETE";
    requestInfo.m_resourcePath = "{endpoint}/volumes/{volume_id}";
    requestInfo.m_auth = request.GetUserInfo();
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = endpoint;
    requestInfo.m_pathParams["volume_id"] = request.GetVolumeId();
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("DeleteVolume CallApi failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<CreateVolumeTypeResponse> CinderClient::CreateVolumeType(CreateVolumeTypeRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "{endpoint}/types";
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
 
    VolumeTypeRequestBodyMsg requestBody = request.GetBody();
    if (!Module::JsonHelper::StructToJsonString(requestBody, requestInfo.m_body)) {
        ERRLOG("Convert request.m_body to json string failed");
        return nullptr;
    }
 
    std::shared_ptr<CreateVolumeTypeResponse> response = std::make_shared<CreateVolumeTypeResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("create volume type response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}

std::shared_ptr<VolumeGroupResponse> CinderClient::CreateVolumeGroup(VolumeGroupRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "POST";
    reqInfo.m_resourcePath = "{endpoint}/groups";
    reqInfo.m_pathParams["endpoint"] = std::move(endpoint);
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    reqInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
    reqInfo.m_auth = request.GetUserInfo();

    VolumeGroupRequestBodyMsg requestBody = request.GetBody();
    if (!Module::JsonHelper::StructToJsonString(requestBody, reqInfo.m_body)) {
        ERRLOG("Convert request.m_body to json string failed");
        return nullptr;
    }

    std::shared_ptr<VolumeGroupResponse> response = std::make_shared<VolumeGroupResponse>();
    if (CallApi(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("create volume group failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("create volume group serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<UpdateVolumeGroupResponse> CinderClient::UpdateVolumeGroup(UpdateVolumeGroupRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "PUT";
    reqInfo.m_resourcePath = "{endpoint}/groups/{group_id}";
    reqInfo.m_pathParams["endpoint"] = std::move(endpoint);
    reqInfo.m_pathParams["group_id"] = request.GetGroupId();
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    reqInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
    reqInfo.m_auth = request.GetUserInfo();

    UpdateGroupRequestBodyMsg requestBody = request.GetBody();
    if (!Module::JsonHelper::StructToJsonString(requestBody, reqInfo.m_body)) {
        ERRLOG("Convert request.m_body to json string failed");
        return nullptr;
    }

    std::shared_ptr<UpdateVolumeGroupResponse> response = std::make_shared<UpdateVolumeGroupResponse>();
    if (CallApi(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("update volume group failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}

std::shared_ptr<CreateGroupSnapShotResponse> CinderClient::CreateGroupSnapShot(CreateGroupSnapShotRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "POST";
    reqInfo.m_resourcePath = "{endpoint}/group_snapshots";
    reqInfo.m_pathParams["endpoint"] = std::move(endpoint);
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    reqInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
    reqInfo.m_auth = request.GetUserInfo();

    GroupSnapShotRequestBodyMsg requestBody = request.GetBody();
    if (!Module::JsonHelper::StructToJsonString(requestBody, reqInfo.m_body)) {
        ERRLOG("Convert request.m_body to json string failed");
        return nullptr;
    }

    std::shared_ptr<CreateGroupSnapShotResponse> response = std::make_shared<CreateGroupSnapShotResponse>();
    if (CallApi(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Create group snapshot failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }

    if (!response->Serial()) {
        ERRLOG("create volume group snapshot serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<DeleteSnapshotResponse> CinderClient::DeleteSnapshot(DeleteSnapshotRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "POST";
    reqInfo.m_resourcePath = "{endpoint}/snapshots/{snapshot_id}/action";
    reqInfo.m_pathParams["endpoint"] = endpoint;
    reqInfo.m_pathParams["snapshot_id"] = request.GetSnapshotId();
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    // fusionstorage 以快照名称标识ID snapshot-ID, 调用删除接口时需要用纯ID调用
    if (reqInfo.m_pathParams["snapshot_id"].find(SNAPSHOT_PTRFIX_NAME) != std::string::npos) {
        reqInfo.m_pathParams["snapshot_id"] = reqInfo.m_pathParams["snapshot_id"].substr(SNAPSHOT_PTRFIX_NAME.size());
    }
    reqInfo.m_auth = request.GetUserInfo();
    reqInfo.m_body = "{\"os-force_delete\":{}}";
    INFOLOG("Force delete snapshot action.");
    std::shared_ptr<DeleteSnapshotResponse> response = std::make_shared<DeleteSnapshotResponse>();
    if (CallApi(reqInfo, response, request) != SUCCESS) {
        ERRLOG("Call delete snapshot API failed. Error: %s - %s",
            std::to_string(response->GetErrCode()).c_str(), response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}

std::shared_ptr<DeleteVolumeGroupResponse> CinderClient::DeleteGroup(DeleteVolumeGroupRequest &request)
{
    RequestInfo reqInfo;
    reqInfo.m_method = "POST";
    reqInfo.m_resourcePath = "{endpoint}/groups/{group_id}/action";
    reqInfo.m_pathParams["group_id"] = request.GetGroupId();
    reqInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
 
    DeleteVolumeGroupRequestBodyMsg requestBody = request.GetBody();
    if (!Module::JsonHelper::StructToJsonString(requestBody, reqInfo.m_body)) {
        ERRLOG("Convert request.m_body to json string failed.");
        return nullptr;
    }
 
    std::shared_ptr<DeleteVolumeGroupResponse> response = std::make_shared<DeleteVolumeGroupResponse>();
    if (SendRequest(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Cinder delete group(%s) failed, errorCode: %d, errorString: %s", request.GetGroupId().c_str(),
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}

bool CinderClient::UpdateToken(ModelBase &request, std::string &tokenStr)
{
    if (!OpenStackTokenMgr::GetInstance().ReacquireToken(request, tokenStr)) {
        ERRLOG("Get token failed.");
        return false;
    }
    return true;
}

std::shared_ptr<GetSnapshotListResponse> CinderClient::GetSnapshotList(GetSnapshotListRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "GET";
    reqInfo.m_resourcePath = "{endpoint}/snapshots/detail";
    reqInfo.m_pathParams["endpoint"] = std::move(endpoint);
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    reqInfo.m_auth = request.GetUserInfo();
    std::string volumeId = request.GetVolumeId();
    if (!volumeId.empty()) {
        reqInfo.m_queryParams["volume_id"] = volumeId;
    }
    std::shared_ptr<GetSnapshotListResponse> response = std::make_shared<GetSnapshotListResponse>();
    if (response == nullptr) {
        ERRLOG("Create GetSnapshotListResponse pointer failed.");
        return nullptr;
    }
    
    if (CallApi(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Cinder get snap list info failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Cinder get snap serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<GetSnapshotResponse> CinderClient::GetGroupSnapshot(GetSnapshotRequest &request)
{
    RequestInfo reqInfo;
    reqInfo.m_method = "GET";
    reqInfo.m_resourcePath = "{endpoint}/group_snapshots/{group_snapshot_id}";
    reqInfo.m_pathParams["group_snapshot_id"] = request.GetGroupSnapshotId();
    reqInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
 
    std::shared_ptr<GetSnapshotResponse> response = std::make_shared<GetSnapshotResponse>();
    if (response == nullptr) {
        ERRLOG("Create GetSnapshotResponse pointer failed.");
        return nullptr;
    }

    if (SendRequest(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Cinder get group snapshot info failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->GroupSnapshotSerial()) {
        ERRLOG("Cinder get group snapshot info serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<GroupTypeResponse> CinderClient::GetGroupType(GroupTypeRequest &request)
{
    RequestInfo reqInfo;
    reqInfo.m_method = "GET";
    reqInfo.m_resourcePath = "{endpoint}/group_types/{group_type_id}";
    reqInfo.m_pathParams["group_type_id"] = request.GetGroupTypeId();
    reqInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();

    std::shared_ptr<GroupTypeResponse> response = std::make_shared<GroupTypeResponse>();
    if (response == nullptr) {
        ERRLOG("Create GroupTypeResponse pointer failed.");
        return nullptr;
    }
    if (SendRequest(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Cinder get group type info failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Cinder get group type info serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<GroupTypeResponse> CinderClient::DeleteGroupType(GroupTypeRequest &request)
{
    RequestInfo requestInfo;
    requestInfo.m_method = "DELETE";
    requestInfo.m_resourcePath = "{endpoint}/group_types/{group_type_id}";
    requestInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
    requestInfo.m_pathParams["group_type_id"] = request.GetGroupTypeId();
 
    std::shared_ptr<GroupTypeResponse> response = std::make_shared<GroupTypeResponse>();
    if (response == nullptr) {
        ERRLOG("Create DeleteGroupType pointer failed.");
        return nullptr;
    }
    if (SendRequest(requestInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Delete group type response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    
    return response;
}
 
std::shared_ptr<DeleteSnapshotResponse> CinderClient::DeleteGroupSnapShot(DeleteSnapshotRequest &request)
{
    RequestInfo reqInfo;
    reqInfo.m_method = "DELETE";
    reqInfo.m_resourcePath = "{endpoint}/group_snapshots/{group_snapshot_id}";
    reqInfo.m_pathParams["group_snapshot_id"] = request.GetGroupSnapshotId();
    reqInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
 
    std::shared_ptr<DeleteSnapshotResponse> response = std::make_shared<DeleteSnapshotResponse>();
    if (response == nullptr) {
        ERRLOG("Create DeleteGroupSnapShot pointer failed.");
        return nullptr;
    }
 
    if (SendRequest(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Delete group snapshot failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}
 
std::shared_ptr<VolumeGroupResponse> CinderClient::GetVolumeGroupStatus(VolumeGroupRequest &request)
{
    RequestInfo reqInfo;
    reqInfo.m_method = "GET";
    reqInfo.m_resourcePath = "{endpoint}/groups/{group_id}";
    reqInfo.m_pathParams["group_id"] = request.GetGroupId();
    reqInfo.m_headerParams["OpenStack-API-Version"] = request.GetApiVersion();
 
    std::shared_ptr<VolumeGroupResponse> response = std::make_shared<VolumeGroupResponse>();
    if (response == nullptr) {
        ERRLOG("Create GetVolumeGroupStatus pointer failed.");
        return nullptr;
    }
 
    if (SendRequest(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Get group snapshot failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
 
    if (!response->Serial()) {
        ERRLOG("Get volume Groups status failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<GetVolumeTypesResponse> CinderClient::GetVolumeTypes(GetVolumeTypesRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "GET";
    reqInfo.m_resourcePath = "{endpoint}/types";
    reqInfo.m_pathParams["endpoint"] = std::move(endpoint);
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    reqInfo.m_auth = request.GetUserInfo();
    std::string volumeTypeName = request.GetVolumeTypeName();
    if (!volumeTypeName.empty()) {
        reqInfo.m_queryParams["name"] = volumeTypeName;
    }
    std::shared_ptr<GetVolumeTypesResponse> response = std::make_shared<GetVolumeTypesResponse>();
    if (response == nullptr) {
        ERRLOG("Create GetVolumeTypeResponse pointer failed.");
        return nullptr;
    }
    if (CallApi(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Cinder get volume type info failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Cinder get volume type serial failed.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<UpdateVolumeBootableResponse> CinderClient::UpdateVolumeBootable(UpdateVolumeBootableRequest &request)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(request, tokenStr, endpoint)) {
        return nullptr;
    }
    if (!request.VolumeIdIsSet() && !request.GetVolumeId().empty()) {
        ERRLOG("Update volume bootable failed, invalid volume id:%s", request.GetVolumeId());
        return nullptr;
    }
    RequestInfo reqInfo;
    reqInfo.m_method = "POST";
    reqInfo.m_resourcePath = "{endpoint}/volumes/{volume_id}/action";
    reqInfo.m_pathParams["endpoint"] = std::move(endpoint);
    reqInfo.m_pathParams["volume_id"] = request.GetVolumeId();
    reqInfo.m_queryParams = {};
    reqInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    Json::Value body;
    Json::Value bootable;
    bootable["bootable"] = "True";
    body["os-set_bootable"] = bootable;
    Json::FastWriter fastWriter;
    reqInfo.m_body = fastWriter.write(body);
    reqInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<UpdateVolumeBootableResponse> response = std::make_shared<UpdateVolumeBootableResponse>();
    if (response == nullptr) {
        ERRLOG("Create UpdateVolumeBootable pointer failed.");
        return nullptr;
    }
    if (CallApi(reqInfo, response, request) != VirtPlugin::SUCCESS) {
        ERRLOG("Cinder update volume bootable failed, errorCode: %d, errorString: %s",
            response->GetErrCode(), response->GetErrString().c_str());
        return nullptr;
    }
    return response;
}

int32_t CinderClient::SendRequest(RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model)
{
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!GetTokenEndPoint(model, tokenStr, endpoint)) {
        return VirtPlugin::FAILED;
    }

    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = model.GetUserInfo();
    
    if (CallApi(requestInfo, response, model) != VirtPlugin::SUCCESS) {
        ERRLOG("Call Api Failed.");
        return VirtPlugin::FAILED;
    }
    return VirtPlugin::SUCCESS;
}

OPENSTACK_PLUGIN_NAMESPACE_END
