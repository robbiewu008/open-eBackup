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
#include "OpenStackProtectEngine.h"
#include <ClientInvoke.h>
#include <vector>
#include <algorithm>
#include "job_controller/jobs/backup/BackupJob.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/resource_discovery/OpenStackMessage.h"
#include "common/Structs.h"
#include "common/uuid/Uuid.h"
#include "curl_http/HttpStatus.h"

using namespace VirtPlugin;

namespace {
    const std::string MODULE = "OpenStackProtectEngine";
    const std::string SERVER_STATUS_SHUTOFF = "SHUTOFF";
    const std::string SERVER_STATUS_ACTIVE = "ACTIVE";
    const std::string SERVER_STATUS_ERROR = "ERROR";
    const std::string OPENSTACK_VOLTYPE_SHARED = "shared";
    const std::string LOCK = "lock";
    const std::string UNLOCK = "unlock";

    const std::string VOLUME_STATUS_INUSE = "in-use";
    const std::string AVAILABLE_STATUS = "available";
    constexpr uint32_t GB_SIZE = 1024 * 1024 * 1024UL;
    static std::atomic<std::int32_t> g_curCreateSnapshotValue { 0 };  // 单代理创建快照限制
    static std::mutex m_mutex;
    const int32_t CREATE_SNAPSHOT_WAIT_TIME = 10;  // unit: s
    const int32_t CREATE_SNAPSHOT_RETRY_WAIT_TIME = 20;  // unit: s
    const int32_t COMMON_WAIT_TIME = 10;
    const std::string OPENSTACK_CREATE_SNAPSHOT_FAILED_GENERAL_CODE = "1577210036";
    const std::string OPENSTACK_INSUFFICIENT_SNAPSHOT_QUOTA = "1577209935";
    const int32_t MAX_EXEC_COUNT = 5;
    const int NOT_DELETE_SNAPSHOT_VOLUME = 0;
    const std::string DELETE_SNAPSHOT_VOLUME_FAILED_ALARM_CODE = "0x6403400004";
    const uint32_t MAX_RETRY_TIMES = 20;
    const uint32_t RETRY_COUNT = 10;
    const std::string OPENSTACK_CONF = "OpenStackConfig";
    const uint64_t CREATE_VM_FAILED_EC = 1593988896;
};

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

bool OpenStackProtectEngine::FormHttpRequest(ModelBase &request, bool useAdmin)
{
    AuthObj auth;
    if (useAdmin) {  // 管理员token采用系统机机帐号
        auth.name = m_appEnv.auth.authkey;
        auth.passwd = m_appEnv.auth.authPwd;
    } else {  // 非管理员token采用domain管理员帐号
        auth.name = m_application.auth.authkey;
        auth.passwd = m_application.auth.authPwd;
        CloudServerResource serverExtendInfo;
        if (!Module::JsonHelper::JsonStringToStruct(m_application.extendInfo, serverExtendInfo)) {
            ERRLOG("FormHttpRequest failed, %s", m_taskId.c_str());
            return false;
        }
        request.SetScopeValue(m_application.parentId);
        request.SetDomain(serverExtendInfo.m_domainName);
        request.SetDomainId(serverExtendInfo.m_domainId);
    }
    auth.certVerifyEnable = m_certMgr->IsVerifyCert();
    auth.cert = m_certMgr->GetCertPath();
    auth.revocationList = m_certMgr->GetRevocationListPath();
    request.SetUserInfo(auth);
    request.SetEnvAddress(m_appEnv.endpoint);

    return true;
}

void OpenStackProtectEngine::InitRepoHandler()
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

void OpenStackProtectEngine::DeleteVolumesAction(const std::string &file, NewCreatedVolumeList &deleteFailVols)
{
    if (m_metaRepoHandler == nullptr) {
        ERRLOG("m_metaRepoHandler is nullptr, RepositoryHandler init failed.");
        return;
    }
    if (!m_metaRepoHandler->Exists(file)) {
        INFOLOG("New volume file(%s) does not exist, %s", file.c_str(), m_taskId.c_str());
        return;
    }
    INFOLOG("Begin to delele volume from file(%s).", file.c_str());
    uint32_t flag = Module::ConfigReader::getInt("OpenStackConfig", "DeleteSnapshotVolume");
    if (flag == NOT_DELETE_SNAPSHOT_VOLUME) {
        WARNLOG("Save volume flag exist.");
        return;
    }
    NewCreatedVolumeList volumeList;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, file, volumeList) != SUCCESS) {
        ERRLOG("Load volume file(%s) failed, %s", file.c_str(), m_taskId.c_str());
        return;
    }
    VolInfo volObj;
    Volume volumeDetail;
    for (const auto &volume : volumeList.m_volumelist) {
        volObj.m_uuid = volume.m_id;
        INFOLOG("Begin to delete volume(%s).", volObj.m_uuid.c_str());
        if (!GetVolumeDetail(volObj.m_uuid, volumeDetail)) {
            ERRLOG("Show volume(%s) detail failed, %s", volObj.m_uuid.c_str(), m_taskId.c_str());
            deleteFailVols.m_volumelist.push_back(volume);
            continue;
        }
        // 若卷处于挂载状态，需要先卸载卷
        if (volumeDetail.m_status == VOLUME_STATUS_INUSE &&
            DetachVolumeHandle(volObj, volumeDetail.m_attachPoints[0].m_serverId) != SUCCESS) {
            deleteFailVols.m_volumelist.push_back(volume);
            continue;
        }
        if (DeleteVolume(volObj) != SUCCESS) {
            ERRLOG("Delete volume(%s) failed, %s", volObj.m_uuid.c_str(), m_taskId.c_str());
            deleteFailVols.m_volumelist.push_back(volume);
            continue;
        }
        INFOLOG("Delete volume(%s) success.", volObj.m_uuid.c_str());
    }
    return;
}

bool OpenStackProtectEngine::CheckVolumesIfDelete(const VolResidualInfo &volResidualInfo)
{
    Volume volume;
    for (const auto &residualVol : volResidualInfo.m_volumelist) {
        if (GetVolumeDetail(residualVol.m_id, volume)) {
            INFOLOG("Show clone volume(%s) detail success, %s", residualVol.m_id.c_str(), m_taskId.c_str());
            return false;
        }
    }
    return true;
}
 
void OpenStackProtectEngine::SaveResidualVolumes()
{
    std::string volResidualListInfoStr;
    if (!Module::JsonHelper::StructToJsonString(m_volResidualList, volResidualListInfoStr)) {
        ERRLOG("Convert volume residual list to json string failed, %s", m_taskId.c_str());
        return;
    }
    std::string volResidualFile = m_metaRepoPath + VIRT_PLUGIN_VOLUME_RESIDUAL;
    if (Utils::SaveToFileWithRetry(m_metaRepoHandler, volResidualFile, volResidualListInfoStr) != SUCCESS) {
        ERRLOG("Save volume residual list failed, %s", m_taskId.c_str());
        return;
    }
    INFOLOG("Save volume residual list success, %s", m_taskId.c_str());
}
 
void OpenStackProtectEngine::ClearVolumesAlarm()
{
    std::vector<VolResidualInfo>::iterator it = m_volResidualList.m_volResidualInfoList.begin();
    while (it != m_volResidualList.m_volResidualInfoList.end()) {
        if (CheckVolumesIfDelete(*it)) {
            ActionResult result;
            AppProtect::AlarmDetails alarm;
            alarm.alarmId = it->m_alarmId;
            alarm.parameter = it->m_param;
            JobService::ClearAlarm(result, alarm);
            INFOLOG("Clear the residule volume alarm(id:%s, para:%s)", it->m_alarmId.c_str(), it->m_param.c_str());
            it = m_volResidualList.m_volResidualInfoList.erase(it);
        } else {
            ++it;
        }
    }
}
 
void OpenStackProtectEngine::ClearResidualVolumesAlarm(const std::string &file)
{
    if (!m_metaRepoHandler->Exists(file)) {
        INFOLOG("Residual volume file(%s) does not exist, %s", file.c_str(), m_taskId.c_str());
        return;
    }
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, file, m_volResidualList) != SUCCESS) {
        ERRLOG("Load volume residual file(%s) failed, %s", file.c_str(), m_taskId.c_str());
        return;
    }
    INFOLOG("Load volume residual info file success, %s", m_taskId.c_str());
    ClearVolumesAlarm();
    SaveResidualVolumes();
}

bool OpenStackProtectEngine::SaveDeleteFailVolumes(const std::string &file, NewCreatedVolumeList &deleteFailVols)
{
    std::string volumeStr;
    if (!Module::JsonHelper::StructToJsonString(deleteFailVols, volumeStr)) {
        ERRLOG("Convert volume list to json string failed, %s", m_taskId.c_str());
        return false;
    }
    if (Utils::SaveToFileWithRetry(m_metaRepoHandler, file, volumeStr) != SUCCESS) {
        ERRLOG("Save volume list failed, %s", m_taskId.c_str());
        return false;
    }

    INFOLOG("Save volume list success, %s", m_taskId.c_str());
    return true;
}

void OpenStackProtectEngine::DeleteVolumes()
{
    NewCreatedVolumeList deleteFailVols;
    std::string newVolumeInfoFile = m_metaRepoPath + VIRT_PLUGIN_SNAPHOT_CREATE_VOLUME_INFO;
    DeleteVolumesAction(newVolumeInfoFile, deleteFailVols);
    SendDeleteVolumeFailedAlarm(deleteFailVols.m_volumelist);
    std::string historyVolFile = m_metaRepoPath + VIRT_PLUGIN_VOLUME_TOBEDELETED;
    DeleteVolumesAction(historyVolFile, deleteFailVols);
    SaveDeleteFailVolumes(historyVolFile, deleteFailVols);
    m_metaRepoHandler->Remove(newVolumeInfoFile);
    DoGetMachineMetadata();
    GetSnapshotCreateVolumesAndDelete();
    std::string residualVolFile = m_metaRepoPath + VIRT_PLUGIN_VOLUME_RESIDUAL;
    ClearResidualVolumesAlarm(residualVolFile);
    return;
}

int OpenStackProtectEngine::PreHook(const ExecHookParam &para)
{
    if (!InitJobPara()) {
        return FAILED;
    }
    InitRepoHandler();
    if (m_jobHandle->GetJobType() == JobType::BACKUP && para.stage == JobStage::POST_JOB) {
        DeleteVolumes();
    }
    if (m_jobHandle->GetJobType() != JobType::RESTORE) {
        return SUCCESS;
    }
    if (!InitRestoreLevel()) {
        return FAILED;
    }
    if (para.stage == JobStage::EXECUTE_SUB_JOB) {
        m_reportParam = {
            "virtual_plugin_restore_job_execute_subjob_start_label",
            JobLogLevel::TASK_LOG_INFO,
            SubJobStatus::RUNNING, 0, 0 };
    }
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        if (para.stage == JobStage::POST_JOB) {
            DeleteOriginServer(para.jobExecRet);
        }
        return SUCCESS;
    }
    if (para.stage == JobStage::EXECUTE_POST_SUB_JOB || para.stage == JobStage::POST_JOB) {
        return ControlMachine(UNLOCK);
    }
    return SUCCESS;
}

