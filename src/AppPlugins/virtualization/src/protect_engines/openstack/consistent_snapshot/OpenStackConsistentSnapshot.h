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
#ifndef OPENSTACK_CONSISTENT_SNAPSHOT_H
#define OPENSTACK_CONSISTENT_SNAPSHOT_H

#include <vector>
#include "common/Structs.h"
#include "common/utils/Utils.h"
#include "common/cert_mgr/CertMgr.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/api/nova/NovaClient.h"
#include "protect_engines/openstack/api/cinder/CinderClient.h"
#include "repository_handlers/factory/RepositoryFactory.h"
 
using AppProtect::Application;
using AppProtect::ApplicationEnvironment;
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class OpenStackConsistentSnapshot {
public:
    explicit OpenStackConsistentSnapshot(ApplicationEnvironment appEnv, Application application);
    ~OpenStackConsistentSnapshot();
 
    bool DoCreateConsistencySnapshot(const std::vector<VolInfo> &volList,
        SnapshotInfo &snapshot, std::string &errCode);
    bool InitParam(std::vector<AppProtect::StorageRepository> backupRepos,
        const std::shared_ptr<VirtPlugin::CertManger> &certMgr, const std::string &requestId);
    int32_t DeleteConsistencySnapshot(const SnapshotInfo &snapshot);
    int32_t QueryConsistencySnapshotExists(SnapshotInfo &snapshot);
    bool DoDeleteGroupVolume(ConsistentSnapshotGroupInfo &groupInfo);
 
private:
    template<typename T>
    bool SetOpenStackRequest(T &request);
    bool LoadConsistentSnapshotInfoFromFile(ConsistentSnapshotGroupInfo &groupInfo);
    bool UpdateConsistentSnapshotInfoToFile(ConsistentSnapshotGroupInfo &newGroupInfo, bool isDeleteGroupInfo = false);
    bool DoGetVolumeTypeId(const std::vector<VolInfo> &volList, ConsistentSnapshotGroupInfo &groupInfo);
    bool DoCreateGroupType();
    std::string GetGroupTypeName();
    std::string GetGroupName();
    int32_t CreateNewGroupType(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t DoCreateNewVolumeGroup(ConsistentSnapshotGroupInfo &groupInfo);
    bool DoCreateVolumeGroup(const std::vector<VolInfo> &volList);
    int32_t AddListToVolumeGroup(const std::vector<VolInfo> &volList, ConsistentSnapshotGroupInfo &groupInfo);
    int32_t CreateVolumeGroupSnapShot(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t DoWaitGroupSnapshotStatusAvailable(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t DoGetGroupSnapshotStatus(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t DoGetSnapShotList(const std::vector<VolInfo> &volList, SnapshotInfo &snapshot,
        ConsistentSnapshotGroupInfo &groupInfo);
    int32_t DoDeleteVolumeGroup(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t DoDeleteGroupType(ConsistentSnapshotGroupInfo &groupInfo);
    bool DoAddVolumeListToGroup(const std::vector<VolInfo> &volList);
    bool DoCreateVolumeGroupSnapShot();
    std::string GetGroupSnapshotName();
    int32_t RemoveVolumeFromGroup(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t DoGetGroupStatus(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t ConfirmVolumeGroupNotExist(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t DoDeleteGroupSnapshot(ConsistentSnapshotGroupInfo &groupInfo);
    int32_t ConfirmGroupSnapshotNotExist(const ConsistentSnapshotGroupInfo &groupInfo);
 
private:
    ApplicationEnvironment m_appEnv;
    Application m_application;
    OpenStackPlugin::CinderClient m_cinderClient;
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::string m_metaRepoPath;
    std::shared_ptr<VirtPlugin::CertManger> m_certMgr;
    std::string m_consistentSnapshotFile;
    ConsistentSnapshotGroupInfo m_groupInfo;
    ConsistentSnapshotGroupInfoList m_consistentGroupInfolist;
    std::string m_requestId;
    std::string m_apiVersion;
};
 
OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif