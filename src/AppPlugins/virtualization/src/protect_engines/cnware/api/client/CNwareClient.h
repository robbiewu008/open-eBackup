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
#ifndef APPPLUGINS_VIRTUALIZATION_CNWARECLIENT_H
#define APPPLUGINS_VIRTUALIZATION_CNWARECLIENT_H

#include "log/Log.h"
#include "common/client/RestClient.h"
#include "common/cert_mgr/CertMgr.h"
#include "common/Structs.h"
#include "model/CNwareRequest.h"
#include "model/CNwareResponse.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "protect_engines/cnware/common/StorageStruct.h"
#include "protect_engines/cnware/common/ErrorCode.h"
#include "protect_engines/cnware/common/CNwareConstants.h"
#include "protect_engines/cnware/api/client/CNwareSession.h"
#include "model/CreateSnapshotRequest.h"
#include "model/CreateSnapshotResponse.h"
#include "model/InquiriesSnapshotRequest.h"
#include "model/InquiriesSnapshotResponse.h"
#include "model/AttachVolumeSnapshotRequest.h"
#include "model/AttachvolumeSnapshotResponse.h"
#include "model/AddEmptyDiskRequest.h"
#include "model/AddEmptyDiskResponse.h"
#include "model/GetVMInfoRequest.h"
#include "model/GetVMInfoResponse.h"
#include "model/CheckNameUniqueRequest.h"
#include "model/CheckNameUniqueResponse.h"
#include "model/DeleteDiskResponse.h"
#include "model/UninstallOrDeleteDiskRequest.h"
#include "model/BuildNewVMRequest.h"
#include "model/BuildNewVMResponse.h"
#include "model/compute/MigrationRequest.h"
#include "model/network/VswitchsResponse.h"
#include "model/store/AddNfsRequest.h"
#include "model/store/AddNfsStorageRequest.h"
#include "model/store/StoreResourceRequest.h"
#include "model/store/StoreResourceResponse.h"
#include "model/store/NfsRequest.h"
#include "model/store/NfsStoreResponse.h"
#include "model/store/StorageVolumeResponse.h"
#include "model/store/StoreScanResponse.h"
#include "model/CheckShareFileSysConfUniqueRequest.h"
#include "model/CheckShareFileSysConfUniqueResponse.h"
#include "model/DeleteVMRequest.h"
#include "model/MigrateHostsRequest.h"
#include "model/MigrateHostsResponse.h"
#include "model/QueryTaskRequest.h"
#include "model/QueryTaskResponse.h"
#include "model/GetVMDiskInfoRequest.h"
#include "model/GetVMDiskInfoResponse.h"
#include "model/DelDiskOnStorageRequest.h"
#include "model/DeleteDiskOnStorageResponse.h"
#include "model/DetachDiskOnVMRequest.h"
#include "model/GetDiskInfoOnStorageRequest.h"
#include "model/GetDiskInfoOnStorageResponse.h"
#include "model/GetDiskInfoOnStorageRequest.h"
#include "model/GetDiskInfoOnStorageRequest.h"
#include "model/AssociateHostResponse.h"
#include "model/AssociateHostRequest.h"
#include "model/RemoveAssociateHostResponse.h"
#include "model/RemoveAssociateHostRequest.h"
#include "model/store/StoragePoolResponse.h"
#include "model/store/StoragePoolExResponse.h"
#include "model/PortGroupResponse.h"
#include "model/GetVMListRequest.h"
#include "model/GetVMListResponse.h"
#include "model/GetVMInterfaceInfoRequest.h"
#include "model/GetVMInterfaceInfoResponse.h"
#include "model/store/VolExistResponse.h"
#include "model/ModifyBootsRequest.h"
#include "model/DelStoragePoolRequest.h"
#include "model/DelStoragePoolResponse.h"
#include "model/compute/AddDiskRequest.h"
#include "model/compute/AddDiskResponse.h"
#include "model/compute/QueryHostListRequest.h"
#include "model/compute/QueryHostListResponse.h"
#include "model/compute/UpdateVMDiskRequest.h"
#include "model/compute/UpdateVMDiskResponse.h"
#include "model/compute/GetHostInfoResponse.h"
#include "model/compute/HealthRequest.h"
#include "model/compute/InterfaceRequest.h"
#include "model/compute/AddNetworkCardRequest.h"
#include "model/compute/GetHostResourceStatResponse.h"
#include "model/notify/QueryVMTaskResponse.h"
#include "model/tag/AssociateResponse.h"

