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
#ifndef APPPLUGINS_VIRTUALIZATION_CNWAREPROTECTENGINE_H
#define APPPLUGINS_VIRTUALIZATION_CNWAREPROTECTENGINE_H
#include "log/Log.h"
#include "common/cert_mgr/CertMgr.h"
#include "common/uuid/Uuid.h"
#include "common/utils/Utils.h"
#include "curl_http/HttpStatus.h"
#include "job_controller/jobs/backup/BackupJob.h"
#include "protect_engines/ProtectEngine.h"
#include "protect_engines/cnware/common/ErrorCode.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "protect_engines/cnware/resource/CNwareResourceManager.h"
#include "volume_handlers/cloud_volume/cnware_volume/CNwareVolumeHandler.h"

using namespace VirtPlugin;

namespace CNwarePlugin {
extern std::mutex g_attachVolumeMutex;
class CNwareProtectEngine : public ProtectEngine {
public:
    CNwareProtectEngine()
    {
    }
    explicit CNwareProtectEngine(std::shared_ptr<VirtPlugin::JobHandle> &m_jobHandle,
        std::string jobId, std::string subJobId) : ProtectEngine(m_jobHandle, jobId, subJobId)
    {
    }

    ~CNwareProtectEngine() {};
    bool Init(const bool &idFlag = true);
    int PreHook(const ExecHookParam &para) override;
    int PostHook(const ExecHookParam &para) override;

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
    int32_t DoCreateVolume(const VolInfo &backupVol, const std::string &volMetaData,
        const std::string &vmMoRef, const DatastoreInfo &storage, VolInfo &newVol);
    int32_t DetachVolume(const VolInfo &volObj) override;
    int32_t DoDetachVolume(const VolInfo &volObj);
    int32_t AttachVolume(const VolInfo &volObj) override;
    int32_t DoAttachVolume(const VolInfo &volObj);
    int32_t DeleteVolume(const VolInfo &volObj) override;
    int32_t ReplaceVolume(const VolInfo &volObj) override;
    // machine
    int32_t CreateMachine(VMInfo &vmInfo) override;
    int32_t DoCreateMachine(VMInfo &vmInfo,
        const AddDomainRequest &addDomainInfo, BuildNewVMRequest &createVmReq);
    int32_t DeleteMachine(const VMInfo &vmInfo) override;
    int32_t RenameMachine(const VMInfo &vmInfo, const std::string &newName) override;
    int32_t PowerOnMachine(const VMInfo &vmInfo) override;
    int32_t DoPowerOnMachine(const VMInfo &vmInfo);
    int32_t PowerOffMachine(const VMInfo &vmInfo) override;
    int32_t DoPowerOffMachine(const VMInfo &vmInfo);
    int32_t AddBridgeInterface(const std::string &vmId, std::vector<AddBridgeInterfaceRequest> &InterfaceList);
    int32_t DoAddNetworkCard(const std::string vmId, const AddBridgeInterfaceRequest &req);

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

