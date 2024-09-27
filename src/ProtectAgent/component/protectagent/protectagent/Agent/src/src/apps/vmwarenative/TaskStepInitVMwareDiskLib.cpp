/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepInitVMwareDiskLib.cpp
 * @brief  Contains function declarations for TaskStepInitVMwareDiskLib
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/vmwarenative/TaskStepInitVMwareDiskLib.h"
#include "plugins/DataProcessClientHandler.h"
#include "message/tcp/CDppMessage.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Path.h"

TaskStepInitVMwareDiskLib::TaskStepInitVMwareDiskLib(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStepVMwareNative(id, taskId, name, ratio, order)
{
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
}

TaskStepInitVMwareDiskLib::~TaskStepInitVMwareDiskLib()
{
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
}

mp_int32 TaskStepInitVMwareDiskLib::Init(const Json::Value &param)
{
    // parse VDDK version info
    if (!param.isObject() || !param.isMember(EXT_CMD_PROTECT_PRODUCTMANAGER_INFO)) {
        ERRLOG("Input param does not have key: '%s'.", EXT_CMD_PROTECT_PRODUCTMANAGER_INFO.c_str());
        return MP_FAILED;
    }

    mp_string strVMVer;
    mp_string strVMIp;
    mp_uint32 vmPort;
    Json::Value vmPM = param[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO];
    GET_JSON_STRING(vmPM, PARAM_KEY_PRODUCTMANAGER_VERSION, strVMVer);
    GET_JSON_STRING(vmPM, PARAM_KEY_PRODUCTMANAGER_IP, strVMIp);
    GET_JSON_UINT32(vmPM, PARAM_KEY_PRODUCTMANAGER_PORT, vmPort);
    if (strVMVer.empty()) {
        m_stepStatus = STATUS_INITIAL;
        ERRLOG("The version of vCenter must be provided!");
        return MP_FAILED;
    }

    mp_string thumbPrint = GetThumbPrint(strVMIp, vmPort);
    if (thumbPrint.empty()) {
        ERRLOG("Get VMware(%s:%u) thumbprint failed.", strVMIp.c_str(), vmPort);
        return ERROR_VMWARE_NETWORK;
    }

    // update json param
    // vddk version must be digital string
    if (!(strVMVer.find_first_not_of(AVAILABLE_STRING) == std::string::npos &&
        strVMVer.find_first_of(DIGITAL_STRING) != std::string::npos)) {
        ERRLOG("Invalid vddk version string '%s'.", strVMVer.c_str());
        return MP_FAILED;
    }
    mp_string strVddkVersion = strVMVer.substr(0, TARGET_STRLEN);
    INFOLOG("The version of VDDK should be '%s' for task '%s'.", strVddkVersion.c_str(), m_taskId.c_str());

    // append actual vddk path to data process service
    mp_string strVddkLibPath = CPath::GetInstance().GetLibPath() + PATH_SEPARATOR + "vddk" + PATH_SEPARATOR +
                               strVddkVersion + PATH_SEPARATOR + VMWAREDEF::STR_VIXDISKLIB_PATH;

    m_reqMsgToDataProcess = param;
    m_reqMsgToDataProcess[VMWAREDEF::PARAM_VDDKLIB_PATH] = strVddkLibPath;
    m_reqMsgToDataProcess[EXT_CMD_PROTECT_PRODUCTMANAGER_INFO][PARAM_KEY_PRODUCTMANAGER_THUMBPRINT] = thumbPrint;

    // set status
    TaskContext::GetInstance()->SetValueString(m_taskId, VMWAREDEF::PARAM_VDDKLIB_INIT_STATUS, "false");
    TaskContext::GetInstance()->SetValueString(m_taskId, VMWAREDEF::PARAM_VDDKLIB_VERSION, strVddkVersion);
    m_stepStatus = STATUS_INITIAL;

    return MP_SUCCESS;
}

mp_int32 TaskStepInitVMwareDiskLib::Run()
{
    mp_int32 iRet = DataProcessLogic(m_reqMsgToDataProcess,
        m_respMsgFromDataProcess,
        EXT_CMD_VMWARENATIVE_INIT_VDDKLIB,
        EXT_CMD_VMWARENATIVE_INIT_VDDKLIB_ACK);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    // set VDDK initization status to taskcontext
    TaskContext::GetInstance()->SetValueString(m_taskId, VMWAREDEF::PARAM_VDDKLIB_INIT_STATUS, "true");

    return iRet;
}