using VirtPlugin::RestClient;
using VirtPlugin::RequestInfo;
using VirtPlugin::SUCCESS;
using VirtPlugin::ResponseModel;
using VirtPlugin::CertManger;
using AppProtect::QueryByPage;
using AppProtect::ApplicationEnvironment;

namespace CNwarePlugin {

/* Error response body format:
 * {
 *     "errorCode":4199,
 *     "message":"存储卷【422c9d13-aab7-4941-b02e-a1b633815f08】不存在",
 *     "exception":"存储卷【422c9d13-aab7-4941-b02e-a1b633815f08】不存在"
 * }
 */
struct ResponseErrorMsg {
    uint64_t errorCode;
    std::string message;
    std::string exception;

    uint64_t GetErrCode()
    {
        return errorCode;
    }

    std::string GetErrString()
    {
        return message;
    }

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(errorCode)
    SERIAL_MEMEBER(message)
    SERIAL_MEMEBER(exception)
    END_SERIAL_MEMEBER
};

extern std::mutex g_mutexSession;
class CNwareClient : public RestClient {
public:

    explicit CNwareClient(Authentication auth) : m_auth(auth)
    {};
    ~CNwareClient()
    {
        if (m_cnwareSessionCache != nullptr) {
            DBGLOG("CNwareclient m_cnwareSessionCache have ptr!");
            m_cnwareSessionCache->DecreaseRegistreCnt();
        }
        this->Loginout();
    };

    bool CheckParams(ModelBase &model) override;
    int32_t InitCNwareClient(const ApplicationEnvironment& appEnv);
    int32_t CheckAuth(CNwareRequest &req, int64_t &errorCode, std::string &errorDes);
    int32_t GetSessionAndlogin(CNwareRequest &req, RequestInfo &requestInfo,
        int64_t &errorCode, std::string &errorDes, const bool &checkFlag = false);
    int32_t Loginout();
    void ForceLogOut(const CNwareRequest &request);
    int32_t GetVersionInfo(CNwareRequest& req, ApplicationEnvironment& returnEnv);
    std::shared_ptr<ResponseModel> GetVersionInfo(CNwareRequest &req);
    int32_t GetResource(CNwareRequest &req, std::shared_ptr<ResponseModel> response, const QueryByPage &pageInfo,
        CNwareType parentType);
    std::string UrlEncode(const std::string& str);

