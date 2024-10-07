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
#ifndef __PROTECT_ENGINE_MOCK_H__
#define __PROTECT_ENGINE_MOCK_H__

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <map>
#include <string>
#include <memory>
#include <list>
#include <unordered_map>
#include <common/Structs.h>
#include <protect_engines/ProtectEngine.h>

namespace HDT_TEST {
class ProtectEngineMock : public VirtPlugin::ProtectEngine {
public:
    ProtectEngineMock(std::shared_ptr<VirtPlugin::JobHandle> jobHandle) : ProtectEngine(jobHandle, "", "") {}
    ProtectEngineMock() : ProtectEngine() {}
    virtual ~ProtectEngineMock() {}

    MOCK_METHOD(std::shared_ptr<VirtPlugin::JobHandle>, GetJobHandle, (), (const));
    MOCK_METHOD(int32_t, PreHook, (const VirtPlugin::ExecHookParam &para), (override));
    MOCK_METHOD(int32_t, PostHook, (const VirtPlugin::ExecHookParam &para), (override));
    MOCK_METHOD(int32_t, CreateSnapshot, (VirtPlugin::SnapshotInfo &snapshot, std::string &errCode), (override));
    MOCK_METHOD(int32_t, DeleteSnapshot, (const VirtPlugin::SnapshotInfo &snapshot), (override));
    MOCK_METHOD(int32_t, QuerySnapshotExists, (VirtPlugin::SnapshotInfo &snapshot), (override));
    MOCK_METHOD(int32_t, GetMachineMetadata, (VirtPlugin::VMInfo &vmInfo), (override));
    using VolMetadataMapType = std::unordered_map<std::string, std::string>;
    MOCK_METHOD(int32_t, GetVolumesMetadata, (const VirtPlugin::VMInfo &vmInfo, VolMetadataMapType &volsMetadata), (override));
    MOCK_METHOD(int32_t, CreateVolume, (const VirtPlugin::VolInfo &volObj, const std::string &volMetaData, const std::string &vmMoRef,
                                        const VirtPlugin::DatastoreInfo &dsInfo, VirtPlugin::VolInfo &newVol), (override));
    MOCK_METHOD(int32_t, DetachVolume, (const VirtPlugin::VolInfo &volObj), (override));
    MOCK_METHOD(int32_t, AttachVolume, (const VirtPlugin::VolInfo &volObj), (override));
    MOCK_METHOD(int32_t, DeleteVolume, (const VirtPlugin::VolInfo &volObj), (override));
    MOCK_METHOD(int32_t, ReplaceVolume, (const VirtPlugin::VolInfo &volObj), (override));
    using volDataMap = const std::unordered_map<std::string, std::string>;
    MOCK_METHOD(int32_t, CreateMachine, (VirtPlugin::VMInfo &vmInfo), (override));
    MOCK_METHOD(int32_t, DeleteMachine, (const VirtPlugin::VMInfo &vmInfo), (override));
    MOCK_METHOD(int32_t, RenameMachine, (const VirtPlugin::VMInfo &vmInfo, const std::string &newName), (override));
    MOCK_METHOD(int32_t, PowerOnMachine, (const VirtPlugin::VMInfo &vmInfo), (override));
    MOCK_METHOD(int32_t, PowerOffMachine, (const VirtPlugin::VMInfo &vmInfo), (override));
    MOCK_METHOD(int32_t, AllowBackupInLocalNode, (const AppProtect::BackupJob& job, int32_t &errorCode), (override));
    MOCK_METHOD(int32_t, AllowBackupSubJobInLocalNode, (const AppProtect::BackupJob& job, const AppProtect::SubJob& subJob, int32_t &errorCode), (override));
    MOCK_METHOD(int32_t, AllowRestoreInLocalNode, (const AppProtect::RestoreJob& job, int32_t &errorCode), (override));
    MOCK_METHOD(int32_t, AllowRestoreSubJobInLocalNode, (const AppProtect::RestoreJob& job, const AppProtect::SubJob& subJob, int32_t &errorCode), (override));
    MOCK_METHOD(int32_t, CheckBackupJobType, (const VirtPlugin::JobTypeParam& jobTypeParam, bool& checkRet), (override));
    MOCK_METHOD(int32_t, GetVolumeHandler, (const VirtPlugin::VolInfo &volInfo, std::shared_ptr<VirtPlugin::VolumeHandler> &volHandler), (override));
    using metaData = std::unordered_map<std::string, std::string>;
    using volData = std::vector<VirtPlugin::VolInfo>;
    MOCK_METHOD(void, CheckApplication, (AppProtect::ActionResult &_return, const AppProtect::ApplicationEnvironment &appEnv,
        const AppProtect::Application &application), (override));
    MOCK_METHOD(void, DiscoverApplications, (std::vector<Application>& returnValue, const std::string& appType), (override));
    MOCK_METHOD(void, ListApplicationResource, (std::vector<ApplicationResource>& returnValue, const ApplicationEnvironment& appEnv,
        const Application& application, const ApplicationResource& parentResource), (override));
    MOCK_METHOD(void, ListApplicationResourceV2, (ResourceResultByPage &page, const ListResourceRequest &request), (override));
    MOCK_METHOD(void, DiscoverHostCluster, (ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv), (override));
    MOCK_METHOD(void, DiscoverAppCluster, (ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv, const Application& application), (override));
    MOCK_METHOD(int32_t, CheckBeforeMount, (), (override));
    MOCK_METHOD(int32_t, CancelLiveMount, (const VirtPlugin::VMInfo &liveVm), (override));
    MOCK_METHOD(int32_t, CreateLiveMount, (const VirtPlugin::VMInfo &copyVm, VirtPlugin::VMInfo &newVm), (override));
    MOCK_METHOD(int32_t, MigrateLiveVolume, (const VirtPlugin::VMInfo &liveVm), (override));
    MOCK_METHOD(int, GenVolPair, (VirtPlugin::VMInfo &vmObj, const VirtPlugin::VolInfo &copyVol, const ApplicationResource &targetVol,
        VirtPlugin::VolMatchPairInfo &volPairs));
    MOCK_METHOD(int32_t, CheckBeforeRecover, (const VirtPlugin::VMInfo &vmObj));
    MOCK_METHOD(int32_t, GetSnapshotsOfVolume, (const VirtPlugin::VolInfo &volInfo, std::vector<VirtPlugin::VolSnapInfo> &snapList));
};
}

#endif //__PROTECT_ENGINE_MOCK_H__