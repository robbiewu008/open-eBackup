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
#ifndef __AGENT_VDDK_DEADLOCK_CHECK_H__
#define __AGENT_VDDK_DEADLOCK_CHECK_H__

#include <map>
#include "common/Types.h"
#include "dataprocess/vmwarenative/MessageLoopThread.h"
#include "common/ConfigXmlParse.h"

typedef struct VddkApiInfo {
    std::string strName;
    std::chrono::steady_clock::time_point invokeTime;
    uint64_t requestID;
    uint32_t timeout;
} st_vddkApiInfo;

class VddkDeadlockCheck {
public:
    /*
     * A deadlock check thread will be started, run and stopped.
     * Check rule: the invoke time of a api call
     */
    class DeadlockCheck {
    public:
        DeadlockCheck(const std::string& strApiName);
        virtual ~DeadlockCheck();
        friend class VddkDeadlockCheck;

    private:
        uint64_t m_ID;
    };

    static VddkDeadlockCheck* GetInstance();

private:
    VddkDeadlockCheck();
    virtual ~VddkDeadlockCheck();

    EXTER_ATTACK void DoDeadlockCheck();
    uint64_t GenerateID(const uint64_t& requestID, const std::string& strApiName);
    bool StartDeadlockThread();
    bool GenerateIDInner(const uint64_t requestID, const std::string& strApiName, uint64_t& tempID);
    void ReleaseID(const uint64_t& ID);
    void KillProcess();

private:
    static std::mutex m_mutex;
    static uint32_t MAX_TRY_TIMES;
    uint64_t m_index;
    int m_invokeTimeout;  // elapsed time of a API call
    std::map<mp_uint64, st_vddkApiInfo> m_invokingApis;
    st_vddkApiInfo m_vddkApiInfo;
    MessageLoopThread m_deadlockCheckThread;  // deadlock check thread
};
#endif