    int GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
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
    bool AppInitClient(int64_t& errorCode, std::string &errorDes);
    int32_t FindSourceVol(const std::string &id, VolInfo &volInfo);
    int32_t FindTargetVol(const std::string &id, const VMInfo &vmInfo, VolInfo &targetVol);
    int32_t RestoreVolMetadata(VolMatchPairInfo &volPairs, const VMInfo &vmInfo) override;

protected:
    void SetAppEnv(const ApplicationEnvironment &appEnv);
    void SetCommonInfo(CNwareRequest &req);
    bool CheckTaskStatus(const std::string &taskId, const bool cache = false);
    int32_t PrepareStorage(std::string &storeId, std::string &storageId);
    int32_t PrepareStorageDevice(std::string &storeId);
    int32_t PrepareStoragePool(const std::string &storeId, std::string &resourceId,
        std::string &storageId, std::string &liveStorageName);
    int32_t DoCreateLiveMount(const VMInfo &copyVm, VMInfo &newVm,
        std::string &storeId, std::string &storageId);
    // preHook
    bool InitJobPara();
    bool InitCancelLiveMountJobPara(const std::shared_ptr<ThriftDataBase> &jobInfo);
    bool InitLiveMountJobPara();
    bool InitRestoreJobPara(std::shared_ptr<ThriftDataBase> &jobInfo);
    void InitRepoHandler();
    void FormVolumeInfo(const Json::Value &targetVolume, VolInfo &volObj, DatastoreInfo &storage);
    int32_t LoadCopyVolumeMatadata(const std::string &volId, VolInfo &volInfo);
    int32_t InitParaAndGetTargetVolume(const ApplicationResource &targetVol, Json::Value &targetVolume);
    int32_t GetDiskFromVMInfo(const std::string &volName, VolInfo &newVol);
    bool ParseVolume(const DomainDiskDevicesResp &items, VolInfo &newVol, const std::string &volName);
    bool ParseStorage(const Json::Value &targetVolume, DatastoreInfo &storage, StoragePool &pool);
    void FinishLastTaskCache();
    bool FinishLastMigrate();

    // common
    template<typename T>
    bool CommonCheckResponse(const T &response)
    {
        if (response == nullptr) {
            ERRLOG("Response is nullptr, %s", m_taskId.c_str());
            return false;
        }
        if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
            ERRLOG("Response is not ok. http status code: %d, body : %s, %s", response->GetStatusCode(),
                (WIPE_SENSITIVE(response->GetBody())).c_str(), m_taskId.c_str());
            return true;
        }
    }
    bool CheckCNwareVersion();
    int32_t GetCNwareVersion(std::string &version);
    void SetOsVersion(AddDomainRequest &domainInfo);
    int32_t ConnectToEnv(const AppProtect::ApplicationEnvironment &env);
    int32_t CheckStorageConnectionBackup(const VolInfo &volInfo, int32_t &erroCode);
    void CheckCertIsExist(int32_t &errorCode);
    int32_t CheckProtectEnvConn(const AppProtect::ApplicationEnvironment &env,
        const std::string &vmId, int32_t &errorCode);
    bool CheckVolIsInStorage(const std::string &volName, const std::string &poolId);
    int32_t GetWhiteListForLivemount(std::string &ipStr) override;
    int32_t GetIpStrForLivemount();
    int32_t GetNewVMMachineInfo();
    int32_t FormatLiveDiskDev(VMInfo vmInfo, AddDomainRequest &domainInfo);
    bool CheckStore(const std::string &nfsIp, std::string &storeId);
    bool CheckDiskStatus(const DomainDiskInfoResponse &diskInfo);
    bool CheckLunDisk(const DomainDiskInfoResponse &diskInfo);
    bool CheckDisk();
    std::string GetNameFromSourcefile(std::string sourceName);
    int32_t QueryVmInfoByName(const std::string &name, DomainListResponse &dInfo);
    bool CheckHostStatus(int32_t &statusFlag, const std::string &hostId);
    bool CheckHostBeforeRecover(const VMInfo &vmObj);
    bool CheckCpuLimit(const uint32_t &coresNum);
    void SetConfigInfo(AddDomainRequest &domainInfo);
    int32_t HealthDomain(const std::string &domainId);
    int32_t ProcessBridgeInterface(const std::string &vmId, const VMInfo &vmInfo);
    int32_t ShutDownBridgeInterface(const std::string &vmId);
    bool DoShutDownBridgeInterface(const std::string &vmId, const BridgeInterfaces &brigde);
    bool ListDev(std::vector<std::string> &devList);
    bool CheckDetachDevList(const std::vector<std::string> &beforeDevList);
    bool GetNewAttachDev(const std::vector<std::string> &beforeDevList);
    std::string GetAppHostId();
    std::string GetAppHostName();
    std::string GetHostName();
    int32_t GetStorageDriverType(const int32_t &poolType, const int32_t &originType);
    std::string GetStoragePreallocation(const int32_t &poolType,
        const std::string &originAlloc, const std::string &diskName = "");
    void ConfigDomainInfo(const CNwareVMInfo &cwVmInfo, AddDomainRequest &domainInfo);
    void DealCephSnapshot(const std::string &cephTaskId);
    bool CheckVMNotMigrate(const std::string &domainId);

