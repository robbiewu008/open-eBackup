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
#include "common/Utils.h"
#include "CNwareProtectEngine.h"

using namespace VirtPlugin;
using namespace CNwarePlugin::CNwareErrorCode;

namespace {
const std::string MODULE = "CNwareProtectEngine";
const int64_t INIT_FAILED = 200;
const int64_t UNAUTH_ERROR_CODE = 1007;
const int64_t UNAUTH_LOCKED_CODE = 1008;
const int64_t UNAUTH_931ERROR_CODE = 1877; // 931版本登录失败错误码
const int64_t UNAUTH_CERT_CODE = 1869;   // 931版本验证码失败错误码
const int64_t CERT_ERROR_CODE = 60;
const int CN_COLOUM_LENTH = 3;
const int RETRY_TIMES = 3;
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
const int32_t CNWARE_ATTACH_VOLUME_EXIST_SOURCE = 2;
const int32_t FIFTEEN_MIN_IN_SEC = 900;
const int32_t CREATE_NEW_DISK = 1;
const int32_t USE_OLD_DISK = 0;
const int32_t PROTOCAL_NFS_CIFS = 3;
const std::string RESTORE_LOCATION_ORIGINAL = "original";
const std::string RESTORE_LOCATION_NEW = "new";
const std::string CURRENT_BOOT_DISK = "YES";
const std::string STATUS_BEGINNING_RUNNING = "0\%2C1";
const std::string VERSION_KYLIN_CHANGED = "9.3.0";
const std::string KYLIN_OS_VERSION_9_3 = "银河麒麟高级服务器版V10（aarch64）";
const std::string KYLIN_OS_VERSION_OLD = "麒麟V10";
const std::string OTHER_OS_VERSION = "其他操作系统（aarch64）";
const int RESTORE_OS_VERSION = 1;
const int PAGE_1 = 1;
const int NUM_2 = 2;
const int NUM_100 = 100;
const int CNWARE_VM_NAME_LIMIT = 128;
std::vector<int64_t> AUTH_FALED_CODE = {
    1003, // UNAUTH_ERROR_USER
    1007, // UNAUTH_ERROR_CODE
    1008, // UNAUTH_LOCKED_CODE
    1869, // UNAUTH_CERT_CODE
    1877, // UNAUTH_931ERROR_CODE
    1873  // UNAUTH_931USER_CODE
};
std::map<std::string, int32_t> DISK_BUS_MAP = {
    {"virtio", 1},
    {"ide", 2},
    {"scsi", 3},
    {"sata", 4},
    {"usb", 5}
};
std::vector<std::string> g_supportedVersionList {};
std::map<int64_t, std::string> CNWARE_LABEL_DICT = {
    {2479, "virtual_plugin_cnware_memory_limit_label"}
};

std::map<int32_t, std::string> HOST_STATUS_MAP = {
    {0, "Normal"},
    {1, "Disconnected"},
    {2, "Maintaining"},
    {3, "Error"}
};
}  // namespace


namespace CNwarePlugin {

bool CNwareProtectEngine::Init(const bool &idFlag)
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
    if (idFlag) {
        m_domainId = Utils::GetProxyHostId(true);
    }
    return (m_cnwareClient != nullptr);
}

