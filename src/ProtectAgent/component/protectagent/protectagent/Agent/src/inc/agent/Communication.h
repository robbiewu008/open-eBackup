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
#ifndef _AGENT_COMMUNICATION_H_
#define _AGENT_COMMUNICATION_H_

#include "common/Types.h"
#include "message/Message.h"
#include "message/rest/http_cgi.h"
#include "message/rest/message_process.h"
#include "common/CMpThread.h"

// 定义预分配的FCGX_Request对象的使用状态
typedef struct tag_request_handler_info {
    FCGX_Request* pFcgxReq;
    mp_bool isUsed;
} request_handler_info_t;

class Communication {
public:
    static Communication& GetInstance()
    {
        static Communication m_instance;
        return m_instance;
    }

    ~Communication();
    EXTER_ATTACK mp_int32 Init();
    mp_bool IsQueueEmpty()
    {
        return m_reqMsgQueue.empty() ? true : false;
    }

    mp_void ReleaseRequest(const FCGX_Request& pReq);
    mp_int32 PopReqMsgQueue(message_pair_t& msgPair);
    mp_void PushReqMsgQueue(const message_pair_t& msgPair);
    mp_int32 PopRspMsgQueue(message_pair_t& msgPair);
    mp_void PushRspMsgQueue(const message_pair_t& msgPair);
    mp_int32 PopRspInternalMsgQueue(message_pair_t& msgPair);
    mp_int32 GetFrontReqMsgQueue(message_pair_t& msgPair);

private:
    Communication()
    {
        (mp_void) memset_s(&m_hReceiveThread, sizeof(m_hReceiveThread), 0, sizeof(m_hReceiveThread));
        (mp_void) memset_s(&m_hSendThread, sizeof(m_hSendThread), 0, sizeof(m_hSendThread));
        // Coverity&Fortify误报:UNINIT_CTOR
        // Coveirty&Fortify不认识公司安全函数memset_s，提示m_dispatchTid.os_id未初始化
        CMpThread::InitLock(&m_reqTableMutex);
        CMpThread::InitLock(&m_reqMsgQueueMutex);
        CMpThread::InitLock(&m_rspMsgQueueMutex);
        m_iRecvThreadStatus = THREAD_STATUS_IDLE;
        m_iSendThreadStatus = THREAD_STATUS_IDLE;
        m_bNeedExit = MP_FALSE;
    }
    mp_int32 InitRequest(mp_int32 handler);
    mp_int32 InitThread(mp_int32 handler);
    mp_void DeleteRequest();
    mp_bool NeedExit() const;
    mp_void SetRecvThreadStatus(mp_int32 iThreadStatus);
    mp_void SetSendThreadStatus(mp_int32 iThreadStatus);
    FCGX_Request* GetFcgxReq();
#ifdef WIN32
    static DWORD WINAPI ReceiveThreadFunc(mp_void *pThis);
    static DWORD WINAPI SendThreadFunc(mp_void *pThis);
#else
    static mp_void* ReceiveThreadFunc(mp_void *pThis);
    static mp_void* SendThreadFunc(mp_void *pThis);
#endif
    static mp_int32 CheckHeader(Communication& pInstance, FCGX_Request& pFcgiReq, CRequestMsg& req);
    static mp_int32 CheckAuth(Communication& pInstance, FCGX_Request& pFcgiReq, CRequestMsg& req);
    static mp_void SendFailedMsg(
        Communication& pInstance, FCGX_Request& pFcgiReq, mp_int32 iHttpStatus, mp_int32 iRetCode);
    static mp_void HandleReceiveMsg(Communication& pInstance, FCGX_Request& pFcgiReq);
    static mp_int32 CheckIPandPort(mp_string& strPort);

private:
    static const mp_uint32 RCV_SLEEP_TIME = 20;
    std::vector<request_handler_info_t> m_ReqHandlerTable;  // 存储预分配的FCGX_Request对象
    thread_lock_t m_reqTableMutex;                     // m_ReqHandlerTable访问互斥锁
    std::vector<message_pair_t> m_reqMsgQueue;              // 接收消息队列
    thread_lock_t m_reqMsgQueueMutex;                  // m_reqMsgQueue访问互斥锁
    std::vector<message_pair_t> m_rspMsgQueue;              // 发送消息队列
    thread_lock_t m_rspMsgQueueMutex;                  // m_reqMsgQueue访问互斥锁
    thread_id_t m_hReceiveThread;                      // 创建的接收线程句柄
    thread_id_t m_hSendThread;                         // 创建的发送线程句柄
    volatile mp_bool m_bNeedExit;                      // 线程退出标识
    volatile mp_int32 m_iRecvThreadStatus;             // 接收线程状态
    volatile mp_int32 m_iSendThreadStatus;             // 发送线程状态
};

#endif  // _AGENT_COMMUNICATION_H_