    // volume
    bool GetVolInfoOnStorage(const std::string &volId, CNwareDiskInfo &volumeDetail);
    std::string GetBusDevStr(const std::string &volId, const DomainDiskInfoResponse &disksInfo);
    bool CheckBusDevStr(const std::string &busDevStr, const DomainDiskInfoResponse &disksInfo);
    bool DoDetachVolumeOnVm(const std::string &volId, const std::string &domainId);
    bool DetachVolumeHandle(const CNwareDiskInfo &volumeDetail);
    bool DoDeleteVolume(const std::string &volId);
    void DeleteVolumes();
    int32_t DoDeleteMachine(const VMInfo &vmInfo);
    bool QueryCreateSnapshotTask(const std::string &snapShotTaskId,
        std::vector<SnapshotDiskInfo> &snapshotInfoList);
    bool SetVolSnapInfo(const SnapshotDiskInfo &disk, VolSnapInfo &volSnap,
        const std::string &snapShotTaskId);
    void FillTargetDisksToSnapshotMsg(CustomDomainSnapshotReq &reqCustom);
    bool GetSnapshotResult(const std::string &snapShotTaskId, SnapshotInfo &snapshot, std::string &errCode);
    bool DelVolumeByVolId(const std::string &volId);
    int32_t DeleteCommonSnapshot(const SnapshotInfo &snapshot);
    int32_t DeleteCephSnapshot(const std::string &cephSnapFlag);
    bool GetVolDevOnVm(const std::string &volId, const std::string &vmId, std::string &dev);
    bool GetSnapshotOriginVolInfo(const std::string &volId, DomainDiskDevicesResp &deviceInfo);
    int32_t DoGetDiskFromVMInfo(const DomainDiskInfoResponse &disksInfo,
        const std::string &volName, VolInfo &newVol);
    bool FormLiveVolumeMap(const VMInfo &liveVm, const std::string &storageId,
        const std::string &storageName);
    int32_t FormatLiveInterface(AddDomainRequest &domainInfo);
    int32_t CreateStoragePool(std::string &name,
        const std::string &hostId, const std::string &resourceId, std::string &storageId);
    int32_t DoCreateStoragePool(const std::string &name,
        const std::string &hostId, const std::string &resourceId);
    int32_t ScanNfsStorage(const std::string &storeId);
    int32_t GetStorageId(const std::string &hostId, const std::string &resourceId,
        const std::string &name, const NameType &nameType, std::string &storageId);
    int32_t GetResStoragePool(const std::string &hostId, const std::string &resourceId, const std::string &name,
        const NameType &nameType, StoragePool &storagePool);
    int32_t GetResourceId(const std::string &name, const std::string &storeId, std::string &resourceId);
    int32_t GetResourceIdRetry(const std::string &storeId, std::string &resourceId);
    int32_t AddNfs(const std::string &name, const std::string &hostId, const std::string &nfsIp);
    int32_t RefreshStoragePool(const std::string &storageId);