int OpenStackProtectEngine::ControlMachine(const std::string& lockType)
{
    ServerRequest request;
    request.SetServerId(m_jobHandle->GetApp().id);
    FormHttpRequest(request);
    std::shared_ptr<ServerResponse> response = m_novaClient.ActServer(request, lockType);
    if (response == nullptr) {
        ERRLOG("%s response is nullptr, %s", lockType.c_str(), m_taskId.c_str());
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("%s server failed. status code: %d, %s", lockType.c_str(), response->GetStatusCode(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("%s machine success, %s, %s", lockType.c_str(), m_jobHandle->GetApp().id.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int OpenStackProtectEngine::PostHook(const ExecHookParam &para)
{
    if (!InitRestoreLevel()) {
        return FAILED;
    }
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        return SUCCESS;
    }
    if (m_jobHandle->GetJobType() == JobType::RESTORE && para.stage == JobStage::PRE_PREREQUISITE) {
        return ControlMachine(LOCK);
    }
    return SUCCESS;
}

bool OpenStackProtectEngine::ParseCert(const std::string& markId, const std::string& certInfo)
{
    if (certInfo.empty()) {
        ERRLOG("Cert info is empty.");
        return false;
    }

    if (m_certMgr == nullptr) {
        ERRLOG("CertMgr is nullptr, %s", m_taskId.c_str());
        return false;
    }
    CertInfo cert;
    if (!m_certMgr->ParseCertInfo(certInfo, cert)) {
        ERRLOG("Failed to save certinfo, %s", m_taskId.c_str());
        return false;
    }
    if (m_certMgr->IsVerifyCert()) {
        if (!m_certMgr->SaveCertToFile(markId)) {
            ERRLOG("Save cert info to file failed, %s", m_taskId.c_str());
            return false;
        }
    } else {
        m_certMgr->RemoveCrtFile(markId);
        m_certMgr->RemoveCrlFile(markId);
    }
    return true;
}

int32_t OpenStackProtectEngine::AllowBackupInLocalNode(const AppProtect::BackupJob& job, int32_t &errorCode)
{
    return CheckProtectEnvConn(job.protectEnv, job.protectObject, job.protectObject.id);
}

int32_t OpenStackProtectEngine::AllowBackupSubJobInLocalNode(const AppProtect::BackupJob& job,
    const AppProtect::SubJob& subJob, int32_t &errorCode)
{
    return CheckProtectEnvConn(job.protectEnv, job.protectObject, job.protectObject.id);
}

int32_t OpenStackProtectEngine::AllowRestoreInLocalNode(const AppProtect::RestoreJob& job, int32_t &errorCode)
{
    return CheckProtectEnvConn(job.targetEnv, job.targetObject, job.targetObject.id);
}

int32_t OpenStackProtectEngine::AllowRestoreSubJobInLocalNode(const AppProtect::RestoreJob& job,
    const AppProtect::SubJob& subJob, int32_t &errorCode)
{
    return CheckProtectEnvConn(job.targetEnv, job.targetObject, job.targetObject.id);
}

int32_t OpenStackProtectEngine::CheckProtectEnvConn(const AppProtect::ApplicationEnvironment &env,
    const AppProtect::Application& app, const std::string &vmId)
{
    DBGLOG("Product env(%s) conn check, %s", vmId.c_str(), m_taskId.c_str());
    if (!ParseCert(env.id, env.auth.extendInfo)) {
        return FAILED;
    }
    m_appEnv = env;
    m_application = app;
    if (GetKeystoneService() == FAILED) {
        ERRLOG("Check protect env connection failed, %s", m_taskId.c_str());
        return FAILED;
    }
    DBGLOG("Product env project(%s) is reachable.", m_application.parentId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::GetKeystoneService()
{
    if (GetService(OPENSTACK_SERVICE_NAME_KEYSTONE_V3) != SUCCESS &&
        GetService(OPENSTACK_SERVICE_NAME_KEYSTONE) != SUCCESS) {
        ERRLOG("Get service detail failed.");
        return FAILED;
    }
    DBGLOG("Get service detail success, %s", m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::GetService(const std::string& name)
{
    GetServicesRequest request;
    FormHttpRequest(request);
    request.SetServiceName(name);
    KeyStoneClient client;
    std::shared_ptr<GetServicesResponse> response = client.GetServices(request);
    if (response == nullptr) {
        ERRLOG("Failed to get services.");
        return FAILED;
    }
    if (response->GetServices().m_services.size() != 1) {
        ERRLOG("No service find, name: %s.", name.c_str());
        return FAILED;
    }
    DBGLOG("Get services success.");
    return SUCCESS;
}

int32_t OpenStackProtectEngine::GetServerDetailById(const std::string &serverId)
{
    GetServerDetailsRequest request;
    if (!FormHttpRequest(request)) {
        ERRLOG("Initialize OpenStack request failed");
        return FAILED;
    }
    request.SetServerId(serverId);
    std::shared_ptr<GetServerDetailsResponse> resp = m_novaClient.GetServerDetails(request);
    if (resp == nullptr) {
        ERRLOG("Check protect env connection failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (resp->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Check protect env failed, get vm(%s) failed, %s", serverId.c_str(), m_taskId.c_str());
        return FAILED;
    }
    DBGLOG("Server(%s) is reachable, %s", serverId.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::DeleteOriginServer(const int32_t &jobResult)
{
    if (jobResult == FAILED) {
        INFOLOG("Job run failed, dont delete origin server.");
        return SUCCESS;
    }
    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(), m_taskId.c_str());
        return FAILED;
    }
    std::string deleteOriginVm = jobAdvancePara["isDeleteOriginalVM"].asString();
    if (deleteOriginVm != "true") {
        INFOLOG("Dont delete origin server.");
        return SUCCESS;
    }
    InitRepoHandler();
    VMInfo originVm;
    if (LoadCopyVmMatadata(originVm) == FAILED) {
        ERRLOG("LoadCopyVmMatadata failed.");
        return FAILED;
    }
    if (DeleteMachineAndVolumes(originVm) == FAILED) {
        ERRLOG("Delete origin vm(%s) failed.", originVm.m_uuid.c_str());
        m_reportArgs = { originVm.m_uuid.c_str() };
        m_reportParam = {
            "virtual_plugin_openstack_delete_origin_server_failed_label",
            JobLogLevel::TASK_LOG_WARNING,
            SubJobStatus::RUNNING, 0, 0 };
        return FAILED;
    }
    INFOLOG("Delete origin vm(%s) success.", originVm.m_uuid.c_str());
    return SUCCESS;
}

void OpenStackProtectEngine::TransVolumeDetail2VolInfo(const Volume &volDetail, VolInfo &volInfo)
{
    volInfo.m_moRef = volDetail.m_id;
    volInfo.m_uuid = volDetail.m_id;
    volInfo.m_name = volDetail.m_name;
    volInfo.m_type = volDetail.m_shareable ? OPENSTACK_VOLTYPE_SHARED : volDetail.m_volumeType;
    volInfo.m_volSizeInBytes = volDetail.m_size * GB_SIZE;
    volInfo.m_bootable = volDetail.m_bootable;
    volInfo.m_volumeType = volDetail.m_volumeType;
    Volume volObj = volDetail;
    Module::JsonHelper::StructToJsonString(volObj, volInfo.m_metadata);
    DBGLOG("Volume size: %llu, metadata(%s)", volInfo.m_volSizeInBytes, volInfo.m_metadata.c_str());
    for (const auto &it : volDetail.m_attachPoints) {
        DBGLOG("Volume attach device is %s, %s", it.m_device.c_str(), m_taskId.c_str());
        volInfo.m_attachPoints.push_back(VolAttachMents(it.m_device));
    }
    DBGLOG("Trans VolumeDetail to VolInfo success.%s %s %s Volume size: %llu  volume type: %s. %s.",
        volInfo.m_uuid.c_str(), volInfo.m_name.c_str(), volInfo.m_type.c_str(), volInfo.m_volSizeInBytes,
        volInfo.m_volumeType.c_str(), m_taskId.c_str());
    return;
}

bool OpenStackProtectEngine::GetVolumeInfo(const std::string &volId, VolInfo &volInfo)
{
    Volume volume;
    if (!GetVolumeDetail(volId, volume)) {
        ERRLOG("Show volume detail failed, %s", m_taskId.c_str());
        return false;
    }
    TransVolumeDetail2VolInfo(volume, volInfo);
    DBGLOG("Get volume detail success, %s", m_taskId.c_str());
    return true;
}

bool OpenStackProtectEngine::GetVolumeDetail(const std::string &volId, Volume &volDetail)
{
    VolumeRequest request;
    FormHttpRequest(request);
    request.SetVolumeId(volId);
    std::shared_ptr<VolumeResponse> response = m_cinderClient->GetVolumeDetail(request);
    if (response == nullptr) {
        ERRLOG("Show volume(%s) detail failed, %s", volId.c_str(), m_taskId.c_str());
        return false;
    }
    volDetail = response->GetVolume();
    DBGLOG("Get volume(%s) detail success, %s", volId.c_str(), m_taskId.c_str());
    return true;
}

bool OpenStackProtectEngine::TransServerDetail2VMInfo(const ServerDetail &serverDetail, VMInfo &vmInfo)
{
    vmInfo.m_moRef = serverDetail.m_hostServerInfo.m_uuid;
    vmInfo.m_uuid = serverDetail.m_hostServerInfo.m_uuid;
    vmInfo.m_name = serverDetail.m_hostServerInfo.m_name;
    vmInfo.m_location = serverDetail.m_hostServerInfo.m_hostId;
    // 虚拟机额外信息保存flavor和网络数据
    ServerExtendInfo serverExtendInfo;
    serverExtendInfo.m_flavor = serverDetail.m_hostServerInfo.m_flavor;
    std::vector<std::string> addresses = serverDetail.m_hostServerInfo.m_addresses;
    for (int i = 0; i < addresses.size(); ++i) {
        Network network;
        network.m_name = addresses[i];
        serverExtendInfo.m_networks.push_back(network);
    }
    if (!Module::JsonHelper::StructToJsonString(serverExtendInfo, vmInfo.m_extendInfo)) {
        ERRLOG("Server extendInfo format conversion failed.");
        return false;
    }
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        INFOLOG("New server has added volInfo before this function.");
        return true;
    }
    for (const auto &serverVol : serverDetail.m_hostServerInfo.m_osExtendedVolumesvolumesAttached) {
        VolInfo volInfo;

        Volume volume;
        if (!GetVolumeDetail(serverVol.m_uuid, volume)) {
            ERRLOG("Show volume detail failed, %s", m_taskId.c_str());
            return false;
        }
        if (volume.m_metaData.m_fullClone == "0") {
            WARNLOG("Exist clone volume, can not backup. id: %s, %s", serverVol.m_uuid.c_str(), m_taskId.c_str());
            continue;
        }
        TransVolumeDetail2VolInfo(volume, volInfo);
        if (volInfo.m_type == OPENSTACK_VOLTYPE_SHARED) {
            WARNLOG("Skipping shared volume backup in openstack, volume uuid: %s, taskId: %s",
                volInfo.m_uuid.c_str(), m_taskId.c_str());
            continue;
        }
        volInfo.m_vmMoRef = serverDetail.m_hostServerInfo.m_uuid;
        vmInfo.m_volList.push_back(volInfo);
    }
    DBGLOG("Trans ServerDetail to VMInfo success, %s", m_taskId.c_str());
    return true;
}

bool OpenStackProtectEngine::DoGetMachineMetadata()
{
    if (m_machineMetaCached) {
        DBGLOG("Machine metadata cached, %s", m_taskId.c_str());
        return true;
    }

    GetServerDetailsRequest request;
    FormHttpRequest(request);

    DBGLOG("Get machine metadata, server id: %s, %s", m_backupPara->protectObject.id.c_str(), m_taskId.c_str());
    request.SetServerId(m_backupPara->protectObject.id);
    std::shared_ptr<GetServerDetailsResponse> resp = m_novaClient.GetServerDetails(request);
    if (resp == nullptr) {
        ERRLOG("Get Server details failed. %s. ",  m_taskId.c_str());
        return false;
    }

    if (!TransServerDetail2VMInfo(resp->GetServerDetails(), m_vmInfo)) {
        ERRLOG("Translate ServerDetail to VMInfo failed, %s", m_taskId.c_str());
        return false;
    }
    m_machineMetaCached = true;
    DBGLOG("Do get machine metadata success, %s", m_taskId.c_str());
    return true;
}

bool OpenStackProtectEngine::ParseCreateSnapResponse(std::shared_ptr<CreateSnapshotResponse> createSnapRes,
    std::string &errCode)
{
    if (createSnapRes->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        if (createSnapRes->GetStatusCode() == static_cast<uint32_t>(Module::SC_REQUEST_ENTITY_TOO_LARGE)) {
            errCode = OPENSTACK_INSUFFICIENT_SNAPSHOT_QUOTA;
        }
        ERRLOG("Create snapshot failed. Return code: %u, %s", createSnapRes->GetStatusCode(), m_taskId.c_str());
        return false;
    }
    Json::Value responseJsonBody;
    if (!Module::JsonHelper::JsonStringToJsonValue(createSnapRes->GetBody(), responseJsonBody)) {
        ERRLOG("Convert the snapshot body failed.");
        return false;
    }
    if (responseJsonBody.isMember("error_code") && responseJsonBody["error_code"].compare("APIGW.0308") == SUCCESS) {
        ERRLOG("ParseCreateSnapResponse(), response: %s. %s",
            WIPE_SENSITIVE(createSnapRes->GetBody()).c_str(), m_taskId.c_str());
        errCode = "APIGW.0308";
    }
    return true;
}

void OpenStackProtectEngine::FillUpVolSnapInfo(const VolInfo &volInfo,
    const SnapshotDetails &snapDetails, const std::string &snapshotStr, VolSnapInfo &volSnap)
{
    volSnap.m_volUuid = volInfo.m_uuid;
    volSnap.m_createTime = snapDetails.m_createdAt;
    volSnap.m_snapshotName = snapDetails.m_name;
    volSnap.m_extendInfo = snapshotStr;
    volSnap.m_snapshotId = snapDetails.m_id;
    volSnap.m_size = volInfo.m_volSizeInBytes / GB_SIZE;

    Json::Value extendInfo;
    extendInfo["group_snapshot_id"] = snapDetails.m_groupSnapshotId;                // 卷组ID
    extendInfo["volume_type"] = volInfo.m_type;
    Json::FastWriter fastWriter;
    volSnap.m_extendInfo = fastWriter.write(extendInfo);
    return;
}

bool OpenStackProtectEngine::IsSnapShotError(const std::string &snapshotId)
{
    VolSnapInfo snapInfo;
    snapInfo.m_snapshotId = snapshotId;
    int retryTimes = 3;
    while (retryTimes > 0) {
        if (!GetVolumeSnapshot(snapInfo)) {
            retryTimes--;
            sleep(COMMON_WAIT_TIME);
            continue;
        }
        if (snapInfo.m_status == AVAILABLE_STATUS) {
            return false;
        }
        if (snapInfo.m_status == SNAPSHOT_STATUS_ERROR) {
            return true;
        }
        retryTimes--;
        sleep(COMMON_WAIT_TIME);
    }
    return false;
}

bool OpenStackProtectEngine::CreateVolumeSnapshot(const VolInfo &volInfo, VolSnapInfo &volSnap, std::string &errCode)
{
    DBGLOG("Volume type: %s", volInfo.m_type.c_str());
    if (volInfo.m_type == OPENSTACK_VOLTYPE_SHARED) {
        WARNLOG("Skipping shared volume. %s", m_taskId.c_str());
        return true;
    }
 
    CreateSnapshotRequestBodyMsg body;
    body.m_snapshot.m_name = GenerateSnapshotName(volInfo.m_uuid);
    body.m_snapshot.m_volumeId = volInfo.m_uuid;
    body.m_snapshot.m_description = m_snapDescription;
    body.m_snapshot.m_force = "true";
 
    CreateSnapshotRequest request;
    request.SetBody(body);
    FormHttpRequest(request);
 
    std::shared_ptr<CreateSnapshotResponse> response = m_cinderClient->CreateSnapshot(request);
    if (response == nullptr) {
        ERRLOG("Create volume snapshot failed, %s", m_taskId.c_str());
        return false;
    }
    if (!ParseCreateSnapResponse(response, errCode)) {
        return false;
    }
    SnapshotDetails snapshotObj = response->GetSnapshotDetails().m_snapshotDetails;
    INFOLOG("Create volume snapshot, response: %s. %s", snapshotObj.m_name.c_str(), m_taskId.c_str());
    if (IsSnapShotError(snapshotObj.m_id)) {
        errCode = OPENSTACK_CREATE_SNAPSHOT_FAILED_GENERAL_CODE;
        ERRLOG("Create snapshot failed, snapshot status is error, %s", m_taskId.c_str());
        volSnap.m_snapshotId = snapshotObj.m_id;
        // 若本次删除失败，可以等待下次全量备份的后置任务，继续重试删除
        if (!DeleteVolumeSnapshot(volSnap)) {
            ERRLOG("Delete volume snapshot failed, volId: %s, snapId: %s, taskId: %s", volInfo.m_uuid.c_str(),
                volSnap.m_snapshotId.c_str(), m_taskId.c_str());
        }
        return false;
    }
    FillUpVolSnapInfo(volInfo, snapshotObj, "", volSnap);
    return true;
}

bool OpenStackProtectEngine::DoCreateSnapshot(const std::vector<VolInfo> &volList,
    SnapshotInfo &snapshot, std::string &errCode)
{
    if (CheckIsConsistenSnapshot()) {
        INFOLOG("Start create consistent snapshot, %s.", m_taskId.c_str());
        return DoCreateConsistencySnapshot(volList, snapshot, errCode);
    }

    INFOLOG("Start create common snapshot, %s.", m_taskId.c_str());
    return DoCreateCommonSnapshot(volList, snapshot, errCode);
}

bool OpenStackProtectEngine::DoCreateCommonSnapshot(const std::vector<VolInfo> &volList,
    SnapshotInfo &snapshot, std::string &errCode)
{
    {
        // 控制多任务竞争
        std::lock_guard<std::mutex> lock(m_mutex);
        int32_t createSnapshotMaxLimit = Module::ConfigReader::getInt(
            std::string("OpenStackConfig"), std::string("CreateSnapshotLimit"));
        DBGLOG("Start adding the snapshot creation task, current snapshot value[%d]. %s",
            g_curCreateSnapshotValue.load(), m_taskId.c_str());
        while (g_curCreateSnapshotValue > createSnapshotMaxLimit) {
            std::this_thread::sleep_for(std::chrono::seconds(CREATE_SNAPSHOT_WAIT_TIME));
        }
        g_curCreateSnapshotValue += volList.size();
    }
 
    DBGLOG("Start to create snapshots. %s", m_taskId.c_str());
    int32_t remainVolListSize = volList.size();
    for (const auto &vol : volList) {
        VolSnapInfo volSnap;
        bool iRet = true;
        int32_t retryTimes = Module::ConfigReader::getInt(
            std::string("OpenStackConfig"), std::string("CreateSnapshotApigwFailedRetry"));
        while (retryTimes > 0 && !CreateVolumeSnapshot(vol, volSnap, errCode)) {
            if (errCode != "APIGW.0308") {
                g_curCreateSnapshotValue -= remainVolListSize;
                ERRLOG("Create volume snapshot failed. Volume id: %s, %s, current snapshot value[%d]",
                    vol.m_uuid.c_str(), m_taskId.c_str(), g_curCreateSnapshotValue.load());
                return false;
            }
 
            ERRLOG("Create volume snapshot failed. Volume id: %s, %s, retryTimes: %d",
                vol.m_uuid.c_str(), m_taskId.c_str(), retryTimes);
            uint64_t randNum = 0;
            if (Utils::GetRandom(randNum) != SUCCESS) {
                randNum = CREATE_SNAPSHOT_RETRY_WAIT_TIME;
            }
            std::this_thread::sleep_for(std::chrono::seconds(CREATE_SNAPSHOT_RETRY_WAIT_TIME +
                randNum % CREATE_SNAPSHOT_RETRY_WAIT_TIME));
            retryTimes--;
        }
 
        if (retryTimes == 0) {
            g_curCreateSnapshotValue -= remainVolListSize;
            ERRLOG("Create volume snapshot failed. Volume id: %s, %s, current snapshot value[%d]", vol.m_uuid.c_str(),
                m_taskId.c_str(), g_curCreateSnapshotValue.load());
            return false;
        }
 
        if (vol.m_type != OPENSTACK_VOLTYPE_SHARED) {
            snapshot.m_volSnapList.push_back(volSnap);
        }
        --g_curCreateSnapshotValue;
        --remainVolListSize;
    }
    return true;
}

bool OpenStackProtectEngine::DoCreateConsistencySnapshot(const std::vector<VolInfo> &volList,
    SnapshotInfo &snapshot, std::string &errCode)
{
    OpenStackConsistentSnapshot openstackConstSnapshot(m_appEnv, m_application);
    if (!openstackConstSnapshot.InitParam(m_jobHandle->GetStorageRepos(), m_certMgr, m_requestId)) {
        ERRLOG("Init param failed, %s", m_taskId.c_str());
        return false;
    }
    return openstackConstSnapshot.DoCreateConsistencySnapshot(volList, snapshot, errCode);
}


int32_t OpenStackProtectEngine::BackUpAllVolumes(SnapshotInfo &snapshot, std::string &errCode)
{
    DBGLOG("Begin to create snapshot. m_volList size: %d. %s", m_vmInfo.m_volList.size(), m_taskId.c_str());
    if (!DoCreateSnapshot(m_vmInfo.m_volList, snapshot, errCode)) {
        ERRLOG("DoCreateSnapshot failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t OpenStackProtectEngine::BackUpTargetVolumes(SnapshotInfo &snapshot, std::string &errCode)
{
    std::vector<VolInfo> volList;
    for (const auto &volObj : m_backupPara->protectSubObject) {
        INFOLOG("Creating volume snapshot. %s %s. %s", volObj.id.c_str(), volObj.name.c_str(), m_taskId.c_str());
        if (m_volMap.find(volObj.id) != m_volMap.end()) {
            volList.emplace_back(m_volMap[volObj.id]);
        }
    }
    if (volList.empty()) {
        ERRLOG("No volume valid to backup, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!DoCreateSnapshot(volList, snapshot, errCode)) {
        ERRLOG("Get machine metadata failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t OpenStackProtectEngine::CreateSnapshot(SnapshotInfo &snapshot, std::string &errCode)
{
    if (!DoGetMachineMetadata()) {
        ERRLOG("Get mechine metadata failed, %s", m_taskId.c_str());
        return FAILED;
    }
 
    if (m_vmInfo.m_volList.empty()) {
        ERRLOG("Failed, volume list is empty. task id: %s", m_taskId.c_str());
        return FAILED;
    }
 
    snapshot.m_moRef = Uuid::GenerateUuid();
    snapshot.m_vmMoRef = m_vmInfo.m_moRef;
    snapshot.m_vmName = m_vmInfo.m_name;

    for (const auto &vol : m_vmInfo.m_volList) {
        m_volMap[vol.m_uuid] = vol;
    }
    DBGLOG("snapshot.m_moRef: %s, snapshot.m_vmMoRef: %s, snapshot.m_vmName: %s. %s", snapshot.m_moRef.c_str(),
        snapshot.m_vmName.c_str(), snapshot.m_vmName.c_str(), m_taskId.c_str());
 
    if (m_backupPara->protectSubObject.size() == 0) {
        /* backup all volumes */
        if (BackUpAllVolumes(snapshot, errCode) != SUCCESS) {
            return FAILED;
        }
    } else {
        /* backup target volumes */
        if (BackUpTargetVolumes(snapshot, errCode) != SUCCESS) {
            return FAILED;
        }
    }
    return SUCCESS;
}

bool OpenStackProtectEngine::CheckIsConsistenSnapshot()
{
    if (!m_backupPara->extendInfo.empty()) {
        Json::Value extendInfo;
        if (!Module::JsonHelper::JsonStringToJsonValue(m_backupPara->extendInfo, extendInfo)) {
            ERRLOG("Trans json value advanceparams failed, %s", m_taskId.c_str());
            return false;
        }
        if (extendInfo.isMember("open_consistent_snapshots") &&
            extendInfo["open_consistent_snapshots"].asString().compare("true") == SUCCESS) {
            return true;
        }
    }
    return false;
}

int32_t OpenStackProtectEngine::DeleteSnapshot(const SnapshotInfo &snapshot)
{
    if (snapshot.m_volSnapList.empty()) {
        DBGLOG("Snapshot list is empty, no volume snapshot to be deleted, %s", m_taskId.c_str());
        return SUCCESS;
    }
    if (CheckIsConsistenSnapshot()) {
        INFOLOG("Start Delete consistent snapshot, %s.", m_taskId.c_str());
        return DoDeleteConsistencySnapshot(snapshot);
    }

    INFOLOG("Start Delete common snapshot, %s.", m_taskId.c_str());
    return DeleteCommonSnapshot(snapshot);
}

int32_t OpenStackProtectEngine::DoDeleteConsistencySnapshot(const SnapshotInfo &snapshot)
{
    OpenStackConsistentSnapshot openstackConstSnapshot(m_appEnv, m_application);
    if (!openstackConstSnapshot.InitParam(m_jobHandle->GetStorageRepos(), m_certMgr, m_requestId)) {
        ERRLOG("Init param failed, %s", m_taskId.c_str());
        return false;
    }
    return openstackConstSnapshot.DeleteConsistencySnapshot(snapshot);
}

bool OpenStackProtectEngine::SendDeleteSnapshotMsg(const VolSnapInfo &volSnap)
{
    DeleteSnapshotPreHook(volSnap);
    DeleteSnapshotRequest request;
    FormHttpRequest(request, true);
    request.SetSnapshotId(volSnap.m_snapshotId);
    for (int i = 0; i < MAX_EXEC_COUNT; i++) {
        std::shared_ptr<DeleteSnapshotResponse> response = m_cinderClient->DeleteSnapshot(request);
        if (response == nullptr) {
            ERRLOG("Send delete snap(%s) msg failed, retry.", volSnap.m_snapshotId.c_str());
            sleep(COMMON_WAIT_TIME);
            continue;
        }
        if (response->GetStatusCode() == static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
            INFOLOG("Snapshot(%s) deleted.", volSnap.m_snapshotId.c_str());
            return true;
        }
        if (response->GetStatusCode() == static_cast<uint32_t>(Module::SC_ACCEPTED)) {
            INFOLOG("Send delete snap(%s) msg success.", volSnap.m_snapshotId.c_str());
            return true;
        }
        ERRLOG("Send delete snap(%s) msg failed, retry.", volSnap.m_snapshotId.c_str());
        sleep(COMMON_WAIT_TIME);
    }
    return false;
}

int32_t OpenStackProtectEngine::DeleteSnapshotPreHook(const VolSnapInfo &volSnap)
{
    return SUCCESS;
}
 

bool OpenStackProtectEngine::ConfirmIfSnapDeleted(const std::string &snapId, int retryTimes, int &curConsumeRetryTimes)
{
    int32_t ret = SUCCESS;
    TP_START("TP_OpenStackDeleteVolumeSnapshotFailed", 1, &ret);
    TP_END
    if (ret == FAILED) {
        return false;
    }
    GetSnapshotRequest request;
    FormHttpRequest(request);
    request.SetSnapshotId(snapId);
    DBGLOG("Confirm whether snapshot exist , snapshot id: %s", snapId.c_str());
    uint32_t retry = 0;
    while (retry++ < retryTimes) {
        curConsumeRetryTimes++;
        std::shared_ptr<GetSnapshotResponse> reps = m_cinderClient->GetSnapshot(request);
        if (reps == nullptr) {
            continue;
        }
        if (reps->GetStatusCode() == static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
            INFOLOG("Snapshot has been deleted, snapshot id: %s.", snapId.c_str());
            return true;
        }
        if (reps->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
            WARNLOG("Get snapshot return status: %s", std::to_string(reps->GetStatusCode()).c_str());
            sleep(SNAPSHOT_RETRY_PERIOD);
            continue;
        }
        SnapshotDetailsMsg snap = reps->GetSnapshotDetails();
        if (snap.m_snapshotDetails.m_status == SNAPSHOT_STATUS_ERROR_DELETING) {
            ERRLOG("Snapshot status is invalid, status: error_deleting.");
            return false;
        }
        WARNLOG("Snapshot does not be deleted, snap status: %s", snap.m_snapshotDetails.m_status.c_str());
        sleep(SNAPSHOT_RETRY_PERIOD);
    }
    return false;
}

bool OpenStackProtectEngine::InitRestoreLevel()
{
    if (m_jobHandle->GetJobType() == JobType::BACKUP) {
        INFOLOG("Backupjob dont init restore level.");
        return true;
    }
    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(), m_taskId.c_str());
        return false;
    }
    if (jobAdvancePara.isMember("restoreLevel")) {
        std::size_t* startPos = nullptr;
        int base = 10;
        m_restoreLevel = RestoreLevel(std::stoi(jobAdvancePara["restoreLevel"].asString(), startPos, base));
    }
    return true;
}
 
int32_t OpenStackProtectEngine::DeleteCommonSnapshot(const SnapshotInfo &snapshot)
{
    std::vector<VolSnapInfo> snapshotList = snapshot.m_volSnapList;
    for (int i = 0; i < MAX_EXEC_COUNT; i++) {
        // 1. send delete msg
        for (const auto &volSnap : snapshotList) {
            INFOLOG("Delete snapshot, snapshot_id: %s. %s", volSnap.m_snapshotId.c_str(), m_taskId.c_str());
            SendDeleteSnapshotMsg(volSnap);
        }
        // 2. confirm delete result
        int retryTimes = SNAPSHOT_DELETE_RETRY_TIMES;
        int curConsumeRetryTimes = 0;
        for (auto it = snapshotList.begin(); it != snapshotList.end();) {
            retryTimes -= curConsumeRetryTimes;
            if (retryTimes < MAX_EXEC_COUNT) {
                retryTimes = MAX_EXEC_COUNT;
            }
            curConsumeRetryTimes = 0;
            if (ConfirmIfSnapDeleted(it->m_snapshotId, retryTimes, curConsumeRetryTimes)) {
                it = snapshotList.erase(it);
            } else {
                it++;
            }
        }
        if (snapshotList.empty()) {
            break;
        }
        sleep(COMMON_WAIT_TIME);
    }
    return snapshotList.empty() ? SUCCESS : FAILED;
}

bool OpenStackProtectEngine::GetVolumeSnapshot(VolSnapInfo &volSnap)
{
    GetSnapshotRequest request;
    FormHttpRequest(request);

    request.SetSnapshotId(volSnap.m_snapshotId);
    std::shared_ptr<GetSnapshotResponse> response = m_cinderClient->GetSnapshot(request);
    if (response == nullptr) {
        ERRLOG("Get volume snapshot failed, %s", m_taskId.c_str());
        return false;
    }
    if (response->GetStatusCode() == static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
        volSnap.m_deleted = true;
    }
    volSnap.m_status = response->GetSnapShotStatus();
    return true;
}

int32_t OpenStackProtectEngine::QuerySnapshotExists(SnapshotInfo &snapshot)
{
    if (snapshot.m_volSnapList.empty()) {
        ERRLOG("No snapshot provided to query, %s", m_taskId.c_str());
        return FAILED;
    }
    snapshot.m_deleted = true;

    for (auto &volSnap : snapshot.m_volSnapList) {
        if (!GetVolumeSnapshot(volSnap)) {
            ERRLOG("Get volume snapshot failed. Volume id: %s, snapshot id: %s. %s", volSnap.m_volUuid.c_str(),
                volSnap.m_snapshotId.c_str(), m_taskId.c_str());
            return FAILED;
        }
        if (!volSnap.m_deleted) {
            snapshot.m_deleted = false;
        }
    }

    return SUCCESS;
}

int32_t OpenStackProtectEngine::GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList)
{
    GetSnapshotListRequest request;
    FormHttpRequest(request);
    request.SetVolumeId(volInfo.m_uuid);
    INFOLOG("Get snapshots of volume(%s).", volInfo.m_uuid.c_str());
    std::shared_ptr<GetSnapshotListResponse> response = m_cinderClient->GetSnapshotList(request);
    if (response == nullptr) {
        ERRLOG("Get snapshot list failed, %s", m_taskId.c_str());
        return FAILED;
    }
    SnapshotDetialList snapDetailList = response->GetSnapshotList();
    for (const auto &snapDetail : snapDetailList.m_snapshots) {
        if (!MatchSnapshotName(snapDetail.m_name) || !MatchSnapshotDescription(snapDetail.m_description)) {
            DBGLOG("Snapshot name(%s) not match.", snapDetail.m_name.c_str());
            continue;
        }
        DBGLOG("Found snapshot [name]%s[description]%s needs to be deleted, %s", snapDetail.m_name.c_str(),
            snapDetail.m_description.c_str(), m_taskId.c_str());
        VolSnapInfo volSnap;
        FillUpVolSnapInfo(volInfo, snapDetail, "", volSnap);
        snapList.push_back(volSnap);
    }
    return SUCCESS;
}

void OpenStackProtectEngine::GetSnapshotCreateVolumesAndDelete()
{
    SnapshotInfo snapInfo;
    for (const auto &volInfo : m_vmInfo.m_volList) {
        INFOLOG("Volume to check snapshot: %s, %s", volInfo.m_uuid.c_str(), m_taskId.c_str());
        std::vector<VolSnapInfo> snapList;
        if (GetSnapshotsOfVolume(volInfo, snapList) != SUCCESS) {
            ERRLOG("Get Snapshot list of volume failed, volId: %s, %s", volInfo.m_uuid.c_str(), m_taskId.c_str());
            continue;
        }
        for (const auto &volSnap : snapList) {
            DeleteSnapshotCreateVolumes(volSnap);
        }
    }
}

/**
 * @brief 检查是否已经在PreHook时删除过
 *
 * @return true 已经在PreHook时进行过删除，删除失败
 * @return false  未在PreHook地删除过
 */
bool OpenStackProtectEngine::CheckIfPreHookDelete(const std::string &volId, const NewCreatedVolumeList &deleteFailVols)
{
    for (const auto &delFailVol : deleteFailVols.m_volumelist) {
        if (delFailVol.m_id == volId) {
            INFOLOG("Volume(%s) already deleted failed in prehook.", volId.c_str());
            return true;
        }
    }
    return false;
}

std::string OpenStackProtectEngine::GetDeleteFailedVolInfo(const std::vector<Volume> &deleteFailVols) const
{
    std::string str;
    for (const auto &vol : deleteFailVols) {
        str += ("[volume id: " + vol.m_id +
            " volume name: " + vol.m_name + "]");
    }
    return std::move(str);
}
 
void OpenStackProtectEngine::SendDeleteVolumeFailedAlarm(const std::vector<Volume> &deleteFailVolumeList)
{
    if (deleteFailVolumeList.empty()) {
        INFOLOG("No new residual volume (%s).", m_taskId.c_str());
        return;
    }
    std::string residualVolumeStr = GetDeleteFailedVolInfo(deleteFailVolumeList);
    INFOLOG("Job(%s) residule Volume detail(%s).", m_taskId.c_str(), residualVolumeStr.c_str());
    m_reportArgs = { residualVolumeStr };
    m_reportParam = {
        "virtual_plugin_openstack_delete_volume_failed_label",
        JobLogLevel::TASK_LOG_WARNING,
        SubJobStatus::RUNNING, 0, 0 };
    ActionResult result;
    AppProtect::AlarmDetails alarm;
    alarm.alarmId = DELETE_SNAPSHOT_VOLUME_FAILED_ALARM_CODE;
    alarm.parameter = m_backupPara->protectEnv.type + "," + m_backupPara->protectObject.id
        + "," + m_taskId + "," + residualVolumeStr;
    alarm.resourceId = m_backupPara->protectObject.id;
    JobService::SendAlarm(result, alarm);
    INFOLOG("Residual volumes alarm(id: %s) send.", alarm.alarmId.c_str());
    VolResidualInfo volResidualInfo;
    volResidualInfo.m_alarmId = alarm.alarmId;
    volResidualInfo.m_jobId = m_taskId.c_str();
    volResidualInfo.m_param = alarm.parameter;
    volResidualInfo.m_volumelist = deleteFailVolumeList;
    m_volResidualList.m_volResidualInfoList.push_back(volResidualInfo);
    SaveResidualVolumes();
}

bool OpenStackProtectEngine::FilterAndDeleteVolumes(const VolumeList &volList)
{
    std::string historyVolFile = m_metaRepoPath + VIRT_PLUGIN_VOLUME_TOBEDELETED;
    NewCreatedVolumeList deleteFailVols;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, historyVolFile, deleteFailVols) != SUCCESS) {
        WARNLOG("Load volume file(%s) failed, %s", historyVolFile.c_str(), m_taskId.c_str());
    }
    VolInfo volObj;
    bool deleteSuccess = true;
    for (const auto &volume : volList.m_volumeList) {
        if (!MatchVolumeName(volume.m_name)) {
            WARNLOG("Volume(%s) does not match.", volume.m_name.c_str());
            continue;
        }
        if (CheckIfPreHookDelete(volume.m_id, deleteFailVols)) {
            deleteSuccess = false;
            continue;
        }
        volObj.m_uuid = volume.m_id;
        if (volume.m_status == VOLUME_STATUS_INUSE &&
            DetachVolumeHandle(volObj, volume.m_attachPoints[0].m_serverId) != SUCCESS) {
            deleteFailVols.m_volumelist.push_back(volume); // 更新删除失败的卷列表
            deleteSuccess = false;
            continue;
        }
        if (DeleteVolume(volObj) != SUCCESS) {
            ERRLOG("Delete volume(%s) failed, %s", volObj.m_uuid.c_str(), m_taskId.c_str());
            deleteFailVols.m_volumelist.push_back(volume); // 更新删除失败的卷列表
            deleteSuccess = false;
            continue;
        }
        DBGLOG("Delete volume(%s) success.", volObj.m_uuid.c_str());
    }
    SaveDeleteFailVolumes(historyVolFile, deleteFailVols);
    return deleteSuccess;
}
/**
 * @brief 删除通过快照创建的克隆卷
 *
 * @param snapInfo
 * @return true 通过当前快照创建的克隆卷已经删除完成
 * @return false 还存在通过当前快照创建的克隆卷
 */
bool OpenStackProtectEngine::DeleteSnapshotCreateVolumes(const VolSnapInfo &snapInfo)
{
    INFOLOG("Begin to get snapshot(%s) clone volumes.", snapInfo.m_snapshotId.c_str());
    InitRepoHandler();
    GetProjectVolumesRequest request;
    request.SetSnapShotId(snapInfo.m_snapshotId);
    FormHttpRequest(request);
    std::shared_ptr<GetProjectVolumesResponse> response = m_cinderClient->GetProjectVolumes(request);
    if (response == nullptr) {
        return false;
    }
    return FilterAndDeleteVolumes(response->GetProjectVolumes());
}

bool OpenStackProtectEngine::CheckArchitecture(const VolInfo &copyObj, const std::string &targetArch)
{
    if (targetArch.empty()) {
        DBGLOG("Target arch is empty, %s", m_taskId.c_str());
        return true;
    }
    Json::Value metadataJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(copyObj.m_metadata, metadataJson)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(copyObj.m_metadata).c_str(), m_taskId.c_str());
        return false;
    }
    std::string copyArch = metadataJson["volume"]["volume_image_metadata"]["architecture"].asString();
    if (copyArch.empty() && targetArch == "x86_64") {
        DBGLOG("Arch check success, %s", m_taskId.c_str());
        return true;
    }
    if (copyArch != targetArch) {
        ERRLOG("Arch check failed, copy arch=%s, target arch=%s, %s", copyArch.c_str(), targetArch.c_str(),
            m_taskId.c_str());
        return false;
    }
    DBGLOG("Arch check success, copy arch=%s, target arch=%s. %s", copyArch.c_str(), targetArch.c_str(),
        m_taskId.c_str());
    return true;
}

/**
 * @brief 检查卷状态
 *
 * @param curVolState
 * @return true
 * @return false
 */
bool OpenStackProtectEngine::CheckVolStatus(const std::string &curVolState)
{
    std::string supportStatus = Module::ConfigReader::getString("OpenStackConfig", "VolSupportStatus");
    std::set<std::string> statusList;
    boost::split(statusList, supportStatus, boost::is_any_of(","));
    std::set<std::string>::iterator pos = statusList.find(curVolState);
    if (pos == statusList.end()) {
        ERRLOG("Can not exec restore for vol status: %s, %s", curVolState.c_str(), m_taskId.c_str());
        return false;
    }
    return true;
}

/**
 * @brief 盘类型检查(启动盘与非启动盘数据不能互相恢复)
 *
 * @return true
 * @return false
 */
bool OpenStackProtectEngine::CheckVolBootMode(const std::string &copyVolBootable, const std::string &targetVolBootable)
{
    if (targetVolBootable != "true") {
        DBGLOG("No need to check for bootable(%s) volume. %s", targetVolBootable.c_str(), m_taskId.c_str());
        return true;
    }
    if (targetVolBootable != copyVolBootable) {
        ERRLOG("Copy vol bootable =%s, target bootable is %s, %s", copyVolBootable.c_str(),
            targetVolBootable.c_str(), m_taskId.c_str());
        return false;
    }
    DBGLOG("Volume bootable check success. %s", m_taskId.c_str());
    return true;
}

/**
 * @brief 检查卷是否可以执行恢复
 *        1. 检查源卷与目标卷存储架构是否一致(arm/x86)
 *        2. 目标卷状态是否满足恢复要求
 *        3. 目标卷大小必须大于等于副本中卷大小
 * @param copyVolMetaData
 * @return true
 * @return false
 */
bool OpenStackProtectEngine::CheckTargetVolume(const VolInfo &copyVolObj, const ApplicationResource &restoreVol)
{
    Json::Value volExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(restoreVol.extendInfo, volExtendInfo)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(restoreVol.extendInfo.c_str()), m_taskId.c_str());
        return false;
    }
    std::string targetVolsInfoStr = volExtendInfo["targetVolume"].asString();
    DBGLOG("TargetVolsInfo %s. %s", targetVolsInfoStr.c_str(), m_taskId.c_str());
    Json::Value targetVolume;
    if (!Module::JsonHelper::JsonStringToJsonValue(targetVolsInfoStr, targetVolume)) {
        ERRLOG("Transfer %s failed, %s.", targetVolsInfoStr.c_str(), m_taskId.c_str());
        return false;
    }
    // 需要新创的卷不需要检查，直接返回
    std::string isNewDisk = targetVolume["isNewDisk"].asString();
    if (isNewDisk == "true") {
        INFOLOG("New volume dont check.");
        return true;
    }
    VolumeRequest request;
    request.SetVolumeId(targetVolume["id"].asString());
    FormHttpRequest(request);
    std::shared_ptr<VolumeResponse> response = m_cinderClient->GetVolumeDetail(request);
    if (response == nullptr) {
        ERRLOG("Show volume response failed, %s", m_taskId.c_str());
        return false;
    }
    Volume volObj = response->GetVolume();
    if (!CheckArchitecture(copyVolObj, volObj.m_volImageMetadata.m_arch)) {
        return false;
    }
    if (!CheckVolStatus(volObj.m_status)) {
        return false;
    }
    if (!CheckVolBootMode(copyVolObj.m_bootable, volObj.m_bootable)) {
        return false;
    }
    uint64_t sizeInBytes = volObj.m_size * GB_SIZE;
    if (sizeInBytes < copyVolObj.m_volSizeInBytes) {
        ERRLOG("Copy vol size %llu, target vol size %llu, %s", copyVolObj.m_volSizeInBytes, sizeInBytes,
            m_taskId.c_str());
        return false;
    }
    DBGLOG("Target volume(%s) check success, %s", targetVolume["id"].asString().c_str(), m_taskId.c_str());
    return true;
}
bool OpenStackProtectEngine::CheckVolumeList(const std::vector<VolInfo> &copyVolList)

{
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        INFOLOG("Create new server and volumes, dont check volume list.");
        return true;
    }
    std::vector<ApplicationResource> volList = m_jobHandle->GetVolumes();
    for (const auto &restoreVol : volList) {
        bool find = false;
        for (const auto &vol : copyVolList) {
            if (vol.m_uuid != restoreVol.id) {
                continue;
            }
            find = true;
            if (!CheckTargetVolume(vol, restoreVol)) {
                return false;
            }
            DBGLOG("volume(%s) check success, %s", restoreVol.id.c_str(), m_taskId.c_str());
            break;
        }
        if (!find) {
            ERRLOG("No match volume for restore vol id %s, %s", restoreVol.id, m_taskId.c_str());
            return false;
        }
    }
    return true;
}

int32_t OpenStackProtectEngine::CheckBeforeBackup()
{
    if (!CheckVMStatus()) {
        ERRLOG("Check VM Status Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t OpenStackProtectEngine::CheckBeforeRecover(const VMInfo &copyVMObj)
{
    if (!InitJobPara()) {
        ERRLOG("Init Job Para failed, task id: %s.", m_taskId.c_str());
        return FAILED;
    }
    if (!InitRestoreLevel()) {
        ERRLOG("Init RestoreLevel failed.");
        return FAILED;
    }

    if (!CheckVMStatus() || !CheckVolumeList(copyVMObj.m_volList)) {
        m_reportParam = {
            "virtual_plugin_restore_job_check_before_recover_failed_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0, 0 };
        return FAILED;
    }
    return SUCCESS;
}

int32_t OpenStackProtectEngine::CheckBackupJobType(const VirtPlugin::JobTypeParam& jobTypeParam, bool& checkRet)
{
    checkRet = true;
    return SUCCESS;
}

int32_t OpenStackProtectEngine::GetMachineMetadata(VMInfo &vmInfo)
{
    if (!DoGetMachineMetadata()) {
        ERRLOG("Get machine metadata failed, %s", m_taskId.c_str());
        return FAILED;
    }
    vmInfo = m_vmInfo;
    return SUCCESS;
}

std::string OpenStackProtectEngine::GetTaskId() const
{
    return "TaskId=" + m_requestId + ".";
}

bool OpenStackProtectEngine::InitJobPara()
{
    DBGLOG("Initialize Openstack protect engine job parameter handler, %s", m_taskId.c_str());
    if (m_initialized && m_backupPara != nullptr && m_restorePara != nullptr) {
        DBGLOG("Initialized, return directly, %s", m_taskId.c_str());
        return true;
    }

    if (m_jobHandle == nullptr || m_jobHandle->GetJobCommonInfo() == nullptr) {
        ERRLOG("Job handler or job common info is null, %s", m_taskId.c_str());
        return false;
    }

    std::shared_ptr<ThriftDataBase> jobInfo = m_jobHandle->GetJobCommonInfo()->GetJobInfo();
    if (m_jobHandle->GetJobType() == JobType::BACKUP) {
        m_backupPara = std::dynamic_pointer_cast<AppProtect::BackupJob>(jobInfo);
        if (m_backupPara == nullptr) {
            ERRLOG("Get backup job parameter failed, %s", m_taskId.c_str());
            return false;
        }
        m_requestId = m_backupPara->requestId;
        m_appEnv = m_backupPara->protectEnv;
        m_application = m_backupPara->protectObject;
    } else if (m_jobHandle->GetJobType() == JobType::RESTORE) {
        m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo);
        if (m_restorePara == nullptr) {
            ERRLOG("Get restore job parameter failed, %s", m_taskId.c_str());
            return false;
        }
        m_requestId = m_restorePara->requestId;
        m_appEnv = m_restorePara->targetEnv;
        m_application = m_restorePara->targetObject;
    } else {
        ERRLOG("Unsupported job type: %d. %s", static_cast<int>(m_jobHandle->GetJobType()), m_taskId.c_str());
        return false;
    }
    m_taskId = GetTaskId();
    m_initialized = true;

    if (!ParseCert(m_appEnv.id, m_appEnv.auth.extendInfo)) {
        ERRLOG("Parse cert fail. job type: %d. %s", static_cast<int>(m_jobHandle->GetJobType()), m_taskId.c_str());
        return false;
    }

    DBGLOG("Initialize Openstack protect engine job parameter handler success, %s", m_taskId.c_str());
    return true;
}

bool OpenStackProtectEngine::GetVolumeSnapshot(SnapshotDetailsMsg &snapDetailsMsg)
{
    GetSnapshotRequest request;
    FormHttpRequest(request);
 
    request.SetSnapshotId(snapDetailsMsg.m_snapshotDetails.m_id);
    std::shared_ptr<GetSnapshotResponse> response = m_cinderClient->GetSnapshot(request);
    if (response == nullptr) {
        ERRLOG("Get volume snapshot(%s) failed, %s", snapDetailsMsg.m_snapshotDetails.m_id.c_str(), m_taskId.c_str());
        return false;
    }

    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Get Volume snapshot error failed, statusCode: %u, taskId: %s",
            response->GetStatusCode(), m_taskId.c_str());
        return false;
    }

    snapDetailsMsg = response->GetSnapshotDetails();
    return true;
}

bool OpenStackProtectEngine::DeleteVolumeSnapshot(const VolSnapInfo &volSnap)
{
    int consumeTime = 0;
    for (int i = 0; i < MAX_EXEC_COUNT; i++) {
        if (!SendDeleteSnapshotMsg(volSnap)) {
            ERRLOG("Send delete snap(%s) msg failed.", volSnap.m_snapshotId.c_str());
            return false;
        }
        if (ConfirmIfSnapDeleted(volSnap.m_snapshotId.c_str(), MAX_EXEC_COUNT, consumeTime)) {
            INFOLOG("Snapshot(%s) delete success.", volSnap.m_snapshotId.c_str());
            return true;
        }
        WARNLOG("Snap(%s) delete failed.", volSnap.m_snapshotId.c_str());
    }
    return false;
}

bool OpenStackProtectEngine::CheckVMStatus()
{
    std::string jobType;

    std::string vmSupportStatus;
    if (m_jobHandle->GetJobType() == JobType::BACKUP) {
        jobType = "backup";
        vmSupportStatus = "VMSupportBackupStatus";
    } else if (m_jobHandle->GetJobType() == JobType::RESTORE) {
        jobType = "restore";
        vmSupportStatus = "VMSupportRestoreStatus";
        if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
            INFOLOG("Create new server, dont check server status.");
            return true;
        }
    } else {
        ERRLOG("check vm status failed, jobtype: %s. %s", m_jobHandle->GetJobType(), m_taskId.c_str());
        return false;
    }

    GetServerDetailsRequest request;
    FormHttpRequest(request);

    request.SetServerId(m_jobHandle->GetApp().id);
    std::shared_ptr<GetServerDetailsResponse> resp = m_novaClient.GetServerDetails(request);
    if (resp == nullptr || resp->GetOsExtstsVmState() == "") {
        ERRLOG("Can not %s. %s", jobType.c_str(), m_taskId.c_str());
        return false;
    }
    std::string supportStatus = Module::ConfigReader::getString("OpenStackConfig", vmSupportStatus);
    std::set<std::string> statusList;
    boost::split(statusList, supportStatus, boost::is_any_of(","));
    std::set<std::string>::iterator pos = statusList.find(resp->GetOsExtstsVmState());
    if (pos == statusList.end()) {
        ERRLOG("Can not exec %s for vmstate: %s. %s", jobType.c_str(),
            resp->GetOsExtstsVmState().c_str(), m_taskId.c_str());
        return false;
    }
    DBGLOG("Can exec %s for vmstate: %s. %s", jobType.c_str(), resp->GetServerStatus().c_str(), m_taskId.c_str());
    return true;
}

bool OpenStackProtectEngine::GetServerDetails(const VMInfo &vmInfo, ServerDetail &serverDetail)
{
    GetServerDetailsRequest request;
    FormHttpRequest(request);

    request.SetServerId(vmInfo.m_uuid);
    std::shared_ptr<GetServerDetailsResponse> resp = m_novaClient.GetServerDetails(request);
    if (resp == nullptr || resp->GetOsExtstsVmState() == "") {
        ERRLOG("Can not get server(%s). %s", vmInfo.m_uuid.c_str(), m_taskId.c_str());
        return false;
    }
    serverDetail = resp->GetServerDetails();
    return true;
}

int32_t OpenStackProtectEngine::GetVolumesMetadata(const VMInfo &vmInfo, std::unordered_map<std::string,
    std::string> &volsMetadata)
{
    return SUCCESS;
}

int32_t OpenStackProtectEngine::GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler)
{
    std::shared_ptr<CinderVolumeHandler> openStackVolHandler =
        std::make_shared<CinderVolumeHandler>(GetJobHandle(), volInfo, m_jobId, m_subJobId);
    if (openStackVolHandler->InitializeVolumeInfo(OPENSTACK_CONF) != SUCCESS) {
        ERRLOG("Initialize volume info failed.");
        return FAILED;
    }
    volHandler = openStackVolHandler;
    return SUCCESS;
}

void OpenStackProtectEngine::FormCreateVolumeBody(const VolInfo &volObj, const std::string &volMetaData,
    std::string &body)
{
    Json::Value jsonBody;
    jsonBody["size"] = volObj.m_volSizeInBytes;
    jsonBody["name"] = volObj.m_name;
    if (!volMetaData.empty()) {
        jsonBody["metadata"] = volMetaData;
    }
    jsonBody["volume_type"] = volObj.m_volumeType;
    Json::Value volume;
    volume["volume"] = jsonBody;
    Json::FastWriter fastWriter;
    body = fastWriter.write(volume);
}
 
int32_t OpenStackProtectEngine::CreateVolume(const VolInfo &backupVol, const std::string &volMetaData,
    const std::string &vmMoRef, const DatastoreInfo &dsInfo, VolInfo &newVol)
{
    int32_t ret = DoCreateVolume(backupVol, volMetaData, vmMoRef, dsInfo, newVol);
    TP_START("TP_CreateVolume", 1, &ret);
    TP_END
    if (ret != SUCCESS) {
        m_reportArgs = { backupVol.m_name.c_str() };
        m_reportParam = {
            "virtual_plugin_openstack_create_volume_failed_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0, 0 };
        ERRLOG("Create volume %s failed, %s.", backupVol.m_name.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Create volume %s success, %s.", newVol.m_uuid.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::DoCreateVolume(const VolInfo &backupVol, const std::string &volMetaData,
    const std::string &vmMoRef, const DatastoreInfo &dsInfo, VolInfo &newVol)
{
    VolumeRequest request;
    FormHttpRequest(request);
    std::string body;
    FormCreateVolumeBody(backupVol, volMetaData, body);
    request.SetBody(body);
    std::shared_ptr<VolumeResponse> response = m_cinderClient->CreateVolume(request);
    if (response == nullptr) {
        ERRLOG("Failed to create volume, response point is nullptr.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Failed to create volume(%s), %s", backupVol.m_name.c_str(), m_taskId.c_str());
        return FAILED;
    }
    newVol.m_newCreate = true;
    Volume newVolume = response->GetVolume();
    TransVolumeDetail2VolInfo(newVolume, newVol);
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        for (const auto &it : backupVol.m_attachPoints) {
            DBGLOG("Volume attach device is %s, %s", it.m_device.c_str(), m_taskId.c_str());
            newVol.m_attachPoints.push_back(VolAttachMents(it.m_device));
        }
    }
    std::vector<std::string> intermediateState = {"creating"};
    if (DoWaitVolumeStatus(newVol.m_uuid, AVAILABLE_STATUS, intermediateState) == FAILED) {
        ERRLOG("Wait volume %s available failed.", newVol.m_uuid.c_str());
        return FAILED;
    }
    DBGLOG("Create volume success, %s", m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::UpdateTargetServerId()
{
    InitRepoHandler();
    std::string newVMMetaDataPath = m_cacheRepoPath + VirtPlugin::VIRT_PLUGIN_CACHE_ROOT + "new_vm.info";
    if (m_cacheRepoHandler->Exists(newVMMetaDataPath)) {
        VMInfo restoreVm;
        if (Utils::LoadFileToStruct(m_cacheRepoHandler, newVMMetaDataPath, restoreVm) != SUCCESS) {
            ERRLOG("Load %s failed, %s", newVMMetaDataPath.c_str(), m_taskId.c_str());
            return FAILED;
        }
        INFOLOG("Read server id %s from new_vm.info.", restoreVm.m_uuid.c_str());
        m_serverId = restoreVm.m_uuid;
    } else {
        m_serverId = m_jobHandle->GetApp().id;
    }
    DBGLOG("My serverid %s", m_serverId.c_str());
    return SUCCESS;
}
 
int32_t OpenStackProtectEngine::AttachVolume(const VolInfo &volObj)
{
    if (UpdateTargetServerId() == FAILED) {
        ERRLOG("UpdateTargetServerId failed.");
        return FAILED;
    }
    int ret = AttachVolumeHandle(volObj, m_serverId);
    TP_START("TP_RestoreAttachVolume", 1, &ret);
    TP_END
    if (ret != SUCCESS) {
        m_reportArgs = { volObj.m_uuid.c_str() };
        m_reportParam = {
            "virtual_plugin_restore_job_attach_volume_failed_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0, 0 };
        return ret;
    }
    m_reportArgs = { m_serverId.c_str() };
    m_reportParam = {
        "virtual_plugin_restore_job_attach_volume_success_label",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING, 0, 0 };
    return SUCCESS;
}

int32_t OpenStackProtectEngine::DetachVolume(const VolInfo &volObj)
{
    if (UpdateTargetServerId() == FAILED) {
        ERRLOG("UpdateTargetServerId failed.");
        return FAILED;
    }
    int ret = DetachVolumeHandle(volObj, m_serverId);
    TP_START("TP_RestoreDetachVolume", 1, &ret);
    TP_END
    if (ret != SUCCESS) {
        m_reportArgs = { volObj.m_uuid.c_str() };
        m_reportParam = {
            "virtual_plugin_restore_job_detach_volume_failed_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0, 0 };
        return ret;
    }
    m_reportArgs = { m_serverId.c_str() };
    m_reportParam = {
        "virtual_plugin_restore_job_detach_volume_success_label",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING, 0, 0 };
    return SUCCESS;
}

int32_t OpenStackProtectEngine::AttachVolumeHandle(const VolInfo &volObj, const std::string& serverId)
{
    std::string attachServerId;
    if (!GetServerWhichAttachedVolume(volObj, attachServerId)) {
        ERRLOG("Get volume(%s) detail failed. %s.", volObj.m_uuid.c_str(), m_taskId.c_str());
        return FAILED;
    }
    if (attachServerId == serverId) {
        INFOLOG("Volume(%s) have been attached to server(%s), %s.",
            volObj.m_uuid.c_str(), serverId.c_str(), m_taskId.c_str());
        return SUCCESS;
    }
    if (!attachServerId.empty()) {     // if volume in other server, need detach it.
        if (DetachVolumeHandle(volObj, attachServerId) != SUCCESS) {
            ERRLOG("Detach volume(%s) from other server(%s) failed. %s.",
                volObj.m_uuid.c_str(), attachServerId.c_str(), m_taskId.c_str());
            return FAILED;
        }
        INFOLOG("Detach volume(%s) from other server(%s) success.", volObj.m_uuid.c_str(), attachServerId.c_str());
    }
    std::string device = "";
    if (volObj.m_attachPoints.size() != 0) {
        INFOLOG("Have existential attach point device set for volume(%s). %s.", volObj.m_uuid.c_str(),
            m_taskId.c_str());
        device = volObj.m_attachPoints[0].m_device;
    }

    if (!RequestAttachVolume(serverId, volObj.m_uuid, device, volObj)) {
        ERRLOG("Request attach volume failed. %s.", m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Attach volume(%s) to server(%s) success. %s.", volObj.m_uuid.c_str(), serverId.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::DetachVolumeHandle(const VolInfo &volObj, const std::string& serverId)
{
    std::string attachServerId;
    if (!GetServerWhichAttachedVolume(volObj, attachServerId)) {
        ERRLOG("Get volume(%s) detail failed. %s.", volObj.m_uuid.c_str(), m_taskId.c_str());
        return FAILED;
    }
    if (attachServerId == "") {
        INFOLOG("Volume(%s) have been detached. %s.", volObj.m_uuid.c_str(), m_taskId.c_str());
        return SUCCESS;
    }

    for (int i = 0; i < MAX_EXEC_COUNT; i++) {
        if (RequestDetachVolume(serverId, volObj.m_uuid)) {
            INFOLOG("Detach volume(%s) success. %s.", volObj.m_uuid.c_str(), m_taskId.c_str());
            return SUCCESS;
        }
        WARNLOG("Request detach volume failed. %s.", m_taskId.c_str());
    }
    ERRLOG("Request detach volume failed. %s.", m_taskId.c_str());
    return FAILED;
}

bool OpenStackProtectEngine::RequestAttachVolume(const std::string& serverId, const std::string& volumeId,
    const std::string& device, const VolInfo &volObj)
{
    AttachServerVolumeRequest attachRequest;
    FormHttpRequest(attachRequest, true);
    attachRequest.SetServerId(serverId);
    attachRequest.SetVolumeId(volumeId);
    if (!device.empty()) {
        attachRequest.SetDevice(device);
    }
    std::shared_ptr<AttachServerVolumeResponse> resp = m_novaClient.AttachServerVolume(attachRequest);
    if (resp == nullptr) {
        ERRLOG("Request attach volume(%s) to server(%s) device(%s) fail.", volumeId.c_str(), serverId.c_str(),
            device.c_str());
        return false;
    }
    if (resp->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Failed to attach volume(%s), %s", volumeId.c_str(), m_taskId.c_str());
        return false;
    }
    std::vector<std::string> intermediateState = {"reserved", "attaching"};
    if (DoWaitVolumeStatus(volumeId, VOLUME_STATUS_INUSE, intermediateState) == FAILED) {
        ERRLOG("Wait volume %s in use failed.", volumeId.c_str());
        return false;
    }
    DBGLOG("Request attach volume(%s) to server(%s) device(%s) success.", volumeId.c_str(), serverId.c_str(),
        device.c_str());
    return true;
}

bool OpenStackProtectEngine::RequestDetachVolume(const std::string& serverId, const std::string& volumeId)
{
    INFOLOG("Begin to detach volume(%s), server id(%s).", volumeId.c_str(), serverId.c_str());
    DetachVolumeRequest detachRequest;
    FormHttpRequest(detachRequest, true);
    detachRequest.SetServerId(serverId);

    std::shared_ptr<ServerResponse> resp = m_novaClient.DetachServerVolume(detachRequest, volumeId);
    if (resp == nullptr) {
        ERRLOG("Request datach volume(%s) to server(%s) fail, %s", volumeId.c_str(), serverId.c_str(),
            m_taskId.c_str());
        return false;
    }
    if (resp->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Failed to detach volume(%s), %s", volumeId.c_str(), m_taskId.c_str());
        return false;
    }
    std::vector<std::string> intermediateState = {"detaching"};
    if (DoWaitVolumeStatus(volumeId, AVAILABLE_STATUS, intermediateState) == FAILED) {
        ERRLOG("Wait volume %s available failed.", volumeId.c_str());
        return false;
    }
    return true;
}

int32_t OpenStackProtectEngine::DeleteVolume(const VolInfo &volObj)
{
    if (DoDeleteVolume(volObj) != SUCCESS) {
        m_reportArgs = { volObj.m_uuid.c_str() };
        m_reportParam = {
            "virtual_plugin_openstack_delete_volume_failed_label",
            JobLogLevel::TASK_LOG_WARNING,
            SubJobStatus::RUNNING, 0, 0 };
        return FAILED;
    }
    return SUCCESS;
}

int32_t OpenStackProtectEngine::DoDeleteVolume(const VolInfo &volObj)
{
    VolumeRequest deleteVolumeReq;
    FormHttpRequest(deleteVolumeReq);
    deleteVolumeReq.SetVolumeId(volObj.m_uuid);
    std::shared_ptr<VolumeResponse> deleteVolumeServerRsp = m_cinderClient->DeleteVolume(deleteVolumeReq);
    if (deleteVolumeServerRsp == nullptr) {
        ERRLOG("Delete volume %s request failed. %s", volObj.m_uuid.c_str(), m_taskId.c_str());
        return FAILED;
    }
    if (deleteVolumeServerRsp->GetBody().find("itemNotFound") != std::string::npos) {
        INFOLOG("Volume %s dont exist, no need delete.", volObj.m_uuid.c_str());
        return SUCCESS;
    }
    if (deleteVolumeServerRsp->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Request delete volume %s request failed. %s", volObj.m_uuid.c_str(), m_taskId.c_str());
        return FAILED;
    }
    uint32_t retryCount = 0;
    while (retryCount < MAX_EXEC_COUNT) {
        std::shared_ptr<VolumeResponse> response = m_cinderClient->GetVolumeDetail(deleteVolumeReq);
        if (response == nullptr || response->GetStatusCode() == static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
            INFOLOG("Volume(%s) was deleted success.", volObj.m_uuid.c_str());
            return SUCCESS;
        }
        sleep(COMMON_WAIT_TIME);
        retryCount += 1;
    }
    ERRLOG("Delete volume %s failed. %s", volObj.m_uuid.c_str(), m_taskId.c_str());
    return FAILED;
}

bool OpenStackProtectEngine::GetServerWhichAttachedVolume(const VolInfo &volInfo, std::string& serverId)
{
    Volume volume;
    serverId = "";
    if (!GetVolumeDetail(volInfo.m_uuid, volume)) {
        ERRLOG("Get volume(%s) detail failed.", volInfo.m_uuid.c_str());
        return false;
    }

    if (volume.m_attachPoints.size() < 1) {
        INFOLOG("volume(%s) no attach to any server.", volInfo.m_uuid.c_str());
        return true;
    }
    serverId = volume.m_attachPoints[0].m_serverId;
    INFOLOG("vol(%s) been attached to server(%s).", volInfo.m_uuid.c_str(), serverId.c_str());
    return true;
}

int32_t OpenStackProtectEngine::ReplaceVolume(const VolInfo &volObj)
{
    return SUCCESS;
}

int32_t OpenStackProtectEngine::LoadCopyVmMatadata(VMInfo &vmInfo)
{
    DBGLOG("RestoreMode: %s", m_restorePara->jobParam.restoreMode.c_str());
    std::shared_ptr<RepositoryHandler> repositoryHandle =
        (m_metaRepoHandler != nullptr) ? m_metaRepoHandler : m_cacheRepoHandler;
    std::string repoPath = (m_metaRepoHandler != nullptr) ? m_metaRepoPath : m_cacheRepoPath;

    std::string vmmetaDataPath = repoPath + VIRT_PLUGIN_VM_INFO;
    if (Utils::LoadFileToStructWithRetry(repositoryHandle, vmmetaDataPath, vmInfo) != SUCCESS) {
        ERRLOG("Failed to load file %s, %s", vmmetaDataPath.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Load file %s success.", repoPath.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::LoadCopyVolumeMatadata(const std::string &volId, VolInfo &volInfo)
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
    INFOLOG("Load file %s success.", repoPath.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::UpdateVolumeBootable(const std::string &volId)
{
    UpdateVolumeBootableRequest request;
    FormHttpRequest(request);
    request.SetVolumeId(volId);
    std::shared_ptr<UpdateVolumeBootableResponse> response = m_cinderClient->UpdateVolumeBootable(request);
    if (response == nullptr) {
        ERRLOG("Update volume %s bootable request failed. %s", volId.c_str(), m_taskId.c_str());
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Update volume %s bootable request failed, status(%d). body(%s), %s", volId.c_str(),
            response->GetStatusCode(), response->GetBody().c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Update volume %s bootable request success", volId.c_str());
    return SUCCESS;
}


int32_t OpenStackProtectEngine::CreateMachine(VMInfo &vmInfo)
{
    std::string sysVolumeId = "";
    // 入参为副本虚拟机信息，将id置零，防止创建失败后，误删原虚拟机
    vmInfo.m_uuid = "";
    std::string volPairPath = m_cacheRepoPath + VIRT_PLUGIN_VOL_MATCH_INFO;
    VolMatchPairInfo volPairList;
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, volPairPath,
        volPairList) != SUCCESS) {
        ERRLOG("Load vol pair failed, %s", m_taskId.c_str());
        return FAILED;
    }
    for (const VolPair &volPair : volPairList.m_volPairList) {
        if (volPair.m_targetVol.m_bootable == "true" && volPair.m_originVol.m_attachPoints[0].m_device.find("da")
            != std::string::npos) {
            INFOLOG("The system volume of new server is %s.", volPair.m_targetVol.m_uuid.c_str());
            sysVolumeId = volPair.m_targetVol.m_uuid;
            break;
        }
    }
    OpenStackServerInfo newServerInfo;
    if (BuildNewServerInfo(newServerInfo) == FAILED) {
        ERRLOG("Build new serverInfo failed. %s", m_taskId.c_str());
        return FAILED;
    }
    int ret = SendCreateMachineRequest(vmInfo, newServerInfo, sysVolumeId);
    TP_START("TP_CreateMachineFailed", 1, &ret);
    TP_END
    if (ret == FAILED) {
        ERRLOG("Task %s create server failed, msg: %s", m_taskId.c_str(), newServerInfo.m_faultInfo.m_message.c_str());
        m_reportArgs = {};
        std::string errLabel = "virtual_plugin_openstack_create_server_failed_label";
        m_reportParam = { errLabel, JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED};
        m_reportParam.errCode = CREATE_VM_FAILED_EC;
        m_reportParam.extendInfo = newServerInfo.m_faultInfo.m_message;
        return FAILED;
    }
    INFOLOG("Create server %s success. %s", vmInfo.m_uuid.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::BuildNewServerInfo(OpenStackServerInfo &newServerInfo)
{
    Json::Value newServerInfoValue;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->targetObject.extendInfo, newServerInfoValue)) {
        ERRLOG("JsonStringToStruct failed. targetMachine's extendInfo %s, %s",
            m_restorePara->targetObject.extendInfo.c_str(), m_taskId.c_str());
        return FAILED;
    }
    if (!newServerInfoValue.isMember("flavor") || !newServerInfoValue.isMember("network") ||
        !newServerInfoValue.isMember("availabilityZone")) {
        ERRLOG("New server's para is null, extend info: %s.", m_restorePara->targetObject.extendInfo.c_str());
        return FAILED;
    }
    newServerInfo.m_name = m_restorePara->targetObject.name;
    Json::Reader reader;
    Json::Value flavorId;
    Json::Value availabilityZoneName;
    Json::Value networkIds;
    reader.parse(newServerInfoValue["flavor"].asString(), flavorId);
    reader.parse(newServerInfoValue["availabilityZone"].asString(), availabilityZoneName);
    reader.parse(newServerInfoValue["network"].asString(), networkIds);
    newServerInfo.m_flavor.m_id = flavorId["id"].asString();
    newServerInfo.m_oSEXTAZavailabilityZone = availabilityZoneName["name"].asString();

    for (int i = 0; i < networkIds.size(); ++i) {
        newServerInfo.m_addresses.push_back(networkIds[i]["id"].asString());
        std::string tIp = "";
        if (networkIds[i].isMember("ip") && networkIds[i]["ip"].isString()) {
            DBGLOG("set ip %s for network id %s", networkIds[i]["ip"].asString().c_str(),
                networkIds[i]["id"].asString().c_str());
            tIp = networkIds[i]["ip"].asString();
        }
        newServerInfo.m_addressesIp.push_back(tIp);
    }
    return SUCCESS;
}

int32_t OpenStackProtectEngine::SendCreateMachineRequest(VMInfo &vmInfo, OpenStackServerInfo &serverExtendInfo,
    std::string &sysVolumeId)
{
    CreateServerRequest request;
    FormHttpRequest(request);
    std::string body = FormCreateServerBody(serverExtendInfo, sysVolumeId);
    if (body.empty()) {
        ERRLOG("FormCreateServerBody failed");
        return FAILED;
    }
    request.SetBody(body);
    std::shared_ptr<CreateServerResponse> response = m_novaClient.CreateServer(request);
    if (response == nullptr) {
        ERRLOG("Create server request failed. %s", m_taskId.c_str());
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Create server failed, status(%d). body(%s), %s", response->GetStatusCode(),
            response->GetBody().c_str(), m_taskId.c_str());
        serverExtendInfo.m_faultInfo.m_message = response->GetBody().c_str();
        return FAILED;
    }
    ServerDetail server = response->GetServerDetail();
    vmInfo.m_uuid = server.m_hostServerInfo.m_uuid;
    if (WaitForServerCreate(server) == FAILED)  {
        ERRLOG("Create server failed.");
        serverExtendInfo = server.m_hostServerInfo;
        return FAILED;
    }
    INFOLOG("Create server success.");
    if (!TransServerDetail2VMInfo(server, vmInfo)) {
        ERRLOG("Translate ServerDetail to VMInfo failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

std::string OpenStackProtectEngine::FormCreateServerBody(OpenStackServerInfo &serverExtendInfo, std::string sysVolumeId)
{
    Json::Value server;
    server["name"] = serverExtendInfo.m_name;
    server["flavorRef"] = serverExtendInfo.m_flavor.m_id;
    server["availability_zone"] = serverExtendInfo.m_oSEXTAZavailabilityZone;
    Json::Value block_device;
    block_device["uuid"] = sysVolumeId;
    block_device["source_type"] = "volume";
    block_device["destination_type"] = "volume";
    block_device["boot_index"] = 0;
    server["block_device_mapping_v2"].append(block_device);
    if (serverExtendInfo.m_addressesIp.size() != serverExtendInfo.m_addresses.size()) {
        ERRLOG("ip and nic mismatch. Nic num: %d, ip num: %d", serverExtendInfo.m_addresses.size(),
            serverExtendInfo.m_addressesIp.size());
        return "";
    }
    for (uint64_t i = 0; i < serverExtendInfo.m_addressesIp.size(); i++) {
        Json::Value network;
        network["uuid"] = serverExtendInfo.m_addresses[i];
        if (!serverExtendInfo.m_addressesIp[i].empty()) {
            network["fixed_ip"] = serverExtendInfo.m_addressesIp[i];
        }
        server["networks"].append(network);
    }
    Json::Value body;
    body["server"] = server;
    Json::FastWriter fastWriter;
    return fastWriter.write(body);
}

int32_t OpenStackProtectEngine::WaitForServerCreate(ServerDetail &server)
{
    GetServerDetailsRequest request;
    FormHttpRequest(request);
    request.SetServerId(server.m_hostServerInfo.m_uuid);
    uint32_t retryTimes = Module::ConfigReader::getInt("OpenStackConfig", "CreateMachineWaitRetryTimes");
    for (int32_t retryTime = 0; retryTime < retryTimes; ++retryTime) {
        sleep(COMMON_WAIT_TIME);
        std::shared_ptr<GetServerDetailsResponse> resp = m_novaClient.GetServerDetails(request);
        if (resp == nullptr) {
            ERRLOG("Get server (%s) detail failed.", server.m_hostServerInfo.m_uuid.c_str());
            return FAILED;
        }
        if (resp->GetOsExtstsVmState() == "" || resp->GetOsExtstsVmState() == "error") {
            server = resp->GetServerDetails();
            ERRLOG("Can not get server(%s) detail, server state: %s. %s", server.m_hostServerInfo.m_uuid.c_str(),
                resp->GetOsExtstsVmState().c_str(), m_taskId.c_str());
            return FAILED;
        }
        if (resp->GetOsExtstsVmState() == "active") {
            server = resp->GetServerDetails();
            INFOLOG("Server has been created successly. %s", m_taskId.c_str());
            return SUCCESS;
        }
        DBGLOG("VM state: %s", resp->GetOsExtstsVmState().c_str());
    }
    ERRLOG("Get server detail of new server %s failed.", server.m_hostServerInfo.m_uuid.c_str());
    return FAILED;
}

int32_t OpenStackProtectEngine::DeleteMachine(const VMInfo &vmInfo)
{
    if (!vmInfo.m_uuid.empty()) {
        if (DoDeleteServer(vmInfo.m_uuid) != SUCCESS) {
            ERRLOG("Delete server %s failed, %s.", vmInfo.m_uuid.c_str(), m_taskId.c_str());
            m_reportArgs = { vmInfo.m_uuid.c_str() };
            m_reportParam = {
                "virtual_plugin_openstack_delete_new_server_failed_label",
                JobLogLevel::TASK_LOG_WARNING,
                SubJobStatus::RUNNING, 0, 0 };
            return FAILED;
        }
    }
    INFOLOG("Delete server(%s) success, %s", vmInfo.m_uuid.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::DeleteMachineAndVolumes(const VMInfo &vmInfo)
{
    if (DeleteMachine(vmInfo) != SUCCESS) {
        ERRLOG("Delete server %s failed, %s.", vmInfo.m_uuid.c_str(), m_taskId.c_str());
        return FAILED;
    }
    for (const auto &volObj : vmInfo.m_volList) {
        if (DeleteVolume(volObj) != SUCCESS) {
            ERRLOG("Delete volume(%s) failed, %s", volObj.m_uuid.c_str(), m_taskId.c_str());
            continue;
        }
        DBGLOG("Delete volume(%s) success, %s", volObj.m_uuid.c_str(), m_taskId.c_str());
    }
    INFOLOG("Delete server(%s) success, %s", vmInfo.m_uuid.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::WaitForServerDelete(const std::string &serverId)
{
    GetServerDetailsRequest request;
    FormHttpRequest(request);
    request.SetServerId(serverId);
    uint32_t retryCount = MAX_EXEC_COUNT;
    for (uint32_t retryTime = 0; retryTime < retryCount; ++retryTime) {
        std::shared_ptr<GetServerDetailsResponse> resp = m_novaClient.GetServerDetails(request);
        if (resp == nullptr) {
            ERRLOG("Get server (%s) detail failed, nullptr.", serverId.c_str());
            sleep(COMMON_WAIT_TIME);
            continue;
        }
        if (resp->GetStatusCode() == static_cast<uint32_t>(Module::SC_NOT_FOUND)) {
            INFOLOG("Server %s could not be found, delete success.", serverId.c_str());
            return SUCCESS;
        }
        ExpandWaitTime(retryTime, retryCount, resp->GetServerDetails().m_hostServerInfo.m_oSEXTSTStaskState,
            {"deleting"});
        WARNLOG("Wait for server %s delete failed, %d time.", serverId.c_str(), retryTime);
        sleep(COMMON_WAIT_TIME);
    }
    ERRLOG("Wait for server %s delete failed.", serverId.c_str(), m_taskId.c_str());
    return FAILED;
}

int32_t OpenStackProtectEngine::DoDeleteServer(const std::string &serverId)
{
    ServerRequest request;
    FormHttpRequest(request);
    request.SetServerId(serverId);
    for (int32_t retryTime = 1; retryTime < MAX_EXEC_COUNT + 1; ++retryTime) {
        std::shared_ptr<ServerResponse> response = m_novaClient.DeleteServer(request);
        if (response == nullptr) {
            ERRLOG("Delete server failed, nullptr. %s", m_taskId.c_str());
            return FAILED;
        }
        if (response->GetBody().find("itemNotFound") != std::string::npos) {
            INFOLOG("Server %s dont find, no need delete.", serverId.c_str());
            break;
        }
        if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_NO_CONTENT)) {
            ERRLOG("Delete server failed, status(%d). body(%s), %s", response->GetStatusCode(),
                response->GetBody().c_str(), m_taskId.c_str());
            return FAILED;
        }
        if (WaitForServerDelete(serverId) == SUCCESS) {
            INFOLOG("Delete server %s success.", serverId.c_str());
            break;
        }
        WARNLOG("Delete server %s failed, %d time.", serverId.c_str(), retryTime);
        if (retryTime == MAX_EXEC_COUNT) {
            ERRLOG("Delete server %s failed, %s", serverId.c_str(), m_taskId.c_str());
            return FAILED;
        }
        sleep(COMMON_WAIT_TIME);
    }
    return SUCCESS;
}
   
int32_t OpenStackProtectEngine::RenameMachine(const VMInfo &vmInfo, const std::string &newName)
{
    return SUCCESS;
}

int32_t OpenStackProtectEngine::PowerOnMachine(const VMInfo &vmInfo)
{
    int ret = PowerOnMachineHandle(vmInfo);
    m_reportArgs = { vmInfo.m_uuid.c_str() };
    if (ret != SUCCESS) {
        m_reportParam = {
            "virtual_plugin_restore_job_power_on_vm_failed_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0, 0 };
        return ret;
    }
    m_reportParam = {
        "virtual_plugin_restore_job_power_on_vm_success_label",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING, 0, 0 };
    return SUCCESS;
}

int32_t OpenStackProtectEngine::PowerOffMachine(const VMInfo &vmInfo)
{
    bool isPowerOffVmConflict = false;
    int ret = PowerOffMachineHandle(vmInfo, isPowerOffVmConflict);
    m_reportArgs = { vmInfo.m_uuid.c_str() };
    if (ret != SUCCESS) {
        if (isPowerOffVmConflict) {
            m_reportParam = {
            "virtual_plugin_restore_job_power_off_vm_conflicted_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0, 0 };
        } else {
            m_reportParam = {
            "virtual_plugin_restore_job_power_off_vm_failed_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0, 0 };
        }
        return ret;
    }
    m_reportParam = {
        "virtual_plugin_restore_job_power_off_vm_success_label",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING, 0, 0 };
    return SUCCESS;
}

int32_t OpenStackProtectEngine::PowerOnMachineHandle(const VMInfo &vmInfo)
{
    // check if server can power on
    ServerDetail server;
    if (!GetServerDetails(vmInfo, server)) {
        ERRLOG("Get server details failed. %s", m_taskId.c_str());
        return FAILED;
    }
    std::string& status = server.m_hostServerInfo.m_status;
    if (status == SERVER_STATUS_ACTIVE) {
        INFOLOG("Machine already power on.");
        return SUCCESS;
    }
    if (status != SERVER_STATUS_SHUTOFF) {
        ERRLOG("Machine status: %s can not power on.", status.c_str());
        return FAILED;
    }

    // power on
    ServerRequest powerOnServerReq;
    FormHttpRequest(powerOnServerReq);
    powerOnServerReq.SetServerId(vmInfo.m_uuid);
    std::shared_ptr<ServerResponse> powerOnServerRsp = m_novaClient.PowerOnServer(powerOnServerReq);
    if (powerOnServerRsp == nullptr) {
        ERRLOG("Power on machine request failed. %s", m_taskId.c_str());
        return FAILED;
    }
    if (powerOnServerRsp->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Power on machine failed, status(%d). body(%s), %s", powerOnServerRsp->GetStatusCode(),
            powerOnServerRsp->GetBody().c_str(), m_taskId.c_str());
        return FAILED;
    }
    // check if server success active
    if (!GetAndCheckMachineStatus(vmInfo, SERVER_STATUS_ACTIVE, m_commonWaitTimes)) {
        ERRLOG("Power on machine failed. %s", m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Power on machine success.");
    return SUCCESS;
}

int32_t OpenStackProtectEngine::PowerOffMachineHandle(const VMInfo &vmInfo, bool &isPowerOffVmConflict)
{
    // check if server can power off
    ServerDetail server;
    if (!GetServerDetails(vmInfo, server)) {
        ERRLOG("Get server details failed. %s", m_taskId.c_str());
        return FAILED;
    }
    std::string& status = server.m_hostServerInfo.m_status;
    if (status == SERVER_STATUS_SHUTOFF) {
        INFOLOG("Machine already power off.");
        return SUCCESS;
    }
    if ((status != SERVER_STATUS_ACTIVE) && (status != SERVER_STATUS_ERROR)) {
        ERRLOG("machine status: %s can not power off.", status.c_str());
        return FAILED;
    }

    // power off
    ServerRequest powerOffServerReq;
    FormHttpRequest(powerOffServerReq);
    powerOffServerReq.SetServerId(vmInfo.m_uuid);
    std::shared_ptr<ServerResponse> powerOffServerRsp = m_novaClient.PowerOffServer(powerOffServerReq);
    if (powerOffServerRsp == nullptr) {
        ERRLOG("Power off machine request failed. %s", m_taskId.c_str());
        return FAILED;
    }
    if (powerOffServerRsp->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        if (powerOffServerRsp->GetStatusCode() == static_cast<uint32_t>(Module::SC_CONFLICT)) {
            isPowerOffVmConflict = true;
        }
        ERRLOG("Power off machine failed, status(%d). %s", powerOffServerRsp->GetStatusCode(),
            m_taskId.c_str());
        return FAILED;
    }
    // check if server success shutoff
    if (!GetAndCheckMachineStatus(vmInfo, SERVER_STATUS_SHUTOFF, m_commonWaitTimes)) {
        ERRLOG("Power off machine failed. %s", m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Power off machine(%s, %s) success.", vmInfo.m_name.c_str(), vmInfo.m_uuid.c_str());
    return SUCCESS;
}

bool OpenStackProtectEngine::GetAndCheckMachineStatus(const VMInfo &vmInfo, const std::string& status, int waitTimes)
{
    ServerDetail server;
    for (int32_t retryTimes = 0; retryTimes <= waitTimes; retryTimes++) {
        if (!GetServerDetails(vmInfo, server)) {
            ERRLOG("Get machine(%s) id(%s) status(%s) fail, %s", vmInfo.m_name.c_str(), vmInfo.m_uuid.c_str(),
                status.c_str(), m_taskId.c_str());
            return false;
        }
        if (server.m_hostServerInfo.m_status == status) {
            INFOLOG("Check machine(%s) id(%s) status(%s) success, %s", vmInfo.m_name.c_str(), vmInfo.m_uuid.c_str(),
                status.c_str(), m_taskId.c_str());
            return true;
        }
        sleep(m_commonWaitInterval);
    }
    ERRLOG("Check machine(%s) id(%s) status(%s) fail, %s", vmInfo.m_name.c_str(), vmInfo.m_uuid.c_str(),
        status.c_str(), m_taskId.c_str());
    return false;
}

void OpenStackProtectEngine::DiscoverApplications(std::vector<Application>& returnValue, const std::string& appType)
{
    return;
}

void OpenStackProtectEngine::CheckApplication(ActionResult &returnValue, const ApplicationEnvironment &appEnv,
    const AppProtect::Application &application)
{
    if (!ParseCert(appEnv.id, appEnv.auth.extendInfo)) {
        returnValue.__set_bodyErr(CHECK_OPENSTACK_CERT_FAILED);
        returnValue.__set_code(CHECK_OPENSTACK_CERT_FAILED);
        ERRLOG("Parse openstack cert fail.");
        return;
    }
    OpenStackResourceAccess openstackResAccess(appEnv);
    openstackResAccess.SetApplication(application);
    openstackResAccess.SetCertMgr(m_certMgr);
    if (openstackResAccess.CheckAppConnect(returnValue) != SUCCESS) {
        ERRLOG("Failed to check application.");
        return;
    }
    INFOLOG("Success to check application.");
    return;
}
    
void OpenStackProtectEngine::ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application, const ApplicationResource& parentResource)
{
    return;
}

int32_t OpenStackProtectEngine::GetDomains(ResourceResultByPage& page, OpenStackResourceAccess &access)
{
    if (access.GetDomains(page) == FAILED) {
        ERRLOG("Failed to get domain list.");
        return FAILED;
    }
    return SUCCESS;
}

int32_t OpenStackProtectEngine::GetDomainProjects(ResourceResultByPage& page, OpenStackResourceAccess &access,
    const ListResourceRequest& request)
{
    for (const auto &app : request.applications) {
        access.SetApplication(app);
        if (access.GetProjectLists(page) == FAILED) {
            ERRLOG("Failed to get project lists.");
            return FAILED;
        }
    }
    return SUCCESS;
}

int32_t OpenStackProtectEngine::GetProjectServers(ResourceResultByPage& page, OpenStackResourceAccess &access,
    const ListResourceRequest& request)
{
    for (const auto &app : request.applications) {
        access.SetApplication(app);
        if (access.GetProjectServers(page) == FAILED) {
            ERRLOG("Failed to get project servers. name: %s, taskId: %s", app.name.c_str(), m_taskId.c_str());
            continue;
        }
    }
    return SUCCESS;
}

void OpenStackProtectEngine::GetFlavors(ResourceResultByPage& page, OpenStackResourceAccess &access,
    const ListResourceRequest& request)
{
    for (const auto &app : request.applications) {
        access.SetApplication(app);
        if (access.GetFlavors(page) == FAILED) {
            ERRLOG("Failed to get project falvors, name: %s, taskId: %s", app.name.c_str(), m_taskId.c_str());
        }
    }
}

void OpenStackProtectEngine::GetVolumeTypes(ResourceResultByPage& page, OpenStackResourceAccess &access,
    const ListResourceRequest& request)
{
    for (const auto &app : request.applications) {
        access.SetApplication(app);
        if (access.GetVolumeTypes(page) == FAILED) {
            ERRLOG("Failed to get volume types, name: %s, taskId: %s", app.name.c_str(), m_taskId.c_str());
            continue;
        }
    }
}

int32_t OpenStackProtectEngine::GetProjectVolumes(ResourceResultByPage& page, OpenStackResourceAccess &access,
    const ListResourceRequest& request)
{
    for (const auto &app : request.applications) {
        access.SetApplication(app);
        if (access.GetProjectVolumes(page) == FAILED) {
            ERRLOG("Failed to get project volumes. name: %s, taskId: %s", app.name.c_str(), m_taskId.c_str());
            continue;
        }
    }
    return SUCCESS;
}

void OpenStackProtectEngine::GetNetworks(ResourceResultByPage& page, OpenStackResourceAccess &access,
    const ListResourceRequest& request)
{
    for (const auto &app : request.applications) {
        access.SetApplication(app);
        if (access.GetNetworks(page) == FAILED) {
            ERRLOG("Failed to get project networks, name: %s, taskId: %s", app.name.c_str(), m_taskId.c_str());
            continue;
        }
    }
}

void OpenStackProtectEngine::GetAvailabilityZones(ResourceResultByPage& page, OpenStackResourceAccess &access,
    const ListResourceRequest& request)
{
    for (const auto &app : request.applications) {
        access.SetApplication(app);
        if (access.GetAvailabilityZones(page) == FAILED) {
            ERRLOG("Failed to get project availability zones, name: %s, taskId: %s", app.name.c_str(),
                m_taskId.c_str());
        }
    }
}

void OpenStackProtectEngine::ListApplicationResourceV2(ResourceResultByPage& page, const ListResourceRequest& request)
{
    DBGLOG("ListApplicationResourceV2 enter. conditions:%s, %s",
        request.condition.conditions.c_str(), m_taskId.c_str());
    if (!ParseCert(request.appEnv.id, request.appEnv.auth.extendInfo)) {
        ERRLOG("Parse cert fail.");
        return;
    }
    Json::Value conditionJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(request.condition.conditions, conditionJson)) {
        ERRLOG("Failed to read condition, %s", m_taskId.c_str());
        return;
    }
    std::string resourceType = conditionJson["resourceType"].asString();
    OpenStackResourceAccess openstackResAccess(request.appEnv, request.condition);
    openstackResAccess.SetCertMgr(m_certMgr);
    if (resourceType == "domain") {
        GetDomains(page, openstackResAccess);
    } else if (resourceType == "project") {
        GetDomainProjects(page, openstackResAccess, request);
    } else if (resourceType == "server") {
        GetProjectServers(page, openstackResAccess, request);
    } else if (resourceType == "volume") {
        GetProjectVolumes(page, openstackResAccess, request);
    } else if (resourceType == "flavor") {
        GetFlavors(page, openstackResAccess, request);
    } else if (resourceType == "network") {
        GetNetworks(page, openstackResAccess, request);
    } else if (resourceType == "volumeType") {
        GetVolumeTypes(page, openstackResAccess, request);
    } else if (resourceType == "availabilityZone") {
        GetAvailabilityZones(page, openstackResAccess, request);
    } else {
        ERRLOG("Not found the resource type: %s, %s", resourceType.c_str(), m_taskId.c_str());
    }
    return;
}

void OpenStackProtectEngine::DiscoverHostCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv)
{
    return;
}

void OpenStackProtectEngine::DiscoverAppCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    if (!ParseCert(appEnv.id, appEnv.auth.extendInfo)) {
        ERRLOG("Parse cert fail.");
        return;
    }
    OpenStackResourceAccess openstackResAccess(appEnv);
    openstackResAccess.SetCertMgr(m_certMgr);
    openstackResAccess.SetApplication(application);
    openstackResAccess.GetAppCluster(returnEnv);
    return;
}

void OpenStackProtectEngine::FormVolumeInfo(const Json::Value &targetVolume, VolInfo &volObj)
{
    std::string volName = targetVolume["name"].asString();
    if (volName.empty()) {
        uint64_t curTimeStamp =
            std::chrono::system_clock::now().time_since_epoch().count() / std::chrono::system_clock::period::den;
        volName = volObj.m_name + std::to_string(curTimeStamp);
    }
    volObj.m_name = volName;
    volObj.m_volSizeInBytes = targetVolume["size"].asInt();
    volObj.m_volumeType = targetVolume["volumeTypeName"].asString();
}

int OpenStackProtectEngine::GenVolPair(VMInfo &vmObj, const VolInfo &copyVol,
    const ApplicationResource &targetVol, VolMatchPairInfo &volPairs)
{
    Json::Value targetVolume;
    if (InitParaAndGetTargetVolume(targetVol, targetVolume) != SUCCESS) {
        ERRLOG("InitParaAndGetTargetVolume failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!InitRestoreLevel()) {
        INFOLOG("Init RestoreLevel failed.");
        return FAILED;
    }
    std::string targetUuid = targetVolume["id"].asString();
    std::string isNewDisk = targetVolume["isNewDisk"].asString();
    VolInfo dstVolInfo;
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM || isNewDisk == "true") {
        VolInfo backupvolInfo;
        if (LoadCopyVolumeMatadata(copyVol.m_uuid, backupvolInfo) == FAILED) {
            ERRLOG("LoadCopyVolumeMatadata failed.");
            return FAILED;
        }
        FormVolumeInfo(targetVolume, backupvolInfo);
        if (CreateVolume(backupvolInfo, "", "", backupvolInfo.m_datastore, dstVolInfo) == FAILED) {
            ERRLOG("Create volume failed.");
            DeleteVolume(dstVolInfo);
            return FAILED;
        }
        if (backupvolInfo.m_bootable == "true") {
            INFOLOG("System volume need update volume bootable.");
            if (UpdateVolumeBootable(dstVolInfo.m_uuid) != SUCCESS) {
                ERRLOG("Update volume bootable failed.");
                return FAILED;
            }
            dstVolInfo.m_bootable = "true";
        }
    } else {
        if (!GetVolumeInfo(targetUuid, dstVolInfo)) {
            ERRLOG("Get volume(%s) failed, %s", targetUuid.c_str(), m_taskId.c_str());
            return FAILED;
        }
    }
    VolPair restoreVolPair;
    restoreVolPair.m_originVol = copyVol;
    restoreVolPair.m_targetVol = dstVolInfo;
    volPairs.m_volPairList.push_back(restoreVolPair);
    INFOLOG("Copy vol(%s, %s), target vol(%s, %s), %s", copyVol.m_name.c_str(), copyVol.m_uuid.c_str(),
        dstVolInfo.m_name.c_str(), dstVolInfo.m_uuid.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t OpenStackProtectEngine::InitParaAndGetTargetVolume(const ApplicationResource &targetVol,
    Json::Value &targetVolume)
{
    InitRepoHandler();
    Json::Value volExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(targetVol.extendInfo, volExtendInfo)) {
        ERRLOG("JsonStringToJsonValue failed. targetVol's extendInfo, %s", m_taskId.c_str());
        return FAILED;
    }
    std::string targetVolInfo = volExtendInfo["targetVolume"].asString();
    if (!Module::JsonHelper::JsonStringToJsonValue(targetVolInfo, targetVolume)) {
        ERRLOG("Transfer %s failed, %s", targetVolInfo.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Init jobPara and repoHandle success, get targetVolume success");
    return SUCCESS;
}

/**
 * @brief 根据卷当前状态判断是否需要继续延长等待时间
 *
 */
void OpenStackProtectEngine::ExpandWaitTime(uint32_t curRetryTime, uint32_t &totalTimes, const std::string &state,
    std::vector<std::string> intermediateState)
{
    if (curRetryTime != totalTimes || totalTimes > MAX_RETRY_TIMES) {
        return;
    }
    for (const auto &interState: intermediateState) {
        if (interState == state) {
            totalTimes += RETRY_COUNT;
            INFOLOG("State is %s, expand wait time.", interState.c_str());
            return;
        }
    }
}

int32_t OpenStackProtectEngine::DoWaitVolumeStatus(const std::string &volId, const std::string &status,
    std::vector<std::string> intermediateState, uint32_t interval, uint32_t retryCount)
{
    INFOLOG("Retry interval(%d), retry times(%d).", interval, retryCount);
    VolumeRequest request;
    FormHttpRequest(request);
    request.SetVolumeId(volId);
    int32_t retryTime = 0;
    std::shared_ptr<VolumeResponse> response = nullptr;
    while (retryTime < retryCount) {
        response = m_cinderClient->GetVolumeDetail(request);
        if (response != nullptr && response->GetVolume().m_status == status) {
            INFOLOG("Volume(%s) status is %s", volId.c_str(), status.c_str());
            return SUCCESS;
        }
        retryTime++;
        sleep(interval);
        if (response != nullptr) {
            ExpandWaitTime(retryTime, retryCount, response->GetVolume().m_status, intermediateState);
            INFOLOG("Volume(%s) status is %s.", volId.c_str(), response->GetVolume().m_status.c_str());
            if (response->GetVolume().m_status == "error") {
                return FAILED;
            }
        }
    }
    if (response != nullptr) {
        ERRLOG("Volume(%s) status(%s) is not %s", volId.c_str(),
            response->GetVolume().m_status.c_str(), status.c_str());
    }
    return FAILED;
}
 
bool OpenStackProtectEngine::GetifDeleteSnapshot(AppProtect::BackupJobType backuptype, AppProtect::JobResult::type res)
{
    return true;
}

bool OpenStackProtectEngine::IfDeleteLatestSnapShot()
{
    return true;
}

int32_t OpenStackProtectEngine::CheckBeforeMount()
{
    return SUCCESS;
}

int32_t OpenStackProtectEngine::CancelLiveMount(const VMInfo &liveVm)
{
    return SUCCESS;
}

int32_t OpenStackProtectEngine::CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm)
{
    return SUCCESS;
}
OPENSTACK_PLUGIN_NAMESPACE_END
