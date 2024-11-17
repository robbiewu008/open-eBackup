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
#ifndef APPPLUGINS_VIRTUALIZATION_NUTANIXPROTECTENGINE_H
#define APPPLUGINS_VIRTUALIZATION_NUTANIXPROTECTENGINE_H
#include "log/Log.h"
#include "common/cert_mgr/CertMgr.h"
#include "common/uuid/Uuid.h"
#include "common/utils/Utils.h"
#include "curl_http/HttpStatus.h"
#include "job_controller/jobs/backup/BackupJob.h"
#include "protect_engines/ProtectEngine.h"
#include "protect_engines/nutanix/common/ErrorCode.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "protect_engines/nutanix/resource/NutanixResourceManager.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"
#include "volume_handlers/cloud_volume/nutanix_volume/NutanixVolumeHandler.h"

using namespace VirtPlugin;

namespace NutanixPlugin {
extern std::mutex g_attachVolumeMutex;
struct NutanixVMMetaDataStruct {
    NutanixVMInfo VmInfo;
    ClusterListResponse clusterInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(VmInfo, VMInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(clusterInfo, clusterInfo)
    END_SERIAL_MEMEBER
};

struct RestorTargetVolume {
    std::string uuid;
    std::string name;
    std::string slotId;
    DatastoreInfo datastore;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(name)
    SERIAL_MEMEBER(slotId)
    SERIAL_MEMEBER(uuid)
    SERIAL_MEMEBER(datastore)
    END_SERIAL_MEMEBER
};

class NutanixProtectEngine : public ProtectEngine {
public:
    NutanixProtectEngine()
    {
    }
    explicit NutanixProtectEngine(std::shared_ptr<VirtPlugin::JobHandle> &m_jobHandle,
        std::string jobId, std::string subJobId) : ProtectEngine(m_jobHandle, jobId, subJobId)
    {
    }

    ~NutanixProtectEngine() = default;
    bool Init();
    int32_t PreHook(const ExecHookParam &para) override;
    int32_t PostHook(const ExecHookParam &para) override;
    // snapshot
    int32_t CreateSnapshot(SnapshotInfo &snapshot, std::string &errCode) override;
    int32_t DeleteSnapshot(const SnapshotInfo &snapshot) override;
    int32_t QuerySnapshotExists(SnapshotInfo &snapshot) override;
    int32_t GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList) override;
    // meta and handle
    int32_t GetMachineMetadata(VMInfo &vmInfo) override;
    int32_t GetVolumesMetadata(const VMInfo &vmInfo,
                               std::unordered_map<std::string, std::string> &volsMetadata) override;
    int32_t GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler) override;
    // volume
    int32_t CreateVolume(const VolInfo &backupVol, const std::string &volMetaData,
        const std::string &vmMoRef, const DatastoreInfo &storage, VolInfo &newVol);
    int32_t DetachVolume(const VolInfo &volObj) override;
    int32_t AttachVolume(const VolInfo &volObj) override;
    int32_t DeleteVolume(const VolInfo &volObj) override;
    int32_t ReplaceVolume(const VolInfo &volObj) override;
    // machine
    int32_t CreateMachine(VMInfo &vmInfo) override;
    int32_t DeleteMachine(const VMInfo &vmInfo) override;
    int32_t RenameMachine(const VMInfo &vmInfo, const std::string &newName) override;
    // 支持指定主机开机，主机UUID通过application.id传入;
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
    int32_t CheckBeforeUnmount() override;
    int32_t CheckResourceUsage(const std::string &resourceId) override;
    int32_t CancelLiveMount(const VMInfo &liveVm) override;
    int32_t CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm) override;
    int32_t MigrateLiveVolume(const VMInfo &liveVm) override;

    int32_t GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
        VolMatchPairInfo &volPairs) override;
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
    bool IfDeleteLatestSnapShot() override;
    bool InitClient(int64_t& errorCode, std::string &errorDes);
    int32_t RestoreVolMetadata(VolMatchPairInfo &volPairs, const VMInfo &vmInfo) override;

