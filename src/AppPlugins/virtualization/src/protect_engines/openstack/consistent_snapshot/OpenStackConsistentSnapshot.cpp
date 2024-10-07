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
#include "OpenStackConsistentSnapshot.h"
#include <thread>
#include "config_reader/ConfigIniReader.h"
 
namespace {
std::mutex m_operateGroupFileLock;
const std::string API_VERSION  = "CinderApiVersion";
const std::string OPENSTACK_CONF = "OpenStackConfig";
const std::string GROUP_TYPE_DESCRIPTION = "The group type created by the backup";
const std::string OPENSTAK_AVAILABLE_STATUS = "available";
constexpr uint32_t GB_SIZE = 1024 * 1024 * 1024UL;
const int32_t RETRY_MAX_TIMES = 5;
const int32_t SLEEP_TIMES = 30;
}
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
OpenStackConsistentSnapshot::OpenStackConsistentSnapshot(ApplicationEnvironment appEnv,
    Application application) : m_appEnv(appEnv), m_application(application)
{}
 
OpenStackConsistentSnapshot::~OpenStackConsistentSnapshot()
{}
 
template<typename T>
bool OpenStackConsistentSnapshot::SetOpenStackRequest(T &request)
{
    Json::Value envExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_application.extendInfo, envExtendInfo)) {
        ERRLOG("Failed to convert extend info.");
        return false;
    }
    std::string domainName = envExtendInfo["domainName"].asString();
    if (domainName.empty()) {
        ERRLOG("No domain name.");
        return false;
    }
    AuthObj auth;
    request.SetScopeValue(m_application.parentId);
    request.SetDomain(domainName);
    auth.name = m_application.auth.authkey;
    auth.passwd = m_application.auth.authPwd;
    auth.certVerifyEnable = m_certMgr->IsVerifyCert();
    auth.cert = m_certMgr->GetCertPath();
    auth.revocationList = m_certMgr->GetRevocationListPath();
    request.SetUserInfo(auth);
    request.SetEnvAddress(m_appEnv.endpoint);
    return true;
}

// 初始化备份参数
bool OpenStackConsistentSnapshot::InitParam(std::vector<AppProtect::StorageRepository> backupRepos,
    const std::shared_ptr<VirtPlugin::CertManger> &certMgr, const std::string &requestId)
{
    for (const auto &repo : backupRepos) {
        if (repo.repositoryType == RepositoryDataType::META_REPOSITORY) {
            m_metaRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);
            if (m_metaRepoHandler == nullptr) {
                ERRLOG("Create meta repository failed.");
                return false;
            }
            m_metaRepoPath = repo.path[0];
            break;
        }
    }
    
    if (m_metaRepoPath.empty()) {
        ERRLOG("Get meta repo path failed.");
        return false;
    }

    std::string metaDir = m_metaRepoPath + VIRT_PLUGIN_META_ROOT;
    if (!m_metaRepoHandler->IsDirectory(metaDir) && !m_metaRepoHandler->CreateDirectory(metaDir)) {
        ERRLOG("Create Directory(%s) failed!", metaDir.c_str());
        return false;
    }

    m_consistentSnapshotFile = m_metaRepoPath + VIRT_PLUGIN_GROUP_INFO;   // 记录一致性快照组信息文件
    m_certMgr = certMgr;
    m_requestId = requestId;
    m_groupInfo.m_uuid = m_requestId;
    m_apiVersion = Module::ConfigReader::getString(OPENSTACK_CONF, API_VERSION);
    if (m_apiVersion.empty()) {
        ERRLOG("Get cinder api version failed.");
        return false;
    }
 
    return true;
}

// 将快照卷组信息记录卷组接口函数
bool OpenStackConsistentSnapshot::LoadConsistentSnapshotInfoFromFile(ConsistentSnapshotGroupInfo &groupInfo)
{
    if (m_metaRepoHandler->Exists(m_consistentSnapshotFile)) {
        std::lock_guard<std::mutex> lock(m_operateGroupFileLock);
        if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, m_consistentSnapshotFile,
            m_consistentGroupInfolist) != SUCCESS) {
            ERRLOG("Load consistent group info list failed.");
            return false;
        }

        for (auto groupInfoItem : m_consistentGroupInfolist.m_consistentGroupInfos) {
            if (groupInfoItem.m_uuid == m_requestId) {  // 依据任务ID判断是否存在该任务的信息
                INFOLOG("Get consistent snapshot group %s info success.", groupInfoItem.m_uuid.c_str());
                groupInfo = groupInfoItem;
                return true;
            }
        }
    }
    ERRLOG("Not found consistent snapshot group info, %s.", m_requestId.c_str());
    return false;
}
 
// 更新文件中记录的卷组信息
bool OpenStackConsistentSnapshot::UpdateConsistentSnapshotInfoToFile(ConsistentSnapshotGroupInfo &newGroupInfo,
    bool isDeleteGroupInfo)
{
    std::lock_guard<std::mutex> lock(m_operateGroupFileLock);
    if (m_metaRepoHandler->Exists(m_consistentSnapshotFile)) {
        if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, m_consistentSnapshotFile,
            m_consistentGroupInfolist) != SUCCESS) {
            ERRLOG("Load consistent group info list failed.");
            return false;
        }
 
        std::vector<ConsistentSnapshotGroupInfo>::iterator it;
        for (it = m_consistentGroupInfolist.m_consistentGroupInfos.begin();
            it != m_consistentGroupInfolist.m_consistentGroupInfos.end(); it++) {
            if ((*it).m_uuid == newGroupInfo.m_uuid) {
                m_consistentGroupInfolist.m_consistentGroupInfos.erase(it);
                break;
            }
        }
    }
 
    if (!isDeleteGroupInfo) {
        m_consistentGroupInfolist.m_consistentGroupInfos.push_back(newGroupInfo);
    }
 
    std::string newConsistentGroupListStr;
    if (!Module::JsonHelper::StructToJsonString(m_consistentGroupInfolist, newConsistentGroupListStr)) {
        ERRLOG("Convert consistent snapshot group to json string failed.");
        return false;
    }

    if (Utils::SaveToFileWithRetry(m_metaRepoHandler, m_consistentSnapshotFile,
        newConsistentGroupListStr) != SUCCESS) {
        ERRLOG("Save consistent snapshot group info list failed.");
        return false;
    }
    return true;
}
 
// 创建一致性快照
bool OpenStackConsistentSnapshot::DoCreateConsistencySnapshot(const std::vector<VolInfo> &volList,
    SnapshotInfo &snapshot, std::string &errCode)
{
    // 读取文件记录中的groupInfo消息
    LoadConsistentSnapshotInfoFromFile(m_groupInfo);
    if (!DoCreateGroupType()) {
        ERRLOG("Create Group Type Failed.");
        return false;
    }
 
    if (!DoCreateVolumeGroup(volList)) {
        ERRLOG("Create volume group failed.");
        return false;
    }
 
    if (!DoAddVolumeListToGroup(volList)) {
        ERRLOG("Add list to volume group failed.");
        return false;
    }
 
    if (!DoCreateVolumeGroupSnapShot()) {
        ERRLOG("Create Group snapshot failed.");
        return false;
    }
 
    if (DoWaitGroupSnapshotStatusAvailable(m_groupInfo) != SUCCESS) {
        ERRLOG("Group Snapshot status is not avaliable.");
        return false;
    }
 
    if (DoGetSnapShotList(volList, snapshot, m_groupInfo) != SUCCESS) {
        ERRLOG("Get snapshot list failed.");
        return false;
    }
    return true;
}
 