bool CNwareProtectEngine::AppInitClient(int64_t& errorCode, std::string &errorDes)
{
    if (m_cnwareClient == nullptr) {
        ERRLOG("CNware Client nullptr! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    if (m_cnwareClient->InitCNwareClient(m_appEnv) != SUCCESS) {
        ERRLOG("Build login body failed! Taskid: %s", m_appEnv.id.c_str());
        errorCode = INIT_CLIENT_FAILED;
        return false;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient->CheckAuth(req, errorCode, errorDes) != SUCCESS) {
        ERRLOG("Login client failed! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    return true;
}

bool CNwareProtectEngine::InitClient(int64_t& errorCode, std::string &errorDes)
{
    if (m_cnwareClient == nullptr) {
        ERRLOG("CNware Client nullptr! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    if (m_cnwareClient->InitCNwareClient(m_appEnv) != SUCCESS) {
        ERRLOG("Build login body failed! Taskid: %s", m_appEnv.id.c_str());
        errorCode = INIT_CLIENT_FAILED;
        return false;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    RequestInfo requestInfo;
    if (m_cnwareClient->GetSessionAndlogin(req, requestInfo, errorCode, errorDes) != SUCCESS) {
        ERRLOG("Login client failed! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    int32_t pageNum = Module::ConfigReader::getInt("CNwareConfig", "RequestPageNums");
    if (pageNum > 0) {
        m_cnwareClient->SetPageSize(pageNum);
    }
    return true;
}

void CNwareProtectEngine::SetCommonInfo(CNwareRequest& req)
{
    if (m_certMgr == nullptr) {
        ERRLOG("SetCommonInfo m_certMgr nullptr! Taskid: %s", m_jobId.c_str());
        return;
    }
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

void CNwareProtectEngine::SetAppEnv(const ApplicationEnvironment &appEnv)
{
    m_appEnv = appEnv;
    return;
}

void CNwareProtectEngine::DiscoverApplications(std::vector<Application>& returnValue, const std::string& appType)
{
    return;
}

void CNwareProtectEngine::CheckApplication(ActionResult &returnValue, const ApplicationEnvironment &appEnv,
    const AppProtect::Application &application)
{
    INFOLOG("CheckApplication start! Task id:%s", m_jobId.c_str());
    SetAppEnv(appEnv);
    int64_t errorCode(0);
    std::vector<std::string> params {};
    if (!Init(false)) {
        ERRLOG("Init CNware engine failed! Taskid: %s", appEnv.id.c_str());
        errorCode = CNWARE_ERR_PARAM;
        params.emplace_back("");
        returnValue.__set_bodyErr(errorCode);
        returnValue.__set_bodyErrParams(params);
        returnValue.__set_code(ACTION_ERROR);
        return;
    }
    std::string errorDes;

    if (!AppInitClient(errorCode, errorDes)) {
        if (std::find(AUTH_FALED_CODE.begin(), AUTH_FALED_CODE.end(), errorCode) != AUTH_FALED_CODE.end()) {
            if (errorCode == UNAUTH_ERROR_CODE || errorCode == UNAUTH_LOCKED_CODE) {
                errorDes.erase(errorDes.size() - CN_COLOUM_LENTH);
            }
            params.emplace_back(errorDes);
            ERRLOG("Auth error, errorcode= %d, Des: %s! Taskid: %s", errorCode, errorDes.c_str(), m_jobId.c_str());
            returnValue.__set_bodyErr(CNWARE_AUTH_FAILED);
        } else if (errorCode == CERT_ERROR_CODE) {
            ERRLOG("Cert error, errorcode= %d, Des: %s! Taskid: %s", errorCode, errorDes.c_str(), m_jobId.c_str());
            returnValue.__set_bodyErr(CNWARE_CERT_FAILED);
            params.emplace_back("");
        } else {
            ERRLOG("InitClient failed, errorcode= %d, Des: %s! Taskid: %s",
                errorCode, errorDes.c_str(), m_jobId.c_str());
            returnValue.__set_bodyErr(CNWARE_CONNECT_FAILED);
            params.emplace_back("");
        }
        returnValue.__set_bodyErrParams(params);
        returnValue.__set_code(ACTION_ERROR);
        return;
    }
    INFOLOG("CheckApplication success! Task id:%s", m_jobId.c_str());
    params.emplace_back("");
    returnValue.__set_bodyErrParams(params);
    returnValue.__set_code(ACTION_SUCCESS);
    return;
}

void CNwareProtectEngine::DiscoverHostCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv)
{
    return;
}

void CNwareProtectEngine::DiscoverAppCluster(ApplicationEnvironment& returnEnv,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    SetAppEnv(appEnv);
    int64_t errorCode(FAILED);
    std::string errorDes;
    if (!Init(false)) {
        ERRLOG("Init CNware engine failed! Taskid: %s", appEnv.id.c_str());
        errorCode = INIT_CLIENT_FAILED;
        ThrowPluginException(errorCode);
    }
    if (m_cnwareClient == nullptr || m_cnwareClient->InitCNwareClient(m_appEnv) != SUCCESS) {
        ERRLOG("Build login body failed! Taskid: %s", m_appEnv.id.c_str());
        errorCode = INIT_CLIENT_FAILED;
        ThrowPluginException(errorCode);
    }
    CNwareRequest request;
    SetCommonInfo(request);
    RequestInfo requestInfo;
    m_cnwareClient->ForceLogOut(request);
    if (m_cnwareClient->GetSessionAndlogin(request, requestInfo, errorCode, errorDes) != SUCCESS) {
        ERRLOG("Login client failed! Taskid: %s", m_appEnv.id.c_str());
        ThrowPluginException(errorCode);
    }
    int32_t iret = m_cnwareClient->GetVersionInfo(request, returnEnv);
    if (iret != SUCCESS) {
        ERRLOG("DiscoverAppCluster failed! Taskid: %s", appEnv.id.c_str());
        errorDes = "Get CNware VersionInfo failed.";
        ThrowPluginException(errorCode, errorDes);
    }
    returnEnv.__set_id(appEnv.id);
    returnEnv.__set_name(appEnv.name);
    returnEnv.__set_type(appEnv.type);
    returnEnv.__set_subType(appEnv.subType);
    returnEnv.__set_endpoint(appEnv.endpoint);
    return;
}

void CNwareProtectEngine::ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application, const ApplicationResource& parentResource)
{
    return;
}

void CNwareProtectEngine::ListApplicationResourceV2(ResourceResultByPage& page, const ListResourceRequest& request)
{
    SetAppEnv(request.appEnv);
    int64_t errorCode(0);
    if (!Init(false)) {
        ERRLOG("Init CNware engine failed! Subtype: %s", request.appEnv.subType.c_str());
        return;
    }
    std::string errorDes;
    if (!InitClient(errorCode, errorDes)) {
        ERRLOG("InitClient failed! Subtype: %s", request.appEnv.subType.c_str());
        return;
    }
    std::shared_ptr<CNwareResourceManager> ResourceMgrPtr = std::make_shared<CNwareResourceManager>(
        request.appEnv, request.condition, m_cnwareClient);
    if (ResourceMgrPtr == nullptr) {
        ERRLOG("Init ResourceMgrPtr failed! Subtype: %s", request.appEnv.subType.c_str());
        return;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    if (ResourceMgrPtr->GetTargetResource(page, req) != SUCCESS) {
        ERRLOG("Get target resource failed! Subtype: %s", request.appEnv.subType.c_str());
        return;
    }
    return;
}

int32_t CNwareProtectEngine::GetCNwareVersion(std::string &version)
{
    CNwareRequest req;
    SetCommonInfo(req);
    std::shared_ptr<ResponseModel> response = m_cnwareClient->GetVersionInfo(req);
 
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Request VersionInfo return failed. Task id: %s", m_taskId.c_str());
        return FAILED;
    }
    CNwareVersionInfo resBody;
    if (!Module::JsonHelper::JsonStringToStruct(response->GetBody(), resBody)) {
        ERRLOG("Transfer CNwareVersionInfo failed, %s", m_taskId.c_str());
        return FAILED;
    }
    version = std::move(resBody.m_productVersion);
    INFOLOG("CNware version is %s", version.c_str());
    return SUCCESS;
}

bool CNwareProtectEngine::CheckCNwareVersion()
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
    if (g_supportedVersionList.empty()) {
        std::string supportVersion = Module::ConfigReader::getString("CNwareConfig", "SupportVersion");
        INFOLOG("supportVersion %s", supportVersion.c_str());
        if (supportVersion.empty()) {
            ERRLOG("Read support version from config failed.");
            return false;
        }
        boost::split(g_supportedVersionList, supportVersion, boost::is_any_of(","));
    }
    for (const auto &version : g_supportedVersionList) {
        // 以已支持版本开头的后续子版本同样通过检查
        if (resBody.m_productVersion.find(version) == 0) {
            return true;
        }
    }
    ERRLOG("Not supported CNware version, %s", resBody.m_productVersion.c_str());
    return false;
}

bool CNwareProtectEngine::InitCancelLiveMountJobPara(const std::shared_ptr<ThriftDataBase> &jobInfo)
{
    m_cancelLivemountPara = std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(jobInfo);
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("Get cancel live mount job parameter failed, %s", m_taskId.c_str());
        return false;
    }
    m_requestId = m_cancelLivemountPara->requestId;
    m_appEnv = m_cancelLivemountPara->targetEnv;
    m_application = m_cancelLivemountPara->targetObject;
    return true;
}

bool CNwareProtectEngine::InitJobPara()
{
    INFOLOG("Enter");
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
        m_subObjects = m_backupPara->protectSubObject;
    } else if (m_jobHandle->GetJobType() == JobType::RESTORE ||
        m_jobHandle->GetJobType() == JobType::INSTANT_RESTORE) {
        if (!InitRestoreJobPara(jobInfo)) {
            ERRLOG("Init restore job parameter failed. %s", m_taskId.c_str());
            return false;
        }
    } else if (m_jobHandle->GetJobType() == JobType::CANCELLIVEMOUNT) {
        if (!InitCancelLiveMountJobPara(jobInfo)) {
            ERRLOG("Init cancel live mount job parameter failed. %s", m_taskId.c_str());
            return false;
        }
    } else if (m_jobHandle->GetJobType() != JobType::LIVEMOUNT) {
        ERRLOG("Unsupported job type: %d. %s", static_cast<int>(m_jobHandle->GetJobType()), m_taskId.c_str());
        return false;
    }
    m_taskId = GetTaskId();
    m_initialized = true;

    if (m_jobHandle->GetJobType() != JobType::LIVEMOUNT) {
        INFOLOG("Job is not livemount.");
        return ConnectToEnv(m_appEnv) == SUCCESS;
    }

    INFOLOG("Initialize CNware protect engine job parameter handler success, %s", m_taskId.c_str());
    return true;
}

bool CNwareProtectEngine::InitRestoreJobPara(std::shared_ptr<ThriftDataBase> &jobInfo)
{
    m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo);
    if (m_restorePara == nullptr) {
        ERRLOG("Get restore job parameter failed, %s", m_taskId.c_str());
        return false;
    }
    m_requestId = m_restorePara->requestId;
    m_appEnv = m_restorePara->targetEnv;
    m_application = m_restorePara->targetObject;
    m_subObjects = m_restorePara->restoreSubObjects;
    m_copy = m_restorePara->copies[0];

    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert m_restorePara extendInfo failed, %s", m_jobId.c_str());
        return false;
    }
    m_jobAdvPara = jobAdvancePara;
    if (jobAdvancePara.isMember("restoreLevel") && jobAdvancePara.isMember("targetLocation")) {
        m_restoreLevel = RestoreLevel(std::stoi(jobAdvancePara["restoreLevel"].asString()));
        m_targetLocation = jobAdvancePara["targetLocation"].asString();
    }
    if (GetIpStrForLivemount() != SUCCESS) {
        ERRLOG("Get IpStr For Livemount failed. %s", m_jobId.c_str());
    }
    return true;
}

bool CNwareProtectEngine::InitLiveMountJobPara()
{
    INFOLOG("Enter");
    if (m_initialized && m_livemountPara != nullptr) {
        DBGLOG("Initialized, return directly, %s", m_taskId.c_str());
        return true;
    }
    std::shared_ptr<ThriftDataBase> jobInfo = m_jobHandle->GetJobCommonInfo()->GetJobInfo();
    if (m_jobHandle->GetJobType() == JobType::LIVEMOUNT) {
        m_livemountPara = std::dynamic_pointer_cast<AppProtect::LivemountJob>(jobInfo);
        if (m_livemountPara == nullptr) {
            ERRLOG("Get restore job parameter failed, %s", m_taskId.c_str());
            return false;
        }
        m_requestId = m_livemountPara->requestId;
        m_appEnv = m_livemountPara->targetEnv;
        m_application = m_livemountPara->targetObject;
        m_subObjects = m_livemountPara->targetSubObjects;
        m_copy = m_livemountPara->copy;

        Json::Value jobAdvancePara;
        if (!Module::JsonHelper::JsonStringToJsonValue(m_livemountPara->extendInfo, jobAdvancePara)) {
            ERRLOG("Convert m_livemountPara extendInfo failed, %s", m_jobId.c_str());
            return false;
        }
        m_jobAdvPara = jobAdvancePara;
        if (jobAdvancePara.isMember("livemountType")) {
            m_livemountType = LivemountType(std::stoi(jobAdvancePara["livemountType"].asString()));
        }
        if (GetIpStrForLivemount() != SUCCESS) {
            ERRLOG("Get IpStr For Livemount failed. %s", m_jobId.c_str());
            return false;
        }
        m_taskId = GetTaskId();
        m_initialized = true;

        if (ConnectToEnv(m_appEnv) != SUCCESS) {
            ERRLOG("Failed to Connect to Env.");
            return false;
        }
        INFOLOG("Initialize CNware protect engine job parameter handler success, %s", m_taskId.c_str());
    }
    return true;
}

bool CNwareProtectEngine::GetVolInfoOnStorage(const std::string &volId, CNwareDiskInfo &volumeDetail)
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

std::string CNwareProtectEngine::GetBusDevStr(const std::string &volId, const DomainDiskInfoResponse &disksInfo)
{
    std::string ret = "";
    for (const auto &disk : disksInfo.m_diskDevices) {
        if (disk.m_volId != volId) {
            continue;
        } else {
            ret = disk.m_bus + "-" + disk.m_dev;
        }
    }
    return std::move(ret);
}

bool CNwareProtectEngine::CheckBusDevStr(const std::string &busDevStr, const DomainDiskInfoResponse &disksInfo)
{
    for (const auto &disk : disksInfo.m_diskDevices) {
        if ((disk.m_bus + "-" + disk.m_dev) == busDevStr) {
            WARNLOG("Find dev(%s) still mounted, volId(%s)", busDevStr.c_str(), disk.m_volId.c_str());
            return false;
        }
    }
    return true;
}

bool CNwareProtectEngine::CheckDetachDevList(const std::vector<std::string> &beforeDevList)
{
    std::vector<std::string> afterDevList;
    int countDev = 0;
    int retryTimes = 0;
    std::string detachDev;
    while (retryTimes < RETRY_TIMES) {
        afterDevList.clear();
        countDev = 0;
        if (!ListDev(afterDevList)) {
            ERRLOG("ListDev afterfailed, %s", m_taskId.c_str());
            return false;
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
            return true;
        } else {
            retryTimes++;
            WARNLOG("Detach vol is not ready, retry %d times", retryTimes);
            sleep(COMMON_WAIT_TIME);
        }
    }
    ERRLOG("CheckDetachDevList failed. %s", m_taskId.c_str());
    return false;
}

bool CNwareProtectEngine::GetNewAttachDev(const std::vector<std::string> &beforeDevList)
{
    std::vector<std::string> afterDevList {};
    int retryTimes = 0;
    int countDev = 0;
    std::string newDev;
    while (retryTimes < RETRY_TIMES) {
        afterDevList.clear();
        countDev = 0;
        if (!ListDev(afterDevList)) {
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

bool CNwareProtectEngine::DoDetachVolumeOnVm(const std::string &voId, const std::string &domainId)
{
    if (domainId.empty()) {
        INFOLOG("Domain id is null, no need detatch disk.");
        return true;
    }
    GetVMDiskInfoRequest reqDiskInfo;
    reqDiskInfo.SetDomainId(domainId);
    SetCommonInfo(reqDiskInfo);
    std::shared_ptr<GetVMDiskInfoResponse> responseDiskInfo = m_cnwareClient->GetVMDiskInfo(reqDiskInfo);
    if (responseDiskInfo == nullptr) {
        ERRLOG("Get disk info on vm %s failed, %s", domainId.c_str(), m_taskId.c_str());
        return false;
    }
    DetachDiskOnVMRequest reqDetach;
    std::string busDevStr = GetBusDevStr(voId, responseDiskInfo->GetInfo());
    if (busDevStr == "") {
        ERRLOG("Get disk %s bus-Dev string on vm %s failed, %s", voId.c_str(), domainId.c_str(), m_taskId.c_str());
        return false;
    }
    reqDetach.SetParam(domainId, busDevStr);
    SetCommonInfo(reqDetach);
    int retryNum {0};
    std::vector<std::string> beforeDevList {};
    if (domainId == m_domainId && !ListDev(beforeDevList)) {
        ERRLOG("ListDev on vm %s failed, %s", domainId.c_str(), m_taskId.c_str());
        return false;
    }
    std::shared_ptr<CNwareResponse> responseDetach = std::make_shared<CNwareResponse>();
    while (retryNum < RETRY_TIMES) {
        responseDetach = m_cnwareClient->DetachDiskOnVM(reqDetach);
        if (responseDetach == nullptr || !CheckTaskStatus(responseDetach->GetTaskId())) {
            ERRLOG("Detach disk %s on vm %s failed, %s", voId.c_str(), domainId.c_str(), m_taskId.c_str());
            responseDiskInfo = m_cnwareClient->GetVMDiskInfo(reqDiskInfo);
            if (responseDiskInfo == nullptr) {
                ERRLOG("Get disk info on vm %s failed, %s", domainId.c_str(), m_taskId.c_str());
                return false;
            }
            if (!CheckBusDevStr(busDevStr, responseDiskInfo->GetInfo())) {
                ++retryNum;
                sleep(COMMON_WAIT_TIME);
                continue;
            }
            break;
        }
        break;
    }
    return (domainId != m_domainId && retryNum < RETRY_TIMES) ? true : CheckDetachDevList(beforeDevList);
}

bool CNwareProtectEngine::ListDev(std::vector<std::string> &devList)
{
    std::string diskPath;
    std::vector<std::string> cmdOut;
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::vector<Module::CmdParam> cmdParam{
        Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
        Module::CmdParam(Module::SCRIPT_CMD_NAME, agentHomedir + SUDO_DISK_TOOL_PATH),
        "scan_dev"
    };
    if (Utils::CallAgentExecCmd(cmdParam, cmdOut) != 0) {
        ERRLOG("lsblk command is not enable.");
        return false;
    }
    for (const std::string& buff : cmdOut) {
        std::string strLine(buff);
        size_t pos = strLine.find('\n');
        if (pos != std::string::npos) {
            strLine.erase(pos, 1);
        }
        if (strLine.empty()) {
            ERRLOG("Do not find a dev!");
            return false;
        }
        devList.emplace_back(strLine);
    }
    return true;
}

bool CNwareProtectEngine::DetachVolumeHandle(const CNwareDiskInfo &volumeDetail)
{
    for (const auto &vm : volumeDetail.m_userList) {
        ApplicationLabelType labelParam;
        labelParam.label = CNWARE_DETACH_VOLUME_LABEL;
        labelParam.params = std::vector<std::string>{vm.m_domainName, volumeDetail.m_name};
        ReportJobDetail(labelParam);
        {
            std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
            if (!DoDetachVolumeOnVm(volumeDetail.m_id, vm.m_domainId)) {
                ERRLOG("Detach vol %s on vm %s failed.", volumeDetail.m_id.c_str(), vm.m_domainName.c_str());
                labelParam.label = CNWARE_DETACH_VOLUME_FAILED_LABEL;
                labelParam.level = JobLogLevel::TASK_LOG_ERROR;
                ReportJobDetail(labelParam);
                return false;
            }
        }
    }
    INFOLOG("Detach disk %s on all vm success, %s", volumeDetail.m_id.c_str(), m_taskId.c_str());
    return true;
}

bool CNwareProtectEngine::DoDeleteVolume(const std::string &volId)
{
    DelDiskOnStorageRequest req;
    req.SetVolId(volId);
    SetCommonInfo(req);
    std::shared_ptr<DeleteDiskOnStorageResponse> response = m_cnwareClient->DeleteDiskOnStorage(req);
    if (!CommonCheckResponse(response)) {
        ERRLOG("Send delete volume %s on storage request failed, %s", volId.c_str(), m_taskId.c_str());
        return false;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Delete volume %s on storage failed, %s", volId.c_str(), m_taskId.c_str());
        return false;
    }
    INFOLOG("Delete disk %s success, %s", volId.c_str(), m_taskId.c_str());
    return true;
}

void CNwareProtectEngine::FinishLastTaskCache()
{
    InitRepoHandler();
    std::string LastTaskFile = m_cacheRepoPath + VIRT_PLUGIN_LAST_TASK_INFO;
    std::string taskId;
    if (Utils::ReadFile(m_cacheRepoHandler, LastTaskFile, taskId) !=
        SUCCESS) {
        ERRLOG("Read LastTaskFile failed.");
    }
    if (!taskId.empty()) {
        CheckTaskStatus(taskId);
    }
    return;
}

int CNwareProtectEngine::PreHook(const ExecHookParam &para)
{
    if (!InitJobPara()) {
        ERRLOG("InitJobPara, %s", m_taskId.c_str());
        return FAILED;
    }

    if (!InitLiveMountJobPara()) {
        ERRLOG("InitLiveMountJobPara, %s", m_taskId.c_str());
        return FAILED;
    }
    FinishLastTaskCache();
    if (para.stage == JobStage::EXECUTE_SUB_JOB) {
        m_reportParam = {
            "virtual_plugin_restore_job_execute_subjob_start_label",
            JobLogLevel::TASK_LOG_INFO,
            SubJobStatus::RUNNING, 0, 0 };
    }

    return SUCCESS;
}

int CNwareProtectEngine::PostHook(const ExecHookParam &para)
{
    if (para.stage != JobStage::POST_JOB) {
        return SUCCESS;
    }
    if (m_backupPara != nullptr) {
        if (HealthDomain(m_backupPara->protectObject.id) != SUCCESS) {
            ERRLOG("Health domain(%s) failed, %s", m_backupPara->protectObject.id.c_str(),
                m_taskId.c_str());
            return FAILED;
        }
    }
    if (m_restorePara != nullptr) {
        if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
            INFOLOG("Create new server, dont health vm status.");
            return SUCCESS;
        }
        if (HealthDomain(m_restorePara->targetObject.id) != SUCCESS) {
            ERRLOG("Health domain(%s) failed, %s", m_restorePara->targetObject.id.c_str(),
                m_taskId.c_str());
            return FAILED;
        }
    }
    return SUCCESS;
}

std::string CNwareProtectEngine::GetTaskId() const
{
    return "TaskId=" + m_requestId + ".";
}

void CNwareProtectEngine::SetCNwareVMInfo2VMInfo(const CNwareVMInfo &cnVmInfo, VMInfo &vmInfo)
{
    vmInfo.m_moRef = cnVmInfo.m_id;
    vmInfo.m_uuid = cnVmInfo.m_id;
    vmInfo.m_name = cnVmInfo.m_name;
    vmInfo.m_location = cnVmInfo.m_hostId;
    vmInfo.m_bootType = std::to_string(cnVmInfo.m_bootType);
    if (!GetHostName(cnVmInfo.m_hostId, vmInfo.m_locationName)) {
        ERRLOG("Do not find host name, use id(%s). %s", cnVmInfo.m_hostId.c_str(), m_taskId.c_str());
        vmInfo.m_locationName = cnVmInfo.m_hostId;
    }

    for (const auto &inter : cnVmInfo.m_bridgeInterfaces) {
        BridgeInterfaceInfo interface;
        interface.m_uuid = inter.m_interfaceId;
        interface.m_name = inter.m_portGroupName;
        interface.m_parentName = inter.m_bridge;
        interface.m_mac = inter.m_mac;
        interface.m_moRef = vmInfo.m_uuid;
        interface.m_ip = inter.m_ip;
        BridgeInterfaces tmpInter = inter;
        if (!Module::JsonHelper::StructToJsonString(tmpInter, interface.m_metadata)) {
            ERRLOG("Convert InterfaceInfo to json string failed, %s", m_taskId.c_str());
        }
        vmInfo.m_interfaceList.emplace_back(interface);
    }

    CNwareVMInfo tmp = cnVmInfo;
    if (!Module::JsonHelper::StructToJsonString(tmp, vmInfo.m_metadata)) {
        ERRLOG("Convert CNwareVMInfo to json string failed, %s", m_taskId.c_str());
        return;
    }
}

bool CNwareProtectEngine::SetVolListInfo2VMInfo(const DomainDiskInfoResponse &disksInfo, VMInfo &vmInfo)
{
    for (const auto &disk : disksInfo.m_diskDevices) {
        CNwareDiskInfo diskObj;
        DBGLOG("Get volume %s info on storage. %s", disk.m_volId.c_str(), m_jobId.c_str());
        if (!GetVolInfoOnStorage(disk.m_volId, diskObj)) {
            ERRLOG("Get Volume(%s) info on storage failed, %s", disk.m_volId.c_str(),
                m_taskId.c_str());
            return false;
        }
        VolInfo volInfo;
        volInfo.m_name = diskObj.m_name;
        volInfo.m_moRef = disk.m_volId;
        volInfo.m_uuid = disk.m_volId;
        volInfo.m_type = disk.m_bus;
        volInfo.m_volSizeInBytes = disk.m_capacity;
        volInfo.m_bootable = std::to_string(disk.m_bootOrder);
        volInfo.m_volumeType = std::to_string(disk.m_cache);
        volInfo.m_slotId = disk.m_dev;
        volInfo.m_vmMoRef = vmInfo.m_uuid;
        volInfo.m_location = disk.m_sourceFile;
        volInfo.m_provisionType = disk.m_preallocation;
        volInfo.m_datastore.m_poolId = disk.m_storagePoolId;
        volInfo.m_datastore.m_name = disk.m_storagePoolName;
        volInfo.m_datastore.m_type = std::to_string(disk.m_storagePoolType);
        DomainDiskDevicesResp tmp = disk;
        if (!Module::JsonHelper::StructToJsonString(tmp, volInfo.m_metadata)) {
            ERRLOG("Convert DomainDiskDevicesResp to json string failed, %s", m_taskId.c_str());
            return false;
        }
        INFOLOG("Set VMInfo volInfo.m_metadata: %s", volInfo.m_metadata.c_str());
        DBGLOG("Trans disk info to VolInfo success.%s %s %s Volume size: %llu  volume type: %s. %s.",
               volInfo.m_uuid.c_str(), volInfo.m_name.c_str(), volInfo.m_type.c_str(), volInfo.m_volSizeInBytes,
               volInfo.m_volumeType.c_str(), m_taskId.c_str());
        vmInfo.m_volList.emplace_back(volInfo);

        INFOLOG("Set VMInfo volInfo.m_uuid: %s", volInfo.m_uuid.c_str());
    }
    return true;
}

bool CNwareProtectEngine::GetVMInfoById(const std::string &domainId, CNwareVMInfo &cnVmInfo)
{
    GetVMInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(domainId);
    if (m_cnwareClient == nullptr) {
        ERRLOG("Get VM info m_cnwareClient nullptr, %s", m_taskId.c_str());
        return false;
    }
    std::shared_ptr<GetVMInfoResponse> resp = m_cnwareClient->GetVMInfo(req);
    if (resp == nullptr) {
        ERRLOG("Get VM info failed, %s", m_taskId.c_str());
        return false;
    }

    GetVMInterfaceInfoRequest reqInterface;
    SetCommonInfo(reqInterface);
    reqInterface.SetDomainId(domainId);
    std::shared_ptr<GetVMInterfaceInfoResponse> respInterface = m_cnwareClient->GetVMInterfaceInfo(reqInterface);
    if (respInterface == nullptr) {
        ERRLOG("Get VM interface list info failed, %s", m_taskId.c_str());
        return false;
    }
    cnVmInfo = std::move(resp->GetInfo());

    if (!respInterface->GetInfo().m_bridgeInterfaces.empty()) {
        cnVmInfo.m_bridgeInterfaces = std::move(respInterface->GetInfo().m_bridgeInterfaces);
    } else {
        INFOLOG("Empty bridgeInterfaces");
    }
    return true;
}

bool CNwareProtectEngine::DoGetMachineMetadata(const std::string &vmId)
{
    if (m_machineMetaCached) {
        INFOLOG("Machine metadata cached, %s", m_taskId.c_str());
        return true;
    }

    CNwareVMInfo cnwareVmInfo;
    if (!GetVMInfoById(vmId, cnwareVmInfo)) {
        ERRLOG("Get VM info failed when get meta data, %s", m_taskId.c_str());
        return false;
    }

    VMInfo protectVmInfo;
    SetCNwareVMInfo2VMInfo(cnwareVmInfo, protectVmInfo);

    GetVMDiskInfoRequest reqVol;
    SetCommonInfo(reqVol);
    reqVol.SetDomainId(vmId);
    std::shared_ptr<GetVMDiskInfoResponse> respVol = m_cnwareClient->GetVMDiskInfo(reqVol);
    if (respVol == nullptr || !SetVolListInfo2VMInfo(respVol->GetInfo(), protectVmInfo)) {
        ERRLOG("Get VM vol list info failed, %s", m_taskId.c_str());
        return false;
    }

    m_vmInfo = protectVmInfo;
    m_machineMetaCached = true;
    DBGLOG("Do get machine metadata success, %s", m_taskId.c_str());
    return true;
}

bool CNwareProtectEngine::QueryCreateSnapshotTask(const std::string &snapShotTaskId,
    std::vector<SnapshotDiskInfo> &snapshotInfoList)
{
    if (!CheckTaskStatus(snapShotTaskId)) {
        ERRLOG("Create snapshot task %s failed, %s", snapShotTaskId.c_str(), m_taskId.c_str());
        return false;
    }
    
    InquiriesSnapshotRequest req;
    SetCommonInfo(req);
    req.SetTaskId(snapShotTaskId);
    std::shared_ptr<InquiriesSnapshotResponse> response = m_cnwareClient->InquiriesSnapshot(req);
    if (!CommonCheckResponse(response)) {
        ERRLOG("Query create snapshot task %s failed, %s", snapShotTaskId.c_str(), m_taskId.c_str());
        return false;
    }
    snapshotInfoList = std::move(response->GetInquiriesSnapshotInfo().m_diskInfoList);

    return true;
}

bool CNwareProtectEngine::GetVolDevOnVm(const std::string &volId, const std::string &vmId, std::string &dev)
{
    CNwareDiskInfo volumeDetail;
    volumeDetail.m_id = "";
    if (!GetVolInfoOnStorage(volId, volumeDetail)) {
        ERRLOG("Query volume(%s) detail failed, %s", volId.c_str(), m_taskId.c_str());
        return false;
    }
    if (volumeDetail.m_id == "") {
        ERRLOG("Volume(%s) does not exist.", volId.c_str());
        return false;
    }
    if (volumeDetail.m_userList.empty()) {
        ERRLOG("Volume(%s) is not used.", volId.c_str());
        return false;
    }
    for (const auto &user : volumeDetail.m_userList) {
        if (user.m_domainId == vmId) {
            dev = user.m_dev;
            break;
        }
    }
    if (dev == "") {
        ERRLOG("Volume(%s) is not used by vm(%s).", volId.c_str(), vmId.c_str());
        return false;
    } else {
        INFOLOG("Volume(%s) is used by vm(%s) on (%s)", volId.c_str(), vmId.c_str(), dev.c_str());
        return true;
    }
}

bool CNwareProtectEngine::GetSnapshotOriginVolInfo(const std::string &volId, DomainDiskDevicesResp &deviceInfo)
{
    std::string dev = "";
    if (!GetVolDevOnVm(volId, m_backupPara->protectObject.id, dev)) {
        ERRLOG("Get snapshot origin volume(%s) dev failed.", volId.c_str());
        return false;
    }

    for (auto &volOnVm : m_vmInfo.m_volList) {
        DomainDiskDevicesResp diskDevice;
        if (!Module::JsonHelper::JsonStringToStruct(volOnVm.m_metadata, diskDevice)) {
            ERRLOG("Volume metadata trans to diskDevice failed.");
            return false;
        }
        if (diskDevice.m_dev == dev) {
            deviceInfo = diskDevice;
            Json::Value jsInfo;
            Json::FastWriter writer;
            jsInfo["rawVolId"] = volId;
            volOnVm.m_extendInfo = writer.write(jsInfo);
            INFOLOG("Get snapshot original vol(%s) info", diskDevice.m_volId.c_str());
            return true;
        }
    }
    ERRLOG("No device:%s on VM.", dev.c_str());
    return false;
}

bool CNwareProtectEngine::SetVolSnapInfo(const SnapshotDiskInfo &disk, VolSnapInfo &volSnap,
    const std::string &snapShotTaskId)
{
    DomainDiskDevicesResp diskDeviceInfo;
    if (!GetSnapshotOriginVolInfo(disk.m_originVolId, diskDeviceInfo)) {
        ERRLOG("Get snapshot origin volId on Vm failed.");
        return false;
    }
    volSnap.m_volUuid = diskDeviceInfo.m_volId;
    volSnap.m_snapshotName = disk.m_volName;
    volSnap.m_snapshotId = disk.m_volId;
    volSnap.m_size = diskDeviceInfo.m_capacity;
    SnapshotDiskInfo tmpDisk = disk;
    if (!Module::JsonHelper::StructToJsonString(tmpDisk, volSnap.m_extendInfo)) {
        ERRLOG("Convert SnapshotDiskInfo to json string failed, %s", m_taskId.c_str());
        return false;
    }
    if (!Module::JsonHelper::StructToJsonString(diskDeviceInfo, volSnap.m_metadata)) {
        ERRLOG("Convert DomainDiskDevicesResp to json string failed, %s", m_taskId.c_str());
        return false;
    }
    return true;
}

void CNwareProtectEngine::FillTargetDisksToSnapshotMsg(CustomDomainSnapshotReq &reqCustom)
{
    for (const auto &subObject : m_backupPara->protectSubObject) {
        reqCustom.m_snapDisks.push_back(subObject.name);
    }
    return;
}

bool CNwareProtectEngine::GetSnapshotResult(const std::string &snapShotTaskId, SnapshotInfo &snapshot,
    std::string &errCode)
{
    DBGLOG("Start to check create snapshots result. %s", m_taskId.c_str());
    std::vector<SnapshotDiskInfo> snapshotInfoList;
    if (!QueryCreateSnapshotTask(snapShotTaskId, snapshotInfoList)) {
        ERRLOG("Create snap failed, %s", m_taskId.c_str());
        return false;
    }

    for (const auto &disk : snapshotInfoList) {
        VolSnapInfo volSnap;
        if (!SetVolSnapInfo(disk, volSnap, snapShotTaskId)) {
            ERRLOG("Get vol snap info failed, %s", m_taskId.c_str());
        }
        snapshot.m_volSnapList.push_back(volSnap);
    }
    return true;
}

void CNwareProtectEngine::DealCephSnapshot(const std::string &cephTaskId)
{
    if (!m_isCeph) {
        return;
    } else {
        InitRepoHandler();
        if (m_metaRepoHandler == nullptr) {
            ERRLOG("m_metaRepoHandler is nullptr, RepositoryHandler init failed.");
            return;
        }
        std::string metaPath =  m_metaRepoPath + VIRT_PLUGIN_META_ROOT;
        if (!m_metaRepoHandler->Exists(metaPath)) {
            WARNLOG("MetaPath(%s) no exist ,create.", metaPath.c_str());
            int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_metaRepoHandler,
            metaPath), true, "CreateDirectory");
            if (res != SUCCESS) {
                ERRLOG("Create volumes metadata directory failed, %s", m_taskId.c_str());
            }
        }
        std::string snapshotCephInfo = m_metaRepoPath + VIRT_PLUGIN_CEPH_SNAPSHOT_INFO;
        INFOLOG("Save ceph id: %s", cephTaskId.c_str());
        Utils::SaveToFileWithRetry(m_metaRepoHandler, snapshotCephInfo, cephTaskId);
    }
    return;
}

int32_t CNwareProtectEngine::CreateSnapshot(SnapshotInfo &snapshot, std::string &errCode)
{
    if (!DoGetMachineMetadata(m_backupPara->protectObject.id)) {
        ERRLOG("Set VM metadata info failed, %s", m_taskId.c_str());
        return FAILED;
    }

    if (m_vmInfo.m_volList.empty()) {
        ERRLOG("Failed, volume list is empty.task id: %s", m_taskId.c_str());
        return FAILED;
    }

    snapshot.m_moRef = Uuid::GenerateUuid();
    snapshot.m_vmMoRef = m_vmInfo.m_moRef;
    snapshot.m_vmName = m_vmInfo.m_name;

    DBGLOG("snapshot.m_moRef: %s, snapshot.m_vmMoRef: %s, snapshot.m_vmName: %s. %s", snapshot.m_moRef.c_str(),
        snapshot.m_vmMoRef.c_str(), snapshot.m_vmName.c_str(), m_taskId.c_str());

    CreateSnapshotRequest req;
    SetCommonInfo(req);
    req.SetDomainId(m_backupPara->protectObject.id);
    CustomDomainSnapshotReq reqCustom;

    if (m_backupPara->protectSubObject.size() != 0) {
        /* backup target volumes */
        FillTargetDisksToSnapshotMsg(reqCustom);
    }

    req.SetCustomDomainSnapshotReq(reqCustom);
    std::shared_ptr<CNwareResponse> response = m_cnwareClient->CreateSnapshot(req);
    if (response == nullptr || response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Create volume snapshot failed, %s", m_taskId.c_str());
        return FAILED;
    } else {
        if (!GetSnapshotResult(response->GetTaskId(), snapshot, errCode)) {
            ERRLOG("Get snapshot result failed, %s", m_taskId.c_str());
            return FAILED;
        }
    }
    snapshot.m_snapFlag = response->GetTaskId();
    DealCephSnapshot(response->GetTaskId());
    return SUCCESS;
}

bool CNwareProtectEngine::DelVolumeByVolId(const std::string &volId)
{
    CNwareDiskInfo volumeDetail;
    INFOLOG("Begin to delete snap volume(%s).", volId.c_str());
    volumeDetail.m_id = "";
    if (!GetVolInfoOnStorage(volId, volumeDetail)) {
        ERRLOG("Query volume(%s) detail failed, %s", volId.c_str(), m_taskId.c_str());
        return false;
    }
    if (volumeDetail.m_id == "") {
        INFOLOG("Volume(%s) does not exist, skip delete.", volId.c_str());
        return true;
    }
    bool isMounted = !volumeDetail.m_userList.empty();
    // 若卷处于挂载状态，需要先卸载卷
    if (isMounted && !DetachVolumeHandle(volumeDetail)) {
        ERRLOG("Detach volume(%s) failed, %s", volId.c_str(), m_taskId.c_str());
        return false;
    }
    if (!DoDeleteVolume(volumeDetail.m_id)) {
        ERRLOG("Delete volume(%s) failed, %s", volId.c_str(), m_taskId.c_str());
        return false;
    }
    INFOLOG("Delete volume(%s) success.", volId.c_str());
    return true;
}

int32_t CNwareProtectEngine::DeleteCommonSnapshot(const SnapshotInfo &snapshot)
{
    std::vector<VolSnapInfo> snapshotList = snapshot.m_volSnapList;
    for (int i = 0; i < MAX_EXEC_COUNT; i++) {
        for (auto it = snapshotList.begin(); it != snapshotList.end();) {
            INFOLOG("Delete snapshot, snapshot_id: %s. %s", it->m_snapshotId.c_str(), m_taskId.c_str());
            if (DelVolumeByVolId(it->m_snapshotId)) {
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

int32_t CNwareProtectEngine::DeleteCephSnapshot(const std::string &cephSnapFlag)
{
    INFOLOG("Enter");
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("GetStorage m_cnwareClient pointer failed.");
        return FAILED;
    }
    std::shared_ptr<ResponseModel> response = m_cnwareClient->DelCephSnapshots(req, cephSnapFlag);
    if (response == nullptr) {
        ERRLOG("DelCephSnapshots(%s) failed.", cephSnapFlag.c_str());
        return FAILED;
    }
    WARNLOG("Delete ceph snapshot %s, res:%s.", cephSnapFlag.c_str(), WIPE_SENSITIVE(response->GetBody()).c_str());
    if (response->GetBody() == "success" || response->GetBody() == "notFound") {
        return SUCCESS;
    }
    return FAILED;
}

int32_t CNwareProtectEngine::DeleteSnapshot(const SnapshotInfo &snapshot)
{
    InitRepoHandler();
    if (m_metaRepoHandler == nullptr) {
        ERRLOG("m_metaRepoHandler is nullptr, RepositoryHandler init failed.");
        return FAILED;
    }
    if (snapshot.m_volSnapList.empty()) {
        DBGLOG("Snapshot list is empty, no volume snapshot to be deleted, %s", m_taskId.c_str());
        return SUCCESS;
    }
    int32_t iRet = SUCCESS;
    if (DeleteCommonSnapshot(snapshot) != SUCCESS) {
        ERRLOG("Start Delete common snapshot failed, %s.", m_taskId.c_str());
        iRet = FAILED;
    }
    std::string snapshotCephInfo = m_metaRepoPath + VIRT_PLUGIN_CEPH_SNAPSHOT_INFO;
    std::string cephSnapFlag = "";
    if (m_metaRepoHandler->Exists(snapshotCephInfo)) {
        int res = Utils::RetryOpWithT<int>(std::bind(&Utils::ReadFile, m_metaRepoHandler, snapshotCephInfo,
            std::ref(cephSnapFlag)), SUCCESS, "ReadFile");
        if (res != SUCCESS) {
            ERRLOG("Failed to read file %s, %s", snapshotCephInfo.c_str(), m_taskId.c_str());
            return FAILED;
        }
    }
    if (!cephSnapFlag.empty() && DeleteCephSnapshot(cephSnapFlag) != SUCCESS) {
        ERRLOG("Delete ceph snapshot %s failed, %s", cephSnapFlag.c_str(), m_taskId.c_str());
        iRet = FAILED;
    }
    if (!snapshot.m_snapFlag.empty() && DeleteCephSnapshot(snapshot.m_snapFlag) != SUCCESS) {
        ERRLOG("Delete ceph snapshot %s failed, %s", snapshot.m_snapFlag.c_str(), m_taskId.c_str());
        iRet = FAILED;
    }
    INFOLOG("Delete snapshot & ceph(%s) finished, %s.", cephSnapFlag.c_str(), m_taskId.c_str());
    return iRet;
}

int32_t CNwareProtectEngine::QuerySnapshotExists(SnapshotInfo &snapshot)
{
    if (snapshot.m_volSnapList.empty()) {
        ERRLOG("No snapshot provided to query, %s", m_taskId.c_str());
        return FAILED;
    }
    snapshot.m_deleted = true;

    CNwareDiskInfo volumeDetail;
    for (auto &volSnap : snapshot.m_volSnapList) {
        volumeDetail.m_id = "";
        if (!GetVolInfoOnStorage(volSnap.m_snapshotId, volumeDetail)) {
            ERRLOG("Get volume snapshot failed. Volume id: %s, snapshot id: %s. %s", volSnap.m_volUuid.c_str(),
                volSnap.m_snapshotId.c_str(), m_taskId.c_str());
            return FAILED;
        }
        if (volumeDetail.m_id != "") {
            snapshot.m_deleted = false;
        }
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList)
{
    InitRepoHandler();
    if (m_metaRepoHandler == nullptr) {
        ERRLOG("m_metaRepoHandler is nullptr, RepositoryHandler init failed.");
        return FAILED;
    }
    std::string file = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
    if (!m_metaRepoHandler->Exists(file)) {
        INFOLOG("Snapshot.info file(%s) does not exist, %s", file.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Begin to query volume from file(%s).", file.c_str());
    SnapshotInfo tmpSnapInfo;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, file, tmpSnapInfo) != SUCCESS) {
        ERRLOG("Load context failed, %s", m_taskId.c_str());
        return FAILED;
    }
    snapList = std::move(tmpSnapInfo.m_volSnapList);
    INFOLOG("snapList size(%d).", snapList.size());
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetMachineMetadata(VMInfo &vmInfo)
{
    std::string vmId;
    if (m_jobHandle->GetJobType() == JobType::BACKUP) {
        vmId = m_backupPara->protectObject.id;
    } else {
        vmId = m_restorePara->targetObject.id;
    }
    if (!DoGetMachineMetadata(vmId)) {
        ERRLOG("Get machine metadata failed, %s", m_taskId.c_str());
        return FAILED;
    }
    vmInfo = m_vmInfo;
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetVolumesMetadata(const VMInfo &vmInfo,
    std::unordered_map<std::string, std::string> &volsMetadata)
{
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler)
{
    std::shared_ptr<CNwareVolumeHandler> CNwareVolHandler =
        std::make_shared<CNwareVolumeHandler>(GetJobHandle(), volInfo, m_jobId, m_subJobId);
    if (CNwareVolHandler->InitializeVolumeInfo(CNWARE_CONF) != SUCCESS ||
        !CNwareVolHandler->InitClient()) {
        ERRLOG("Initialize volume info failed.");
        return FAILED;
    }
    volHandler = CNwareVolHandler;
    return SUCCESS;
}

int32_t CNwareProtectEngine::CreateVolume(const VolInfo &backupVol, const std::string &volMetaData,
    const std::string &vmMoRef, const DatastoreInfo &storage, VolInfo &newVol)
{
    int ret = FAILED;
    ApplicationLabelType labelParam;
    labelParam.label = CNWARE_CREATE_VOLUME_LABEL;
    labelParam.params = std::vector<std::string>{newVol.m_name};
    ReportJobDetail(labelParam);

    std::vector<std::string> beforeDevList {};
    if (!ListDev(beforeDevList)) {
        ERRLOG("Get CreateVolume VolumeDev failed before.");
        return FAILED;
    }
    if (DoCreateVolume(backupVol, volMetaData, vmMoRef, storage, newVol) != SUCCESS) {
        ERRLOG("DoCreateVolume failed. %s", m_jobId.c_str());
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNWARE_CREATE_VOLUME_FAILED_LABEL;
        labelParam.params = std::vector<std::string>{newVol.m_name};
        labelParam.errCode = CNWARE_CREATE_VOLUME_FAILED_ERROR;
        labelParam.additionalDesc = std::vector<std::string>{ "Create volume failed." };
        ReportJobDetail(labelParam);
        return FAILED;
    }
    if (!GetNewAttachDev(beforeDevList)) {
        ERRLOG("Get CreateVolume VolumeDev failed after.");
        return FAILED;
    }
    INFOLOG("Create volume %s success, %s.", newVol.m_uuid.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoCreateVolume(const VolInfo &backupVol, const std::string &volMetaData,
    const std::string &vmMoRef, const DatastoreInfo &storage, VolInfo &newVol)
{
    InitRepoHandler();
    AddEmptyDiskRequest req;
    SetCommonInfo(req);
    req.SetDomainId(m_domainId);
    AddDiskReq diskBody;
    VolInfo volInfo;
    if (LoadCopyVolumeMatadata(backupVol.m_uuid, volInfo) != SUCCESS) {
        ERRLOG("Get VolumeMatadata %s info failed. %s", backupVol.m_uuid.c_str(), m_jobId.c_str());
        return FAILED;
    }
    DomainDiskDevicesResp diskInfo;
    if (!Module::JsonHelper::JsonStringToStruct(volInfo.m_metadata, diskInfo)) {
        ERRLOG("Get new domain info failed.");
        return FAILED;
    }
    StoragePool pool;
    if (!storage.m_details.empty()) {
        if (!Module::JsonHelper::JsonStringToStruct(storage.m_details, pool)) {
            ERRLOG("Get poolExtendInfo details info failed.");
            return FAILED;
        }
    }
    // 恢复时使用virtio临时挂载执行任务，挂载回原虚拟机时使用原盘的类型
    diskBody.bus = CNWARE_ATTACH_VOLUME_BUS_VIRTIO;
    diskBody.cache = diskInfo.m_cache;
    diskBody.capacity = backupVol.m_volSizeInBytes;
    diskBody.poolName = storage.m_name;
    diskBody.volName = newVol.m_name;
    diskBody.type = GetStorageDriverType(pool.m_type, diskInfo.m_driverType);
    diskBody.preallocation = GetStoragePreallocation(
        pool.m_type, diskInfo.m_preallocation, newVol.m_name);
    diskBody.ioHangTimeout = diskInfo.m_ioHangTimeout;
    req.SetAddDiskReq(diskBody);
    std::shared_ptr<AddEmptyDiskResponse> response = m_cnwareClient->AddEmptyDisk(req);
    if (response == nullptr) {
        ERRLOG("AddEmptyDisk response nullptr , %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("DoCreateVolume %s on storage failed, %s", backupVol.m_name.c_str(), m_jobId.c_str());
        return FAILED;
    }
    int32_t ret = GetDiskFromVMInfo(newVol.m_name, newVol);
    if (ret != SUCCESS) {
        ERRLOG("Create volume %s failed, %s.", backupVol.m_name.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Create volume %s success, %s.", newVol.m_uuid.c_str(), m_taskId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetStorageDriverType(const int32_t &poolType,
    const int32_t &originType)
{
    DBGLOG("Get poolType: %d, originType: %d", poolType, originType);
    if (poolType == static_cast<int32_t>(PoolType::Distribute)) {
        WARNLOG("The storage pool type is NAS, the DriverType setting is ignored and set to OFF. %s",
            m_taskId.c_str());
        return static_cast<int32_t>(DriverType::RAW);
    }
    return originType;
}

std::string CNwareProtectEngine::GetStoragePreallocation(const int32_t &poolType,
    const std::string &originAlloc, const std::string &diskName)
{
    if ((poolType == static_cast<int32_t>(PoolType::NAS) ||
        poolType == static_cast<int32_t>(PoolType::Distribute)) && (originAlloc != PREALLOCATION_OFF)) {
        WARNLOG("The storage pool type is NAS, the preallocation setting is ignored and set to OFF. %s",
            m_taskId.c_str());
        ApplicationLabelType labelParam;
        labelParam.label = CNWARE_NAS_PREALLOCATION_WARNING_LABEL;
        labelParam.level = JobLogLevel::TASK_LOG_WARNING;
        labelParam.params = std::vector<std::string>{diskName};
        if (!diskName.empty()) {
            WARNLOG("Disk name empty! %s.", m_taskId.c_str());
            ReportJobDetail(labelParam);
        }
        return PREALLOCATION_OFF;
    }
    return originAlloc;
}

int32_t CNwareProtectEngine::GetDiskFromVMInfo(const std::string &volName, VolInfo &newVol)
{
    GetVMDiskInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(m_domainId);
    std::shared_ptr<GetVMDiskInfoResponse> response = m_cnwareClient->GetVMDiskInfo(req);
    if (response == nullptr) {
        ERRLOG("GetVMDiskInfoResponse failed! domain: %s", m_domainId.c_str());
        return FAILED;
    }
    if (DoGetDiskFromVMInfo(response->GetInfo(), volName, newVol) == SUCCESS) {
        return SUCCESS;
    }
    ERRLOG("Create volume not find! Task id:%s", m_jobId.c_str());
    return FAILED;
}

int32_t CNwareProtectEngine::DoGetDiskFromVMInfo(const DomainDiskInfoResponse &disksInfo,
    const std::string &volName, VolInfo &newVol)
{
    DBGLOG("Enter");
    for (const DomainDiskDevicesResp &item : disksInfo.m_diskDevices) {
        if (!ParseVolume(item, newVol, volName)) {
            continue;
        }
        return SUCCESS;
    }
    return FAILED;
}

bool CNwareProtectEngine::ParseVolume(const DomainDiskDevicesResp &item, VolInfo &newVol,
    const std::string &volName)
{
    DBGLOG("Enter");
    std::vector<std::string> sourceFileSpliteStrs;
    (void)boost::split(sourceFileSpliteStrs, item.m_sourceFile,
        boost::is_any_of("/"));
    if (sourceFileSpliteStrs.back().empty()) {
        WARNLOG("Get empty sourceFileSpliteStrs sourcefile: %s",
            item.m_sourceFile.c_str());
        return false;
    }
    if (sourceFileSpliteStrs.back() != volName) {
        DBGLOG("Get non assiciated sourceFileSpliteStrs vol: %s",
            sourceFileSpliteStrs.back().c_str());
        return false;
    }
    DBGLOG("Get assiciated sourceFileSpliteStrs vol: %s", sourceFileSpliteStrs.back().c_str());
    newVol.m_name = volName;
    newVol.m_uuid = item.m_volId;
    newVol.m_slotId = item.m_dev;
    newVol.m_datastore.m_name = item.m_storagePoolName;
    newVol.m_datastore.m_poolId = item.m_storagePoolId;
    newVol.m_datastore.m_type = std::to_string(item.m_storagePoolType);
    newVol.m_bootable = std::to_string(item.m_bootOrder);
    newVol.m_type = item.m_bus;
    newVol.m_volSizeInBytes = item.m_capacity;
    newVol.m_volumeType = std::to_string(item.m_cache);
    newVol.m_location = item.m_sourceFile;
    newVol.m_provisionType = item.m_preallocation;
    newVol.m_format = item.m_driverType;
    newVol.m_vmMoRef = m_domainId;
    newVol.m_newCreate = true;
    DomainDiskDevicesResp tmp = item;
    if (!Module::JsonHelper::StructToJsonString(tmp, newVol.m_metadata)) {
        ERRLOG("Convert DomainDiskDevicesResp to json string failed, %s", m_taskId.c_str());
        return false;
    }
    return true;
}

int32_t CNwareProtectEngine::DetachVolume(const VolInfo &volObj)
{
    INFOLOG("Enter");
    ApplicationLabelType labelParam;
    if (DoDetachVolume(volObj) != SUCCESS) {
        ERRLOG("DoDetachVolume failed. %s", m_jobId.c_str());
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNWARE_DETACH_VOLUME_FAILED_LABEL;
        labelParam.params = std::vector<std::string>{volObj.m_vmMoRef, volObj.m_name};
        labelParam.errCode = CNWARE_DETACH_VOLUME_FAILED_ERROR;
        labelParam.additionalDesc = std::vector<std::string>{ "Detach volume failed." };
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoDetachVolume(const VolInfo &volObj)
{
    INFOLOG("Enter");
    std::string volObjStr;
    VolInfo tmp = volObj;
    if (!Module::JsonHelper::StructToJsonString(tmp, volObjStr)) {
        ERRLOG("Convert volume info to json string failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (volObj.m_newCreate) {
        WARNLOG("New created vol: %s (id: %s), have been detached before.%s",
            volObj.m_name.c_str(), volObj.m_uuid.c_str(), m_jobId.c_str());
        return SUCCESS;
    }
    CNwareVMInfo cnwareVmInfo;
    if (!GetVMInfoById(volObj.m_vmMoRef, cnwareVmInfo)) {
        ERRLOG("Get VM info failed when do detach volume(%s), %s", volObj.m_uuid.c_str(), m_taskId.c_str());
        return false;
    }
    ApplicationLabelType labelParam;
    labelParam.label = CNWARE_DETACH_VOLUME_LABEL;
    labelParam.params = std::vector<std::string>{cnwareVmInfo.m_name, volObj.m_name};
    ReportJobDetail(labelParam);
    DBGLOG("Volume info: %s, Volume vm moref: %s", volObjStr.c_str(), volObj.m_vmMoRef.c_str());
    {
        std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
        if (!DoDetachVolumeOnVm(volObj.m_uuid, volObj.m_vmMoRef)) {
            ERRLOG("Detach volume on vm failed, %s", m_jobId.c_str());
            labelParam.label = CNWARE_DETACH_VOLUME_FAILED_LABEL;
            labelParam.level = JobLogLevel::TASK_LOG_ERROR;
            ReportJobDetail(labelParam);
            return FAILED;
        }
    }
    INFOLOG("Detach volume %s(id: %s) on vm %s success, %s",
        volObj.m_name.c_str(), volObj.m_uuid.c_str(), volObj.m_vmMoRef.c_str(), m_jobId.c_str());
    return SUCCESS;
}

bool CNwareProtectEngine::ModifyBootDisk(const VolInfo &volObj, const std::string &vmId)
{
    DomainDiskDevicesResp diskDevice;
    if (!Module::JsonHelper::JsonStringToStruct(volObj.m_metadata, diskDevice)) {
        ERRLOG("Volume metadata trans to diskDevice failed.");
        return FAILED;
    }
    Json::Value volExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(volObj.m_extendInfo, volExtendInfo)) {
        ERRLOG("JsonStringToJsonValue failed. volObj's extendInfo, %s", m_taskId.c_str());
        return FAILED;
    }
    UpdateDomainBootRequest UpdateReq;
    try {
        UpdateReq.bootType = volExtendInfo["bootType"].asInt();
    } catch (std::exception &e) {
        ERRLOG("Set bootType failed. Exception is %s.", e.what());
        return FAILED;
    }
    BootItem bootItem;
    bootItem.bus = diskDevice.m_bus;
    bootItem.dev = diskDevice.m_dev;
    UpdateReq.boots.emplace_back(bootItem);

    if (!ModifyDevBoots(vmId, UpdateReq)) {
        ERRLOG("Fail to modify vm(%s) dev boots, %s", vmId.c_str(), m_jobId.c_str());
    }

    INFOLOG("Modify vm boot order success, first boot device(%s)", volObj.m_uuid.c_str());
    return true;
}

int32_t CNwareProtectEngine::AttachVolume(const VolInfo &volObj)
{
    InitRepoHandler();
    std::string newVMMetaDataPath = m_cacheRepoPath + VirtPlugin::VIRT_PLUGIN_CACHE_ROOT + "new_vm.info";
    // 整机恢复，新建虚拟机不需要挂载磁盘到新虚拟机
    if (m_cacheRepoHandler->Exists(newVMMetaDataPath)) {
        INFOLOG("Create new vm, no need attach volume.");
        return SUCCESS;
    }

    std::string domainName = m_jobHandle->GetApp().name;
    ApplicationLabelType labelParam;
    labelParam.label = CNWARE_ATTACH_VOLUME_LABEL;
    labelParam.params = std::vector<std::string>{domainName, volObj.m_name};
    ReportJobDetail(labelParam);
    {
        std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
        if (DoAttachVolume(volObj) != SUCCESS) {
            ERRLOG("DoAttachVolume failed. %s", m_taskId.c_str());
            labelParam.level = JobLogLevel::TASK_LOG_ERROR;
            labelParam.label = CNWARE_ATTACH_VOLUME_FAILED_LABEL;
            labelParam.params = std::vector<std::string>{domainName, volObj.m_name};
            labelParam.errCode = CNWARE_ATTACH_VOLUME_FAILED_ERROR;
            labelParam.additionalDesc = std::vector<std::string>{ "Attach volume failed." };
            ReportJobDetail(labelParam);
            return FAILED;
        }
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoAttachVolume(const VolInfo &volObj)
{
    INFOLOG("Enter");
    std::string domainId = m_jobHandle->GetApp().id;
    AddDiskRequest req;
    SetCommonInfo(req);
    req.SetDomainId(domainId);
    AddDomainDiskDevicesReq reqDisk;
    reqDisk.oldPool = volObj.m_datastore.m_name;
    reqDisk.oldVol = volObj.m_name;
    reqDisk.source = CNWARE_ATTACH_VOLUME_EXIST_SOURCE;

    VolInfo srcVolInfo;
    VolInfo volInfo;
    if (FindSourceVol(volObj.m_uuid, srcVolInfo) != SUCCESS) {
        ERRLOG("Find volume(%s)'s source metadata failed. %s", volObj.m_uuid.c_str(), m_jobId.c_str());
        return FAILED;
    }
    if (LoadCopyVolumeMatadata(srcVolInfo.m_uuid, volInfo) != SUCCESS) {
        ERRLOG("Get VolumeMatadata %s info failed. %s", srcVolInfo.m_uuid.c_str(), m_jobId.c_str());
        return FAILED;
    }
    DomainDiskDevicesResp diskInfo;
    if (!Module::JsonHelper::JsonStringToStruct(volInfo.m_metadata, diskInfo)) {
        ERRLOG("Get new domain info failed.");
        return FAILED;
    }
    reqDisk.preallocation = GetPreallocation(volObj, diskInfo);
    reqDisk.bus = diskInfo.m_busType;
    reqDisk.cache = diskInfo.m_cache;
    reqDisk.ioHangTimeout = diskInfo.m_ioHangTimeout;
    reqDisk.shareable = diskInfo.m_shareable;
    reqDisk.type = diskInfo.m_driverType;
    req.SetDomainDiskDevices(reqDisk);
    std::shared_ptr<AddDiskResponse> response = m_cnwareClient->AddDisk(req);
    if (response == nullptr) {
        ERRLOG("Get disk info on storage failed, %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Attach volume %s on storage failed, %s", volObj.m_uuid.c_str(), m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Attach volume success, %s", m_jobId.c_str());
    if (volObj.m_bootable == CURRENT_BOOT_DISK && !ModifyBootDisk(volObj, domainId)) {
        WARNLOG("Fail to set boot disk, %s", m_jobId.c_str());
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::FormatCreateMachineParam(VMInfo &vmInfo, AddDomainRequest &domainInfo)
{
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, domainInfo)) {
        ERRLOG("Get new domain info failed. %s", m_jobId.c_str());
        return FAILED;
    }
    // 确保光驱没有挂载
    for (auto &cdrom : domainInfo.mCdromDevices) {
        cdrom.mIsMount = false;
    }
    if (GetNewVMMachineName(domainInfo.mName) != SUCCESS) {
        ERRLOG("Get new vm machine name failed. %s", m_jobId.c_str());
        return FAILED;
    }
    vmInfo.m_name = domainInfo.mName;

    INFOLOG("vm info metadata: %s", vmInfo.m_metadata.c_str());
    CNwareVMInfo cwVmInfo;
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, cwVmInfo)) {
        ERRLOG("Get new vm machine info failed.%s", m_jobId.c_str());
        return FAILED;
    }
    ConfigDomainInfo(cwVmInfo, domainInfo);
    SetOsVersion(domainInfo);
    if (m_application.subType == "CNwareHost") {
        domainInfo.mHostId = m_application.id;
    } else if (m_application.subType == "CNwareVm") {
        domainInfo.mHostId = m_application.parentId;
    } else {
        domainInfo.mHostId = "";
    }
    INFOLOG("Set vm host id: %s", domainInfo.mHostId.c_str());
    if (FormatDomainDiskDev(vmInfo, domainInfo) != SUCCESS) {
        ERRLOG("Format domain disk device info failed. %s", m_jobId.c_str());
        return FAILED;
    }

    if (FormatDomainInterface(domainInfo) != SUCCESS) {
        ERRLOG("Format domain disk device info failed. %s", m_jobId.c_str());
        return FAILED;
    }

    return SUCCESS;
}

void CNwareProtectEngine::SetOsVersion(AddDomainRequest &domainInfo)
{
    std::string version;
    if (GetCNwareVersion(version) != SUCCESS || !Utils::CompareVersion(version, VERSION_KYLIN_CHANGED)) {
        ERRLOG("Get CNware version failed or version no need to fix. %s", m_jobId.c_str());
        return;
    }
    if (Module::ConfigReader::getInt("CNwareConfig", "RestoreOsVersion") !=
        RESTORE_OS_VERSION) {
        WARNLOG("Not restore os version, will change osversion to %s.", OTHER_OS_VERSION.c_str());
        domainInfo.mOsVersion = OTHER_OS_VERSION;
        return;
    }
    DBGLOG("Kylin os-version(%s) and new type(%s).", domainInfo.mOsVersion.c_str(),
        KYLIN_OS_VERSION_9_3.c_str());
    if (domainInfo.mOsVersion.find(KYLIN_OS_VERSION_OLD) != std::string::npos) {
        INFOLOG("Change kylin os-version(%s) to new type(%s).", domainInfo.mOsVersion.c_str(),
            KYLIN_OS_VERSION_9_3.c_str());
        domainInfo.mOsVersion = KYLIN_OS_VERSION_9_3;
        return;
    }
    return;
}

void CNwareProtectEngine::ConfigDomainInfo(const CNwareVMInfo &cwVmInfo, AddDomainRequest &domainInfo)
{
    int index {0};
    for (const auto &bri : cwVmInfo.m_bridgeInterfaces) {
        domainInfo.mBridgeInterfaces.at(index).mQueues = bri.m_queues;
        DBGLOG("DomainInfo que %d, bri que %d",
            domainInfo.mBridgeInterfaces.at(index).mQueues, bri.m_queues);
        index++;
    }
    domainInfo.mMemorySize = cwVmInfo.m_memory.m_memory;
    domainInfo.mCpuInfo = cwVmInfo.m_cpu;
    m_targetVMCpuCurrent = cwVmInfo.m_cpu.m_current;
}

int32_t CNwareProtectEngine::FormatDomainInterface(AddDomainRequest &domainInfo)
{
    INFOLOG("Enter");
    if (m_restorePara == nullptr) {
        ERRLOG("Get m_restorePara pointer null. %s", m_jobId.c_str());
        return FAILED;
    }
    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(), m_jobId.c_str());
        return FAILED;
    }
    if (!jobAdvancePara.isMember("bridgeInterface")) {
        WARNLOG("jobAdvancePara has no bridgeInterface, %s", m_jobId.c_str());
        return SUCCESS;
    }
    InterfacePortPairList interfacePortPairList;
    if (!Module::JsonHelper::JsonStringToStruct(jobAdvancePara["bridgeInterface"].asString(),
        interfacePortPairList)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(
            jobAdvancePara["bridgeInterface"].asString()).c_str(), m_jobId.c_str());
        return FAILED;
    }
    if (domainInfo.mBridgeInterfaces.size() == 0) {
        ERRLOG("Empty interface info from copy");
        return FAILED;
    }
    AddBridgeInterfaceRequest InterfaceReq = domainInfo.mBridgeInterfaces[0];
    DBGLOG("Origin bridge mac id: %s", InterfaceReq.mMac.c_str());
    for (auto &pair : interfacePortPairList.m_detail) {
        DBGLOG("Target bridge mac id: %s", pair.m_bridge.m_mac.c_str());
        if (InterfaceReq.mMac == pair.m_bridge.m_mac) {
            DBGLOG("Find mac id: %s", pair.m_portGroup.m_id.c_str());
            InterfaceReq.mPortGroupId = pair.m_portGroup.m_id;
            break;
        }
    }
    domainInfo.mBridgeInterfaces.clear();
    domainInfo.mBridgeInterfaces.emplace_back(InterfaceReq);
    return SUCCESS;
}

int32_t CNwareProtectEngine::FormatLiveInterface(AddDomainRequest &domainInfo)
{
    INFOLOG("Enter");
    if (!m_jobAdvPara.isMember("bridgeInterface")) {
        WARNLOG("jobAdvancePara has no bridgeInterface, %s", m_jobId.c_str());
        return SUCCESS;
    }
    InterfacePortPairList interfacePortPairList;
    if (!Module::JsonHelper::JsonStringToStruct(m_jobAdvPara["bridgeInterface"].asString(),
        interfacePortPairList)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(
            m_jobAdvPara["bridgeInterface"].asString()).c_str(), m_jobId.c_str());
        return FAILED;
    }
    AddBridgeInterfaceRequest InterfaceReq = domainInfo.mBridgeInterfaces[0];
    DBGLOG("Origin bridge mac id: %s", InterfaceReq.mMac.c_str());
    for (auto &pair : interfacePortPairList.m_detail) {
        DBGLOG("Target bridge mac id: %s", pair.m_bridge.m_mac.c_str());
        if (InterfaceReq.mMac == pair.m_bridge.m_mac) {
            DBGLOG("Find mac id: %s", pair.m_portGroup.m_id.c_str());
            InterfaceReq.mPortGroupId = pair.m_portGroup.m_id;
            break;
        }
    }
    domainInfo.mBridgeInterfaces.clear();
    domainInfo.mBridgeInterfaces.emplace_back(InterfaceReq);
    return SUCCESS;
}

int32_t CNwareProtectEngine::FormatDomainDiskDev(VMInfo &vmInfo, AddDomainRequest &domainInfo)
{
    InitRepoHandler();
    std::string volPairPath = m_cacheRepoPath + VIRT_PLUGIN_VOL_MATCH_INFO;
    VolMatchPairInfo volPairList;
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, volPairPath,
        volPairList) != SUCCESS) {
        ERRLOG("Load vol pair failed, %s", m_taskId.c_str());
        return FAILED;
    }
    std::sort(volPairList.m_volPairList.begin(), volPairList.m_volPairList.end(), [](auto volX, auto volY)->bool {
        return (volX.m_originVol.m_slotId.compare(volY.m_originVol.m_slotId) < 0);
    });
    for (const VolPair &volPair : volPairList.m_volPairList) {
        DomainDiskDevicesReq diskDevReq;
        DomainDiskDevicesResp diskDev;
        if (!Module::JsonHelper::JsonStringToStruct(volPair.m_originVol.m_metadata, diskDev)) {
            ERRLOG("Get volume metadata failed.");
            return FAILED;
        }
        if (DISK_BUS_MAP.find(diskDev.m_bus) == DISK_BUS_MAP.end()) {
            ERRLOG("Unkown disk bus type %s", diskDev.m_bus.c_str());
            return FAILED;
        }
        diskDevReq.mBus = DISK_BUS_MAP[diskDev.m_bus];
        diskDevReq.mCache = diskDev.m_cache;
        diskDevReq.mIoHangTimeout = diskDev.m_ioHangTimeout;
        diskDevReq.mOldVol = volPair.m_targetVol.m_name;
        diskDevReq.mOldPool = volPair.m_targetVol.m_datastore.m_name;
        diskDevReq.mPreallocation = (
            Module::SafeStoi(volPair.m_targetVol.m_datastore.m_type) == static_cast<int>(PoolType::NAS) ||
            Module::SafeStoi(volPair.m_targetVol.m_datastore.m_type) == static_cast<int>(PoolType::Distribute)) ?
            PREALLOCATION_OFF : diskDev.m_preallocation;
        diskDevReq.mType = Module::SafeStoi(volPair.m_targetVol.m_datastore.m_type) ==
            static_cast<int32_t>(PoolType::Distribute) ? static_cast<int32_t>(DriverType::RAW) : diskDev.m_driverType;
        diskDevReq.mShareable = diskDev.m_shareable;
        diskDevReq.mSource = static_cast<int>(VolSource::OLD_DISK);
        domainInfo.mDiskDevices.emplace_back(diskDevReq);
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::FormatLiveDiskDev(VMInfo vmInfo, AddDomainRequest &domainInfo)
{
    std::sort(vmInfo.m_volList.begin(), vmInfo.m_volList.end(), [](auto volX, auto volY)->bool {
        return (volX.m_slotId.compare(volY.m_slotId) < 0);
    });
    for (const VolInfo &vol : vmInfo.m_volList) {
        DBGLOG("Find volume %s. %s.", vol.m_uuid.c_str(), m_jobId.c_str());
        if (m_volIdToReq.count(vol.m_uuid) != 0) {
            domainInfo.mDiskDevices.emplace_back(m_volIdToReq[vol.m_uuid]);
        } else {
            ERRLOG("Not find volume %s. %s.", vol.m_uuid.c_str(), m_jobId.c_str());
            return FAILED;
        }
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::DeleteVolume(const VolInfo &volObj)
{
    if (m_jobHandle->GetJobType() == JobType::RESTORE) {
        return SUCCESS;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::ReplaceVolume(const VolInfo &volObj)
{
    return 0;
}

bool CNwareProtectEngine::ModifyDevBoots(const string &vmId, const UpdateDomainBootRequest &UpdateReq)
{
    ModifyBootsRequest req;
    req.SetUpdateDomainBootRequest(UpdateReq);
    req.SetDomainId(vmId);
    SetCommonInfo(req);
    std::shared_ptr<CNwareResponse> response = m_cnwareClient->ModifyVMBoots(req);
    if (response == nullptr || !CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Modify vm boots failed.");
        return false;
    }
    return true;
}

bool CNwareProtectEngine::ModifyVMDevBoots(const DomainListResponse &domainInfo, VMInfo vmInfo)
{
    CNwareVMInfo cwVmInfo;
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, cwVmInfo)) {
        ERRLOG("Get new vm machine info failed.");
        return false;
    }
    InitRepoHandler();
    std::string volPairPath = m_cacheRepoPath + VIRT_PLUGIN_VOL_MATCH_INFO;
    VolMatchPairInfo volPairList;
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, volPairPath,
        volPairList) != SUCCESS) {
        ERRLOG("Load vol pair failed, %s", m_taskId.c_str());
        return false;
    }
    // 按照引导顺序升序排序
    std::sort(volPairList.m_volPairList.begin(), volPairList.m_volPairList.end(), [](auto volX, auto volY)->bool {
        return (std::stoi(volX.m_originVol.m_bootable) < std::stoi(volY.m_originVol.m_bootable));
    });
    UpdateDomainBootRequest UpdateReq;
    UpdateReq.bootType = cwVmInfo.m_bootType;
    for (const auto &volPair : volPairList.m_volPairList) {
        for (const DomainDiskDbRsp &diskInfo : domainInfo.domainDiskDbRspList) {
            if (diskInfo.fileName.find(volPair.m_targetVol.m_name) == std::string::npos) {
                continue;
            }
            BootItem bootItem;
            bootItem.bus = diskInfo.bus;
            bootItem.dev = diskInfo.dev;
            UpdateReq.boots.emplace_back(bootItem);
            break;
        }
    }
    if (!ModifyDevBoots(domainInfo.id, UpdateReq)) {
        ERRLOG("Fail to modify vm(%s) dev boots, %s", vmInfo.m_uuid.c_str(), m_jobId.c_str());
    }
    INFOLOG("Modify vm boot order success, first boot device.");
    return true;
}

bool CNwareProtectEngine::ModifyVMDevBootsForDiskRestore()
{
    VMInfo vmInfo;
    std::string targetVmInfoFile = m_cacheRepoPath + VIRT_PLUGIN_RESTORE_TARGET_VM_INFO;
    if (Utils::LoadFileToStruct(m_cacheRepoHandler, targetVmInfoFile, vmInfo) != SUCCESS) {
        ERRLOG("Load %s failed, %s", targetVmInfoFile.c_str(), m_jobId.c_str());
        return false;
    }
    CNwareVMInfo cwVmInfo;
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, cwVmInfo)) {
        ERRLOG("Get new vm machine info failed.");
        return false;
    }
    if (vmInfo.m_volList.empty()) {
        WARNLOG("No volume info in target vm info file.");
        return true;
    }
    // 按照引导顺序升序排序
    std::sort(vmInfo.m_volList.begin(), vmInfo.m_volList.end(), [](auto volX, auto volY)->bool {
        return (std::stoi(volX.m_bootable) < std::stoi(volY.m_bootable));
    });
    UpdateDomainBootRequest UpdateReq;
    UpdateReq.bootType = cwVmInfo.m_bootType;
    INFOLOG("vmInfo.m_volList size is %d", vmInfo.m_volList.size());
    for (const auto &diskInfo : vmInfo.m_volList) {
        BootItem bootItem;
        bootItem.bus = diskInfo.m_type;
        bootItem.dev = diskInfo.m_slotId;
        UpdateReq.boots.emplace_back(bootItem);
    }
    if (!ModifyDevBoots(vmInfo.m_moRef, UpdateReq)) {
        ERRLOG("Fail to modify vm(%s) dev boots, %s", vmInfo.m_uuid.c_str(), m_jobId.c_str());
        return false;
    }
    INFOLOG("Modify vm boot order success, first boot device.");
    return true;
}

bool CNwareProtectEngine::ModifyLiveDevBoots(const DomainListResponse &domainInfo, VMInfo vmInfo)
{
    // 按照引导顺序升序排序
    std::sort(vmInfo.m_volList.begin(), vmInfo.m_volList.end(), [](auto volX, auto volY)->bool {
        return (std::stoi(volX.m_bootable) < std::stoi(volY.m_bootable));
    });
    CNwareVMInfo cwVmInfo;
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, cwVmInfo)) {
        ERRLOG("Get new vm machine info failed.");
        return false;
    }
    UpdateDomainBootRequest UpdateReq;
    UpdateReq.bootType = cwVmInfo.m_bootType;
    for (const VolInfo &vol : vmInfo.m_volList) {
        for (const DomainDiskDbRsp &diskInfo : domainInfo.domainDiskDbRspList) {
            if (diskInfo.fileName.find(vol.m_uuid + ".raw") == std::string::npos) {
                continue;
            }
            BootItem bootItem;
            bootItem.bus = diskInfo.bus;
            bootItem.dev = diskInfo.dev;
            UpdateReq.boots.emplace_back(bootItem);
            break;
        }
    }
    if (!ModifyDevBoots(domainInfo.id, UpdateReq)) {
        ERRLOG("Fail to modify vm(%s) dev boots, %s", vmInfo.m_uuid.c_str(), m_jobId.c_str());
    }
    INFOLOG("Modify vm boot order success, first boot device");
    return true;
}

int32_t CNwareProtectEngine::CreateMachine(VMInfo &vmInfo)
{
    INFOLOG("Enter");
    // 入参为副本虚拟机信息，将id置零，防止创建失败后，误删原虚拟机
    vmInfo.m_uuid = "";
    BuildNewVMRequest createVmReq;
    SetCommonInfo(createVmReq);
    AddDomainRequest addDomainInfo;
    if (FormatCreateMachineParam(vmInfo, addDomainInfo) != SUCCESS) {
        ERRLOG("Format create machine param failed.");
        return FAILED;
    }
    createVmReq.SetDomainInfo(addDomainInfo);
    std::string hostName;
    if (m_application.subType == "CNwareHost") {
        hostName = m_application.name;
    } else if (m_application.subType == "CNwareVm") {
        hostName = m_application.parentName;
    } else {
        ERRLOG("Format subType param error!");
        hostName = "";
    }
    ApplicationLabelType labelParam;
    labelParam.label = CNWARE_CREATE_MACHINE_LABEL;
    labelParam.params = std::vector<std::string>{hostName, addDomainInfo.mName};
    ReportJobDetail(labelParam);

    if (DoCreateMachine(vmInfo, addDomainInfo, createVmReq) != SUCCESS) {
        ERRLOG("DoCreateMachine failed. %s", m_taskId.c_str());
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNWARE_CREATE_MACHINE_FAILED_LABEL;
        labelParam.params = std::vector<std::string>{addDomainInfo.mHostId, addDomainInfo.mName};
        labelParam.errCode = CNWARE_CREATE_MACHINE_FAILED_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }

    if (ProcessBridgeInterface(vmInfo.m_uuid, vmInfo) != SUCCESS) {
        ERRLOG("Process BridgeInterface on vm(%s). %s", vmInfo.m_uuid.c_str(), m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoAddNetworkCard(const std::string vmId, const AddBridgeInterfaceRequest &req)
{
    AddNetworkCardRequest NetworkCardReq;
    SetCommonInfo(NetworkCardReq);
    NetworkCardReq.SetDomainId(vmId);
    NetworkCardReq.SetAddBridgeInterfaceRequest(req);
    std::shared_ptr<CNwareResponse> response = m_cnwareClient->AddNetworkCard(NetworkCardReq);
    if (response == nullptr || !CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Add Network Card failed.");
        return FAILED;
    }
    INFOLOG("Add one network card success");
    return SUCCESS;
}

int32_t CNwareProtectEngine::AddBridgeInterface(const std::string &vmId,
    std::vector<AddBridgeInterfaceRequest> &InterfaceList)
{
    if (!m_jobAdvPara.isMember("bridgeInterface")) {
        ERRLOG("jobAdvancePara has no bridgeInterface, %s", m_jobId.c_str());
        return FAILED;
    }
    InterfacePortPairList interfacePortPairList;
    if (!Module::JsonHelper::JsonStringToStruct(m_jobAdvPara["bridgeInterface"].asString(),
        interfacePortPairList)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(
            m_jobAdvPara["bridgeInterface"].asString()).c_str(), m_jobId.c_str());
        return FAILED;
    }
    // Add other network card after creating vm to avoid a cnware bug
    if (InterfaceList.size() < NUM_2) {
        ERRLOG("number of network cards of the target vm is less than 2, no need to add");
        return FAILED;
    }
    std::vector<AddBridgeInterfaceRequest>::iterator it = InterfaceList.begin();
    for (it = it + 1; it != InterfaceList.end(); ++it) {
        DBGLOG("Origin bridge mac id: %s", it->mMac.c_str());
        for (auto &pair : interfacePortPairList.m_detail) {
            DBGLOG("Target bridge mac id: %s", pair.m_bridge.m_mac.c_str());
            if (it->mMac == pair.m_bridge.m_mac) {
                DBGLOG("Find mac id: %s", pair.m_portGroup.m_id.c_str());
                it->mPortGroupId = pair.m_portGroup.m_id;
                break;
            }
        }
        if (DoAddNetworkCard(vmId, *it) != SUCCESS) {
            ERRLOG("Add network card failed");
            return FAILED;
        }
    }
    INFOLOG("Add all network card success");
    return SUCCESS;
}

void CNwareProtectEngine::WaitVMTaskEmpty(const std::string &domainId)
{
    if (m_cnwareClient == nullptr) {
        ERRLOG("WaitVMTaskEmpty cnwareClient nullptr! Job id: %s", m_jobId.c_str());
        return;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    std::shared_ptr<QueryVMTaskResponse> response = std::make_shared<QueryVMTaskResponse>();
    int wait_time = 0;
    do {
        sleep(COMMON_WAIT_TIME_3S);
        wait_time += COMMON_WAIT_TIME_3S;
        response = m_cnwareClient->QueryVMTasks(req, domainId, STATUS_BEGINNING_RUNNING, PAGE_1, NUM_100);
        if (response == nullptr) {
            ERRLOG("WaitVMTaskEmpty failed! domain id: %s.", domainId.c_str());
            return;
        }
        if (FIFTEEN_MIN_IN_SEC < wait_time) {
            ERRLOG("WaitVMTaskEmpty timeout! domain id: %s.", domainId.c_str());
            break;
        }
    } while (!response->GetVMTaskInfo().mData.empty());
    INFOLOG("Task empty! domain id: %s.", domainId.c_str());
    return;
}

int32_t CNwareProtectEngine::PostProcessCreateMachine(VMInfo &vmInfo)
{
    // 通过虚拟机名称查询新虚拟机信息
    GetVMListRequest getVMListReq;
    SetCommonInfo(getVMListReq);
    getVMListReq.SetQueryName(vmInfo.m_name);
    std::shared_ptr<GetVMListResponse> getVMListRsp = m_cnwareClient->GetVMList(getVMListReq);
    if (getVMListRsp == nullptr) {
        ERRLOG("Get vm %s info by name failed. %s", vmInfo.m_name.c_str(), m_jobId.c_str());
        return FAILED;
    }
    DataResponse listRsp = getVMListRsp->GetVMList();
    if (listRsp.data.empty()) {
        ERRLOG("Get vm info by name failed, no info return.");
        return FAILED;
    }
    DomainListResponse dInfo = listRsp.data[0];
    
    sleep(COMMON_WAIT_TIME_3S);
    WaitVMTaskEmpty(dInfo.id);
    if (!ModifyVMDevBoots(dInfo, vmInfo)) {
        ERRLOG("Modify VM(%s) dev boots failed. %s", dInfo.id.c_str(), m_jobId.c_str());
        return FAILED;
    }
    vmInfo.m_uuid = dInfo.id;
    // 清除卷信息，查询新创建的卷重新赋值
    vmInfo.m_volList.clear();

    GetVMDiskInfoRequest reqVol;
    SetCommonInfo(reqVol);
    reqVol.SetDomainId(vmInfo.m_uuid);
    std::shared_ptr<GetVMDiskInfoResponse> respVol = m_cnwareClient->GetVMDiskInfo(reqVol);
    if (respVol == nullptr || !SetVolListInfo2VMInfo(respVol->GetInfo(), vmInfo)) {
        ERRLOG("Get VM vol list info failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}
int32_t CNwareProtectEngine::DoCreateMachine(VMInfo &vmInfo,
    const AddDomainRequest &addDomainInfo, BuildNewVMRequest &createVmReq)
{
    INFOLOG("Enter");
    std::shared_ptr<BuildNewVMResponse> response = m_cnwareClient->BuildNewClient(createVmReq);
    if (response == nullptr || !CheckTaskStatus(response->GetTaskId())) {
        if (CNWARE_LABEL_DICT.count(m_cnwareClient->GetErrorCode().m_errorCode) != 0) {
            m_reportArgs = { addDomainInfo.mName };
            m_reportParam = {
                CNWARE_LABEL_DICT[m_cnwareClient->GetErrorCode().m_errorCode],
                JobLogLevel::TASK_LOG_ERROR,
                SubJobStatus::FAILED, 0, 0 };
        }
        ERRLOG("Create vm failed.");
        return FAILED;
    }
    if (PostProcessCreateMachine(vmInfo) != SUCCESS) {
        ERRLOG("PostProcessCreateMachine vm failed. %s.", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Create vm %s id: %s success. %s", vmInfo.m_name.c_str(), vmInfo.m_uuid.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetNewVMMachineName(std::string &vmName)
{
    Json::Value jobAdvancePara;
    if (m_restorePara == nullptr) {
        ERRLOG("Convert m_restorePara nullptr failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert m_restorePara extendInfo failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (jobAdvancePara.isMember("vmName")) {
        vmName = jobAdvancePara["vmName"].asString();
    }
    if (vmName.empty()) {
        ERRLOG("VM machine name provided is empty. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Get vm machine name: %s", vmName.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetNewVMMachineInfo()
{
    INFOLOG("Enter");
    DBGLOG("APP %s", m_appEnv.id.c_str());
    if (m_jobHandle->GetJobType() == JobType::LIVEMOUNT) {
        INFOLOG("Enter LIVEMOUNT");
        ConfigLivemount config;
        if (!Module::JsonHelper::JsonValueToStruct(m_jobAdvPara, config)) {
            ERRLOG("Convert LIVEMOUNT m_jobAdvPara failed, %s", m_taskId.c_str());
            return FAILED;
        }
        m_config = config;
    } else if (m_jobHandle->GetJobType() == JobType::RESTORE ||
        m_jobHandle->GetJobType() == JobType::INSTANT_RESTORE) {
        INFOLOG("Enter GetJobType INSTANT_RESTORE: %d", static_cast<int32_t>(m_jobHandle->GetJobType()));
        if (!m_jobAdvPara.isMember("vmName") || !m_jobAdvPara.isMember("powerState")) {
            ERRLOG("Convert INSTANT_RESTORE m_jobAdvPara failed, %s", m_taskId.c_str());
            return FAILED;
        }
        m_config.m_name = m_jobAdvPara["vmName"].asString();
        m_config.m_powerOn = m_jobAdvPara["powerState"].asString() == "0" ? false : true;
    }
    INFOLOG("GetNewVMMachineInfo success: %s, %s", m_config.m_name.c_str(), m_jobId.c_str());
    return SUCCESS;
}

bool CNwareProtectEngine::CheckHostBeforeRecover(const VMInfo &vmObj)
{
    ApplicationLabelType hostCheckLabel;
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
        hostCheckLabel.level = JobLogLevel::TASK_LOG_ERROR;
        hostCheckLabel.label = CNWARE_HOST_CHECK_FAILED_LABEL;
        hostCheckLabel.params = std::vector<std::string>{hostId, HOST_STATUS_MAP[statusFlag]};
        ReportJobDetail(hostCheckLabel);
        return false;
    }

    hostCheckLabel.label = CNWARE_ARCH_CHECK_LABEL;
    hostCheckLabel.params = std::vector<std::string>{};
    ReportJobDetail(hostCheckLabel);
    std::string copyArch;
    std::string targetArch;
    if (CheckHostArchitectures(vmObj, copyArch, targetArch) != SUCCESS) {
        ERRLOG("Check host architectures failed.");
        hostCheckLabel.level = JobLogLevel::TASK_LOG_ERROR;
        hostCheckLabel.label = CNWARE_ARCH_CHECK_FAILED_LABEL;
        hostCheckLabel.params = std::vector<std::string>{copyArch, targetArch};
        hostCheckLabel.errCode = CNWARE_ARCH_CHECK_FAILED_ERROR;
        ReportJobDetail(hostCheckLabel);
        return false;
    }
    return true;
}

bool CNwareProtectEngine::CheckVMNameValidity(const std::string &vmName)
{
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM ||
        m_jobHandle->GetJobType() == JobType::LIVEMOUNT) {
        static const std::regex e("^[\u4e00-\u9fa5a-zA-Z0-9_\\-\\.]{1,384}$");
        if (vmName.empty() || Utils::GetUtf8CharNumber(vmName) > CNWARE_VM_NAME_LIMIT ||
            !std::regex_match(vmName, e)) {
            ERRLOG("Invalid vm name: %s", vmName.c_str());
            return false;
        }
    }
    return true;
}

int32_t CNwareProtectEngine::CheckBeforeRecover(const VMInfo &vmObj)
{
    INFOLOG("Enter");
    if (m_restorePara == nullptr) {
        if (!InitJobPara()) {
            ERRLOG("InitJobPara, %s", m_taskId.c_str());
            return FAILED;
        }
    }
    ApplicationLabelType labelParam;
    labelParam.label = CNWARE_RESTORE_PRECHECK_LABEL;
    ReportJobDetail(labelParam);

    if (!CheckHostBeforeRecover(vmObj)) {
        ERRLOG("Check host before recover failed. %s", m_jobId.c_str());
        return FAILED;
    }

    std::string vmName;
    GetNewVMMachineName(vmName);
    if (!CheckVMNameValidity(vmName) || CheckVMNameUnique(vmName) != SUCCESS) {
        ERRLOG("Check vm name failed. %s", m_jobId.c_str());
        return FAILED;
    }

    if (!CheckVMStatus()) {
        ERRLOG("Check VM Status Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (CheckStoragePoolRestore(m_restorePara->restoreSubObjects) != SUCCESS) {
        ERRLOG("Check storage pool failed.");
        ApplicationLabelType labelParam;
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNWARE_STORAGEPOOL_FAILED_LABEL;
        ReportJobDetail(labelParam);
        return FAILED;
    }

    INFOLOG("CheckBeforeRecover success.");
    return SUCCESS;
}

int32_t CNwareProtectEngine::CheckStoragePoolRestore(
    const std::vector<ApplicationResource> &restoreSubObjects)
{
    INFOLOG("Enter");
    // 检查目标位置存储连通性
    // 获取-目标存储池ID(列表), <volName, poolId, poolName>
    std::set<std::tuple<std::string, std::string, std::string>> poolList;
    for (auto restoreVol : restoreSubObjects) {
        std::transform(restoreVol.id.begin(), restoreVol.id.end(), restoreVol.id.begin(), ::tolower);
        if (GetTargetPoolIDList(restoreVol, poolList) != SUCCESS) {
            ERRLOG("Get target pool id list failed.");
            return FAILED;
        }
    }

    // 查询代理主机是否联通目标存储池
    std::string storeLists;
    std::vector<std::string> noTasksArgs {};
    int32_t iRet {SUCCESS};
    for (auto [ volName, id, name ] : poolList) {
        ApplicationLabelType labelParam;
        labelParam.label = CNWARE_STORAGEPOOL_CHECK_LABEL;
        std::string poolName;
        GetStorageName(id, poolName);
        labelParam.params = std::vector<std::string>{volName, poolName};
        ReportJobDetail(labelParam);

        StoragePoolInfo poolInfo;
        if (QueryAgentHostStoragePoolList(poolInfo, id) != SUCCESS ||
            poolInfo.m_total == 0 || (poolInfo.m_data.at(0).m_status != static_cast<int32_t>(PoolStatus::USED))) {
            ERRLOG("Check storage pool connection failed, volume: %s, pool: %s, %s",
                volName.c_str(), id.c_str(), m_jobId.c_str());
            labelParam.level = JobLogLevel::TASK_LOG_WARNING;
            labelParam.label = CNWARE_STORAGEPOOL_NOTAVAILABLE_LABEL;
            labelParam.params = std::vector<std::string>{volName, poolName};
            labelParam.errCode = CNWARE_STORAGEPOOL_NOTAVAILABLE_ERROR;
            ReportJobDetail(labelParam);
            std::string name;
            storeLists = GetStorageName(id, name) ? (storeLists + name + ",") : storeLists;
            iRet = FAILED;
        }
    }
    if (!storeLists.empty()) {
        storeLists.pop_back();
        noTasksArgs = std::vector<std::string> {GetHostName(), storeLists};
        SetNoTasksArgs(noTasksArgs);
        return FAILED;
    }
    INFOLOG("Check storage pool for restore success.");
    return iRet;
}

bool CNwareProtectEngine::GetStorageName(const std::string &poolId, std::string &poolName)
{
    INFOLOG("Enter");
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("GetStorage m_cnwareClient pointer failed.");
        return false;
    }
    std::shared_ptr<StoragePoolExResponse> response = m_cnwareClient->GetStoragePool(req, poolId);
    if (response == nullptr) {
        ERRLOG("GetStorage(%s) failed.", poolId.c_str());
        return false;
    }
    StoragePoolExInfo poolInfo = response->GetStoragePoolInfo();
    if (poolInfo.m_data.m_name.empty()) {
        ERRLOG("GetStorage(%s) name null failed.", poolId.c_str());
        return false;
    }
    poolName = poolInfo.m_data.m_name;
    return true;
}

bool CNwareProtectEngine::GetHostName(const std::string &hostId, std::string &hostName)
{
    INFOLOG("Enter");
    CNwareRequest req;
    SetCommonInfo(req);
    if (m_cnwareClient == nullptr) {
        ERRLOG("GetHostName m_cnwareClient pointer failed.");
        return false;
    }
    std::shared_ptr<GetHostInfoResponse> response = m_cnwareClient->GetHostInfo(req, hostId);
    if (response == nullptr) {
        ERRLOG("GetHostName(%s) failed.", hostId.c_str());
        return false;
    }
    HostInfo hostInfo = response->GetHostInfo();
    if (hostInfo.name.empty()) {
        ERRLOG("GetHostName(%s) name null failed.", hostId.c_str());
        return false;
    }
    hostName = hostInfo.name;
    return true;
}

int32_t CNwareProtectEngine::FindTargetPool(
    const std::string &volName, const std::string &poolId, const StoragePoolInfo &poolInfo)
{
    for (const StoragePool &pool : poolInfo.m_data) {
        if (pool.m_id == poolId) {
            INFOLOG("Check volume %s storage pool %s connection success, %s",
                volName.c_str(), poolId.c_str(), m_jobId.c_str());
            return SUCCESS;
        }
    }
    ERRLOG("Cannot find target storage pool. %s", m_jobId.c_str());
    return FAILED;
}

int32_t CNwareProtectEngine::GetTargetPoolIDList(const ApplicationResource &targetVol,
    std::set<std::tuple<std::string, std::string, std::string>> &poolList)
{
    INFOLOG("Enter");
    Json::Value targetVolume;
    if (InitParaAndGetTargetVolume(targetVol, targetVolume) != SUCCESS) {
        ERRLOG("Get target volume info failed, %s", m_jobId.c_str());
        return FAILED;
    }

    std::string volId;
    std::string volName;
    std::string isNewDisk;
    bool existingDiskRestore = false;
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_DISK) {
        if (!targetVolume.isMember("uuid") || !targetVolume.isMember("name") || !targetVolume.isMember("isNewDisk")) {
            ERRLOG("Invalid param, no target volume 'uuid', 'name' or 'isNewDisk' provided.");
            return FAILED;
        }
        volId = targetVolume["uuid"].asString();
        volName = targetVolume["name"].asString();
        isNewDisk = targetVolume["isNewDisk"].asString();
        // 磁盘恢复 - 恢复到已有磁盘
        if (isNewDisk == "false" && !volId.empty()) {
            INFOLOG("Existing disk restore.");
            existingDiskRestore = true;
        }
    }
    DatastoreInfo storage;
    if (GetTargetVolumeDatastoreParam(targetVolume, storage, existingDiskRestore) != SUCCESS) {
        ERRLOG("GetTargetVolumeDatastoreParam failed, %s", m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("Restore level %d", static_cast<int32_t>(m_restoreLevel));
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM || isNewDisk == "true") {
        poolList.insert(std::make_tuple(targetVol.name, storage.m_poolId, storage.m_name));
        INFOLOG("Target volume %s storage pool id: %s", targetVol.id.c_str(), storage.m_poolId.c_str());
    } else {
        CNwareDiskInfo volumeDetail;
        if (!GetVolInfoOnStorage(volId, volumeDetail)) {
            ERRLOG("Get volume info on storage failed, %s", m_jobId.c_str());
            return FAILED;
        }
        std::string poolId = volumeDetail.mStoragePoolId;
        std::string poolName = GetNameFromSourcefile(volumeDetail.m_path);
        poolList.insert(std::make_tuple(volName, poolId, poolName));
        INFOLOG("Target volume %s storage pool id: %s", volName.c_str(), poolId.c_str());
    }
    INFOLOG("Get target pool id list success.");
    return SUCCESS;
}

int32_t CNwareProtectEngine::GetTargetVolumeDatastoreParam(
    const Json::Value &targetVolume, DatastoreInfo &storage, bool existingDiskRestore)
{
    // 整机恢复和新键盘恢复，任务参数都会带datastore信息
    // 使用已有磁盘进行磁盘恢复，任务参数不带datastore信息
    INFOLOG("Enter");
    std::string volId = targetVolume["uuid"].asString();
    // 使用已有磁盘恢复，从目标虚拟机中获取目标已有磁盘datastore信息
    if (existingDiskRestore) {
        // 查询目标虚拟机信息
        VMInfo vmInfo;
        vmInfo.m_uuid = m_application.id;
        GetVMDiskInfoRequest reqVol;
        SetCommonInfo(reqVol);
        reqVol.SetDomainId(vmInfo.m_uuid);
        std::shared_ptr<GetVMDiskInfoResponse> respVol = m_cnwareClient->GetVMDiskInfo(reqVol);
        if (respVol == nullptr || !SetVolListInfo2VMInfo(respVol->GetInfo(), vmInfo)) {
            ERRLOG("Get VM vol list info failed, %s", m_taskId.c_str());
            return FAILED;
        }

        // 匹配磁盘信息
        for (const VolInfo &vol : vmInfo.m_volList) {
            if (vol.m_uuid == volId) {
                // 获取datastore
                storage = vol.m_datastore;
                INFOLOG("Get datastore from target vm disk list success.");
                return SUCCESS;
            }
        }
        ERRLOG("No disk found from the copy.");
        m_checkErrorCode = CNwareErrorCode::CNWARE_DISK_NOT_FOUND;
        return FAILED;
    }

    if (!targetVolume.isMember("datastore")) {
        ERRLOG("No datastore provided.");
        return FAILED;
    }
    Json::Value dsJsonValue = targetVolume["datastore"];
    if (!Module::JsonHelper::JsonValueToStruct(dsJsonValue, storage)) {
        ERRLOG("Failed to convert storagePool to Struct datastore, taskID: %s",
            m_jobId.c_str());
        return FAILED;
    }
    if (storage.m_name.empty() || storage.m_poolId.empty()) {
        WARNLOG("No datastore info provided. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Get datastore from request parameter(targetVolume) success.");
    return SUCCESS;
}

int32_t CNwareProtectEngine::CheckVMNameUnique(const std::string &vmName)
{
    DBGLOG("Enter");
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM ||
        m_jobHandle->GetJobType() == JobType::LIVEMOUNT ||
        m_jobHandle->GetJobType() == JobType::INSTANT_RESTORE) {
        ApplicationLabelType labelParam;
        labelParam.label = CNWARE_VMNAME_CHECK_LABEL;
        labelParam.params = std::vector<std::string>{vmName};
        ReportJobDetail(labelParam);

        CheckNameUniqueRequest checkVmNameReq;
        SetCommonInfo(checkVmNameReq);
        checkVmNameReq.SetName(vmName);
        std::shared_ptr<CheckNameUniqueResponse> response = m_cnwareClient->CheckNameUnique(checkVmNameReq);
        if (response == nullptr) {
            ERRLOG("Check vm unique name failed.");
            return FAILED;
        }
        ResOfNameCheck result = response->GetDevices();
        if (!result.mName) {
            ERRLOG("New vm name %s exists.", vmName.c_str());
            ApplicationLabelType labelParam;
            labelParam.level = JobLogLevel::TASK_LOG_ERROR;
            labelParam.label = CNWARE_VMNAME_NOTUNIQUE_LABEL;
            labelParam.params = std::vector<std::string>{vmName};
            labelParam.errCode = CNWARE_VMNAME_CHECK_ERROR;
            ReportJobDetail(labelParam);
            return FAILED;
        }
    }
    DBGLOG("Check vm name(%s) unique success. %s", vmName.c_str(), m_jobId.c_str());
    return SUCCESS;
}

std::string CNwareProtectEngine::GetNameFromSourcefile(std::string sourceName)
{
    std::vector<std::string> sourceFileSpliteStrs;
    (void)boost::split(sourceFileSpliteStrs, sourceName, boost::is_any_of("/"));
    if (sourceFileSpliteStrs.back().empty()) {
        WARNLOG("Get empty sourceFileSpliteStrs sourcefile: %s",
            sourceName.c_str());
        return "";
    }
    return sourceFileSpliteStrs.back();
}

bool CNwareProtectEngine::CheckHostResourceUsage(const std::string &hostId)
{
    CNwareRequest req;
    SetCommonInfo(req);
    std::shared_ptr<GetHostResourceStatResponse> response =
        m_cnwareClient->GetHostResourceStat(req, hostId);
    if (response == nullptr) {
        ERRLOG("CheckHostResourceUsage failed! host id: %s", hostId.c_str());
        return false;
    }
    uint32_t cpuLimit = Module::ConfigReader::getUint("CNwareConfig", "CpuLimit");
    uint32_t MemoryLimit = Module::ConfigReader::getUint("CNwareConfig", "MemoryLimit");
    HostResource hostResourceInfo = response->GetHostResourceInfo();
    ApplicationLabelType resourceCheckLabel;
    resourceCheckLabel.level = JobLogLevel::TASK_LOG_ERROR;

    if (uint32_t(hostResourceInfo.cpuRate) > cpuLimit) {
        ERRLOG("Target host cpu usage(%lf) is over limit (%d)! host id: %s",
            hostResourceInfo.cpuRate, cpuLimit, hostId.c_str());
        resourceCheckLabel.label = CNWARE_CHECK_CPU_USAGE_FAILED_LABEL;
        resourceCheckLabel.params = std::vector<std::string>{to_string(cpuLimit)};
        ReportJobDetail(resourceCheckLabel);
        return false;
    }
    if (uint32_t(hostResourceInfo.memoryRate) > MemoryLimit) {
        ERRLOG("Target host memory usage(%lf) is over limit (%d)! host id: %s",
            hostResourceInfo.memoryRate, MemoryLimit, hostId.c_str());
        resourceCheckLabel.label = CNWARE_CHECK_MEMORY_USAGE_FAILED_LABEL;
        resourceCheckLabel.params = std::vector<std::string>{to_string(MemoryLimit)};
        ReportJobDetail(resourceCheckLabel);
        return false;
    }
    return true;
}

bool CNwareProtectEngine::GetStoragePoolOfSpecifiedVol(const std::string volDev,
    std::shared_ptr<GetVMDiskInfoResponse> &resp, std::unordered_set<std::string> &poolList)
{
    for (auto disk : resp->GetInfo().m_diskDevices) {
        if (disk.m_dev != volDev) {
            continue;
        }
        poolList.insert(disk.m_storagePoolId);
        return true;
    }
    ERRLOG("No specified disk device(%s) on target vm", volDev.c_str());
    return false;
}
bool CNwareProtectEngine::GetTargetStoragePoolListBackup(std::unordered_set<std::string> &poolList)
{
    GetVMDiskInfoRequest req;
    SetCommonInfo(req);
    req.SetDomainId(m_backupPara->protectObject.id);
    std::shared_ptr<GetVMDiskInfoResponse> resp = m_cnwareClient->GetVMDiskInfo(req);
    if (m_backupPara->protectSubObject.size() == 0) {
        for (auto disk : resp->GetInfo().m_diskDevices) {
            poolList.insert(disk.m_storagePoolId);
        }
    } else {
        for (const auto &subObject : m_backupPara->protectSubObject) {
            if (!GetStoragePoolOfSpecifiedVol(subObject.name, resp, poolList)) {
                ERRLOG("GetStoragePoolOfSpecifiedVol failed");
                return false;
            }
        }
    }
    return true;
}

bool CNwareProtectEngine::CheckStorageUsage(const StoragePool &pool, const int32_t &storageLimit)
{
    if ((double(pool.m_available) / double(pool.m_capacity)) * NUM_100 <
        (double(storageLimit))) {
        ERRLOG("storage pool(%s) usage is over limit(%d).", pool.m_name.c_str(), storageLimit);
        ApplicationLabelType resourceCheckLabel;
        resourceCheckLabel.level = JobLogLevel::TASK_LOG_ERROR;
        resourceCheckLabel.label = CNWARE_CHECK_STORAGE_USAGE_FAILED_LABEL;
        resourceCheckLabel.params = std::vector<std::string>{pool.m_name, to_string(storageLimit)};
        ReportJobDetail(resourceCheckLabel);
        return false;
    }
    return true;
}

int32_t CNwareProtectEngine::GetStorageLimit()
{
    int32_t defaultStorageLimit = Module::ConfigReader::getInt("CNwareConfig", "StorageLimit");
    if (m_backupPara != nullptr && !m_backupPara->extendInfo.empty()) {
        Json::Value extendInfo;
        if (!Module::JsonHelper::JsonStringToJsonValue(m_backupPara->extendInfo, extendInfo)) {
            ERRLOG("Trans job extend info to json value failed");
            return defaultStorageLimit;
        }
        if (extendInfo.isMember("available_capacity_threshold")) {
            DBGLOG("Get available_capacity_threshold.");
            int32_t capacity = extendInfo["available_capacity_threshold"].asString().empty() ?
                defaultStorageLimit : Module::SafeStoi(extendInfo["available_capacity_threshold"].asString(),
                defaultStorageLimit);
            INFOLOG("Get available_capacity_threshold %d.", capacity);
            return capacity;
        }
    }
    ERRLOG("Get no backup ptr, return default.");
    return defaultStorageLimit;
}

bool CNwareProtectEngine::CheckStorage()
{
    std::unordered_set<std::string> poolList;
    if (!GetTargetStoragePoolListBackup(poolList)) {
        ERRLOG("Get target storage pool list Failed, %s", m_taskId.c_str());
        return false;
    }
    int32_t storageLimit = GetStorageLimit();
    std::string unUsedPools {};
    for (auto poolId : poolList) {
        CNwareRequest req;
        SetCommonInfo(req);
        if (m_cnwareClient == nullptr) {
            ERRLOG("GetStorage m_cnwareClient pointer failed.");
            return false;
        }
        std::shared_ptr<StoragePoolExResponse> response = m_cnwareClient->GetStoragePool(req, poolId);
        if (response == nullptr) {
            ERRLOG("GetStorage(%s) failed.", poolId.c_str());
            return false;
        }
        StoragePoolExInfo poolInfo = response->GetStoragePoolInfo();
        if (poolInfo.m_data.m_capacity == 0 ||
            poolInfo.m_data.m_status != static_cast<int32_t>(PoolStatus::USED) ||
            !CheckStorageUsage(poolInfo.m_data, storageLimit)) {
            ERRLOG("storage pool(%s) is unable to use. Status: %d. Capacity: %lld", poolInfo.m_data.m_name.c_str(),
                poolInfo.m_data.m_status, poolInfo.m_data.m_capacity);
            unUsedPools = unUsedPools + poolInfo.m_data.m_name + ",";
        }
    }
    if (!unUsedPools.empty()) {
        unUsedPools.pop_back();
        ApplicationLabelType storageLabel;
        storageLabel.level = JobLogLevel::TASK_LOG_ERROR;
        storageLabel.label = CNWARE_STORAGE_STATUS_UNNORMAL_LABEL;
        storageLabel.params = std::vector<std::string>{unUsedPools};
        ReportJobDetail(storageLabel);
        return false;
    }
    return true;
}

int32_t CNwareProtectEngine::CheckResourceUsage(const std::string &resourceId)
{
    INFOLOG("Checking resource usage...");
    if (!CheckHostResourceUsage(resourceId)) {
        ERRLOG("Check host resouce usage Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!CheckStorage()) {
        ERRLOG("Check storage usage Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::CheckBeforeBackup()
{
    ApplicationLabelType hostCheckLabel;
    hostCheckLabel.label = CNWARE_BACKUP_CHECK_LABEL;
    hostCheckLabel.params = std::vector<std::string>{};
    ReportJobDetail(hostCheckLabel);
    std::string hostId;
    if (m_application.subType == "CNwareHost") {
        hostId = m_application.id;
    } else if (m_application.subType == "CNwareVm") {
        hostId = m_application.parentId;
    } else {
        hostId = "";
    }
    int32_t statusFlag {0};
    if (!CheckHostStatus(statusFlag, hostId)) {
        ERRLOG("Check host Status Failed, %s", m_taskId.c_str());
        hostCheckLabel.level = JobLogLevel::TASK_LOG_ERROR;
        hostCheckLabel.label = CNWARE_HOST_CHECK_FAILED_LABEL;
        hostCheckLabel.params = std::vector<std::string>{hostId, HOST_STATUS_MAP[statusFlag]};
        ReportJobDetail(hostCheckLabel);
        return FAILED;
    }
    if (!CheckVMStatus()) {
        ERRLOG("Check VM Status Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!CheckDisk()) {
        ERRLOG("Check disk type Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (CheckResourceUsage(hostId) != SUCCESS) {
        ERRLOG("Check resouce usage Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool CNwareProtectEngine::GetReserveCopyOption(bool &isReserved)
{
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_cancelLivemountPara->extendInfo, extendInfo)) {
        ERRLOG("Convert extendInfo failed, %s", m_taskId.c_str());
        return false;
    }
    if (!extendInfo.isMember("reserveCopy")) {
        ERRLOG("No reserveCopy provided.");
        return false;
    }
    if (extendInfo["reserveCopy"].asBool()) {
        isReserved = true;
    }
    isReserved = false;
    return true;
}

DeleteVMType CNwareProtectEngine::GetDeleteVMType()
{
    return DeleteVMType::DELETE_VM_KEEP_VOLUMES;
}

int32_t CNwareProtectEngine::DeleteMachine(const VMInfo &vmInfo)
{
    if (vmInfo.m_uuid.empty()) {
        DBGLOG("No need to delete VM.");
        return SUCCESS;
    }
    if (DoPowerOffMachine(vmInfo) != SUCCESS) {
        ERRLOG("PowerOffMachine(%s) before delete failed, %s", vmInfo.m_name.c_str(),
            m_taskId.c_str());
        return FAILED;
    }
    ApplicationLabelType labelParam;
    labelParam.params = std::vector<std::string>{vmInfo.m_locationName, vmInfo.m_name};
    labelParam.label = CNWARE_DELETE_MACHINE_LABEL;
    ReportJobDetail(labelParam);

    if (DoDeleteMachine(vmInfo) != SUCCESS) {
        labelParam.label = CNWARE_DELETE_MACHINE_FAILED_LABEL;
        labelParam.level = JobLogLevel::type::TASK_LOG_WARNING;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    INFOLOG("Delete vm %s success. %s", vmInfo.m_name.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoDeleteMachine(const VMInfo &vmInfo)
{
    DeleteVMRequest delVmReq;
    SetCommonInfo(delVmReq);
    delVmReq.SetDomainId(vmInfo.m_uuid);
    delVmReq.SetDeleteType(GetDeleteVMType());
    std::shared_ptr<CNwareResponse> response = m_cnwareClient->DeleteVM(delVmReq);
    if (response == nullptr) {
        ERRLOG("Delete vm failed. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Query DeleteMachine task status failed.");
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::RenameMachine(const VMInfo &vmInfo, const std::string &newName)
{
    if (newName.empty()) {
        WARNLOG("Empty name, exit. %s", m_jobId.c_str());
        return FAILED;
    }
    CNwareRequest req;
    SetCommonInfo(req);
    {
        Json::Value baseInfo;
        baseInfo["name"] = newName;
        Json::FastWriter writer;
        std::string bodyStr = writer.write(baseInfo);
        req.BuildBody(bodyStr);
    }
    std::shared_ptr<CNwareResponse> response = m_cnwareClient->ModifyBaseInfo(req, newName, vmInfo.m_uuid);
    if (response == nullptr) {
        ERRLOG("RenameMachine(%s) failed. %s", newName.c_str(), m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Query RenameMachine task status failed. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("RenameMachine vm %s to %s success. %s", vmInfo.m_name.c_str(),
        newName.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::PowerOnMachine(const VMInfo &vmInfo)
{
    if (m_jobResult != AppProtect::JobResult::type::SUCCESS) {
        INFOLOG("Not to power on vm when job failed, %s", m_jobId.c_str());
        return SUCCESS;
    }
    m_reportParam.label = "";
    m_reportArgs = {};
    ApplicationLabelType labelParam;
    if (!vmInfo.m_name.empty()) {
        labelParam.label = CNWARE_POWERON_MACHINE_LABEL;
        labelParam.params = std::vector<std::string>{vmInfo.m_name};
        ReportJobDetail(labelParam);
    }

    if (DoPowerOnMachine(vmInfo) != SUCCESS) {
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNWARE_POWERON_MACHINE_FAILED_LABEL;
        labelParam.params = std::vector<std::string>{vmInfo.m_name};
        labelParam.errCode = CNWARE_POWEROFF_MACHINE_FAILED_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::DoPowerOnMachine(const VMInfo &vmInfo)
{
    CNwareRequest req;
    SetCommonInfo(req);
    req.SetDomainId(vmInfo.m_uuid);
    std::shared_ptr<CNwareResponse> response = m_cnwareClient->PowerOnVM(req);
    if (response == nullptr) {
        ERRLOG("PowerOnMachine failed. %s", m_jobId.c_str());
        if (CNWARE_LABEL_DICT.count(m_cnwareClient->GetErrorCode().m_errorCode) != 0) {
            m_reportArgs = { vmInfo.m_name };
            m_reportParam = {
                CNWARE_LABEL_DICT[m_cnwareClient->GetErrorCode().m_errorCode],
                JobLogLevel::TASK_LOG_ERROR,
                SubJobStatus::FAILED, 0, 0 };
        }
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetTaskId())) {
        ERRLOG("Query PowerOnMachine task status failed. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Poweron vm %s success. %s", vmInfo.m_name.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::AllowBackupInLocalNode(const AppProtect::BackupJob &job, int32_t &errorCode)
{
    return CheckProtectEnvConn(job.protectEnv, job.protectObject.id, errorCode);
}

int32_t CNwareProtectEngine::ConnectToEnv(const AppProtect::ApplicationEnvironment &env)
{
    SetAppEnv(env);
    if (!Init()) {
        ERRLOG("Init CNware engine failed! %s", m_taskId.c_str());
        return FAILED;
    }
    int64_t errorCode(0);
    std::string errorDes;
    if (!InitClient(errorCode, errorDes)) {
        ERRLOG("InitClient failed! %s, error is %llu:%s", m_taskId.c_str(),
               errorCode, errorDes);
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::CheckStorageConnectionBackup(const VolInfo &volInfo, int32_t &erroCode)
{
    INFOLOG("Target volume %s storage pool id: %s", volInfo.m_name.c_str(),  volInfo.m_datastore.m_poolId.c_str());
    StoragePoolInfo poolInfo;
    ApplicationLabelType labelParam;
    labelParam.label = CNWARE_STORAGEPOOL_CHECK_LABEL;
    labelParam.params = std::vector<std::string>{volInfo.m_name, volInfo.m_datastore.m_name};
    ReportJobDetail(labelParam);
    std::vector<std::string> noTasksArgs {};
    if (QueryAgentHostStoragePoolList(poolInfo, volInfo.m_datastore.m_poolId) != SUCCESS || poolInfo.m_total == 0) {
        ERRLOG("Get agent host storage pool failed.");
        labelParam.level = JobLogLevel::TASK_LOG_WARNING;
        labelParam.label = CNWARE_STORAGEPOOL_NOTAVAILABLE_LABEL;
        labelParam.params = std::vector<std::string>{volInfo.m_name, volInfo.m_datastore.m_name};
        ReportJobDetail(labelParam);
        noTasksArgs = std::vector<std::string>{GetHostName(), volInfo.m_datastore.m_name};
        SetNoTasksArgs(noTasksArgs);
        return FAILED;
    }

    INFOLOG("Check agent connection to storage(%s) of vol(%s) success",
        volInfo.m_datastore.m_poolId.c_str(), volInfo.m_name.c_str());
    return SUCCESS;
}

int32_t CNwareProtectEngine::AllowBackupSubJobInLocalNode(const AppProtect::BackupJob &job,
    const AppProtect::SubJob &subJob, int32_t &errorCode)
{
    if (CheckProtectEnvConn(job.protectEnv, job.protectObject.id, errorCode) != SUCCESS) {
        ERRLOG("Failed to check protect environment, taskId: %s", m_taskId.c_str());
        errorCode = CHECK_CNWARE_CONNECT_FAILED;
        CheckCertIsExist(errorCode);
        return FAILED;
    }
    if (subJob.jobName == REPORT_COPY_SUB_JOB) {
        return SUCCESS;
    }
    if (subJob.jobType == SubJobType::type::POST_SUB_JOB) {
        return SUCCESS;
    }
    BackupSubJobInfo backupSubJob {};
    if (!Module::JsonHelper::JsonStringToStruct(subJob.jobInfo, backupSubJob)) {
        ERRLOG("Get backup subjob info failed, %s", m_taskId.c_str());
        return FAILED;
    }
    int32_t erro;
    if (CheckStorageConnectionBackup(backupSubJob.m_volInfo, erro) != SUCCESS) {
        errorCode = CNWARE_STORAGEPOOL_NOTAVAILABLE_ERROR;
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::AllowRestoreInLocalNode(const RestoreJob &job, int32_t &errorCode)
{
    m_checkErrorCode = CNwareErrorCode::ALLOW_COMMON_ERROR;
    if (CheckProtectEnvConn(job.targetEnv, job.targetObject.id, errorCode) != SUCCESS) {
        ERRLOG("Failed to check protect environment, taskId: %s", m_taskId.c_str());
        errorCode = CHECK_CNWARE_CONNECT_FAILED;
        CheckCertIsExist(errorCode);
        return FAILED;
    }
    m_requestId = job.requestId;
    m_appEnv = job.targetEnv;
    m_application = job.targetObject;
    m_subObjects = job.restoreSubObjects;
    m_copy = job.copies[0];
    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(job.extendInfo, jobAdvancePara)) {
        ERRLOG("Convert m_restorePara extendInfo failed, %s", m_jobId.c_str());
        return FAILED;
    }
    m_jobAdvPara = jobAdvancePara;
    if (jobAdvancePara.isMember("restoreLevel") && jobAdvancePara.isMember("targetLocation")) {
        m_restoreLevel = RestoreLevel(std::stoi(jobAdvancePara["restoreLevel"].asString()));
        m_targetLocation = jobAdvancePara["targetLocation"].asString();
    }
    if (CheckStoragePoolRestore(job.restoreSubObjects) != SUCCESS) {
        ERRLOG("CheckStoragePoolRestore failed, %s", m_taskId.c_str());
        errorCode = m_isSetArgs ? CNWARE_STORAGEPOOL_NOTAVAILABLE_ERROR : m_checkErrorCode;
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareProtectEngine::AllowRestoreSubJobInLocalNode(const RestoreJob &job, const SubJob &subJob,
    int32_t &errorCode)
{
    if (CheckProtectEnvConn(job.targetEnv, job.targetObject.id, errorCode) != SUCCESS) {
        ERRLOG("Failed to check protect environment, taskId: %s", m_taskId.c_str());
        errorCode = CHECK_CNWARE_CONNECT_FAILED;
        CheckCertIsExist(errorCode);
        return FAILED;
    }
    if (subJob.jobType == SubJobType::type::POST_SUB_JOB) {
        return SUCCESS;
    }
    if (subJob.jobName == "PostSubJob") {
        return SUCCESS;
    }
    SubJobExtendInfo restoreSubJob {};
    if (!Module::JsonHelper::JsonStringToStruct(subJob.jobInfo, restoreSubJob)) {
        ERRLOG("Get backup subjob info failed, %s", m_taskId.c_str());
        return FAILED;
    }
    VolInfo targetVolumeInfo;
    if (!Module::JsonHelper::JsonStringToStruct(restoreSubJob.m_targetVolumeInfo, targetVolumeInfo)) {
        ERRLOG("Get backup subjob info failed, %s", m_taskId.c_str());
        return FAILED;
    }
    int32_t erro;
    if (CheckStorageConnectionBackup(targetVolumeInfo, erro) != SUCCESS) {
        errorCode = CNWARE_STORAGEPOOL_NOTAVAILABLE_ERROR;
        return FAILED;
    }
    return SUCCESS;
}

void CNwareProtectEngine::CheckCertIsExist(int32_t &errorCode)
{
    if (m_certMgr->IsVerifyCert() && m_certMgr->GetCertPath() == "") {
        ERRLOG("The certification is not exists.");
        errorCode = HCS_CERT_NOT_EXIST;
        return;
    }
    return;
}
}