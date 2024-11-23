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
#ifndef APSARA_STACK_PROTECT_ENGINE_H
#define APSARA_STACK_PROTECT_ENGINE_H

#include "log/Log.h"
#include "common/Structs.h"
#include "common/uuid/Uuid.h"
#include "protect_engines/ProtectEngine.h"
#include "protect_engines/apsara_stack/resource_discovery/ApsaraStackResourceAccess.h"
#include "protect_engines/openstack/OpenStackProtectEngine.h"
#include "volume_handlers/cloud_volume/apsara_volume/ApsaraVolumeHandler.h"

using namespace OpenStackPlugin;
namespace ApsaraStackPlugin {

class ApsaraStackProtectEngine : public OpenStackProtectEngine {
public:
    ApsaraStackProtectEngine()
        : OpenStackProtectEngine()
    {
        Utils::InitAndRegTracePoint();
    };

explicit ApsaraStackProtectEngine(std::shared_ptr<VirtPlugin::JobHandle> jobHandle, std::string jobId,
    std::string subJobId) : OpenStackProtectEngine(jobHandle, jobId, subJobId)
    {
        m_pythonObject =  m_subJobId.empty() ? m_jobId : m_subJobId;
        Utils::InitAndRegTracePoint();
    };

    ~ApsaraStackProtectEngine()
    {}

    int PreHook(const ExecHookParam &para) override;
    int PostHook(const ExecHookParam &para) override;
    
    // snapshot
    int32_t GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList) override;
    // meta and handle
    int32_t GetMachineMetadata(VMInfo &vmInfo) override;
    int32_t GetVolumesMetadata(const VMInfo &vmInfo,
        std::unordered_map<std::string, std::string> &volsMetadata) override;
    int32_t GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler) override;
    // volume
    int32_t ReplaceVolume(const VolInfo &volObj) override;
    // machine
    int32_t CreateMachine(VMInfo &vmInfo) override;
    int32_t DeleteMachine(const VMInfo &vmInfo) override;
    int32_t RenameMachine(const VMInfo &vmInfo, const std::string &newName) override;

    // scheduler
    int32_t AllowBackupInLocalNode(const AppProtect::BackupJob& job, int32_t &errorCode) override;
    int32_t AllowBackupSubJobInLocalNode(const AppProtect::BackupJob &job,
        const AppProtect::SubJob &subJob, int32_t &errorCode) override;
    int32_t AllowRestoreInLocalNode(const AppProtect::RestoreJob& job, int32_t &errorCode) override;
    int32_t AllowRestoreSubJobInLocalNode(const AppProtect::RestoreJob& job,
        const AppProtect::SubJob& subJob, int32_t &errorCode) override;
    // check
    int32_t CheckBeforeBackup() override;
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
    int32_t GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
        VolMatchPairInfo &volPairs);
    bool GetifDeleteSnapshot(AppProtect::BackupJobType backuptype, AppProtect::JobResult::type res) override;
    bool IfDeleteLatestSnapShot() override;