// 删除一执行快照接口
int32_t OpenStackConsistentSnapshot::DeleteConsistencySnapshot(const SnapshotInfo &snapshot)
{
    if (!LoadConsistentSnapshotInfoFromFile(m_groupInfo)) {
        ERRLOG("Not foud the volume group info. %s.", m_requestId.c_str());
        return FAILED;
    }

    if (m_groupInfo.m_isGroupSnapshotCreated && DoDeleteGroupSnapshot(m_groupInfo) != SUCCESS) {
        ERRLOG("Delete Group Snapshot (%s) Failed. %s.", snapshot.m_consistengroupId.c_str(), m_requestId.c_str());
        return FAILED;
    }

    if (!DoDeleteGroupVolume(m_groupInfo)) {
        ERRLOG("Delete Group Volume(%s) Failed. %s.", m_groupInfo.m_groupName.c_str(), m_requestId.c_str());
        return FAILED;
    }
    return SUCCESS;
}
 
std::string OpenStackConsistentSnapshot::GetGroupTypeName()
{
    return "Protect_" + m_requestId + "_GROUPTYPE";
}
 
std::string OpenStackConsistentSnapshot::GetGroupName()
{
    return "Protect_" + m_requestId + "_GROUP";
}
 
std::string OpenStackConsistentSnapshot::GetGroupSnapshotName()
{
    return "Protect_" + m_requestId + "_SNAP_" + std::to_string(time(0));
}
 
// 创建groupType
bool OpenStackConsistentSnapshot::DoCreateGroupType()
{
    if (m_groupInfo.m_isGroupTypeCreated) {
        INFOLOG("The groupType(%s) has been created. %s.", m_groupInfo.m_groupTypeName.c_str(), m_requestId.c_str());
        return true;
    }
 
    // 需要新创建快照组
    m_groupInfo.m_groupTypeName = GetGroupTypeName();
    if (CreateNewGroupType(m_groupInfo) != SUCCESS) {
        ERRLOG("Create groupType failed.");
        return false;
    }
    
    // 更新文件记录
    return UpdateConsistentSnapshotInfoToFile(m_groupInfo);
}
 
// 创建GroupType
int32_t OpenStackConsistentSnapshot::CreateNewGroupType(ConsistentSnapshotGroupInfo &groupInfo)
{
    GroupTypeRequest request;
    if (!SetOpenStackRequest<GroupTypeRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
    request.SetApiVersion(m_apiVersion);
    GroupTypeRequestBodyMsg body;
    body.m_groupType.m_name = groupInfo.m_groupTypeName;
    body.m_groupType.m_description = GROUP_TYPE_DESCRIPTION;
    body.m_groupType.m_isPublic = true;
    body.m_groupType.m_groupspecs.m_consistentGroupSnapshotEnable = "<is> True";
    request.SetBody(body);
    std::shared_ptr<GroupTypeResponse> response = m_cinderClient.CreateGroupType(request);
    if (response == nullptr) {
        ERRLOG("Create Group Type reponse is nullptr.");
        return FAILED;
    }

    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Create Group Type failed, status code %d", response->GetStatusCode());
        return FAILED;
    }

    groupInfo.m_groupTypeId = response->GetGroupTypeId();
    groupInfo.m_isGroupTypeCreated = true;
    return SUCCESS;
}
 
bool OpenStackConsistentSnapshot::DoCreateVolumeGroup(const std::vector<VolInfo> &volList)
{
    if (m_groupInfo.m_isGroupCreated) {
        INFOLOG("The volume group(%s) has been created.", m_groupInfo.m_groupName.c_str());
        return true;
    }
    
    if (!DoGetVolumeTypeId(volList, m_groupInfo)) {
        ERRLOG(" Get volume type id falied. %s.", m_requestId.c_str());
        return false;
    }
 
    m_groupInfo.m_groupTypeName = GetGroupName();
    DBGLOG("Start create volume group %s.", m_groupInfo.m_groupTypeName.c_str());
    if (DoCreateNewVolumeGroup(m_groupInfo) != SUCCESS) {
        ERRLOG("Create volume group failed, group name %s. %s.", m_groupInfo.m_groupName.c_str(),
            m_requestId.c_str());
        return false;
    }
 
    return UpdateConsistentSnapshotInfoToFile(m_groupInfo);
}
 
