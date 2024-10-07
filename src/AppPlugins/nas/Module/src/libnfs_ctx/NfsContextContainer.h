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
#ifndef MODULE_NFS_CONTEXT_CONTAINER_H
#define MODULE_NFS_CONTEXT_CONTAINER_H

#include <map>
#include <mutex>
#include "define/Types.h"
#include "ThreadPoolFactory.h"
#include "NfsContextWrapper.h"

namespace Module {
constexpr uint8_t POLL_WAIT_10MS = 10;

class NfsContextContainer {
public:
    explicit NfsContextContainer(std::size_t reqId = 0);
    ~NfsContextContainer();
    std::shared_ptr<NfsContextWrapper> Get(int id);
    void Insert(int id, std::shared_ptr<NfsContextWrapper> nfsContext);
    std::shared_ptr<NfsContextWrapper> Traverse();
    std::map<int, std::shared_ptr<NfsContextWrapper>>::size_type Size();
    int64_t GetCurTime();
    int32_t DestroyNfsContext();

    inline void IncSendCnt(int32_t reqPerNfsContext)
    {
        m_sendReqCount++;
        if (reqPerNfsContext > 0 && m_sendReqCount % reqPerNfsContext == 0) {
            Traverse();
            return;
        }
        return;
    }
    void PrintCounters(const std::string containerType);
    std::shared_ptr<NfsContextWrapper> GetCurrContext();
private:
    std::mutex mtx {};
    std::size_t m_reqId = 0;
    std::map<int, std::shared_ptr<NfsContextWrapper>> m_nfsContexts {};
    std::map<int, std::shared_ptr<NfsContextWrapper>>::iterator m_currentIter = m_nfsContexts.end();
    uint64_t m_currJobCount = 0;
    uint64_t m_totJobCount = 0;
    uint64_t m_timedNfsServiceCount = 0;
    // Module::JobScheduler m_jobScheduler;
    int64_t m_sendReqCount = 0;
    // ThreadPool *m_threadPool {};
    bool m_printed = false;
};

constexpr int NFS_SERVICE_CIRCLE = 1000;
}
#endif // MODULE_NFS_CONTEXT_CONTAINER_H
