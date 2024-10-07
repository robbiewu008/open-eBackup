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
#include "SmbContextContainer.h"

namespace {
    constexpr auto MODULE = "SMB_CONTEXT_CONTAINER";
    constexpr uint16_t MAX_CNT = 1000;
}

using namespace Module;
using namespace std;

SmbContextContainer::SmbContextContainer(size_t reqId)
    : m_reqId(reqId)
{
    HCP_Log(INFO, MODULE) << "SmbContextContainer constructed!" << HCPENDLOG;
}

SmbContextContainer::~SmbContextContainer()
{
    HCP_Log(INFO, MODULE) << "SmbContextContainer destructed!" << HCPENDLOG;
}

shared_ptr<SmbContextWrapper> SmbContextContainer::Get(string guid)
{
    lock_guard<std::mutex> lk(mtx);
    auto iter = m_smbContexts.find(guid);
    if (iter == m_smbContexts.end()) {
        return nullptr;
    }
    return iter->second;
}

void SmbContextContainer::Insert(shared_ptr<SmbContextWrapper> smbContext)
{
    lock_guard<std::mutex> lk(mtx);
    if (smbContext != nullptr) {
        string guid = smbContext->SmbGetClientGuid();
        m_smbContexts.emplace(guid, smbContext);
    }
}

shared_ptr<SmbContextWrapper> SmbContextContainer::Traverse()
{
    lock_guard<std::mutex> lk(mtx);
    if (m_currentIter == m_smbContexts.end()) {
        m_currentIter = m_smbContexts.begin();
    }
    shared_ptr<SmbContextWrapper> smbContext = m_currentIter->second;
    m_currentIter++;
    return smbContext;
}

void SmbContextContainer::Erase(const string& guid)
{
    lock_guard<std::mutex> lk(mtx);
    m_smbContexts.erase(guid);
    m_currentIter = m_smbContexts.end();
}

size_t SmbContextContainer::Size()
{
    return m_smbContexts.size();
}

uint64_t SmbContextContainer::GetCurTimeSecond()
{
    timeval curTime {};
    gettimeofday(&curTime, nullptr);
    return curTime.tv_sec;
}

uint32_t SmbContextContainer::Poll()
{
    struct pollfd pfd = {0};
    bool newEventFlag = false;
    short revents = 0;
    int ret = 0;
    uint64_t enterPollTime = GetCurTimeSecond();

    do {
        newEventFlag = false;
        for (auto smbContext : m_smbContexts) {
            smbContext.second->InitPfd(pfd);
            ret = poll(&pfd, 1, 0);
            if (ret < 0) {
                HCP_Log(ERR, MODULE) << "Poll failed, errno:" << errno << HCPENDLOG;
                return FAILED;
            } else if (ret == 0) {
                continue;
            } else {
                revents = pfd.revents;
            }

            uint64_t pollTime = GetCurTimeSecond();
            if (pfd.fd == -1 && (pollTime - enterPollTime > (uint64_t)(smbContext.second->GetTimeout()))) {
                HCP_Log(ERR, MODULE) << "Timeout expired and no connection exists" << HCPENDLOG;
                break;
            }
            if (pfd.revents == 0) {
                continue;
            }
            // start a job scheduler
            
            if (!((uint32_t)revents & (POLLERR|POLLHUP))) {
                newEventFlag = true;
            }
        }
    } while (newEventFlag);
    return SUCCESS;
}

uint32_t SmbContextContainer::DestroySmbContext()
{
    for (auto smbContext : m_smbContexts) {
        smbContext.second->SmbDisconnect();
    }
    m_smbContexts.clear();
    m_sendReqCount = 0;
    return SUCCESS;
}
