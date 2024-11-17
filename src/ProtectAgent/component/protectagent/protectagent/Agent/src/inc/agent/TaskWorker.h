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
#ifndef _AGENT_TASK_WORKER_H_
#define _AGENT_TASK_WORKER_H_

#include "common/Types.h"
#include "common/CMpThread.h"
#include "agent/Communication.h"
#include "pluginfx/iplugin.h"
#include "pluginfx/PluginCfgParse.h"
#include "pluginfx/PluginManager.h"
#include "plugins/ServicePlugin.h"
#include "message/Message.h"
#include "message/rest/message_process.h"
#include "message/tcp/CDppMessage.h"

enum WORK_STATUS { IDLE = 0, RUNNING = 1 };

class TaskWorker {
public:
    TaskWorker();
    virtual ~TaskWorker();

    virtual mp_int32 ReqProc();
    mp_bool NeedExit();
    mp_void SetThreadStatus(mp_int32 iThreadStatus);
    mp_bool GetThreadProcReqStatus();
    mp_int32 PopRequest(message_pair_t& msg);
    mp_void PushRequest(message_pair_t& msg);
    mp_int32 Init(PluginCfgParse& pPlgCfgParse, CPluginManager& pPlgMgr);
    mp_void Exit();
    mp_bool CanUnloadPlugin(mp_uint64 newSCN, const mp_string& plgName);

private:
    thread_lock_t m_tPlgLock;
    thread_lock_t m_tReqLock;
    thread_id_t m_threadId;
    mp_bool m_bClosed;
    std::vector<message_pair_t> m_vecReqQueue;
    mp_uint64 m_SCN;
    mp_string m_plgName;
    mp_string m_plgVersion;
    PluginCfgParse* m_plgCfgParse;
    CPluginManager* m_plgMgr;
    volatile mp_bool m_bProcReq;
    volatile mp_bool m_bNeedExit;       // 线程退出标识
    volatile mp_int32 m_iThreadStatus;  // 接收线程状态

private:
    CServicePlugin* GetPlugin(const mp_string& pszService);
    mp_void SetPlugin(CServicePlugin& pPlug);
#ifdef WIN32
    static DWORD WINAPI WorkProc(void* pThis);
#else
    static void* WorkProc(void* pThis);
#endif
    
    mp_void ExitError(message_pair_t &msg, mp_int64 exitCode);
    mp_int32 DispatchMsg(message_pair_t &msg);
    mp_int32 DispatchRestMsg(CRequestMsg &requestMsg, CResponseMsg &responseMsg);
    mp_int32 DispatchTcpMsg(CDppMessage &requestmsg, CDppMessage &responseMsg);
    
    mp_bool IsExternalPlugin(const mp_string& pszService);
};

#endif  // _AGENT_TASK_WORKER_H_
