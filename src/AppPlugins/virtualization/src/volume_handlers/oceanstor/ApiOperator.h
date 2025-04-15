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
#ifndef API_OPERATOR_H
#define API_OPERATOR_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "define/Types.h"
#include "json/json.h"
#include "common/CleanMemPwd.h"
#include "volume_handlers/common/ControlDevice.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "SessionCache.h"
#include "curl_http/HttpClientInterface.h"
#include "common/Structs.h"

namespace VirtPlugin {
namespace ApiErrorCode {
const int32_t FILESYSTEMALREADYEXIST = 1077948993;
const int32_t NFSSHAREALREADYEXIST = 1077939724;
const int32_t CIFSSHAREALREADYEXIST = 1077939715;
const int32_t CIFSSHARE_PERMISSON_EXIST = 1077939718;
const int32_t ALREADYINWHITE = 1077939727;
const int32_t FILESYSTEMNOTEXIST = 1077939726;
const int32_t FILESYSTEMIDNOTEXIST = 1073752065;
const int32_t FILESYSTEMSNAPSHOTEXIST = 1073754142;
const int32_t HOSTLUNMAPPINGEXIST = 1073804588;
const int32_t HOSTEXIST = 1077948993;
const int32_t NOTNEEDADDNUMBER = 1073947144;
const int32_t SNAPSHOT_NOTEXIST = 1077937880;
const int32_t FSSNAPSHOT_NOTEXIST = 1073754118;
const int32_t WINDOWSUSERNOTEXIST = 37749698;
const int32_t NOUSERPERMISSION = 1077949058;
const int32_t AUTHIPINCONSISTENCY = 1073793620;
const int32_t LUNGROUP_HOST_MAPPING_NOTEXIST = 1073804589;
const int32_t LUN_HOST_MAPPING_NOTEXIST = 1073804587;
const int32_t LUN_HOST_MAPPING_EXIST = 1073804588;
const int32_t LUNGROUP_HOST_MAPPING_EXIST = 1073804590;
const int32_t FC_AND_ISCSI_NOTEXIST = 1077948996;
const int32_t REMOTE_DEVICE_NOTEXIST = 37100137;
const int32_t REMOTE_USER_NOTEXIST = 1077949057;
const int32_t RETURN_SNAP_REACH_FILE_SYSTEM_MAX_NUM = 1073754137;
const int32_t RETURN_SNAP_REACH_ENTIRE_SYSTEM_MAX_NUM = 1073754138;
const int32_t RETRUN_SNAP_REACH_PARENT_FS_MAX_NUM = 1073844275;
const int32_t RETURN_SNAP_FS_SPACE_NOT_ENOUGH = 1073754124;
const int32_t DTREENAMEISEXIST = 1077955353;
const int32_t DTREENOTEXIST = 1077955336;
const int32_t FILESYSTEM_IN_WRITE_PROTECTION_STATE = 1073754115;
const int32_t FILESYSTEM_DATA_STATUS_INCONSISTENT = 1073754153;
const int32_t PARAMETER_INCORRECT = 50331651;
const int32_t HOST_NOTIN = 1073745412;
const int32_t HOST_NOTEXIST = 1077937498;
const int32_t HOST_GROUP_NOTIN = 1073804552;
const int32_t HOST_GROUP_NOTEXIST = 1077937500;
const int32_t HOST_GROUP_EXIST = 1077948993;
const int32_t HOST_IS_ALREADY_IN_HOSTGRP = 1077937501;
const int32_t LUN_GROUP_NOTEXIST = 1077936861;
const int32_t LUN_GROUP_NOTIN = 1073804554;
const int32_t MAPVIEW_NOTEXIST = 1077951819;
const int32_t ISCSI_INITIATOR_NOTIN = 1077950342;
const int32_t FC_INITIATOR_NOTIN = 1077950342;  // same to iscsi
const int32_t OBJECT_NOTEXIST = 1077948996;
const int32_t V3_IS_BUSY = 1073793588;
const int32_t V3_SYSTEM_BUSY = 1077949006;
const int32_t LUN_MAPPED_SAME_HOST = 1077936864;
const int32_t RETURN_OM_AUTH_CONNECT_FAILED = 1077949069;
const int32_t UNAUTH = -401;
};  // namespace ApiErrorCode

namespace RetryErrorCode {
const int32_t NO_NEED_RETRY_ERROR_CODE[] = {
    ApiErrorCode::FILESYSTEMALREADYEXIST, ApiErrorCode::NFSSHAREALREADYEXIST,
    ApiErrorCode::CIFSSHAREALREADYEXIST, ApiErrorCode::ALREADYINWHITE, ApiErrorCode::FILESYSTEMNOTEXIST,
    ApiErrorCode::FILESYSTEMSNAPSHOTEXIST, ApiErrorCode::FSSNAPSHOT_NOTEXIST,
    ApiErrorCode::HOSTLUNMAPPINGEXIST, ApiErrorCode::FILESYSTEMIDNOTEXIST, ApiErrorCode::WINDOWSUSERNOTEXIST,
    ApiErrorCode::HOSTEXIST, ApiErrorCode::SNAPSHOT_NOTEXIST, ApiErrorCode::NOTNEEDADDNUMBER,
    ApiErrorCode::LUN_HOST_MAPPING_EXIST, ApiErrorCode::LUNGROUP_HOST_MAPPING_NOTEXIST,
    ApiErrorCode::FC_AND_ISCSI_NOTEXIST, ApiErrorCode::LUNGROUP_HOST_MAPPING_EXIST,
    ApiErrorCode::LUN_HOST_MAPPING_NOTEXIST, ApiErrorCode::RETURN_SNAP_REACH_FILE_SYSTEM_MAX_NUM,
    ApiErrorCode::RETURN_SNAP_REACH_ENTIRE_SYSTEM_MAX_NUM, ApiErrorCode::RETRUN_SNAP_REACH_PARENT_FS_MAX_NUM,
    ApiErrorCode::DTREENOTEXIST, ApiErrorCode::DTREENAMEISEXIST,
    ApiErrorCode::FILESYSTEM_IN_WRITE_PROTECTION_STATE, ApiErrorCode::FILESYSTEM_DATA_STATUS_INCONSISTENT,
    ApiErrorCode::RETURN_SNAP_FS_SPACE_NOT_ENOUGH, ApiErrorCode::PARAMETER_INCORRECT, ApiErrorCode::HOST_NOTIN,
    ApiErrorCode::HOST_NOTEXIST, ApiErrorCode::HOST_GROUP_NOTEXIST, ApiErrorCode::HOST_GROUP_NOTIN,
    ApiErrorCode::HOST_IS_ALREADY_IN_HOSTGRP, ApiErrorCode::LUN_GROUP_NOTEXIST, ApiErrorCode::LUN_GROUP_NOTIN,
    ApiErrorCode::MAPVIEW_NOTEXIST, ApiErrorCode::ISCSI_INITIATOR_NOTIN, ApiErrorCode::OBJECT_NOTEXIST,
    ApiErrorCode::HOST_GROUP_EXIST
};
}


class ApiOperator {
public:
    explicit ApiOperator(ControlDeviceInfo deviceInfo) : m_deviceInfo(deviceInfo)
    {
        m_httpClient = Module::IHttpClient::CreateClient();
        m_useCache = true;
        m_sessionPtr = nullptr;
        InitHttpStatusCodeForRetry();
    }
    virtual ~ApiOperator();
    void SetDiskName(const std::string &name)
    {
        m_diskName = name;
    }
    std::string GetDiskName(const std::string& /* name */) const
    {
        return m_diskName;
    }
    virtual int32_t CreateMappingSet(const std::string& objId, MO_TYPE objType)
    {
        return Module::SUCCESS;
    }
    virtual int32_t DeleteMappingSet(const std::string& objId, MO_TYPE objType)
    {
        return Module::SUCCESS;
    }
    void ClearCacheSession();
    virtual void SetDeviceInfo(const ControlDeviceInfo &info);
    virtual StorageSessionInfo Login(std::string &errorDes);
    virtual int32_t Logout(StorageSessionInfo sessionInfo);
    virtual StorageSessionInfo LoginForIp(std::string &errorDes, const std::string &ip);
    virtual int32_t SendRequestEx(Module::HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode,
        StorageSessionInfo &sessionInfo);
    virtual int32_t ParseResponse(const std::string &body, Json::Value &data, std::string &errorDes, int &errorCode);
    virtual int32_t ParseCookie(const std::set<std::string> &cookieValues, StorageSessionInfo &sessionInfo);
    virtual int32_t SendHttpReq(std::shared_ptr<Module::IHttpResponse> &rsp, const Module::HttpRequest &req,
        std::string &errorDes, int& errorCode);
    virtual int32_t SendRequestOnce(Module::HttpRequest req, Json::Value &data, std::string &errorDes, int &errorCode);
    virtual int32_t ResponseSuccessHandle(Module::HttpRequest req, std::shared_ptr<Module::IHttpResponse>& rsp,
        Json::Value &data, std::string &errorDes, int &errorCode);
    virtual void DelayTimeSendRequest();
    virtual void DeleteDeviceSession();
    virtual void CreateDeviceSession(std::string &errorDes);
    virtual void LoginAndGetSessionInfo(std::string &errorDes);
    virtual int32_t SendRequest(Module::HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode,
        bool lockSession = false);
    virtual bool IsNeedRetryErrorCode(const int32_t &errorCode);
    virtual void InitHttpStatusCodeForRetry();
    virtual bool ResposeNeedRetry(const int32_t ret);
    virtual bool CheckDosAttack();
    virtual int32_t GetSystemInfo(StorageSysInfo &storageSysInfo, std::string &errorDes);
    virtual int32_t GetUserRoleAndLevel(const std::string& userName, UserRoleLevel &userInfo, std::string &errorDes,
        const std::string& productMode);
    virtual int32_t GetServerStatus(std::string &errorDes);
    virtual int32_t GetLunByID(const std::string &lunId, LunMO &lunMo, std::string &errorDes);
    virtual int32_t GetLunByName(const std::string &lunName, LunMO &lunMo, std::string &errorDes);
    virtual int32_t GetLunChunkSize(const std::string &lunID, uint64_t &chunkSize, std::string &errorDes);
    virtual int32_t GetLunAllocBitmap(const std::string &objectId, DiffBitmap &diffBitmap, std::string &errorDes);
    virtual int32_t GetLunUnsharedBitmap(const std::string &objectId, const std::string &parentObjectId,
        DiffBitmap &diffBitmap, std::string &errorDes);
    virtual int32_t GetSnapshotByName(const std::string &snapshotName, SnapshotMO &snapshotMO, std::string &errorDes);
    virtual int32_t GetSnapShotByID(const std::string &snapShotId, SnapshotMO &snapshotMO, std::string &errorDes);
    virtual int32_t CreateHost(const std::string &hostName, const std::string &hostIp, HostMO &hostMO,
        std::string &errorDes);
    virtual int32_t GetHostByID(const std::string &hostID, HostMO &hostMO, std::string &errorDes);
    virtual int32_t GetHostByName(const std::string &hostName, HostMO &hostMO, std::string &errorDes);
    virtual int32_t GetIscsiLogicPortIP(std::vector<std::string> &storageIps, std::string &errorDes);
    virtual int32_t GetEthPortIP(std::vector<std::string> &storageIps, std::string &errorDes);
    virtual int32_t GetBondPortIP(std::vector<std::string> &storageIps, std::string &errorDes);
    virtual int32_t GetTargetIP(std::vector<std::string> &storageIps, std::string &errorDes);
    virtual int32_t ParseEthPortIPJson(Json::Value data, uint32_t i, std::string &storageIP);
    virtual bool GetIpByNetworkType(const std::string &storageIPv4, const std::string &storageIPv6,
        std::string &storageIP);
    virtual int32_t CreateIscsiInitiator(const std::string &id, std::string &errorDes);
    virtual int32_t GetIscsiInitiatorByID(const std::string &id, IscsiInitiatorMO &oRtnIscsiInitiatorMO,
        std::string &errorDes);
    virtual int32_t ParseIscsiInfo(const std::string &id, IscsiInitiatorMO &oRtnIscsiInitiatorMO, Json::Value &data);
    virtual int32_t GetAllIscsiInitiatorByHostID(const std::string &hostID, std::vector<std::string> &iscsiIds,
        std::string &errorDes);
    virtual int32_t AddIscsiInitiatorToHost(const std::string& iscsiID, const std::string& hostID,
        std::string &errorDes);
    virtual int32_t RemoveIscsiInitiatorFromHost(const std::string &iscsiID, std::string &errorDes);
    virtual int32_t CreateFcInitiator(const std::string &id, FCInitiatorMO &fcMO, std::string &errorDes);
    virtual int32_t GetFcInitiatorByID(const std::string &id, FCInitiatorMO &fcMO, std::string &errorDes);
    virtual int32_t GetAllFcInitiatorByHostID(const std::string &hostID, std::vector<std::string> &fcIds,
        std::string &errorDes);
    virtual int32_t AddFcInitiatorToHost(const std::string& fcInitiatorID, const std::string& hostID,
        std::string &errorDes);
    virtual int32_t RemoveFcInitiatorFromHost(const std::string &fcInitiatorID, std::string &errorDes);
    virtual int32_t CreateLunGroup(const std::string &lunGroupName, LunGroupMO &lunGroupMO, std::string &errorDes);
    virtual int32_t AddObjToLunGroup(const std::string& lunGroupID, const std::string& objId, MO_TYPE objType,
        std::string &errorDes);
    virtual int32_t RemoveObjFromLunGroup(const std::string& lunGroupID, const std::string& objId, MO_TYPE objType,
        std::string &errorDes);
    virtual int32_t CreateHostGroup(const std::string &hostGroupName, HostGroupMO &hostGroupMO, std::string &errorDes);
    virtual int32_t AddHostToHostGroup(const std::string& hostID, const std::string& hostGroupID,
        std::string &errorDes);
    virtual int32_t GetHostGroupByName(const std::string &hostGroupName, HostGroupMO &hostGroupMO,
        std::string &errorDes);
    virtual int32_t GetAllHostIDFromHostGroup(const std::string &hostgroupID, std::vector<std::string> &hostIDs,
        std::string &errorDes);
    virtual int32_t CreateMappingView(const std::string& mappingViewName, MappingViewMO &mappingViewMO,
        std::string &errorDes);
    virtual int32_t GetMappingViewByName(const std::string &mapViewName, MappingViewMO& mappingMO,
        std::string &errorDes);
    virtual int32_t AddObjToMappingView(const std::string& mapViewID, const std::string& objId, MO_TYPE objType,
        std::string &errorDes);
    virtual int32_t RemoveObjFromMappingView(const std::string& mapViewID, const std::string& objId,
        MO_TYPE objType, std::string &errorDes);
    virtual int32_t DeleteMapViewByID(const std::string &mapViewID, std::string &errorDes);
    virtual int32_t RemoveHostFromHostGroup(const std::string &hostID, const std::string &hostGroupID,
        std::string &errorDes);
    virtual int32_t DeleteHostGroupByID(const std::string &hostGroupID, std::string &errorDes);
    virtual int32_t DeleteLunGroupByID(const std::string &lunGroupID, std::string &errorDes);
    virtual int32_t DeleteHostByID(const std::string &hostID, std::string &errorDes);
    virtual bool CheckIscsiIsLogined(const std::vector<std::string> &targetIps);
    virtual bool CheckContain(const std::string &strTarget, const std::vector<IscsiSessionStatus> &loginedSessions);
    virtual int32_t GetLunGroupByName(const std::string &lunGroupName, LunGroupMO &lunGroupMO,
        std::string &errorDes);
    virtual int32_t InitMappingNameInfo();
    virtual int32_t CreateInitiatorAndHost(std::string& hostName, HostMO& hostMO);
    virtual int32_t CreateFcInitAndHost(std::string& hostName, HostMO& hostMO);
    virtual int32_t CheckAndLoginIscsiTarget();
    virtual int32_t LoginIscsiTarget(std::vector<std::string>& targetIps);
    virtual int32_t CreateIscsiInitAndHost(std::string& hostName, HostMO& hostMO, std::string& iqnNumber);
    virtual int32_t CreateLunGroupAndAddObj(const std::string& lunGroupName, const std::string& objId, MO_TYPE objType);
    virtual int32_t CreateHostGroupAndAddHost(const std::string& hostGroupName, const std::string& hostId);
    virtual int32_t AssocGetObjIdsByLunGroupID(const std::string &lunGroupId, MO_TYPE objType,
        std::vector<std::string> &objIds, std::string &errorDes);
    virtual int32_t RemoveObjFromLunGroupExp(const std::string& lunGroupName, const std::string& objId,
        MO_TYPE objType);
    virtual int32_t RemovedObjFromAllLunGroup(const std::string &objId, MO_TYPE objType, std::string &errorDes);
    virtual int32_t GetLunGroupsByObjId(const std::string& objId, MO_TYPE objType,
        std::vector<std::string> &lunGroupIds, std::string &errorDes);
    virtual int32_t GetHostGroupFromMappingView(const std::string &mapViewID, HostGroupMO &hostGroupMO,
        std::string &errorDes);
    virtual int32_t GetLunGroupFromMappingView(const std::string &mapViewID, LunGroupMO &lunGroupMO,
        std::string &errorDes);
    virtual int32_t TestDeviceConnection();
    virtual int32_t CheckResponse(const std::shared_ptr<Module::IHttpResponse> rsp, Json::Value &data,
        std::string &errorDes);
    bool RefreshSession(const StorageSessionInfo &sessionInfo);
    bool CheckStorageSession();
    void CheckLength(std::string &input);
    void SetRetry(const bool retry);
    void SetCurlTimeOut(const uint64_t &curlTimeOut);
    int32_t TestIscsiTargetIpConnect();
    int32_t QueryStoragePoolUsedRate(double &usedCapacityRate);
    int32_t TestFcConnect();
    void SetOpService(bool isOpService)
    {
        m_isOpService = isOpService;
    }

private:
    std::string FormatFullUrl(const std::string &fullUrl);
    void DosAttackDefense();
    void CheckIpNotEmpty(std::string &ip);

protected:
    ControlDeviceInfo m_deviceInfo {};
    bool m_useCache = false;
    bool m_isRetry = true;
    std::shared_ptr<StorageSession> m_sessionPtr = nullptr;
    std::shared_ptr<Module::IHttpClient> m_httpClient = nullptr;
    bool m_compress = false;
    bool m_dedup = false;
    int32_t m_storagePoolId = -1;
    std::string m_curlHttp = "https://";
    std::mutex m_mutexSession {};
    std::mutex m_mutexInitiator {};
    std::mutex m_mutexSetInfo {};
    std::mutex m_mutexDos {};
    uint64_t m_lastReqTimeStamp = 0;
    uint64_t m_reqCounts = 0;
    uint64_t m_curlTimeOut = 15;
    int32_t m_retryIntervalTime = 15;
    int32_t m_retryTimes = 3;
    static std::unique_ptr<SessionCache> m_sessionCache;
    std::vector<int32_t> m_httpRspStatusCodeForRetry;
    std::string m_diskName;
    std::string m_hostName;
    std::string m_hostUuid;
    std::string m_lunGroupName;
    std::string m_lunGroupID;
    std::string m_hostGroupName;
    std::string m_hostGroupID;
    std::string m_mappingViewName;
    std::string m_mappingViewID;
    bool m_initMappingNameInfo {false};
    bool m_isOpService {false};
    std::string m_iscsiIqnName = "";
};
}
#endif  // API_OPERATOR_H
