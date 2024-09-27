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
#include "CinderVolumeHandler.h"
#include <boost/uuid/uuid_io.hpp>
#include "common/Structs.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "protect_engines/openstack/resource_discovery/OpenStackMessage.h"

namespace {
const std::string MODULE_NAME = "CinderVolumeHandler";
const std::string D_FINDDISK = "../";
const int32_t WWN_POS_OFFSET = 7;
const uint32_t GB = 1024 * 1024 * 1024UL;
const std::int32_t SLEEP_TIMES = 30;
constexpr uint32_t RETRY_INTERVAL_SECOND = 5;
const uint32_t MAX_EXEC_COUNT = 5;
const uint8_t BIT_WITH = 8;
const uint8_t DEFAULT_RETRY_INTEVAL = 60;
const uint32_t QUERY_CREATE_VOL_RETRY_INTERVAL = 240;
const uint32_t PER_GB_RETYR_TIMES = 5;
const uint32_t RETRY_COUNT = 10;
const uint32_t MAX_RETRY_TIMES = 20;
const std::string DOMAIN_NAME = "DomainName";
// 	错误场景：执行调用生产端接口操作时，由于调用生产端接口失败，操作失败。
constexpr int64_t INVOKE_API_FAILED_GENERAL_CODE = 1577213460;
}

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

template<typename T>
void CinderVolumeHandler::SetOpenStackRequest(T &request, bool useAdmin)
{
    if (m_domainName.empty()) {
        m_domainName = Module::ConfigReader::getString("OpenStackConfig", DOMAIN_NAME);
        INFOLOG("DomainName %s.", m_domainName.c_str());
    }
    AuthObj auth;
    if (useAdmin) {
        auth.name = m_appEnv.auth.authkey;
        auth.passwd = m_appEnv.auth.authPwd;
    } else {
        auth.name = m_application.auth.authkey;
        auth.passwd = m_application.auth.authPwd;
        CloudServerResource serverExtendInfo;
        Module::JsonHelper::JsonStringToStruct(m_application.extendInfo, serverExtendInfo);
        request.SetScopeValue(m_application.parentId);
        request.SetDomain(serverExtendInfo.m_domainName);
    }
    request.SetUserInfo(auth);
    request.SetEnvAddress(m_appEnv.endpoint);
    return;
}

int32_t CinderVolumeHandler::DoAttachVolumeToHost(const std::string &volumeId)
{
    INFOLOG("Begin to attach volume(%s) to server(%s).", volumeId.c_str(), m_serverId.c_str());
    std::lock_guard<std::mutex> lock(m_attachVolumeMutex);
    AttachServerVolumeRequest request;
    SetOpenStackRequest(request, true);
    request.SetVolumeId(volumeId);
    request.SetServerId(m_serverId);
    std::shared_ptr<AttachServerVolumeResponse> response = m_novaClient.AttachServerVolume(request);
    if (response == nullptr) {
        ERRLOG("Attach Volume failed.");
        return FAILED;
    }

    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Failed to attach volume(%s), status code:%d.", volumeId.c_str(), response->GetStatusCode());
        return FAILED;
    }
    std::vector<std::string> intermediateState = {"reserved", "attaching"};
    if (DoWaitVolumeStatus(volumeId, OPENSTACK_VOLUME_IN_USE_STATUS, intermediateState) != SUCCESS) {
        ERRLOG("Wait volume(%s) status in-use timeout.", volumeId.c_str());
        return FAILED;
    }
    INFOLOG("Attach Volume(%s) to Host success.", volumeId.c_str());
    return SUCCESS;
}

