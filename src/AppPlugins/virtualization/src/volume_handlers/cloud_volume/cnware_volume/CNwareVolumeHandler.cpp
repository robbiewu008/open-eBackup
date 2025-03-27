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
#include "CNwareVolumeHandler.h"
#include <boost/uuid/uuid_io.hpp>
#include "common/Structs.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "curl_http/HttpStatus.h"

using namespace CNwarePlugin::CNwareErrorCode;

namespace {
const std::string MODULE_NAME = "CNwareVolumeHandler";
const int32_t COMMON_WAIT_TIME = 10;
const int32_t COMMON_WAIT_TIME_3S = 3;
const int32_t SCAN_DISK_RETRY_TIMES = 3;
const int32_t CNWARE_ATTACH_VOLUME_BUS_VIRTIO = 1;
const int32_t CNWARE_ATTACH_VOLUME_EXIST_SOURCE = 2;
const int32_t FIFTEEN_MIN_IN_SEC = 900;
const int RETRY_TIMES = 3;
std::mutex m_fileMutex;
}

namespace CNwarePlugin {
uint64_t g_attachLimit = -1;
uint64_t g_versionAttachLimit = -1;
std::mutex g_attachCntMutex;
std::condition_variable g_attachCntCv;
std::mutex g_attachVolumeMutex;

std::string CNwareVolumeHandler::GetProxyHostId()
{
    return Utils::GetProxyHostId(true);
}

int32_t CNwareVolumeHandler::DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot)
{
    return SUCCESS;
}

int32_t CNwareVolumeHandler::DoWaitVolumeStatus(const std::string &volId, const std::string &status,
    std::vector<std::string> intermediateState, uint32_t interval, uint32_t retryCount)
{
    return SUCCESS;
}

int32_t CNwareVolumeHandler::DoCreateVolumeFromSnapShotId(const VolSnapInfo &snapshot)
{
    return SUCCESS;
}

bool CNwareVolumeHandler::CheckAndDetachVolume(const std::string &volId)
{
    return true;
}

