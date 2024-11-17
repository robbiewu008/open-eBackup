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
#include <typeinfo>
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "message/tcp/MessageHandler.h"
#include "message/Message.h"
#include "message/rest/message_process.h"
#include "message/tcp/CDppMessage.h"
#include "agent/TaskWorker.h"

using namespace std;
namespace {
const int TASK_WORKER_SLEEP_TIME = 100;
}

TaskWorker::TaskWorker()
{
    m_plgCfgParse = NULL;
    m_bClosed = MP_FALSE;
    m_SCN = 0;
    m_plgCfgParse = 0;
    m_plgMgr = 0;
    (mp_void) memset_s(&m_threadId, sizeof(m_threadId), 0, sizeof(m_threadId));
    // Coverity&Fortify误报:UNINIT_CTOR
    // Coveirty&Fortify不认识公司安全函数memset_s，提示m_threadId.os_id未初始化
    CMpThread::InitLock(&m_tPlgLock);
    CMpThread::InitLock(&m_tReqLock);
    m_iThreadStatus = THREAD_STATUS_IDLE;
    m_bNeedExit = MP_FALSE;
    m_bProcReq = MP_FALSE;
}

TaskWorker::~TaskWorker()
{
    CMpThread::DestroyLock(&m_tPlgLock);
    CMpThread::DestroyLock(&m_tReqLock);
}

