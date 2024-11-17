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
#include <set>
#include <map>
#include <unordered_map>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <common/JsonHelper.h>
#include <config_reader/ConfigIniReader.h>
#include "protect_engines/cnware/common/ErrorCode.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "common/Utils.h"
#include "CNwareProtectEngine.h"

using namespace VirtPlugin;

namespace {
const std::string MODULE = "CNwareProtectEngine";
const int64_t INIT_FAILED = 200;
const int64_t MB = 1024 * 1024;
const int64_t UNAUTH_ERROR_CODE = 1007;
const int64_t UNAUTH_LOCKED_CODE = 1008;
const int CN_COLOUM_LENTH = 3;
const int64_t INIT_CLIENT_FAILED = 1677929218;
const int32_t ACTION_SUCCESS = 0;
const int32_t ACTION_CONTINUE = 100;
const int32_t ACTION_BUSY = 101;
const int32_t ACTION_ERROR = 200;
const int32_t CHECK_CNWARE_CONNECT_FAILED = 1577213556;
const std::string CNWARE_CONF = "CNwareConfig";
const int32_t COMMON_WAIT_TIME = 10;
const int32_t COMMON_WAIT_TIME_3S = 3;
const std::string DELETE_SNAPSHOT_VOLUME_FAILED_ALARM_CODE = "0x6403400004";
const int32_t MAX_EXEC_COUNT = 5;
const int32_t CHECK_STORAGE_CONNECT_FAILED = 1577213520;
const int32_t HCS_CERT_NOT_EXIST = 1577090049;
const int32_t CNWARE_VM_STATUS_ABNORMAL = 0;
const int32_t CNWARE_ATTACH_VOLUME_BUS_VIRTIO = 1;
const int32_t CNWARE_MAX_TIME_OUT = 720000; // 防呆设置最大200小时超时时间
const int32_t CREATE_NEW_DISK = 1;
const int32_t USE_OLD_DISK = 0;
const int32_t PROTOCAL_NFS_CIFS = 3;
const int32_t STORAGE_TYPE_NAS = 3;
const int32_t STORAGE_TYPE_CEPH = 4;
const int32_t HOST_DISCONNECT = 1;
const int32_t HOST_MAINTAIN = 2;
const int32_t HOST_ERR = 3;
const int32_t ISCSI_BLOCK_LUN_DISK_TYPE = 2;
const std::string RESTORE_LOCATION_ORIGINAL = "original";
const std::string RESTORE_LOCATION_NEW = "new";
const std::string CURRENT_BOOT_DISK = "YES";
const std::string DEFAULT_PREALLOCATION = "off";
const std::string STATUS_BEGINNING_RUNNING = "0\%2C1";
const int32_t STATUS_BEGINNING = 0;
const int32_t STATUS_RUNNING = 1;
const std::string TASK_MIGRATE_CODE = "domain.migrate";
const bool DISENABLED_INTERFACE = false;
std::map<std::string, int32_t> DISK_BUS_MAP = {
    {"virtio", 1},
    {"ide", 2},
    {"scsi", 3},
    {"sata", 4},
    {"usb", 5}
};
std::vector<std::string> CNWARE_VERSION_SUPPORTED = {
    "8.2.2",
    "8.2.3",
    "9.1",
    "9.2"
};

/* 1.x86 32位 2.x86 64位 3.aarch64 4.mips64el 5 申威 6 龙芯5000 */
std::map<uint32_t, std::string> CNFWARE_CPU_ARCH_MAP = {
    {1, "x86_32"},
    {2, "x86_64"},
    {3, "aarch64"},
    {4, "mips64el"},
    {5, "sw_64"},
    {6, "loongarch64"}
};
std::map<std::string, uint32_t> CNFWARE_CPU_ARCH_RMAP = {
    {"x86_32", 1},
    {"x86_64", 2},
    {"aarch64", 3},
    {"mips64el", 4},
    {"sw_64", 5},
    {"loongarch64", 6}
};

std::map<int32_t, std::string> HOST_STATUS_MAP = {
    {0, "Normal"},
    {1, "Disconnected"},
    {2, "Maintaining"},
    {3, "Error"}
};
}  // namespace

namespace CNwarePlugin {
bool CNwareProtectEngine::FillSpecifiedTargetVolsDeviceInfo(
    const AppProtect::BackupJob &job, std::vector<DomainDiskDevicesResp> &targetVolsInfo)
{
    for (const auto &targetVolObj : job.protectSubObject) {
        for (const auto &vol : m_vmInfo.m_volList) {
            DomainDiskDevicesResp diskDevice;
            if (!Module::JsonHelper::JsonStringToStruct(vol.m_metadata, diskDevice)) {
                ERRLOG("Volume metadata trans to diskDevice failed.");
                return false;
            }
            if (targetVolObj.name == diskDevice.m_dev) {
                targetVolsInfo.push_back(diskDevice);
            }
        }
    }
    if (targetVolsInfo.size() != job.protectSubObject.size()) {
        ERRLOG("Fail to find target dev on vm. task id: %s", m_taskId.c_str());
        return false;
    }
}

bool CNwareProtectEngine::FillTargetVolsDeviceInfo(
    const AppProtect::BackupJob &job, std::vector<DomainDiskDevicesResp> &targetVolsInfo)
{
    // 指定磁盘备份时，过滤待备份磁盘
    if (!job.protectSubObject.empty()) {
        return FillSpecifiedTargetVolsDeviceInfo(job, targetVolsInfo);
    } else { // 整机备份时填充所有磁盘信息
        for (const auto &vol : m_vmInfo.m_volList) {
            DomainDiskDevicesResp diskDevice;
            if (!Module::JsonHelper::JsonStringToStruct(vol.m_metadata, diskDevice)) {
                ERRLOG("Volume metadata trans to diskDevice failed.");
                return false;
            }
            targetVolsInfo.push_back(diskDevice);
        }
    }
    return true;
}

bool CNwareProtectEngine::CheckVolPreSnapshotValidity(
    const DomainDiskDevicesResp &currVol, const VolSnapInfo &snapshotVol)
{
    DomainDiskDevicesResp diskDeviceInfo;
    if (!Module::JsonHelper::JsonStringToStruct(snapshotVol.m_metadata, diskDeviceInfo)) {
        ERRLOG("Transfer DomainDiskDevicesResp failed, %s", m_taskId.c_str());
        return false;
    }
    DBGLOG("Enter, currVol.m_capacity(%llu) snapshotVol.m_size(%llu)", currVol.m_capacity, snapshotVol.m_size);
    if (currVol.m_capacity > diskDeviceInfo.m_capacity) {
        WARNLOG("Target vol(%s) capacity increased", currVol.m_volId.c_str());
        return false;
    }
    return true;
}

bool CNwareProtectEngine::CheckPreSnapShotValidity(
    const AppProtect::BackupJob &job, const std::vector<VolSnapInfo> &lastVolList)
{
    std::vector<DomainDiskDevicesResp> targetVolsInfo;
    if (!FillTargetVolsDeviceInfo(job, targetVolsInfo)) {
        ERRLOG("Fail to fill target vols device info");
        return false;
    }

    // 关联前次快照原卷当前的dev
    std::unordered_map<std::string, int> mapDevIndex;
    for (int idx = 0; idx < lastVolList.size(); ++idx) {
        SnapshotDiskInfo volSnapshotInfo;
        if (!Module::JsonHelper::JsonStringToStruct(lastVolList[idx].m_extendInfo, volSnapshotInfo)) {
            ERRLOG("Transfer SnapshotDiskInfo failed, %s", m_taskId.c_str());
            return false;
        }
        std::string dev = "";
        if (!GetVolDevOnVm(volSnapshotInfo.m_originVolId, m_vmInfo.m_uuid, dev)) {
            ERRLOG("Get snapshot origin volume(%s) dev failed, previous snap invalid.",
                volSnapshotInfo.m_originVolId.c_str());
            return false;
        }
        mapDevIndex[dev] = idx;
    }

    for (const auto &targetVol : targetVolsInfo) {
        auto itMap = mapDevIndex.find(targetVol.m_dev);
        if (itMap == mapDevIndex.end() || !CheckVolPreSnapshotValidity(targetVol, lastVolList[itMap->second])) {
            WARNLOG("Check vol (%s) previous snapshot validity failed. task id: %s",
                targetVol.m_volId.c_str(), m_taskId.c_str());
            return false;
        }
    }
    return true;
}

int32_t CNwareProtectEngine::CheckBeforeUnmount()
{
    return SUCCESS;
}

void CNwareProtectEngine::SaveLiveMountStorageInfo(VMInfo &liveVm, std::string &storeId, std::string &storageId)
{
    Json::Value jsInfo;
    Json::FastWriter writer;
    jsInfo["storeId"] = storeId;
    jsInfo["storageId"] = storageId;
    liveVm.m_extendInfo = writer.write(jsInfo);
    INFOLOG("Save live mount temp storage info success");
    return;
}

bool CNwareProtectEngine::GetLiveMountStorageInfo(const VMInfo &liveVm, std::string &storeId, std::string &storageId)
{
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(liveVm.m_extendInfo, extendInfo)) {
        ERRLOG("Convert extendInfo failed, %s", m_taskId.c_str());
        return false;
    }
    if (!extendInfo.isMember("storeId") || !extendInfo.isMember("storageId")) {
        ERRLOG("Invalid extend info.");
        return false;
    }
    storeId = extendInfo["storeId"].asString();
    storageId = extendInfo["storageId"].asString();
    INFOLOG("Get live mount temp storage info success");

    return true;
}

bool CNwareProtectEngine::RemoveStorage(const std::string &storageId)
{
    DelStoragePoolRequest req;
    SetCommonInfo(req);
    if (storageId.empty()) {
        WARNLOG("No need to delete pool, %s", m_taskId.c_str());
        return true;
    }
    req.SetPoolId(storageId);
    std::shared_ptr<DelStoragePoolResponse> response = m_cnwareClient->DelStoragePool(req);
    if (!CommonCheckResponse(response)) {
        ERRLOG("Send delete pool %s request failed, %s", storageId.c_str(), m_taskId.c_str());
        return false;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Delete pool %s failed, %s", storageId.c_str(), m_taskId.c_str());
        return false;
    }
    INFOLOG("Delete pool %s success, %s", storageId.c_str(), m_taskId.c_str());
    return true;
}