bool OpenStackConsistentSnapshot::DoGetVolumeTypeId(const std::vector<VolInfo> &volList,
    ConsistentSnapshotGroupInfo &groupInfo)
{
    for (const auto &volume : volList) {
        if (std::find(groupInfo.m_volumeType.begin(), groupInfo.m_volumeType.end(), volume.m_volumeType)
            == groupInfo.m_volumeType.end()) {
            DBGLOG("volume type is %s", volume.m_volumeType.c_str());
            groupInfo.m_volumeType.push_back(volume.m_volumeType);
        }
    }
 
    if (groupInfo.m_volumeType.empty()) {
        ERRLOG("volume type id is empty. %s.", m_requestId.c_str());
        return false;
    }
    return true;
}
 
int32_t OpenStackConsistentSnapshot::DoGetGroupStatus(ConsistentSnapshotGroupInfo &groupInfo)
{
    VolumeGroupRequest request;
    if (!SetOpenStackRequest<VolumeGroupRequest>(request)) {
        ERRLOG("Initialize openstack request failed");
        return FAILED;
    }
 
    request.SetApiVersion(m_apiVersion);
    request.SetGroupId(groupInfo.m_groupId);
    int32_t retryTime = 0;
    while (retryTime < RETRY_MAX_TIMES) {
        std::shared_ptr<VolumeGroupResponse> response = m_cinderClient.GetVolumeGroupStatus(request);
        if (response != nullptr && response->GetGroupStatus() == OPENSTAK_AVAILABLE_STATUS) {
            groupInfo.m_groupStatus = response->GetGroupStatus();
            return SUCCESS;
        }
 
        retryTime++;
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMES));
    }
    ERRLOG("Group(%s) status(%s) is not available", groupInfo.m_groupId.c_str(), groupInfo.m_groupStatus.c_str());
    DoDeleteGroupVolume(groupInfo);     // 长时间卷组状态未设置成avaliable，删除卷组
    return FAILED;
}
 
int32_t OpenStackConsistentSnapshot::DoCreateNewVolumeGroup(ConsistentSnapshotGroupInfo &groupInfo)
{
    VolumeGroupRequest request;
    if (!SetOpenStackRequest<VolumeGroupRequest>(request)) {
        ERRLOG("Initialize openstack request failed");
        return FAILED;
    }
 
    request.SetApiVersion(m_apiVersion);
    VolumeGroupRequestBodyMsg body;
    body.m_group.m_name = groupInfo.m_groupName;
    body.m_group.m_groupType = groupInfo.m_groupTypeId;
    body.m_group.m_volumeType = groupInfo.m_volumeType;
    body.m_group.m_description = "The group created by the backup";
    request.SetBody(body);
    std::shared_ptr<VolumeGroupResponse> response = m_cinderClient.CreateVolumeGroup(request);
    if (response == nullptr) {
        ERRLOG("Create Volume Group response is nullptr.");
        return FAILED;
    }
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Create Volume Group failed, status code %d.", response->GetStatusCode());
        return FAILED;
    }

    groupInfo.m_groupId = response->GetGroupID();
    if (DoGetGroupStatus(groupInfo) != SUCCESS) {
        ERRLOG("Create volume group failed. group id is: %s. %s", groupInfo.m_groupId.c_str(), m_requestId.c_str());
        return FAILED;
    }

    groupInfo.m_isGroupCreated = true;
    DBGLOG("Create volume group successful. group id is: %s.", groupInfo.m_groupId.c_str());
    return SUCCESS;
}
 
bool OpenStackConsistentSnapshot::DoAddVolumeListToGroup(const std::vector<VolInfo> &volList)
{
    if (m_groupInfo.m_isVolumeAddGroup) {
        INFOLOG("The volume list has been add group(%s).", m_groupInfo.m_groupName.c_str());
        return true;
    }
 
    if (AddListToVolumeGroup(volList, m_groupInfo) != SUCCESS) {
        ERRLOG("Add volume to group failed.");
        return false;
    }
    return UpdateConsistentSnapshotInfoToFile(m_groupInfo);
}
 
