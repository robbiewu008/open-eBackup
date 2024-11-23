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
#ifndef OPENSTACK_PROTECT_ENGINE_H
#define OPENSTACK_PROTECT_ENGINE_H

#include "log/Log.h"
#include "common/Structs.h"
#include "common/cert_mgr/CertMgr.h"
#include "protect_engines/ProtectEngine.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/resource_discovery/OpenStackResourceAccess.h"
#include "protect_engines/openstack/consistent_snapshot/OpenStackConsistentSnapshot.h"
#include "protect_engines/openstack/api/nova/NovaClient.h"
#include "protect_engines/openstack/api/cinder/CinderClient.h"
#include "protect_engines/openstack/api/neutron/NeutronClient.h"
#include "volume_handlers/cloud_volume/cinder_volume/CinderVolumeHandler.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

using namespace VirtPlugin;

class ProtectAppExtendInfo {
public:
    std::string m_domainName;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName)
    END_SERIAL_MEMEBER
};

struct VolResidualInfo {
    std::string m_jobId;
    std::string m_alarmId;
    std::string m_param;
    std::vector<Volume> m_volumelist;
    
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_jobId, jobId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_alarmId, alarmId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_param, parm)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumelist, volumelist)
    END_SERIAL_MEMEBER
};
struct VolResidualListInfo {
    std::vector<VolResidualInfo> m_volResidualInfoList;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volResidualInfoList, volResidualInfoList)
    END_SERIAL_MEMEBER
};

class OpenStackProtectEngine : public ProtectEngine {
public:
    OpenStackProtectEngine()
        : m_certMgr(std::make_shared<VirtPlugin::CertManger>())
    {
        if (m_cinderClient == nullptr) {
            m_cinderClient = std::make_unique<CinderClient>();
        }
        if (m_neutronClient == nullptr) {
            m_neutronClient = std::make_unique<NeutronClient>();
        }
    }

    explicit OpenStackProtectEngine(std::shared_ptr<VirtPlugin::JobHandle> &m_jobHandle, std::string jobId,
        std::string subJobId)
        :   ProtectEngine(m_jobHandle, jobId, subJobId),
            m_certMgr(std::make_shared<VirtPlugin::CertManger>())
    {
        if (m_cinderClient == nullptr) {
            m_cinderClient = std::make_unique<CinderClient>();
        }
        if (m_neutronClient == nullptr) {
            m_neutronClient = std::make_unique<NeutronClient>();
        }
    }

    virtual ~OpenStackProtectEngine() {};
    virtual int PreHook(const ExecHookParam &para);
    virtual int PostHook(const ExecHookParam &para);
    int ControlMachine(const std::string& lockType);

