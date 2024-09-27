/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file TaskStepUmountVMwareNasMedia.cpp
 * @author w00558987
 * @brief 提供VMwareNative高级备份特性 NAS文件系统Umount功能
 * @version 0.1
 * @date 2021-01-05
 *
 */

#include "apps/vmwarenative/TaskStepUmountVMwareNasMedia.h"
#include "plugins/DataProcessClientHandler.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"

TaskStepUmountVMwareNasMedia::TaskStepUmountVMwareNasMedia(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStepPrepareNasMedia(id, taskId, name, ratio, order)
{}

TaskStepUmountVMwareNasMedia::~TaskStepUmountVMwareNasMedia()
{
    m_strParentTaskId = "";
    m_strNasStorageIP = "";
    m_strNasSharePath = "";
}

mp_int32 TaskStepUmountVMwareNasMedia::Init(const Json::Value &param)
{
    // parse parent task id
    GET_JSON_STRING(param, VMWAREDEF::PARAM_PARENTTASK_STR, m_strParentTaskId);

    // set nas storage param
    // parse nas storage ip
    if (!param.isObject() || !param.isMember(VMWAREDEF::PARAM_STORAGE_STR)) {
        ERRLOG("Input param has no key: '%s'.", VMWAREDEF::PARAM_STORAGE_STR.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    std::vector<mp_string> storageIps;
    GET_JSON_ARRAY_STRING_OPTION(param[VMWAREDEF::PARAM_STORAGE_STR], VMWAREDEF::PARAM_STORAGE_IP_STR, storageIps);
    if (storageIps.size() == 0) {
        WARNLOG("The backend storage ip not provided, will try unmount nas over FC");
        m_unmountNasOverFC = true;
    } else {
        for (int i = 0; i < storageIps.size(); ++i) {
            m_strNasStorageIP = m_strNasStorageIP + NODE_SEMICOLON + storageIps[i];
        }
    }

    // parse nas storage type
    if (!param[VMWAREDEF::PARAM_STORAGE_STR].isMember(VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR)) {
        COMMLOG(OS_LOG_WARN, "Input param has no key: '%s'.", VMWAREDEF::PARAM_STORAGE_TYPE_STR.c_str());
        m_iStorageProtocol = VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI;
    } else {
        GET_JSON_INT32(param[VMWAREDEF::PARAM_STORAGE_STR], VMWAREDEF::PARAM_STORAGE_PROTOCOL_STR, m_iStorageProtocol);
    }

    // parse nas storage share path
    if (!param.isMember(VMWAREDEF::PARAM_MEDIUMID_STR)) {
        ERRLOG("Input param has no key: '%s'.", VMWAREDEF::PARAM_MEDIUMID_STR.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    GET_JSON_STRING(param, VMWAREDEF::PARAM_MEDIUMID_STR, m_strNasSharePath);

    m_stepStatus = STATUS_INITIAL;

    return MP_SUCCESS;
}

mp_int32 TaskStepUmountVMwareNasMedia::Run()
{
    mp_string strServiceType =  m_unmountNasOverFC ? "vmware_fc" : "vmware";
    std::vector<mp_string> vecRst;
    std::ostringstream ossParam;
    ossParam << SERVICE_TYPE << STR_EQUAL << strServiceType << NODE_COLON << VMWAREDEF::KEY_STORAGE_IP << STR_EQUAL
             << m_strNasStorageIP << NODE_COLON << VMWAREDEF::KEY_NAS_FILESYSTEM_NAME << STR_EQUAL << m_strNasSharePath
             << NODE_COLON << VMWAREDEF::KEY_PARENTTASK_ID << STR_EQUAL << m_strParentTaskId << NODE_COLON
             << VMWAREDEF::KEY_BACKUP_ID << STR_EQUAL << m_taskId << NODE_COLON;
    if (MP_SUCCESS != UmountNasMedia(ossParam.str(), vecRst)) {
        ERRLOG("Unable to umount backend nas storage '%s'.", m_strNasStorageIP.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