int32_t OpenStackConsistentSnapshot::AddListToVolumeGroup(const std::vector<VolInfo> &volList,
    ConsistentSnapshotGroupInfo &groupInfo)
{
    UpdateVolumeGroupRequest request;
    if (!SetOpenStackRequest<UpdateVolumeGroupRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
 
    if (volList.empty()) {
        ERRLOG("volume list is empty.");
        return FAILED;
    }

    UpdateGroupRequestBodyMsg body;
    body.m_group.m_name = groupInfo.m_groupName;
    std::string volumesUUid;
    for (const auto &it : volList) {
        volumesUUid += it.m_uuid + ",";
    }
    volumesUUid.erase(volumesUUid.end() - 1);
    DBGLOG("Add volume(%s) to group.", volumesUUid.c_str());
    body.m_group.m_addVolumes = volumesUUid;
    request.SetBody(body);
    request.SetGroupId(groupInfo.m_groupId);
    request.SetApiVersion(m_apiVersion);
    std::shared_ptr<UpdateVolumeGroupResponse> response = m_cinderClient.UpdateVolumeGroup(request);
    if (response == nullptr) {
        ERRLOG("Update Volume Group failed response is nullptr.");
        return FAILED;
    }
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Update Volume Group failed, status code %d.", response->GetStatusCode());
        return FAILED;
    }

    if (DoGetGroupStatus(groupInfo) != SUCCESS) {
        ERRLOG("Update Volume Group failed status.");
        return FAILED;
    }

    groupInfo.m_isVolumeAddGroup = true;
    groupInfo.m_volumelist = volumesUUid;
    return SUCCESS;
}

bool OpenStackConsistentSnapshot::DoCreateVolumeGroupSnapShot()
{
    if (m_groupInfo.m_isGroupSnapshotCreated) {
        INFOLOG("The group snapshot has been created(%s).", m_groupInfo.m_groupSnapshotId.c_str());
        return true;
    }
 
    m_groupInfo.m_groupSnapshotName = GetGroupSnapshotName();
    if (CreateVolumeGroupSnapShot(m_groupInfo) != SUCCESS) {
        ERRLOG("Create group snapshot failed.");
        return false;
    }
 
    return UpdateConsistentSnapshotInfoToFile(m_groupInfo);
}
 
int32_t OpenStackConsistentSnapshot::CreateVolumeGroupSnapShot(ConsistentSnapshotGroupInfo &groupInfo)
{
    CreateGroupSnapShotRequest request;
    if (!SetOpenStackRequest<CreateGroupSnapShotRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
 
    request.SetApiVersion(m_apiVersion);
    GroupSnapShotRequestBodyMsg body;
    body.m_groupSnapshot.m_name = groupInfo.m_groupSnapshotName;
    body.m_groupSnapshot.m_groupId = groupInfo.m_groupId;
    body.m_groupSnapshot.m_description = "Snapshot created by backup";
    request.SetBody(body);
    std::shared_ptr<CreateGroupSnapShotResponse> response = m_cinderClient.CreateGroupSnapShot(request);
    if (response == nullptr) {
        ERRLOG("Create Volume GroupSnapshot response is nullptr. %s.", m_requestId.c_str());
        return FAILED;
    }
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Create Volume GroupSnapshot failed, status code %d. %s.", response->GetStatusCode(),
            m_requestId.c_str());
        return FAILED;
    }
 
    groupInfo.m_groupSnapshotId = response->GetGroupSnapshotId();
    groupInfo.m_isGroupSnapshotCreated = true;
    return SUCCESS;
}
 