/* ------------------------------------------------------------
Description  : 初始化task worker线程
Input        : pPlgCfgParse -- 插件配置文件解析对象指针
                pPlgMgr -- 插件管理对象指针
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 TaskWorker::Init(PluginCfgParse& pPlgCfgParse, CPluginManager& pPlgMgr)
{
    mp_int32 iRet;

    COMMLOG(OS_LOG_DEBUG, "Begin init task worker.");

    m_plgCfgParse = &pPlgCfgParse;
    m_plgMgr = &pPlgMgr;
    m_SCN = m_plgMgr->GetSCN();

    iRet = CMpThread::Create(&m_threadId, WorkProc, this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init task worker failed, ret %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Init task worker succ.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 退出task worker线程
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_void TaskWorker::Exit()
{
    // 暂时忽略线程返回?
    m_bNeedExit = MP_TRUE;
    if (m_threadId.os_id != 0) {
        CMpThread::WaitForEnd(&m_threadId, NULL);
    }
}

/* ------------------------------------------------------------
Description  : 从消息队列中获取消息
Output       : msg -- 获取的消?
Return       : MP_SUCCESS -- 成功
                MP_FAILED -- 请求队列中没有请?
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 TaskWorker::PopRequest(message_pair_t& msg)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    if (m_vecReqQueue.empty()) {
        return MP_FAILED;
    }

    vector<message_pair_t>::iterator iter = m_vecReqQueue.begin();
    msg = *iter;
    m_vecReqQueue.erase(iter);

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 把消息保存到消息队列
Input        : msg -- 要保存到队列的消?
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_void TaskWorker::PushRequest(message_pair_t& msg)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    m_vecReqQueue.push_back(msg);
}

mp_bool TaskWorker::NeedExit()
{
    return m_bNeedExit;
}

mp_bool TaskWorker::GetThreadProcReqStatus()
{
    return m_bProcReq;
}

mp_void TaskWorker::SetThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}

/* ------------------------------------------------------------
Description  : 进行消息处理，task worker线程回调函数调用该函数处理请?
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 TaskWorker::ReqProc()
{
    message_pair_t msg;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_DEBUG, "Begin process request.");

    SetThreadStatus(THREAD_STATUS_RUNNING);
    while (!NeedExit()) {
        m_SCN = m_plgMgr->GetSCN();
        iRet = PopRequest(msg);
        if (iRet != MP_SUCCESS) {
            m_bProcReq = MP_FALSE;
            DoSleep(TASK_WORKER_SLEEP_TIME);
            continue;
        }
        m_bProcReq = MP_TRUE;
        COMMLOG(OS_LOG_DEBUG, "Get request succ.");

        // 根据类型分发消息
        iRet = DispatchMsg(msg);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "DispatchMsg failed.");
        }
    }

    SetThreadStatus(THREAD_STATUS_EXITED);
    COMMLOG(OS_LOG_INFO, "Process request succ.");
    return MP_SUCCESS;
}

mp_void TaskWorker::ExitError(message_pair_t &msg, mp_int64 exitCode)
{
    msg.pRspMsg->SetRetCode(exitCode);
    if (msg.pReqMsg->GetTypeID() == REQMESSAGE_TYPE) {
        Communication::GetInstance().PushRspMsgQueue(msg);
    } else {
        MessageHandler::GetInstance().PushRspMsg(msg);
    }
    DoSleep(TASK_WORKER_SLEEP_TIME);
}

mp_int32 TaskWorker::DispatchMsg(message_pair_t &msg)
{
    if (msg.pReqMsg->GetTypeID() == REQMESSAGE_TYPE) {
        CRequestMsg* requestMsg = dynamic_cast<CRequestMsg *>(msg.pReqMsg);
        CResponseMsg* responseMsg = dynamic_cast<CResponseMsg *>(msg.pRspMsg);
        mp_bool bCheck = requestMsg == NULL || responseMsg == NULL;
        if (bCheck) {
            COMMLOG(OS_LOG_ERROR, "convert messsage to CRequestMsg or CResponseMsg failed.");
            msg.pRspMsg->SetRetCode((mp_int64)ERROR_COMMON_PLUGIN_LOAD_FAILED);
            Communication::GetInstance().PushRspMsgQueue(msg);
            return MP_FAILED;
        }

        return DispatchRestMsg(*requestMsg, *responseMsg);
    } else if (msg.pReqMsg->GetTypeID() == DPPMESSAGE_TYPE) {
        CDppMessage* requestMsg = dynamic_cast<CDppMessage*>(msg.pReqMsg);
        CDppMessage* responseMsg = dynamic_cast<CDppMessage*>(msg.pRspMsg);

        mp_bool bCheck = requestMsg == NULL || responseMsg == NULL;
        if (bCheck) {
            COMMLOG(OS_LOG_ERROR, "convert messsage to CDppMessage failed.");
            msg.pRspMsg->SetRetCode((mp_int64)ERROR_COMMON_PLUGIN_LOAD_FAILED);
            MessageHandler::GetInstance().PushRspMsg(msg);
            return MP_FAILED;
        }

        return DispatchTcpMsg(*requestMsg, *responseMsg);
    } else {
        COMMLOG(OS_LOG_ERROR, "message type is invalid.");
        return MP_FAILED;
    }
}
#ifdef FRAME_SIGN
mp_int32 TaskWorker::DispatchRestMsg(CRequestMsg& requestMsg, CResponseMsg& responseMsg)
{
    message_pair_t msg(requestMsg, responseMsg);

    mp_string strService = requestMsg.GetURL().GetServiceName();
    DBGLOG("Get service %s.", strService.c_str());
    DBGLOG("Get uri: %s, method: %s.", requestMsg.GetURL().GetProcURL().c_str(),
        requestMsg.GetHttpReq().GetMethod().c_str());

    if (IsExternalPlugin(strService)) {
        mp_string oriURL = requestMsg.GetURL().GetOriURL();
        mp_size nPos = oriURL.find(strService);
        if (nPos != mp_string::npos) {
            oriURL.replace(nPos, strService.size(), "tasks");
        }
        requestMsg.GetURL().SetOriURL(oriURL);
        requestMsg.GetURL().SetQueryParam("appType=" + strService);
        strService = requestMsg.GetURL().GetServiceName();
        DBGLOG("Get service %s.", strService.c_str());
    }

    CServicePlugin* pPlugin = GetPlugin(strService);
    if (pPlugin == nullptr) {
        ERRLOG("Get plugin failed, type %s.", strService.c_str());
        ExitError(msg, (mp_int64)ERROR_COMMON_PLUGIN_LOAD_FAILED);
        return MP_FAILED;
    }

    SetPlugin(*pPlugin);
    mp_int32 iRet = pPlugin->Invoke(requestMsg, responseMsg);
    if (iRet != MP_SUCCESS) {
        if (iRet == MP_FAILED) {
            ExitError(msg, (mp_int64)ERR_OPERATION_FAILED);
        } else {
            ExitError(msg, (mp_int64)iRet);
        }
        return MP_FAILED;
    }
    Communication::GetInstance().PushRspMsgQueue(msg);
    return MP_SUCCESS;
}
#else
mp_int32 TaskWorker::DispatchRestMsg(CRequestMsg& requestMsg, CResponseMsg& responseMsg)
{
    message_pair_t msg(requestMsg, responseMsg);

    mp_string strService = requestMsg.GetURL().GetServiceName();
    COMMLOG(OS_LOG_DEBUG, "Get service %s.", strService.c_str());

    CServicePlugin* pPlugin = GetPlugin(strService);
    if (pPlugin == nullptr) {
        ERRLOG("Get plugin failed, type %s.", strService.c_str());
        ExitError(msg, (mp_int64)ERROR_COMMON_PLUGIN_LOAD_FAILED);
        return MP_FAILED;
    }

    // ȡǰĲƼscn
    SetPlugin(*pPlugin);
    mp_int32 iRet = pPlugin->Invoke(requestMsg, responseMsg);
    if (iRet != MP_SUCCESS) {
        ExitError(msg, iRet);
        return MP_FAILED;
    }

    Communication::GetInstance().PushRspMsgQueue(msg);
    return MP_SUCCESS;
}
#endif

mp_int32 TaskWorker::DispatchTcpMsg(CDppMessage& requestMsg, CDppMessage& responseMsg)
{
    message_pair_t msg(requestMsg, responseMsg);
    mp_uint32 manageCmd = requestMsg.GetManageCmd();
    if (manageCmd == MANAGE_CMD_NO_INVALID) {
        COMMLOG(OS_LOG_ERROR, "requestMsg have no valid manage cmd.");
        ExitError(msg, (mp_int64)ERROR_COMMON_PLUGIN_LOAD_FAILED);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Get service by manage cmd %u. sqno", manageCmd, requestMsg.GetOrgSeqNo());
    IPlugin* pPlg = m_plgMgr->GetPlugin(manageCmd);
    if (pPlg == NULL) {
        // the plugin loaded by tcp cmd must be preload, so return failed if there is no plugin
        COMMLOG(OS_LOG_ERROR, "Get plugin by manage cmd %u failed.", manageCmd);
        ExitError(msg, (mp_int64)ERROR_COMMON_PLUGIN_LOAD_FAILED);
        return MP_FAILED;
    }

    CServicePlugin *pPlugin = static_cast<CServicePlugin*>(pPlg);

    // 获取当前的插件名称及scn
    SetPlugin(*pPlugin);
    mp_int32 iRet = pPlugin->Invoke(requestMsg, responseMsg);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Invoke service plugin by tcp channel failed, iRet %d.", iRet);
    }
    MessageHandler::GetInstance().PushRspMsg(msg);
    return iRet;
}

/* ------------------------------------------------------------
Description  : 判断插件是否可以卸载(插件框架动态升级预?
Input        : newSCN -- scn?
                plgName -- 插件?
Return       : MP_TRUE -- 可以卸载
                MP_FALSE -- 不可以卸?
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_bool TaskWorker::CanUnloadPlugin(mp_uint64 newSCN, const mp_string& plgName)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    // work没有工作或者当前使用的插件不是需要删除的插件则可以删?
    if (m_plgName.empty() || strcmp(plgName.c_str(), m_plgName.c_str()) != 0) {
        return MP_TRUE;
    }

    // work当前使用的插件和需要删除的一致，scn一致说明work已经在使用新的插件则可以删除
    if (newSCN == m_SCN) {
        return MP_TRUE;
    }

    // 当前work还在使用旧的插件，不允许删除
    return MP_FALSE;
}

/* ------------------------------------------------------------
Description  : 根据服务名称获取插件
Input        : pszService -- 服务?
Return       : 成功返回获取的插件指针，失败返回NULL
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
CServicePlugin* TaskWorker::GetPlugin(const mp_string& pszService)
{
    mp_int32 iRet;
    plugin_def_t plgDef;

    COMMLOG(OS_LOG_DEBUG, "Begin get plugin.");
    if (pszService.empty()) {
        COMMLOG(OS_LOG_ERROR, "Input param is null.");
        return NULL;
    }
    // CodeDex误报，Dead Code
    iRet = m_plgCfgParse->GetPluginByService(pszService, plgDef);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get plugin failed.");
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, "Get plugin %s.", plgDef.name.c_str());
    IPlugin* pPlg = m_plgMgr->GetPlugin(plgDef.name);
    if (pPlg == NULL) {
        COMMLOG(OS_LOG_INFO, "Load new plugin %s.", plgDef.name.c_str());
        pPlg = m_plgMgr->LoadPlugin(plgDef.name);
        if (pPlg == NULL) {
            COMMLOG(OS_LOG_DEBUG, "Load plugin failed, name %s.", plgDef.name.c_str());
            return NULL;
        }
    }

    if (pPlg->GetTypeId() != CServicePlugin::APP_PUGIN_ID) {
        COMMLOG(OS_LOG_DEBUG,
            "Plugin's type is wrong. expect = %d, actual = %d.",
            CServicePlugin::APP_PUGIN_ID,
            pPlg->GetTypeId());
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, "Get plugin succ.");
    return static_cast<CServicePlugin*>(pPlg);
}

/* ------------------------------------------------------------
Description  : 保存插件相关信息(插件框架动态升级预?
Input        : pPlug -- 插件指针
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_void TaskWorker::SetPlugin(CServicePlugin& pPlug)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    m_plgVersion = pPlug.GetVersion();
    m_plgName = pPlug.GetName();
}

/* ------------------------------------------------------------
Description  : Task worker线程回调函数
Input        : pThis -- 线程回调函数参数
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
#ifdef WIN32
DWORD WINAPI TaskWorker::WorkProc(void* pThis)
#else
void* TaskWorker::WorkProc(void* pThis)
#endif
{
    TaskWorker* pTaskWorker = static_cast<TaskWorker*>(pThis);

    COMMLOG(OS_LOG_DEBUG, "Begin request process.");

    (void)pTaskWorker->ReqProc();

    COMMLOG(OS_LOG_DEBUG, "End request process.");

    CMPTHREAD_RETURN;
}

mp_bool TaskWorker::IsExternalPlugin(const mp_string& pszService)
{
    plugin_def_t plgDef;
    return m_plgCfgParse->GetPluginByService(pszService, plgDef) == MP_FAILED;
}