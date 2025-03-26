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
#ifndef HCS_PROTECT_ENGINE_H
#define HCS_PROTECT_ENGINE_H

#include <regex>
#include <map>
#include <vector>
#include "cstdint"
#include "protect_engines/ProtectEngine.h"
#include "define/Types.h"
#include "common/Macros.h"
#include "log/Log.h"
#include "common/Structs.h"
#include "common/cert_mgr/CertMgr.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/hcs/api/evs/EvsClient.h"
#include "protect_engines/hcs/api/ecs/EcsClient.h"
#include "protect_engines/hcs/resource_discovery/HcsResourceAccess.h"
#include "protect_engines/hcs/api/cinder/HcsCinderClient.h"
#include "protect_engines/openstack/OpenStackProtectEngine.h"

using namespace OpenStackPlugin;

namespace HcsPlugin {
enum class HcsVolumeType {
    VOL_NORMAL,
    VOL_SHARED
};

class ProtectEnvExtendInfo {
public:
    std::string m_projectId;
    std::string m_regionId;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, projectId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, regionId)
    END_SERIAL_MEMEBER
};

class HcsVdcInfo {
public:
    std::string m_name;
    std::string m_passwd;
    std::string m_domainName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_passwd, passwd)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName)
    END_SERIAL_MEMEBER

    ~HcsVdcInfo() {
        Module::CleanMemoryPwd(m_passwd);
    }
};

class HcsStorage {
public:
    std::string m_username;
    std::string m_password;
    std::string m_ip;
    int32_t m_port;
    int32_t m_enableCert;
    std::string m_certification;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_username, username)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_password, password)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_port, port)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enableCert, enableCert)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_certification, certification)
    END_SERIAL_MEMEBER

    ~HcsStorage() {
        Module::CleanMemoryPwd(m_password);
        Module::FreeContainer(m_certification);
    }
};

/* HCS volume datastore extend info */
class HCSVolumeDSExtendInfo : public VolumeDSExtendInfo {
public:
    /* HCS specific items if any */
};

class ProtectEnvAuthExtendInfo {
public:
    HcsVdcInfo m_vdcInfo;
    std::string m_vdcInfoStr;
    std::string m_certification;
    std::string m_storages;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vdcInfoStr, vdcInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_certification, certification)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storages, storages)
    END_SERIAL_MEMEBER

    ~ProtectEnvAuthExtendInfo() {
        Module::CleanMemoryPwd(m_vdcInfoStr);
        Module::CleanMemoryPwd(m_storages);
        Module::FreeContainer(m_certification);
    }
};


class HCSProtectEngine : public OpenStackProtectEngine {
public:
    HCSProtectEngine()
        : OpenStackProtectEngine()
        {
            m_cinderClient = std::make_unique<HcsPlugin::HcsCinderClient>();
            m_cinderCertMgr = std::make_shared<VirtPlugin::CertManger>();
        };
    explicit HCSProtectEngine(std::shared_ptr<VirtPlugin::JobHandle> jobHandle, std::string jobId, std::string subJobId)
        : OpenStackProtectEngine(jobHandle, jobId, subJobId)
        {
            m_cinderClient = std::make_unique<HcsPlugin::HcsCinderClient>();
            m_cinderCertMgr = std::make_shared<VirtPlugin::CertManger>();
        };

