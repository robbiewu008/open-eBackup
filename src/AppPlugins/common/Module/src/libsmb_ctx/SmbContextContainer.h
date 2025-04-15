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
#ifndef LIBSMB_CONTEXT_CONTAINER_H
#define LIBSMB_CONTEXT_CONTAINER_H

#include <map>
#include <mutex>
#include "SmbContextWrapper.h"

constexpr uint8_t SMB_POLL_WAIT_10MS = 10;


namespace Module {
class SmbContextContainer {
public:
    explicit SmbContextContainer(std::size_t reqId = 0);
    ~SmbContextContainer();
    std::shared_ptr<SmbContextWrapper> Get(std::string guid);
    void Insert(std::shared_ptr<SmbContextWrapper> smbContext);
    std::shared_ptr<SmbContextWrapper> Traverse();
    void Erase(const std::string& guid);
    std::size_t Size();
    uint64_t GetCurTimeSecond();
    uint32_t Poll();
    uint32_t DestroySmbContext();
    inline bool IncSendReqCntAndCheck(uint32_t reqPerSmbContext)
    {
        m_sendReqCount++;
        if (reqPerSmbContext > 0 && m_sendReqCount % reqPerSmbContext == 0) {
            return true;
        }
        return false;
    }

    uint32_t TimedSmbService();

private:
    using SmbContextMap = std::map<std::string, std::shared_ptr<SmbContextWrapper> >;
    std::mutex mtx {};
    std::size_t m_reqId = 0;
    SmbContextMap m_smbContexts {};
    SmbContextMap::iterator m_currentIter = m_smbContexts.end();
    uint64_t m_currJobCount = 0;
    uint64_t m_totJobCount = 0;
    uint64_t m_sendReqCount = 0;
    bool m_printed = false;
};

constexpr int SMB_SERVICE_CIRCLE = 1000;

}
#endif // LIBSMB_CONTEXT_CONTAINER_H