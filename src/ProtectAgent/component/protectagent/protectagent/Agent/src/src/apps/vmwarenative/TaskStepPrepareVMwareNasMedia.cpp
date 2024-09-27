#include "apps/vmwarenative/TaskStepPrepareVMwareNasMedia.h"
#include "plugins/DataProcessClientHandler.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Ip.h"
#include "host/host.h"
#include "message/tcp/CDppMessage.h"

namespace {
    const mp_string PARAM_NAS_OVER_FC_STR = "accessNASOverFC";
}

TaskStepPrepareVMwareNasMedia::TaskStepPrepareVMwareNasMedia(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStepPrepareNasMedia(id, taskId, name, ratio, order)
{}

TaskStepPrepareVMwareNasMedia::~TaskStepPrepareVMwareNasMedia()
{
    m_strParentTaskId = "";
    m_strNasStorageIP = "";
    m_OtherNasStorageIP = "";
    m_strNasSharePath = "";
}

mp_int32 TaskStepPrepareVMwareNasMedia::Init(const Json::Value &param)
{
    // parse parent task id
    GET_JSON_STRING(param, VMWAREDEF::PARAM_PARENTTASK_STR, m_strParentTaskId);

    // set nas storage param
    CHECK_FAIL_EX(GetStorageParam(param));

    // parse nas storage share path
    if (!param.isObject() || !param.isMember(VMWAREDEF::PARAM_VOLUMES_STR)) {
        ERRLOG("Input param has no key: '%s'.", VMWAREDEF::PARAM_VOLUMES_STR.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    std::vector<Json::Value> jsonVolumes;
    GET_JSON_ARRAY_JSON_OPTION(param, VMWAREDEF::PARAM_VOLUMES_STR, jsonVolumes);
    // value of all elements' attr 'mediumID' is the same
    if (jsonVolumes.size() == 0) {
        ERRLOG("Empty value of key param: '%s'.", VMWAREDEF::PARAM_STORAGE_STR.c_str());
        return MP_FAILED;
    } else {
        Json::Value jsonVolume = jsonVolumes[0];
        GET_JSON_STRING(jsonVolume, VMWAREDEF::PARAM_MEDIUMID_STR, m_strNasSharePath);
        CHECK_FAIL_EX(CheckCmdDelimiter(m_strNasSharePath));
    }

    m_stepStatus = STATUS_INITIAL;

    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareVMwareNasMedia::GetDataTurboAuthInfoFromParam(const Json::Value &param)
{
    if (CJsonUtils::GetJsonBool(param[VMWAREDEF::PARAM_STORAGE_STR],
        PARAM_NAS_OVER_FC_STR, m_accessNASOverFC) == MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "set accessNASOverFC: '%d'.", m_accessNASOverFC);
    }
    if (param.isObject() && param[VMWAREDEF::PARAM_STORAGE_STR].isObject() &&
        param[VMWAREDEF::PARAM_STORAGE_STR].isMember("srcDedupInfo") &&
        param[VMWAREDEF::PARAM_STORAGE_STR]["srcDedupInfo"].isObject()) {
        if (param[VMWAREDEF::PARAM_STORAGE_STR]["srcDedupInfo"].isMember("srcDedup") &&
            param[VMWAREDEF::PARAM_STORAGE_STR]["srcDedupInfo"]["srcDedup"].isBool()) {
            isSrcDedup = param[VMWAREDEF::PARAM_STORAGE_STR]["srcDedupInfo"]["srcDedup"].asBool();
        }
        if (isSrcDedup || m_accessNASOverFC) {
            if (!m_accessNASOverFC) {
                // if not access NAS over FC, ip will be used
                GET_JSON_ARRAY_STRING_OPTION(param[VMWAREDEF::PARAM_STORAGE_STR]["srcDedupInfo"], "storageIps",
                    storageDataturboIps);
            }
            if (param[VMWAREDEF::PARAM_STORAGE_STR]["srcDedupInfo"].isMember("auth")) {
                GET_JSON_STRING(param[VMWAREDEF::PARAM_STORAGE_STR]["srcDedupInfo"]["auth"], "name", m_dataturboUser);
                GET_JSON_STRING(param[VMWAREDEF::PARAM_STORAGE_STR]["srcDedupInfo"]["auth"], "passwd",
                    m_dataturboPassword);
            }
        }
    }
}

mp_int32 TaskStepPrepareVMwareNasMedia::GetStorageParam(const Json::Value &param)
{
    if (!param.isObject() || !param.isMember(VMWAREDEF::PARAM_STORAGE_STR)) {
        ERRLOG("Input param has no key: '%s'.", VMWAREDEF::PARAM_STORAGE_STR.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    GetDataTurboAuthInfoFromParam(param);
    if (param[VMWAREDEF::PARAM_STORAGE_STR].isMember("linkEncryption")) {
        isLinkEncry = param[VMWAREDEF::PARAM_STORAGE_STR]["linkEncryption"].asBool();
    }
    if (!m_accessNASOverFC) {
        std::vector <mp_string> storageIps;
        GET_JSON_ARRAY_STRING_OPTION(param[VMWAREDEF::PARAM_STORAGE_STR], VMWAREDEF::PARAM_LOCAL_STORAGE_IP_STR,
                                     storageIps);
        mp_size ipListSize = storageIps.size();
        for (mp_size i = 0; i < ipListSize; ++i) {
            CHECK_FAIL_EX(CheckIpAddressValid(storageIps[i]));
            m_strNasStorageIP = m_strNasStorageIP + NODE_SEMICOLON + storageIps[i];
        }
        std::vector <mp_string> otherStorageIps;
        GET_JSON_ARRAY_STRING_OPTION(param[VMWAREDEF::PARAM_STORAGE_STR], VMWAREDEF::PARAM_OTHER_STORAGE_IP_STR,
                                     otherStorageIps);
        ipListSize = otherStorageIps.size();
        for (mp_size i = 0; i < ipListSize; ++i) {
            CHECK_FAIL_EX(CheckIpAddressValid(otherStorageIps[i]));
            m_OtherNasStorageIP = m_OtherNasStorageIP + NODE_SEMICOLON + otherStorageIps[i];
        }

        if (storageIps.size() == 0 && otherStorageIps.size() == 0) {
            ERRLOG("The backend storage ip must be provided!", VMWAREDEF::PARAM_STORAGE_STR.c_str());
            return MP_FAILED;
        }
    }
    // parse nas storage type
    if (!param[VMWAREDEF::PARAM_STORAGE_STR].isObject() ||
        !param[VMWAREDEF::PARAM_STORAGE_STR].isMember(VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR)) {
        COMMLOG(OS_LOG_WARN, "Input param has no key: '%s'.", VMWAREDEF::PARAM_STORAGE_TYPE_STR.c_str());
        m_iStorageProtocol = VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI;
    } else {
        GET_JSON_INT32(param[VMWAREDEF::PARAM_STORAGE_STR], VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR, m_iStorageProtocol);
    }
    return MP_SUCCESS;
}

void TaskStepPrepareVMwareNasMedia::SetLogInfo(
    const mp_string &label, const mp_int32 &errorCode, const std::vector<std::string> &errorParams)
{
    INFOLOG("Enter SetLogInfo, iRet %s.  %d", label.c_str(), errorCode);
    mp_string hostIP;
    mp_string strPort;
    if (CIP::GetListenIPAndPort(hostIP, strPort) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Agent listen IP and port failed.");
        return;
    }
    m_respMsg[PARAM_KEY_LOGLABEL] = label;
    m_respMsg[PARAM_KEY_LOGLABELPARAM].append(hostIP);
    m_respMsg[PARAM_KEY_LOGDETAIL] = CMpString::to_string(errorCode);
    if (errorCode == ERR_MOUNT_DATA_TURBO_FILE_SYSTEM || errorCode == ERR_CREATE_DATA_TURBO_LINK ||
        errorCode == ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT) {
        for (size_t i = 0; i < errorParams.size(); i++) {
            m_respMsg[PARAM_KEY_LOGPARAMS].append(errorParams[i]);
            INFOLOG(" SetLogInfo, error %s", errorParams[i].c_str());
        }
    }
}

mp_int32 TaskStepPrepareVMwareNasMedia::Run()
{
    mp_string strServiceType = "vmware";
    std::vector<mp_string> vecRst;
    std::ostringstream ossParam;
    if (isSrcDedup || m_accessNASOverFC) {
        DataturboMountParam param;
        CHost host;
        mp_int32 iRet = host.GetHostSN(param.storageName); // 使用HostSN做Dataturbo链接对象的storage_name
        if (iRet != MP_SUCCESS) {
            ERRLOG("GetHostSN failed, iRet %d.", iRet);
            return MP_FAILED;
        }
        ossParam << SERVICE_TYPE << STR_EQUAL << strServiceType << NODE_COLON << VMWAREDEF::KEY_NAS_FILESYSTEM_NAME
                 << STR_EQUAL << m_strNasSharePath << NODE_COLON << VMWAREDEF::KEY_PARENTTASK_ID << STR_EQUAL
                 << m_strParentTaskId << NODE_COLON << VMWAREDEF::KEY_DATATURBOSTORAGENAME << STR_EQUAL
                 << param.storageName << NODE_COLON << VMWAREDEF::KEY_BACKUP_ID << STR_EQUAL << m_taskId << NODE_COLON;
        param.authUser = m_dataturboUser;
        param.authPwd = m_dataturboPassword;
        param.vecDataturboIP = storageDataturboIps;
        iRet = MountDataturboMedia(ossParam.str(), vecRst, param);
        if (MP_SUCCESS == iRet) {
            return MP_SUCCESS;
            COMMLOG(OS_LOG_WARN, "Mount Backup Storage meida through dataturbo protocol success!");
        }
        SetLogInfo(DATATURBO_FAILED_LABEL, iRet, {m_strNasSharePath});
        COMMLOG(OS_LOG_WARN, "Mount Backup Storage meida through dataturbo protocol failed!");
    }
    mp_int32 linkEncryption = isLinkEncry == true ? 1 : 0;
    ossParam << SERVICE_TYPE << STR_EQUAL << strServiceType << NODE_COLON << VMWAREDEF::KEY_STORAGE_IP << STR_EQUAL
             << m_strNasStorageIP << NODE_COLON << VMWAREDEF::KEY_NAS_FILESYSTEM_NAME << STR_EQUAL << m_strNasSharePath
             << NODE_COLON << VMWAREDEF::KEY_PARENTTASK_ID << STR_EQUAL << m_strParentTaskId << NODE_COLON
             << VMWAREDEF::KEY_BACKUP_ID << STR_EQUAL << m_taskId << NODE_COLON << VMWAREDEF::KEY_LINKENCRYPTION
             << STR_EQUAL << linkEncryption << NODE_COLON;

    // 挂载同控制器的节点失败后，再尝试挂载其他控制器的节点
    if (m_strNasStorageIP.empty() || MountNasMedia(ossParam.str(), vecRst) != MP_SUCCESS) {
        WARNLOG("Unable to mount current control nas storage '%s'.", m_strNasStorageIP.c_str());
        ossParam.str("");
        vecRst.clear();
        ossParam << SERVICE_TYPE << STR_EQUAL << strServiceType << NODE_COLON << VMWAREDEF::KEY_STORAGE_IP << STR_EQUAL
            << m_OtherNasStorageIP << NODE_COLON << VMWAREDEF::KEY_NAS_FILESYSTEM_NAME << STR_EQUAL << m_strNasSharePath
            << NODE_COLON << VMWAREDEF::KEY_PARENTTASK_ID << STR_EQUAL << m_strParentTaskId << NODE_COLON
            << VMWAREDEF::KEY_BACKUP_ID << STR_EQUAL << m_taskId << NODE_COLON << VMWAREDEF::KEY_LINKENCRYPTION
            << STR_EQUAL << linkEncryption << NODE_COLON;;
        if (m_OtherNasStorageIP.empty() || MountNasMedia(ossParam.str(), vecRst) != MP_SUCCESS) {
            ERRLOG("Unable to mount backend nas storage '%s'.", m_OtherNasStorageIP.c_str());
            return ERROR_DEVICE_NAS_MOUNT_FAILED;
        }
    }
    return MP_SUCCESS;
}