    void SetBootAbleDisk(std::vector<VolInfo> &vmVolList, uint32_t bootType);
    bool FillSpecifiedTargetVolsDeviceInfo(
        const AppProtect::BackupJob &job, std::vector<DomainDiskDevicesResp> &targetVolsInfo);
    bool FillTargetVolsDeviceInfo(
        const AppProtect::BackupJob &job, std::vector<DomainDiskDevicesResp> &targetVolsInfo);
    bool CheckVolPreSnapshotValidity(const DomainDiskDevicesResp &currVol, const VolSnapInfo &snapshotVol);
    bool CheckPreSnapShotValidity(const AppProtect::BackupJob &job, const std::vector<VolSnapInfo> &lastVolList);
    int32_t CreateDiskByCopy(const VolInfo &copyVol, VolInfo &dstVolInfo, DatastoreInfo &storage);
    bool AddLiveVolMigReq(const DomainDiskInfoResponse &liveVols,
        const ApplicationResource &sub, MigrationRequest &migReq);
    int32_t DoMigrateLiveVolume(const VMInfo &liveVm);
    int32_t PutInMigrateParams(MigrationRequest &migReq, const std::string &domainId,
        std::shared_ptr<GetVMDiskInfoResponse> &diskResponse);
    bool GetStorageName(const std::string &poolId, std::string &poolName);
    bool GetHostName(const std::string &hostId, std::string &hostName);
    bool CheckPoolExists(std::string &liveStorageName, const std::string &hostId,
        const std::string &resourceId, std::string &storageId);

    // machine
    bool GetVMInfoById(const std::string &domainId, CNwareVMInfo &cnVmInfo);
    void SetCNwareVMInfo2VMInfo(const CNwareVMInfo &cnVmInfo, VMInfo &vmInfo);
    bool SetVolListInfo2VMInfo(const DomainDiskInfoResponse &disksInfo, VMInfo &vmInfo);
    bool DoGetMachineMetadata(const std::string &vmId);
    bool CheckVMStatus();
    bool ModifyDevBoots(const string &vmId, const UpdateDomainBootRequest &UpdateReq);
    bool ModifyVMDevBoots(const DomainListResponse &domainInfo, VMInfo vmInfo);
    bool ModifyVMDevBootsForDiskRestore();
    bool ModifyLiveDevBoots(const DomainListResponse &domainInfo, VMInfo vmInfo);
    bool ModifyBootDisk(const VolInfo &volObj, const std::string &vmId);
    void WaitVMTaskEmpty(const std::string &domainId);

    int32_t PostProcessCreateMachine(VMInfo &vmInfo);
    int32_t FormatLiveMachineParam(const VMInfo &vmInfo, AddDomainRequest &domainInfo);
    int32_t BuildLiveVm(const VMInfo &liveVm, VMInfo &newVm,
        const std::string &storageId, const std::string &storageName);
    int32_t PostOperationLiveMount(const VMInfo &liveVm, VMInfo &newVm,
        const DomainListResponse &dInfo);
    int32_t DoBuildLiveVm(const VMInfo &liveVm, VMInfo &newVm,
        const std::string &storageId, const std::string &storageName);
    int32_t GetTargetDomainId(std::string &domainId);
    int32_t FormatCreateMachineParam(VMInfo &vmInfo, AddDomainRequest &domainInfo);
    int32_t GetNewVMMachineName(std::string &vmName);
    int32_t FormatDomainDiskDev(VMInfo &vmInfo, AddDomainRequest &domainInfo);
    int32_t FormatDomainInterface(AddDomainRequest &domainInfo);
    int32_t GenVolPairLocationNew(
        VMInfo &vmObj, const VolInfo &copyVol, const std::string &dstVolId, VolMatchPairInfo &volPairs);
    int32_t GenVolPairLocationOriginal(
        VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol, VolMatchPairInfo &volPairs);
    int32_t DoGenVolPairForDiskRestore(
        VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol, VolMatchPairInfo &volPairs);
    bool CheckIsNewDisk(const VolInfo &copyVol, Json::Value &targetVolume, VolMatchPairInfo &volPairs);
    bool CheckVMNameValidity(const std::string &vmName);
    int32_t CheckVMNameUnique(const std::string &vmName);
    int32_t CheckHostArchitectures(const VMInfo &vmObj, std::string &copyArch, std::string &targetArch);
    int32_t GetCopyHostArch(const VMInfo &vmObj, uint32_t &iCopyArch, std::string &copyArch);
    int32_t GetTargetHostArch(uint32_t &iTargetHostArch, std::string &targetArch);
    int32_t CheckStoragePoolRestore(const std::vector<ApplicationResource> &restoreSubObjects);
    int32_t FindTargetPool(
        const std::string &volName, const std::string &poolId, const StoragePoolInfo &poolInfo);
    int32_t GetTargetPoolIDList(const ApplicationResource &targetVol,
        std::set<std::tuple<std::string, std::string, std::string>> &poolList);
    bool GetReserveCopyOption(bool &isReserved);
    DeleteVMType GetDeleteVMType();
    int32_t RestoreSrcToDstDiskMetadata(const VolInfo &srcVol, const VolInfo &dstVol, const VMInfo &vmInfo,
        const DomainDiskInfoResponse &vmDisks);
    int32_t DoRestoreVolMetadata(const string &domainId, const DomainDiskDevicesResp &srcDiskInfo,
        const DomainDiskDevicesResp &dstDiskInfo, const std::string &volId,
        const DomainDiskInfoResponse &vmDisks);
    bool CheckHostResourceUsage(const std::string &hostId);
    bool GetStoragePoolOfSpecifiedVol(const std::string volDev,
        std::shared_ptr<GetVMDiskInfoResponse> &resp, std::unordered_set<std::string> &poolList);
    bool GetTargetStoragePoolListBackup(std::unordered_set<std::string> &poolList);
    bool CheckStorageUsage(const StoragePool &pool, const int32_t &storageLimit);
    int32_t GetStorageLimit();
    bool CheckStorage();