int32_t OpenStackConsistentSnapshot::DoWaitGroupSnapshotStatusAvailable(ConsistentSnapshotGroupInfo &groupInfo)
{
    int32_t retryTime = 0;
    std::shared_ptr<VolumeResponse> response;
    while (retryTime < RETRY_MAX_TIMES) {
        if (DoGetGroupSnapshotStatus(groupInfo) == SUCCESS &&
            groupInfo.m_groupsnapshotStatus == OPENSTAK_AVAILABLE_STATUS) {
            INFOLOG("group snapshot(%s) status is avaliable.", groupInfo.m_groupSnapshotId.c_str());
            return SUCCESS;
        }
        WARNLOG("group snapshot(%s) status is %s.", groupInfo.m_groupSnapshotId.c_str(),
            groupInfo.m_groupsnapshotStatus.c_str());
        retryTime++;
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMES));
    }
    return FAILED;
}
 
int32_t OpenStackConsistentSnapshot::DoGetGroupSnapshotStatus(ConsistentSnapshotGroupInfo &groupInfo)
{
    GetSnapshotRequest request;
    if (!SetOpenStackRequest<GetSnapshotRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
 
    request.SetApiVersion(m_apiVersion);
    request.SetGroupSnapshotId(groupInfo.m_groupSnapshotId);
    std::shared_ptr<GetSnapshotResponse> response = m_cinderClient.GetGroupSnapshot(request);
    if (response == nullptr) {
        ERRLOG("Get group snapshot status response is nullptr. %s.", m_requestId.c_str());
        return FAILED;
    }
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Get group snapshot status failed, status code %d. %s.", response->GetStatusCode(), m_requestId.c_str());
        return FAILED;
    }
 
    groupInfo.m_groupsnapshotStatus = response->GetGroupSnapshotStatus();
    return SUCCESS;
}
 
int32_t OpenStackConsistentSnapshot::DoGetSnapShotList(const std::vector<VolInfo> &volList, SnapshotInfo &snapshot,
    ConsistentSnapshotGroupInfo &groupInfo)
{
    GetSnapshotListRequest request;
    if (!SetOpenStackRequest<GetSnapshotListRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }

    for (const auto &volume : volList) {
        request.SetVolumeId(volume.m_uuid);
        std::shared_ptr<GetSnapshotListResponse> response = m_cinderClient.GetSnapshotList(request);
        if (response == nullptr) {
            ERRLOG("Get Snapshot List Failed response is nullptr.");
            return FAILED;
        }

        if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
            ERRLOG("Get Volume snapshot list failed, status code %d, %s.", response->GetStatusCode(),
                m_requestId.c_str());
            return FAILED;
        }

        for (const auto &snapDetails : response->GetSnapshotList().m_snapshots) {
            if (groupInfo.m_groupSnapshotName == snapDetails.m_name) {
                VolSnapInfo volSnap;
                volSnap.m_volUuid = volume.m_uuid;
                volSnap.m_createTime = snapDetails.m_createdAt;
                volSnap.m_snapshotName = snapDetails.m_name;
                volSnap.m_snapshotId = snapDetails.m_id;
                volSnap.m_size = volume.m_volSizeInBytes / GB_SIZE;
                Json::Value extendInfo;
                extendInfo["group_snapshot_id"] = snapDetails.m_groupSnapshotId;                // 一致性快照ID
                extendInfo["volume_type"] = volume.m_type;
                Json::FastWriter fastWriter;
                volSnap.m_extendInfo = fastWriter.write(extendInfo);
                snapshot.m_volSnapList.push_back(volSnap);
            }
        }
    }
    snapshot.m_consistengroupId = groupInfo.m_groupSnapshotId;
    snapshot.m_isconsistentSnapshot = true;
    return SUCCESS;
}