    // snapshot
    virtual int32_t DeleteSnapshotPreHook(const VolSnapInfo &volSnap);
    int32_t CreateSnapshot(SnapshotInfo &snapshot, std::string &errCode) override;
    int32_t DeleteSnapshot(const SnapshotInfo &snapshot) override;
    int32_t QuerySnapshotExists(SnapshotInfo &snapshot) override;
    int32_t GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList) override;
    int32_t ActiveSnapConsistency(const SnapshotInfo& snapshotInfo) override;
    int32_t ActiveSnapInit();
    virtual int32_t GetSnapshotProviderAuth(std::vector<std::string>& proAuth,
        GetSnapshotRequest& request, const VolSnapInfo& volSnap);
    // meta and handle
    int32_t GetMachineMetadata(VMInfo &vmInfo) override;
    int32_t GetVolumesMetadata(const VMInfo &vmInfo,
        std::unordered_map<std::string, std::string> &volsMetadata) override;
    int32_t GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler) override;
    // volume
    int32_t CreateVolume(const VolInfo &backupVol, const std::string &volMetaData, const std::string &vmMoRef,
        const DatastoreInfo &dsInfo, VolInfo &newVol) override;
    int32_t DetachVolume(const VolInfo &volObj) override;
    int32_t AttachVolume(const VolInfo &volObj) override;
    int32_t DeleteVolume(const VolInfo &volObj) override;
    int32_t ReplaceVolume(const VolInfo &volObj) override;
    // machine
    int32_t CreateMachine(VMInfo &vmInfo) override;
    int32_t DeleteMachine(const VMInfo &vmInfo) override;
    int32_t RenameMachine(const VMInfo &vmInfo, const std::string &newName) override;
    int32_t PowerOnMachine(const VMInfo &vmInfo) override;
    int32_t PowerOffMachine(const VMInfo &vmInfo) override;

    // scheduler
    int32_t AllowBackupInLocalNode(const AppProtect::BackupJob& job, int32_t &errorCode) override;
    int32_t AllowBackupSubJobInLocalNode(const AppProtect::BackupJob &job,
        const AppProtect::SubJob &subJob, int32_t &errorCode) override;
    int32_t AllowRestoreInLocalNode(const AppProtect::RestoreJob& job, int32_t &errorCode) override;
    int32_t AllowRestoreSubJobInLocalNode(const AppProtect::RestoreJob& job,
        const AppProtect::SubJob& subJob, int32_t &errorCode) override;
    // check
    int32_t CheckBeforeBackup() override;
    int32_t CheckBeforeRecover(const VMInfo &vmObj) override;
    int32_t CheckBackupJobType(const VirtPlugin::JobTypeParam& jobTypeParam, bool& checkRet) override;
    int32_t CheckBeforeMount() override;
    int32_t CancelLiveMount(const VMInfo &liveVm) override;
    int32_t CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm) override;
    // application
    void DiscoverApplications(std::vector<Application>& returnValue, const std::string& appType) override;
    void CheckApplication(ActionResult &returnValue, const ApplicationEnvironment &appEnv,
        const AppProtect::Application &application) override;
    void ListApplicationResource(std::vector<ApplicationResource>& returnValue, const ApplicationEnvironment& appEnv,
        const Application& application, const ApplicationResource& parentResource) override;
    void ListApplicationResourceV2(ResourceResultByPage& page, const ListResourceRequest& request) override;
    // cluster
    void DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv) override;
    void DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
        const Application& application) override;
    // vol pair
    int GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
        VolMatchPairInfo &volPairs) override;
    bool GetifDeleteSnapshot(AppProtect::BackupJobType backuptype, AppProtect::JobResult::type res) override;
    bool IfDeleteLatestSnapShot() override;

protected:
    virtual bool FormHttpRequest(ModelBase &request, bool useAdmin = false);
    bool DeleteVolumeSnapshot(const VolSnapInfo &volSnap);
    bool InitJobPara();
    bool InitRestoreLevel();
    bool CheckArchitecture(const VolInfo &copyObj, const std::string &targetArch);
    bool CheckVolBootMode(const std::string &copyVolBootable, const std::string &targetVolBootable);
    int32_t BackUpTargetVolumes(SnapshotInfo &snapshot, std::string &errCode);
    void ExpandWaitTime(uint32_t curRetryTime, uint32_t &totalTimes, const std::string &state,
        std::vector<std::string> intermediateState);
    void InitRepoHandler();
    int32_t LoadCopyVolumeMatadata(const std::string &volId, VolInfo &vmInfo);
    int32_t InitParaAndGetTargetVolume(const ApplicationResource &targetVol, Json::Value &targetVolume);
    virtual bool CheckVolumesIfDelete(const VolResidualInfo &volResidualInfo);
    virtual void DeleteVolumesAction(const std::string &file, NewCreatedVolumeList &deleteFailVols);
    virtual bool DeleteSnapshotCreateVolumes(const VolSnapInfo &snapInfo);
    virtual int32_t DoDeleteVolume(const VolInfo &volObj);
    bool CheckIfPreHookDelete(const std::string &volId, const NewCreatedVolumeList &deleteFailVols);
    bool SaveDeleteFailVolumes(const std::string &file, NewCreatedVolumeList &deleteFailVols);
    void DeleteVolumes();
    virtual bool SendDeleteSnapshotMsg(const VolSnapInfo &volSnap);
    virtual bool ConfirmIfSnapDeleted(const std::string &snapId, int retryTimes, int &curConsumeRetryTimes);
    virtual bool CheckIsConsistenSnapshot();
    virtual int32_t DoDeleteConsistencySnapshot(const SnapshotInfo &snapshot);
    virtual bool GetVolumeSnapshot(VolSnapInfo &volSnap);
    virtual int32_t CheckBeforeCreateSnapshot(const std::vector<VolInfo> &volList)
    {
        return SUCCESS;
    }
    virtual bool MatchVolumeName(const std::string &volumeName)
    {
        std::regex reg("^Backup_volume_.*$");
        bool ret = std::regex_match(volumeName, reg);
        if (ret) {
            DBGLOG("The volume name matched, volume: %s", volumeName.c_str());
            return true;
        }
        return false;
    }