bool CinderVolumeHandler::FormCreateVolmeInfo(const VolSnapInfo &snapshot, std::string &body)
{
    Json::Value jsonBody;
    jsonBody["size"] = m_volSizeInBytes / GB;
    jsonBody["snapshot_id"] = snapshot.m_snapshotId;

    SnapShotExtenInfo extendInfo;
    if (!Module::JsonHelper::JsonStringToStruct(snapshot.m_extendInfo, extendInfo)) {
        ERRLOG("Failed to trans data from json string to snapshot extendInfo.");
        return FAILED;
    }
    Json::Value metadataBody;
    std::string flag = Module::ConfigReader::getString("OpenStackConfig", "SupportCloneVolume");
    if (flag == "true") {
        metadataBody["full_clone"] = "0"; // 表示创建链接克隆卷
    }
    jsonBody["volume_type"] = extendInfo.m_volumeType;
    jsonBody["consistencygroup_id"] =
        extendInfo.m_groupSnapShotId.empty() ? Json::Value::null : extendInfo.m_groupSnapShotId;
    jsonBody["metadata"] = metadataBody;
    jsonBody["name"] = GenNewVolumeName(snapshot.m_snapshotId);
    jsonBody["description"] = GetNewVolumeDesciption();

    Json::Value volume;
    volume["volume"] = jsonBody;
    Json::FastWriter fastWriter;
    body = fastWriter.write(volume);
    return SUCCESS;
}

int32_t CinderVolumeHandler::DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot)
{
    VolumeRequest request;
    SetOpenStackRequest(request);

    std::string body;
    if (FormCreateVolmeInfo(snapshot, body) != SUCCESS) {
        ERRLOG("Form create volumes info failed.");
        return FAILED;
    }
    request.SetBody(body);

    std::shared_ptr<VolumeResponse> response = m_cinderClient.CreateVolume(request);
    if (response == nullptr) {
        ERRLOG("Failed to create volume, respnose point is nullptr.");
        return FAILED;
    }

    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Failed to create volume, status: %d, recive body is %s", response->GetStatusCode(),
            response->GetBody().c_str());
        return FAILED;
    }
    m_newCreateVolInfo = response->GetVolume();
    m_newCreateVolId = m_newCreateVolInfo.m_id;
    INFOLOG("New volume(%s) created from snap(%s).", m_newCreateVolInfo.m_id.c_str(), snapshot.m_snapshotId.c_str());
    UpdateNewVolumeInfoToFile(m_newCreateVolInfo);  // 更新记录中卷的信息
    uint32_t interval = Module::ConfigReader::getInt("OpenStackConfig", "CreateVolumeWaitInterval");
    uint32_t retryTimes = Module::ConfigReader::getInt("OpenStackConfig", "CreateVolumeWaitRetryTimes");
    interval == 0 ? interval = QUERY_CREATE_VOL_RETRY_INTERVAL : interval;
    retryTimes == 0 ? retryTimes = (m_volSizeInBytes / GB) * PER_GB_RETYR_TIMES  : retryTimes;
    std::vector<std::string> interState = {};
    int ret = DoWaitVolumeStatus(m_newCreateVolInfo.m_id, OPENSTAK_VOLUME_AVAILABLE_STATUS, interState,
        interval, retryTimes);
    if (ret != SUCCESS) {
        ERRLOG("Volume Status is not available.");
        m_reportArgs = { snapshot.m_snapshotId, m_newCreateVolInfo.m_id};
        m_reportPara = {
            "virtual_plugin_cinder_volume_create_volume_failed_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0, 0, INVOKE_API_FAILED_GENERAL_CODE};
        return FAILED;
    }
    return SUCCESS;
}