int32_t OpenStackConsistentSnapshot::ConfirmVolumeGroupNotExist(ConsistentSnapshotGroupInfo &groupInfo)
{
    VolumeGroupRequest request;
    if (!SetOpenStackRequest<VolumeGroupRequest>(request)) {
        ERRLOG("Initialize openstack request failed");
        return FAILED;
    }

    request.SetApiVersion(m_apiVersion);
    request.SetGroupId(groupInfo.m_groupId);
    int32_t retryTime = 0;
    while (retryTime < RETRY_MAX_TIMES) {
        std::shared_ptr<VolumeGroupResponse> response = m_cinderClient.GetVolumeGroupStatus(request);
        if (response == nullptr) {
            ERRLOG("The volume group's response is nullptr.");
            retryTime++;
            std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMES));
            continue;
        }
        if (response != nullptr && response->GetStatusCode() == static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
            groupInfo.m_isGroupCreated = false;
            return SUCCESS;
        }

        if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
            ERRLOG("Get Volume group failed: %d. %s.", response->GetStatusCode(), m_requestId.c_str());
            break;
        }
 
        retryTime++;
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMES));
    }
    ERRLOG("Consistent Group(%s) snapshot exist.", groupInfo.m_groupId.c_str());
    return FAILED;
}

int32_t OpenStackConsistentSnapshot::DoDeleteVolumeGroup(ConsistentSnapshotGroupInfo &groupInfo)
{
    DeleteVolumeGroupRequest request;
    if (!SetOpenStackRequest<DeleteVolumeGroupRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
 
    DeleteVolumeGroupRequestBodyMsg body;
    body.m_delete.m_deleteVolumes = false;
    request.SetBody(body);
    request.SetGroupId(groupInfo.m_groupId);
    request.SetApiVersion(m_apiVersion);
 
    std::shared_ptr<DeleteVolumeGroupResponse> response = m_cinderClient.DeleteGroup(request);
    if (response == nullptr) {
        ERRLOG("Delete Group failed response is nullptr.");
        return FAILED;
    }
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Get Status delete group type failed, status code %d. %s.", response->GetStatusCode(),
            m_requestId.c_str());
        return FAILED;
    }
    
    if (ConfirmVolumeGroupNotExist(groupInfo) != SUCCESS) {
        ERRLOG("Delete Volume Group(%s) Failed", groupInfo.m_groupId.c_str());
        return FAILED;
    }

    return UpdateConsistentSnapshotInfoToFile(groupInfo) ? SUCCESS : FAILED;
}

int32_t OpenStackConsistentSnapshot::ConfirmGroupSnapshotNotExist(const ConsistentSnapshotGroupInfo &groupInfo)
{
    GetSnapshotRequest request;
    if (!SetOpenStackRequest<GetSnapshotRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
 
    request.SetApiVersion(m_apiVersion);
    request.SetGroupSnapshotId(groupInfo.m_groupSnapshotId);
    int32_t retryTime = 0;
    while (retryTime < RETRY_MAX_TIMES) {
        std::shared_ptr<GetSnapshotResponse> response = m_cinderClient.GetGroupSnapshot(request);
        if (response == nullptr) {
            ERRLOG("The group snapshot's response is nullptr.");
            retryTime++;
            std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMES));
            continue;
        }
        if (response->GetStatusCode() == static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
            return SUCCESS;
        }

        if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
            ERRLOG("Get group snapshot failed: %d, %s.", response->GetStatusCode(), m_requestId.c_str());
            break;
        }
 
        retryTime++;
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMES));
    }
    ERRLOG("Group snapshot(%s) exist.", groupInfo.m_groupSnapshotId.c_str());
    return FAILED;
}

int32_t OpenStackConsistentSnapshot::DoDeleteGroupSnapshot(ConsistentSnapshotGroupInfo &groupInfo)
{
    DeleteSnapshotRequest request;
    if (!SetOpenStackRequest<DeleteSnapshotRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
    request.SetApiVersion(m_apiVersion);
    request.SetGroupSnapshotId(groupInfo.m_groupSnapshotId);
    std::shared_ptr<DeleteSnapshotResponse> response = m_cinderClient.DeleteGroupSnapShot(request);
    if (response == nullptr) {
        ERRLOG("Delete Group snapshot reponse is nullptr.");
        return FAILED;
    }
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Delete group snapshot failed, status code %d, %s.", response->GetStatusCode(), m_requestId.c_str());
        return FAILED;
    }

    if (ConfirmGroupSnapshotNotExist(groupInfo) != SUCCESS) {
        ERRLOG("Delete group snapshot(%s) failed, %s.", groupInfo.m_groupSnapshotId.c_str(), m_requestId.c_str());
        return FAILED;
    }
    groupInfo.m_isGroupSnapshotCreated = false;
    return UpdateConsistentSnapshotInfoToFile(groupInfo) ? SUCCESS : FAILED;
}
 