protected:
    AppProtect::ApplicationEnvironment m_appEnv;
    std::shared_ptr<CinderClient> m_cinderClient = nullptr;
    std::shared_ptr<NeutronClient> m_neutronClient = nullptr;

protected:
    bool m_initialized { false };
    std::shared_ptr<AppProtect::BackupJob> m_backupPara = nullptr;
    std::shared_ptr<AppProtect::RestoreJob> m_restorePara = nullptr;
    std::shared_ptr<VirtPlugin::CertManger> m_certMgr = nullptr;
    OpenStackPlugin::NovaClient m_novaClient;
    
    std::string m_taskId;
    std::string m_requestId;
    VolResidualListInfo m_volResidualList;
    AppProtect::Application m_application;
    std::string m_serverId;
    int32_t m_commonWaitInterval {10};
    int32_t m_commonWaitTimes {30};
    bool m_machineMetaCached { false };
    VMInfo m_vmInfo;
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::string m_metaRepoPath;
    std::string m_cacheRepoPath;
    RestoreLevel m_restoreLevel = RestoreLevel::RESTORE_TYPE_UNKNOW;
    std::map<std::string, std::string> m_volNameToId;
    VMInfo m_newVm;
    std::unordered_map<std::string, VolInfo> m_volMap;

private:
    // common
    bool ParseCert(const std::string& markId, const std::string& certInfo);
    std::string GetTaskId() const;

    // access
    int32_t GetDomains(ResourceResultByPage& page, OpenStackResourceAccess &access);
    int32_t GetDomainProjects(ResourceResultByPage& page, OpenStackResourceAccess &access,
        const ListResourceRequest& request);
    int32_t GetProjectServers(ResourceResultByPage& page, OpenStackResourceAccess &access,
        const ListResourceRequest& request);
    int32_t GetProjectVolumes(ResourceResultByPage& page, OpenStackResourceAccess &access,
        const ListResourceRequest& request);
    void GetNetworks(ResourceResultByPage& page, OpenStackResourceAccess &access,
        const ListResourceRequest& request);
    void GetFlavors(ResourceResultByPage& page, OpenStackResourceAccess &access,
        const ListResourceRequest& request);
    void GetVolumeTypes(ResourceResultByPage& page, OpenStackResourceAccess &access,
        const ListResourceRequest& request);
    void GetAvailabilityZones(ResourceResultByPage& page, OpenStackResourceAccess &access,
        const ListResourceRequest& request);
    // mainchine
    int32_t CheckProtectEnvConn(const AppProtect::ApplicationEnvironment &env, const AppProtect::Application& app,
        const std::string &vmId);
    virtual bool CheckVMStatus();
    bool GetServerDetails(const VMInfo &vmInfo, ServerDetail &serverDetail);
    void SendDeleteVolumeFailedAlarm(const std::vector<Volume> &deleteFailVolumeList);
    void SaveResidualVolumes();
    void ClearVolumesAlarm();
    void ClearResidualVolumesAlarm(const std::string &file);
    void GetSnapshotCreateVolumesAndDelete();
    std::string GetDeleteFailedVolInfo(const std::vector<Volume> &deleteFailVols) const;
    virtual int32_t PowerOnMachineHandle(const VMInfo &vmInfo);
    int32_t PowerOffMachineHandle(const VMInfo &vmInfo, bool &isPowerOffVmConflict);
    bool GetAndCheckMachineStatus(const VMInfo &vmInfo, const std::string& status, int waitTimes = 1);
    virtual bool DoGetMachineMetadata();
    int32_t SendCreateMachineRequest(VMInfo &vmInfo, OpenStackServerInfo &serverExtendInfo, std::string &sysVolumeId);
    int32_t GetNetworksId(const std::vector<std::string> &networksName, std::vector<std::string> &networksId);
    std::string FormCreateServerBody(OpenStackServerInfo &serverExtendInfo, std::string sysVolumeId);
    int32_t WaitForServerCreate(ServerDetail &server);
    bool TransServerDetail2VMInfo(const ServerDetail &serverDetail, VMInfo &vmInfo);
    int32_t GetProjectServers();
    int32_t DeleteOriginServer(const int32_t &jobResult);
    int32_t WaitForServerDelete(const std::string &serverId);
    int32_t BuildNewServerInfo(OpenStackServerInfo &newServerInfo);
    int32_t UpdateTargetServerId();
    int32_t GetKeystoneService();
    int32_t GetServerDetailById(const std::string &serverId);
    int32_t GetService(const std::string& name);
    int32_t DoDeleteServer(const std::string &serverId);

    // volume
    void TransVolumeDetail2VolInfo(const Volume &volDetail, VolInfo &volInfo);
    virtual bool GetVolumeInfo(const std::string &volId, VolInfo &volInfo);
    bool GetVolumeDetail(const std::string &volId, Volume &volDetail);
    void FormCreateVolumeBody(const VolInfo &volObj, const std::string &volMetaData, std::string &body);
    void FormVolumeInfo(const Json::Value &targetVolume, VolInfo &volObj);

    int32_t LoadCopyVmMatadata(VMInfo &vmInfo);

    int32_t AttachVolumeHandle(const VolInfo &volObj, const std::string& serverId);
    int32_t DetachVolumeHandle(const VolInfo &volObj, const std::string& serverId);
    virtual bool RequestAttachVolume(const std::string& serverId, const std::string& volumeId, const std::string& path,
        const VolInfo &volObj);
    virtual bool RequestDetachVolume(const std::string& serverId, const std::string& volumeId);
    virtual bool GetServerWhichAttachedVolume(const VolInfo &volInfo, std::string& serverId);
    bool GetVolumeSnapshot(SnapshotDetailsMsg &snapDetailsMsg);
    int32_t DoWaitVolumeStatus(const std::string &volId, const std::string &status,
        std::vector<std::string> intermediateState = {}, uint32_t interval = 30, uint32_t retryCount = 5);
    int32_t UpdateVolumeBootable(const std::string &volId);
    virtual int32_t DoCreateVolume(const VolInfo &backupVol, const std::string &volMetaData, const std::string &vmMoRef,
        const DatastoreInfo &dsInfo, VolInfo &newVol);

    bool ParseCreateSnapResponse(std::shared_ptr<CreateSnapshotResponse> createSnapRes, std::string &errCode);
    void FillUpVolSnapInfo(const VolInfo &volInfo, const SnapshotDetails &snapDetails, const std::string &snapshotStr,
        VolSnapInfo &volSnap);
    bool CreateVolumeSnapshot(const VolInfo &volInfo, VolSnapInfo &volSnap, std::string &errCode);
    virtual bool DoCreateSnapshot(const std::vector<VolInfo> &volList, SnapshotInfo &snapshot, std::string &errCode);
    int32_t BackUpAllVolumes(SnapshotInfo &snapshot, std::string &errCode);
    virtual bool CheckTargetVolume(const VolInfo &copyVolObj, const ApplicationResource &restoreVol);
    bool CheckVolStatus(const std::string &curVolState);
    virtual bool CheckVolumeList(const std::vector<VolInfo> &copyVolList);
    bool FilterAndDeleteVolumes(const VolumeList &volList);
    bool IsSnapShotError(const std::string &snapshotId);
    int32_t DeleteMachineAndVolumes(const VMInfo &vmInfo);
    bool DoCreateConsistencySnapshot(const std::vector<VolInfo> &volList,
        SnapshotInfo &snapshot, std::string &errCode);
    bool DoCreateCommonSnapshot(const std::vector<VolInfo> &volList, SnapshotInfo &snapshot, std::string &errCode);
    int32_t DeleteCommonSnapshot(const SnapshotInfo &snapshot);
};

OPENSTACK_PLUGIN_NAMESPACE_END

#endif