protected:
    PyObject* SetCommonPara();
    int32_t CheckEnvConnection(
        const AppProtect::ApplicationEnvironment &env, const std::string &vmId, int32_t &errorCode);
    // instance
    bool DoGetMachineMetadata();
    bool CheckVMStatus() override;
    bool GetInstanceDetail(const std::string &instanceId, Instance& ins);
    int32_t TransInstanceDetail2VMInfo(const Instance &ins, VMInfo &vmInfo);
    int32_t PowerOnMachineHandle(const VMInfo &vmInfo) override;
    bool GetAndCheckMachineStatus(const VMInfo &vmInfo, const std::string& status, int waitTimes);
    int32_t PowerOffMachine(const VMInfo &vmInfo) override;
    int32_t PowerOffMachineHandle(const VMInfo &vmInfo);
    // disk
    int32_t GetDisksOfInstance(const std::string &instanceId, DiskArray &diskList);
    bool CheckIfCreateSnapGroup(const std::vector<VolInfo> &volList);
    void TransDisk2VolInfo(const Disk &disk, VolInfo &volInfo);
    int32_t GetDiskDetail(const std::string &diskId, Disk &diskDetail, int32_t &erroCode);
    bool GetVolumeSnapshot(VolSnapInfo &volSnap) override;
    bool GetVolumeInfo(const std::string &volId, VolInfo &volInfo) override;
    int32_t DoCreateVolume(const VolInfo &backupVol, const std::string &volMetaData,
        const std::string &vmMoRef, const DatastoreInfo &dsInfo, VolInfo &newVol) override;
    bool CheckVolMode(const VolInfo &copyDisk, const Disk &targetDisk);
    bool CheckTargetVolume(const VolInfo &copyVolObj, const ApplicationResource &restoreVol) override;
    void DeleteVolumesAction(const std::string &file, NewCreatedVolumeList &deleteFailVolumes) override;
    int32_t DoDeleteVolume(const VolInfo &volObj) override;
    bool DeleteSnapshotCreateVolumes(const VolSnapInfo &snapInfo) override;
    bool FilterAndDeleteDisks(const DiskArray &diskList);
    bool CheckVolumesIfDelete(const VolResidualInfo &volResidualInfo) override;
    bool RequestDetachVolume(const std::string& serverId, const std::string& volumeId) override;
    int32_t DetachVolumeHandle(const VolInfo &volObj, const std::string& serverId);
    bool GetServerWhichAttachedVolume(const VolInfo &volInfo, std::string& serverId) override;
    bool CheckVolStatus(const Disk &diskInfo);
    bool RequestAttachVolume(const std::string& instanceId, const std::string& diskId,
        const std::string& device, const VolInfo &volObj) override;
    // snapshot
    bool CheckIsConsistenSnapshot() override;
    bool DoCreateSnapshot(const std::vector<VolInfo> &volList, SnapshotInfo &snapshot, std::string &errCode);
    int32_t GetSnapshotGroupDetail(const std::string &groupId, APSSnapshotGroup &snapshotGroup,
        SnapshotInfo &snapshot);
    bool CreateDiskSnapshot(const VolInfo &volInfo, VolSnapInfo &volSnap, std::string &errCode);
    bool CreateConsistencySnapshot(const std::vector<VolInfo> &volList,
        SnapshotInfo &snapshot, std::string &errCode);
    bool DoCreateCommonSnapshot(const std::vector<VolInfo> &volList,
        SnapshotInfo &snapshot, std::string &errCode);
    int32_t GetSnapshotDetail(const std::string snapshotId, APSSnapshot &snapshot, int32_t &err);
    bool ConfirmIfSnapDeleted(const std::string &snapId, int retryTimes, int &curConsumeRetryTimes) override;
    bool SendDeleteSnapshotMsg(const VolSnapInfo &volSnap) override;
    int32_t DoDeleteConsistencySnapshot(const SnapshotInfo &snapshot) override;
    void FillUpVolSnapInfo(const VolInfo &volInfo, const APSSnapshot &snapDetails, VolSnapInfo &volSnap);
    int32_t MatchSnapshotOfBackup(const APSSnapshot &snap);
    int32_t DeleteSnapshotGroup(const std::string &groupId);
    bool WaitSnapshotGroup(const CreateSnapshotGroupResponse &createRes, const std::string &snapGroupName,
        SnapshotInfo &snapshot);
    int32_t ActiveSnapConsistency(const SnapshotInfo& snapshotInfo) override;
    int32_t GetOrganizationInfo(const std::string &organizationId, Organization &org);

    Instance m_instance;
    CommonApsaraExtendInfo m_envParam;
    std::string m_regionId = "";
    std::string m_pythonObject;
};
    
} /* namespace  ApsaraStackPlugin */
#endif