int32_t OpenStackConsistentSnapshot::DoDeleteGroupType(ConsistentSnapshotGroupInfo &groupInfo)
{
    GroupTypeRequest request;
    if (!SetOpenStackRequest<GroupTypeRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
 
    request.SetGroupTypeId(groupInfo.m_groupTypeId);
    request.SetApiVersion(m_apiVersion);
    std::shared_ptr<GroupTypeResponse> response = m_cinderClient.DeleteGroupType(request);
    if (response == nullptr) {
        ERRLOG("Delete Group Type failed response is nullptr.");
        return FAILED;
    }
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED) &&
        response->GetStatusCode() != static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
        ERRLOG("Delete group type failed, status code %d, %s.", response->GetStatusCode(), m_requestId.c_str());
        return FAILED;
    }

    groupInfo.m_isGroupTypeCreated = false;
    return UpdateConsistentSnapshotInfoToFile(groupInfo, true) ? SUCCESS : FAILED;
}
 
int32_t OpenStackConsistentSnapshot::RemoveVolumeFromGroup(ConsistentSnapshotGroupInfo &groupInfo)
{
    UpdateVolumeGroupRequest request;
    if (!SetOpenStackRequest<UpdateVolumeGroupRequest>(request)) {
        ERRLOG("Initialize openstack request failed.");
        return FAILED;
    }
 
    UpdateGroupRequestBodyMsg body;
    body.m_group.m_name = groupInfo.m_groupName;
    body.m_group.m_removeVolumes = groupInfo.m_volumelist;
    request.SetBody(body);
    request.SetGroupId(groupInfo.m_groupId);
    request.SetApiVersion(m_apiVersion);
    std::shared_ptr<UpdateVolumeGroupResponse> response = m_cinderClient.UpdateVolumeGroup(request);
    if (response == nullptr) {
        ERRLOG("Update Volume Group failed response is nullptr.");
        return FAILED;
    }
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED) &&
        response->GetStatusCode() != static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
        ERRLOG("Update Volume Group failed, status code %d. %s.", response->GetStatusCode(), m_requestId.c_str());
        return FAILED;
    }

    if (DoGetGroupStatus(groupInfo) != SUCCESS) {
        ERRLOG("Update volume group, The status is not normal.");
        return FAILED;
    }
    groupInfo.m_isVolumeAddGroup = false;
    groupInfo.m_volumelist.erase();
    return UpdateConsistentSnapshotInfoToFile(groupInfo) ? SUCCESS : FAILED;
}
 
bool OpenStackConsistentSnapshot::DoDeleteGroupVolume(ConsistentSnapshotGroupInfo &groupInfo)
{
    if (groupInfo.m_isVolumeAddGroup && RemoveVolumeFromGroup(groupInfo) != SUCCESS) {
        ERRLOG("Remove volume frome group failed. %s.", m_requestId.c_str());
        return false;
    }
 
    if (groupInfo.m_isGroupCreated && DoDeleteVolumeGroup(groupInfo) != SUCCESS) {
        ERRLOG("Delete volume group failed. %s.", m_requestId.c_str());
        return false;
    }
    
    if (groupInfo.m_isGroupTypeCreated && DoDeleteGroupType(groupInfo) != SUCCESS) {
        ERRLOG("Delete Group Type Failed. %s.", m_requestId.c_str());
        return false;
    }
 
    return true;
}

OPENSTACK_PLUGIN_NAMESPACE_END