int32_t CNwareProtectEngine::CancelLiveMount(const VMInfo &liveVm)
{
    std::string storeId = "";
    std::string storageId = "";
    if (!GetLiveMountStorageInfo(liveVm, storeId, storageId)) {
        ERRLOG("Get live mount storage info failed.");
        return FAILED;
    }

    // 移除临时存储
    if (!RemoveStorage(storageId)) {
        ERRLOG("Remove tmp storage failed.");
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::CheckProtectEnvConn(
    const AppProtect::ApplicationEnvironment &env, const std::string &vmId, int32_t &errorCode)
{
    if (ConnectToEnv(env) != SUCCESS) {
        ERRLOG("Failed to check protect environment.");
        errorCode = CHECK_CNWARE_CONNECT_FAILED;
        CheckCertIsExist(errorCode);
        return FAILED;
    }
    if (!CheckCNwareVersion()) {
        ERRLOG("Check protect environment version failed.");
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::CheckBackupJobType(const JobTypeParam &jobTypeParam, bool &checkRet)
{
    if (ConnectToEnv(jobTypeParam.m_job.protectEnv) != SUCCESS) {
        ERRLOG("Failed to connect protect environment.");
        return FAILED;
    }

    if (!DoGetMachineMetadata(jobTypeParam.m_job.protectObject.id)) {
        ERRLOG("Set VM metadata info failed, %s", m_taskId.c_str());
        return FAILED;
    }

    if (m_vmInfo.m_volList.empty()) {
        ERRLOG("Fail to get vm volume list is empty.task id: %s", m_taskId.c_str());
        return FAILED;
    }

    if (jobTypeParam.m_snapshotInfo.m_volSnapList.size() < jobTypeParam.m_job.protectSubObject.size()) {
        WARNLOG("Number of protected volumes increased, convert to full backup.task id: %s",
            jobTypeParam.m_job.protectObject.id.c_str(), m_taskId.c_str());
        checkRet = false;
        return SUCCESS;
    }

    if (!CheckPreSnapShotValidity(jobTypeParam.m_job, jobTypeParam.m_snapshotInfo.m_volSnapList)) {
        WARNLOG("Check last snapshot info validity failed, convert to full backup.task id: %s",
            m_taskId.c_str());
        checkRet = false;
        return SUCCESS;
    }
    checkRet = true;
    return SUCCESS;
}

int CNwareProtectEngine::GenVolPair(VMInfo &vmObj, const VolInfo &copyVol,
    const ApplicationResource &targetVol, VolMatchPairInfo &volPairs)
{
    InitRepoHandler();
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        VolInfo dstVolInfo;
        Json::Value targetVolume;
        if (InitParaAndGetTargetVolume(targetVol, targetVolume) != SUCCESS) {
            ERRLOG("Get target volume info failed.");
            return FAILED;
        }
        DatastoreInfo storage;
        FormVolumeInfo(targetVolume, dstVolInfo, storage);
        if (CreateDiskByCopy(copyVol, dstVolInfo, storage) != SUCCESS) {
            ERRLOG("Create disk by copy failed.");
            return FAILED;
        }
        volPairs.AddVolPair(copyVol, dstVolInfo);
    } else {
        if (DoGenVolPairForDiskRestore(vmObj, copyVol, targetVol, volPairs) != SUCCESS) {
            ERRLOG("Generate volume pair for disk restore failed.");
            return FAILED;
        }
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::CreateDiskByCopy(const VolInfo &copyVol, VolInfo &dstVolInfo,
    DatastoreInfo &storage)
{
    VolInfo volInfo;
    InitRepoHandler();
    if (LoadCopyVolumeMatadata(copyVol.m_uuid, volInfo)) {
        ERRLOG("Load volume %s metadata failed.", copyVol.m_uuid.c_str());
        return FAILED;
    }
    DomainDiskDevicesResp diskDev;
    if (!Module::JsonHelper::JsonStringToStruct(volInfo.m_metadata, diskDev)) {
        ERRLOG("Get volume metadata failed.");
        return FAILED;
    }
    uint64_t curTimeStamp =
        std::chrono::system_clock::now().time_since_epoch().count() / std::chrono::system_clock::period::den;
    dstVolInfo.m_name = copyVol.m_name + "_" + std::to_string(curTimeStamp);
    if (storage.m_name.empty()) {
        storage.m_name = diskDev.m_storagePoolName;
    }
    // cnware添加空磁盘接口不能带有短横线
    DBGLOG("Try replace '-' to '_' for volume: %s.", dstVolInfo.m_name.c_str());
    std::replace(dstVolInfo.m_name.begin(), dstVolInfo.m_name.end(), '-', '_');
    DBGLOG("Create volume, volume name %s, storage pool name %s", dstVolInfo.m_name.c_str(), storage.m_name.c_str());
    {
        std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
        if (CreateVolume(copyVol, "", "", storage, dstVolInfo) == FAILED) {
            ERRLOG("Create volume failed.");
            return FAILED;
        }
        INFOLOG("Create disk %s success.", dstVolInfo.m_uuid.c_str());
        // 卸载避免创建磁盘堆积导致代理磁盘数量达到上限，同时支持调度其他代理执行该盘的恢复子任务
        if (!DoDetachVolumeOnVm(dstVolInfo.m_uuid, m_domainId)) {
            ERRLOG("Detach new create vol on agent Failed");
            return FAILED;
        }
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::GenVolPairLocationNew(
    VMInfo &vmObj, const VolInfo &copyVol, const std::string &dstVolId, VolMatchPairInfo &volPairs)
{
    for (const VolInfo &vol : vmObj.m_volList) {
        if (dstVolId.empty()) {
            ERRLOG("No target volume provided.");
            return FAILED;
        }
        // 新位置磁盘恢复：在目标虚拟机上找到和参数下发(用户选择)的目标盘id相同的盘，则添加对应关系
        if (vol.m_uuid == dstVolId) {
            volPairs.AddVolPair(copyVol, vol);
            INFOLOG("Disk restore(new location) - Restore disk %s to (VM: %s, Disk: %s), %s",
                copyVol.m_uuid.c_str(), vmObj.m_name.c_str(), dstVolId.c_str(), m_taskId.c_str());
        }
        // 按照盘符顺序升序排序
        std::sort(volPairs.m_volPairList.begin(), volPairs.m_volPairList.end(), [](auto volX, auto volY)->bool {
            return (volX.m_targetVol.m_slotId < volY.m_targetVol.m_slotId);
        });
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::GenVolPairLocationOriginal(
    VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol, VolMatchPairInfo &volPairs)
{
    Json::Value copyExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(copyVol.m_extendInfo, copyExtendInfo)) {
        ERRLOG("Convert copyExtendInfo failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!copyExtendInfo.isMember("rawVolId")) {
        ERRLOG("No rawVolId provided.");
        return FAILED;
    }
    std::string rawVolId = copyExtendInfo["rawVolId"].asString();
    // 处理原位置磁盘恢复，原磁盘id变化的场景，查询副本原盘所在盘符
    std::string dev = "";
    if (!GetVolDevOnVm(rawVolId, vmObj.m_uuid, dev)) {
        ERRLOG("Get copy origin volume(%s) dev failed.", copyVol.m_extendInfo.c_str());
        return false;
    }
    for (const VolInfo &vol : vmObj.m_volList) {
        // 原位置磁盘恢复：在目标虚拟机上找到和副本盘当前所在盘符相同的盘，添加对应关系
        DomainDiskDevicesResp diskDevice;
        if (!Module::JsonHelper::JsonStringToStruct(vol.m_metadata, diskDevice)) {
            ERRLOG("Volume metadata trans to diskDevice failed.");
            return FAILED;
        }
        if (diskDevice.m_dev == dev) {
            volPairs.AddVolPair(copyVol, vol);
            INFOLOG("Disk restore(original location) - Restore disk %s to (VM: %s, Disk: %s), %s",
                copyVol.m_uuid.c_str(), vmObj.m_name.c_str(), targetVol.id.c_str(), m_taskId.c_str());
        }
    }
    return SUCCESS;
}

void CNwareProtectEngine::SetBootAbleDisk(std::vector<VolInfo> &vmVolList, uint32_t bootType)
{
    if (vmVolList.empty()) {
        ERRLOG("Empty vol list when set boot able disk");
        return;
    }
    std::sort(vmVolList.begin(), vmVolList.end(), [](auto volX, auto volY)->bool {
        return (std::stoi(volX.m_bootable) < std::stoi(volY.m_bootable));
    });
    vmVolList.front().m_bootable = CURRENT_BOOT_DISK;
    Json::Value jsInfo;
    Json::FastWriter writer;
    jsInfo["bootType"] = bootType;
    vmVolList.front().m_extendInfo = writer.write(jsInfo);
    INFOLOG("Get boot disk success");
    return;
}

int32_t CNwareProtectEngine::DoGenVolPairForDiskRestore(
    VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol, VolMatchPairInfo &volPairs)
{
    INFOLOG("Enter");
    if (GetMachineMetadata(vmObj) != SUCCESS) {
        ERRLOG("Get target vm metadata failed.");
        return FAILED;
    }
    std::string targetVmStr = "";
    if (!Module::JsonHelper::StructToJsonString(vmObj, targetVmStr)) {
        ERRLOG("Convert target vm info failed, %s", m_jobId.c_str());
        return FAILED;
    }
    std::string targetVmInfoFile = m_cacheRepoPath + VIRT_PLUGIN_RESTORE_TARGET_VM_INFO;
    if (Utils::SaveToFileWithRetry(m_cacheRepoHandler, targetVmInfoFile, targetVmStr) != SUCCESS) {
        ERRLOG("Failed to save target vm info to file, %s", m_jobId.c_str());
        return FAILED;
    }
    Json::Value targetVolume;
    if (InitParaAndGetTargetVolume(targetVol, targetVolume) != SUCCESS) {
        ERRLOG("Get target volume info failed.");
        return FAILED;
    }
    if (!targetVolume.isMember("uuid") || !targetVolume.isMember("isNewDisk")) {
        ERRLOG("No target volume uuid provided.");
        return FAILED;
    }
    if (targetVolume["isNewDisk"].asString() == "true") {
        return CheckIsNewDisk(copyVol, targetVolume, volPairs) ? SUCCESS : FAILED;
    }
    CNwareVMInfo cwVmInfo;
    if (!Module::JsonHelper::JsonStringToStruct(vmObj.m_metadata, cwVmInfo)) {
        ERRLOG("Get new domain info failed.");
        return FAILED;
    }

    DBGLOG("Target vm volume list size: %d, target volume id: %s",
        vmObj.m_volList.size(), targetVolume["uuid"].asString().c_str());

    if (m_targetLocation == RESTORE_LOCATION_NEW) {
        return GenVolPairLocationNew(vmObj, copyVol, targetVolume["uuid"].asString(), volPairs);
    } else if (m_targetLocation == RESTORE_LOCATION_ORIGINAL) {
        return GenVolPairLocationNew(vmObj, copyVol, targetVolume["uuid"].asString(), volPairs);
    } else {
        ERRLOG("Bad restore location %s", m_targetLocation.c_str());
        return FAILED;
    }
}

bool CNwareProtectEngine::CheckIsNewDisk(const VolInfo &copyVol, Json::Value &targetVolume,
    VolMatchPairInfo &volPairs)
{
    DatastoreInfo storage;
    VolInfo dstVolInfo;
    dstVolInfo.m_name = copyVol.m_name;
    INFOLOG("New created disk restore metadata:%s", copyVol.m_metadata.c_str());
    FormVolumeInfo(targetVolume, dstVolInfo, storage);
    {
        std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
        if (CreateVolume(copyVol, "", "", storage, dstVolInfo) == FAILED) {
            ERRLOG("Create volume failed.");
            return false;
        }
        if (!DoDetachVolumeOnVm(dstVolInfo.m_uuid, m_domainId)) {
            ERRLOG("Detach new create vol on agent Failed");
            return FAILED;
        }
    }
    volPairs.AddVolPair(copyVol, dstVolInfo);
    INFOLOG("New created disk restore - Copy vol(%s, %s), target vol(%s, %s), %s",
        copyVol.m_name.c_str(), copyVol.m_uuid.c_str(),
        dstVolInfo.m_name.c_str(), dstVolInfo.m_uuid.c_str(), m_taskId.c_str());
    return true;
}

int32_t CNwareProtectEngine::InitParaAndGetTargetVolume(const ApplicationResource &targetVol,
    Json::Value &targetVolume)
{
    Json::Value volExtendInfo;
    INFOLOG("Target volume(%s) extend info: %s", targetVol.id.c_str(), targetVol.extendInfo.c_str());
    if (!Module::JsonHelper::JsonStringToJsonValue(targetVol.extendInfo, volExtendInfo)) {
        ERRLOG("JsonStringToJsonValue failed. targetVol's extendInfo, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!volExtendInfo.isMember("targetVolume")) {
        ERRLOG("No targetVolume provided. %s", m_jobId.c_str());
        return FAILED;
    }
    std::string targetVolumeStr = volExtendInfo["targetVolume"].asString();
    if (!Module::JsonHelper::JsonStringToJsonValue(targetVolumeStr, targetVolume)) {
        ERRLOG("JsonStringToJsonValue failed. targetVol's extendInfo, %s", m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Init jobPara and repoHandle success, get targetVolume success");
    return SUCCESS;
}

void CNwareProtectEngine::InitRepoHandler()
{
    if (m_metaRepoHandler != nullptr && m_cacheRepoHandler != nullptr) {
        return;
    }
    std::vector<AppProtect::StorageRepository> repos = m_jobHandle->GetStorageRepos();
    for (const auto &repo : repos) {
        if (repo.repositoryType == RepositoryDataType::META_REPOSITORY) {
            m_metaRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);
            m_metaRepoPath = repo.path[0];
        } else if (repo.repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);
            m_cacheRepoPath = repo.path[0];
        }
    }
}

void CNwareProtectEngine::FormVolumeInfo(const Json::Value &targetVolume, VolInfo &volObj, DatastoreInfo &storage)
{
    std::string volName = targetVolume["name"].asString();
    Json::Value jsonDatastore = targetVolume["datastore"];
    if (!Module::JsonHelper::JsonValueToStruct(jsonDatastore, storage)) {
        ERRLOG("Failed to convert storagePool to Struct datastore, taskID: %s",
            m_jobId.c_str());
        return;
    }
    if (storage.m_name.empty() || storage.m_poolId.empty()) {
        WARNLOG("No datastore name provided. %s", m_jobId.c_str());
        return;
    }
    if (volName.empty()) {
        WARNLOG("No target volume name provided. %s", m_jobId.c_str());
        return;
    }
    // cnware添加空磁盘接口不能带有短横线
    DBGLOG("Try replace '-' to '_' for volume: %s", volName.c_str());
    std::replace(volName.begin(), volName.end(), '-', '_');
    DBGLOG("Create volume, volume name %s, storage pool name %s", volName.c_str(), storage.m_name.c_str());
    if (CheckVolIsInStorage(volName, storage.m_poolId)) {
        INFOLOG("Volume name %s exists, try rename it.", volName.c_str());
        uint64_t curTimeStamp =
            std::chrono::system_clock::now().time_since_epoch().count() / std::chrono::system_clock::period::den;
        volName = volName + "_" + std::to_string(curTimeStamp);
    }
    volObj.m_name = volName;
    return;
}

bool CNwareProtectEngine::CheckVolIsInStorage(const std::string &volName, const std::string &poolId)
{
    CNwareRequest req;
    SetCommonInfo(req);
    req.SetDomain(poolId);
    if (m_cnwareClient == nullptr) {
        ERRLOG("CheckVolIsInStorage m_cnwareClient pointer failed.");
        return false;
    }
    std::shared_ptr<VolExistResponse> response = m_cnwareClient->StorageVolumesExist(req, volName);
    if (response == nullptr) {
        ERRLOG("CheckVolIsInStorage response pointer failed.");
        return false;
    }
    if (response->IsExist()) {
        WARNLOG("Vol %s exist in storage pool: %s, %s", volName.c_str(),
            poolId.c_str(), m_jobId.c_str());
        return true;
    }
    INFOLOG("Vol %s not exist in storage pool: %s, %s", volName.c_str(),
        poolId.c_str(), m_jobId.c_str());
    return false;
}

int32_t CNwareProtectEngine::LoadCopyVolumeMatadata(const std::string &volId, VolInfo &volInfo)
{
    DBGLOG("RestoreMode: %s", m_restorePara->jobParam.restoreMode.c_str());
    std::shared_ptr<RepositoryHandler> repositoryHandle =
        (m_metaRepoHandler != nullptr) ? m_metaRepoHandler : m_cacheRepoHandler;
    std::string repoPath = (m_metaRepoHandler != nullptr) ? m_metaRepoPath : m_cacheRepoPath;

    std::string volmetaDataPath = repoPath + VIRT_PLUGIN_VOLUMES_META_DIR + volId + ".ovf";
    if (Utils::LoadFileToStructWithRetry(repositoryHandle, volmetaDataPath, volInfo) != SUCCESS) {
        ERRLOG("Failed to load file %s, %s", volmetaDataPath.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Load file %s success.", volmetaDataPath.c_str());
    return SUCCESS;
}

bool CNwareProtectEngine::CheckTaskStatus(const std::string &taskId)
{
    QueryTaskRequest req;
    req.SetTaskId(taskId);
    SetCommonInfo(req);

    int32_t taskStatus = static_cast<int>(CNwareTaskStatus::CNWARE_TASK_STARTED);
    std::shared_ptr<QueryTaskResponse> response;
    uint32_t times = 0;
    while (times * COMMON_WAIT_TIME_3S < CNWARE_MAX_TIME_OUT) {
        times++;
        response = m_cnwareClient->QueryTask(req);
        if (!CommonCheckResponse(response)) {
            ERRLOG("Query task %s status failed, %s", taskId.c_str(), m_taskId.c_str());
            return false;
        }

        taskStatus = response->GetTaskStatus();
        INFOLOG("taskStatus : %d", taskStatus);
        if (taskStatus != static_cast<int>(CNwareTaskStatus::CNWARE_TASK_STARTED) &&
            taskStatus != static_cast<int>(CNwareTaskStatus::CNWARE_TASK_BEFORE_START)) {
            break;
        } else {
            sleep(COMMON_WAIT_TIME_3S);
        }
    }
    if (taskStatus != static_cast<int>(CNwareTaskStatus::CNWARE_TASK_SUCCESS)) {
        ERRLOG("CNware taskId %s failed, status %d, stepDesc %s, %s", taskId.c_str(),
               taskStatus, (response->GetTaskStepDesc()).c_str(), m_taskId.c_str());
        return false;
    }
    return true;
}

bool CNwareProtectEngine::IfDeleteLatestSnapShot()
{
    return true;
}

int32_t CNwareProtectEngine::AddNfs(const std::string &name,
    const std::string &hostId, const std::string &nfsIp)
{
    INFOLOG("Enter");
    AddNfsRequest req;
    SetCommonInfo(req);
    if (!req.SetNfsReq(name, hostId, nfsIp)) {
        ERRLOG("AddNfs SetNfsReq failed.");
        return FAILED;
    }
    if (m_cnwareClient == nullptr) {
        ERRLOG("AddNfs m_cnwareClient pointer failed.");
        return FAILED;
    }
    std::shared_ptr<StoreScanResponse> response = m_cnwareClient->AddNfs(req);
    if (response == nullptr) {
        ERRLOG("AddNfs response pointer null. %s", m_jobId.c_str());
        return FAILED;
    }
    WARNLOG("AddNfs GetTaskId %s in host failed: %s, %s", response->GetTaskId().c_str(),
            hostId.c_str(), m_jobId.c_str());
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("AddNfs %s in host failed: %s, %s", nfsIp.c_str(),
            hostId.c_str(), m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("AddNfs %s  in host succeed: %s, %s", nfsIp.c_str(),
        hostId.c_str(), m_jobId.c_str());
    return SUCCESS;
}

bool CNwareProtectEngine::CheckStore(const std::string &nfsIp, std::string &storeId)
{
    INFOLOG("Enter");
    NfsRequest req;
    SetCommonInfo(req);
    if (!req.SetNfsReq("")) {
        ERRLOG("CheckStore SetNfsReq failed.");
        return false;
    }
    if (m_cnwareClient == nullptr) {
        ERRLOG("CheckStore m_cnwareClient pointer failed.");
        return false;
    }
    std::shared_ptr<NfsStoreResponse> response = m_cnwareClient->GetNfsInfo(req);
    if (response == nullptr) {
        ERRLOG("CheckStore response pointer null. %s", m_jobId.c_str());
        return false;
    }
    NfsStoreRes storeInfo = response->GetNfsStore();
    for (const auto &store : storeInfo.m_data) {
        if (store.m_ip == nfsIp) {
            storeId = store.m_id;
            DBGLOG("CheckStoreUnique %s succeed, %s", storeId.c_str(), m_jobId.c_str());
            return true;
        }
    }
    ERRLOG("GetStoreId of %s not find, %s", nfsIp.c_str(), m_jobId.c_str());
    return false;
}

int32_t CNwareProtectEngine::GetResourceId(const std::string &name,
    const std::string &storeId, std::string &resourceId)
{
    INFOLOG("Enter");
    StoreResourceRequest req;
    SetCommonInfo(req);
    if (!req.SetStoreResourceReq(name, storeId)) {
        ERRLOG("GetResourceId SetNfsReq failed.");
        return FAILED;
    }
    if (m_cnwareClient == nullptr) {
        ERRLOG("GetResourceId m_cnwareClient pointer failed.");
        return FAILED;
    }
    std::shared_ptr<StoreResourceResponse> response = m_cnwareClient->GetNfsStorageResource(req);
    if (response == nullptr) {
        ERRLOG("GetResourceId response pointer null. %s", m_jobId.c_str());
        return FAILED;
    }
    StoreResourceRes resourceInfo = response->GetStoreResource();
    for (const auto &resource : resourceInfo.m_data) {
        if (resource.m_resourceName == name) {
            resourceId = resource.m_id;
            DBGLOG("GetResourceId(%s) %s succeed, %s", name.c_str(), resource.m_id.c_str(),
                m_jobId.c_str());
            return SUCCESS;
        }
    }
    ERRLOG("GetResourceId not find resource %s. %s", name.c_str(), m_jobId.c_str());
    return FAILED;
}

int32_t CNwareProtectEngine::GetStorageId(const std::string &hostId, const std::string &name, std::string &storageId)
{
    INFOLOG("Enter");
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("GetStorageId m_cnwareClient pointer failed.");
        return FAILED;
    }
    int32_t start = 0;
    int32_t total = 0;
    int32_t pageNum = Module::ConfigReader::getInt("CNwareConfig", "RequestPageNums");
    if (pageNum <= 0) {
        ERRLOG("Invalid pageNum: %d", pageNum);
        return FAILED;
    }
    do {
        start++;
        std::shared_ptr<StoragePoolResponse> response = m_cnwareClient->GetStoragePoolInfo(
            req, hostId, "", start, pageNum);
        if (response == nullptr) {
            ERRLOG("GetStorageId failed.");
            return FAILED;
        }
        total = response->GetStoragePoolInfo().m_total;
        for (const auto &pool : response->GetStoragePoolInfo().m_data) {
            if (pool.m_name == name) {
                storageId = pool.m_id;
                DBGLOG("GetStorageId(%s) %s succeed, %s", name.c_str(), pool.m_id.c_str(),
                    m_jobId.c_str());
                return SUCCESS;
            }
        }
    } while (total > (start * pageNum));
    ERRLOG("GetStorageId not find resource. %s", m_jobId.c_str());
    return FAILED;
}

int32_t CNwareProtectEngine::RefreshStoragePool(const std::string &storageId)
{
    INFOLOG("Enter");
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("RefreshStoragePool m_cnwareClient pointer failed.");
        return FAILED;
    }
    std::shared_ptr<StoreScanResponse> response = m_cnwareClient->RefreshStoragePool(req, storageId);
    if (response == nullptr) {
        ERRLOG("RefreshStoragePool response pointer null. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("RefreshStoragePool %s failed, %s", storageId.c_str(), m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("RefreshStoragePool %s succeed, %s", storageId.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::ScanNfsStorage(const std::string &storeId)
{
    INFOLOG("Enter");
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("ScanNfsStorage m_cnwareClient pointer failed.");
        return FAILED;
    }
    std::shared_ptr<StoreScanResponse> response = m_cnwareClient->ScanNfsStorage(req, storeId);
    if (response == nullptr) {
        ERRLOG("ScanNfsStorage response pointer null. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("ScanNfsStorage %s failed, %s", storeId.c_str(), m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("ScanNfsStorage %s succeed, %s", storeId.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::CreateStoragePool(const std::string &name,
    const std::string &hostId, const std::string &resourceId)
{
    INFOLOG("Enter");
    AddNfsStorageRequest req;
    SetCommonInfo(req);
    if (!req.SetNfsStorageReq(name, hostId, resourceId)) {
        ERRLOG("CreateStoragePool SetNfsStorageReq failed.");
        return FAILED;
    }
    if (m_cnwareClient == nullptr) {
        ERRLOG("CreateStoragePool m_cnwareClient pointer failed.");
        return FAILED;
    }
    std::shared_ptr<StoreScanResponse> response = m_cnwareClient->AddNfsStorage(req);
    if (response == nullptr) {
        ERRLOG("CreateStoragePool response pointer null. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("CreateStoragePool %s in host failed: %s, %s", resourceId.c_str(),
            hostId.c_str(), m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("CreateStoragePool %s in host succeed: %s, %s", resourceId.c_str(),
        hostId.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::CheckBeforeMount()
{
    INFOLOG("Enter");
    int32_t statusFlag {0};
    std::string hostId {};
    if (m_application.subType == "CNwareHost") {
        hostId = m_application.id;
    } else if (m_application.subType == "CNwareVm") {
        hostId = m_application.parentId;
    } else {
        hostId = "";
    }
    if (!CheckHostStatus(statusFlag, hostId)) {
        ERRLOG("Check host Status Failed, %s", m_taskId.c_str());
        ApplicationLabelType hostCheckLabel;
        hostCheckLabel.level = JobLogLevel::TASK_LOG_ERROR;
        hostCheckLabel.label = CNwareErrorCode::CNWARE_HOST_CHECK_FAILED_LABEL;
        hostCheckLabel.params = std::vector<std::string>{hostId, HOST_STATUS_MAP[statusFlag]};
        ReportJobDetail(hostCheckLabel);
        return FAILED;
    }
    if (GetNewVMMachineInfo() != SUCCESS) {
        ERRLOG("GetNewVMMachineInfo vm name unique failed. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckVMNameValidity(m_config.m_name) || CheckVMNameUnique(m_config.m_name) != SUCCESS) {
        ERRLOG("Check vm name failed. %s", m_jobId.c_str());
        return FAILED;
    }
    if (GetIpStrForLivemount() != SUCCESS) {
        ERRLOG("Get IpStr For Livemount failed. %s", m_jobId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm)
{
    m_reportParam.label = "";
    m_reportArgs = std::vector<std::string>{};
    if (m_cnwareClient == nullptr || m_application.id.empty()) {
        ERRLOG("CreateLiveMount ptr nullptr. %s", m_jobId.c_str());
        return FAILED;
    }

    if (GetNewVMMachineInfo() != SUCCESS) {
        ERRLOG("Get new vm machine name failed.");
        return FAILED;
    }
    std::string storeId;
    std::string storageId;
    int32_t iRet = DoCreateLiveMount(copyVm, newVm, storeId, storageId);
    SaveLiveMountStorageInfo(newVm, storeId, storageId);
    return iRet;
}

int32_t CNwareProtectEngine::PrepareStorageDevice(std::string &storeId)
{
    // 添加Nas存储
    if (m_nfsAddress.empty()) {
        ERRLOG("NfsAddress null failed.");
        return FAILED;
    }
    if (!CheckStore(m_nfsAddress, storeId)) {
        ApplicationLabelType labelParam;
        labelParam.label = CNwareErrorCode::CNWARE_ADD_STORAGE_LABEL;
        labelParam.params = std::vector<std::string>{m_application.name};
        ReportJobDetail(labelParam);
        std::string storeName = "Nas" + m_nfsAddress;
        std::replace(storeName.begin(), storeName.end(), '.', '_');
        std::replace(storeName.begin(), storeName.end(), ':', '_');

        if (AddNfs(storeName, m_application.id,
            m_nfsAddress) != SUCCESS) {
            ERRLOG("AddNfs(%s) to host(%s). %s", m_nfsAddress.c_str(),
                m_application.id.c_str(), m_jobId.c_str());
            labelParam.label = CNwareErrorCode::CNWARE_ADD_STORAGE_FAILED_LABEL;
            labelParam.level = JobLogLevel::TASK_LOG_ERROR;
            ReportJobDetail(labelParam);
            return FAILED;
        }
        if (!CheckStore(m_nfsAddress, storeId)) {
            return FAILED;
        }
    }

    // 扫描存储文件系统
    if (ScanNfsStorage(storeId) != SUCCESS) {
        ERRLOG("ScanNfsStorage(%s). %s", storeId.c_str(), m_jobId.c_str());
        m_reportArgs = std::vector<std::string>{m_application.name};
        m_reportParam.label = CNwareErrorCode::CNWARE_SCAN_STORAGE_FAILED_LABEL;
        m_reportParam.level = JobLogLevel::TASK_LOG_ERROR;
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::PrepareStoragePool(const std::string &storeId, std::string &resourceId,
    std::string &storageId, std::string &liveStorageName)
{
    // 获取对应data仓资源Id
    DBGLOG("GetResourceId(%s). %s", m_livemountRepoPath.c_str(), m_jobId.c_str());
    if (GetResourceId(m_livemountRepoPath, storeId, resourceId) != SUCCESS) {
        ERRLOG("GetResourceId(%s). %s", m_livemountRepoPath.c_str(), m_jobId.c_str());
        return FAILED;
    }
    // 创建对应存储池，关联目标主机
    ApplicationLabelType labelParam;
    labelParam.label = CNwareErrorCode::CNWARE_CREATE_STORAGE_POOL_LABEL;
    ReportJobDetail(labelParam);
    if (CreateStoragePool(liveStorageName, m_application.id,
        resourceId) != SUCCESS) {
        ERRLOG("CreateStoragePool(%s) from resourceId(%s). %s", liveStorageName.c_str(),
            resourceId.c_str(), m_jobId.c_str());
        labelParam.label = CNwareErrorCode::CNWARE_CREATE_STORAGE_POOL_FAILED_LABEL;
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    // 获取存储池id
    if (GetStorageId(m_application.id, liveStorageName, storageId) != SUCCESS) {
        ERRLOG("GetResourceId(%s). %s", m_livemountRepoPath.c_str(), m_jobId.c_str());
        return FAILED;
    }
        // 刷新存储池配置
    if (RefreshStoragePool(storageId)) {
        ERRLOG("RefreshStoragePool(%s). %s", storageId.c_str(), m_jobId.c_str());
        m_reportParam.label = CNwareErrorCode::CNWARE_REFRESH_STORAGE_POOL_FAILED_LABEL;
        m_reportParam.level = JobLogLevel::TASK_LOG_ERROR;
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoCreateLiveMount(const VMInfo &copyVm, VMInfo &newVm,
    std::string &storeId, std::string &storageId)
{
    if (PrepareStorageDevice(storeId) != SUCCESS) {
        ERRLOG("Prepare cnware storage device failed. %s", m_jobId.c_str());
        return FAILED;
    }

    std::string resourceId;
    std::string liveStorageName = "TemporarySpForIR-" + m_jobId;
    std::replace(liveStorageName.begin(), liveStorageName.end(), '-', '_');
    DBGLOG("Try create live mount temp storage pool(%s)", liveStorageName.c_str());
    if (PrepareStoragePool(storeId, storageId, resourceId, liveStorageName) != SUCCESS) {
        ERRLOG("Prepare cnware storage pool failed. %s", m_jobId.c_str());
        return FAILED;
    }
    std::string hostId {};
    std::string hostName {};
    if (m_application.subType == "CNwareHost") {
        hostId = m_application.id;
        hostName = m_application.name;
    } else if (m_application.subType == "CNwareVm") {
        hostId = m_application.parentId;
        hostName = m_application.parentName;
    }
    newVm.m_location = hostId;
    newVm.m_locationName = hostName;
    // 根据磁盘创建即时挂载临时虚拟机
    ApplicationLabelType labelParam;
    labelParam.label = CNwareErrorCode::CNWARE_CREATE_MACHINE_LABEL;
    labelParam.params = std::vector<std::string>{hostName, m_config.m_name};
    ReportJobDetail(labelParam);
    if (BuildLiveVm(copyVm, newVm, hostId, storageId, liveStorageName) != SUCCESS) {
        ERRLOG("BuildLiveVm(%s) from resourceId(%s). %s", liveStorageName.c_str(),
            resourceId.c_str(), m_jobId.c_str());
        labelParam.label = CNwareErrorCode::CNWARE_CREATE_MACHINE_FAILED_LABEL;
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

bool CNwareProtectEngine::FormLiveVolumeMap(const VMInfo &liveVm, const std::string &storageId,
    const std::string &storageName)
{
    INFOLOG("Enter");
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("FormLiveVolumeMap m_cnwareClient nullptr. %s", m_jobId.c_str());
        return false;
    }
    std::shared_ptr<StorageVolumeResponse> response = m_cnwareClient->GetStorageVolume(req, storageId);
    if (response == nullptr) {
        ERRLOG("GetStorageId failed.");
        return FAILED;
    }
    StorageVolumeRes liveVols = response->GetStorageVolume();
    std::string temp;
    for (const VolInfo &vol : liveVm.m_volList) {
        int check = 0;
        temp = vol.m_uuid + ".raw";
        for (const auto &live : liveVols.m_data) {
            if (temp == live.m_name) {
                DBGLOG("Vol(%s), liveVol(%s), %s", vol.m_uuid.c_str(), live.m_name.c_str(),
                    m_jobId.c_str());
                DomainDiskDevicesReq diskReq;
                diskReq.mBus = DISK_BUS_MAP[vol.m_type];
                diskReq.mCache = Module::SafeStoi(vol.m_volumeType);
                diskReq.mPreallocation = PREALLOCATION_OFF;
                diskReq.mOldPool = storageName;
                diskReq.mOldVol = live.m_name;
                diskReq.mShareable = false;
                diskReq.mCapacity = live.m_capacity;
                diskReq.mType = live.m_type;
                diskReq.mSource = static_cast<int>(VolSource::OLD_DISK);
                m_volIdToReq.insert(std::make_pair(vol.m_uuid, diskReq));
                check = 1;
                break;
            }
        }
        if (check == 0) {
            ERRLOG("Not find Vol(%s), name(%s), %s", vol.m_uuid.c_str(), vol.m_name.c_str(),
                m_jobId.c_str());
            return false;
        }
    }
    return true;
}
bool CNwareProtectEngine::DoShutDownBridgeInterface(const std::string &vmId,
    const BridgeInterfaces &brigde)
{
    INFOLOG("Enter");
    if (m_cnwareClient == nullptr) {
        ERRLOG("ShutDownBridgeInterface cnwareClient nullptr. %s", m_jobId.c_str());
        return false;
    }
    InterfaceRequest req;
    SetCommonInfo(req);
    req.SetDomain(vmId);
    req.SetInterfaceReq(brigde, m_targetVMCpuCurrent, DISENABLED_INTERFACE);
    std::shared_ptr<CNwareResponse> response = m_cnwareClient->ModifyInterface(req);
    if (response == nullptr) {
        ERRLOG("DoShutDownBridgeInterface failed. %s", m_jobId.c_str());
        return false;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("DoShutDownBridgeInterface %s in vm failed: %s, %s", brigde.m_mac.c_str(),
            vmId.c_str(), m_jobId.c_str());
        return false;
    }
    DBGLOG("DoShutDownBridgeInterface %s in vm succeed: %s, %s", brigde.m_mac.c_str(),
        vmId.c_str(), m_jobId.c_str());
    return true;
}

int32_t CNwareProtectEngine::ProcessBridgeInterface(const std::string &vmId, const VMInfo &vmInfo)
{
    INFOLOG("Enter");
    AddDomainRequest addDomainReq;
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, addDomainReq)) {
        ERRLOG("Get new vm machine info failed.%s", m_jobId.c_str());
        return FAILED;
    }
    if (addDomainReq.mBridgeInterfaces.size() > 1 &&
        AddBridgeInterface(vmId, addDomainReq.mBridgeInterfaces) != SUCCESS) {
        ERRLOG("AddBridgeInterface failed");
        return FAILED;
    }
    if (ShutDownBridgeInterface(vmId) != SUCCESS) {
        ERRLOG("ShutDownBridgeInterface failed");
        return FAILED;
    }
    INFOLOG("ProcessBridgeInterface SUCCESS");
    return SUCCESS;
}

int32_t CNwareProtectEngine::ShutDownBridgeInterface(const std::string &vmId)
{
    DBGLOG("Enter");
    if (m_jobAdvPara.isMember("openInterface") && m_jobAdvPara["openInterface"].asString() == "true") {
        INFOLOG("Dont shut down bridge interface.");
        return SUCCESS;
    }
    if (m_cnwareClient == nullptr) {
        ERRLOG("ShutDownBridgeInterface cnwareClient nullptr. %s", m_jobId.c_str());
        return FAILED;
    }
    GetVMInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(vmId);
    std::shared_ptr<GetVMInfoResponse> res = m_cnwareClient->GetVMInfo(req);
    if (res == nullptr) {
        ERRLOG("ShutDownBridgeInterface GetVMInfo nullptr. %s", m_jobId.c_str());
        return FAILED;
    }
    for (const auto brigde : res->GetInfo().m_bridgeInterfaces) {
        if (!DoShutDownBridgeInterface(vmId, brigde)) {
            ERRLOG("ShutDownBridgeInterface(%s) failed. %s", brigde.m_mac.c_str(),
                m_jobId.c_str());
        }
    }
    INFOLOG("ShutDownBridgeInterface SUCCESS");
    return SUCCESS;
}

int32_t CNwareProtectEngine::BuildLiveVm(const VMInfo &liveVm, VMInfo &newVm, const std::string &hostId,
    const std::string &storageId, const std::string &storageName)
{
    // 入参为副本虚拟机信息，将id置零，防止创建失败后，误删原虚拟机
    if (!FormLiveVolumeMap(liveVm, storageId, storageName)) {
        return FAILED;
    }
    BuildNewVMRequest createVmReq;
    SetCommonInfo(createVmReq);
    AddDomainRequest addDomainInfo;
    if (FormatLiveMachineParam(liveVm, addDomainInfo) != SUCCESS) {
        ERRLOG("Format create machine param failed.");
        return FAILED;
    }
    newVm.m_name = addDomainInfo.mName;
    createVmReq.SetDomainInfo(addDomainInfo);
    std::shared_ptr<BuildNewVMResponse> response = m_cnwareClient->BuildNewClient(createVmReq);
    if (response == nullptr || !CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Create vm failed.");
        return FAILED;
    }
    DomainListResponse dInfo;
    if (QueryVmInfoByName(newVm.m_name, dInfo) != SUCCESS) {
        ERRLOG("QueryVmIdByName vm failed.");
        return FAILED;
    }
    WaitVMTaskEmpty(dInfo.id);
    newVm.m_uuid = dInfo.id;
    if (!ModifyLiveDevBoots(dInfo, liveVm)) {
        ERRLOG("Modify VM(%s) dev boots failed. %s", dInfo.id.c_str(), m_jobId.c_str());
        return FAILED;
    }
    if (ProcessBridgeInterface(newVm.m_uuid, liveVm) != SUCCESS) {
        ERRLOG("Process BridgeInterface on vm(%s). %s", newVm.m_uuid.c_str(), m_taskId.c_str());
        return FAILED;
    }
    Json::Value extend;
    extend["storageName"] = storageName;
    extend["storageId"] = storageId;
    Json::FastWriter writer;
    newVm.m_extendInfo = writer.write(extend);

    GetVMDiskInfoRequest reqVol;
    SetCommonInfo(reqVol);
    reqVol.SetDomainId(newVm.m_uuid);
    std::shared_ptr<GetVMDiskInfoResponse> respVol = m_cnwareClient->GetVMDiskInfo(reqVol);
    if (respVol == nullptr || !SetVolListInfo2VMInfo(respVol->GetInfo(), newVm)) {
        ERRLOG("Get VM vol list info failed, %s", m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Create vm(%s) id(%s) in host(%s) success. %s", newVm.m_name.c_str(),
        newVm.m_uuid.c_str(), hostId.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::QueryVmInfoByName(const std::string &name, DomainListResponse &dInfo)
{
    // 通过虚拟机名称查询新虚拟机信息
    GetVMListRequest getVMListReq;
    SetCommonInfo(getVMListReq);
    getVMListReq.SetQueryName(name);
    std::shared_ptr<GetVMListResponse> getVMListRsp = m_cnwareClient->GetVMList(getVMListReq);
    if (getVMListRsp == nullptr) {
        ERRLOG("Get vm %s info by name failed. %s", name.c_str(), m_jobId.c_str());
        return FAILED;
    }
    DataResponse listRsp = getVMListRsp->GetVMList();
    if (listRsp.data.empty()) {
        ERRLOG("Get vm info by name failed, no info return.");
        return FAILED;
    }
    dInfo = listRsp.data[0];
    return SUCCESS;
}

bool CNwareProtectEngine::CheckCpuLimit(const uint32_t &coresNum)
{
    if (m_cnwareClient == nullptr || m_application.id.empty()) {
        ERRLOG("CheckDiskType para nullptr! %s", m_jobId.c_str());
        return false;
    }
    if (m_hostCpuCores != 0) {
        return coresNum <= m_hostCpuCores;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    std::shared_ptr<GetHostInfoResponse> response = m_cnwareClient->GetHostInfo(req, m_application.id);
    if (response == nullptr) {
        ERRLOG("GetHostInfoResponse failed! host id: %s", m_application.id.c_str());
        return false;
    }
    HostInfo hostInfo = response->GetHostInfo();
    m_hostCpuCores = hostInfo.cpuCores;
    return coresNum <= hostInfo.cpuCores;
}

void CNwareProtectEngine::SetConfigInfo(AddDomainRequest &domainInfo)
{
    INFOLOG("Enter");
    domainInfo.mName = m_config.m_name;
    uint32_t cpuNums = domainInfo.mCpuInfo.mCore * domainInfo.mCpuInfo.mSockets;
    if (!m_config.m_config.m_cpu.m_useOriginal) {
        DBGLOG("Get new vm m_cpu info %s. %s", m_config.m_config.m_cpu.m_core.c_str(),
            m_config.m_config.m_cpu.m_socket.c_str());
        domainInfo.mCpuInfo.mCore = Module::SafeStoi(m_config.m_config.m_cpu.m_socket);
        domainInfo.mCpuInfo.mSockets = Module::SafeStoi(m_config.m_config.m_cpu.m_core) /
            Module::SafeStoi(m_config.m_config.m_cpu.m_socket);

        cpuNums = domainInfo.mCpuInfo.mCore * domainInfo.mCpuInfo.mSockets;
        DBGLOG("Get new vm m_cpu info %d. %d", domainInfo.mCpuInfo.mCore,
            domainInfo.mCpuInfo.mSockets);
        if (!CheckCpuLimit(cpuNums)) {
            ERRLOG("Check Cpu Limit failed. %s", m_jobId.c_str());
            ApplicationLabelType labelParam;
            labelParam.label = CNwareErrorCode::CNWARE_HOST_CPU_LIMIT_LABEL;
            labelParam.level = JobLogLevel::TASK_LOG_WARNING;
            labelParam.params = std::vector<std::string>{std::to_string(m_hostCpuCores)};
            ReportJobDetail(labelParam);
        }
    }
    WARNLOG("Cpu cores(%d) exceeds current(%d). %s", cpuNums, domainInfo.mCpuInfo.mCurrent, m_jobId.c_str());
    if (cpuNums < domainInfo.mCpuInfo.mCurrent) {
        ERRLOG("Cpu cores(%d) exceeds current(%d). %s", cpuNums, domainInfo.mCpuInfo.mCurrent, m_jobId.c_str());
        domainInfo.mCpuInfo.mCurrent = cpuNums;
    }
    m_targetVMCpuCurrent = domainInfo.mCpuInfo.mCurrent;

    if (!m_config.m_config.m_memory.m_useOriginal) {
        WARNLOG("Get new vm m_memory info %d.", m_config.m_config.m_memory.m_size);
        domainInfo.mMemorySize = Module::SafeStol(m_config.m_config.m_memory.m_size) * MB;
    }
}

int32_t CNwareProtectEngine::FormatLiveMachineParam(const VMInfo &vmInfo, AddDomainRequest &domainInfo)
{
    CNwareVMInfo cwVmInfo;
    DBGLOG("VmInfo metadata: %s", vmInfo.m_metadata.c_str());
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, cwVmInfo)) {
        ERRLOG("Get new vm machine info failed.");
        return FAILED;
    }
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, domainInfo)) {
        ERRLOG("Get new vm machine info failed.");
        return FAILED;
    }
    int index {0};
    for (const auto &bri : cwVmInfo.m_bridgeInterfaces) {
        domainInfo.mBridgeInterfaces.at(index).mQueues = bri.m_queues;
        DBGLOG("DomainInfo %d, bri %d", domainInfo.mBridgeInterfaces.at(index).mQueues, bri.m_queues);
        index++;
    }
    DBGLOG("Get FormatLiveMachineParam new vm meta info %s.", vmInfo.m_metadata.c_str());
    domainInfo.mMemorySize = cwVmInfo.m_memory.m_memory;
    domainInfo.mCpuInfo = cwVmInfo.m_cpu;
    if (!CheckCpuLimit(cwVmInfo.m_cpu.m_cores * cwVmInfo.m_cpu.m_sockets)) {
        ERRLOG("Check Cpu Limit failed. %s", m_jobId.c_str());
        return FAILED;
    }
    SetConfigInfo(domainInfo);
    if (m_application.subType == "CNwareHost") {
        domainInfo.mHostId = m_application.id;
    } else if (m_application.subType == "CNwareVm") {
        domainInfo.mHostId = m_application.parentId;
    } else {
        domainInfo.mHostId = "";
    }
    INFOLOG("Set vm host id: %s", domainInfo.mHostId.c_str());
    if (FormatLiveDiskDev(vmInfo, domainInfo) != SUCCESS) {
        ERRLOG("Format domain disk device info failed. %s", m_jobId.c_str());
        return FAILED;
    }

    if (FormatLiveInterface(domainInfo) != SUCCESS) {
        ERRLOG("Format domain disk device info failed. %s", m_jobId.c_str());
        return FAILED;
    }

    return SUCCESS;
}

int32_t CNwareProtectEngine::DoMigrateLiveVolume(const VMInfo &liveVm)
{
    MigrationRequest migReq;
    SetCommonInfo(migReq);
    GetVMListRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("FormLiveVolumeMap m_cnwareClient nullptr. %s", m_jobId.c_str());
        return FAILED;
    }
    req.SetQueryName(m_config.m_name);
    std::string storageId;
    std::shared_ptr<GetVMListResponse> response = m_cnwareClient->GetVMList(req);
    if (response == nullptr) {
        ERRLOG("GetVMList failed.");
        return FAILED;
    }
    DBGLOG("GetVMList %s.", response->GetVMList().data[0].id.c_str());
    GetVMDiskInfoRequest diskReq;
    SetCommonInfo(diskReq);
    diskReq.SetDomainId(response->GetVMList().data[0].id);
    std::shared_ptr<GetVMDiskInfoResponse> diskResponse = m_cnwareClient->GetVMDiskInfo(diskReq);
    if (diskResponse == nullptr) {
        ERRLOG("GetStorageId failed.");
        return FAILED;
    }
    for (const auto &sub : m_subObjects) {
        if (!AddLiveVolMigReq(diskResponse->GetInfo(), sub, migReq)) {
            ERRLOG("AddLiveVolMigReq failed. %s", m_jobId.c_str());
            return FAILED;
        }
    }
    migReq.SetMigReq(m_application.id);
    migReq.SetDomain(response->GetVMList().data[0].id);
    std::shared_ptr<CNwareResponse> migResponse = m_cnwareClient->MigrateVols(migReq);
    if (migResponse == nullptr) {
        ERRLOG("GetStorageId failed.");
        return FAILED;
    }
    if (!CheckTaskStatus(migResponse->GetTaskId())) {
        ERRLOG("MigrateVols volume failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::MigrateLiveVolume(const VMInfo &liveVm)
{
    if (m_cnwareClient == nullptr || m_subObjects.empty()) {
        ERRLOG("MigrateLiveVolume ptr nullptr. %s", m_jobId.c_str());
        return FAILED;
    }
    ApplicationLabelType labelParam;
    labelParam.label = CNwareErrorCode::CNWARE_VM_MIGRATE_LABEL;
    labelParam.params = std::vector<std::string>{liveVm.m_name};
    ReportJobDetail(labelParam);
    if (DoMigrateLiveVolume(liveVm) != SUCCESS) {
        ERRLOG("MigrateLiveVolume ptr nullptr. %s", m_jobId.c_str());
        labelParam.label = CNwareErrorCode::CNWARE_VM_MIGRATE_FAILED_LABEL;
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

bool CNwareProtectEngine::ParseStorage(const Json::Value &targetVolume, DatastoreInfo &storage, StoragePool &pool)
{
    if (!targetVolume.isMember("datastore")) {
        ERRLOG("Failed to get targetVolume datastore, taskID: %s", m_jobId.c_str());
        return false;
    }
    Json::Value dsJsonValue = targetVolume["datastore"];
    if (!Module::JsonHelper::JsonValueToStruct(dsJsonValue, storage)) {
        ERRLOG("Failed to convert storagePool to Struct datastore, taskID: %s",
            m_jobId.c_str());
        return false;
    }
    if (!storage.m_details.empty()) {
        if (!Module::JsonHelper::JsonStringToStruct(storage.m_details, pool)) {
            ERRLOG("Get poolExtendInfo details info failed.");
            return false;
        }
    }
    if (storage.m_poolId.empty()) {
        WARNLOG("No datastore info provided. %s", m_jobId.c_str());
        return false;
    }
    return true;
}

bool CNwareProtectEngine::AddLiveVolMigReq(const DomainDiskInfoResponse &liveVols,
    const ApplicationResource &sub, MigrationRequest &migReq)
{
    std::string name = sub.id + ".raw";
    Json::Value targetVolume;
    InitRepoHandler();
    if (InitParaAndGetTargetVolume(sub, targetVolume) != SUCCESS) {
        ERRLOG("Get target volume info failed, %s", m_jobId.c_str());
        return false;
    }
    if (!targetVolume.isMember("datastore")) {
        ERRLOG("No datastore provided.");
        return false;
    }
    DatastoreInfo storage;
    StoragePool pool;
    if (!ParseStorage(targetVolume, storage, pool)) {
        ERRLOG("No ParseStorage info provided. %s", m_jobId.c_str());
        return false;
    }
    std::string poolId = storage.m_poolId;
    std::string preallocation = targetVolume.isMember("preallocation") ?
        targetVolume["preallocation"].asString() : DEFAULT_PREALLOCATION;
    for (const auto &disk : liveVols.m_diskDevices) {
        std::vector<std::string> sourceFileSpliteStrs;
        (void)boost::split(sourceFileSpliteStrs, disk.m_sourceFile,
            boost::is_any_of("/"));
        if (sourceFileSpliteStrs.back().empty()) {
            DBGLOG("Get empty sourceFileSpliteStrs sourcefile: %s",
                disk.m_sourceFile.c_str());
            return false;
        }
        DBGLOG("Get sourceFileSpliteStrs sourcefile: %s",
            disk.m_sourceFile.c_str());
        if (sourceFileSpliteStrs.back() == name) {
            preallocation = preallocation.empty() ? disk.m_preallocation : preallocation;
            preallocation = GetStoragePreallocation(pool.m_type, preallocation, name);
            migReq.AddMigVol(disk.m_bus, disk.m_dev, preallocation, poolId);
            DBGLOG("m_bus:%s, m_dev:%s, preallocation:%s, poolId:%s, %s", disk.m_bus.c_str(),
                disk.m_dev.c_str(), preallocation.c_str(), poolId.c_str(), m_jobId.c_str());
            return true;
        }
    }
    ERRLOG("Failed to AddLiveVolMigReq %s, taskID: %s", name.c_str(),
        m_jobId.c_str());
    return false;
}

int32_t CNwareProtectEngine::GetIpStrForLivemount()
{
    INFOLOG("Enter");
    if (m_copy.repositories.empty()) {
        ERRLOG("Repositories is none. %s", m_jobId.c_str());
        return FAILED;
    }
    for (const auto &repo : m_copy.repositories) {
        if (repo.repositoryType != RepositoryDataType::type::DATA_REPOSITORY) {
            continue;
        }
        m_livemountRepoPath = repo.remotePath;
        for (const auto &host : repo.remoteHost) {
            if (host.supportProtocol == PROTOCAL_NFS_CIFS) {
                m_nfsAddress = host.ip;
                DBGLOG("Get storage livemount info! IP: %s, %s",
                    m_nfsAddress.c_str(), GetTaskId().c_str());
                return SUCCESS;
            }
        }
    }
    ERRLOG("Did not find repositories. %s", m_jobId.c_str());
    return FAILED;
}

int32_t CNwareProtectEngine::GetWhiteListForLivemount(std::string &ipStr)
{
    INFOLOG("Enter");
    if (m_cnwareClient == nullptr || m_application.id.empty()) {
        ERRLOG("GetWhiteListForLivemount ptr null failed. %s", m_jobId.c_str());
        return FAILED;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    std::shared_ptr<VswitchsResponse> res = m_cnwareClient->GetVswitchInfo(
        req, m_application.id);
    // 添加白名单
    if (res == nullptr) {
        ERRLOG("GetVswitchsRes nullptr failed. %s", m_jobId.c_str());
        return FAILED;
    }
    std::vector<VswitchsRes> vsList = res->GetVswitchsRes();
    if (vsList.empty()) {
        ERRLOG("GetVswitchsRes failed. %s", m_jobId.c_str());
        return FAILED;
    }
    for (const auto &vs : vsList) {
        if (!vs.m_ip.empty()) {
            ipStr = ipStr + vs.m_ip + ",";
        }
    }
    if (!ipStr.empty()) {
        ipStr.pop_back();
    }
    DBGLOG("GetVswitchsRes %s. %s", ipStr.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::CheckHostArchitectures(
    const VMInfo &vmObj, std::string &copyArch, std::string &targetArch)
{
    DBGLOG("Enter");
    uint32_t iCopyArch = 0;
    if (GetCopyHostArch(vmObj, iCopyArch, copyArch) != SUCCESS) {
        ERRLOG("Get copy host arch failed.");
        return FAILED;
    }
    uint32_t iTargetHostArch = 0;
    if (GetTargetHostArch(iTargetHostArch, targetArch) != SUCCESS) {
        ERRLOG("Get target host arch failed.");
        return FAILED;
    }
    if ((iCopyArch != iTargetHostArch) || ((iCopyArch == 0) && (iTargetHostArch == 0))) {
        ERRLOG("Arch not match, copy arch %d(%s), target arch %d(%s). Also, 0 is invalid arch value. %s",
            iCopyArch, copyArch.c_str(), iTargetHostArch, targetArch.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("CheckHostArchitectures success, copy arch %d(%s), target arch %d(%s). %s",
        iCopyArch, copyArch.c_str(), iTargetHostArch, targetArch.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetCopyHostArch(const VMInfo &vmObj, uint32_t &iCopyArch, std::string &copyArch)
{
    DBGLOG("Enter");
    CNwareVMInfo cwVmInfo;
    if (!Module::JsonHelper::JsonStringToStruct(vmObj.m_metadata, cwVmInfo)) {
        ERRLOG("Get copy vm metadata info failed. %s", m_taskId.c_str());
        return FAILED;
    }
    iCopyArch = cwVmInfo.m_cpu.m_arch;
    if (CNFWARE_CPU_ARCH_MAP.count(iCopyArch) != 0) {
        copyArch = CNFWARE_CPU_ARCH_MAP[iCopyArch];
    } else {
        WARNLOG("Unknown copy cpu arch type: %d. %s", iCopyArch, m_taskId.c_str());
        copyArch = std::to_string(iCopyArch);
    }
    INFOLOG("Copy host arch: %s", copyArch.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetTargetHostArch(uint32_t &iTargetHostArch, std::string &targetArch)
{
    std::string targetHostId;
    if (m_application.subType == "CNwareHost") {
        targetHostId = m_application.id;
    } else if (m_application.subType == "CNwareVm") {
        targetHostId = m_application.parentId;
    } else {
        ERRLOG("Invalid application subType %s, %s", m_application.subType.c_str(), m_jobId.c_str());
        return FAILED;
    }
    if (targetHostId.empty()) {
        ERRLOG("Invalid target host id.", m_jobId.c_str());
        return FAILED;
    }
    QueryHostListRequest req;
    SetCommonInfo(req);
    req.SetHostIds(std::vector<std::string>{ targetHostId });
    std::shared_ptr<QueryHostListResponse> response = m_cnwareClient->QueryHostList(req);
    if (response == nullptr) {
        ERRLOG("Get host %s info failed. %s", targetHostId.c_str(), m_jobId.c_str());
        return FAILED;
    }
    HostDataResponse hostData = response->GetHostData();
    if (hostData.total <= 0) {
        ERRLOG("Get host %s info failed, total %d. %s", targetHostId.c_str(), hostData.total, m_jobId.c_str());
        return FAILED;
    }
    std::string hostName;
    for (auto host : hostData.data) {
        if (host.id == targetHostId) {
            targetArch = host.cpuArchitecture;
            hostName = host.hostname;
        }
    }
    if (targetArch.empty()) {
        ERRLOG("No info returned for host %s. %s", targetHostId.c_str(), m_jobId.c_str());
        return FAILED;
    }
    if (CNFWARE_CPU_ARCH_RMAP.count(targetArch) != 0) {
        iTargetHostArch = CNFWARE_CPU_ARCH_RMAP[targetArch];
    } else {
        WARNLOG("Unknown copy cpu arch type: %S. %s", targetArch.c_str(), m_jobId.c_str());
    }
    INFOLOG("Target host %s arch: %d(%s), %s", hostName.c_str(), iTargetHostArch, targetArch.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::QueryAgentHostStoragePoolList(StoragePoolInfo &poolInfo, const std::string &poolId)
{
    INFOLOG("Enter");
    std::string agentDomainId = Utils::GetProxyHostId(true);
    if (agentDomainId.empty()) {
        ERRLOG("Get agentDomainId failed, %s", m_jobId.c_str());
    }
    CNwareVMInfo agentVmInfo;
    if (!GetVMInfoById(agentDomainId, agentVmInfo)) {
        ERRLOG("Get VM info failed when get meta data, %s", m_jobId.c_str());
        return FAILED;
    }
    // 查询-代理所在主机ID存储池列表
    CNwareRequest req;
    SetCommonInfo(req);
    std::shared_ptr<StoragePoolResponse> response = m_cnwareClient->GetStoragePoolInfo(
        req, agentVmInfo.m_hostId, poolId);
    if (response == nullptr) {
        ERRLOG("GetStoragePoolInfo failed.");
        return FAILED;
    }
    poolInfo = response->GetStoragePoolInfo();
    INFOLOG("QueryAgentHostStoragePoolList success.");
    return SUCCESS;
}

int32_t CNwareProtectEngine::FindSourceVol(const std::string &id, VolInfo &volInfo)
{
    std::string volPairPath = m_cacheRepoPath + VIRT_PLUGIN_VOL_MATCH_INFO;
    VolMatchPairInfo volPairList;
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, volPairPath, volPairList) != SUCCESS) {
        ERRLOG("Load vol pair failed, %s", m_taskId.c_str());
        return FAILED;
    }
    for (const auto &[srcVol, dstVol] : volPairList.m_volPairList) {
        if (dstVol.m_uuid == id) {
            volInfo = srcVol;
            return SUCCESS;
        }
    }
    return FAILED;
}

int32_t CNwareProtectEngine::FindTargetVol(const std::string &id, const VMInfo &vmInfo, VolInfo &targetVol)
{
    for (const auto &vol : vmInfo.m_volList) {
        if (vol.m_uuid == id) {
            targetVol = vol;
            return SUCCESS;
        }
    }
    return FAILED;
}

int32_t CNwareProtectEngine::RestoreSrcToDstDiskMetadata(const VolInfo &srcVol, const VolInfo &dstVol,
    const VMInfo &vmInfo, const DomainDiskInfoResponse &vmDisks)
{
    DBGLOG("src volume: %s, dst volume: %s", srcVol.m_name.c_str(), dstVol.m_name.c_str());
    VolInfo sVolInfo;
    if (LoadCopyVolumeMatadata(srcVol.m_uuid, sVolInfo)) {
        ERRLOG("Load volume %s metadata failed.", srcVol.m_uuid.c_str());
        return FAILED;
    }
    DBGLOG("Disk source meta(in copy): %s, %s",
        WIPE_SENSITIVE(sVolInfo.m_metadata.c_str()), m_jobId.c_str());

    VolInfo dVolInfo;
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_DISK) {
        dVolInfo = dstVol;
    } else {
        if (FindTargetVol(dstVol.m_uuid, vmInfo, dVolInfo) != SUCCESS) {
            ERRLOG("Load volume %s metadata failed. %s", dstVol.m_uuid.c_str(), m_jobId.c_str());
            return FAILED;
        }
    }
    DBGLOG("Disk dest meta(temporarily of the dest volume): %s, %s",
        WIPE_SENSITIVE(dstVol.m_metadata).c_str(), m_jobId.c_str());

    DomainDiskDevicesResp srcDiskInfo;
    DomainDiskDevicesResp dstDiskInfo;
    if (!Module::JsonHelper::JsonStringToStruct(sVolInfo.m_metadata, srcDiskInfo) ||
        !Module::JsonHelper::JsonStringToStruct(dVolInfo.m_metadata, dstDiskInfo)) {
        ERRLOG("Get volume metadata failed.");
        return FAILED;
    }
    std::string vmId = m_restoreLevel == RestoreLevel::RESTORE_TYPE_DISK ? m_application.id : vmInfo.m_uuid;
    if (DoRestoreVolMetadata(vmId, srcDiskInfo, dstDiskInfo, dstVol.m_uuid, vmDisks) != SUCCESS) {
        ERRLOG("Restore disk(name:%s, id:%s) metadata failed, domain id: %s. %s",
            dstVol.m_name.c_str(), dstVol.m_uuid.c_str(), vmId.c_str(), m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Restore volume(%s) metadata on vm(%s) success. %s",
        dstVol.m_name.c_str(), vmId.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::RestoreVolMetadata(VolMatchPairInfo &volPairs, const VMInfo &vmInfo)
{
    DBGLOG("Enter, vol pair list size: %d", volPairs.m_volPairList.size());
    InitRepoHandler();

    if (volPairs.m_volPairList.empty()) {
        WARNLOG("No need to restore vol metadata. %s", m_taskId.c_str());
        return SUCCESS;
    }
    std::string vmId = m_restoreLevel == RestoreLevel::RESTORE_TYPE_DISK ? m_application.id : vmInfo.m_uuid;

    GetVMDiskInfoRequest reqDiskInfo;
    reqDiskInfo.SetDomainId(vmId);
    SetCommonInfo(reqDiskInfo);

    std::shared_ptr<GetVMDiskInfoResponse> responseDiskInfo = m_cnwareClient->GetVMDiskInfo(reqDiskInfo);
    if (responseDiskInfo == nullptr) {
        ERRLOG("Get disk info on vm %s failed, %s", vmId.c_str(), m_taskId.c_str());
        return FAILED;
    }

    for (const auto &[srcVol, dstVol] : volPairs.m_volPairList) {
        if (RestoreSrcToDstDiskMetadata(srcVol, dstVol, vmInfo, responseDiskInfo->GetInfo()) != SUCCESS) {
            ERRLOG("Restore src(%s) to dstDisk(%s) metadata failed, %s", srcVol.m_uuid.c_str(),
                dstVol.m_uuid.c_str(), m_taskId.c_str());
            return FAILED;
        }
    }
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_DISK && !ModifyVMDevBootsForDiskRestore()) {
        ERRLOG("Modify VM(%s) dev boots failed. %s", m_application.id.c_str(), m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Restore disks metadata success. %s", m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoRestoreVolMetadata(const string &domainId, const DomainDiskDevicesResp &srcDiskInfo,
    const DomainDiskDevicesResp &dstDiskInfo, const std::string &volId, const DomainDiskInfoResponse &vmDisks)
{
    UpdateVMDiskRequest req;
    SetCommonInfo(req);

    UpdateDomainDiskRequest diskMeta;
    diskMeta.bus = srcDiskInfo.m_busType;
    diskMeta.busDev = GetBusDevStr(volId, vmDisks);
    diskMeta.cache = srcDiskInfo.m_cache;
    diskMeta.id = volId;
    diskMeta.ioHangTimeout = srcDiskInfo.m_ioHangTimeout;
    diskMeta.readBytesSecond = srcDiskInfo.m_readBytesSecond;
    diskMeta.readIops = srcDiskInfo.m_readIops;
    diskMeta.shareable = srcDiskInfo.m_shareable;
    diskMeta.writeBytesSecond = srcDiskInfo.m_writeBytesSecond;
    diskMeta.writeIops = srcDiskInfo.m_writeIops;

    std::string strMeta;
    if (!Module::JsonHelper::StructToJsonString(diskMeta, strMeta)) {
        WARNLOG("Convert UpdateDomainDiskRequest to json string failed!");
    }
    DBGLOG("Restore disk metadata: %s", WIPE_SENSITIVE(strMeta).c_str());

    req.SetDomainId(domainId);
    req.SetBusDev(diskMeta.busDev);
    req.SetDomainDiskMeta(diskMeta);
    std::shared_ptr<UpdateVMDiskResponse> response = m_cnwareClient->UpdateDomainDiskMetadata(req);
    if (response == nullptr) {
        ERRLOG("Restore volume metadata failed, %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Restore volume metadata %s on storage failed, %s", volId.c_str(), m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Restore volume metadata success, %s", m_jobId.c_str());
    return SUCCESS;
}

std::string CNwareProtectEngine::GetPreallocation(const VolInfo &volObj, const DomainDiskDevicesResp &diskInfo)
{
    if (volObj.m_datastore.m_type == std::to_string(STORAGE_TYPE_NAS) ||
        volObj.m_datastore.m_type == std::to_string(STORAGE_TYPE_CEPH)) {
        WARNLOG("NAS or Ceph pool supports thin disk only. Change type from %s to 'off(thin)' for disk %s. %s",
            diskInfo.m_preallocation.c_str(), volObj.m_name.c_str(), m_jobId.c_str());
        return DEFAULT_PREALLOCATION;
    }
    return diskInfo.m_preallocation;
}

bool CNwareProtectEngine::CheckHostStatus(int32_t &statusFlag, const std::string &hostId)
{
    if (m_cnwareClient == nullptr || hostId.empty()) {
        ERRLOG("CheckDiskType para nullptr! %s", m_jobId.c_str());
        statusFlag = HOST_ERR;
        return false;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    std::shared_ptr<GetHostInfoResponse> response = m_cnwareClient->GetHostInfo(req, hostId);
    if (response == nullptr) {
        ERRLOG("GetHostInfoResponse failed! host id: %s", hostId.c_str());
        statusFlag = HOST_ERR;
        return false;
    }
    HostInfo hostInfo = response->GetHostInfo();
    if (!hostInfo.isConnected || hostInfo.isMaintain) {
        ERRLOG("CheckHostStatus failed! Connected:%d, Maintain:%d, id: %s", static_cast<int>(hostInfo.isConnected),
            static_cast<int>(hostInfo.isMaintain), hostId.c_str());
        statusFlag = hostInfo.isMaintain ? HOST_MAINTAIN : HOST_DISCONNECT;
        return false;
    }
    return true;
}

bool CNwareProtectEngine::CheckVMNotMigrate(const std::string &domainId)
{
    INFOLOG("Enter.");
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("Check VM Not Migrate m_cnwareClient nullptr, %s", m_taskId.c_str());
        return false;
    }
    int32_t start = 0;
    int32_t pageNum = Module::ConfigReader::getInt("CNwareConfig", "RequestPageNums");
    if (pageNum <= 0) {
        ERRLOG("Invalid pageNum: %d", pageNum);
        return false;
    }
    int32_t total = 0;
    do {
        start++;
        std::shared_ptr<QueryVMTaskResponse> response = m_cnwareClient->QueryVMTasks(req, domainId,
            STATUS_BEGINNING_RUNNING, start, pageNum);
        if (response == nullptr) {
            ERRLOG("CheckVMNotMigrate failed! domain id: %s", domainId.c_str());
            return false;
        }
        total = response->GetVMTaskInfo().mTotal;
        for (const auto &task : response->GetVMTaskInfo().mData) {
            if (task.mCode.find(TASK_MIGRATE_CODE) != std::string::npos &&
                (task.mStatus == STATUS_BEGINNING || task.mStatus == STATUS_RUNNING)) {
                ERRLOG("VM is migrating, domain id: %s", domainId.c_str());
                return false;
            }
        }
    } while (total > (start * pageNum));
    return true;
}

bool CNwareProtectEngine::CheckVMStatus()
{
    CNwareVMInfo cnwareVmInfo;
    std::string objectId;
    if (m_jobHandle->GetJobType() == JobType::BACKUP && m_backupPara != nullptr) {
        objectId = m_backupPara->protectObject.id;
        if (!GetVMInfoById(m_backupPara->protectObject.id, cnwareVmInfo)) {
            ERRLOG("Get VM info failed when get meta data, %s", m_taskId.c_str());
            return false;
        }
        if (cnwareVmInfo.m_status == CNWARE_VM_STATUS_ABNORMAL ||
            !CheckVMNotMigrate(m_backupPara->protectObject.id)) {   // 检查虚拟机是否处于迁移状态
            ERRLOG("Target Vm is not in status for backup, status is : %d", cnwareVmInfo.m_status);
            ApplicationLabelType labelParam;
            labelParam.level = JobLogLevel::TASK_LOG_ERROR;
            labelParam.label = CNwareErrorCode::CNWARE_VM_ABNORMAL_LABEL;
            labelParam.params = std::vector<std::string>{};
            ReportJobDetail(labelParam);
            return false;
        }
    } else if (m_jobHandle->GetJobType() == JobType::RESTORE && m_restorePara != nullptr) {
        if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
            INFOLOG("Create new server, dont check server status.");
            return true;
        }
        objectId = m_restorePara->targetObject.id;
        if (!GetVMInfoById(m_restorePara->targetObject.id, cnwareVmInfo)) {
            ERRLOG("Get VM info failed when get meta data, %s", m_taskId.c_str());
            return false;
        }
        if (cnwareVmInfo.m_status == CNWARE_VM_STATUS_ABNORMAL) {
            ERRLOG("Target Vm is not in status for backup, status is : %d", cnwareVmInfo.m_status);
            ApplicationLabelType labelParam;
            labelParam.level = JobLogLevel::TASK_LOG_ERROR;
            labelParam.label = CNwareErrorCode::CNWARE_VM_ABNORMAL_LABEL;
            labelParam.params = std::vector<std::string>{};
            ReportJobDetail(labelParam);
            return false;
        }
    } else {
        ERRLOG("check vm status failed, jobtype: %s. %s", m_jobHandle->GetJobType(), m_taskId.c_str());
        return false;
    }
    if (HealthDomain(objectId) != SUCCESS) {
        ERRLOG("Health domain(%s) failed, %s", objectId.c_str(),
            m_taskId.c_str());
        return false;
    }
    DBGLOG("Check vm state: %d. %s", cnwareVmInfo.m_status, m_taskId.c_str());
    return true;
}

bool CNwareProtectEngine::CheckDiskStatus(const DomainDiskInfoResponse &diskInfo)
{
    ApplicationLabelType diskLabel;
    std::string diskString = "";
    bool bRet {true};
    for (const auto &disk : diskInfo.m_diskDevices) {
        if (!disk.m_status || disk.m_storagePoolId.empty()) {
            ERRLOG("Disk(%s) status failed or storageid(%s) error! %s.", disk.m_sourceFile.c_str(),
                disk.m_storagePoolId.c_str(), m_jobId.c_str());
            diskString = disk.m_sourceFile.empty() ? diskString :
                diskString + GetNameFromSourcefile(disk.m_sourceFile) + ",";
            bRet = false;
        }
    }
    if (!diskString.empty()) {
        diskString.pop_back();
        diskLabel.level = JobLogLevel::TASK_LOG_ERROR;
        diskLabel.label = CNwareErrorCode::CNWARE_VOLUME_STATUS_UNNORMAL_LABEL;
        diskLabel.params = std::vector<std::string>{diskString};
        ReportJobDetail(diskLabel);
    }
    return bRet;
}

bool CNwareProtectEngine::CheckLunDisk(const DomainDiskInfoResponse &diskInfo)
{
    ApplicationLabelType labelParam;
    std::string lunString = "";
    for (auto &vol : diskInfo.m_diskDevices) {
        m_isCeph = vol.m_storagePoolType ==
                static_cast<int32_t>(CNWareStorageType::DISTRIBUTED) ? true : m_isCeph;
        if (vol.m_diskType == ISCSI_BLOCK_LUN_DISK_TYPE &&
            vol.m_storagePoolType == static_cast<int32_t>(CNWareStorageType::ISCSI)) {
            // 保存块存储卷信息，备份完成后重新挂载
            std::string volName = GetNameFromSourcefile(vol.m_sourceFile);
            lunString = volName.empty() ? lunString : (lunString + volName + ",");
            ERRLOG("Find lun block vol(%s) in target vm. %s",
                volName.c_str(), m_jobId.c_str());
            if (volName.empty()) {
                ERRLOG("Find lun blocks vol(%s) in target vm. %s",
                    lunString.c_str(), m_jobId.c_str());
                return false;
            }
        }
    }
    if (!lunString.empty()) {
        lunString.pop_back();
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNwareErrorCode::CNWARE_NOT_SUPPORT_BLOCK_LUN_LABEL;
        labelParam.params = std::vector<std::string>{lunString};
        ReportJobDetail(labelParam);
        ERRLOG("Find lun blocks vol(%s) in target vm. %s",
            lunString.c_str(), m_jobId.c_str());
        return false;
    }
    return true;
}

bool CNwareProtectEngine::CheckDisk()
{
    if (m_jobHandle->GetJobType() == JobType::BACKUP) {
        if (m_cnwareClient == nullptr || m_backupPara == nullptr) {
            ERRLOG("CheckDiskType para nullptr! %s", m_jobId.c_str());
            return false;
        }
        GetVMDiskInfoRequest req;
        SetCommonInfo(req);
        req.SetDomainId(m_application.id);
        std::shared_ptr<GetVMDiskInfoResponse> response = m_cnwareClient->GetVMDiskInfo(req);
        if (response == nullptr) {
            ERRLOG("GetVMDiskInfoResponse failed! domain: %s", m_application.id.c_str());
            return false;
        }
        return CheckLunDisk(response->GetInfo()) && CheckDiskStatus(response->GetInfo());
    } else if (m_jobHandle->GetJobType() == JobType::RESTORE) {
        return true;
    }
}

int32_t CNwareProtectEngine::PowerOffMachine(const VMInfo &vmInfo)
{
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        return SUCCESS;
    }

    ApplicationLabelType labelParam;
    labelParam.label = CNwareErrorCode::CNWARE_POWEROFF_MACHINE_LABEL;
    labelParam.params = std::vector<std::string>{vmInfo.m_name};
    ReportJobDetail(labelParam);

    if (DoPowerOffMachine(vmInfo) != SUCCESS) {
        ERRLOG("DoPowerOffMachine failed. %s", m_taskId.c_str());
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNwareErrorCode::CNWARE_POWEROFF_MACHINE_FAILED_LABEL;
        labelParam.params = std::vector<std::string>{vmInfo.m_name};
        labelParam.errCode = CNwareErrorCode::CNWARE_POWERON_MACHINE_FAILED_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoPowerOffMachine(const VMInfo &vmInfo)
{
    // 查询虚拟机状态，如果是运行状态则关闭电源
    CNwareVMInfo cnwareVmInfo;
    if (!GetVMInfoById(vmInfo.m_uuid, cnwareVmInfo)) {
        ERRLOG("Get VM info failed when get meta data, %s", m_taskId.c_str());
        return FAILED;
    }
    DBGLOG("Query return vm status: %d", cnwareVmInfo.m_status);
    if (cnwareVmInfo.m_status != static_cast<int32_t>(VMStatus::RUNNING)) {
        INFOLOG("No need to power off machine.");
        return SUCCESS;
    }
    if (m_cnwareClient == nullptr) {
        ERRLOG("Query power off m_cnwareClient nullptr. %s", m_jobId.c_str());
        return FAILED;
    }
    CNwareRequest poweroffReq;
    SetCommonInfo(poweroffReq);
    poweroffReq.SetDomainId(vmInfo.m_uuid);
    std::shared_ptr<CNwareResponse> response = m_cnwareClient->PowerOffVM(poweroffReq);
    if (response == nullptr) {
        ERRLOG("Power off vm failed.");
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Query power off vm job status failed. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Power off vm success. %s", m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::HealthDomain(const std::string &domainId)
{
    if (m_cnwareClient == nullptr) {
        ERRLOG("HealthDomain m_cnwareClient nullptr. %s", m_jobId.c_str());
        return FAILED;
    }
    HealthRequest healthReq;
    SetCommonInfo(healthReq);
    healthReq.AddToHealthDomain(domainId);

    std::shared_ptr<CNwareResponse> response = m_cnwareClient->HealthVm(healthReq);
    if (response == nullptr) {
        ERRLOG("Health vm failed.");
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Health vm job status failed. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Health vm success. %s", m_jobId.c_str());
    return SUCCESS;
}

std::string CNwareProtectEngine::GetHostName()
{
    if (!m_serverName.empty()) {
        return m_serverName;
    }
    GetVMInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(Utils::GetProxyHostId(true));
    std::shared_ptr<GetVMInfoResponse> resp = m_cnwareClient->GetVMInfo(req);
    if (resp == nullptr) {
        ERRLOG("Get VM info failed, %s", m_taskId.c_str());
        return "";
    }
    m_serverName = std::move(resp->GetInfo().m_name);
    INFOLOG("Set serverName : %s", m_serverName.c_str());
    return m_serverName;
}
}