bool CNwareVolumeHandler::InitClient()
{
    if (m_cnwareClient == nullptr) {
        m_cnwareClient = std::make_shared<CNwareClient>(m_appEnv.auth);
    }
    m_certMgr = std::make_shared<CertManger>();
    if (m_certMgr == nullptr) {
        ERRLOG("CertMgr is nullptr, %s", m_jobId.c_str());
        return false;
    }
    CertInfo cert;
    if (!m_certMgr->ParseCert(m_appEnv.endpoint, m_appEnv.auth.extendInfo, cert)) {
        ERRLOG("Parse Cert failed! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    if (m_cnwareClient->InitCNwareClient(m_appEnv) != SUCCESS) {
        ERRLOG("Build login body failed! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    int64_t errorCode;
    std::string errorDes;
    RequestInfo requestInfo;
    if (m_cnwareClient->GetSessionAndlogin(req, requestInfo, errorCode, errorDes) != SUCCESS) {
        ERRLOG("Login client failed! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    return true;
}

bool CNwareVolumeHandler::LoadCreatedSnapshotVolumeInfo(const BackupSubJobInfo &subJob,
    SnapshotDiskInfo &newCreateVolume)
{
    SnapshotDiskInfo tmp;
    if (!Module::JsonHelper::JsonStringToStruct(subJob.m_curSnapshotInfo.m_extendInfo, tmp)) {
        ERRLOG("Load create snapshot volume info failed, %s", m_taskId.c_str());
        return false;
    }
    newCreateVolume = tmp;
    return true;
}

void CNwareVolumeHandler::SetCommonInfo(CNwareRequest& req)
{
    AuthObj authObj;
    authObj.name = m_appEnv.auth.authkey;
    authObj.passwd = m_appEnv.auth.authPwd;
    authObj.certVerifyEnable = m_certMgr->IsVerifyCert();
    authObj.cert = m_certMgr->GetCertPath();
    authObj.revocationList = m_certMgr->GetRevocationListPath();
    req.SetEndpoint(m_appEnv.endpoint);
    req.SetEnvAddress(m_appEnv.endpoint);
    req.SetIpPort(std::to_string(m_appEnv.port));
    req.SetUserInfo(authObj);
    return;
}

bool CNwareVolumeHandler::GetVolInfoOnStorage(const std::string &volId, CNwareDiskInfo &volumeDetail)
{
    GetDiskInfoOnStorageRequest req;
    SetCommonInfo(req);
    req.SetVolId(volId);
    std::shared_ptr<GetDiskInfoOnStorageResponse> response = m_cnwareClient->GetDiskInfoOnStorage(req);
    if (response == nullptr) {
        ERRLOG("Get disk info on storage failed, %s", m_taskId.c_str());
        return false;
    }
    volumeDetail = response->GetInfo().m_data;
    return true;
}

template<typename T>
bool CNwareVolumeHandler::CommonCheckResponse(const T &response)
{
    if (response == nullptr) {
        ERRLOG("%s response is nullptr, %s", m_taskId.c_str());
        return false;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Response is not ok. http status code: %d, body : %s, %s", response->GetStatusCode(),
               (response->GetBody()).c_str(), m_taskId.c_str());
        return true;
    }
}

bool CNwareVolumeHandler::CheckTaskStatus(const std::string taskId)
{
    QueryTaskRequest req;
    req.SetTaskId(taskId);
    SetCommonInfo(req);
    
    int32_t taskStatus = static_cast<int>(CNwareTaskStatus::CNWARE_TASK_STARTED);
    std::shared_ptr<QueryTaskResponse> response;
    uint32_t times = 0;
    while (times * COMMON_WAIT_TIME_3S < FIFTEEN_MIN_IN_SEC) {
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
               taskStatus, response->GetTaskStepDesc().c_str(), m_taskId.c_str());
        return false;
    }
    return true;
}

int32_t CNwareVolumeHandler::DoAttachVolumeToHostInner(const std::string &volId)
{
    AddDiskRequest req;
    SetCommonInfo(req);
    req.SetDomainId(m_serverId);
    AddDomainDiskDevicesReq reqDisk;
    reqDisk.oldPool = m_poolName;
    reqDisk.oldVol = m_volName;
    reqDisk.source = CNWARE_ATTACH_VOLUME_EXIST_SOURCE;
    reqDisk.preallocation = m_preallocation;
    // 备份&恢复都使用virtio临时挂载执行任务，RestoreVolMeatadata接口会更新bus类型
    reqDisk.bus = CNWARE_ATTACH_VOLUME_BUS_VIRTIO;
    if (!m_isBackup) {
        DomainDiskDevicesResp diskInfo;
        if (!Module::JsonHelper::JsonStringToStruct(m_volInfo.m_metadata, diskInfo)) {
            ERRLOG("Get new domain info failed.");
            return FAILED;
        }
        reqDisk.cache = diskInfo.m_cache;
        reqDisk.ioHangTimeout = diskInfo.m_ioHangTimeout;
        reqDisk.shareable = diskInfo.m_shareable;
        reqDisk.type = diskInfo.m_driverType;
    }
    req.SetDomainDiskDevices(reqDisk);

    std::shared_ptr<AddDiskResponse> response = m_cnwareClient->AddDisk(req);
    if (response == nullptr) {
        ERRLOG("Get disk info on storage failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Attach volume %s on storage failed, %s", volId.c_str(), m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareVolumeHandler::AttachDiskAndGetDev(const std::string &volId)
{
    std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
    std::vector<std::string> beforeDevList {};
    if (ListDev(beforeDevList) != SUCCESS) {
        ERRLOG("Get VolumeDev failed before.");
        return FAILED;
    }
    if (DoAttachVolumeToHostInner(volId) != SUCCESS) {
        if (GetAttachLimit() > 0) {
            DBGLOG("Attach number check: notify. %s", m_jobId.c_str());
            g_attachCntCv.notify_one();
        }
        ApplicationLabelType labelParam;
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNWARE_ATTACH_VOLUME_FAILED_LABEL;
        labelParam.params = std::vector<std::string>{GetHostName(), m_volName};
        labelParam.errCode = CNWARE_ATTACH_VOLUME_FAILED_ERROR;
        labelParam.additionalDesc = std::vector<std::string>{ "Attach volume failed." };
        ReportJobDetail(labelParam);
        return FAILED;
    } else {
        if (!GetAttachedVolInfo(volId, beforeDevList) != SUCCESS) {
            ERRLOG("Scan disk failed.");
            return FAILED;
        }
    }
    return SUCCESS;
}


int32_t CNwareVolumeHandler::DoAttachVolumeToHost(const std::string &volId)
{
    INFOLOG("Wait to attach volume ...");
    int32_t ret = FAILED;
    {
        std::unique_lock<std::mutex> lock(g_attachCntMutex);
        if (GetAttachLimit() == 0) {
            g_attachCntCv.wait(lock, [] {return (g_attachLimit > 0);});
            DBGLOG("Attach number check: can attach %ld disk. %s", g_attachLimit, m_jobId.c_str());
        }
        ApplicationLabelType labelParam;
        labelParam.label = CNWARE_ATTACH_VOLUME_LABEL;
        labelParam.params = std::vector<std::string>{GetHostName(), m_volName};
        ReportJobDetail(labelParam);
        if (AttachDiskAndGetDev(volId) != SUCCESS) {
            ERRLOG("AttachDiskAndGetDev disk(%s) failed. %s", volId.c_str(), m_jobId.c_str());
            return FAILED;
        }
    }
    return SUCCESS;
}

bool CNwareVolumeHandler::CheckAndAttachVolume()
{
    std::string volId;
    if (m_isBackup) {
        volId = m_newCreateVolInfo.m_volId;
        m_poolName = m_newCreateVolInfo.m_storagePoolName;
        m_volName = m_newCreateVolInfo.m_volName;
    } else {
        volId = m_volInfo.m_uuid;
        m_poolName = m_volInfo.m_datastore.m_name;
        m_volName = m_volInfo.m_name;
        DomainDiskDevicesResp diskDevice;
        if (!Module::JsonHelper::JsonStringToStruct(m_volInfo.m_metadata, diskDevice)) {
            ERRLOG("Volume metadata trans to diskDevice failed, failed to get disk preallocation.");
            return false;
        }
        m_preallocation = diskDevice.m_preallocation;
        DBGLOG("Disk preallocation: %s", m_preallocation.c_str());
    }

    if (!CheckVolIsAttached(volId)) {
        ERRLOG("Vol(%s) is attached or detach it failed. %s", volId.c_str(), m_taskId.c_str());
        return false;
    }

    if (DoAttachVolumeToHost(volId) == SUCCESS) {
        INFOLOG("Attach Volume %s to host success, %s.", volId.c_str(), m_taskId.c_str());
        return true;
    }
    m_reportArgs = { volId, GetHostName() };
    m_reportPara = {
                "virtual_plugin_cinder_volume_attach_volume_failed_label", // 标签待修改
                JobLogLevel::TASK_LOG_ERROR,
                SubJobStatus::FAILED, 0, 0 };
    return false;
}

bool CNwareVolumeHandler::CheckVolIsAttached(const std::string &volId)
{
    CNwareDiskInfo volDetail;
    if (!GetVolInfoOnStorage(volId, volDetail)) {
        ERRLOG("Get volume(%s) detail failed.", volId.c_str());
        return false;
    }

    if (volDetail.m_id == "") {
        ERRLOG("Snapshot volume(%s) does not exist.", volId.c_str());
    }

    if (volDetail.m_status != static_cast<int>(CNwareDiskStatus::NORMAL)) {
        ERRLOG("Snapshot volume(%s) status is abnormal.", volId.c_str());
    }
    INFOLOG("Volume(%s) status is %d.", volId.c_str(), volDetail.m_status);
    bool iRet {true};
    if (!volDetail.m_userList.empty()) {
        for (const auto &domain : volDetail.m_userList) {
            WARNLOG("Vol(%s) has been attached on Vm(%s). %s", volId.c_str(),
                domain.m_domainId.c_str(), m_taskId.c_str());
            iRet = (DetachVolumeHandle(volId, domain.m_domainId) == SUCCESS) ? iRet : false;
        }
    }
    return iRet;
}

bool CNwareVolumeHandler::GetNewAttachDev(const std::vector<std::string> &beforeDevList)
{
    std::vector<std::string> afterDevList {};
    int retryTimes = 0;
    int countDev = 0;
    std::string newDev;
    while (retryTimes < RETRY_TIMES) {
        afterDevList.clear();
        countDev = 0;
        if (ListDev(afterDevList) != SUCCESS) {
            ERRLOG("ListDev failed, %s", m_taskId.c_str());
            return false;
        }
        for (const auto &dev : afterDevList) {
            if (std::find(beforeDevList.begin(), beforeDevList.end(), dev) != beforeDevList.end()) {
                DBGLOG("Find dev(%s) mounted before.", dev.c_str());
                continue;
            }
            newDev = dev;
            countDev++;
        }
        if (countDev == 1) {
            m_diskDevicePath = "/dev/" + newDev;
            INFOLOG("Find dev(%s) just mounted. %s", newDev.c_str(), m_taskId.c_str());
            return true;
        } else {
            ERRLOG("Dev mount num(%d) is err! %s", countDev, m_taskId.c_str());
            ++retryTimes;
            sleep(COMMON_WAIT_TIME);
        }
    }
    ERRLOG("GetNewAttachDev failed. %s", m_taskId.c_str());
    return false;
}

bool CNwareVolumeHandler::GetBusDevByVolId(std::string &bus, std::string &dev,
    const std::string &volId, const std::string &serverId)
{
    INFOLOG("Enter");
    GetVMDiskInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(serverId);
    std::shared_ptr<GetVMDiskInfoResponse> response = m_cnwareClient->GetVMDiskInfo(req);
    if (response == nullptr) {
        ERRLOG("Get list of vols on agent failed, %s", m_taskId.c_str());
        return false;
    }
    for (const auto &disk : response->GetInfo().m_diskDevices) {
        if (disk.m_volId != volId) {
            continue;
        } else {
            bus = disk.m_bus;
            dev = disk.m_dev;
            return true;
        }
    }
    return false;
}

bool CNwareVolumeHandler::GetAttachedVolInfo(const std::string &volId,
    const std::vector<std::string> &beforeDevList)
{
    INFOLOG("Enter");
    if (!GetBusDevByVolId(m_bus, m_dev, volId, m_serverId)) {
        ERRLOG("Get bus dev of vol(%s) on Vm(%s) failed, %s", volId.c_str(), m_serverId.c_str(),
            m_taskId.c_str());
        return false;
    }

    if (!GetNewAttachDev(beforeDevList)) {
        ERRLOG("Get NewAttach Dev failed, %s", m_taskId.c_str());
        return false;
    }
    INFOLOG("Get device path is %s.", m_diskDevicePath.c_str());
    int retryTimes = 0;
    while (retryTimes < SCAN_DISK_RETRY_TIMES) {
        if (boost::filesystem::exists(m_diskDevicePath)) {
            INFOLOG("Device path %s exists.", m_diskDevicePath.c_str());
            return true;
        } else {
            retryTimes++;
            WARNLOG("Attached vol(%s) path(%s) is not ready, retry %d times",
                    volId.c_str(), m_diskDevicePath.c_str(), retryTimes);
            sleep(COMMON_WAIT_TIME);
        }
    }

    ERRLOG("Find the attached vol %s on agent failed, %s", volId.c_str(), m_taskId.c_str());
    return false;
}

int32_t CNwareVolumeHandler::Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo)
{
    if (!LoadCreatedSnapshotVolumeInfo(jobInfo, m_newCreateVolInfo)) {
        ERRLOG("Load snapshot info failed.");
        return FAILED;
    }

    if (!CheckAndAttachVolume()) {
        return FAILED;
    }

    if (!ChangeFilePriviledge(m_diskDevicePath, mode)) {
        return FAILED;
    }
    int32_t owMode = (mode == VolOpenMode::READ_WRITE) ? O_WRONLY : O_RDONLY;
    DISK_DEVICE_RETURN_CODE deviceCode = m_spDeviceFile->Open(m_diskDevicePath, owMode, m_volSizeInBytes);
    if (deviceCode != DISK_DEVICE_OK) {
        ERRLOG("Open disk device file[%s] failed.", m_diskDevicePath.c_str());
        return FAILED;
    }

    INFOLOG("Start to open disk for backup success.");
    return SUCCESS;
}

int32_t CNwareVolumeHandler::Open(const VolOpenMode &mode)
{
    LOGGUARD("Enter");
    if (!CheckAndAttachVolume()) {
        return FAILED;
    }

    if (!ChangeFilePriviledge(m_diskDevicePath, mode)) {
        return FAILED;
    }
    int32_t owMode = (mode == VolOpenMode::READ_WRITE) ? O_WRONLY : O_RDONLY;
    DISK_DEVICE_RETURN_CODE deviceCode = m_spDeviceFile->Open(m_diskDevicePath, owMode, m_volSizeInBytes);
    if (deviceCode != DISK_DEVICE_OK) {
        ERRLOG("Open disk device file[%s] failed.", m_diskDevicePath.c_str());
        return FAILED;
    }

    INFOLOG("Start to open disk for backup success.");
    return SUCCESS;
}

bool CNwareVolumeHandler::GetVolBusDev(std::string &busDevStr, const std::string &detachVolId,
    const std::string &serverId)
{
    if (m_bus == "" || m_dev == "") {
        std::string bus;
        std::string dev;
        if (!GetBusDevByVolId(bus, dev, detachVolId, serverId)) {
            ERRLOG("Get list of vols on agent failed, %s", m_taskId.c_str());
            return false;
        }
        busDevStr = bus + "-" + dev;
    } else {
        busDevStr = m_bus + "-" + m_dev;
    }
    DBGLOG("Get vol(%s) bus dev(%s) on Vm(%s). %s", detachVolId.c_str(), busDevStr.c_str(), serverId.c_str(),
        m_taskId.c_str());
    return true;
}

bool CNwareVolumeHandler::CheckBusDevStr(const std::string &BusDevStr, const std::string &serverId)
{
    GetVMDiskInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(serverId);
    std::shared_ptr<GetVMDiskInfoResponse> response = m_cnwareClient->GetVMDiskInfo(req);
    if (response == nullptr) {
        ERRLOG("Get list of vols on agent failed, %s", m_taskId.c_str());
        return false;
    }
    for (const auto &disk : response->GetInfo().m_diskDevices) {
        if ((disk.m_bus + "-" + disk.m_dev) == BusDevStr) {
            WARNLOG("Find dev(%s) still mounted, volId(%s)", BusDevStr.c_str(), disk.m_volId.c_str());
            return false;
        }
    }
    return true;
}

int32_t CNwareVolumeHandler::DoDetachVolumeFromeHostInner(const std::string &detachVolId, const std::string &serverId)
{
    std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
    DBGLOG("Begin Detach Volume(%s)", detachVolId.c_str());

    std::string busDevStr;
    if (!GetVolBusDev(busDevStr, detachVolId, serverId)) {
        ERRLOG("Get disk(%s) bus-dev string on vm(%s) failed, %s", detachVolId.c_str(), serverId.c_str(),
            m_taskId.c_str());
        return FAILED;
    }
    
    DetachDiskOnVMRequest req;
 
    req.SetParam(serverId, busDevStr);
    SetCommonInfo(req);
    std::vector<std::string> beforeDevList {};
    if (ListDev(beforeDevList) != SUCCESS) {
        ERRLOG("ListDev on vm %s failed, %s", serverId.c_str(), m_taskId.c_str());
        return FAILED;
    }
    int retryNum {0};
    std::shared_ptr<CNwareResponse> response = std::make_shared<CNwareResponse>();
    while (retryNum < RETRY_TIMES) {
        response = m_cnwareClient->DetachDiskOnVM(req);
        if (response == nullptr || !CheckTaskStatus(response->GetTaskId())) {
            ERRLOG("Detach disk %s on vm %s failed, %s", detachVolId.c_str(), serverId.c_str(), m_taskId.c_str());
            if (!CheckBusDevStr(busDevStr, serverId)) {
                ++retryNum;
                sleep(COMMON_WAIT_TIME);
                continue;
            }
            break;
        }
        break;
    }
    return CheckDetachDevList(beforeDevList);
}

int32_t CNwareVolumeHandler::CheckDetachDevList(const std::vector<std::string> &beforeDevList)
{
    std::vector<std::string> afterDevList;
    int countDev = 0;
    int retryTimes = 0;
    std::string detachDev;
    while (retryTimes < RETRY_TIMES) {
        afterDevList.clear();
        countDev = 0;
        if (ListDev(afterDevList) != SUCCESS) {
            ERRLOG("ListDev afterfailed, %s", m_taskId.c_str());
            return FAILED;
        }
        for (const std::string &dev : beforeDevList) {
            if (std::find(afterDevList.begin(), afterDevList.end(), dev) != afterDevList.end()) {
                DBGLOG("Find dev(%s) mounted before.", dev.c_str());
                continue;
            }
            detachDev = dev;
            countDev++;
        }
        if (countDev == 1) {
            INFOLOG("Check vol dev(%s) is detached.", detachDev.c_str());
            return SUCCESS;
        } else {
            retryTimes++;
            WARNLOG("Detach vol is not ready, retry %d times", retryTimes);
            sleep(COMMON_WAIT_TIME);
        }
    }
    ERRLOG("CheckDetachDevList failed. %s", m_taskId.c_str());
    return FAILED;
}

int32_t CNwareVolumeHandler::DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId)
{
    int32_t ret = FAILED;
    if (DoDetachVolumeFromeHostInner(detachVolId, serverId) != SUCCESS) {
        return FAILED;
    }
 
    std::unique_lock<std::mutex> lock(g_attachCntMutex);
    if (GetAttachLimit() > 0) {
        DBGLOG("Attach number check: notify. %s", m_jobId.c_str());
        g_attachCntCv.notify_one();
    }
    return SUCCESS;
}

int32_t CNwareVolumeHandler::Close()
{
    if (m_spDeviceFile->Close() != DISK_DEVICE_OK) {
        std::string errorDes = m_spDeviceFile->GetErrString();
        ERRLOG("Close volume file failed, errorDes:%s", errorDes.c_str());
        return FAILED;
    }

    return CloudVolumeHandler::Close();
}

bool CNwareVolumeHandler::GetCNwareVersion(std::string &version)
{
    CNwareRequest req;
    SetCommonInfo(req);
    std::shared_ptr<ResponseModel> response = m_cnwareClient->GetVersionInfo(req);
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Request VersionInfo return failed. Task id: %s", m_taskId.c_str());
        return false;
    }
    CNwareVersionInfo resBody;
    if (!Module::JsonHelper::JsonStringToStruct(response->GetBody(), resBody)) {
        ERRLOG("Transfer CNwareVersionInfo failed, %s", m_taskId.c_str());
        return false;
    }
    version = std::move(resBody.m_productVersion);
    INFOLOG("CNware version is %s", version.c_str());
    return true;
}
 
uint64_t CNwareVolumeHandler::GetAttachLimit()
{
    if (g_versionAttachLimit == -1) {
        std::string cwVersion = "";
        // 8.2系列版本支持16个虚拟机磁盘挂载
        if (!GetCNwareVersion(cwVersion) ||
            cwVersion.substr(0, CNWARE_VERSION_V1.length()) == CNWARE_VERSION_V1) {
            g_versionAttachLimit = CNWARE_ATTACH_LIMIT_V1;
        } else {
            g_versionAttachLimit = CNWARE_ATTACH_LIMIT_V2;
        }
        INFOLOG("Attach number check: version attach limit %d", g_versionAttachLimit);
    }
    uint64_t attached = GetAttachedVolNumber();
    if (attached == 0) {
        g_attachLimit = g_versionAttachLimit;
    } else {
        g_attachLimit = g_versionAttachLimit <= attached ? 0 : g_versionAttachLimit - attached;
    }
    INFOLOG("Attach number check: already attached: %ld, left: %ld, %s", attached, g_attachLimit, m_jobId.c_str());
    return g_attachLimit;
}

uint64_t CNwareVolumeHandler::GetAttachedVolNumber()
{
    GetVMDiskInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(m_serverId);
    std::shared_ptr<GetVMDiskInfoResponse> response = m_cnwareClient->GetVMDiskInfo(req);
    if (response == nullptr) {
        ERRLOG("Attach number check: get list of vols on agent failed, %s", m_taskId.c_str());
        return 0;
    }
    return response->GetInfo().m_diskDevices.size();
}

std::string CNwareVolumeHandler::GetHostName()
{
    if (!m_serverName.empty()) {
        return m_serverName;
    }
    GetVMInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(m_serverId);
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