    std::shared_ptr<GetHostInfoResponse> GetHostInfo(CNwareRequest &req, const std::string &hostId);
    std::shared_ptr<StoreScanResponse> AddNfs(AddNfsRequest &req);
    std::shared_ptr<CheckShareFileSysConfUniqueResponse> CheckShareFileSysConfUnique(
        CheckShareFileSysConfUniqueRequest &req);
    std::shared_ptr<CNwareResponse> CreateSnapshot(CreateSnapshotRequest &req);
    std::shared_ptr<InquiriesSnapshotResponse> InquiriesSnapshot(InquiriesSnapshotRequest &req);
    std::shared_ptr<CheckNameUniqueResponse> CheckNameUnique(CheckNameUniqueRequest &req);
    std::shared_ptr<GetVMInfoResponse> GetVMInfo(GetVMInfoRequest &req);
    std::shared_ptr<GetVMListResponse> GetVMList(GetVMListRequest &req);
    std::shared_ptr<DeleteDiskResponse> UninstallOrDeleteDiskVolId(UninstallOrDeleteDiskRequest &req);
    std::shared_ptr<ResponseModel> DeleteSnapshotVolume(CNwareRequest &req);
    std::shared_ptr<BuildNewVMResponse> BuildNewClient(BuildNewVMRequest &req);
    std::shared_ptr<MigrateHostsResponse> MigrateHosts(MigrateHostsRequest &req);
    std::shared_ptr<CNwareResponse> AttachVolumeSnapshot(AttachVolumeSnapshotRequest &req);
    std::shared_ptr<AddEmptyDiskResponse> AddEmptyDisk(AddEmptyDiskRequest &req);
    std::shared_ptr<AssociateHostResponse> AssociateHost(AssociateHostRequest &req);
    std::shared_ptr<RemoveAssociateHostResponse> RemoveAssociateHost(RemoveAssociateHostRequest &req);
    std::shared_ptr<GetVMDiskInfoResponse> GetVMDiskInfo(GetVMDiskInfoRequest &req);
    std::shared_ptr<GetDiskInfoOnStorageResponse> GetDiskInfoOnStorage(GetDiskInfoOnStorageRequest &req);
    std::shared_ptr<CNwareResponse> DetachDiskOnVM(DetachDiskOnVMRequest &req);
    std::shared_ptr<QueryTaskResponse> QueryTask(QueryTaskRequest &req, const bool &needDetail = false);
    std::shared_ptr<QueryVMTaskResponse> QueryVMTasks(CNwareRequest &req, const std::string &domainId,
        const std::string &status, const int32_t &start = 1, const int32_t &size = 500);
    std::shared_ptr<DeleteDiskOnStorageResponse> DeleteDiskOnStorage(DelDiskOnStorageRequest &req);
    std::shared_ptr<StoragePoolResponse> GetStoragePoolInfo(CNwareRequest &req, const std::string &hostId,
        const std::string &poolId = "", const NameType &nameType = NameType::UnSet, const int32_t &start = 1);
    std::shared_ptr<StoragePoolResponse> SearchStoragePool(CNwareRequest &req,
        const std::string &hostId, const std::string &nameLike, const NameType &nameType);
    std::shared_ptr<PortGroupResponse> GetPortGroupInfo(CNwareRequest &req,
        const std::string &hostId);
    std::shared_ptr<CNwareResponse> DeleteVM(DeleteVMRequest &req);
    std::shared_ptr<CNwareResponse> PowerOnVM(CNwareRequest &req);
    std::shared_ptr<CNwareResponse> PowerOffVM(CNwareRequest &req);
    std::shared_ptr<GetVMInterfaceInfoResponse> GetVMInterfaceInfo(GetVMInterfaceInfoRequest &req);
    std::shared_ptr<VolExistResponse> StorageVolumesExist(CNwareRequest &request,
        const std::string &volname);
    std::shared_ptr<CNwareResponse> ModifyVMBoots(ModifyBootsRequest &req);
    std::shared_ptr<StoreScanResponse> AddNfsStorage(AddNfsStorageRequest &req);
    std::shared_ptr<StoreResourceResponse> GetNfsStorageResource(StoreResourceRequest &req);
    std::shared_ptr<StoreScanResponse> ScanNfsStorage(CNwareRequest &req, const std::string &storeId);
    std::shared_ptr<StorageVolumeResponse> GetStorageVolume(CNwareRequest &req,
        const std::string &storageId);
    std::shared_ptr<CNwareResponse> MigrateVols(MigrationRequest &req);
    std::shared_ptr<NfsStoreResponse> GetNfsInfo(NfsRequest &req);
    std::shared_ptr<VswitchsResponse> GetVswitchInfo(CNwareRequest &req, const std::string &hostId);
    std::shared_ptr<DelStoragePoolResponse> DelStoragePool(DelStoragePoolRequest &req);
    std::shared_ptr<AddDiskResponse> AddDisk(AddDiskRequest &req);
    std::shared_ptr<QueryHostListResponse> QueryHostList(QueryHostListRequest &req);
    std::shared_ptr<UpdateVMDiskResponse> UpdateDomainDiskMetadata(UpdateVMDiskRequest &req);
    std::shared_ptr<StoreScanResponse> RefreshStoragePool(CNwareRequest &req, const std::string &poolId);
    std::shared_ptr<CNwareResponse> HealthVm(HealthRequest &req);
    std::shared_ptr<CNwareResponse> ModifyInterface(InterfaceRequest &req);
    std::shared_ptr<StoragePoolExResponse> GetStoragePool(CNwareRequest &req, const std::string &poolId);
    std::shared_ptr<CNwareResponse> AddNetworkCard(AddNetworkCardRequest &req);
    std::shared_ptr<GetHostResourceStatResponse> GetHostResourceStat(CNwareRequest &req, const std::string &hostId);
    std::shared_ptr<ResponseModel> DelCephSnapshots(CNwareRequest &req, const std::string &snapId);
    std::shared_ptr<AssociateResponse> PostVMTagsInfo(CNwareRequest &req, const QueryByPage &pageInfo);
    std::shared_ptr<CNwareResponse> ModifyBaseInfo(CNwareRequest &req,
        const std::string &newName, const std::string &domainId);
    void SetPageSize(const int32_t &pageSize);
    CNwareError GetErrorCode();

protected:
    int32_t Login(CNwareRequest& req, int64_t& errorCode, std::string &errorDes);
    int32_t DoLogin(CNwareRequest &req, int64_t &errorCode, std::string &errorDes,
        std::shared_ptr<ResponseModel> response);
    int32_t ParseResonse(const std::shared_ptr<ResponseModel>& response, int64_t& errorCode, std::string &errorDes,
        const CNwareRequest& req);
    bool ParseResonseBody(const std::shared_ptr<ResponseModel> &response, int64_t &errorCode,
        std::string &errorDes, const CNwareRequest &req);
    int32_t DoCheckSessionValidity(CNwareRequest &req, std::shared_ptr<CNwareSession> cnwareSessionPtr);
    bool CheckSessionValidity(const std::tuple<std::string, std::string>& cnwareInfo, CNwareRequest &req,
        const bool &checkFlag);
    int32_t RefreshSession(const std::shared_ptr<ResponseModel>& response, CNwareRequest& req);
    std::shared_ptr<CNwareSession> GetResponseSession(const std::shared_ptr<ResponseModel> &response,
        CNwareRequest &req);
    bool LoginoutCNware();
    bool SetVersionInfoResonse(const std::shared_ptr<ResponseModel>& response, ApplicationEnvironment& returnEnv);
    bool SetSession(const CNwareRequest &request, RequestInfo &requestInfo);
    bool BuildLoginBody(std::string &body);
    int32_t SendRequest(std::shared_ptr<ResponseModel> response, CNwareRequest &req, RequestInfo &requestInfo,
        std::string &errorDes, int64_t &errorCode);
    int32_t DoSendRequest(std::shared_ptr<ResponseModel> response,
        CNwareRequest &req, RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode);
    int32_t ResponseSuccessHandle(CNwareRequest &req, RequestInfo &requestInfo,
        std::shared_ptr<ResponseModel> &response, std::string &errorDes, int64_t &errorCode);
    void DelayTimeSendRequest();
    void SetResourceParams(RequestInfo &requestInfo, CNwareRequest &req, CNwareType &parentType);
    int32_t ParseErrorBody(const std::shared_ptr<ResponseModel> &response, int64_t &errorCode, std::string &errorDes);
    bool NeedRetry(std::shared_ptr<ResponseModel> response, CNwareRequest &req,
        RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode);

protected:
    std::string m_taskId;
    int32_t m_retryIntervalTime = 15;
    int32_t m_retryTimes = 3;
    std::mutex m_mutexCache {};
    std::shared_ptr<CNwareSession> m_cnwareSessionPtr;
    std::shared_ptr<CNwareSessionCache> m_cnwareSessionCache;
    ResponseErrorMsg m_rspErrMsg;
    CNwareError m_error;
    int32_t m_pageStart = 1;
    int32_t m_pageSize = 500;
    bool m_isPaginate {false};

private:
    // common
    Authentication m_auth;
};
}

#endif // APPPLUGINS_VIRTUALIZATION_CNWARECLIENT_H