protected:
    void SetAppEnv(const ApplicationEnvironment &appEnv);
    void SetCommonInfo(NutanixRequest &req);
    bool InitJobPara();
    bool CheckHostStatus(const std::string &hostId) ;
    bool GetVMInfoById(const std::string &vmId, NutanixVMInfo &nutanixVmInfo) ;
    int32_t SetNutanixVMInfo2VMInfo(const NutanixVMInfo &nutanixVmInfo, VMInfo &vmInfo);
    int32_t SetVmNetwork(const NutanixVMInfo &nutanixVmInfo, VMInfo &vmInfo);
    int32_t GetNetworkById(const std::string &networkId, NetworkInfo &networkInfo);
    bool SetVolListInfo2VMInfo(const NutanixVMInfo &nutanixVmInfo, VMInfo &vmInfo);
    bool DoGetMachineMetadata(const std::string &vmId) ;
    bool CheckTaskStatus(const std::string &taskId) ;
    bool GetSnapshotResult(const std::string &snapShotTaskId, const std::string &snapshotId, SnapshotInfo &snapshot,
                std::string &errCode) ;
    int32_t ConnectToEnv(const AppProtect::ApplicationEnvironment &env);
    bool InitRestoreJobPara(std::shared_ptr<ThriftDataBase> &jobInfo);
    bool DoDeleteSnapshot(const std::string &snapshotId);
    bool SetVolSnapInfo(const VmDisk &disk, VolSnapInfo &volSnap, const NutanixSnapshotInfo &nutanixSnapShot);
    bool GetContainerInfo(const std::string &storrageContainerId, NutanixStorageContainerInfo &storageContainerInfo);
    bool CheckPreSnapShotValidity(const AppProtect::BackupJob &job, const std::vector<VolSnapInfo> &lastVolList);
    bool CheckVMStatus();
    int32_t CheckProtectEnvConn(const AppProtect::ApplicationEnvironment &env,
        const std::string &vmId, int32_t &errorCode);
    void CheckCertIsExist(int32_t &errorCode);
    int32_t CheckStorageConnectionBackup(const VolInfo &volInfo, int32_t &erroCode);
    int32_t GetClusterArch(ClusterListResponse &clusterInfo);
    bool GetVirtualDiskInfo(const std::string &diskId, VirtualDiskInfo &backupDiskInfo);
    // Restore
    bool DetachVolsOnVM(const std::vector<std::string> &vols, const std::string &vmId);
    int32_t CleanTmpDiskOnAgent();
    bool LoadNewVolumeFromFile(const VolInfo &volObj, NewVolumeInfo &newVolInfo);
    void SetCloneByIdDiskInfo(AttachCloneByIdVmDisk &disk, const NewVolumeInfo &newVolInfo,
        const VolInfo &volObj);
    bool AttachVol2TargetVmWithCloneMode(const NewVolumeInfo &tmpVolInfo, const VolInfo &volObj);
    bool GetTargetVmId(std::string &vmId);

protected:
    std::shared_ptr<NutanixClient> m_nutanixClient = nullptr;
    std::shared_ptr<CertManger> m_certMgr = nullptr;
    std::string m_domainId;

protected:
    bool m_initialized { false };
    std::shared_ptr<AppProtect::BackupJob> m_backupPara = nullptr;
    std::shared_ptr<AppProtect::RestoreJob> m_restorePara = nullptr;
    std::shared_ptr<AppProtect::LivemountJob> m_livemountPara = nullptr;
    std::shared_ptr<AppProtect::CancelLivemountJob> m_cancelLivemountPara = nullptr;
    std::string m_taskId;
    std::string m_requestId;
    AppProtect::ApplicationEnvironment m_appEnv;
    AppProtect::Application m_application;
    std::vector<ApplicationResource> m_subObjects;
    bool m_machineMetaCached { false };
    VMInfo m_vmInfo;

private:
    RestoreLevel m_restoreLevel = RestoreLevel::RESTORE_TYPE_UNKNOW;
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::string m_metaRepoPath;
    std::string m_cacheRepoPath;
    NutanixRestoreExtendInfo m_restoreExtendInfo;
    // common
    std::string GetTaskId() const;
    // for restore
    int32_t DoPowerOffMachine(const VMInfo &vmInfo);
    int32_t DoPowerOnMachine(const VMInfo &vmInfo);
    bool CheckHostStatus(int32_t &statusFlag, const std::string &hostId);
    bool CheckHostBeforeRecover(const VMInfo &vmObj);
    int32_t GetCopyHostArch(const VMInfo &vmObj, std::string &copyArch);
    int32_t GetTargetHostArch(std::string &targetArch);
    int32_t CheckHostArchitectures(const VMInfo &vmObj, std::string &copyArch, std::string &targetArch);
    int32_t GetNewVMMachineName(std::string &vmName);
    bool CheckVMNameValidity(const std::string &vmName);
    int32_t CheckVMNameUnique(const std::string &vmName);
    int32_t CheckStoragePoolRestore(const std::vector<ApplicationResource> &restoreSubObjects);
    int32_t GetTargetContainerList(const ApplicationResource &targetVol,
        std::map< std::string, std::vector< std::string > > &poolList);
    int32_t InitParaAndGetTargetVolume(const ApplicationResource &targetVol, struct RestorTargetVolume &targetVolume);
    int32_t CheckTargetVolumeDatastoreParam(const struct RestorTargetVolume &targetVolume);
    bool GetStorageName(const std::string &containerId, std::string &containerName);
    int32_t QueryAgentHostClusterUUID(std::string &clusterUUID, std::string &agentHostName);
    void InitRepoHandler(void);
    int32_t CreateDiskByCopy(const VolInfo &copyVol, VolInfo &dstVolInfo, DatastoreInfo &storage);
    int32_t LoadCopyVolumeMatadata(const std::string &volId, VolInfo &volInfo);
    int32_t DoCreateMachine(VMInfo &vmInfo, CreateVMRequest &createVmReq);
    int32_t FormatCreateMachineParam(VMInfo &vmInfo, struct NutanixVMInfo &domainInfo,
        std::vector<struct NewVMNicStruct> &newVMNics);
    int32_t FormatDomainInterface(struct NutanixVMInfo &domainInfo, std::vector<struct NewVMNicStruct> &newVMNics);
    bool GetStorageUserFreeBytes(const Json::Value &usageStats, uint64_t &freeBytes);
    bool GetTargetNic(const std::string &originNicId, const std::vector<VmNic> &vmNics, NewVMNicStruct &newNic);
    bool CheckVolPreSnapshotValidity(const VmDiskInfo &currVol, const VolSnapInfo &snapshotVol);
};
}
#endif // APPPLUGINS_VIRTUALIZATION_NUTANIXPROTECTENGINE_H