    int PreHook(const ExecHookParam &para) override;
    int PostHook(const ExecHookParam &para) override;
    int LockMachine();
    int UnLockMachine();
    int32_t QuerySnapshotExists(SnapshotInfo &snapshot) override;
    int32_t GetMachineMetadata(VMInfo &vmInfo) override;
    int32_t GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler) override;
    int32_t DetachVolume(const VolInfo &volObj) override;
    int32_t AttachVolume(const VolInfo &volObj) override;
    int32_t PowerOffMachine(const VMInfo &vmInfo) override;
    int32_t AllowBackupInLocalNode(const AppProtect::BackupJob& job, int32_t &errorCode) override;
    int32_t AllowRestoreInLocalNode(const AppProtect::RestoreJob& job, int32_t &errorCode) override;
    int32_t CheckBackupJobType(const VirtPlugin::JobTypeParam& jobTypeParam, bool& checkRet) override;
    void DiscoverApplications(std::vector<Application>& returnValue, const std::string& appType) override;
    void CheckApplication(ActionResult &returnValue, const ApplicationEnvironment &appEnv,
        const AppProtect::Application &application) override;
    void ListApplicationResource(std::vector<ApplicationResource>& returnValue, const ApplicationEnvironment& appEnv,
        const Application& application, const ApplicationResource& parentResource) override;
    void ListApplicationResourceV2(ResourceResultByPage& page, const ListResourceRequest& request) override;
    void DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv) override;
    void DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
        const Application& application) override;
    int GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
        VolMatchPairInfo &volPairs) override;
    int32_t CheckBeforeBackup() override;
    int32_t CheckBeforeMount() override;
    int32_t CancelLiveMount(const VMInfo &liveVm) override;
    int32_t CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm) override;
    std::string GetTaskId() const;
    int32_t AllowBackupSubJobInLocalNode(
        const AppProtect::BackupJob &job, const AppProtect::SubJob &subJob, int32_t &errorCode) override;
    int32_t AllowRestoreSubJobInLocalNode(
        const AppProtect::RestoreJob &job, const AppProtect::SubJob &subJob, int32_t &errorCode) override;
    int32_t GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList) override;
    bool IfDeleteLatestSnapShot() override;
    void SetErrorCodeParam(const int32_t errorCode, std::vector<std::string> &certParams) override;
    int32_t AllowDelCopyInLocalNode(const AppProtect::DelCopyJob &job, ActionResult& returnValue) override;
    int32_t AllowDelCopySubJobInLocalNode(const AppProtect::DelCopyJob& job,
        const AppProtect::SubJob& subJob, ActionResult& returnValue) override;

protected:
    bool FormHttpRequest(ModelBase &request, bool useAdmin = false) override;
    bool FormHttpRequestInCommonScene(ModelBase &request, bool useAdmin);
    bool FormHttpRequestInOpService(ModelBase &request, ApplicationEnvironment &appEnv);

