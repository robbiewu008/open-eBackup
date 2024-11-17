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
#include "apps/vmwarenative/TaskStepVMwareNative.h"

#include <map>
#include <cstdio>
#include <sstream>
#include <vector>

#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Ip.h"
#include "common/CSystemExec.h"
#include "array/array.h"
#include "array/disk.h"
#include "plugins/DataProcessClientHandler.h"
#include "message/curlclient/CurlHttpClient.h"

namespace {
    constexpr int THREAD_TIMEOUT_TIMES_DEFAULT = 600;
};

TaskStepVMwareNative::TaskStepVMwareNative(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStep(id, taskId, name, ratio, order)
{
    m_internalTimeout = THREAD_TIMEOUT_TIMES_DEFAULT;
    m_bTargetLinked = MP_FALSE;
    m_bDiskScaned = MP_FALSE;
    m_strCurrentVddkVersion.clear();
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_iSystemVirt = 0;
    m_bObtainSystemVirt = false;
}

TaskStepVMwareNative::~TaskStepVMwareNative()
{
    m_bTargetLinked = MP_FALSE;
    m_bDiskScaned = MP_FALSE;
    m_strCurrentVddkVersion.clear();
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_iSystemVirt = 0;
    m_bObtainSystemVirt = false;
}

mp_int32 TaskStepVMwareNative::Init(const Json::Value &param)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::Run()
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::Cancel()
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::Cancel(Json::Value &respParam)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::Stop(const Json::Value &param)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::Update(const Json::Value &param)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::Update(Json::Value &param, Json::Value &respParam)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::Finish(const Json::Value &param)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::Finish(Json::Value &param, Json::Value &respParam)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::FillSpeedAndProgress(mp_string vecResult, mp_int32 &speed, mp_int32 &progress)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 TaskStepVMwareNative::ConvertBackupSpeed(mp_string speed)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_string TaskStepVMwareNative::GetThumbPrint(const mp_string& pIp, mp_uint32 uPort)
{
    std::string tempIp = CIP::FormatFullUrl(pIp);
    std::string url = "https://" + tempIp + ":" +  CMpString::to_string(uPort);

    CurlHttpClient curlClient;
    mp_string strThumbPrint;
    int ret = curlClient.GetThumbPrint(url, strThumbPrint);
    if (ret != 0) {
        ERRLOG("Get thumbprint(%s:%u) failed, ret=%d.", pIp.c_str(), uPort, ret);
    } else {
        DBGLOG("Get thumbprint(%s:%u) successfully.", pIp.c_str(), uPort);
    }
    return strThumbPrint;
}

mp_int32 TaskStepVMwareNative::ExchangeMsgWithDataProcessService(
    Json::Value &param, Json::Value &respParam, mp_uint32 reqCmd, mp_uint32 rspCmd, mp_uint32 timeout)
{
    LOGGUARD("");
    CDppMessage *msg = NULL;
    DataPathProcessClient *pClient = DataProcessClientHandler::GetInstance().FindDpClient(m_strCurrentVddkVersion);
    if (pClient == NULL) {
        ERRLOG("DP client(%s) not exist, taskId=%s.", m_strCurrentVddkVersion.c_str(), m_taskId.c_str());
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    // parse hostagent system virt type
    if (ObtainSystemVirtValue() != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN,
            "Unable to obtain hostagent system virt value from config file, will use default value: '%d'",
            m_iSystemVirt);
    }

    mp_uint64 seq = pClient->GetSeqNo();
    NEW_CATCH_RETURN_FAILED(msg, CDppMessage);
    msg->InitMsgHead(MSG_DATA_TYPE_MANAGE, 0, seq);
    msg->SetMsgSrc(ROLE_HOST_AGENT);
    msg->SetMsgTgt(ROLE_HOST_DATAPROCESS);

    Json::Value protectedMsg;
    // add hostagent type to each msg body
    param[PARAM_KEY_HOSTAGENT_SYSTEM_VIRT] = m_iSystemVirt;
    protectedMsg[MANAGECMD_KEY_CMDNO] = reqCmd;
    protectedMsg[MANAGECMD_KEY_BODY] = param;
    msg->SetMsgBody(protectedMsg);

    INFOLOG("Generate req msg taskid=%s, cmd=0x%x, seq=%llu", m_taskId.c_str(), reqCmd, seq);
    CDppMessage *rspMsg = NULL;
    pClient->SendDPMessage(m_taskId, msg, rspMsg, timeout);
    if (rspMsg == NULL) {
        ERRLOG("DPclient send DPP message failed, taskid=%s, cmd=0x%x, seq=%llu.", m_taskId.c_str(), reqCmd, seq);
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    // receive message from dataprocess service
    mp_int32 dppErr = MP_SUCCESS;
    mp_int32 iRet = ResponseMsgProcesser(rspCmd, seq, rspMsg, dppErr, respParam);

    // delete response message
    delete rspMsg;

    if (iRet != MP_SUCCESS && dppErr == MP_SUCCESS) {
        ERRLOG("Errors occur in dp service, taskid=%s, cmd=0x%x(%llu), iRet=%d.", m_taskId.c_str(), reqCmd, seq, iRet);
        mp_int32 iRet1 = pClient->ResetConnection();
        if (iRet1 == MP_SUCCESS) {
            ERRLOG("Reset dp connection, taskid=%s, cmd=0x%x(%llu), iRet=%d.", m_taskId.c_str(), reqCmd, seq, iRet1);
        }
        return iRet;
    }

    return iRet;
}
// 获取dp进程是否在最近重启过，用来解决因为dp进程调用vddk接口导致的崩溃问题
mp_bool TaskStepVMwareNative::DPRestartRecently()
{
    mp_string CurrentVddkVersion;
    TaskContext::GetInstance()->GetValueString(m_taskId, VMWAREDEF::PARAM_VDDKLIB_VERSION, CurrentVddkVersion);
    // strCmd无外部注入
    mp_string strCmd = "ps -eo etime,cmd | grep '" + OM_DPP_EXEC_NAME + " " +
        CMpString::to_string(OCEAN_VMWARENATIVE_SERVICE) + " " + CurrentVddkVersion.c_str() +
        "' | grep -v 'grep' | grep -v 'gdb' | grep -v 'vi' | grep -v 'tail'"; // 目前就支持OCEAN_VMWARENATIVE_SERVICE一种类型
    std::vector<mp_string> vecReturn;
    CHECK_FAIL_EX(CheckCmdDelimiter(CurrentVddkVersion));
    CSystemExec::ExecSystemWithEcho(strCmd, vecReturn);
    for (int i = 0; i < vecReturn.size(); i++) {
        mp_string strCMDReturn = vecReturn[i];
        std::vector<mp_string> vecCMDReturn;
        CMpString::StrSplit(vecCMDReturn, strCMDReturn, ' ');
        mp_string strRunningTime;

        // 获取到的cmd输出有空格，需要获取的时间为第一个非空字段,增加相关获取逻辑
        for (int j = 0; j < vecCMDReturn.size(); j++) {
            if (!vecCMDReturn[j].empty()) {
                strRunningTime = vecCMDReturn[j];
                break;
            }
        }

        // 时间有三种格式，如果超过一天则是d-hh:mm:ss,如果不超过一小时，则是mm:ss，其余情况则是hh:mm:ss
        // 查询进度是每30秒一次，考虑到执行时间耗时，以50秒内重启过作为判断依据，所以只判断mm:ss场景(字符长度为5），
        if (!strRunningTime.empty() && strRunningTime.size() == 5) {
            vecCMDReturn.clear();
            CMpString::StrSplit(vecCMDReturn, strRunningTime, ':');
            mp_int32 iDPRecentlyRestartTime = 50;
            if (vecCMDReturn.size() > 1 && vecCMDReturn[0] == "00" &&
                atoi(vecCMDReturn[1].c_str()) < iDPRecentlyRestartTime) {
                return true;
            }
        }
    }
    if (vecReturn.size() == 0) { // dp进程core掉，但是尚未被重启的场景
        ERRLOG("dataprocess does not exists.");
        return true;
    }
    return false;
}

mp_int32 TaskStepVMwareNative::DataProcessLogic(
    Json::Value &param, Json::Value &respParam, mp_uint32 reqCmd, mp_uint32 rspCmd)
{
    // Here, we should package the VDDK version and DataProcess service type params to a json body
    TaskContext::GetInstance()->GetValueString(m_taskId, VMWAREDEF::PARAM_VDDKLIB_VERSION, m_strCurrentVddkVersion);

    // parse timeout value from config file
    if (CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SYSTEM_SECTION, CFG_THREAD_TIMEOUT, m_internalTimeout) != MP_SUCCESS) {
        WARNLOG("Unabel to obtin thread timeout value '%d' from config file, will use default value: '%d' .",
            m_internalTimeout);
    }

    mp_int32 iRet = ExchangeMsgWithDataProcessService(param, respParam, reqCmd, rspCmd, m_internalTimeout);
    if (iRet != MP_SUCCESS) {
        if (DPRestartRecently()) {
            ERRLOG("Error occurs when interact with data process service, because data process service restart,\
            reqCmd=0x%x, rspCmd=0x%x, task id '%s', errorcode: '%d'",
                reqCmd,
                rspCmd,
                m_taskId.c_str(),
                iRet);

            return ERROR_VM_PROCESS_RESTART_NEED_RETRY;
        }
        ERRLOG("Error occurs when interact with data process service, \
        reqCmd=0x%x, rspCmd=0x%x, task id '%s', errorcode: '%d'",
            reqCmd,
            rspCmd,
            m_taskId.c_str(),
            iRet);

        return ERROR_AGENT_INTERNAL_ERROR;
    }

    return iRet;
}

mp_bool TaskStepVMwareNative::IsVddkLibInited(mp_void)
{
    mp_string strState;
    INFOLOG("Current taskid=%s.", m_taskId.c_str());
    TaskContext::GetInstance()->GetValueString(m_taskId, VMWAREDEF::PARAM_VDDKLIB_INIT_STATUS, strState);
    return (strState == "true") ? MP_TRUE : MP_FALSE;
}

mp_int32 TaskStepVMwareNative::Redo(mp_string &innerPID)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepVMwareNative::ResponseMsgProcesser(
    mp_uint32 exCmd, mp_uint64 exSeq, CDppMessage *dppMsg, mp_int32 &dppError, Json::Value &respParam)
{
    dppError = MP_SUCCESS;
    mp_uint32 rspCmd = dppMsg->GetManageCmd();
    mp_uint64 rspSeq = dppMsg->GetOrgSeqNo();
    if (exCmd != rspCmd) {
        mp_int64 errCode;
        mp_string errDetail;
        if (dppMsg->GetManageError(errCode, errDetail) != MP_SUCCESS) {
            errCode = ERROR_AGENT_INTERNAL_ERROR;
        }
        if (errCode != 0) {
            errCode = ERROR_AGENT_INTERNAL_ERROR;
        }

        ERRLOG("expect cmd=0x%x(%llu), res cmd=0x%x(%llu), taskid=%s.", exCmd, exSeq, rspCmd, rspSeq, m_taskId.c_str());
        CreateFadeResponseMsg(exCmd, errCode, errDetail, respParam);
        return MP_FAILED;
    }

    mp_string strbuffer;
    if (WipeSensitiveForJsonData(dppMsg->GetBuffer(), strbuffer) != MP_SUCCESS) {
        strbuffer = dppMsg->GetBuffer();
    }

    INFOLOG("Response from dp service, taskid=%s, expect=0x%x(%llu), res=0x%x(%llu).",
        m_taskId.c_str(),
        exCmd,
        exSeq,
        rspCmd,
        rspSeq);

    mp_int32 iRet = dppMsg->GetManageBody(respParam);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get msg body failed, taskid=%s, expect=0x%x(%llu), res=0x%x(%llu).",
            m_taskId.c_str(),
            exCmd,
            exSeq,
            rspCmd,
            rspSeq);
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    // judge the return code: if not zero, means current task performs failed
    GET_JSON_INT32(respParam, MANAGECMD_KEY_ERRORCODE, dppError);
    if (MP_SUCCESS != dppError) {
        ERRLOG("Error occurs in data process service, taskid=%s, errcode=%d.", m_taskId.c_str(), dppError);
        return dppError;
    }

    return MP_SUCCESS;
}

mp_void TaskStepVMwareNative::CreateFadeResponseMsg(
    mp_uint32 rspCmd, mp_int32 errorCode, const mp_string &errorDetail, Json::Value &respParam)
{
    Json::Value resBody;
    respParam[MANAGECMD_KEY_BODY] = std::move(resBody);
    respParam[MANAGECMD_KEY_CMDNO] = rspCmd;
    respParam[MANAGECMD_KEY_ERRORCODE] = errorCode;
    respParam[MANAGECMD_KEY_ERRORDETAIL] = errorDetail;
}

mp_int32 TaskStepVMwareNative::ObtainSystemVirtValue()
{
    if (!m_bObtainSystemVirt) {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
            CFG_DATAPROCESS_SECTION, CFG_HOSTAGENT_SYSTEM_VIRT, m_iSystemVirt);
        if (iRet == MP_SUCCESS) {
            m_bObtainSystemVirt = true;
            COMMLOG(OS_LOG_DEBUG,
                "The hostagent system virt value from config file is: '%d'",
                m_iSystemVirt);
        }
    }
    return m_bObtainSystemVirt ? MP_SUCCESS : MP_FAILED;
}