    // cancel live mount
    void SaveLiveMountStorageInfo(VMInfo &liveVm, std::string &storeId, std::string &storageId);
    bool GetLiveMountStorageInfo(const VMInfo &liveVm, std::string &storeId, std::string &storageId);
    bool RemoveStorage(const std::string &storageId);
    int32_t QueryAgentHostStoragePoolList(StoragePoolInfo &poolInfo, const std::string &poolId = "");
    int32_t GetTargetVolumeDatastoreParam(
        const Json::Value &targetVolume, DatastoreInfo &storage, bool existingDiskRestore);
    std::string GetPreallocation(const VolInfo &volObj, const DomainDiskDevicesResp &diskInfo);

protected:
    std::shared_ptr<CNwareClient> m_cnwareClient = nullptr;
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
    AppProtect::Copy m_copy;
    std::vector<ApplicationResource> m_subObjects;
    ConfigLivemount m_config;
    int32_t m_commonWaitInterval {10};
    int32_t m_commonWaitTimes {30};
    bool m_machineMetaCached { false };
    VMInfo m_vmInfo;
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::string m_metaRepoPath;
    std::string m_cacheRepoPath;
    std::string m_livemountRepoPath;
    std::string m_livemountStoreName;
    RestoreLevel m_restoreLevel = RestoreLevel::RESTORE_TYPE_UNKNOW;
    std::map<std::string, std::string> m_volNameToId;
    VMInfo m_newVm;
    std::string m_targetLocation; // original / new
    Json::Value m_jobAdvPara;
    std::string m_nfsAddress;
    std::map<std::string, DomainDiskDevicesReq> m_volIdToReq;
    int32_t m_hostCpuCores {0};
    int32_t m_targetVMCpuCurrent {0};
    int32_t m_restoreAttachVol2TargetVMCnt {0};
    std::string m_serverName; // agent vm name in cnware
    std::string m_tempLiveName;
    bool m_machineNeedRename {false};
    bool m_isCeph {false};
    int64_t m_checkErrorCode = -1;

private:
    // common
    std::string GetTaskId() const;
};
}
#endif // APPPLUGINS_VIRTUALIZATION_CNWAREPROTECTENGINE_H