private:
    int32_t CheckStorageConnection(const VolInfo &volInfo, const std::string &authExtendInfo, int32_t &erroCode);
    int32_t PowerOnMachineHandle(const VMInfo &vmInfo);
    int32_t PowerOffMachineHandle(const VMInfo &vmInfo);
    int32_t AttachVolumeHandle(const VolInfo &volObj);
    int32_t DetachVolumeHandle(const VolInfo &volObj);
    int32_t GetSnapshotProviderAuth(std::vector<std::string>& proAuth, GetSnapshotRequest& request,
        const VolSnapInfo& volSnap) override;

    bool CheckVMStatus();
    bool CheckTargetVolume(const VolInfo &copyVolObj, const std::string &targetVolUUID);
    bool CheckTargetVolumeList(const VolInfo &copyVolObj, const ApplicationResource &restoreVol);
    bool CheckVolStatus(const std::string &curVolState);
    bool CheckVolumeList(const std::vector<VolInfo> &copyVolList);
    bool GetVMDetails(const VMInfo &vmInfo, ServerDetail &serverDetail);
    bool GetVolDetails(const VolInfo &volInfo, HSCVolDetail &volDetail);
    bool DoGetMachineMetadata();
    bool TransServerDetail2VMInfo(const ServerDetail &serverDetail, VMInfo &vmInfo);
    bool DoCreateSnapshot(const std::vector<VolInfo> &volList, SnapshotInfo &snapshot, std::string &errCode);
    bool CreateVolumeSnapshot(const VolInfo &volInfo, VolSnapInfo &volSnap, std::string &errCode, bool useEvs);
    bool ShowVolumeSnapshot(VolSnapInfo &volSnap);
    bool FillUpVolSnapInfo(const VolInfo &volInfo,
        const SnapshotDetails &snapDetails, const std::string &snapshotStr, VolSnapInfo &volSnap);
    bool ShowVolumeDetail(const std::string &volId, VolInfo &volInfo);
    bool TransVolumeDetail2VolInfo(const HSCVolDetail &volDetail, VolInfo &volInfo);
    bool TransferVolumeInfo(const HSCVolDetail &volDetail, VolInfo &volInfo);

    int32_t CheckVolumeStatus(const VolInfo &volObj, const std::string &status);
    bool AssemblyTargetVolInfo(VolPair &volumePair, const std::shared_ptr<ShowVolumeResponse> &response);
    int32_t GetProjectResource(ResourceResultByPage &page, HcsResourceAccess &access,
        const ListResourceRequest& request, std::string &erroParam);
    int32_t GetVirtualMachineResource(ResourceResultByPage &page, HcsResourceAccess &access,
        const ListResourceRequest& request);
    int32_t GetDiskResourceDetail(ResourceResultByPage &page, HcsResourceAccess &access,
        const ListResourceRequest& request);
    void CheckApplicationReturnValue(ActionResult &returnValue,
        const ApplicationEnvironment &appEnv, const AppProtect::Application &application);
    int32_t BackUpAllVolumes(SnapshotInfo &snapshot, std::string &errCode);
    int32_t CheckAttachServerVolume(const VolInfo &volObj);
    int32_t CheckDetachServerVolume(const VolInfo &volObj);
    int32_t CheckProtectEnvConn(const AppProtect::ApplicationEnvironment& env, const std::string &vmId,
        int32_t &errorCode);
    int32_t CheckStorageEnvConn(const ApplicationEnvironment &appEnv);
    bool ParseCert(const std::string &markId, const std::string &certInfo);
    bool ParseCinderCert(const std::string &cinderCertInfo);
    void AddInvalidItemInfo(ResourceResultByPage &page, const std::string& extendInfo, const std::string& parentId);
    void ParseCreateSnapResponse(std::shared_ptr<CreateSnapshotResponse> createSnapRes, std::string &errCode);
    int32_t ActiveSnapInit();
    int32_t GetHcsBackupMode(std::string &backupMode);
    int32_t CheckEnvConnection(
        const AppProtect::ApplicationEnvironment &env, const std::string &vmId, int32_t &errorCode);
    void CheckCertIsExist(int32_t &errorCode);
    void FillSnapshotRequestBody(CreateSnapshotRequestBodyMsg &body, const std::string &snapName,
        const VolInfo &volInfo);
    int32_t GetVDCResource(ResourceResultByPage &page, HcsResourceAccess &access,
        const ListResourceRequest& request);
    void AddVDCErrorMsg(const std::vector<std::string>& errorUserName,
        ResourceResultByPage &page, const std::string& parentId);
    int32_t CheckDistributeVersion(std::vector<StorageInfo> &storageVector,
        const VirtPlugin::JobTypeParam &jobTypeParam);
    int32_t DeleteSnapshotPreHook(const VolSnapInfo &volSnap) override;
    bool CheckIsConsistenSnapshot() override;
    int32_t CheckBeforeCreateSnapshot(const std::vector<VolInfo> &volList) override;
    double GetMinAvailableCapacity();
    int32_t DealWithVolumeHander(const VolSnapInfo &volSnap);
    bool SendDeleteSnapshotMsg(const VolSnapInfo &volSnap) override;
    bool NormalDeleteSnapshot(const VolSnapInfo &volSnap);
    int32_t CheckPreSnapshotExist(const VirtPlugin::JobTypeParam &jobTypeParam);

private:
    bool m_certCpsIsExists { true };
    bool m_certHcsIsExists { true };
    bool m_noNeedInitOceanVolHandler { false };
    bool m_hasUpdateToken = false;
    std::shared_ptr<VirtPlugin::CertManger> m_cinderCertMgr = nullptr;
    HcsPlugin::EcsClient m_ecsClient;
    HcsPlugin::EvsClient m_evsClient;
    HcsPlugin::HcsCinderClient m_hcsCinderClient;
    HcsOpServiceUtils m_hcsOpServiceUtils;
};
}

#endif // HCS_PROTECT_ENGINE_H