int32_t CinderVolumeHandler::DoWaitVolumeStatus(const std::string &volId, const std::string &status,
    std::vector<std::string> intermediateState, uint32_t interval, uint32_t retryCount)
{
    INFOLOG("Retry interval(%d), retry times(%d).", interval, retryCount);
    VolumeRequest request;
    SetOpenStackRequest(request);
    request.SetVolumeId(volId);
    int32_t retryTime = 0;
    std::shared_ptr<VolumeResponse> response = nullptr;
    while (retryTime < retryCount) {
        response = m_cinderClient.GetVolumeDetail(request);
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

int32_t CinderVolumeHandler::DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId)
{
    DBGLOG("Begin Detach Volume(%s)", detachVolId.c_str());
    DetachVolumeRequest request;
    SetOpenStackRequest(request, true);
    request.SetServerId(serverId);
    std::shared_ptr<ServerResponse> response = m_novaClient.DetachServerVolume(request, detachVolId);
    if (response == nullptr) {
        ERRLOG("Detach Volume(%s) failed.", detachVolId.c_str());
        return FAILED;
    }

    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_ACCEPTED)) {
        ERRLOG("Failed to detach volume, status code:%d, resp body : %s", response->GetStatusCode(),
            response->GetBody().c_str());
        return FAILED;
    }

    if (DoWaitVolumeStatus(detachVolId, OPENSTAK_VOLUME_AVAILABLE_STATUS) != SUCCESS) {
        ERRLOG("Wait volume(%s) available timeout.", detachVolId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool CinderVolumeHandler::GetVolumeDetail(const std::string &volId, Volume &volDetail)
{
    VolumeRequest request;
    SetOpenStackRequest(request);
    request.SetVolumeId(volId);
    std::shared_ptr<VolumeResponse> response = m_cinderClient.GetVolumeDetail(request);
    if (response == nullptr) {
        ERRLOG("Show volume(%s) detail failed, %s", volId.c_str(), m_taskId.c_str());
        return false;
    }
    volDetail = response->GetVolume();
    DBGLOG("Get volume(%s) detail success, %s", volId.c_str(), m_taskId.c_str());
    return true;
}

bool CinderVolumeHandler::WaitVolumeStatus(const Volume &volDetail, std::string &status)
{
    status = OPENSTAK_VOLUME_AVAILABLE_STATUS;
    if (volDetail.m_status == OPENSTACK_VOLUME_DETACHING_STATUS) {
        if (DoWaitVolumeStatus(m_newCreateVolInfo.m_id, OPENSTAK_VOLUME_AVAILABLE_STATUS) != SUCCESS) {
            ERRLOG("Wait detaching volume(%s) available timeout", m_newCreateVolInfo.m_id.c_str());
            return false;
        }
        return true;
    }
    if (volDetail.m_attachPoints[0].m_serverId != m_serverId) {
        if (DetachVolumeHandle(m_newCreateVolInfo.m_id, volDetail.m_attachPoints[0].m_serverId) != SUCCESS) {
            ERRLOG("Volume has been attach other host,  Detach volume(%s) from other host(%s) failed.",
                m_newCreateVolInfo.m_id.c_str(), volDetail.m_attachPoints[0].m_serverId.c_str());
            return false;
        }
    } else {
        if (DoWaitVolumeStatus(m_newCreateVolInfo.m_id, OPENSTACK_VOLUME_IN_USE_STATUS) != SUCCESS) {
            ERRLOG("Wait volume(%s) status in-use timeout.", m_newCreateVolInfo.m_id.c_str());
            return false;
        }
        status = OPENSTACK_VOLUME_IN_USE_STATUS;
    }
    return true;
}

bool CinderVolumeHandler::CheckAndAttachVolume()
{
    Volume volDetail;
    if (!GetVolumeDetail(m_newCreateVolId, volDetail)) {
        ERRLOG("Get volume(%s) detail failed.", m_newCreateVolId.c_str());
        return false;
    }

    INFOLOG("Volume(%s) status is %s.", m_newCreateVolId.c_str(), volDetail.m_status.c_str());
    if (!volDetail.m_attachPoints.empty()) {
        std::string status;
        if (!WaitVolumeStatus(volDetail, status)) {
            ERRLOG("Wait volume(%s) status ready failed.", m_newCreateVolId.c_str());
            return false;
        }
        if (status == m_volumeInUseStatus) {
            return true;
        }
    }
    uint32_t interval = Module::ConfigReader::getInt("OpenStackConfig", "CreateVolumeWaitInterval");
    uint32_t retryTimes = Module::ConfigReader::getInt("OpenStackConfig", "CreateVolumeWaitRetryTimes");
    interval == 0 ? interval = DEFAULT_RETRY_INTEVAL : interval;
    retryTimes == 0 ? retryTimes = (m_volSizeInBytes / GB) * PER_GB_RETYR_TIMES  : retryTimes;
    std::vector<std::string> interState = {};
    if (DoWaitVolumeStatus(m_newCreateVolId, m_voluemAvailableStatus, interState,
        interval, retryTimes) != SUCCESS) {
        ERRLOG("Wait volume(%s) available timeout.", m_newCreateVolId.c_str());
        return false;
    }
    for (int i = 0; i < RETRY_COUNT; i++) {
        if (DoAttachVolumeToHost(m_newCreateVolId) == SUCCESS) {
            INFOLOG("Attach Volume %s to host success, %s.", m_newCreateVolId.c_str(), m_taskId.c_str());
            return true;
        }
        WARNLOG("Attach Volume(%s) to host failed. serverId is (%s)", m_newCreateVolId.c_str(),
            m_serverId.c_str());
        sleep(SLEEP_TIMES);
    }
    m_reportArgs = { m_newCreateVolId, m_serverId};
    m_reportPara = {
                "virtual_plugin_cinder_volume_attach_volume_failed_label",
                JobLogLevel::TASK_LOG_ERROR,
                SubJobStatus::FAILED, 0, 0 };
    return false;
}

std::string CinderVolumeHandler::GetProxyHostId()
{
    return Utils::GetProxyHostId(true);
}

bool CinderVolumeHandler::CheckAndDetachVolume(const std::string &volId)
{
    VolumeRequest request;
    SetOpenStackRequest(request);
    request.SetVolumeId(volId);
    std::shared_ptr<VolumeResponse> response = m_cinderClient.GetVolumeDetail(request);
    if (response == nullptr) {
        ERRLOG("Get Volume(%s) info failed.", volId.c_str());
        return false;
    }
    if (!response->Serial()) {
        ERRLOG("Serial Volume(%s) info failed, body(%s).", volId.c_str(), response->GetBody().c_str());
        return false;
    }
    if (response->GetVolume().m_attachPoints.empty()) {
        INFOLOG("Volume(%s) is available.", volId.c_str());
        return true;
    }
    std::string curAttachServer = response->GetVolume().m_attachPoints[0].m_serverId;
    if (DetachVolumeHandle(volId, curAttachServer) != SUCCESS) {
        ERRLOG("Detach volume(%s) from host(%s) failed.", volId.c_str(), curAttachServer.c_str());
        return false;
    }
    INFOLOG("Detach volume(%s) from host(%s) success.", volId.c_str(), curAttachServer.c_str());
    return true;
}

bool CinderVolumeHandler::UpdateNewVolumeInfoToFile(Volume &newCreateVolume)
{
    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (m_volumeInfoRepoHandler->Exists(m_snapshotCreateVolumeFile)) {
        if (Utils::LoadFileToStructWithRetry(m_volumeInfoRepoHandler, m_snapshotCreateVolumeFile,
            m_newCreatedVolumeList) != SUCCESS) {
            ERRLOG("Load create volume list info failed.");
            return false;
        }

        std::vector<Volume>::iterator it;
        for (it = m_newCreatedVolumeList.m_volumelist.begin();
            it != m_newCreatedVolumeList.m_volumelist.end(); it++) {
            if ((*it).m_snapshotId == newCreateVolume.m_snapshotId && (*it).m_id == newCreateVolume.m_id) {
                m_newCreatedVolumeList.m_volumelist.erase(it);
                break;
            }
        }
    }
    m_newCreatedVolumeList.m_volumelist.push_back(newCreateVolume);

    std::string newCreatedVolumeListStr;
    if (!Module::JsonHelper::StructToJsonString(m_newCreatedVolumeList, newCreatedVolumeListStr)) {
        ERRLOG("Convert created volume list to json string failed.");
        return false;
    }

    if (Utils::SaveToFileWithRetry(m_volumeInfoRepoHandler, m_snapshotCreateVolumeFile,
        newCreatedVolumeListStr) != SUCCESS) {
        ERRLOG("Save created volume list failed.");
        return false;
    }
    return true;
}
OPENSTACK_PLUGIN_NAMESPACE_END